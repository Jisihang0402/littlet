#include "LittleTFrame.h"

//#include "feedback/Feedback.h"
#include "AppHelper.h"
#include "../common/LittleTUIcmn.h"
#include "../common/Worker.h"
#include "../common/ProcessMan.h"

#include "auto_update.h"
#include "QEventRemindDlg.h"
#include "QUtilWnd.h"
#include "LittleT.h"
#include "LAboutDlg.h"

struct _APPMODE_2_TABIDX
{
    ENUM_APP_MODE  eMode;
    int             iTab;
};
const _APPMODE_2_TABIDX G_MODE2TABIDX[] = 
{
    {APP_MODE_PLAN, 0},
    {APP_MODE_EVENTS, 1},
    {APP_MODE_TODO, 2},
//    {APP_MODE_EVENTS, 2},
};

namespace littlet
{
    int AppMode2Index(ENUM_APP_MODE eMode)
    {
        for(int i = 0; i < ARRAYSIZE(G_MODE2TABIDX); ++i)
        {
            if (G_MODE2TABIDX[i].eMode == eMode)
            {
                return G_MODE2TABIDX[i].iTab;
            }
        }
        ASSERT(FALSE);
        return -1;
    }

    ENUM_APP_MODE Index2AppMode(int iTabIdx)
    {
        for(int i = 0; i < ARRAYSIZE(G_MODE2TABIDX); ++i)
        {
            if (G_MODE2TABIDX[i].iTab == iTabIdx)
            {
                return G_MODE2TABIDX[i].eMode;
            }
        }
        ASSERT(FALSE);
        return APP_MODE_INVALID;
    }

}

LPCWSTR G_ATOMSTR = L"QQ474999196";

QUI_BEGIN_EVENT_MAP(LittleTFrame,QFrame)
    ELEMENT_EXPANDED_NAME(L"panel_item", &LittleTFrame::OnTabSwitched)
    BN_CLICKED_ID(L"a-about", &LittleTFrame::OnClkAbout)
    BN_CLICKED_ID(L"a-setting", &LittleTFrame::OnClkSetting)
    BN_CLICKED_ID(L"a-weibo", &LittleTFrame::OnClkMyWeibo)
    BN_CLICKED_ID(L"btn_feedback", &LittleTFrame::OnClkFeedback)
    BN_CLICKED_ID(L"btn_check_update", &LittleTFrame::OnClkCheckUpdation)
QUI_END_EVENT_MAP()

LittleTFrame::LittleTFrame(void)
    :QFrame(L"qabs:main/index.htm")
{
    m_bStartup = FALSE;
    m_bIsCloseForUpdate = FALSE;
}

LittleTFrame::~LittleTFrame(void)
{
}

void LittleTFrame::OnClose()
{
    if (    m_bIsCloseForUpdate 
        || (XMsgBox::YesNoMsgBox(L"ȷ���˳�����") == IDYES) )
    {
        SetMsgHandled(FALSE);

        ConfigHotKey(FALSE);

        m_trayicon.RemoveIcon();

//        LDesktopTodo::GetInstance()->PostMessage(WM_CLOSE);

        // ��¼����λ��
        QUIGetApp()->SaveWindowPos();
        // �˳�
        PostQuitMessage(0);
    }

    LittleTApp* pApp = (LittleTApp*)QUIGetApp();
    pApp->m_bStartUpdater = m_bIsCloseForUpdate;
    pApp->m_sUpdateFile = m_sUpdateFile;
}

QView* LittleTFrame::CustomControlCreate( HELEMENT he )
{
    CStdString sID = ECtrl(he).ID();
    if (sID.CompareNoCase(L"WND-PLAN") == 0)
        return &m_viewPlan;
    else if (sID.CompareNoCase(L"WND-EVENT") == 0)
        return &m_viewEvent;
    else if (sID.CompareNoCase(L"WND-todo") == 0)
        return &m_viewTodo;
    return NULL;
}

