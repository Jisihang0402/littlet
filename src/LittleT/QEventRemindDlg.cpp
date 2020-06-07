#include "QEventRemindDlg.h"
#include "../common/QAutoTask.h"
#include "../common/LittleTUIcmn.h"
#include "../common/ConstValues.h"
#include "../common/QTimer.h"
#include <Mmsystem.h>
#include "LDatas.h"
#include "LViewEvent.h"

#pragma comment(lib,"Winmm.lib")

QUI_BEGIN_EVENT_MAP(QSingleRmdDlg,QDialog)
    BN_CLICKED_ID(L"btn_skip",&QSingleRmdDlg::OnClkDontExecThis)
QUI_END_EVENT_MAP()

QSingleRmdDlg::QSingleRmdDlg( LPTASK_REMINDER_PARAM pRP )
    :QDialog(L"qabs:dlgs/rmddlg.htm")
{
    m_pRP = pRP;
    m_nCountDown = 300;
}

QSingleRmdDlg::~QSingleRmdDlg( void )
{
    if (IsWindow())
    {
        SendMessage(WM_CLOSE);
        while (IsWindow())
        {
            Sleep(1);
        }
    }
}

void QSingleRmdDlg::OnClkDontRemindAgain( HELEMENT hBtn )
{

}

void QSingleRmdDlg::OnClkDontExecThis( HELEMENT hBtn )
{
    QAutoTaskMan *pMan = QAutoTaskMan::GetInstance();
    if (pMan->JumpoverTaskThisExec(m_pRP->nTaskID))
    {
        // ת��LEvent�ϵ�
        QUISendCodeTo(LDatas::GetInstance()->GetViewEventPtr()->GetSafeHwnd(),
            EVENTWND_NOTIFY_USERJUMPEVENTEXEC,(LPARAM)m_pRP->nTaskID);
        // �رյ��Լ�
        PostMessage(WM_CLOSE);
    }
    else
    {
        ECtrl(hBtn).ShowTooltip(L"�޷������˴�����");
    }
}

void QSingleRmdDlg::OnDestroy()
{
    LReminderBox::GetInstance()->RmdDlgDestroying(this);
}

void QSingleRmdDlg::SetRmdParam( LPTASK_REMINDER_PARAM pRP )
{
    m_pRP = pRP;

    if ( IsWindow() )
    {
        StartCountDown();

        RefreshRemindMessage();
    }
}

void QSingleRmdDlg::OnTimer( UINT nTimerID )
{
    if (AUTOTASK_REMINDER_COUNTDOWN_TIMERID == nTimerID)
    {
        if ( --m_nCountDown <= 0 )
        {   // ����ִ���ˣ����ԹرնԻ�����
            KillTimer(AUTOTASK_REMINDER_COUNTDOWN_TIMERID);
            PostMessage(WM_CLOSE);
        }
        else
        {
            CStdString str;
            str.Format(L" / %s",QTimeSpan((DWORD)m_nCountDown).Format(L"%H:%M:%S"));
            m_ctlCountdown.SetText(str);
        }
    }
    SetMsgHandled(FALSE);
}

LRESULT QSingleRmdDlg::OnDocumentComplete()
{
    if (NULL != m_pRP)
    {
        m_ctlCountdown = GetCtrl("#td_countdown");

        StartCountDown();

        RefreshRemindMessage(); 
    }

    CenterWindow();

    return 0;
}

void QSingleRmdDlg::StartCountDown()
{
    if (NULL == m_pRP)
    {
        ASSERT(FALSE);
        return;
    }
    // Ҳ�������öԻ����������õ����ݣ���ô��ֹ֮ͣǰ�ĵ�����ʱ��
    KillTimer(AUTOTASK_REMINDER_COUNTDOWN_TIMERID);

    // ��������������ʱ��
    m_nCountDown = m_pRP->nSeconds;

    SetTimer(AUTOTASK_REMINDER_COUNTDOWN_TIMERID, 1000, NULL);
}

