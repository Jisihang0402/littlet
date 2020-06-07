#include "LViewPlan.h"
#include "LDatas.h"
#include "LFormPlan.h"

//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(LViewPlan,LittleTView)
//    BN_CLICKED_ID(L"tbl-curgoal",&LViewPlan::OnClkCurrentGoal)
    BN_CLICKED_ID(L"id_btn_newplan",&LViewPlan::OnClkNewPlan)
    BN_CLICKED_NAME(L"goal-item",&LViewPlan::OnClkGoalCtl)
    BN_CLICKED_ID(L"CHK_SHOW_WAITING",&LViewPlan::OnClkChkGoalitemShow)
    BN_CLICKED_ID(L"CHK_SHOW_FINISHED", &LViewPlan::OnClkChkGoalitemShow)
    CHAIN_HLEVENT_TO_FORM(&m_formPlan,"#id-plan-wrapper")
    CHAIN_HLEVENT_TO_FORM(&m_formGoal,"#id-goal-box")
    CHAIN_HLEVENT_TO_FORM(&m_formStage,"#id-stage-box-wrapper")
QUI_END_EVENT_MAP()

QUI_BEGIN_REFLECT_EVENT_MAP(LViewPlan, LittleTView)
//    BN_STATECHANGED_NAME(L"plan-item", &LViewPlan::OnPlanSelectChanged)
//     BN_CLICKED_ID(L"btn-next-stage", &LViewPlan::OnStageItemChanged)
//     BN_CLICKED_ID(L"btn-prev-stage", &LViewPlan::OnStageItemChanged)
QUI_END_REFLECT_EVENT_MAP()

LViewPlan::LViewPlan(void)
{
    LDatas::GetInstance()->SetViewPlanPtr(this);
}

LViewPlan::~LViewPlan(void)
{
}

void LViewPlan::OnKeyDown( UINT nChar,UINT nRepCnt,UINT nFlags )
{
    if (GetKeyState(VK_CONTROL) < 0)
    {
        SetMsgHandled(littlet::OnCtrlKeyPressDown(nChar,nRepCnt,nFlags));
        return;
    }
    SetMsgHandled(FALSE);
}

void LViewPlan::OnClkNewPlan( HELEMENT hBtn )
{
//     LPlanDlg PDlg(NULL);
//     if (PDlg.DoModal() == IDOK)
//     {
//         m_formPlan.NewPlanAdded(PDlg.m_pPlanInout);
//     }
    m_formPlan.OnMenuItemNewPlan(NULL,NULL);
}

LRESULT LViewPlan::OnDocumentComplete()
{
    m_formPlan.RefreshPlans();

    NotifyPlanNumChanged();

    return 0;
}

//////////////////////////////////////////////////////////////////////////
// ����ѡ��仯
void LViewPlan::OnPlanSelectChanged( HELEMENT he )
{
//     QPlan *pPlan = m_formPlan.GetCurrentPlan();
//     m_formStage.SetPlan(pPlan);
// 
//     // �����ˢ��plan����Ŀ(���ã����Ƿ���)
//     NotifyPlanNumChanged();

    OnPlanSelect((LPARAM)m_formPlan.GetCurrentPlan());
}

void LViewPlan::OnPlanSelect( LPARAM lParam )
{
//    QPlan *pPlan = m_formPlan.GetCurrentPlan();
    QPlan *pPlan = reinterpret_cast<QPlan*>(lParam);

    m_formStage.SetPlan(pPlan);

    // �����ˢ��plan����Ŀ(���ã����Ƿ���)
    NotifyPlanNumChanged();
}

