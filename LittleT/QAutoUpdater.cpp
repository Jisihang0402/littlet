#include "QAutoUpdater.h"
#include "ui/QUIGlobal.h"
#include <algorithm>
#include <iterator>
#include "inet/WinHttpClient.h"
#include "crypt/QDataEnDe.h"
#include "AppHelper.h"

VOID CALLBACK CheckUpdateCallback(
    __in  PVOID lpParameter,__in  BOOLEAN TimerOrWaitFired)
{
    QAutoUpdater* pThis = reinterpret_cast<QAutoUpdater*>(lpParameter);
    if (NULL == pThis)
    {
        ASSERT(FALSE);
        return ;
    }
    pThis->CheckUpdate();
}

QAutoUpdater::QAutoUpdater(void)
{
    //m_bEnd = FALSE;
    m_hTimer = NULL;
}

BOOL QAutoUpdater::CheckUpdate( )
{
    _Download_Param *p = new _Download_Param;
    p->sURL = m_sUrl;
    p->sRefer = m_sRefer;
    p->pData = this;
    return _beginthreadex(NULL,0, &QAutoUpdater::thread_download,p,0,NULL) > 0;
}

BOOL IsHigherVersion(QString& sVer)
{
    // �汾��Ӧ��ֻ��һ��С����
    return _wtof(sVer.Trim()) > _wtof(QUIGetAppVersion());
}

BOOL ExtractVerAndUrl(__in QString& sHtml, __out QString& sVersion,
    __out QString& sFileUrl)
{
    // ��δ�������İ�������������ʽ��
    // version
    int ib = sHtml.Find(L"[[[ver:");
    if (-1 == ib)
        return FALSE;
    ib += 7;
    int ie = sHtml.Find(L"]]]", ib);
    if (-1 == ie)
        return FALSE;
    sVersion = sHtml.Mid(ib, ie-ib);

    // file
    sHtml = sHtml.Mid(ie);
    ib = sHtml.Find(L"--begin--");
    if (-1 == ib)
        return FALSE;
    ib = sHtml.Find(L"real_src",ib);
    if (-1 == ib)
        return FALSE;
    ib += 8;
    ie = sHtml.Find(L"&amp;", ib);
    if (-1 == ie)
        return FALSE;
    sFileUrl = sHtml.Mid(ib, ie-ib);
    ib = sFileUrl.Find(L"http://");
    if (-1 == ib)
        return FALSE;
    sFileUrl = sFileUrl.Mid(ib);
    return TRUE;
}

BOOL DecryptUpdateFile(QBuffer& bufEncrypt, const QString& sSavePath)
{
    DWORD dwPicSize = bufEncrypt.GetBufferLen();
    if (dwPicSize < 2048)
    {
        return FALSE;
    }

    // ���д��20���ֽڣ�˳������

    // ��Կ��ͼ�������е�ƫ��
    DWORD dwOffset;
    bufEncrypt.ReadLast((BYTE*)&dwOffset,sizeof(DWORD));
    // ��Կ����
    DWORD dwKeyLen;
    bufEncrypt.ReadLast((BYTE*)&dwKeyLen,sizeof(DWORD));
    // ͼ���С
    bufEncrypt.ReadLast((BYTE*)&dwPicSize,sizeof(DWORD));
    // ���ܺ�����ݳ���
    DWORD dwEncrypDataLen;
    bufEncrypt.ReadLast((BYTE*)&dwEncrypDataLen,sizeof(DWORD));
    // ԭʼ���ݳ���
    DWORD dwOriginLen;
    bufEncrypt.ReadLast((BYTE*)&dwOriginLen,sizeof(DWORD));

    QBuffer bufData;
    if (!bufData.AllocBuffer(dwEncrypDataLen))
    {
        return FALSE;
    }
    if (!bufData.Write(bufEncrypt.GetBuffer(dwPicSize),dwEncrypDataLen))
    {
        // eInfo.SetText(L"��ȡ���ݴ���");
        return FALSE;
    }

    QDataEnDe ende;
    if (!ende.SetCodeData(bufEncrypt.GetBuffer(dwOffset),dwKeyLen))
    {
        return FALSE;
        // eInfo.SetText(L"������������ʧ��");
    }
    if (!ende.DecryptData(bufData))
    {
        // eInfo.SetText(L"��������ʧ��!");
        return FALSE;
    }
    if (!bufData.FileWrite(sSavePath))
    {
        // eInfo.SetText(L"��������ʧ��");
        return FALSE;
    }

    // eInfo.SetText(L"ȫ���㶨�����Ѿ�����Ϊ��" + sFileName);
    return TRUE;
}

