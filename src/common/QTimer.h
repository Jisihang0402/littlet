#pragma once

#include <list>
#include "exarray.h"
#include "time/QTime.h"
#include "ConstValues.h"
#include "QTimerListener.h"
#include "AppHelper.h"

// enum ENUM_AUTOTASK_EXECFLAG
// {
//     AUTOTASK_EXEC_NOTSET	= 0,	// ϵͳ����
//     // ���ʱ��
//     AUTOTASK_EXEC_AFTERSYSBOOT	= 0x00000001,	// ϵͳ����
//     AUTOTASK_EXEC_AFTERTASKSTART = 0x00000002,	// ��������
//     AUTOTASK_EXEC_AFTERMINDERSTART = 0x00000004,// ����������
//     AUTOTASK_EXEC_AFTERPROGSTART = 0x00000008,// �ⲿ��������
//     AUTOTASK_EXEC_AFTERPROGEXIT = 0x00000010,// �ⲿ�����˳�
//     // ����ʱ����
//     AUTOTASK_EXEC_ATDATE = 0x00010000,	// �������� 2011/11/11
//     AUTOTASK_EXEC_ATDAILY = 0x00020000,	// ÿ��x��
//     AUTOTASK_EXEC_ATMONTHDAY = 0x00040000,	// ÿ�µ�x��
//     AUTOTASK_EXEC_ATWEEKDAY = 0x00080000,	// ÿ�µ�x�� x[������|��һ��|����|��4��]
//
//     //////////////////////////////////////////////////////////////////////////
//     AUTOTASK_EXEC_RELATE_EXECANDTHEN = 0x01000000,	// ���ʱ��֮���ٴ�ִ�ж�μ��
//
// };
//
// // ��������ʱ״̬
// enum ENUM_AUTOTASK_RUNNING_STATUS
// {
//     AUTOTASK_RUNNING_STATUS_BADTIMER = -2,	// ���ܽ���timer���ʽ
//     AUTOTASK_RUNNING_STATUS_APPERROR = -1,	// Ӧ�ó�������˴���
//     AUTOTASK_RUNNING_STATUS_OK = 0,	// ������������
//     AUTOTASK_RUNNING_STATUS_NOTSTARTUP = 1,	// ����δ����
//     AUTOTASK_RUNNING_STATUS_PAUSED,	// ����δ����
//     AUTOTASK_RUNNING_STATUS_OVERDUE,	// ���������
//     AUTOTASK_RUNNING_STATUS_UNTILNEXTSYSREBOOT,	// ��Ҫ�´λ��������������ִ��
//     AUTOTASK_RUNNING_STATUS_UNTILNEXTMINDERREBOOT,	// ��Ҫ���������������ִ��
//     AUTOTASK_RUNNING_STATUS_BASEDONEXETERNALPROG,	// �������ⲿ����û������
//     //////////////////////////////////////////////////////////////////////////
//     // ����ʱ��
//     AUTOTASK_RUNNING_STATUS_TIMENOTMATCH,	// �޿�ִ�е�ʱ��ƥ��
//     AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC,	// ��Ȼ����δ���ڣ��������µ�ʱ�������û�л�����ִ����
// };

//
// // �������ȼ�
// enum ENUM_TASK_PRIORITY
// {
// 	TASK_PRIORITY_LOWEST = 1,	// ���
// 	TASK_PRIORITY_LOW = 2,		// ��
// 	TASK_PRIORITY_NOMAL = 3,	// ����
// 	TASK_PRIORITY_HIGH = 4,		// ��
// 	TASK_PRIORITY_HIGHEST = 5,	// ������ȼ�
// };
//

