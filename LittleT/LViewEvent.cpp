#include "LViewEvent.h"
#include "../common/LittleTUIcmn.h"
#include "../common/QAutoTask.h"
#include "../common/QDBHelper.h"

#include "LAddTask.h"
#include "LDatas.h"

QUI_BEGIN_EVENT_MAP(LViewEvent,QView)
    // auto task
    BN_CLICKED_ID(L"id_btn_newtask", &LViewEvent::OnClkNewEvent)
    BN_CLICKED_ID(L"btn_TestAutoTask", &LViewEvent::OnCalcTestEvent)
    BN_CLICKED_NAME(L"btn_del", &LViewEvent::OnClkDeleteEvent)
    BN_CLICKED_NAME(L"btn_edit", &LViewEvent::OnClkEditEvent)
    BN_CLICKED_NAME(L"td_PlayOrPauseAutoTask", &LViewEvent::OnClkPlayOrPauseEvent)
    BN_CLICKED_NAME(L"td_EnableReminder", &LViewEvent::OnClkEnableEventReminder)
    BN_CLICKED_NAME(L"item_event", &LViewEvent::OnClkEventItem)
    MOUSE_LEAVE_ID(L"EVENT_LIST",&LViewEvent::OnMouseLeaveEventList)
    DROP_NAME(L"item_event",&LViewEvent::OnDragDropDelete)
QUI_END_EVENT_MAP()

LViewEvent::LViewEvent(void)
{
    LDatas::GetInstance()->SetViewEventPtr(this);
}

LViewEvent::~LViewEvent(void)
{
}

LRESULT LViewEvent::OnDocumentComplete()
{
    RefreshEventList();

    return 0;
}

void LViewEvent::OnClkNewEvent( HELEMENT hBtn )
{
    QAutoTask* pTask = LAddEventDlg::NewEvent();
    if (NULL != pTask)
    {
        NewEventAdd(pTask);
    }
}

void LViewEvent::NewEventAdd(QAutoTask *pTask)
{
    if (NULL != pTask)
    {
        pTask->Run();

        AddEvent(pTask);

        RefreshEventNum();
    }
}

void LViewEvent::AddEvent( ECtrl& eList,QAutoTask* pTask )
{
    ECtrl eItem = ECtrl::create("table");
    eList.append(eItem);
    eItem.set_attribute("name",L"item_event");
    eItem.set_attribute("event_id",aux::itow(pTask->ID()));
    eItem.SetData(pTask);
    RefreshEventItem(eItem);
}

void LViewEvent::AddEvent( QAutoTask* pTask )
{
    AddEvent(_TaskList(),pTask);
}

void LViewEvent::OnClkDeleteEvent( HELEMENT hBtn )
{
    if (XMsgBox::YesNoMsgBox(L"ȷ��Ҫɾ��������") != IDYES)
        return ;

    ECtrl eTable = ECtrl(hBtn).select_parent(L"table",4);
    QAutoTask *pTask = reinterpret_cast<QAutoTask*>(eTable.GetData());
    if (QAutoTaskMan::GetInstance()->DeleteTask(pTask))
    {
        QUIPostCodeToMainWnd(MWND_NOTIFY_AUTOTASKDELETE,(LPARAM)pTask->ID());

        eTable.destroy();

        RefreshEventNum();
    }
}

void LViewEvent::RefreshEventList()
{
    AutoTaskList lst;
    QAutoTaskMan::GetInstance()->GetTaskList(lst);

    ECtrl eListTask = _TaskList();
    eListTask.DeleteAllChild();

    QAutoTask* pTask = NULL;
//    ENUM_AUTOTASK_RUNNING_STATUS eStatus;
    for (AutoTaskList::iterator itr = _BeginItr(lst); itr != _EndItr(lst); ++itr)
    {
        (*itr)->Run();
//        eStatus = pTask->GetLastStartStatus();
//         if (AUTOTASK_RUNNING_STATUS_OVERDUE == eStatus)
//         {
//             // ɾ����������
//             QUIPostCodeToMainWnd(MWND_NOTIFY_AUTOTASKOVERDUEDELETE,(LPARAM)pTask->ID());
//         }
//         else
//         {
            AddEvent(eListTask,*itr);
//         }
    }

    AutoSelect();
}

