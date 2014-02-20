#pragma once

#include "QTimer.h"
#include "ConstValues.h"

DWORD HowManySecondsWillPendding();
VOID CALLBACK TaskCallback(__in PVOID lpParameter,__in BOOLEAN TimerOrWaitFired);



class QDBEvents;
class QWorker;
class QAutoTask
{
	friend VOID CALLBACK TaskCallback(__in  PVOID lpParameter,__in  BOOLEAN TimerOrWaitFired);

	friend class QAutoTaskMan;
	friend class QDBEvents;
    friend class QWorker;
    
public:
// 	[ID] INTEGER PRIMARY KEY AUTOINCREMENT DEFAULT (1),
// 	[TimerID] INT(4) NOT NULL,
//	[Task] TEXT NOT NULL,
// 	[What] INT(4) NOT NULL, 
// 	[Flag] INT(4) NOT NULL, 
// 	[CreateTime] DOUBLE(8) NOT NULL DEFAULT (0.0)
	QAutoTask(LPCWSTR szTask, int nID,int nTimerID,
		ENUM_AUTOTASK_DOWHAT eDo, int nFlag,QTime tmCreate );
	QString GetNextExecTimeString();
	BOOL GetNextExecTime(QTime &tmNext);
	QString GetDoWhatString();
	QString GetWhenDoString();
	QString GetLifeTimeString();

	DWORD Flag()const { return m_nFlag; }
	// �Ƿ����ö�ʱ��
	BOOL EnableReminder(BOOL bEnable=TRUE);
	BOOL IsReminderEnabled()const ;
	BOOL IsOverdue()const;
	BOOL IsStartup()const;
	BOOL IsPaused()const;
	inline QTime CreationTime()const { return m_tmCreate; }
	inline int ID()const { return m_nID; }
	inline QString Task()const { return m_sTask; }
	inline QString RemindExp()const { return m_pTimer?m_pTimer->GetRemindExp():L"";}
	inline QString TimerExp()const { return m_pTimer?m_pTimer->GetWhenExp():L"";}
	inline QTimer* GetTimer()const  { return m_pTimer; }
	inline QTime LifeBegin()const { return m_pTimer?m_pTimer->GetLifeBegin():QTime();}
	inline QTime LifeEnd()const { return m_pTimer?m_pTimer->GetLifeEnd():QTime(); }
	inline ENUM_AUTOTASK_DOWHAT GetDoWhat()const { return m_eDoWhat; }
	inline BOOL IsHasRunningFlag(int bitFlag)const { return m_nRunningFlag & bitFlag; }
	inline int GetTimerID() const 
	{ return (nullptr!=m_pTimer)?m_pTimer->ID():INVALID_ID; };

	BOOL Edit( LPCWSTR szTask,ENUM_AUTOTASK_DOWHAT eDo,int nFlag );

	// Run 
	// ���������Pause��־����Startup
	BOOL Run();

	// ��������ͣ����
	BOOL Startup();

	BOOL Pause();
	// �����˴������ִ�У�ֱ�ӵ���һ��ִ��ʱ��ִ��
	BOOL JumpoverThisExec();

	QString GetLastStartStatusDes();
	ENUM_AUTOTASK_RUNNING_STATUS GetLastStartStatus()const { return m_eLastStatus; };

	BOOL SetDoWhat( ENUM_AUTOTASK_DOWHAT eDo ,LPCWSTR szTask);
	// ������ĺ�Ķ������ݿ�
//	BOOL Update();
	ENUM_AUTOTASK_EXECFLAG GetExecFlag();
	BOOL SetTimer(const QTime&tmBegin,const QTime&tmEnd,LPCWSTR sTimerExp);
	QString GetRemindString()const;

protected:
    // ��ʱ��ִ����
    BOOL TaskFired();
	// �˺������ڽ�flag���浽���ݿ���
	BOOL FlagChanged();
private:
	int					m_nID;			// ����ID
	ENUM_AUTOTASK_DOWHAT	m_eDoWhat;			// 
	QTimer*				m_pTimer;
	ENUM_AUTOTASK_RUNNING_STATUS	m_eLastStatus; // ���һ�ε���Start��״̬
	int					m_nFlag;
	QString				m_sTask;
	QTime				m_tmCreate;

private:
	DWORD				m_nRunningFlag;	//������״̬��־
};

typedef std::list<QAutoTask*> AutoTaskList;
typedef AutoTaskList::iterator TaskListItr;

class QAutoTaskMan
{
	SINGLETON_ON_DESTRUCTOR(QAutoTaskMan){ RemoveAll();}

public:
	QAutoTaskMan();

	int GetTaskCount()const; 
	int GetOverdueTaskCount();
	BOOL DeleteTask( QAutoTask* pTask );
	QAutoTask* AddTask( LPCWSTR szTask,int nTimerID, ENUM_AUTOTASK_DOWHAT eDo,int nFlag );
	// �����ݿ��ж�ȡ����������
	QAutoTask* GetTask(int nID);
	BOOL SetTaskTimer(QAutoTask* pTask,QTimer* pTimer);

	BOOL Startup();
	void GetTaskList(AutoTaskList &lst);
	// �Ƿ������Զ��������ʾ��Ϣ
	BOOL EnableTaskReminder(int nTaskID,BOOL bEnable=TRUE);
	BOOL IsTaskReminderEnabled( int nTaskID );
	// ��������Ĵ˴�ִ�У�����һ��ִ��
	BOOL JumpoverTaskThisExec(INT nTaskID);
	// ��ȡ��ݵ�ǰ���ִ�е�����
	QAutoTask* GetMostCloseExecute();
	// Task ������
	void TaskOverdue(QAutoTask* pTask);
	// ������������
	BOOL ResetOverdueTask(QAutoTask* pTask);
    
    /** ����һ���µ��Զ�����
     *	return:
     *      QAutoTask*    �����õ�����ָ��
     *	params:
     *		-[in]
     *          
     *		-[out]
     *          sError      ������Ϣ
    **/
    QAutoTask* NewAutoTask( ENUM_AUTOTASK_DOWHAT nDoWhat, 
        const CStdStringW& sDoWhatParam, const CStdStringW& sWhenDo, 
        const CStdStringW& sRemindexp, QTime tmBegin, QTime tmEnd, 
        __out CStdStringW& sError );

protected:
	void RemoveAll();
	TaskListItr _FindTask( const QAutoTask* pTask );
	TaskListItr _FindTask( int nID );
	void OnWindowCreated(HWND hWnd);
	void OnWindowDestroyed(HWND hWnd);

private:
	AutoTaskList	m_lstTask;
};

