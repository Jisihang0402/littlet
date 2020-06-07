#include "LFormGoal.h"
#include "../common/LittleTUIcmn.h"
#include "ui/QUIGlobal.h"

QUI_BEGIN_EVENT_MAP(LFormGoal,QForm)
//    BN_STATECHANGED_NAME(L"goal-item",&LFormGoal::OnGoalSelChanged)
    MENUITEM_CLICK_ID(L"mi-goal-newgoalitem", &LFormGoal::OnMenuItemClkNewGoalSubItem)
    MENUITEM_CLICK_ID(L"mi-goal-newgoal", &LFormGoal::OnMenuItemClkNewGoal)
    MENUITEM_CLICK_ID(L"mi-goal-edit", &LFormGoal::OnMenuItemClkEdit)
    MENUITEM_CLICK_ID(L"mi-goal-delete", &LFormGoal::OnMenuItemClkDelete)
    BN_CLICKED_NAME(L"btn-delete-goal",&LFormGoal::OnClkDeleteGoal)
    BN_STATECHANGED_NAME(L"goal-item",&LFormGoal::OnGoalSelChanged)
QUI_END_EVENT_MAP()

LFormGoal::LFormGoal(void)
{
    m_pStage = NULL;
}

LFormGoal::~LFormGoal(void)
{
    ClearMem();
}

void LFormGoal::SetStage( QStage* pStage )
{
    m_pStage = pStage;
    m_formGoalItem.Destroy();
    _GoalBox().DeleteAllChild();
    ClearMem();

    if (NULL != m_pStage)
    {
        int nGoal = m_pStage->GetAllGoals(m_goals);
        for (int i = 0; i < nGoal; ++i)
        {
            AddGoalCtl(m_goals[i]);
        }
    }
    SelectGoal(NULL);
    _GoalBox().refresh();
}

void LFormGoal::RefreshGoal( ETable &tblGoal )
{
    if (!tblGoal.is_valid())
    {
        ASSERT(FALSE);
        return;
    }
    QGoal *pGoal = reinterpret_cast<QGoal*>(tblGoal.GetData());
    if (NULL == pGoal)
    {
        ASSERT(FALSE);
        return;
    }

    CStdString sHtml;
    sHtml.Format(
        L"<tr>"
        L"  <td .goal-title>%s</td>"  // %s: goal-item
        L"  <td><div .number-text>%d / %d</div></td>" // %d / %d (working/ all)goal num
//        L"  <td .close-btn name=\"btn-delete-goal\">r</td>"
        L"</tr>",
        pGoal->Goal(),
        pGoal->GetGoalItemUnfinishedNum(),
        pGoal->GetGoalItemNum());
    tblGoal.SetHtml(sHtml);
}

ETable LFormGoal::AddGoalCtl( QGoal* pGoal )
{
    ETable tblGoal = ETable::create("table");
    _GoalBox().append(tblGoal);
    tblGoal.set_attribute("name", L"goal-item");
    tblGoal.SetData(pGoal);
    RefreshGoal(tblGoal);
    return tblGoal;
}

// void LFormGoal::OnGoalSelChanged( HELEMENT he )
// {
//     _GoalBox().ShowCtrl(sho)
// }
void LFormGoal::NewGoalAdded( QGoal* pGoal )
{
    ASSERT(NULL != pGoal);
    m_goals.push_back(pGoal);
    ETable tblGoal = AddGoalCtl(pGoal);
    // ѡ������ӵ����
    SelectGoal(tblGoal);
}

void LFormGoal::NewGoalSubItemAdded( QGoalItem *pGoalItem )
{
    ASSERT(NULL != pGoalItem);
    QGoal* pGoal = GetCurrentGoal();
    if ( (NULL == pGoal) || (pGoal->ID() != pGoalItem->GoalID()))
    {
        ASSERT(FALSE);
        return;
    }
    RefreshGoal(_CurrentGoalCtl());
}

void LFormGoal::OnMenuItemClkNewGoal( HELEMENT,EMenuItem )
{
    if (NULL != m_pStage)
    {
        LGoalDlg GDlg(m_pStage);
        if (GDlg.DoModal() == IDOK)
        {
            //SelectGoal();
            NewGoalAdded(GDlg.m_pGoalInout);

            QUISendCodeTo(GetSafeHwnd(), VIEWPLAN_NOTIFY_GOALADDED,
                (LPARAM)(GDlg.m_pGoalInout));
        }
    }
}