LRESULT LittleTFrame::OnDocumentComplete()
{
    // ok, �Ѿ������ˣ��������̿��Թ���ͨ����
    // �Զ���������
    wnd_autohide_.RegisterAutoHide(GetSafeHwnd(), CAutoHideWnd::ALIGN_ALL, 10, 1);

    // ��ʼ���汾
    SetTitle(CStdString(L"LittleT v") + QUIGetAppVersion());  

    QTime tmStartup = QProcessMan::GetSystemStartupTime();
    GetCtrl("#bar_status").SetText(tmStartup.Format(L"ϵͳ�����ڣ�%X"));

    ConfigHotKey(TRUE); 
    
    littlet::EnterAppMode(APP_MODE_EVENTS);

    // ˢ��ʣ��δ��ɵ�todo����
    OnTodoTaskStatusChanged(0);

    m_bStartup = TRUE;

    RefreshEventNum();

    RefreshHeader();

//    LDesktopTodo::GetInstance()->Create(240);

    return 0;
}

void LittleTFrame::OnTodoTaskStatusChanged( LPARAM lParam )
{
    GetCtrl("#TODO_NUM_QUOTER").SetText(aux::itow(
        QDBEvents::GetInstance()->TodoTask_GetUnfinishNum()));
}

void LittleTFrame::OnEventStatusChanged( LPARAM lParam )
{
    RefreshEventNum();

    // ˢ�������Ҫִ�е�����
    RefreshHeader();
}

void LittleTFrame::RefreshHeader()
{
    if ( !m_bStartup )
        return;

    QAutoTask *pTask = QAutoTaskMan::GetInstance()->GetMostCloseExecute();
    if (NULL == pTask)
        return;

    CStdString sImgSrc;
    switch (pTask->GetDoWhat())
    {
    case AUTOTASK_DO_REMIND:
        {
            sImgSrc = L"qabs:icons/clock_75.png";
            break;
        }
    case AUTOTASK_DO_EXECPROG:
        {
            sImgSrc = L"qabs:icons/cmd_75.png";
            break;
        }
    case AUTOTASK_DO_SYSSHUTDOWN:
        {
            sImgSrc = L"qabs:icons/shutdown_75.png";
            break;
        }
    case AUTOTASK_DO_BREAKAMOMENT:
        {
            sImgSrc = L"qabs:icons/sleep_75.png";
            break;
        }
    }
    GetCtrl("#IMG_EVENT").set_attribute("src",sImgSrc);

    QTime tmNext;
    pTask->GetNextExecTime(tmNext);
    GetCtrl("#LATEST-EVENT #td_time").SetText(tmNext.Format(L"%H:%M"));
    GetCtrl("#LATEST-EVENT #td_date").SetHtml(
            tmNext.Format(L"<b style=\"font-size:14pt;\">:%S</b>") 
            + L"<br />" + tmNext.Format(L"%Y/%m/%d"));
    GetCtrl("#LATEST-EVENT #td_event").SetText(pTask->GetDoWhatString());
}

void LittleTFrame::RefreshEventNum()
{
    CStdString sNum;
    sNum.Format(L"%d",QAutoTaskMan::GetInstance()->GetTaskCount());
    GetCtrl("#EVENT_NUM_QUOTER").SetText(sNum);
}

void LittleTFrame::OnEventPause( LPARAM lParam )
{
    RefreshHeader();
}

void LittleTFrame::OnEventStart( LPARAM lParam )
{
    RefreshHeader();
}

void LittleTFrame::OnEventEdit( LPARAM lParam )
{
    RefreshHeader();

    LReminderBox::GetInstance()->RemoveReminderDlg((int)lParam);
}

void LittleTFrame::OnEventOverdue( LPARAM lParam )
{
    RefreshHeader();

    LReminderBox::GetInstance()->RemoveReminderDlg((int)lParam);
}

