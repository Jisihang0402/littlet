#pragma once

#include "BaseType.h"
#include "time/QTime.h"
#include "../common/ConstValues.h"
#include "../common/QDBRes.h"

// sID�����ַ���;���ָ���磺 1;2;5;8
// ��1;2;3;8 ����Ϊ�������鷵��
BOOL ParseID(const QString&sID,__out IntArray &vi);
// ������ʽ��1;2;3;8
QString MakeIDS(__in IntArray &vi);


class QDBPlan;
enum ENUM_GOAL_STATUS
{
	GOAL_STATUS_WORKING,
	GOAL_STATUS_ABORT,
	GOAL_STATUS_CHECKED,
};

enum ENUM_GOALITEM_STATUS
{
    GOALITEM_STATUS_NOTSET = -1,    // ���ڶ�ȡgoalitem��ʱ�򣬶�ȡȫ������
	GOALITEM_STATUS_WAITING = 0,  //
	GOALITEM_STATUS_GAIN,
};

class LGoalitemWidget;
class QGoalItem
{
	friend class QDBPlan;
    friend class LGoalitemWidget;

public:
    QGoalItem()
    {
        m_nID = INVALID_ID;
        m_nTimerID = INVALID_ID;
        m_nGoalID = INVALID_ID;
        m_eStatus = GOALITEM_STATUS_WAITING;
    }
    QGoalItem(const QGoalItem& g);
    QGoalItem& operator=(const QGoalItem& o);

	inline int ID()const { return m_nID; }
	inline int GoalID()const { return m_nGoalID; }
	inline QString Item()const { return m_sItem; }
	inline QString Notes()const { return m_sNotes; }
	inline QString Attachs()const { return m_sAttachs; }
	inline ENUM_GOALITEM_STATUS Status()const { return m_eStatus; }
	inline QTime BeginTime()const { return m_tmBegin; }
	inline QTime EndTime()const { return m_tmEnd; }
	inline BOOL IsDone()const { return m_eStatus == GOALITEM_STATUS_GAIN; }
    inline int TimerID()const { return m_nTimerID; }
    inline BOOL IsHasTimer()const
    {
        return (INVALID_ID != m_nTimerID) && (m_nTimerID > 0);
    }

	int GetAttachs(__out VecFileData &vfd,BOOL bIncludeData=FALSE);
	int AddNotes(int nNoteID);

public:
    BOOL Edit(const QString& sItem, QTime tmBegin, QTime tmEnd);
    // ɾ���Լ�
    BOOL Delete();

    /** ����״̬
     *	return:
     *      TRUE        �ɹ�
     *	params:
     *		-[in]
     *          bChecked    ���
    **/
    BOOL CheckIt(BOOL bChecked = TRUE);

public:
	QString		m_sItem;
	int			m_nGoalID;
    int         m_nTimerID;
	QTime		m_tmBegin;
	QTime		m_tmEnd;
	ENUM_GOALITEM_STATUS	m_eStatus;
	QString		m_sAttachs;
	QString		m_sNotes;

private:
	int			m_nID;
	QTime		m_tmCreate;
	QTime		m_tmUpdate;
};

typedef std::vector<QGoalItem*> VecGoalItem;
typedef VecGoalItem::iterator VecGoalItemItr;

// �׶�Ŀ��
class LGoalDlg;
class QGoal
{
	friend class QDBPlan;
    friend class LGoalDlg;

public:
	QGoal();
    QGoal(const QGoal& g);
    QGoal& operator=(const QGoal& o);