void QSingleRmdDlg::RefreshRemindMessage()
{
    if (NULL == m_pRP)
    {
        ASSERT(FALSE);
        return;
    }

    QAutoTask *pTask = QAutoTaskMan::GetInstance()->GetTask(m_pRP->nTaskID);
    if (NULL == pTask)
    {
        ASSERT(FALSE);
        return;
    }

    CStdString sImg;
    switch (pTask->GetDoWhat())
    {
    case AUTOTASK_DO_SYSSHUTDOWN:
        {
            sImg = L"qrel:images/shutdown_128.png";
            break;
        }
    case AUTOTASK_DO_EXECPROG:
        {
            sImg = L"qrel:images/cmd_128.png";
            break;
        }
    case AUTOTASK_DO_BREAKAMOMENT:
        {
            sImg = L"qrel:images/sleep_128.png";
            break;
        }
    default:
        {
            sImg = L"qrel:images/clock_128.png";
            break;
        }
    }
    GetCtrl("#img_type").set_attribute("src",sImg);
    GetCtrl("#td_time").SetText(m_pRP->tmExec.Format(L"%H:%M:%S")); // :%S
    GetCtrl("#td_tip").SetHtml(m_pRP->sMsg);
    GetCtrl("#td_dowhat").SetHtml(pTask->GetDoWhatString());

    if (!(m_pRP->sSound.IsEmpty()))
    { // play sound
        ::PlaySound(m_pRP->sSound,NULL,SND_FILENAME|SND_ASYNC);
    }
}

//////////////////////////////////////////////////////////////////////////
BOOL LReminderBox::ShowReminderDlg( LPTASK_REMINDER_PARAM pRP )
{
    CheckRmdList();

    ST_RMDDLG *p = NULL;
    // ÿ��Event��ͬʱֻ����ʾһ����ʾ�Ի���
    // ���������Ƿ�������ʾ��
    BOOL bExist = FALSE;
    for ( RmdListItr i = m_lstRmd.begin(); i != m_lstRmd.end(); ++i)
    {
        p = *i;
        if ( p->GetRmdParam()->nTaskID == pRP->nTaskID )
        {   
            // ����Ի�������ʾ��
            // �����������
            p->SetClose(FALSE);
            bExist = TRUE;
            break;
         }
    }

    if ( !bExist )
    {
        p = new ST_RMDDLG();
        m_lstRmd.push_back(p);
    }

    // �����µĲ���
    p->SetRmdParam(pRP);

    // �Ի����Ƿ�������
    QSingleRmdDlg *pDlg = p->GetRmdDlg();
    if (   !pDlg->IsWindow()    // ���û�У���ô������ 
        && !pDlg->Create(NULL,WS_POPUP|WS_VISIBLE,WS_EX_TOPMOST|WS_EX_TOOLWINDOW))
    {
        return FALSE;
    }
    // �Ի����Ѿ���������ʾ֮
    pDlg->ShowWindow(SW_SHOW);

    return TRUE;
}

void LReminderBox::RemoveAll()
{
    for (RmdListItr i = m_lstRmd.begin(); i != m_lstRmd.end(); )
    {
        delete *i;
        i = m_lstRmd.erase(i);
    }
    ASSERT(m_lstRmd.size() == 0);
}

void LReminderBox::RmdDlgDestroying( QSingleRmdDlg* pDlg )
{
    for ( RmdListItr i = m_lstRmd.begin(); i != m_lstRmd.end(); ++i)
    {
        if ((*i)->GetRmdDlg() == pDlg)
        {
            (*i)->SetClose(TRUE);

            break;
        }
    }
}

void LReminderBox::CheckRmdList()
{
    for ( RmdListItr i = m_lstRmd.begin(); i != m_lstRmd.end(); )
    {
        if ((*i)->IsClosed() && !((*i)->GetRmdDlg()->IsWindow()))
        {
            delete *i;
            i = m_lstRmd.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

void LReminderBox::OnEventTimerChanged( int nEventID )
{
    ST_RMDDLG *p;
    for ( RmdListItr i = m_lstRmd.begin(); i != m_lstRmd.end(); ++i)
    {
        p = *i;
        if ( p->GetTaskID() == nEventID )
        {   
            // ����Ի�������ʾ�У��͹ر���
            QSingleRmdDlg *pDlg = p->GetRmdDlg();
            if (pDlg->IsWindow())
            {
                pDlg->PostMessage(WM_CLOSE);
            }
            break;
        }
    }
}

void LReminderBox::RemoveReminderDlg( int nTaskID )
{
    for ( RmdListItr i = m_lstRmd.begin(); i != m_lstRmd.end(); ++i)
    {
        if ((*i)->GetTaskID() == nTaskID)
        {
            delete *i;
            m_lstRmd.erase(i);
            break;
        }
    }
}

