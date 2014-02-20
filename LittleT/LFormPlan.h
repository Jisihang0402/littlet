#pragma once

#include "ui/QWindow.h"
#include "../common/QLongplan.h"

DECLARE_USER_MESSAGE(QSOFT_LITTLET_PLANDELETE);

class LFormPlan : public QForm
{
    QUI_DECLARE_EVENT_MAP;

public:
    LFormPlan(void);
    ~LFormPlan(void);

    void RefreshPlans();

    QPlan* GetCurrentPlan()
    {
        return reinterpret_cast<QPlan*>(_PlanTitle().GetData());
    }

    void OnMenuItemNewPlan(HELEMENT,EMenuItem mi);

protected:
    void OnMenuItemEdit(HELEMENT,EMenuItem mi);
    void OnMenuItemDelete(HELEMENT,EMenuItem mi);
    void OnMenuItemNewStage(HELEMENT,EMenuItem mi);

    ECtrl _PlanBox()
    {
        return GetCtrl("#id-plan-box");
    }

    ECtrl _PlanTitle()
    {
        return GetCtrl("#id-plan-title");
    }

    ETable _CurrentPlanCtl()
    {
        return _PlanBox().find_first("table:checked");
    }

    QPlan * _PlanOfTable(ETable& ctlPlan)
    {
        ASSERT(ctlPlan.is_valid());
        return reinterpret_cast<QPlan*>(ctlPlan.GetData());
    }

    void RefreshPlanItem(ETable &tblPlan);
    ETable AddPlan(QPlan *pPlan);
    // �ֶ��´�����һ���ƻ�
    void NewPlanAdded(QPlan *pPlan);
    void OnPlanItemSelectChanged(HELEMENT he);
    void OnPlanItemClicked(HELEMENT he);
    /** ѡ��һ���ƻ���Ŀ
     *	return:
     *      TRUE    �ɹ�
     *	params:
     *		-[in]
     *          tblPlan     ��Ҫ��ѡ�е���Ŀ
     *                      ��Чʱѡ���б��еĵ�һ����Ŀ
    **/
    BOOL SelectPlanItem(__in ETable tblPlan = NULL);

    void OnClkChkGoalitemShow( HELEMENT he );
};

//////////////////////////////////////////////////////////////////////////
class LPlanDlg : public QDialog
{
public:
    LPlanDlg(QPlan *pPlan = NULL);    // NULLΪ���ģʽ

protected:
    virtual LRESULT OnDocumentComplete();
    virtual BOOL OnDefaultButton(INT_PTR nBtn);
    BOOL CheckDate(__out QTime &tmBegin,__out QTime &tmEnd);
    // �������ĺϷ���
    BOOL CheckMemberParameters();

private:
    BOOL        m_bEditMode;

public:
    QPlan*      m_pPlanInout;
};