	inline int ID()const { return m_nID; }
	inline int StageID()const { return m_nStageID; }
	inline QString Goal()const { return m_sGoal; }
	inline QString Des()const { return m_sDes; }
	inline ENUM_GOAL_STATUS Status()const { return m_eStatus; }
	inline LONG Flag()const {  return m_nFlag; }
	inline QTime CreationTime()const { return m_tmCreate; }
	BOOL AddItem(__inout QGoalItem &gi);
	BOOL DeleteItem(QGoalItem *gi);
	// eStatus ΪNOTSET ���ȡȫ��
	BOOL GetAllItems(__out VecGoalItem &vgi,ENUM_GOALITEM_STATUS eStatus = GOALITEM_STATUS_NOTSET);
	int GetGoalItemNum();
	int GetGoalItemUnfinishedNum();
	QString TimeFlee();

public:
    BOOL EditGoal( const QString& sGoal, const QString& sDes);
    // ɾ���Լ������µ�goalitemҲ�ᱻɾ��
    BOOL Delete();
    // ɾ��goal item
    BOOL DeleteGoalitem(QGoalItem* pGoalitem);
    // ���ص����ݱ���ʹ��deleteɾ��
    QGoalItem* AddGoalItem( __in const QGoalItem &gi );
public:
	QString				m_sGoal;
	QString				m_sDes;

private:
	int					m_nID;
	int					m_nStageID;
	LONG				m_nFlag;
	ENUM_GOAL_STATUS	m_eStatus;
	QTime				m_tmCreate;
	QTime				m_tmUpdate;
};
typedef std::vector<QGoal*> VecGoal;
typedef VecGoal::iterator VecGoalItr;

// �׶�
enum ENUM_STAGE_STATUS
{
	STAGE_STATUS_WORKING,
	STAGE_STATUS_FINISH,
	STAGE_STATUS_WAITING,
	STAGE_STATUS_ABORT,
};

class LStageDlg;
class QStage
{
	friend class QDBPlan;
    friend class LStageDlg;

public:
    QStage();
	QStage(const QStage& g);
    QStage& operator=(const QStage& o);

	inline int ID()const { return m_nID; }
	inline QString Des()const { return m_sDes; }
	inline QString Stage()const { return m_sStage; }
	inline ENUM_STAGE_STATUS Status()const { return m_eStatus; }
	inline QTime BeginTime()const { return m_tmBegin; }
	inline QTime EndTime()const { return m_tmEnd; }
	inline BOOL IsFinished()const { return FALSE; }
    inline int IconID()const { return m_nIconID; }
    BOOL SetIcon(int nIconID);
    QString IconPath()const;

	int GetGoalNum();
    int GetFinishedGoalNum();
    int GetWorkingGoalNum();
	int GetKonwledgeNum(); 
	// ret:0-100
	int GetStageProcess();  // �׶ν�չ
	// ret:0-100
	int GetTimeProcess();  // �滮�����ڽ�չ
	// ret: 0��3��2��
	QString GetTimeFlee();  // �Ӽƻ���ʼ�������Ѿ���ȥ�˶��

	int GetAllGoals(__out VecGoal &vg);

public:
    // ���ص����ݱ���ʹ��deleteɾ��
    QGoal* AddGoal( __in const QGoal& g );

    // ɾ���Լ�
    BOOL Delete();

    // ɾ�������goal
    BOOL DeleteGoal(QGoal *pGoal);

    // edit
    BOOL Edit(const QString& sStage, const QString& sDes, QTime tmBegin, QTime tmEnd);

public:
	int		m_nPlanID;	// �������ڼƻ�ID
    int     m_nIconID;  // icon id
	LONG	m_nFlag;
	QString	m_sStage;		// �׶μƻ����� �磺����-��������ϰ
	QString m_sDes;		// ����������
	QTime	m_tmBegin;		// ��ʼʱ��
	QTime	m_tmEnd;		// ����ʱ��
	ENUM_STAGE_STATUS	m_eStatus;

private:
	int		m_nID;
	QTime	m_tmCreate;
	QTime	m_tmUpdate;
};

typedef std::vector<QStage*> VecStage;
typedef VecStage::iterator VecStageItr;

// �ɼ�
struct TAchievement 
{
	int			nID;
	int			nParentID;	// ����
	QString		sAchievement;	// �ı�����		
	QString		sAttachment;	// ����ID�б� ���� "2;4;35;"		
};

enum ENUM_PLAN_STATUS
{
	PLAN_STATUS_OK	= 0,	// ������
	PLAN_STATUS_PAUSE = 1,		// ��ͣ
	PLAN_STATUS_ABORT = 2,		// ����
	PLAN_STATUS_FINISH = 3,		// ���
};

