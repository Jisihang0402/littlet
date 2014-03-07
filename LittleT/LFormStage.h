#pragma once

#include "ui/QWindow.h"

#include "../common/QLongplan.h"

class LFormStage : public QForm
{
    QUI_DECLARE_EVENT_MAP;
    void ClearMem();

public:
    LFormStage(void);
    ~LFormStage(void);

    void SetPlan(QPlan* pPlan);

    QStage* GetCurrentStage()
    {
        return _CurrentStage();
    }

    void NewStageAdded(QStage* pStage);

    void RefreshCurrentStageItem();

protected:
//     void OnClkBtnPrevStage(HELEMENT);
//     void OnClkBtnNextStage(HELEMENT);

    void OnMenuItemClkNewGoal(HELEMENT, EMenuItem mi);
    void OnMenuItemClkNewStage(HELEMENT, EMenuItem mi);
    void OnMenuItemClkEdit(HELEMENT, EMenuItem mi);
    void OnMenuItemClkDelete(HELEMENT, EMenuItem mi);
    void OnStageItemSelectChanged(HELEMENT he);
    void OnPageNaveSelectChanged(HELEMENT he);

protected:
    ETable AddStage(QStage* pStage);
    void RefreshStageItem(ETable &tblStage);
    void RefreshPageNum();
//    void RefreshStageBar();
    BOOL DeleteStage( QStage* pStage );
    /** ѡ��һ��stage��Ŀ
     *	return:
     *      TRUE    �ɹ�
     *	params:
     *		-[in]
     *          tblStage     ��Ҫ��ѡ�е���Ŀ
     *                       ��Чʱѡ���б��еĵ�һ����Ŀ
    **/
    BOOL SelectStageItem(__in ETable tblStage = NULL);

protected:
    ECtrl _StageBox()
    {
        //return GetRoot();
        return GetCtrl("#id-stage-box");
    }
    
    ETable _CurrentStageItem()
    {
        return _StageBox().find_first("table:checked");
    }

    QStage* _CurrentStage()
    {
        return _StageOfCtl(_CurrentStageItem());
    }

    QStage* _StageOfCtl(ETable tblStage)
    {
        if (tblStage.is_valid())
        {
            ASSERT(aux::wcseqi(tblStage.get_attribute("name"),L"stage-item"));
            return reinterpret_cast<QStage*>(tblStage.GetData());
        }
        return NULL;
    }

    // stage ���
    ECtrl _StageNO()
    {
        return GetCtrl("#id-stage-bar>#id-stage-num");
    }

    // stage ����
    ECtrl _StagePeriod()
    {
        return GetCtrl("#id-stage-bar>#id-stage-period");
    }

    EPageCtrl _PageNav()
    {
        return GetCtrl("#id_stage_nav");
    }

private:
    QPlan*      m_pPlan;
    VecStage    m_stages;
};

class LStageDlg : public QDialog
{
    QUI_DECLARE_EVENT_MAP
public:
    LStageDlg(QPlan *pPlan);    // ���ģʽ
    LStageDlg(QStage* pStage);  // �༭ģʽ

protected:
    virtual LRESULT OnDocumentComplete();
    virtual BOOL OnDefaultButton(INT_PTR nBtn);
    BOOL CheckDate(__out QTime &tmBegin,__out QTime &tmEnd);
    // �������ĺϷ���
    BOOL CheckMemberParameters();

    void OnClkStageIcon(HELEMENT he);

    ECtrl _StageIcon()
    {
        return GetCtrl("#stage-icon");
    }
private:
    QPlan*      m_pPlan;
    BOOL        m_bEditMode;
    int         m_nIconID;

public:
    QStage*     m_pStageInout;
};
