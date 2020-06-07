#include "LittleT.h"
#include "../common/LittleTUIcmn.h"
#include "../common/Worker.h"
#include "../common/QLongplan.h"
#include "../common/QTimerMan.h"

#include "time/QTime.h"
#include "sys/singleinst.h"
#include "deps/sys/UACSElfElevation.h"
#include "deps/img/GDIpInitializer.h"
#include "ui/QUIMgr.h"
#include "ui/QUIDlgs.h"

#include "auto_update.h"

LittleTApp _Module;

#include "LAboutDlg.h"
#include "deps/FileVersion.hpp"
#include "deps/app.h"
#include "qstrex.h"

BOOL LittleTApp::InitRun()
{
    static CSingleInstance _theInst;
    if (_theInst.InstanceAlreadyRun(LITTLET_APP_NAME))
    {
        return FALSE;
    }
    // ��ʼ��gdiplus
    CGDIpInitializer::Get()->Initialize();

    // ��ЩĿ¼�Ǳ������ȴ��ڵ�
    LittleTConfig *pCfg = (LittleTConfig*)QUIGetConfig();
    // icon dir
    quibase::MakeSureDirExist(pCfg->GetIconsDir());

    SetMainWnd(&m_frame);

    // ��Ϣ��ȡ
//     if (!QUIConnectCenter::GetInstance()->Init())
//     {
//         XMsgBox::ErrorMsgBox(L"������Ϣ��ȡʧ�ܣ�");
// 
//         // return false;
//     }

    if (!QWorker::GetInstance()->Startup())
    {
        XMsgBox::ErrorMsgBox(L"�������߳�����ʧ��!");
        return FALSE;
    }

    if (   !QDBEvents::GetInstance()->Startup()
        || !QPlanMan::GetInstance()->Startup()
        || !QResMan::GetInstance()->Startup(pCfg->GetIconsDir()))
    {
        XMsgBox::ErrorMsgBox(L"���ݿ�δ���������أ�");
        return FALSE;
    }

    // ����ʱ�������
    if ( !QTimerMan::GetInstance()->Startup() )
    {
        XMsgBox::ErrorMsgBox(L"ʱ�����������ʧ�ܣ�");
        return FALSE;
    }
    // �����Զ����������
    if ( !QAutoTaskMan::GetInstance()->Startup() )
    {
        XMsgBox::ErrorMsgBox(L"�¼�����������ʧ�ܣ�");
        return FALSE;
    }
    
#ifdef _DEBUG
    if (!m_frame.Create(NULL,WS_POPUP|WS_VISIBLE))
#else   // release ���
    if (!m_frame.Create(NULL,WS_POPUP|WS_VISIBLE, WS_EX_TOPMOST))
#endif
    {
        return FALSE;
    }

    // �ָ��ϴ��˳�ʱ����λ��
    RestoreWindowPos();

    // ������
    _theInst.SetInstanceMainWnd(m_frame.GetSafeHwnd());
    
    // ����ͨ���¼�
    m_hEventMainWnd = CreateEvent(NULL, TRUE, TRUE,
        LITTLET_MAINWND_STARTUP_EVENTNAME);
    if (NULL == m_hEventMainWnd)
    {
        XMsgBox::ErrorMsgBox(L"�������������ǲ����������д����Զ�����");
    }
    SetEvent(m_hEventMainWnd);
#ifdef _DEBUG
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        htmlayout::debug_output_console()
            .printf("--->LittleTFrame:the event is exist!\n");
    }
#endif

//#ifndef _DEBUG
    // ÿ��������ʱ�򶼼���Ƿ��и���
    CheckUpdate();
//#endif

    return TRUE;
}

