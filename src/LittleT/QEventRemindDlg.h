#pragma once

#include "ui/QDialog.h"
#include "../common/QTimer.h"

enum 
{
    AUTOTASK_REMINDER_COUNTDOWN_TIMERID = 101,
};

class LSingleRmdDlg : public QDialog
{
    QUI_DECLARE_EVENT_MAP;

    BEGIN_MSG_MAP_EX(LSingleRmdDlg)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_TIMER(OnTimer)
        CHAIN_MSG_MAP(QDialog)
    END_MSG_MAP()

public:
    LSingleRmdDlg(LPTASK_REMINDER_PARAM pRp = NULL);
    ~LSingleRmdDlg(void);

    void SetRmdParam(LPTASK_REMINDER_PARAM pRP);

protected:
    virtual LRESULT OnDocumentComplete();
    void OnClkDontRemindAgain(HELEMENT hBtn);
    void OnClkDontExecThis(HELEMENT hBtn);
    void OnDestroy();
    void OnTimer(UINT nTimerID);

    void StartCountDown();
    // ˢ����ʾ��Ϣ
    void RefreshRemindMessage();

protected:
    int         m_nCountDown;
    LPTASK_REMINDER_PARAM	m_pRP;
    ECtrl       m_ctlCountdown;
};

// ֻ�����������߳���ʹ��
class LReminderBox
{
    SINGLETON_ON_DESTRUCTOR(LReminderBox)
    {
        RemoveAll();
    }

    class ST_RMDDLG 
    {
    public:
        ST_RMDDLG()
        {
            m_pRmdP = NULL;
            m_bClosed = FALSE;
        }
        ~ST_RMDDLG()
        {
        }

        void SetRmdParam(LPTASK_REMINDER_PARAM pRP)
        {
            m_pRmdP = pRP;

            m_Dlg.SetRmdParam(m_pRmdP);
        }

        BOOL IsClosed()const
        {
            return m_bClosed;
        }

        void SetClose( BOOL bClose )
        {
            m_bClosed = bClose;
        }

        LSingleRmdDlg* GetRmdDlg()
        {
            return &m_Dlg;
        }

        LPTASK_REMINDER_PARAM GetRmdParam()const
        {
            return m_pRmdP;
        }

        int GetTaskID()
        {
            if (NULL == m_pRmdP)
            {
                ASSERT(FALSE);
                return -1;
            }
            return m_pRmdP->nTaskID;
        }
    private:
        LSingleRmdDlg       m_Dlg;
        BOOL                m_bClosed;
        LPTASK_REMINDER_PARAM m_pRmdP;
    };

    typedef std::list<ST_RMDDLG*> RmdList;
    typedef RmdList::iterator RmdListItr;

public:
    BOOL ShowReminderDlg(LPTASK_REMINDER_PARAM pRP);
    void RemoveReminderDlg(int nTaskID);

    void RmdDlgDestroying(LSingleRmdDlg* pDlg);
    // �¼��Ķ�ʱ���ı��ˣ����Թرյ�ǰ����ʾ�Ի���
    void OnEventTimerChanged( int nEventID );
    // �¼�ɾ����

protected:
    void RemoveAll();
    // ������Ǹ��¼��ĶԻ����Ѿ��ر���
    // ������Ϊ�ѹرգ����Ҵ���ȷʵ�����ˣ����ͷ�����
    void CheckRmdList();
    
private:
    RmdList     m_lstRmd;
};