void LittleTFrame::OnEventToggleReminder( LPARAM lParam )
{
    QAutoTask *pTask = QAutoTaskMan::GetInstance()->GetTask((int)lParam);
    if ((NULL != pTask) && !(pTask->IsReminderEnabled()))
    {   // ������ʾ��
        LReminderBox::GetInstance()->RemoveReminderDlg(pTask->ID());
    }
}

void LittleTFrame::OnCmdShowEventReminder( LPARAM lp )
{
    LPTASK_REMINDER_PARAM pRP = reinterpret_cast<LPTASK_REMINDER_PARAM>(lp);

    LReminderBox::GetInstance()->ShowReminderDlg(pRP);
}

namespace littlet
{
    class abort_shutdown : public htmlayout::gui_task
    {
    public:
        virtual void exec()
        {
            if (IDYES == XMsgBox::YesNoMsgBox(L"�����ػ����Ƿ�ȡ����",
                L"�ػ�ȷ�ϣ�", 30, XMsgBox::ID_MSGBOX_BTN_NO))
            {
                quibase::PreventSystemShutdown();
            }
        }
    };
}

void LittleTFrame::OnAutoTaskFired( LPARAM lp )
{
    QAutoTask* pTask = reinterpret_cast<QAutoTask*>(lp);
    if (NULL == pTask)
    {
        ASSERT(FALSE);
        return ;
    }

    switch (pTask->GetDoWhat())
    {
    case AUTOTASK_DO_REMIND:// = 2, // ��ʾ��Ϣ
        {
            break;
        }
    case AUTOTASK_DO_SYSSHUTDOWN:// = 4,  // �ػ�
        {
            wchar_t szMsg[128] = {0};
            WTL::SecureHelper::strcpy_x(szMsg,128, L"LittleT����ػ�");
            quibase::ShutdownComputer(FALSE, szMsg, 30);

            htmlayout::queue_hl::push(new littlet::abort_shutdown, GetSafeHwnd());

            break;
        }
//     case AUTOTASK_DO_SYSREBOOT: // = 5,  // ����
//         {
//             break;
//         }
    case AUTOTASK_DO_BREAKAMOMENT: // = 5,  // ��Ϣһ���
        {
            LittleTConfig *pcfg = (LittleTConfig*)QUIGetConfig();
            QScreenWnd::Show(pcfg->GetPicFolder(),pcfg->GetPicAlpha(),
                pcfg->GetPicBkcolor(),pcfg->GetBreakSec(),
                pcfg->GetPicSec());
            break;
        }
    default:break;
    }
    // ��view Event �����¼� 
    m_viewEvent.OnAutoTaskFired(pTask);
    // ˢ�����´�ִ��ʱ��
    RefreshHeader();
}

BOOL LittleTFrame::ConfigHotKey(BOOL bConfig)
{
    HWND hWnd = GetSafeHwnd();
    if (bConfig)
    {
        ASSERT(0 == m_idAtom);

        m_idAtom = GlobalAddAtom(G_ATOMSTR);
        if ( 0 == m_idAtom)
        {
            ASSERT(FALSE);
            return FALSE;
        }   

        // ���ؿ�ݼ�
//         RegisterHotKey(hWnd, m_idAtom, MOD_CONTROL, 0x4E);  // ctrl + n
//         RegisterHotKey(hWnd, m_idAtom, MOD_CONTROL, 0x4E);  // ctrl + tab

        // ȫ�ֿ�ݼ�
        RegisterHotKey(hWnd, m_idAtom, MOD_CONTROL | MOD_SHIFT, 0x48);
        RegisterHotKey(hWnd, m_idAtom, MOD_CONTROL | MOD_SHIFT, 0x4A);  // j
        RegisterHotKey(hWnd, m_idAtom, MOD_CONTROL | MOD_SHIFT, 0x4B);  // k
        RegisterHotKey(hWnd, m_idAtom, MOD_CONTROL | MOD_SHIFT, 0x4C);  // l
    }
    else
    {
        ASSERT( 0 != m_idAtom);
        UnregisterHotKey(hWnd, m_idAtom);
        GlobalDeleteAtom(m_idAtom);
        m_idAtom = 0;
    }

    return TRUE;
}

