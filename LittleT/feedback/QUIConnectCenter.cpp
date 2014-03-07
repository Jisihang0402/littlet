#include "QUIConnectCenter.h"
#include "ui/QUIMgr.h"
#include "file/rjson/inc.h"
#include "ui/QUIGlobal.h"
#include <algorithm>
#include <iterator>
#include "crypt/QDataEnDe.h"
#include "AppHelper.h"
#include <atlpath.h>
#include "ui/QConfig.h"
#include "ui/QApp.h"

#ifdef _WININET_

#undef BOOLAPI
#undef SECURITY_FLAG_IGNORE_CERT_DATE_INVALID
#undef SECURITY_FLAG_IGNORE_CERT_CN_INVALID
#define URL_COMPONENTS URL_COMPONENTS_ANOTHER
#define URL_COMPONENTSA URL_COMPONENTSA_ANOTHER
#define URL_COMPONENTSW URL_COMPONENTSW_ANOTHER
#define LPURL_COMPONENTS LPURL_COMPONENTS_ANOTHER
#define LPURL_COMPONENTSA LPURL_COMPONENTS_ANOTHER
#define LPURL_COMPONENTSW LPURL_COMPONENTS_ANOTHER
#define INTERNET_SCHEME INTERNET_SCHEME_ANOTHER
#define LPINTERNET_SCHEME LPINTERNET_SCHEME_ANOTHER
#define HTTP_VERSION_INFO HTTP_VERSION_INFO_ANOTHER
#define LPHTTP_VERSION_INFO LPHTTP_VERSION_INFO_ANOTHER

//#include <winhttp.h>
#include "inet/WinHttpClient.h"

#undef URL_COMPONENTS
#undef URL_COMPONENTSA
#undef URL_COMPONENTSW
#undef LPURL_COMPONENTS
#undef LPURL_COMPONENTSA
#undef LPURL_COMPONENTSW
#undef INTERNET_SCHEME
#undef LPINTERNET_SCHEME
#undef HTTP_VERSION_INFO
#undef LPHTTP_VERSION_INFO

#endif


BOOL QUIConnectCenter::_QUrlExtract::Init()
{
    if (update_url_.size() || user_url_.size())
        return TRUE;

    // ��QUIMgr���������
    QBuffer *pB;
    if (!QUIMgr::GetInstance()->LoadData(L"qabs:url.x", (QView*)nullptr, pB))
    {
        ASSERT(FALSE);
        return FALSE;
    }

    // L"{"
    // L"    "update":"
    // L"    {"
    // L"        "1":"
    // L"        {"
    // L"            "url":"http://www.qiuchengw.com/soft/yulu.htm","
    // L"			"domain":"http://www.qiuchengw.com""
    // L"        }"
    // L"        ,"
    // L"        "2":"
    // L"		{"
    // L"			"url":"http://blog.sina.com.cn/s/blog_936e65db0101n2y7.html","
    // L"			"domain":"http://blog.sina.com.cn""
    // L"		}"
    // L"    },"
    // L"    "user":"
    // L"    {"
    // L"        "1":"
    // L"        {"
    // L"            "url":"http://www.qiuchengw.com/soft/yulu.htm","
    // L"			"domain":"http://www.qiuchengw.com""
    // L"        }"
    // L"        ,"
    // L"        "2":"
    // L"		{"
    // L"			"url":"http://blog.sina.com.cn/s/blog_936e65db0101n2y7.html","
    // L"			"domain":"http://blog.sina.com.cn""
    // L"		}"
    // L"    }"
    // L"}
    RJsonDocumentW d;
    ATL::CA2WEX<256> aw((char*)pB->GetBuffer());
    d.Parse<0>(aw);
    if (d.HasParseError())
        return FALSE;

    // update
    RJsonValueW& upd = d[L"update"];
    if (upd.IsObject())
    {
        for (auto i = upd.MemberBegin(); i != upd.MemberEnd(); ++i)
        {
            RJsonValueW& v = i->value;
            if (v.IsObject())
            {
                _Url u;
                u.url_ = v[L"url"].GetString();
                u.domain_ = v[L"domain"].GetString();
                update_url_.push_back(u);
            }
        }
    }

    // client
    RJsonValueW& user = d[L"user"];
    if (user.IsObject())
    {
        for (auto i = user.MemberBegin(); i != user.MemberEnd(); ++i)
        {
            RJsonValueW& v = i->value;
            _Url u;
            u.url_ = v[L"url"].GetString();
            u.domain_ = v[L"domain"].GetString();
            user_url_.push_back(u);
        }
    }

    RJsonValueW& sta = d[L"stat"];
    if (sta.IsString())
    {
        stat_url_ = sta.GetString();
    }

    return TRUE;
}

