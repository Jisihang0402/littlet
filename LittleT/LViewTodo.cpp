#include "LViewTodo.h"

#include "../common/ConstValues.h"
#include "LDatas.h"
#include "ui/WndHelper.h"

QUI_BEGIN_EVENT_MAP(LFormTodo, QForm)
    BN_CLICKED_NAME(L"Chk_NoteTask", &LFormTodo::OnClkTaskChk)
    BN_CLICKED_NAME(L"btn_todoitem_delete", &LFormTodo::OnClkDeleteTask)
    BN_CLICKED_NAME(L"item_todo", &LFormTodo::OnClkTodoItem)
    BN_CLICKED_NAME(L"item_doit", &LFormTodo::OnClkDoit)
    BN_CLICKED_ID(L"btn_plus5m", &LFormTodo::OnClkPlus5Minutes)
    BN_CLICKED_ID(L"btn_minus5m", &LFormTodo::OnClkMinus5Minutes)
    BN_CLICKED_ID(L"id_btn_newtodo", &LFormTodo::OnClkNewTask)
    BN_CLICKED_ID(L"id_bar_ok", &LFormTodo::OnClkIdbarOK)
    BN_CLICKED_ID(L"id_bar_cancel", &LFormTodo::OnClkIdbarCancel)
    BN_CLICKED_ID(L"CHK_TODO_WAITING", &LFormTodo::OnTodoShow)
    BN_CLICKED_ID(L"CHK_TODO_FINISHED", &LFormTodo::OnTodoShow)
    BN_STATECHANGED_NAME(L"star_Priority", &LFormTodo::OnClkPriority)
QUI_END_EVENT_MAP()

LFormTodo::~LFormTodo(void)
{
}

void LFormTodo::OnAttach()
{
    ShowTask(TODO_STATUS_PROCESSING);
}

//////////////////////////////////////////////////////////////////////////
//  [9/18/2011 FZ] Note Task
void LFormTodo::OnClkTaskChk( HELEMENT hBtn )
{
    ECtrl eBtn(hBtn);
    ECtrl table = eBtn.select_parent(L"table",4);
    int nID = (int)table.GetData();
    ECtrl eStarbox = table.find_first("[name=\"star_Priority\"]");
    if (eBtn.IsChecked())
    {
        table.set_attribute("done",L"");
        eStarbox.EnableCtrl(FALSE);
        // ������todo�е���ʱ����ôֹͣ����ʱ
        ETable tblTest = table.next_sibling();
        if (tblTest.ID().CompareNoCase(L"do_it") == 0)
        {
            tblTest.destroy();
        }

        QDBEvents::GetInstance()->TodoTask_SetStatus(nID,TODO_STATUS_FINISH);
    }
    else 
    {
        table.remove_attribute("done");
        eStarbox.EnableCtrl(TRUE);
        QDBEvents::GetInstance()->TodoTask_SetStatus(nID,TODO_STATUS_PROCESSING);
    }
    RefreshTaskNum();
}

void LFormTodo::OnClkDeleteTask( HELEMENT hBtn )
{
    ECtrl table = ECtrl(hBtn).select_parent(L"table",4);
    if (QDBEvents::GetInstance()->TodoTask_Delete((int)table.GetData()))
    {
        table.destroy();
        RefreshTaskNum();
    }
}

void LFormTodo::OnClkNewTask( HELEMENT hBtn )
{
    ShowPopupBar(TTodoTask(),FALSE);
}

// <table><tr><td><widget type="checkbox" /></td><td>hello wolrd!</td></tr></table>
void LFormTodo::InsertTask(ECtrl& eGroup, TTodoTask* pTask)
{
    ASSERT(pTask != NULL);
    ECtrl etable = element::create("table");
    eGroup.insert(etable,0);
    etable.set_attribute("name",L"todoitem");

    FreshTaskItem(eGroup,etable,pTask);
}