void LittleTFrame::OnHotKey( int nSystemHotKey,UINT nModifier,UINT nVk )
{
    if (!_HasFlag(nModifier,MOD_CONTROL) || !_HasFlag(nModifier,MOD_SHIFT))
        return; // ctrl + shift

    // ȫ�ֿ�ݼ�
    if (0x48 == nVk)
    {   // ctrl + shift + h
        // system tray icon

        static BOOL bTrayiconCreated = FALSE;
        if ( !bTrayiconCreated )
        {  // ֻ���ڴ�����ȫ���ص�ʱ�����ʾ������ͼ��
            HICON hSystrayIcon = LoadIcon(QUIGetInstance(), MAKEINTRESOURCE(IDI_LITTLET));
            m_trayicon.Create(QUIGetInstance(), GetSafeHwnd(), QSOFT_LITTLET_SYSTRAYMSG,
                L"LittleT", hSystrayIcon, IDR_MENU_TRAYICON, FALSE, L"����ɻָ�������ʾ",
                L"LittleT�ѱ���С�������");
            bTrayiconCreated = TRUE;
        }

        if (_HasFlag(GetStyle(),WS_VISIBLE))    // ������ʾ��
        {
            // ����֮
//            m_trayicon.MinimizeToTray(GetSafeHwnd());
            ShowWindow(SW_HIDE);
            m_trayicon.ShowIcon();
        }
        else
        {
//            m_trayicon.MaximiseFromTray(GetSafeHwnd());
            CRect rc;
            GetWindowRect(&rc);
            if (rc.left < 0)
                rc.OffsetRect(-rc.left, 0);
            if (rc.top < 0)
                rc.OffsetRect(0, -rc.top);
            CSize szScrn = quibase::GetScreenSize();
            if (rc.right > szScrn.cx)
                rc.OffsetRect(szScrn.cx - rc.right, 0);

            SetWindowPos(NULL, &rc, SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOSIZE);
            SendMessage(WM_SHOWWINDOW, (WPARAM)TRUE, 0);
//             if (!(GetExStyle() & WS_EX_TOPMOST))
//             {
//                 SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
//                 SetWindowPos(HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
//             }

            m_trayicon.HideIcon();
        }
        return;
    }

    switch (nVk)
    {
    case 0x4A:      // j
        {
            // ���Զ�����
            littlet::EnterAppMode(APP_MODE_EVENTS);
            m_viewEvent.OnClkNewEvent(NULL);

            break;
        }
    case 0x4B:  // k
        {
            // ��todo����
            // ת��todo����ģʽ
            littlet::EnterAppMode(APP_MODE_TODO);
            m_viewTodo.OnClkNewTask(NULL);

            break;
        }
    case 0x4C:  // l
        {
            // �ƻ�ģʽ
            littlet::EnterAppMode(APP_MODE_PLAN);
            m_viewPlan.OnClkNewPlan(NULL);

            break;
        }
    }
}

void LittleTFrame::OnEventDelete( LPARAM lParam )
{
    LReminderBox::GetInstance()->RemoveReminderDlg((int)lParam);

    RefreshHeader();

    RefreshEventNum();
}

void LittleTFrame::OnKeyDown( UINT nChar,UINT nRepCnt,UINT nFlags )
{
    if (GetKeyState(VK_CONTROL) < 0)
    {
        SetMsgHandled(littlet::OnCtrlKeyPressDown(nChar,nRepCnt,nFlags));
        return;
    }
    SetMsgHandled(FALSE);
}