void LViewEvent::OnCalcTestEvent( HELEMENT hBtn )
{
    ECtrl eTaskDes = GetCtrl("#div_AutoTaskDes");
    QAutoTask*pTask = reinterpret_cast<QAutoTask*>(eTaskDes.GetData());
    if (NULL == pTask)
    {
        return;
    }

    ECtrl eTableTestExec = eTaskDes.find_first("table[testexec]");
    eTableTestExec.DeleteChild(1);
    utf8::ostream o;
    QString sTR;

    QTime tmFrom = EDate(eTaskDes.find_first("[id=\"date_TestAutoTask\"]")).GetDate();
    QTime tmTime = ETime(eTaskDes.find_first("[id=\"time_TestAutoTask\"]")).GetTime();
    tmFrom.SetTime(tmTime.GetHour(),tmTime.GetMinute(),tmTime.GetSecond());

    QString ss = tmTime.Format(L"%H:%M:%S");

    QTimer* pTimer = pTask->GetTimer();
    element eTr;
    for (int iExec = 1; iExec < 31; iExec++)
    {
        if (AUTOTASK_RUNNING_STATUS_OK != pTimer->GetNextExecTimeFrom(tmFrom,tmFrom))
            break;
        sTR.Format(L"<td>%d</td><td>%s</td>",iExec,tmFrom.Format(L"%Y/%m/%d %H:%M"));
        o<<sTR;
        eTr = element::create("tr");
        eTableTestExec.append(eTr);
        eTr.set_html(o.data(),o.length());
        o.clear();
        tmFrom += QTimeSpan(0,0,1,0);
    }
}

void LViewEvent::OnClkPlayOrPauseEvent( HELEMENT hBtn )
{
    ECtrl eBtn(hBtn);
    ECtrl eItem = eBtn.select_parent("tr",3);
    QAutoTask* pTask = reinterpret_cast<QAutoTask*>(eItem.GetData());
    if (NULL == pTask)
    {
        ASSERT(FALSE);
        return;
    }

    if (pTask->IsStartup())
    { //
        if (!pTask->Pause())
        {
            eItem.ShowTooltip(L"������ͣ�����Ժ�������");
        }
        else
        {
            QUIPostCodeToMainWnd(MWND_NOTIFY_AUTOTASKPAUSE,(LPARAM)pTask->ID());
        }
    }
    else
    {
        if (!pTask->Startup())
        {
            switch (pTask->GetLastStartStatus())
            {
            case AUTOTASK_RUNNING_STATUS_OVERDUE:
                {
                    // TODO: ���¹���������ʾ
                    QUIPostCodeToMainWnd(MWND_NOTIFY_AUTOTASKOVERDUE,(LPARAM)pTask->ID());

                    break;
                }
//             case AUTOTASK_RUNNING_STATUS_BADTIMER:
//                 {
//                     // ���������Ӧ�ó���
//                     if (XMsgBox::YesNoMsgBox(L"��ʱ����Ч���Ƿ��������ã�") == IDYES)
//                     {
//                         QExecTimeDlg ETDlg;
//                         if (ETDlg.DoModal() == IDYES)
//                         {
//                             pTask->SetTimer(ETDlg.m_tmLifeBegin,ETDlg.m_tmLifeEnd,ETDlg.m_sExp);
//                         }
//                     }
//                     break;
//                 }
            }
        }
        else
        {
            // ������
            QUIPostCodeToMainWnd(MWND_NOTIFY_AUTOTASKSTART,(LPARAM)pTask->ID());
        }
    }

    RefreshEventItem(eItem);
}

void LViewEvent::OnClkEditEvent( HELEMENT hBtn )
{
    ECtrl eItem = ECtrl(hBtn).select_parent(L"table",4);
    QAutoTask *pTask = reinterpret_cast<QAutoTask*>(eItem.GetData());
    BOOL bNeedStart = FALSE;
    if (pTask->IsStartup())
    { // �ڱ༭�����ڼ���ͣ����
        pTask->Pause();
        bNeedStart = TRUE;
        RefreshEventItem(eItem);
    }
    if (LAddEventDlg::EditEvent(pTask))
    {
        bNeedStart = TRUE;
        QUIPostCodeToMainWnd(MWND_NOTIFY_AUTOTASKEDIT,(LPARAM)pTask->ID());
    }
    if (bNeedStart)
    {
        pTask->Startup();
    }
    RefreshEventItem(eItem);
}