void LittleTApp::CheckUpdaterExe()
{
    // �����³����Ƿ����°汾
    CStdString sDir = quibase::GetModulePath();
    CStdString sNewUpdater = sDir + LITTLET_UPDATER_NEWNAME;
    if ( quibase::IsFileExist( sNewUpdater ) )
    {
        // �����Լ�������Ȩ�ޣ���Ӧ��vista������ϵͳ��UAC����
        CUACSElfElevations::SelfElevation();

        // �滻���µĽ���
        MoveFileEx(sNewUpdater, sDir + L"LittleTUpdater.exe", 
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
    }
}

BOOL LittleTApp::CheckUpdate()
{
//     QBuffer* pBuf = NULL;
//     if (!QUIMgr::GetInstance()->LoadData(L"qabs:url.x", (QView*)nullptr, pBuf))
//     {
//         XMsgBox::ErrorMsgBox(L"�汾̫���� -_-!");
//     }
// 
//     if (NULL != pBuf)
//     {
//         // ȥ��ǰ3���ֽڵ��ļ����룬ʣ�µľ����ļ�������
//         DWORD dw;
//         pBuf->Read((PBYTE)&dw, 3);
//         CStdString sLines = ATL::CA2WEX<256>((char*)pBuf->GetBuffer());
//         CStdString sUrl = sLines, sRefer;
//         int idx = sLines.Find(L';');
//         if (-1 != idx)
//         {
//             sUrl = sLines.Left(idx);
//             sRefer = sLines.Mid(idx+1);
//         }
//         // �����Զ����¼��
//         _Url url_updation;
//         url_updation.url_ = sUrl;
//         url_updation.domain_ = sRefer;
//         // ÿ5��Сʱ���һ�θ���
//        return QAutoUpdater::GetInstance()->Startup(url_updation, _Url(), 4*60);
// //        return QAutoUpdater::GetInstance()->Startup();
//      }
//    return FALSE;

    // �����Զ����¼��
    _Url url_updation;
    url_updation.url_ = L"http://www.jiubaibu.com/appcast/littlet/littlet.asp";
    url_updation.domain_ = "http://www.jiubaibu.com";
    // ÿ2��Сʱ���һ�θ���
    return QAutoUpdater::GetInstance()->Startup(url_updation, _Url(), 2 * 60);
}

LittleTApp::~LittleTApp()
{
    // ���̼����˳���
    if (NULL != m_hEventMainWnd)
    {
        CloseHandle(m_hEventMainWnd);
        m_hEventMainWnd = NULL;
    }

    if (m_bStartUpdater)
    {
        StartUpdateExe();
    }

    CGDIpInitializer::Get()->Deinitialize();
}

LittleTApp::LittleTApp()
{
    m_bStartUpdater = FALSE;
    m_hEventMainWnd = NULL;
}

void LittleTApp::StartUpdateExe()
{
    // �ҵ����³���
    CStdString sCurDir = quibase::GetModulePath();
    sCurDir.Replace(L'/',L'\\');
    CStdString sUpdateExe = sCurDir + L"LittleTUpdater.exe";
    if (!quibase::IsFileExist(sUpdateExe))
    {
        ::MessageBox(NULL, L"���³��򲻼��ˣ������������������� -_-!", L"����", MB_OK);
        // ��Ҫʹ��������䣬����ɱ���
        // XMsgBox::ErrorMsgBox(L"���³��򲻼��ˣ�������ɶ�ˣ������������������� -_-!");
        return;
    }

    // �����¼����ø��½��̵ȴ�
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, LITTLET_UPDATER_EVENTNAME);

    // �����ӽ���
    CStdString sCmd = m_sUpdateFile;
    sCmd = L"\"" + sCmd + L"\"";    // ����ո�
    sCmd += L" ";   // һ���ո�ָ�����
    sCmd += L"\"" + sCurDir + L"\\\"";  // �����������б��
    ShellExecute(NULL, L"open", sUpdateExe, sCmd, 
        sCurDir + L"\\", SW_SHOWDEFAULT );

    // ok��
    SetEvent(hEvent);
    CloseHandle(hEvent);
}