/************************************************************************/
/*
���ʱ����ʽ��
	R=1;P=20m;Q=22m;C=100;
	R: �����1��TASK_EXEC_AFTERSYSBOOT��
	P: 20minutes��ִ������ P[1s~24h]
	Q: Ȼ��ÿ��22minutesִ��һ�� Q[1s~24h]
	C: ִ��100�κ�ֹͣ��C[1~86400]

����ʱ����ʽ��
	A=d;S=d;E=d;P=d;T=d1,d2,d3...;X=s;
	
    A: ִ��ʱ���־ENUM_TASK_EXECFLAG
	
    S,E: ��ֹ���ڣ��������Ч��

    P: ����INTֵ
		������ڣ�A=TASK_EXEC_ATDAILY��Ч����λ�졣 ����ÿ��3�� P=3;

    Q: ����INTֵ
        ��ǰ������A=AUTOTASK_EXEC_ATYEARDAY��Ч����λ�죬������ǰ3��

	T: ִ��ʱ��㣬һ������INTֵ����ֵ�ɽ���Ϊ����ʱ��
		����һ���е�13��30��14��30��16��20������

	X: 1���������ֵ���ԡ�,���ָ������庬����A����
		1>A=TASK_EXEC_ATDATE, XΪ������ڵ㣬�ɽ���Ϊ����
		2>A=TASK_EXEC_ATDAILY, XΪһ������ֵ������P���������
		3>A=TASK_EXEC_ATMONTHDAY, Xһ������ֵ����Ϊ�µ��պ�
				����ֵΪ��һ���ִ�У�ʹ��λ��ʾ����32λ�����λΪ1��..
		4>A=TASK_EXEC_ATWEEKDAY, X����һ������ֵ��
				��ֵ��ʾ��һ�죨�����յ�����������ִ�У�ʹ��λ��ʶ��ʾ��ENUM_NTH_WEEKDAY
        5>A=AUTOTASK_EXEC_ATYEARDAY��XΪһ������ֵ������ [��-��] 
                ÿ���ĳ������ 2011~2020 �� [11/11]��������һ����ǰ������������������
*/
/************************************************************************/

#define MONTH_LAST_DAY 32

enum ENUM_NTH_WEEK
{
    FIRST_WEEK = 0x1,
    SECOND_WEEK = 0x2,
    THIRD_WEEK = 0x4,
    FORTH_WEEK = 0x8,
    FIVETH_WEEK = 0x10,
};

enum ENUM_NTH_WEEKDAY
{
    SUNDAY = 0x1,
    MONDAY= 0x2,
    TUESDAY	= 0x4,
    WEDNESDAY = 0x8,
    THURSDAY = 0x10,
    FRIDAY = 0x20,
    SATURDAY = 0x40,
};

enum ENUM_TIMER_FLAG
{
    TIMER_FLAG_REMINDERDISABLED = 0x00000001,	// ������ʾ��ʱ��
};

CStdString GetRunningStatusDescription(ENUM_AUTOTASK_RUNNING_STATUS eStatus);
CStdString GetExecFlagText(ENUM_AUTOTASK_EXECFLAG eFlag);

class QTimerMan;
class QTimer
{
    friend class QTimerMan;
    enum ENUM_TIMER_TYPE
    {
        TIMER_TYPE_NOTSET = 0,
        TIMER_TYPE_RELATE,
        TIMER_TYPE_ABSOLUTE,
    };
public:
    QTimer(void);
    QTimer(int nID,QTime tmBegin,QTime tmEnd,
           LPCWSTR szWhen,LPCWSTR szReminder,LPCWSTR szXFiled);
    ~QTimer(void);

    // ��������
    BOOL Update(int nTimerID, QTime tmB, QTime tmEnd, LPCWSTR szExpWhen,
                LPCWSTR szExpRemind, LPCWSTR szExpXFiled);

    inline void SetEventHandler(QTimerEventHandlerBase* pHandler)
    {
        m_pTEH = pHandler;
    }

    // ִ������ǰ����ʾ���ʽ
    // ��ʽ��
    // A=15[s|h|m];\nsound=path;\nmsg=message;\n
    // A=15[s|h|m] ִ������ǰ15[�룬�֣�ʱ]��ʾ
    // S=path; path Ϊ�����ļ�·�� ,��Ϊ����������
    // M=message; messsage Ϊ�Զ�����Ϣ����Ϊ���Զ�����Ϣ
    static BOOL ParseRemindExp( const CStdString&sExp,__out int &nA,
                                __out WCHAR&cAUnit,__out CStdString&sSound,__out CStdString&sMsg );