void LFormTodo::FreshTaskItem( ECtrl& eGroup, ECtrl &etable, TTodoTask* pTask )
{
    if (NULL == pTask)
    {
        ASSERT(FALSE);
        return;
    }

    etable.SetData((LPVOID)pTask->nID);
    QString str;
//     str.Format(L"<tr><td .item-exec><widget type=\"checkbox\" name=\"Chk_NoteTask\" %s>"
//         L"<b.gray>%s</b></widget></td><td .item-todo name=\"item_todo\">%s</td>"
//         L"<td .item-creat>%s</td>"
//         L"<td><ul type=\"starbox\" name=\"star_Priority\" stars=\"5\" index=\"%d\" %s/></td>"
//         L"<td .btn name=\"btn_todoitem_delete\" /></tr>",
//         (pTask->eStatus == TODO_STATUS_FINISH)?L"checked":L"",
//         (_HasFlag(pTask->nFlag,TODO_FLAG_HASEXECTIME))
//         ?(pTask->tmExec.Format(L"%Y/%m/%d %H:%M"))
//         :(L"<b.gray>��ִ��ʱ��</b>"),
//         pTask->sTask,
//         pTask->tmCreate.Format(L"%Y/%m/%d %H:%M"),
//         pTask->nPriority,
//         (pTask->eStatus == TODO_STATUS_FINISH)?L"disabled":L"");
    str.Format(L"<tr title=\"������:%s\">"
        L"<td .item-exec><widget type=\"checkbox\" name=\"Chk_NoteTask\" %s /></td>"
        L"<td name=\"item_doit\" .qbtn/>"
        L"<td .item-todo name=\"item_todo\">%s</td>"
        L"<td><ul type=\"starbox\" name=\"star_Priority\" stars=\"3\" index=\"%d\" %s/></td>"
        L"<td name=\"btn_todoitem_delete\">r</td>"
        L"</tr>",
        pTask->tmCreate.Format(L"%c"),      // ����ʱ��
        (pTask->eStatus == TODO_STATUS_FINISH)?L"checked":L"",
        pTask->sTask,
        pTask->nPriority,
        (pTask->eStatus == TODO_STATUS_FINISH)?L"disabled":L"");

    etable.SetHtml(str);
    if (pTask->eStatus == TODO_STATUS_FINISH)
    {
        etable.set_attribute("done",L"");
    }
}

void LFormTodo::OnClkPriority( HELEMENT he)
{
    EStarBox eStarBox(he);
    ECtrl eTable = eStarBox.select_parent("table",4);
    int nID = (int)(eTable.GetData());
    QDBEvents::GetInstance()->TodoTask_SetPriority(nID,eStarBox.GetCurSel());
}

BOOL LFormTodo::ShowTask( ENUM_TODO_STATUS eStatus )
{
    QDBEvents *pMgr = QDBEvents::GetInstance();
    TodoTaskList lst;
    if (TODO_STATUS_SHOWALL == eStatus)
        pMgr->TodoTask_GetAll(lst);
    else if (TODO_STATUS_FINISH == eStatus)
        pMgr->TodoTask_GetFinished(lst);
    else if (TODO_STATUS_PROCESSING == eStatus)
        pMgr->TodoTask_GetUnfinished(lst);

//     class delete_todo_items : public htmlayout::dom::callback
//     {
//     public:
//          // return false to continue enumeration
//          virtual bool on_element(HELEMENT he)
//          {
//              ECtrl item(he);
//              ASSERT(wcseqi(item.get_attribute("name"), L"todoitem"));
//              item.destroy();
// 
//              return false;
//          }
//     };

    ECtrl ctlList = _TodoList();
    // ɾ��todoitem�����ǲ�Ҫɾ��do_it ���table
    //ctlList.select_elements(&delete_todo_items(), L"table[name=\"todoitem\"]");
    ctlList.DeleteAllChild();
    for (TodoTaskListItr itr = _BeginItr(lst); itr != _EndItr(lst); ++itr)
    {
        InsertTask(ctlList, &(*itr));
    }
    return TRUE;
}