//////////////////////////////////////////////////////////////////////////
void LViewPlan::RefreshCurrentGoal()
{
//     QGoal* pGoal = m_formGoal.GetCurrentGoal();
//     ETable ctlGoal = _CurrentGoalCtl();
//     ctlGoal.SetData(pGoal);
// 
//     if (NULL != pGoal)
//     {
//         ECtrl ctl = ctlGoal.find_first("#id-goal");
//         QString sText;
//         sText.Format(L"%s  <b .red>[%d]</b>", pGoal->Goal(), 
//             pGoal->GetGoalItemUnfinishedNum()/*, pGoal->GetGoalItemNum()*/);
//         ctl.SetHtml(sText);
// //        ctl = ctlGoal.find_first("#id-goalitem-checked");
// //         QString sHtml;
// //         sHtml.Format(L"<div .number-text>%d / %d</div>", 
// //             pGoal->GetGoalItemUnfinishedNum(), pGoal->GetGoalItemNum());
// //         ctl.SetHtml(sHtml);
//         ctl = GetCtrl("#id-goal-des");
//         ctl.SetText(pGoal->Des());
//     }
//     else
//     {
//         ECtrl ctl = ctlGoal.find_first("#id-goal");
//         ctl.SetText(L"��û��Ŀ�겻���Ͻ����꡿");
// //         ctl = ctlGoal.find_first("#id-goalitem-checked");
// //         ctl.SetText(L"-");
//         ctl = GetCtrl("#id-goal-des");
//         ctl.SetText(L"�������һ��Ŀ���^_^");
//     }
}

// ѡ��һ��goalʱ����ʾ���µ�goal-item
void LViewPlan::OnClkGoalCtl( HELEMENT he )
{
}

//////////////////////////////////////////////////////////////////////////
// stage ��ӡ�ɾ�����仯
void LViewPlan::OnStageAdded( LPARAM lParam )
{
}

void LViewPlan::OnStageDeleted( LPARAM lParam )
{
    //OnStageSelChanged((LPARAM)m_formStage.GetCurrentStage());
}

void LViewPlan::OnStageSelChanged( LPARAM lParam )
{
    QStage *pStage = reinterpret_cast<QStage*>(lParam);
    m_formGoal.SetStage(pStage);
}

//////////////////////////////////////////////////////////////////////////
// goal ��Ӻ�ɾ��
void LViewPlan::OnGoalAdded( LPARAM lParam )
{
    // ��ʾgoal
    //ShowGoalPane(TRUE);
    m_formStage.RefreshCurrentStageItem();
}

void LViewPlan::OnGoalDeleted( LPARAM lParam )
{
    // ��ʾgoal
    //ShowGoalPane(TRUE);
    m_formStage.RefreshCurrentStageItem();
}

void LViewPlan::OnGoalSelChanged( LPARAM lParam )
{

}

//////////////////////////////////////////////////////////////////////////
// goalitem ��ӡ�ɾ����״̬�ı�
void LViewPlan::OnGoalSubitemAdded( LPARAM lParam )
{
    QGoalItem *pGoalItem = reinterpret_cast<QGoalItem*>(lParam);
    ASSERT(NULL != pGoalItem);
    m_formGoal.NewGoalSubItemAdded(pGoalItem);

    RefreshCurrentGoal();
}

void LViewPlan::OnGoalSubitemDeleted( LPARAM lParam )
{
    QGoal* pGoalParent = reinterpret_cast<QGoal*>(lParam);
    ASSERT(NULL != pGoalParent);
    m_formGoal.GoalSubItemDelete(pGoalParent);
}

void LViewPlan::OnGoalSubitemStatusChanged( LPARAM lParam )
{
    QGoalItem* pGoalItem = reinterpret_cast<QGoalItem*>(lParam);
    ASSERT(NULL != pGoalItem);
    m_formGoal.GoalSubItemStatusChanged(pGoalItem);

    // ˢ�µ�ǰ��goal��ʾ
    RefreshCurrentGoal();
}

void LViewPlan::NotifyPlanNumChanged()
{
    QUIPostCodeToMainWnd(MWND_NOTIFY_PLANNUMCHANGED, 
        QPlanMan::GetInstance()->GetWorkingPlanNum());
}

void LViewPlan::OnClkChkGoalitemShow( HELEMENT he )
{
    BOOL bShowWaiting = ECheck(GetCtrl("#CHK_SHOW_WAITING")).IsChecked();
    BOOL bShowDone = ECheck(GetCtrl("#CHK_SHOW_FINISHED")).IsChecked();
    m_formGoal.SetGoalitemShow(bShowDone, bShowWaiting);
}