void LViewEvent::RefreshEventItem(ECtrl &eItem)
{
    QAutoTask* pTask = reinterpret_cast<QAutoTask*>(eItem.GetData());
    if (NULL == pTask)
    {
        ASSERT(FALSE);
        return;
    }

//     switch (pTask->GetLastStartStatus())
//     {
//     case AUTOTASK_RUNNING_STATUS_OVERDUE:
//         {
// //             eItem.destroy();
// //             // ɾ����������
// //             QUIPostCodeToMainWnd(MWND_NOTIFY_AUTOTASKOVERDUEDELETE,(LPARAM)pTask->ID());
//             return;
//         }
//     case AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC:
//     case AUTOTASK_RUNNING_STATUS_BADTIMER: { eItem.set_attribute("class",L"qtask red_back");break; }
//     case AUTOTASK_RUNNING_STATUS_APPERROR: { eItem.set_attribute("class",L"qtask purple_back"); break; }
//     case AUTOTASK_RUNNING_STATUS_UNTILNEXTSYSREBOOT: 
//     case AUTOTASK_RUNNING_STATUS_UNTILNEXTMINDERREBOOT:{ eItem.set_attribute("class",L"qtask yellow_back"); break; }
//     case AUTOTASK_RUNNING_STATUS_OK: { eItem.set_attribute("class",L"qtask silver_back");break; }
//     }

    QString sEType;
    switch (pTask->GetDoWhat())
    {
    case AUTOTASK_DO_REMIND: { sEType = L"tip"; break; }
    case AUTOTASK_DO_EXECPROG: { sEType = L"execprog"; break; }
    case AUTOTASK_DO_SYSSHUTDOWN: { sEType = L"shutdown"; break; }
    case AUTOTASK_DO_BREAKAMOMENT: { sEType = L"sleep"; break; }
    default:
        {
            ASSERT(FALSE); 
            break;
        }
    }
    
    QString sTime,sDate,sLifeEnd;
    if (pTask->IsStartup())
    {
        QTime tmExec;
        pTask->GetNextExecTime(tmExec);
        sTime = tmExec.Format(L"%H:%M:%S");
        sDate = tmExec.Format(L"%x");
        sLifeEnd = pTask->LifeEnd().Format(L"%c");
    }
    else
    {
        sTime = L"--:--:--";
        sDate = L"--/--/--";
        //sDate = L"00/00/00";
        sLifeEnd = L"<b .red>" + pTask->GetLastStartStatusDes() + L"</b>";
    }

//     <table .event_item>
//         <tr>
//              <td rowspan=2 etype="tip" />
//              <td .time>12:30</td>
//              <td .event colspan=2>���ǲ���ҳ��</td>
//         </tr>
//         <tr>
//              <td .date>2013/12/30</td>
//              <td .btns>
//                  <div .btn name="btn_edit" title="�༭" />
//                  <div .btn name="btn_del" title="ɾ��" />
//                  <div .btn name="btn_disable" title="��ִֹ��" />
//              </td>
//         </tr>
//     </table>

    QString sHtml;
    sHtml.Format(
        L"<tr>"
        L"   <td rowspan=2 etype=\"%s\" />"       // event type
        L"   <td .time>%s</td>"     // ִ��ʱ��
        L"   <td .event colspan=2>%s</td>" // %s ִ����������,
        L"</tr>"
        L"<tr>"
        L"   <td .date>%s</td>" // %s ִ������ 2013/12/30
        L"   <td .lifend>%s</td>"
        L"   <td .btns>"
        L"      <div .btn name=\"btn_edit\" title=\"�༭\" />"
//        L"      <div .btn name=\"btn_del\" title=\"ɾ��\" />"
//        L"      <div .btn name=\"btn_disable\" title=\"��ִֹ��\" />"
        L"   </td>"
        L"</tr>", // %s Reminder Flag
        sEType,
        sTime,
        pTask->GetDoWhatString(),
        sDate,
        sLifeEnd);
    eItem.SetHtml(sHtml);
}

void LViewEvent::RefreshEventItem(QAutoTask *pTask)
{
    ECtrl eTableTask = _FindEventItem(pTask);

    RefreshEventItem(eTableTask);
}

void LViewEvent::OnAutoTaskFired( QAutoTask* pTask )
{
    RefreshEventItem(pTask);
}

void LViewEvent::OnClkEnableEventReminder( HELEMENT hBtn )
{
    ECtrl eBtn(hBtn);
    ECtrl eItem = eBtn.select_parent("table",4);
    QAutoTask* pTask = reinterpret_cast<QAutoTask*>(eItem.GetData());
    BOOL bEnable = !(pTask->IsReminderEnabled());
    if (pTask->EnableReminder(bEnable))
    {
        eBtn.SetBkgndImage(pTask->IsReminderEnabled()?L"images/sound_16.png":L"images/nosound_16.png");

        QUIPostCodeToMainWnd(MWND_NOTIFY_AUTOTASKTOGGLEREMINDER,(LPARAM)pTask->ID());
    }
    else
    {
        eItem.ShowTooltip(L"����δ��ɣ�Ҳ��֮ǰû���趨��ʾ����");
    }
}

void LViewEvent::FreshEventReminderFlag( QAutoTask* pTask )
{
    if (NULL != pTask)
    {
        ECtrl eItem = _FindEventItem(pTask);
        if (!eItem.is_valid())
        {
            return;
        }
        ECtrl eBtn = eItem.find_first("td[name=\"td_EnableReminder\"]");
        eBtn.SetBkgndImage(pTask->IsReminderEnabled()?L"images/sound_16.png":L"images/nosound_16.png");
    }
}