void LittleTFrame::OnEnterAppMode( LPARAM lParam )
{
    ENUM_APP_MODE eMode = (ENUM_APP_MODE)lParam;
    if (APP_MODE_INVALID != eMode)
    {
        int idx = littlet::AppMode2Index(eMode);
        if (-1 != idx)
        {
            _Tabs().SelectTab(idx);
        }
    }

//     switch (lParam)
//     {
//     case APP_MODE_EVENTS:
//         {
//             break;
//         }
//     case APP_MODE_TODO:
//         {
//             _Tabs().SelectTab(1);
//             break;
//         }
//     case APP_MODE_DING:
//         {
//             //_Tabs().SelectTab(1);
//             break;
//         }
//     }
}

void LittleTFrame::OnTabSwitched( HELEMENT he )
{
    int idx = /*ECtrl(he).index(); //*/_Tabs().GetCurrentTab().index();
    ENUM_APP_MODE eMode = littlet::Index2AppMode(idx);
    ASSERT(APP_MODE_INVALID != eMode);
    if (APP_MODE_INVALID != eMode)
    {
        littlet::EnterAppMode(eMode);
    }
}

void LittleTFrame::OnPlanNumChanged( LPARAM lParam )
{
    CStdString sNum;
    sNum.Format(L"%d",lParam);
    GetCtrl("#PLAN_NUM_QUOTER").SetText(sNum);
}

LRESULT LittleTFrame::OnAppVersionCheck( UINT uMsg, 
    WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    if (0 == wParam)
        return 0;

    LPQUI_USERMSGPARAM p = reinterpret_cast<LPQUI_USERMSGPARAM>(lParam);
    if (NULL == p)
    {
        ASSERT(FALSE);
        return 0;
    }
    if ( !quibase::IsFileExist(p->sParam))
    {
        ASSERT(FALSE);
    }
    else
    {
        // �����Ի���ѯ���û��Ƿ�������
        // 2013/6/18 v2.9 ����ѯ�ʣ���Ĭ����
//        if (LUpdateInfoDlg(p->sParam).DoModal() == IDOK)
//         if (XMsgBox::YesNoMsgBox(L"Ӧ�ó�������Ѿ����أ��Ƿ���£�<br/>"
//                 L"���İɣ����ᳬ��10���ӣ�") == IDYES)
        {
            // �������³���
            m_bIsCloseForUpdate = TRUE;
            m_sUpdateFile = p->sParam;
            PostMessage(WM_CLOSE);
        }
    }
    
    // �ͷŲ����ڴ�
    if (p->ShouldIDeleteThis())
    {
        QUIReleaseUserMsgParam(p);
    }
    
    return 0;
}

void LittleTFrame::OnClkAbout( HELEMENT )
{
    LAboutDlg::ShowModal();
}

// LRESULT LittleTFrame::HandleAutohideMessage( UINT uMsg, WPARAM wParam, 
//     LPARAM lParam, BOOL& bHandled )
// {
//     CAutoHideWnd::GetInstance()->AutoHideWndProc(
//         GetSafeHwnd(), uMsg, wParam, lParam);
// 
//     bHandled = FALSE;
// 
//     return 0;
// }

LRESULT LittleTFrame::OnComputerPowerChanged( DWORD dwEvent, DWORD )
{
    if (   (PBT_APMRESUMEAUTOMATIC == dwEvent)  // windows vista or higher
        || (PBT_APMRESUMECRITICAL == dwEvent) ) // windows xp
    {
        // ˢ���Զ�����
        m_viewEvent.RefreshEventList();
    }
    return TRUE;
}

LRESULT LittleTFrame::OnSysTrayMessage( UINT uMsg, WPARAM wParam, 
    LPARAM lParam, BOOL& bHandled )
{
    if (LOWORD(lParam) == WM_LBUTTONUP)
    {
        // ��ʾ����
        // ShowWindow(SW_RESTORE);

        // �����ͣ�������ŵģ���ʾһ��
        CRect rc;
        if (GetWindowRect(&rc))
        {
            if (rc.left < 0)
                rc.OffsetRect(-rc.left + 1, 0);
            else if (rc.top < 0)
                rc.OffsetRect(0, -rc.top + 1);
            else
            {
                CSize sz = quibase::GetScreenSize();
                if (rc.right > sz.cx)
                    rc.OffsetRect(sz.cx - rc.right, 0);
            }
        }
        MoveWindow(&rc);

        // ����trayicon
        m_trayicon.HideIcon();
    }
    return m_trayicon.OnTrayNotification(wParam, lParam);
}