uint __stdcall QAutoUpdater::thread_download( void* pparam )
{
    _Download_Param *p = (_Download_Param*)pparam;
    if (NULL == p)
    {
        ASSERT(FALSE);
        return 0;
    }

    BOOL bOK = FALSE;
    WinHttpClient cl(p->sURL);
    //WinHttpClient cl(L"http://blog.sina.com.cn/s/blog_936e65db0101n2y7.html");
    cl.SetUserAgent(L"Mozilla/5.0 (Windows NT 6.1; WOW64) "
        L"AppleWebKit/535.7 (KHTML, like Gecko) Chrome/16.0.912.75 Safari/535.7");
    cl.SetReferrer(p->sRefer);
    //cl.SetReferrer(L"http://blog.sina.com.cn");

    // �ȼ���Ƿ��и���
    QString sVer = L"0", sFileUrl;
    for (int iTry = 0; iTry < 3; iTry++)
    {
        if (!cl.SendHttpRequest())
            continue;
        
        // ����������
        QString sHtml = cl.GetResponseContent().c_str();
        if ( ExtractVerAndUrl(sHtml, sVer, sFileUrl) )
        {
            bOK = TRUE;
        }
        break;
    }

    QString sSavePath;
    BOOL bHasUpdate = IsHigherVersion(sVer);
    if (bOK && bHasUpdate)
    {
        bOK = FALSE;
        for (int iTry = 0; iTry < 3; iTry++)
        {
            cl.UpdateUrl(sFileUrl);
            if (!cl.SendHttpRequest())
                continue;

            // ����������
            p->bufDown.ClearBuffer();
            p->bufDown.Write(cl.GetRawResponseContent(),
                cl.GetRawResponseReceivedContentLength());
            bOK = TRUE;
            break;
        }
        // ���سɹ�
        if (bOK)
        {
            sSavePath = qcwbase::GetModulePath() + L"__temp_update.zip";
            if ( !DecryptUpdateFile(p->bufDown, sSavePath))
            {
                bOK = FALSE;
            }
        }
    }

    if (bHasUpdate)
    {
        LPQUI_USERMSGPARAM pMsg = QUIGetUserMsgParam(
                (bOK)?1:0,   // ���ظ����Ƿ����
                0,  // ��ʱ��ʹ��
                NULL,
                sSavePath, // �����ļ��ĵ�ַ
                TRUE);  // ������Ӧ���ͷ��ڴ�
        // ֪ͨ�������
        ::PostMessage(QUIGetMainWnd(), QSOFT_MSG_UPDATEAPP, 
                1, // �и���
                (LPARAM)pMsg);
    }
    else
    {
        // ֪ͨ�������
        ::PostMessage(QUIGetMainWnd(), QSOFT_MSG_UPDATEAPP, 
                0, // û�и���
                0);
    }
    delete p;
    return 0;
}

BOOL QAutoUpdater::Startup( __in const QString& sURL, __in LPCWSTR szRefer,
    __in DWORD dwCheckPeriod /*= 30*/ )
{
    if (NULL != m_hTimer)
    {
        ASSERT(FALSE);  
        return TRUE;
    }
    m_sUrl = sURL;
    m_sUrl.Trim();
    m_sRefer = szRefer;
    m_sRefer.Trim();
    // ����3���ʱ���ȼ��һ�θ���
    return CreateTimerQueueTimer(&m_hTimer, NULL, CheckUpdateCallback, this,
            3*1000, dwCheckPeriod * 60 * 1000, WT_EXECUTEDEFAULT);
}