    BOOL GetRemindString(__out CStdString& sReminderDes);
    //----------------------------------------------------------
    // status
    // ��ʱ���Ƿ��Ѿ�����
    BOOL IsStarted()const
    {
        return m_hTimerTask != NULL;
    }
    // nTaskID �����Ϊ��Чid����ô���ֹͣʧ�ܣ�������������
    BOOL Stop(HANDLE hTimerQueue, int nTaskID = INVALID_ID);
    ENUM_AUTOTASK_RUNNING_STATUS Start( HANDLE hTimerQueue ,int nTaskID);
    // �����˴������ִ�У�ֱ�ӵ���һ��ִ��ʱ��ִ��
    ENUM_AUTOTASK_RUNNING_STATUS JumpoverThisExec(HANDLE hTimerQueue,int nTaskID);
    QTime NextExecTime()const
    {
        ASSERT(IsStarted());
        return m_tmNextExec;
    }
    // ��������Ƿ�Ϸ�����ڣ����������趨��ʱ��
    ENUM_AUTOTASK_RUNNING_STATUS TestStart( );
    BOOL IsReminderEnabled()const
    {
        return !(m_dwTimerFlag & TIMER_FLAG_REMINDERDISABLED);
    }
    BOOL EnableReminder( HANDLE hTimerQueue,int nTaskID,BOOL bEnable=TRUE );

    //----------------------------------------------------------
    BOOL IsTheYear(int y)const;
    int ID()const
    {
        return m_nID;
    }
    inline BOOL IsValid()const
    {
        return m_nID != INVALID_ID;
    }
    inline BOOL IsRelateTimer()
    {
        return m_eTimerType == TIMER_TYPE_RELATE;
    }
    inline ENUM_AUTOTASK_EXECFLAG GetExecFlag()const
    {
        return eflag_exec_;
    }

    // tmTest֮����´�ִ��ʱ��
    // tmTest �������������뼶�𽫻������Ϊ0
    ENUM_AUTOTASK_RUNNING_STATUS GetNextExecTimeFrom( __inout QTime& tmTest, __out QTime& tmExec );
    BOOL SetLifeTime(QTime tmLifeBegin,QTime tmLifeEnd);
    QTime GetLifeBegin()
    {
        return m_tmLifeBegin;
    }
    QTime GetLifeEnd()
    {
        return m_tmLifeEnd;
    }

    BOOL SetExp(const CStdString& sExp);

    int GetTimerID()const
    {
        return m_nID;
    }

protected:
    // ��ʱ��tmBegin��ʼ�趨����
    // �������ֻ����QTimer�ڲ�����
    ENUM_AUTOTASK_RUNNING_STATUS StartFrom(QTime &tmBegin,HANDLE hTimerQueue ,int nTaskID);

public: // relate
    DWORD GetExecSpan()const
    {
        return m_dwSpan;
    }
    // ����Ϊ�뷵��
    DWORD GetExecSpanSeconds()const ;
    WCHAR GetExecSpanUnit()const
    {
        return m_cSpanUnit;
    }
    DWORD GetExecSpan2()const
    {
        return m_dwSpan2;
    }
    DWORD GetExecSpanSeconds2()const ;
    WCHAR GetExecSpanUnit2()const
    {
        return m_cSpanUnit2;
    }
    DWORD GetExecCount()const
    {
        return m_iExecCount;
    }
    // ִ�е�һ�κ��Ƿ��ټ��ִ��
    BOOL IsExecSpan2()const
    {
        return (m_dwSpan2>0);
    }
    BOOL IsExecCount()const
    {
        return m_iExecCount > 0;
    }

public: // abs
    /** ����ʱ��ģ�ִ��ʱ���
     *	return:
     *      ʱ������  >= 1
     *      �������== 0
     *	params:
     *		-[out]
     *      vTimes      ʱ���
    **/
    // ִ��ʱ���
    int GetExecTimeSpot(__out std::vector<QTime>& vTimes);
    DWORD GetExecDate( );
    CStdString GetXFiled()const
    {
        return m_sXFiledExp;
    }
    CStdString GetRemindExp()const
    {
        return m_sExpRemind;
    }
    CStdString GetWhenExp()const
    {
        return m_sExpWhen;
    }
    BOOL GetWhenDoString(CStdString &sWhenDoDes);

protected:
    HANDLE GetTimerHandle()
    {
        return m_hTimerTask;
    }
    ENUM_AUTOTASK_RUNNING_STATUS AbsTime_NextExecDate(__inout DWORD& dwDate);
    ENUM_AUTOTASK_RUNNING_STATUS _AbsTime_NextRightTimeFrom(__in const QTime&tmTest,
            __in const QTime& tmExec,__inout DWORD &dwNextExecTime);
    // �������������ڼ�����ʱ��ִ��
    // BOOL IsTheDate( const QTime& d );
//     BOOL IsTheTime(WORD wTime);
//     BOOL IsTheTime(const QTime& t);
    // ��������������ʱ��ִ�еĸ�������
    // �����tmX���ҳ����ڵ���tmTest��ִ��ʱ�䣬���뵽tmExec��
    ENUM_AUTOTASK_RUNNING_STATUS _RelateTime_CheckWith( const QTime& tmX,
            const QTime& tmTest,__out QTime& tmExec);

protected:
    inline BOOL IsInArrayX(DWORD dw)const;
    void ResetAllFiled();