CStdString LittleTApp::GetAppVersion() const
{
    // ������ʷ��<br/>
    // LittleT v5.1 2015 / 7 / 8 < br / >
    // ----------------------<br / >
    // ��ǿ����ǩ����Tab������<br / >
    // ��ǿ����ǩ����Ctrl + [Shift] + Tab���б�ǩ����<br / >
    // <br / >
    // LittleT v5.0 2015 / 6 / 11 < br / >
    // ----------------------<br / >
    // ��ǿ���������ͼƬ<br/>
    // ��������ӱ�ǩ���� <br / >
    // <br / >
    // LittleT v4.0 2015 / 3 / 18 < br / >
    // ----------------------<br / >
    // ��ǿ�������ʾ��Ϣ�Ƿ�ֻ��һ�Σ����������Ҫ�ֶ��ر���ʾ����������Ҫ����ʾ��
    // �޸ģ����ѽ���������ƣ������һ�㣨������ѿ��ˣ�������<br / >
    // �޸����ƻ����������б���ʾ������������ <br / >
    // <br / >
    // LittleT v3.8 2014 / 12 / 6 < br / >
    // ----------------------<br / >
    // ��������ӵ���������������<br / >
    // �޸ģ����ѽ���������ƣ������һ�㣨������ѿ��ˣ�������<br / >
    // �޸ģ�������ʾ�������¼�������´�ѡ��
    // �޸ģ��ദ����΢����<br / >
    // <br / >
    // LittleT v3.6 2014/7/6<br/>
    // ----------------------<br/>
    // �������������<br/>
    // �޸ģ�����΢��<br/>
    // �������������û���ͳ�ƵĴ���<br/>
    // <br/>
    // LittleT v3.4 2014/5/21<br/>
    // ----------------------<br/>
    // �������Զ�����ÿ���ĳ�족ִ�й���<br/>
    // �޸ģ��޸�Ϊÿ5��Сʱ�Զ�������<br/>
    // �������ҵ�΢�����ӵ�ַ<br/>
    // 
    // LittleT v3.2 2013/10/28<br/>
    // ----------------------<br/>
    // �޸����Զ����������������ʾʱ�䲻��ȷ<br/>
    // �޸�����ʾ����������/�����޷����������<br/>
    // ����������������ѡ��<br/>
    // ��ǿ���ƻ��Ľ׶ο���ʹ��ҳ�ŵ����ˣ�ͬʱ�޸�N����������<br/>
    // ��ǿ��Ŀǰctrl+shift+h��ʾ���ڿ�ݼ�һ����������壬����֮ǰ���Ķ���ʾ<br/>
    // ��ǿ��������ʾ����ʹ��html���롣�� <b .red>����</b> �ᱻ�Ժ�ɫ������ʾ<br/>
    // <br/>
    // LittleT v3.1 2013/10/15<br/>
    // ----------------------<br/>
    // �޸����Զ��������һ��ִ�У������޷���ȷ��ʾ�´ν�Ҫִ�е�����<br/>
    // �޸��������ϵ�һЩ�����޸�<br/>
    // �޸����Զ�����༭�������ִ�У�����һ���ڴ�й¶<br/>
    // �޸ģ������ؼ����������<br/>
    // �޸ģ�ȥ�����������/��ʾ�Ķ���Ч������Ϊxp����ʾ�п��٣�<br/>
    // ��л������Ooo�������� ����bug<br/>
    // <br/>
    // LittleT v3.0 2013/6/18<br/>
    // ----------------------<br/>
    // �޸ģ����ƻ��������������<br/>
    // �޸����ػ������ܱ�ȡ��bug<br/>
    // <br/>
    // LittleT v2.9 2013/6/18<br/>
    // ----------------------<br/>
    // <b .red>˵����������˼��V2.8���µ�update���������������������ʾ����ֱ�Ӱ��»س�������������Զ�������</b><br/>
    // ��ǿ����ס�ϴ��˳�ʱ����λ�ã��´�������ָ�λ��<br/>
    // �޸ģ���Ĭ���£�������ʾ������ʾ<br/>
    // �޸���ͨ������������ò����������ļ���ȡʧ�ܡ�����<br/>
    // <br/>
    // LittleT v2.8 2013/6/8<br/>
    // ----------------------<br/>
    // ��ӣ��������öԻ���<br/>
    // �޸ģ��Զ������ʱ����ֶ���д��<br/>
    // ��ǿ����С����ϵͳ����ʱ��ʾ������ʾ<br/>
    // <br/>
    // LittleT v2.7 2013/6/3<br/>
    // ----------------------<br/>
    // �޸���<b .red>ʵ�ڱ�Ǹ����ʱ��̫�٣�����ά��û����λ��
    // ���±������v2.6�汾���ܴ����Զ����񡣴˰汾��Ϊ�޸���һ���ͼ�����
    // ������ʹ�ù����з���bug���뼰ʱ�ύ���ҡ��м�������</b><br/>
    // <br/>
    // LittleT v2.6 2013/6/1<br/>
    // ----------------------<br/>
    // ��ǿ��ESC���˳�ȫ������<br/>
    // �޸ģ���С��header������<br/>
    // �޸���������ʱ��ִ�С�->��ÿ�ܵ�ĳ�족�������󡣸�л�����ύbug<br/>
    // �޸���ȫ�ֿ�ݼ��½�todo���񣬲��������������⣬��лԬ���ύbug<br/>
    // <br/>
    // LittleT v2.5 2013/5/14<br/>
    // ----------------------<br/>
    // ��ǿ��"�Զ�����-ִ�г���" �����ֶ�������ַ��<br/>
    // ��ǿ���Զ�������ʾ�Ի�����ʾ��ϸ��������<br/>
    // �޸ģ����ɽ���ϸ�ڵ���<br/>
    // �޸ģ�ȥ��������ͼ��<br/>
    // �޸ģ�������ͼ��ֻ���ڳ�����ȫ���ص�ʱ��Ż���ʾ<br/>
    // �޸����Զ����³�����ɾ�������ļ�����<br/>
    // �޸���"�Զ�����->����ʱ��->��������"ִ�����ڽ�������<br/>
    // <br/>
    // LittleT v2.4	2013/5/10<br/>
    // ----------------------<br/>
    // ���ӣ�ϵͳ������ͼ��<br/>
    // ���ӣ������з�ʽ�����Զ�����<br/>
    // ���ӣ������Զ�����<br/>
    // �޸����Զ����³���ȱ��DLL����<br/>
    // �޸ģ�TODO�Ƴ���������ɡ�����<br/>
    // �޸ģ�����ϸ΢����<br/>
    // <br/>
    // LittleT v2.2	2013/4/25<br/>
    // ----------------------<br/>
    // �޸ģ����ƻ����в�����ȷѡ��stage��Ŀ<br/>
    // ��ǿ��������release������Դ���<br/>
    // <br/>
    // LittleT v2.1	2013/4/22<br/>
    // ----------------------<br/>
    // �޸ģ��Զ����¹��ܵ���Ϊÿ30���Ӽ��һ��<br/>
    // �޸ģ�����ϸ�ڵ�����׼������<br/>
    // �޸ģ��Զ����¹��ܵ���Ϊÿ30���Ӽ��һ��<br/>
    // �޸ģ�����ϸ�ڵ�����׼������<br/>
    // <br/>
    // LittleT v2.0	2013/4/18<br/>
    // ----------------------<br/>
    // ���ӣ����ƻ�������<br/>
    // ���ӣ�TODO������ɵ���ʱ<br/>
    // ���ӣ������Զ����¹���<br/>
    // �޸ģ������������<br/>
    // �޸������ɸ�Сbug<br/>
    // <br/>
    // LittleT v1.2	2013/3/18<br/>
    // ----------------------<br/>
    // ��ǿ��������ش����ع��������ٶȱ�����<br/>
    // �޸������������ͼ�������Ӧ��Ϣ<br/>
    // �޸���������ctrl��Ȼ��ʹ�ÿ�ݼ�������<br/>
    // �޸����ػ�����ʵ��Ϊ������bug<br/>
    // �޸ģ������Զ�����ʱ������ʾ<br/>
    // <br/>
    // LittleT v1.1	2013/3/1<br/>
    // ----------------------<br/>
    // ����������ʱ�������ʱ���ִ��<br/>
    // �޸ģ���������ˡ��½��Զ����񡱵Ľ��棬����Ӧ�ø�����Ư��Щ��<br/>
    // <br/>
    // LittleT v1.0	2013/2/16<br/>
    // ----------------------<br/>
    // ��һ���汾

    return FileVersion().GetMajorMinorVersion();
}

//////////////////////////////////////////////////////////////////////////
LittleTConfig _Config;

std::vector<CStdString> LittleTConfig::GetHistorySoundFile()
{
    CStdString s_files = GetValue(L"Sound", L"file");
    std::vector<CStdString> ret;
    QStrEx(L'*', s_files).GetAll(ret);
    return ret;
}

std::vector<CStdString> LittleTConfig::AddSoundFilePath(const CStdString& file)
{
    auto files = GetHistorySoundFile();
    QStrEx se(L'*', L"");
    for (auto &s : files)
    {
        se.AddString(s);
    }
    se.AddString(file);
    se.Unique();
    SetValue(L"Sound", L"file", se.Commbine());

    return GetHistorySoundFile();
}