void LFormGoal::OnMenuItemClkEdit( HELEMENT he,EMenuItem )
{
    QGoal *pGoal = _GoalOfCtl(he);
    if (NULL == pGoal)
        return;

    LGoalDlg GDlg(pGoal);
    if (GDlg.DoModal() == IDOK)
    {
        ETable tblGoal = _GoalBox().FindFirstWithData(pGoal);
        RefreshGoal(tblGoal);
        // ���ϲ�Ҳˢ����ʾgoal
        SelectGoal(he);
    }
}

void LFormGoal::OnMenuItemClkDelete( HELEMENT he,EMenuItem )
{
    if (NULL != _GoalOfCtl(he))
    {
        OnClkDeleteGoal(ETable(he).child(0));
    }
}

void LFormGoal::OnMenuItemClkNewGoalSubItem( HELEMENT he,EMenuItem )
{
    SelectGoal(he);

    m_formGoalItem.NewGoalItem( he );
}

BOOL LFormGoal::SelectGoal( __in ETable tblGoal /*= NULL*/ )
{
    if ( !tblGoal.is_valid() )
    {   
        // ��Ч�Ļ���ѡ�е�һ����Ŀ
        // ѡ�е�һ��
        tblGoal = _GoalBox().find_first("[name=\"goal-item\"]");
    }
#ifdef _DEBUG
    else
    {
        ASSERT(aux::wcseqi(L"goal-item", tblGoal.get_attribute("name")));
    }
#endif
    if (tblGoal.is_valid())
    {
        // ������������һ��form
        CreateGoalitemForm(tblGoal);

        tblGoal.SetCheck(TRUE,TRUE);
        QGoal *pGoal = _GoalOfCtl(tblGoal);

        tblGoal.scroll_to_view(true, true);

        // ֪ͨ�ϼ��Լ�ѡ��仯��
        QUIPostCodeTo(GetSafeHwnd(), VIEWPLAN_NOTIFY_GOALSELCHANGED,
            (LPARAM)pGoal);
    }
    return TRUE;
}

void LFormGoal::CreateGoalitemForm(ETable &tblGoal)
{
    int idx = m_formGoalItem.GetIndex();
    int iPos = tblGoal.index();
    if ((-1 != idx) && (iPos < idx))
        iPos += 1;
    iPos = (0 == iPos) ? 1 : iPos;
    // ���½������form
    m_formGoalItem.CreateForm(GetRoot(), iPos, _GoalOfCtl(tblGoal));

    RemoveFormEntry(&m_formGoalItem);

    // ����form��Ϣӳ��
    __AddFormEntry(&m_formGoalItem);
}

void LFormGoal::OnClkDeleteGoal( HELEMENT he)
{
    ETable tblGoal = ECtrl(he).select_parent("table", 4);
    QGoal* pGoal = _GoalOfCtl(tblGoal);
    ASSERT(NULL != pGoal);

    BOOL bIsCurrentSel = tblGoal.IsChecked();
    // find goal
    VecGoalItr iEnd = m_goals.end();
    VecGoalItr i = m_goals.begin();
    while ( i != iEnd )
    {
        if ((*i) == pGoal)
            break;
        ++i;
    }
    ASSERT(i != iEnd);
    
    CStdString sTip;
    sTip.Format(L"ȷ��ɾ�����Ŀ���𣿣�������[%d]��Ŀ�ꡣ", 
        pGoal->GetGoalItemNum());
    if (XMsgBox::YesNoMsgBox(sTip) == IDYES)
    {
        int nGoalID = pGoal->ID();
        if (m_pStage->DeleteGoal(pGoal))
        {
            delete pGoal;
            m_goals.erase(i);

            tblGoal.destroy();
            if (bIsCurrentSel)
            {
                m_formGoalItem.Destroy();
                SelectGoal(NULL);
            }

            QUIPostCodeTo(GetSafeHwnd(), VIEWPLAN_NOTIFY_GOALDELETED, 
                (LPARAM)nGoalID);
        }
    }
}