    BOOL ParseExp(const CStdString& sExp);
    BOOL ParseRelateExp(const CStdString& sExp);
    BOOL ParseAbsoluteExp(const CStdString& sExp);

    // sExp: r=1;b=2;c=3
    // result:
    //	sExp = b=2;c=3;
    //	sProp = r;
    //	sValue = 1;
    static BOOL _Parse(__inout CStdString&sExp ,__out WCHAR& sProp, __out CStdString& sValue);
    // 200[s|m|h]
    // 200m = 200 * 60s;  cUnit = m;
    // 200h = 200 * 3600s	cUnit = h;
    // 200s = 200s;		cUnit = s;
    BOOL _ParseSpanTime(__in const CStdString &sExp , __out WCHAR& cUnit, __out DWORD& dwSpan );

    // 2,4,8,9
    // �� Ӣ�ġ�,�� �ָ��������ַ���ת��������ֵ
    BOOL _ParseToIntArray( __inout CStdString& sExp,__out IntArray & ar );

private:
    // ������ʱ����ʱ����ã����趨��ǰ��ʾ����
    // tmExec �����ִ��ʱ��
    BOOL SetRemindTimer( HANDLE hTimerQueue ,int nTaskID,const QTime& tmExec);
    // ������ʾ��ʱ�����ʽ
    BOOL SetRemindExp(LPCWSTR pszRmdExp);
private:
    int				m_nID;	// ���ݿ��е�id
    CStdString			m_sExpWhen;		// ��ʱ��
    CStdString			m_sExpRemind;	// ��ʱ����
    ENUM_AUTOTASK_EXECFLAG	eflag_exec_;
    ENUM_TIMER_TYPE		m_eTimerType;		// ���ʱ�仹�Ǿ���ʱ������
    QTime			m_tmLifeBegin;	// �������Чʱ��
    QTime			m_tmLifeEnd;	// �������ʱ��
    CStdString			m_sXFiledExp;	//
    HANDLE			m_hTimerTask;	// ���ж�ʱ�����
    HANDLE			m_hTimerReminder;	// ��ǰ��ʾ��ʱ��
    QTime			m_tmNextExec; // �´�ִ��ʱ��
    DWORD			m_dwTimerFlag;
    TASK_REMINDER_PARAM   m_stTRP; // ������ʾ������
    QTimerEventHandlerBase*   m_pTEH;  // QTimer�������¼�����������

private:	// Relate;
    // �����ENUM_TASK_EXECFLAG��ָʾ�������
    // ���������ʱ��ִ�е�һ�����񣬵�λ����Ϊ��
    // ��ʱ������Ϊ24Сʱ֮��
    DWORD			m_dwSpan;		// ���ʱ���������
    WCHAR			m_cSpanUnit;	// ԭʼ��λs/m/h
    // ����ڵ�һ������ִ�к����ʱ��ִ��
    DWORD			m_dwSpan2;	// �ڶ���ʱ����
    WCHAR			m_cSpanUnit2;
    int				m_iExecCount;	// ��һ��ִ�к���ִ�еĴ���

private: // absolute
    //----------------------------------------
    IntArray		m_arX;		// ִ�����ڵ㣬�����ENUM_TASK_EXECFLAG����ȷ����ֵ������
    IntArray		m_arTime;	// ִ��ʱ��㣬
    //-----------------------------------------------
    // ��һ���е� [03:00 ~ 20:00] ÿ�� 30m ִ��һ��
    WORD			m_wTimeBegin;	// ִ��ʱ��
    WORD			m_wTimeEnd;
};