BOOL QUIConnectCenter::_QUrlExtract::GetAddr(__in LstAddr& lst, __inout _Url& adr)
{
    if (lst.empty())
        return FALSE;

    if (adr.url_.IsEmpty())
    {
        adr = lst.front();
        return TRUE;
    }

    auto iEnd = lst.end();
    for (auto i = lst.begin(); i != iEnd; ++i)
    {
        if (adr.url_.CompareNoCase((*i).url_) == 0)
        {
            if (++i != iEnd)
            {
                adr = *i;
                return TRUE;
            }
        }
    }
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////

BOOL QUIConnectCenter::VisitStatisticPage()
{
    QString sUrl = ui_url_.GetStaticsUrl();
    if (!sUrl.IsEmpty())
    {
        AddDownload(sUrl);
    }
    return TRUE;
}

BOOL QUIConnectCenter::LoadBaseData(__in const QString& sDataFile)
{
    if (!basedata_.LoadDataFile(sDataFile))
        return FALSE;

    // ��ȡqiuinfo
    QBuffer *pBuf = basedata_.GetResource(UIBASE_MAILPSWD_PATH);
    if (nullptr != pBuf)
    {
        ATL::CA2WEX<256> aw((LPCSTR)pBuf->GetBuffer());
        qiu_info_.SetAddrData((LPCWSTR)aw);
    }

    return TRUE;
}

void QUIConnectCenter::OnClose()
{
//     if (QUIGetApp()->IsQuiting())
//     {
//         SetMsgHandled(FALSE);
//     }
}

BOOL QUIConnectCenter::Init()
{
    if (IsInited())
    {
        return TRUE;
    }

    Create(NULL, CRect(0,0,0,0), NULL, 
        WS_POPUP, WS_EX_TOOLWINDOW|WS_EX_TRANSPARENT );

    if (!IsWindow())
        return FALSE;

    if (!ui_url_.Init())
        return FALSE;

    return TRUE;
}

void QUIConnectCenter::OnAfterRequestSend( FCHttpRequest& rTask )
{

}

void QUIConnectCenter::OnAfterRequestFinish( FCHttpRequest& rTask )
{

}

void QUIConnectCenter::OnOverRequestLifetime( FCHttpRequest& rTask )
{

}


//////////////////////////////////////////////////////////////////////////
QAutoUpdater::QAutoUpdater(void)
{
    //m_bEnd = FALSE;
    m_hTimer = NULL;
}

VOID CALLBACK QAutoUpdater::CheckUpdateCallback(
    __in  PVOID lpParameter,__in  BOOLEAN TimerOrWaitFired)
{
    QAutoUpdater* pThis = reinterpret_cast<QAutoUpdater*>(lpParameter);
    if (NULL == pThis)
    {
        ASSERT(FALSE);
        return ;
    }
    pThis->CheckUpdate();
    pThis->CheckBaseData();
}

BOOL QAutoUpdater::CheckUpdate( )
{
    _Download_Param *p = new _Download_Param(TASK_TYPE_UPDATION);
    p->sURL = url_updation_.url_;
    p->sRefer = url_updation_.domain_;
    p->pData = this;
    HANDLE hThread = (HANDLE)_beginthreadex(NULL,0, 
        &QAutoUpdater::thread_download,p,0,NULL);
    if (hThread > 0)
    {
        CloseHandle(hThread);
        return TRUE;
    }
    delete p;
    return FALSE;
}

BOOL QAutoUpdater::CheckBaseData( )
{
    _Download_Param *p = new _Download_Param(TASK_TYPE_UPDATION);
    p->sURL = url_basedata_.url_;
    p->sRefer = url_basedata_.domain_;
    p->pData = this;
    HANDLE hThread = (HANDLE)_beginthreadex(NULL,0, 
        &QAutoUpdater::thread_download,p,0,NULL);
    if (hThread > 0)
    {
        CloseHandle(hThread);
        return TRUE;
    }
    delete p;
    return FALSE;
}

BOOL IsHigherVersion(QString& sVer, BOOL IsUpdation)
{
    // �汾��Ӧ��ֻ��һ��С����
    if (IsUpdation) 
    {
        return _wtof(sVer.Trim()) > _wtof(QUIGetAppVersion());
    }

    // �������ļ��ж�ȡ
    QString str = QUIGetConfig()->GetValue(L"APP",L"base_data_ver").Trim();

    // ��������ļ������ڣ���ôҲ��Ϊ�и��µ�
    return (_wtof(sVer.Trim()) > _wtof(str)) 
        || !ATL::ATLPath::FileExists(__BasedataCacheFile());
}

// J��¼�汾��
void SaveBaseDataVersion(const QString& sVer)
{
    QUIGetConfig()->SetValue(L"APP",L"base_data_ver", sVer);
}

BOOL updation_ExtractVerAndUrl(__in QString& sHtml, __out QString& sVersion,
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

BOOL basedata_ExtractVerAndUrl(__in QString& sHtml, __out QString& sVersion,
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

BOOL DecryptPictureData(__in QBuffer& bufEncrypt, __out QBuffer& bufData)
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

    // eInfo.SetText(L"ȫ���㶨�����Ѿ�����Ϊ��" + sFileName);
    return TRUE;
}

BOOL DecryptUpdateFile(__in QBuffer& bufEncrypt, __in const QString& sSavePath)
{
    QBuffer bufData;
    if (DecryptPictureData(bufEncrypt, bufData))
    {
        if (bufData.FileWrite(sSavePath))
        {
            // eInfo.SetText(L"��������ʧ��");
            return TRUE;
        }
    }
    return FALSE;
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
        if (p->IsUpdationTask())
        {
            // �Զ�����
            bOK = updation_ExtractVerAndUrl(sHtml, sVer, sFileUrl);
        }
        else
        {
            // ��������
            bOK = basedata_ExtractVerAndUrl(sHtml, sVer, sFileUrl);
        }
        break;
    }

    QString sSavePath;
    BOOL bHasUpdate = IsHigherVersion(sVer, p->IsUpdationTask());
    if (!bOK || !bHasUpdate)
    {
        delete p;
        return 0;
    }

    bOK = FALSE;
    // ������ȥ��������
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
        // ������µĻ���������д����ʱ�ļ���
        if (p->IsUpdationTask())
        {
            sSavePath = __UpdationTempFile();
            bOK = DecryptUpdateFile(p->bufDown, sSavePath);
        }
        // �������ݾ�ֱ��д�����ͺ��ˣ���Ҫ���ܣ��ٺ٣��������ܰ�
        else
        {
            sSavePath = __BasedataCacheFile();
            bOK = p->bufDown.FileWrite(sSavePath);
            if (bOK)
            {
                SaveBaseDataVersion(sVer);
            }
        }
    }

    if (bHasUpdate)
    {
        LPQUI_USERMSGPARAM pMsg = QUIGetUserMsgParam(
            (bOK) ? 1 : 0,   // ���ظ����Ƿ����
            p->IsUpdationTask() ? 0 : 1, 
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

// BOOL QAutoUpdater::Startup( __in _Url& urlUpdation, 
//     __in _Url& urlBaseData, __in DWORD dwCheckPeriod /*= 30*/ )
// {
//     if (NULL != m_hTimer)
//     {
//         ASSERT(FALSE);  
//         return TRUE;
//     }
// 
//     url_updation_ = urlUpdation;
//     url_basedata_ = urlBaseData;
// 
//     dwCheckPeriod = max(10, dwCheckPeriod);
// 
//     // ����15���ʱ���ȼ��һ�θ���
//     return CreateTimerQueueTimer(&m_hTimer, NULL, CheckUpdateCallback, this,
//         15 * 1000, dwCheckPeriod * 60 * 1000, WT_EXECUTEDEFAULT);
// }

BOOL QAutoUpdater::Startup( __in DWORD dwCheckPeriod /*= 30*/ )
{
    if (NULL != m_hTimer)
    {
        ASSERT(FALSE);  
        return TRUE;
    }

    QUIConnectCenter*pCC = QUIConnectCenter::GetInstance();

    if (pCC->GetUpdationUrl(url_updation_) 
        && pCC->GetUserUrl(url_basedata_))
    {
        dwCheckPeriod = max(10, dwCheckPeriod);
        
        // ����15���ʱ���ȼ��һ�θ���
        return CreateTimerQueueTimer(&m_hTimer, NULL, CheckUpdateCallback, this,
            15 * 1000, dwCheckPeriod * 60 * 1000, WT_EXECUTEDEFAULT);
    }
    return FALSE;
}