void LFormTodo::ShowPopupBar( TTodoTask &t,BOOL bEdit,HELEMENT he )
{
    if (NULL == he)
    {
        he = GetCtrl("#id_todo_caption");
    }

    // ȷ��������ǰ̨����
    quibase::SetForegroundWindowInternal(QUIGetMainWnd());

    ECtrl bar = _PopupBar();
    //  Ŀ¼
//     ECombobox eCmbCate = bar.find_first("#id_bar_cate");
//     eCmbCate.DeleteAllItem();
//     // ��һ��ΪĬ��
//     eCmbCate.InsertItem(L"Ĭ��");
//     eCmbCate.SetItemData(0,(LPVOID)(-1));
//     QDBHelper *pMan = QDBHelper::GetInstance();
//     VecCate vc;
//     pMan->Cate_GetAll(vc);
//     for (int i = 0; i < vc.size(); i++)
//     {
//         int iItem = eCmbCate.InsertItem(vc[i].sCate);
//         eCmbCate.SetItemData(iItem,(LPVOID)vc[i].nID);
//     }

    EEdit ctlInput = bar.find_first("#id_bar_todo");
    // ģʽ����
    if (bEdit)
    {
//         ECheckBox(bar.find_first("#id_bar_hasexectime")).SetCheck(_HasFlag(t.nFlag,TODO_FLAG_HASEXECTIME));
//         EDate(bar.find_first("#id_bar_date")).SetDate(t.tmExec);
//         ETime(bar.find_first("#id_bar_time")).SetTime(t.tmExec);
        ctlInput.SetText(t.sTask);

//      ECombobox(bar.find_first("#id_bar_cate")).SelectItem_ItemData((LPVOID)t.nCateID);
        EStarBox(bar.find_first("#id_bar_priority")).SetCurSel(t.nPriority);

 //       ECtrl(bar.find_first("#id_bar_ok")).SetText(L"�༭");
        bar.set_attribute("edit",L"true");
    }
    else
    {
//         ECheckBox(bar.find_first("#id_bar_hasexectime")).SetCheck(FALSE);
        ctlInput.SetText(L"");
//        ECtrl(bar.find_first("#id_bar_ok")).SetText(L"���");
        bar.set_attribute("edit",L"false");
    }
    ctlInput.SelectText();
    ctlInput.SetFocus();

    HTMLayoutShowPopup(bar,he,2);
}

void LFormTodo::OnClkIdbarOK( HELEMENT )
{
    ECtrl bar = _PopupBar();

    TTodoTask t;
    t.nFlag = 0;
//     if (ECheckBox(bar.find_first("#id_bar_hasexectime")).IsChecked())
//     {
//         _AddFlg(t.nFlag,TODO_FLAG_HASEXECTIME);
//         t.tmExec = EDate(bar.find_first("#id_bar_date")).GetDate();
//         t.tmExec.SetTime(ETime(bar.find_first("#id_bar_time")).GetTime());
//     }
    t.sTask = EEdit(bar.find_first("#id_bar_todo")).GetText();
    t.sTask = t.sTask.Trim();
    if (t.sTask.IsEmpty())
    {
        return ;
    }

//     ECombobox eCmb = bar.find_first("#id_bar_cate");
//     t.nCateID = (int)eCmb.GetItemData(eCmb.GetCurSel());
    t.nCateID = INVALID_ID;
    t.nPriority = EStarBox(bar.find_first("#id_bar_priority")).GetCurSel();
    t.eStatus = TODO_STATUS_PROCESSING;

    if (aux::wcseqi(L"true",bar.get_attribute("edit")))
    { // edit
        t.nID = (int)m_eItemEdit.GetData();
        if (QDBEvents::GetInstance()->TodoTask_Edit(&t))
        {
            FreshTaskItem(_TodoList(),m_eItemEdit,&t);
        }
    }
    else
    { // add 
        t.nID = QDBEvents::GetInstance()->TodoTask_Add(&t);
        if (INVALID_ID != t.nID)
        {
            InsertTask(_TodoList(),&t);
            RefreshTaskNum();            
        } 
    }
    HTMLayoutHidePopup(_PopupBar());
}

void LFormTodo::OnClkIdbarCancel( HELEMENT )
{
    HTMLayoutHidePopup(_PopupBar());
}

void LFormTodo::OnClkTodoItem( HELEMENT he)
{
    m_eItemEdit = ECtrl(he).select_parent("table",3);
    int nID = (int)m_eItemEdit.GetData();
    QDBEvents *pDB = QDBEvents::GetInstance();
    if (!pDB->TodoTask_IsDone(nID))
    {
        TTodoTask t;
        if (QDBEvents::GetInstance()->TodoTask_Get(nID,t))
        {
            ShowPopupBar(t,TRUE,m_eItemEdit);
        }
    }
}