void LFormGoal::ClearMem()
{
    std::for_each(m_goals.begin(), m_goals.end(),
        [](QGoal* p) { delete p; } );
    m_goals.clear();
}

void LFormGoal::GoalSubItemDelete( QGoal *pParentGoal )
{
    // ˢ����ʾ��Ŀ
    ETable tblGoal = _GoalBox().FindFirstWithData(pParentGoal);
    RefreshGoal(tblGoal);
}

void LFormGoal::GoalSubItemStatusChanged( QGoalItem *pGoalItem )
{
    QGoal *pGoal = NULL;
    for (VecGoalItr i = m_goals.begin(); i != m_goals.end(); ++i)
    {
        if ( (*i)->ID() == pGoalItem->GoalID())
        {
            pGoal = *i;
            break;
        }
    }
    // ˢ����ʾ��Ŀ
    ETable tblGoal = _GoalBox().FindFirstWithData(pGoal);
    RefreshGoal(tblGoal);
}

void LFormGoal::OnGoalSelChanged( HELEMENT )
{
    SelectGoal(_CurrentGoalCtl());
}

void LFormGoal::SetGoalitemShow( BOOL bShowFinished, BOOL bShowUnFinished )
{
    m_formGoalItem.SetShowFlag(bShowFinished, bShowUnFinished);
}

//////////////////////////////////////////////////////////////////////////
LGoalDlg::LGoalDlg( QGoal *pGoal )
    :QDialog(L"qabs:plandlgs/GoalDlg.htm")
{
    ASSERT(NULL != pGoal);
    m_pGoalInout = pGoal;
    m_pStage = NULL;
    m_bEditMode = TRUE;
}

LGoalDlg::LGoalDlg( QStage* pStage )
    :QDialog(L"qabs:plandlgs/GoalDlg.htm")
{
    ASSERT(NULL != pStage);
    m_pGoalInout = NULL;
    m_pStage = pStage;
    m_bEditMode = FALSE;
}

BOOL LGoalDlg::OnDefaultButton( INT_PTR nBtn )
{
    if (IDOK != nBtn)
        return TRUE;

    if (!CheckMemberParameters())
        return TRUE;

    CStdString sStage = EEdit(GetCtrl("#txt-goal")).GetText().Trim();
    if (sStage.IsEmpty())
    {
        XMsgBox::OkMsgBox(L"Ŀ��������Ϊ��");
        return FALSE;
    }
    // ��Ҫ����
    BOOL bOK = FALSE;
    if (m_bEditMode)
    {
        bOK = m_pGoalInout->EditGoal(
            sStage,EEdit(GetCtrl("#txt-goaldes")).GetText());
    }
    else
    {
        QGoal      newGoal;
        newGoal.m_sGoal = sStage;
        newGoal.m_nStageID = m_pStage->ID();
        newGoal.m_sDes = EEdit(GetCtrl("#txt-goaldes")).GetText();
        m_pGoalInout = m_pStage->AddGoal(newGoal);
        bOK = (NULL != m_pGoalInout);
    }
    if (!bOK)
    {
        XMsgBox::ErrorMsgBox(L"����ʧ�ܣ�������.");
        return FALSE;
    }
    return TRUE;
}

BOOL LGoalDlg::CheckMemberParameters()
{
    BOOL bOK = FALSE;
    if (m_bEditMode)
    {
        bOK = (m_pGoalInout != NULL);
    }
    else
    {
        bOK = (m_pStage != NULL);
    }
    if ( !bOK )
    {
        XMsgBox::ErrorMsgBox(L"���ݴ��󣬲���ʧ��!");
    }
    return bOK;
}

LRESULT LGoalDlg::OnDocumentComplete()
{
    if (m_bEditMode && (NULL != m_pGoalInout))
    {
        EEdit(GetCtrl("#txt-goal")).SetText(m_pGoalInout->Goal());
        EEdit(GetCtrl("#txt-goaldes")).SetText(m_pGoalInout->Des());
//         EDate(GetCtrl("#date_begin")).SetDate(m_pStage->BeginTime());
//         EDate(GetCtrl("#date_end")).SetDate(m_pStage->EndTime());
    }
    return 0;
}