HELEMENT LViewEvent::_FindEventItem( QAutoTask* pTask )
{
    return (HELEMENT)_TaskList().FindFirstWithData(pTask);
}

void LViewEvent::SelectEventItem(QAutoTask *pTask)
{
    HELEMENT hItem = _FindEventItem(pTask);
    if (NULL != hItem)
    {
        // ѡ���µ�
        ECtrl eItem(hItem);
        eItem.SetCheck(TRUE,TRUE);
        // ������������
        eItem.scroll_to_view(true);
    }
}

void LViewEvent::RefreshEventNum()
{
    QUIPostCodeToMainWnd(MWND_NOTIFY_EVENTNUMCHANGED,0);
}

void LViewEvent::OnClkEventItem( HELEMENT he )
{
    ETable tblItem(he);
    QAutoTask* pTask = reinterpret_cast<QAutoTask*>(tblItem.GetData());
    ASSERT(NULL != pTask);
    ECtrl ctlPopup = _InfoPopup();
    ECtrl td = ctlPopup.find_first("#p_crtime");
    td.SetText(pTask->CreationTime().Format(L"%c"));
    td = ctlPopup.find_first("#p_status");
    td.SetHtml(L"<b .yellow>" + pTask->GetLastStartStatusDes() + L"</b>");
    td = ctlPopup.find_first("#p_do_what");
    td.SetHtml(pTask->GetDoWhatString());
    td = ctlPopup.find_first("#p_do_time");
    QString str = pTask->GetWhenDoString();
    str.Replace(L",",L"<br/>");
    td.SetHtml(str);
    td = ctlPopup.find_first("#p_rmd");
    td.SetHtml(pTask->GetRemindString());
    td = ctlPopup.find_first("#p_tlife");
    td.SetHtml(pTask->LifeBegin().Format(L"%c") + L"<br />" + pTask->LifeEnd().Format(L"%c"));

    WTL::CRect rcWnd = tblItem.get_location();
    WTL::CPoint ptShow(rcWnd.right + 10,rcWnd.top);
    ::HTMLayoutShowPopupAt(ctlPopup,ptShow,MAKELONG(1,7));
}

void LViewEvent::OnMouseLeaveEventList( HELEMENT )
{
    
}

void LViewEvent::OnCmdSelectEventItem( LPARAM lp )
{
    SelectEventItem(QAutoTaskMan::GetInstance()->GetTask((int)lp));
}

void LViewEvent::OnNotifyJumpOverExec( LPARAM lp )
{
    QAutoTask* pTask = QAutoTaskMan::GetInstance()->GetTask((int)lp);
    if (NULL != pTask)
    {
        // ѡ��
        SelectEventItem(pTask); 

        // ˢ��
        ETable tblItem(_FindEventItem(pTask));
        if (tblItem.is_valid())
        {
            RefreshEventItem(tblItem);
        }
        // ��ʾ
//         if (tblItem.visible())
//         {
//             QString sTip;
//             sTip.Format(L"��<b .yellow>[%s]</b>����һ������ִ��",
//                 QTime::GetCurrentTime().Format(L"%c"));
//             tblItem.ShowTooltip(sTip);
//         }
        // ֪ͨ������ˢ�����´�ִ��ʱ��
        QUIPostCodeToMainWnd(MWND_NOTIFY_AUTOTASKSTART,lp);
    }
}

void LViewEvent::OnDragDropDelete( HELEMENT contbox, HELEMENT src, HELEMENT target )
{
    ETable tblItem(src);
    QAutoTask *pTask = reinterpret_cast<QAutoTask*>(tblItem.GetData());
    if (NULL == pTask)
    {
        int nTaskID = tblItem.get_attribute_int("event_id");
        pTask = QAutoTaskMan::GetInstance()->GetTask(nTaskID);
    }

    if (NULL != pTask)
    {
        QAutoTaskMan::GetInstance()->DeleteTask(pTask);
        QUIPostCodeToMainWnd(MWND_NOTIFY_AUTOTASKDELETE, (LPARAM)pTask->ID());
        AutoSelect();
    }
}

void LViewEvent::AutoSelect()
{
    ETable eItem = _CurSelTask();
    if (!eItem.is_valid())
    {
        ECtrl ctlList = _TaskList();
        int nChild = ctlList.children_count();
        if (nChild > 0)
        {
            eItem = ctlList.child(0);  // ѡ���м����һ��
            eItem.SetCheck(TRUE,FALSE);
//             eItem.scroll_to_view();
        }
    }
}

void LViewEvent::OnKeyDown( UINT nChar,UINT nRepCnt,UINT nFlags )
{
    if (GetKeyState(VK_CONTROL) < 0)
    {
        SetMsgHandled(littlet::OnCtrlKeyPressDown(nChar,nRepCnt,nFlags));
        return;
    }
    SetMsgHandled(FALSE);
}