void LFormTodo::OnTodoShow( HELEMENT he )
{
    QDBEvents *pDB = QDBEvents::GetInstance();
    BOOL bShowWaiting = ECheck(GetCtrl("#CHK_TODO_WAITING")).IsChecked();
    BOOL bShowDone = ECheck(GetCtrl("#CHK_TODO_FINISHED")).IsChecked();
    if (bShowDone && bShowWaiting)
        ShowTask(TODO_STATUS_SHOWALL);
    else if (bShowDone)
        ShowTask(TODO_STATUS_FINISH);
    else if (bShowWaiting)
        ShowTask(TODO_STATUS_PROCESSING);
    else
        ShowTask(TODO_STATUS_SHOWNONE);
}

void LFormTodo::RefreshTaskNum()
{
    QUIPostCodeToMainWnd(MWND_NOTIFY_TODOTASKCHANGED,0);
}

void LFormTodo::OnClkDoit( HELEMENT he)
{
    ETable ctlItem = ECtrl(he).select_parent("table",3);
    ECtrl ctlList = _TodoList();
    ETable tblCountdown = ctlList.find_first("table#do_it");
    if ( tblCountdown.is_valid() )
    {
        // �����������ظ������
        if (tblCountdown == ctlItem.next_sibling())
        {
            // ȡ������ʱ
            tblCountdown.destroy();
            return;
        }

        // ��ǰ�������û���꣬����ȥ������һ�����ˣ�
        ETextCountdown tcd = tblCountdown.find_first("#td_countdown");
        if (tcd.GetCountdown() > 0)
        {
            ctlItem.ShowTooltip(L"����ע������ֻ��һ���¾ͺ�����");
            return;
        }
        // ��ǰ�������Ѿ������ˡ�
        tblCountdown.destroy(); // ��һ�����ٵ�
    }

    // ÿ��ֻ����һ��������ʱ
    tblCountdown = (HELEMENT)ETable::create("table");
    ctlList.insert(tblCountdown, ctlItem.index() + 1);
    tblCountdown.set_attribute("id",L"do_it");
    tblCountdown.SetHtml(
                L"<tr>"
                L"  <td .icon rowspan=2 />"
                L"  <td id=\"td_countdown\" rowspan=2>00:15:00</td>"    // ��ʼ��15����
                L"  <td id=\"btn_plus5m\" .qbtn/>"
                L"</tr>"
                L"<tr>"
                L"  <td id=\"btn_minus5m\" .qbtn/>"
                L"</tr>");
}

void LFormTodo::OnClkPlus5Minutes( HELEMENT )
{
    ETable tblCountdown = _TodoList().find_first("table#do_it");
    ETextCountdown tcd = tblCountdown.find_first("#td_countdown");
    tcd.Increase(5 * 60);
}

void LFormTodo::OnClkMinus5Minutes( HELEMENT )
{
    ETable tblCountdown = _TodoList().find_first("table#do_it");
    ETextCountdown tcd = tblCountdown.find_first("#td_countdown");
    tcd.Decrease(5 * 60);
}

//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(LViewTodo, QView)
    CHAIN_HLEVENT_TO_FORM(&m_formTodo, "body")
QUI_END_EVENT_MAP()

LViewTodo::LViewTodo()
{
    LDatas::GetInstance()->SetViewTodoPtr(this);
}

void LViewTodo::OnKeyDown( UINT nChar,UINT nRepCnt,UINT nFlags )
{
    if (GetKeyState(VK_CONTROL) < 0)
    {
        SetMsgHandled(littlet::OnCtrlKeyPressDown(nChar,nRepCnt,nFlags));
        return;
    }
    SetMsgHandled(FALSE);
}

//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(LDesktopTodo, QView)
    CHAIN_HLEVENT_TO_FORM(&m_formTodo, "body")
QUI_END_EVENT_MAP()

void LDesktopTodo::OnKeyDown( UINT nChar,UINT nRepCnt,UINT nFlags )
{
    if (GetKeyState(VK_CONTROL) < 0)
    {
        SetMsgHandled(littlet::OnCtrlKeyPressDown(nChar,nRepCnt,nFlags));
        return;
    }
    SetMsgHandled(FALSE);
}

LRESULT LDesktopTodo::OnDocumentComplete()
{
//    GetCtrl("body").set_attribute("id", L"wc-top");

    GetCtrl("#wc-caption").ShowCtrl(SHOW_MODE_SHOW);
    GetCtrl("#id_todo_caption").ShowCtrl(SHOW_MODE_COLLAPSE);

    ECtrl cStyle = GetCtrl("link#stylebag");
    cStyle.set_attribute("href", L"qrel:style-d.css");
    cStyle.xcall("activate");

    return 0;
}