void LittleTFrame::OnCommand( UINT nType, int nID ,HWND hWnd )
{
    switch (nID)
    {
    case IDM_EXIT:
        {
            PostMessage(WM_CLOSE);
            break;
        }
    case IDM_ABOUT:
        {
            LAboutDlg::ShowModal();
            break;
        }
    default:
        SetMsgHandled(FALSE);
        break;
    }
}

LRESULT LittleTFrame::OnCopyData( HWND hWnd, PCOPYDATASTRUCT pps )
{
    if (NULL == pps)
    {
        ASSERT(FALSE);
        return TRUE;
    }

    if (LITTLET_SHELL_COMMANDCODE_ADDAUTOTASK == pps->dwData)
    { // ����Զ�����
        LP_LittleTShell_CopyData_Param p = 
            reinterpret_cast<LP_LittleTShell_CopyData_Param>(pps->lpData);
        if (NULL == p)
        {
            ASSERT(FALSE);
            return TRUE;
        }

        CStdString sError;
        QAutoTask *pTask = QAutoTaskMan::GetInstance()->NewAutoTask(p->eDoWhat, 
            p->szDoWhatParam,p->szWhenDo, p->szRemindExp,
            p->tmBegin, p->tmEnd, sError);
        if (NULL == pTask)
        {
            WTL::SecureHelper::strcpy_x(p->szResult, 256, sError);
        }
        else
        {
            WTL::SecureHelper::strcpy_x(p->szResult, 256, L"�Զ������Ѿ�����");
            m_viewEvent.NewEventAdd(pTask);
        }
    }

//     QString sWhenDo = p->szWhenDo;
//     QString sDoWhatParam = p->szDoWhatParam;
//     QString sRemindExp = p->szRemindExp;
//     ENUM_AUTOTASK_DOWHAT eDoWhat = p->eDoWhat;
//     QTime tmBegin = p->tmBegin;
//     QTime tmEnd = p->tmEnd;
// 
//     QString sTmp;
//     sTmp.Format(L"When:%s<br/>What:%d %s<br/>REmind:%s<br/>Life:%s ~ %s<br/>",
//                 sWhenDo, eDoWhat, sDoWhatParam, sRemindExp,
//                 tmBegin.Format(L"%c"), tmEnd.Format(L"%c"));
//     XMsgBox::OkMsgBox(sTmp);
    return TRUE;
}

void LittleTFrame::OnSize( UINT nType, WTL::CSize sz )
{
    if (SIZE_MINIMIZED == nType)
    {
        ShowWindow(SW_RESTORE);
        OnHotKey(0, MOD_SHIFT | MOD_CONTROL, 0x48);
    }
    SetMsgHandled(FALSE);
}

void LittleTFrame::OnClkSetting( HELEMENT )
{
    LSettingDlg::ShowModal();
}

void LittleTFrame::OnClkMyWeibo(HELEMENT)
{
    quibase::VisitWebsiteWithDefaultBrowser(L"http://weibo.com/qiuchengw");    
}

void LittleTFrame::OnClkFeedback( HELEMENT )
{
//    QUserFeedbackWnd::Show();
}

void LittleTFrame::OnClkCheckUpdation( HELEMENT )
{
    if ( QAutoUpdater::GetInstance()->CheckUpdate())
    {
        XMsgBox::OkMsgBox(L"<b .red>���ڼ�顭��</b><br/><br/>����æ������^_^", L"������", 6);
    }
}
