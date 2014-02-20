#pragma once

#include "ui/QWindow.h"
#include "../common/QTimer.h"

/*
class QEventRemindDlg : public QDialog
{
	QUI_DECLARE_EVENT_MAP;

	typedef std::list<LPTASK_REMINDER_PARAM> RmdList;
	typedef RmdList::iterator RmdListItr;

public:
	QEventRemindDlg(void);
	~QEventRemindDlg(void);

	void AddRemind(LPTASK_REMINDER_PARAM pTRP);
	void DeleteRemind(int nTaskID);

protected:
	void OnClkPrevRemind(HELEMENT hBtn);
	void OnClkNextRemind(HELEMENT hBtn);
	void OnClkDeleteRemind(HELEMENT hBtn);
	void OnClkDontRemindAgain(HELEMENT hBtn);
	void OnClkDontExecThis(HELEMENT hBtn);

	void ShowRemind(LPTASK_REMINDER_PARAM pTRP);
	void SetRemind(LPTASK_REMINDER_PARAM pTRP);

	virtual BOOL OnDefaultButton(INT_PTR nID);

	RmdListItr	_FindRemind(int nTaskID);
	LPTASK_REMINDER_PARAM NextRemind();
	LPTASK_REMINDER_PARAM PrevRemind();

	// ����й��ڵ���ɾ��֮
	void CheckRmdList();

protected:
	RmdList		m_lstRmd;
	LPTASK_REMINDER_PARAM	m_pCur;
};
*/

enum 
{
    AUTOTASK_REMINDER_COUNTDOWN_TIMERID = 101,
};

class QSingleRmdDlg : public QDialog
{
    QUI_DECLARE_EVENT_MAP;

    BEGIN_MSG_MAP_EX(QSingleRmdDlg)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_TIMER(OnTimer)
        CHAIN_MSG_MAP(QDialog)
    END_MSG_MAP()

public:
    QSingleRmdDlg(LPTASK_REMINDER_PARAM pRp = NULL);
    ~QSingleRmdDlg(void);

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

        QSingleRmdDlg* GetRmdDlg()
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
        QSingleRmdDlg       m_Dlg;
        BOOL                m_bClosed;
        LPTASK_REMINDER_PARAM m_pRmdP;
    };

    typedef std::list<ST_RMDDLG*> RmdList;
    typedef RmdList::iterator RmdListItr;

public:
    BOOL ShowReminderDlg(LPTASK_REMINDER_PARAM pRP);
    void RemoveReminderDlg(int nTaskID);

    void RmdDlgDestroying(QSingleRmdDlg* pDlg);
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