class LPlanDlg;
class QPlan
{
	friend class QDBPlan;
    friend class LPlanDlg;

public:
    QPlan();
	QPlan(const QPlan& p);
	~QPlan(void);
	QPlan& operator=(const QPlan& o);

public:
	/**
	 * @brief	:	�����������б�
	 */
// 	QLongPlanTrackList* GetTaskTrackList();
// 
// 	inline BOOL IsHasFlag(UINT flag) { return (m_nFlag & flag);}
// 	inline BOOL IsAborted()const { return (m_eStatus==TASK_STATUS_ABORT);}
// 	inline BOOL IsFinished()const { return (m_eStatus==TASK_STATUS_FINISH);}
// 	inline BOOL IsPaused()const { return (m_eStatus==TASK_STATUS_PAUSE);}
// 	inline ENUM_TASK_STATUS GetStatus()const { return m_eStatus; }

	inline int ID()const { return m_nID; }
	inline int IconID()const { return m_nIconID; }
    inline QString IconPath()const;
	inline LONG Flag()const { return m_nFlag;}		// �����־
	inline QTime BeginTime()const { return m_tmBegin;}		// �������ڿ�ʼ��Чʱ��
	inline QTime EndTime()const { return m_tmEnd; }		// �������ڣ�Ԥ�ڽ���ʱ��
	inline QString Plan()const { return m_sPlan; }		// 
	inline QString Des()const { return m_sDes; }			// 
	inline QTime CreationTime()const { return m_tmCreate; }
	inline QTime UpdateTime()const { return m_tmUpdate; }
	inline QTime FinishedTime()const { return m_tmFinish; }
	inline ENUM_PLAN_STATUS Status()const { return m_eStatus; }

	int GetStageCount();
	int GetFinishStageCount();
	int GetAllStage(__out VecStage & vs);

	// ret:0-100
	int GetPlanProcess();  // �׶ν�չ
	// ret:0-100
	int GetTimeProcess();  // �滮�����ڽ�չ
	// ret: 0��3��2��
	QString GetTimeFlee();  // �Ӽƻ���ʼ�������Ѿ���ȥ�˶��

public:
    /** ���һ��stage
     *	return:
     *      QStage* ָ�룬����ʹ��deleteɾ��
     *	params:
     *		-[in]
     *          s       ����
     *
    **/
    QStage* AddStage(const QStage &s);

    // ɾ���Լ�
    BOOL Delete();

    BOOL Edit(const QString& sPlan, const QString& sDes, QTime tmBegin, QTime tmEnd);

    BOOL DeleteStage(QStage *pStage);

protected:
	// ��������
	BOOL HungupTask();
	BOOL IsRunning()const ;

public:
	LONG		m_nFlag;		// �����־
	QTime		m_tmBegin;		// �������ڿ�ʼ��Чʱ��
	QTime		m_tmEnd;		// �������ڣ�Ԥ�ڽ���ʱ��
	QString		m_sPlan;		// 
	QString		m_sDes;			// 
	int			m_nIconID;		// ������ͼ��

private:
	int			m_nID;
	QTime		m_tmCreate;		// ����ʱ��
	QTime		m_tmFinish;		// ʵ�����/������ʱ��
	QTime		m_tmUpdate;		// ����������ʱ��
	ENUM_PLAN_STATUS	m_eStatus;	// ����״̬

//	ENUM_TASK_RUNNING_STATUS	m_eRunning;	// ��������״̬
//	QLongPlanTrackList*	m_pTrackList;
};

typedef std::vector<QPlan*> VecPlan;
typedef VecPlan::iterator VecPlanItr;

class QPlanMan 
{
    SINGLETON_ON_DESTRUCTOR(QPlanMan)
    {
        ClearMem();
    }

public:
    BOOL Startup();
   
    /** ���һ���ƻ�
     *	return:
     *      TRUE        �ɹ�
     *	params:
     *		-[in]
     *          plan       �ƻ�����
     *
    **/
    QPlan* AddPlan( __in QPlan plan );

    QPlan* GetPlan(int nID);

    BOOL DeletePlan(int nID);

    inline VecPlan* GetAllPlan()
    {
        return &m_plans;
    }

    // ��ȡ��ǰ�����еļƻ���Ŀ
    int GetWorkingPlanNum();
protected:
    VecPlanItr FindPlan(int nID);

    inline VecPlanItr _End() { return m_plans.end(); }
    inline VecPlanItr _Begin() { return m_plans.begin(); }

    void ClearMem();
private:
    VecPlan     m_plans;
};