#pragma warning(disable:4244 4018)

#include "QTimer.h"
#include "QHelper.h"
#include "ProcessMan.h"
#include "QTimerMan.h"

static QTimerEventHandler       g_DefaultTimerEH;

// const WORD TIME_000000 = QTime(2000,)::MakeTime(0,0,0);
// const WORD TIME_235959 = QTime::MakeTime(23,59,59);
#ifdef _DEBUG
	void TraceReminderSet(const QTime& tmRemind)
	{
		QTime tmNow = QTime::GetCurrentTime();
		QString str;
		str.Format(L"��%s�� �趨�����Ѷ�ʱ������%s�� ʱ��ִ��\n",
			tmNow.Format(L"%c"),tmRemind.Format(L"%c"));
		TRACE(str);
	}
#endif

#ifdef _DEBUG
#	define TRACE_REMIND(x) TraceReminderSet(x);
#else
#	define TRACE_REMIND(x) 
#endif

VOID CALLBACK TaskCallback(__in PVOID lpParameter,__in BOOLEAN TimerOrWaitFired);
// �Զ�������ǰ��ʾ�ĵĻص�����
VOID CALLBACK TaskRemindCallback(__in PVOID lpParameter,__in BOOLEAN TimerOrWaitFired);

QString GetRunningStatusDescription( ENUM_AUTOTASK_RUNNING_STATUS eStatus )
{
	switch(eStatus)
	{
	case AUTOTASK_RUNNING_STATUS_BADTIMER://-2://	// ���ܽ���timer���ʽ
        return L"��Ч�Ķ�ʱ��";
	case AUTOTASK_RUNNING_STATUS_APPERROR://-1://	// Ӧ�ó�������˴���
        return L"Ӧ�ó������";
	case AUTOTASK_RUNNING_STATUS_OK://0://	// ������������
        return L"һ������";
	case AUTOTASK_RUNNING_STATUS_NOTSTARTUP://1://	// ����δ����
        return L"����δ����";
	case AUTOTASK_RUNNING_STATUS_PAUSED: // ��ͣ��
        return L"������ͣ";
	case AUTOTASK_RUNNING_STATUS_OVERDUE://	// ���������
        return L"�������";
	case AUTOTASK_RUNNING_STATUS_UNTILNEXTSYSREBOOT://	// ��Ҫ�´λ��������������ִ��
        return L"�´ο���ִ��";
	case AUTOTASK_RUNNING_STATUS_UNTILNEXTMINDERREBOOT:	// ��Ҫ���������������ִ��
        return L"�´�������������";
	case AUTOTASK_RUNNING_STATUS_BASEDONEXETERNALPROG://	// �������ⲿ����û������
        return L"�������ⲿ����û������";
		//////////////////////////////////////////////////////////////////////////
		// ����ʱ��
	case AUTOTASK_RUNNING_STATUS_TIMENOTMATCH://	// �޿�ִ�е�ʱ��ƥ��
        return L"�޿�ִ�е�ʱ��ƥ��";
	case AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC://	// ��Ȼ����δ���ڣ��������µ�ʱ�������û�л�����ִ����
        return L"����û�л�����ִ��";
	default: return L"δ֪��־";
	}
}

QString GetExecFlagText( ENUM_AUTOTASK_EXECFLAG eFlag )
{
	switch (eFlag)
	{
	case AUTOTASK_EXEC_NOTSET: // 0,	// δ����
		return L"δ����";
		// ���ʱ��
	case AUTOTASK_EXEC_AFTERSYSBOOT: // 0x00000001,	// ϵͳ����
		return L"ϵͳ����";
	case AUTOTASK_EXEC_AFTERTASKSTART : // 0x00000002,	// �������� 
        return L"����������";
	case AUTOTASK_EXEC_AFTERMINDERSTART : // 0x00000004,// ����������
        return L"����������";
	case AUTOTASK_EXEC_AFTERPROGSTART : // 0x00000008,// �ⲿ��������
        return L"������ⲿ��������";
	case AUTOTASK_EXEC_AFTERPROGEXIT : // 0x00000010,// �ⲿ�����˳�
        return L"������ⲿ�����˳�";
		// ����ʱ����
	case AUTOTASK_EXEC_ATDATE : // 0x00010000,	// �������� 2011/11/11
        return L"����";
	case AUTOTASK_EXEC_ATDAILY : // 0x00020000,	// ÿ��x��
        return L"ÿ��x��";
	case AUTOTASK_EXEC_ATMONTHDAY : // 0x00040000,	// ÿ�µ�x�� 
        return L"ÿ�µ�x��";
	case AUTOTASK_EXEC_ATWEEKDAY : // 0x00080000,	// ÿ�µ�x�� x[������|��һ��|����|��4��]
        return L"ÿ�µ�x��";

		//////////////////////////////////////////////////////////////////////////
	case AUTOTASK_EXEC_RELATE_EXECANDTHEN : // 0x01000000,	// ���ʱ��֮���ٴ�ִ�ж�μ��
        return L"���ʱ��֮���ٴ�ִ�ж�μ��";
	}
    ASSERT(FALSE);
	return L"GetExecFlagText->δ֪����";
}

QTimer::QTimer()
{
	ResetAllFiled();
}

QTimer::QTimer( int nID,QTime tmBegin,QTime tmEnd, 
	LPCWSTR szWhen,LPCWSTR szReminder,LPCWSTR szXFiled )
{
	ResetAllFiled();

    VERIFY(Update(nID, tmBegin, tmEnd, szWhen, szReminder, szXFiled));
}

QTimer::~QTimer(void)
{
    ASSERT(!IsStarted());
}

// �˺���ִ���������������Ժ���
// ������ֽ�ϻ�һ�������꣬�����3��ʱ���������˼·����ʾʱ�䣬����ִ��ʱ�䣬����ʱ��
// �ƶ�����ʱ���������
BOOL QTimer::SetRemindTimer( HANDLE hTimerQueue ,int nTaskID,const QTime& tmExec)
{
    QTime tmNow = QTime::GetCurrentTime();
    if ((tmExec - tmNow).GetTotalSeconds() < 5) 
    {
        // ��������ִ��ʱ��̫���ˣ��Ͳ���ʾ�ˡ�
        return FALSE;
    }

    WCHAR cUnit;
    int nA;
    QString sSound,sMsg;
    if (!QTimer::ParseRemindExp(m_sExpRemind,nA,cUnit,sSound,sMsg))
    {
        return FALSE;
    }

    m_stTRP.nSeconds = QHelper::HowManySeconds(nA,cUnit);
    ASSERT(m_stTRP.nSeconds > 0);
    m_stTRP.sSound = sSound;
    m_stTRP.sMsg = sMsg;
    m_stTRP.nTaskID = nTaskID;
    m_stTRP.tmExec = tmExec;

    // ��ʱӦ����ʾʱ��
    QTime tmRemind = tmExec - QTimeSpan( (m_stTRP.nSeconds) / SECONDS_OF_DAY );
    if ( tmNow > tmRemind )
    {   // �Ѿ�������ʾʱ����
        // Ӧ��������ʾ
        m_stTRP.nSeconds = (tmExec - tmNow).GetTotalSeconds();   // ��������ִ��ʱ���ж�����
        if (NULL != m_pTEH)
        {
            m_pTEH->OnTimerReminderSetted(&m_stTRP);
        }
        return TRUE;
    }

    // ��û�е���ʾʱ�� 
    // ���廹�ж�����ȥ��ʾ�أ�
    DWORD dwSecToRemind = (tmRemind - tmNow).GetTotalSeconds();
    if (dwSecToRemind < 5)
    {   // ������ʾʱ�仹��5�룬̫������ֱ����ʾ��ʾ���
        // ���ǵ���ʱӦ�ü������ʱ��
        m_stTRP.nSeconds += dwSecToRemind;
        if (NULL != m_pTEH)
        {
            m_pTEH->OnTimerReminderSetted(&m_stTRP);
        }
        return TRUE;
    }

    m_stTRP.nSeconds = QHelper::HowManySeconds(nA,cUnit);
    // ����������ʾ��ʱ�仹�㹻������ô��Ҫ����һ����ʱ���ص�����
    // ���ص�����ʱ���򴰿ڷ���Ϣ��֪ͨ��ʾ��ʾ����
    // �������ζ�ʱ����ִ����ʾ�ص�
    return CreateTimerQueueTimer(&m_hTimerReminder,hTimerQueue,
            TaskRemindCallback,(PVOID)&m_stTRP, dwSecToRemind * 1000,
            0, WT_EXECUTEDEFAULT); // ���ζ�ʱ��
}

ENUM_AUTOTASK_RUNNING_STATUS QTimer::Start(HANDLE hTimerQueue ,int nTaskID)
{
	return StartFrom(QTime::GetCurrentTime(), hTimerQueue, nTaskID);
}

ENUM_AUTOTASK_RUNNING_STATUS QTimer::StartFrom(QTime &tmBegin,HANDLE hTimerQueue ,int nTaskID)
{
	if (IsStarted())
	{
		return AUTOTASK_RUNNING_STATUS_OK;
	}
	double lHowLongToExec;
	ENUM_AUTOTASK_RUNNING_STATUS eStatus;
	while (true)
	{
		if ((eStatus = GetNextExecTimeFrom(tmBegin,m_tmNextExec))
			!= AUTOTASK_RUNNING_STATUS_OK)
		{
			break;
		}
		lHowLongToExec = (m_tmNextExec-QTime::GetCurrentTime()).GetTotalSeconds(); 
		if (lHowLongToExec <= 3.0f) // �����ִ��ʱ��ֻ��3�룬������´�ִ��ʱ���
		{
			tmBegin = m_tmNextExec + QTimeSpan( 1 / SECONDS_OF_DAY );
			continue;
		}
		BOOL bOK = CreateTimerQueueTimer(&m_hTimerTask, hTimerQueue,TaskCallback,
			(PVOID)nTaskID,lHowLongToExec * 1000,0,WT_EXECUTEDEFAULT);
		if (!bOK)
		{
			return AUTOTASK_RUNNING_STATUS_APPERROR;
		}

		// ��ǰ����
		if (IsReminderEnabled())
		{
			SetRemindTimer(hTimerQueue,nTaskID,m_tmNextExec);
		}
		break;
	}
	return eStatus;
}

ENUM_AUTOTASK_RUNNING_STATUS QTimer::TestStart()
{
	double lHowLongToExec;
	QTime tmBegin = QTime::GetCurrentTime();
	ENUM_AUTOTASK_RUNNING_STATUS eStatus;
	while (true)
	{
		if ((eStatus = GetNextExecTimeFrom(tmBegin,m_tmNextExec))
			!= AUTOTASK_RUNNING_STATUS_OK)
		{
			break;
		}
		lHowLongToExec = (m_tmNextExec-tmBegin).GetTotalSeconds(); 
		if (lHowLongToExec <= 3.0f) // �����ִ��ʱ��ֻ��3�룬������´�ִ��ʱ���
		{
			tmBegin = m_tmNextExec + QTimeSpan( 1 / SECONDS_OF_DAY );
			continue;
		}
		break;
	}
	return eStatus;
}

ENUM_AUTOTASK_RUNNING_STATUS QTimer::JumpoverThisExec(HANDLE hTimerQueue,int nTaskID)
{
	if (!Stop(hTimerQueue, nTaskID))
	{
		return AUTOTASK_RUNNING_STATUS_APPERROR;
	}
    // �ӵ�ǰִ���������һ��30�뿪ʼ�����´ε�ִ��ʱ��
	return StartFrom(m_tmNextExec + QTimeSpan((DWORD)30),hTimerQueue,nTaskID);

    //return AUTOTASK_RUNNING_STATUS_OK;
}

BOOL QTimer::Stop(HANDLE hTimerQueue, int nTaskID)
{
	if (IsStarted())
    {
        BOOL bHasReminder = FALSE;
        // Sleep(100);	// ���߳�ʱ���ó���������ʱ������ִ����ɡ�
        if (m_hTimerReminder != NULL)
        {
            if (!DeleteTimerQueueTimer(hTimerQueue,m_hTimerReminder,INVALID_HANDLE_VALUE))
            {
                ASSERT(FALSE);
                return FALSE;
            }
            m_hTimerReminder = NULL;
            bHasReminder = TRUE;
        }
        if (NULL != m_hTimerTask)
        {
            // �ȴ���ֱ��ɾ��
            if (!DeleteTimerQueueTimer(hTimerQueue,m_hTimerTask,INVALID_HANDLE_VALUE))
            {
                if (bHasReminder && (INVALID_ID != nTaskID))
                {
                    SetRemindTimer(hTimerQueue, nTaskID, NextExecTime());
                }
                ASSERT(FALSE);
                return FALSE;
            }
            m_hTimerTask = NULL;
        }
    }
    return TRUE;
}

BOOL QTimer::SetLifeTime(QTime tmLifeBegin,QTime tmLifeEnd)
{
    if (tmLifeEnd <= tmLifeBegin)
    {
        return FALSE;
    }
	m_tmLifeBegin = tmLifeBegin;
	m_tmLifeEnd = tmLifeEnd;
	return TRUE;
}

void QTimer::ResetAllFiled()
{
	m_hTimerTask = NULL;
	m_hTimerReminder = NULL;
    m_pTEH = &g_DefaultTimerEH;

	m_nID = INVALID_ID;
	m_eExecFlag = AUTOTASK_EXEC_NOTSET;
	m_eTimerType = TIMER_TYPE_NOTSET;
	m_dwTimerFlag = 0;

	m_sXFiledExp = L"";

	m_dwSpan = 0;
	m_cSpanUnit = L'';
	m_dwSpan2 = 0;	// �ڶ���ʱ����
	m_cSpanUnit2 = L''; 
	m_iExecCount = 0;	// ִ�д���

	m_arX.clear();
	m_arTime.clear();
	m_wTimeBegin = 0;
	m_wTimeEnd = 0;
	m_dwSpan = 0; // ʱ���� ����λs
}

BOOL QTimer::ParseExp( const QString& sExp )
{
	if (sExp.IsEmpty())
		return FALSE;
	switch(sExp.GetAt(0))
	{
	case L'R':
		return ParseRelateExp(sExp);
	case L'A':
		return ParseAbsoluteExp(sExp);
	}
	return FALSE;
}

BOOL QTimer::ParseAbsoluteExp( const QString& sExp )
{
	if (sExp.IsEmpty())
		return FALSE;
	QString sExpTest = sExp, sValue;
	WCHAR cProp;
	while (!sExpTest.IsEmpty())
	{
		if (!_Parse(sExpTest,cProp,sValue))
			return FALSE;
		switch (cProp)
		{
		case L'A':
			{
				m_eTimerType = TIMER_TYPE_ABSOLUTE;
				m_eExecFlag = (ENUM_AUTOTASK_EXECFLAG)StrToInt(sValue);
				break;
			}
		case L'X':
			{
				if (!_ParseToIntArray(sValue,m_arX))
					return FALSE;
				std::stable_sort(_BeginItr(m_arX),_EndItr(m_arX));
				break;
			}
		case L'S': // ʱ��
			{
				m_wTimeBegin = StrToInt(sValue);
				break;
			}
		case L'E': // ʱ��
			{
				m_wTimeEnd = StrToInt(sValue);
				break;
			}
		case L'P': // ���ʱ��ִ��
			{
				if (!_ParseToIntArray(sValue,m_arX))
					return FALSE;
				if (m_arX[0] < 1)
					return FALSE;
				break;
			}
		case L'T': // ʱ���ִ��
			{
				if (!_ParseToIntArray(sValue,m_arTime))
					return FALSE;
// #ifdef _DEBUG
// 				QTime tmT;
// 				for (int i = 0; i < m_arTime.size(); i++)
// 				{
// 					tmT = QTime::ParseTime(m_arTime[i]);
// 					TRACE(tmT.Format(L"%H:%M:%S\n"));
// 				}
// #endif
				// ��С��������
				std::stable_sort(_BeginItr(m_arTime),_EndItr(m_arTime));
				break;
			}
		default: return FALSE;
		}
	}
	return TRUE;
}

BOOL QTimer::ParseRemindExp( const QString&sExp,__out int &nA, 
    __out WCHAR&cAUnit,__out QString&sSound,__out QString&sMsg )
{
    WCHAR cProp;
    QString sPart,sValue,sTemp = sExp;
    int idx;
    while (true)
    {
        idx = sTemp.Find(L";\n");
        if (-1 == idx)
            return FALSE;
        sPart = sTemp.Left(idx + 1);
        sTemp = sTemp.Mid(idx + 2);
        if (!_Parse(sPart,cProp,sValue))
            return FALSE;
        switch (cProp)
        {
        case L'A':
            {
                if ( !QHelper::ParseUnitTime(sValue, nA, cAUnit) || (nA <= 0))
                    return FALSE;
                break;
            }
        case L'S':	// sound
            {
                sSound = sValue;
                break;
            }
        case L'M':
            {
                sMsg = sValue;
                break;
            }
        default:return FALSE;
        }
        if (sTemp.IsEmpty())
            break;
    }
    return TRUE;
}

BOOL QTimer::ParseRelateExp( const QString& sExp )
{
	if (sExp.IsEmpty())
		return FALSE;
	QString sExpTest = sExp;
	WCHAR cProp;
	QString sValue;
	while (!sExpTest.IsEmpty())
	{
		if (!_Parse(sExpTest,cProp,sValue))
			return FALSE;
		switch (cProp)
		{
		case L'R':
			{
				m_eTimerType = TIMER_TYPE_RELATE;
				m_eExecFlag = (ENUM_AUTOTASK_EXECFLAG)StrToInt(sValue);
				break;
			}
		case L'P':
			{
				if (!_ParseSpanTime(sValue,m_cSpanUnit,m_dwSpan))
					return FALSE;
				break;
			}
		case L'Q': // �ڶ���ʱ����
			{
				if (!_ParseSpanTime(sValue,m_cSpanUnit2,m_dwSpan2))
					return FALSE;
				break;
			}
		case L'C': // ִ�д���
			{
				m_iExecCount = StrToInt(sValue);
				break;
			}
		default: return FALSE;
		}
	}
	return TRUE;
}

BOOL QTimer::_Parse( __inout QString&sExp ,
	__out WCHAR& cProp, __out QString& sValue )
{
	int idx = sExp.Find(L'=');
	if ((-1 == idx) || (idx != 1) || ((idx = sExp.Find(L';')) == -1))
		return FALSE;
	cProp = sExp[0];
	sValue = sExp.Mid(2,idx-2);
	sExp = sExp.Mid(idx + 1);	
	return TRUE;
}

BOOL QTimer::_ParseSpanTime( __in const QString &sExp , 
		__out WCHAR& cUnit, __out DWORD& dwSpan )
{
	int len = sExp.GetLength();
	cUnit = tolower(sExp.back());
	dwSpan = StrToInt(sExp.Mid(0,len-1));
	switch(cUnit)
	{
	case L'm':case L'M':case L's':case L'S':case L'H':case L'h':
		return TRUE;
	}
	ASSERT(FALSE);
	return FALSE;
}

BOOL QTimer::_ParseToIntArray( __inout QString& sExp,__out IntArray & ar )
{
	ar.clear();
	int idx;
	while (!sExp.IsEmpty())
	{
		idx = sExp.Find(L',');
		if (idx != -1)
		{
			ar.push_back(StrToInt(sExp.Mid(0,idx)));
			sExp = sExp.Mid(idx+1);
		}
		else
		{
			ar.push_back(StrToInt(sExp));
			break;
		}
	}
	return ar.size();
}

BOOL QTimer::IsTheDate( const QTime& d )
{
	ASSERT(!IsRelateTimer());
	
	switch (m_eExecFlag)
	{
	case AUTOTASK_EXEC_ATDATE:	// = 0x00010000,	// �������� 2011/11/11
		{
			return m_arX.contain(d.MakeDate());
		}
	case AUTOTASK_EXEC_ATDAILY:	// = 0x00020000,	// ÿx��,
		{
			if (m_arX.size())
			{
				QTime t2 = d;
				t2.SetTime(m_tmLifeBegin.GetHour(),m_tmLifeBegin.GetMinute(),0);
				// ����Ϊ0��ִ��
				return !((DWORD)((t2 - m_tmLifeBegin).GetTotalDays()) % (m_arX[0])); 
			}
			ASSERT(FALSE); return FALSE;
		}
	case AUTOTASK_EXEC_ATMONTHDAY:	// = 0x00040000,	// ÿ�µ�x�� 
		{	
			if (m_arX.size())
			{
				return (m_arX[0] & (0x1<<(d.GetDay()-1)));
			}
			ASSERT(FALSE);return FALSE;
		}
	case AUTOTASK_EXEC_ATWEEKDAY:	// = 0x00080000,	// ÿ�µ�x�� x[������|��һ��|����|��4��]
		{	
			if (m_arX.size())
			{// 1 - sunday , 7 - saturday
				return ((m_arX[0]) & (0x1<<(d.GetDayOfWeek()-1)));
			}
			ASSERT(FALSE); return FALSE;
		}
	default: { ASSERT(FALSE);  return FALSE; }
	}
	return FALSE;
}

BOOL QTimer::IsTheTime(WORD wTime)
{
	ASSERT(!IsRelateTimer());
// 	if ((m_wTimeBegin > wTime) || (m_wTimeEnd < wTime) )
// 		return FALSE;
	return m_arTime.contain(wTime);
}

ENUM_AUTOTASK_RUNNING_STATUS QTimer::_RelateTime_CheckWith(
	const QTime& tmX,const QTime& tmTest,__out QTime& tmExec)
{
	if (tmTest >= m_tmLifeEnd)
	{
		return AUTOTASK_RUNNING_STATUS_OVERDUE;
	}
	QTime tmFirstExec = tmX + QTimeSpan(GetExecSpanSeconds()/SECONDS_OF_DAY);
	if (tmFirstExec <= tmTest) // ����Ҳ���Ǵ����ִ��ʱ��
	{	// ����˵�һ��ִ��ʱ��
		if (!IsExecSpan2())
		{ // �����һ��ִ��ʱ�䣬���ҷǶ��ִ��
			if (AUTOTASK_EXEC_AFTERSYSBOOT == m_eExecFlag)
				return AUTOTASK_RUNNING_STATUS_UNTILNEXTSYSREBOOT; // �ȴ�ϵͳ����
			else if (AUTOTASK_EXEC_AFTERMINDERSTART == m_eExecFlag) 
				return AUTOTASK_RUNNING_STATUS_UNTILNEXTMINDERREBOOT; // �ȴ���������
			else // �Ƕ�ο�ִ�У�����
				return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC; 
		}
		// ��μ��ִ��
		// �Դӵ�һ�ο�ִ��ʱ�䵽tmTest�Ѿ���ȥ�˶೤ʱ��
		double dTotalSeconds = (tmTest - tmFirstExec).GetTotalSeconds(); // 
		// �ڹ�ȥ����ô��ʱ�������ִ�ж��ٴΣ�
		DWORD dwExec = dTotalSeconds / GetExecSpanSeconds2(); //ִ�д���
		if (IsExecCount() && (dwExec >= m_iExecCount))
		{ // ��ִ�д����Ѿ��������ܹ���Ҫִ�еĴ���
			if (AUTOTASK_EXEC_AFTERSYSBOOT == m_eExecFlag)
				return AUTOTASK_RUNNING_STATUS_UNTILNEXTSYSREBOOT; // �ȴ�ϵͳ����
			else if (AUTOTASK_EXEC_AFTERMINDERSTART == m_eExecFlag) 
				return AUTOTASK_RUNNING_STATUS_UNTILNEXTMINDERREBOOT; // �ȴ���������
			else // �Ƕ�ο�ִ�У�����
				return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC; 
		}
		else
		{ // ��ִ�д�����û�г������ܹ���Ҫִ�еĴ���
			tmExec = tmFirstExec + QTimeSpan(((dwExec + 1) * GetExecSpanSeconds2()) / SECONDS_OF_DAY);
			if (tmExec >= m_tmLifeEnd)
			{
				return AUTOTASK_RUNNING_STATUS_OVERDUE;
			}
			return AUTOTASK_RUNNING_STATUS_OK;
		}
	}
	else
	{
		tmExec = tmFirstExec;
		return AUTOTASK_RUNNING_STATUS_OK;
	}
}

// tmTest �������������뼶�𽫻������Ϊ0
ENUM_AUTOTASK_RUNNING_STATUS QTimer::GetNextExecTimeFrom( 
	__inout QTime& tmTest,
	__out QTime& tmExec )
{
	if (tmTest >= m_tmLifeEnd)
		return AUTOTASK_RUNNING_STATUS_OVERDUE;	// ����
	switch (m_eExecFlag)
	{
	//////////////////////////////////////////////////////////////////////////
	// ���ʱ��
	case AUTOTASK_EXEC_AFTERSYSBOOT:	//= 0x00000001,	// ϵͳ����
		{
			return _RelateTime_CheckWith(QProcessMan::GetSystemStartupTime(),tmTest,tmExec);
		}
	case AUTOTASK_EXEC_AFTERMINDERSTART:	// = 0x00000004,// ����������
		{
			return _RelateTime_CheckWith(QProcessMan::GetCurrentProcessStartupTime(),tmTest,tmExec);
		}
/*	case TASK_EXEC_AFTERPROGSTART:	// = 0x00000008,// �ⲿ��������
		{
			QTime tmProgStart;
			if (QProcessMgr::IsExeRun(m_sXFiledExp,tmProgStart))
			{
				return _RelateTime_CheckWith(tmProgStart,tmTest,tmExec);
			}
			return TASK_RUNNING_STATUS_BASEDONEXETERNALPROG;
		}
	case TASK_EXEC_AFTERPROGEXIT:	// = 0x00000010,// �ⲿ�����˳�
		{ 
			return TASK_RUNNING_STATUS_BASEDONEXETERNALPROG;
		}
*/	case AUTOTASK_EXEC_AFTERTASKSTART:	// = 0x00000002,	// �������� 
		{
			return _RelateTime_CheckWith(m_tmLifeBegin,tmTest,tmExec);
		}
	//////////////////////////////////////////////////////////////////////////
	// ����ʱ��
	case AUTOTASK_EXEC_ATDATE:	// = 0x00010000,	// �������� 2011/11/11
	case AUTOTASK_EXEC_ATDAILY:	// = 0x00020000,	// ÿx��,
	case AUTOTASK_EXEC_ATMONTHDAY:	// = 0x00040000,	// ÿ�µ�x�� 
	case AUTOTASK_EXEC_ATWEEKDAY:	// = 0x00080000,	// ÿ�µ�x�� x[������|��һ��|����|��4��]
		{
			DWORD dwNextExecDate,dwNextExecTime;
			ENUM_AUTOTASK_RUNNING_STATUS eStatus;
			QTime tmTempTest = tmExec = tmTest;
			while(true)
			{
				dwNextExecDate = tmExec.MakeDate();
				eStatus = AbsTime_NextExecDate(dwNextExecDate); // ִ������
				if (AUTOTASK_RUNNING_STATUS_OK == eStatus)
				{
					tmExec = QTime::ParseDate(dwNextExecDate);
					eStatus = _AbsTime_NextRightTimeFrom(tmTempTest,tmExec,dwNextExecTime);
					if (AUTOTASK_RUNNING_STATUS_OK == eStatus)
					{ // ִ��ʱ��
						tmExec = QTime::CombineTime(dwNextExecDate,dwNextExecTime);
						if (tmExec > m_tmLifeEnd) // ������ϳɵ�ʱ���Ƿ񳬹�����������
							return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;
						else
							return AUTOTASK_RUNNING_STATUS_OK;
					}
					else if (AUTOTASK_RUNNING_STATUS_TIMENOTMATCH == eStatus)
					{ // ˵����ǰ���Ե������ǲ�����ִ�еģ�ֻ�ܱȽ����������ִ��
						// ��ʱʱ���������С��
						tmExec += QTimeSpan(1,0,0,0);
						tmExec.SetTime(0,0,0);
						tmTempTest.SetTime(0,0,0);
						continue;
					}
					else
					{
						return eStatus;
					}
				}
				return eStatus;
			}
		}
	default:
		{
			ASSERT(FALSE);
			return AUTOTASK_RUNNING_STATUS_BADTIMER;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// AtDate
ENUM_AUTOTASK_RUNNING_STATUS QTimer::AbsTime_NextExecDate(__inout DWORD& dwDate)
{
	if (dwDate > m_tmLifeEnd.MakeDate())
		return AUTOTASK_RUNNING_STATUS_OVERDUE;	// ����
	if (m_arX.size() < 1)
		return AUTOTASK_RUNNING_STATUS_BADTIMER;
	switch (m_eExecFlag)
	{
	case AUTOTASK_EXEC_ATDATE:
		{
			int idx = m_arX.find_first_lgoreq(dwDate);
			if (-1 != idx)
			{
				dwDate = m_arX[idx];
				return AUTOTASK_RUNNING_STATUS_OK;
			}
			return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC; // �޻�����ִ��
		}
	case AUTOTASK_EXEC_ATDAILY:
		{
			ASSERT(m_arX.size() == 1);
			QTimeSpan tsDate = QTime::ParseDate(dwDate) - QTime::ParseDate(m_tmLifeBegin.MakeDate());
			DWORD xDay = (DWORD)(tsDate.GetTotalDays()) % m_arX[0]; // ���м����´�ִ��
			if (xDay > 0)
			{
				QTime tmNextExecDate = QTime::ParseDate(dwDate) + QTimeSpan(xDay,0,0,0);
				if (tmNextExecDate.CompareDate(m_tmLifeEnd) > 0) // ���������ʱ�����������֮��
					return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;	// �޻���ִ���ˡ�
				dwDate = tmNextExecDate.MakeDate();
				return AUTOTASK_RUNNING_STATUS_OK;
			}
			else if (0 == xDay) //dwDate ����Ϊִ������
			{
				return AUTOTASK_RUNNING_STATUS_OK;
			}
			// ��Ӧ��ִ�е����
			return AUTOTASK_RUNNING_STATUS_APPERROR;
		}
	case AUTOTASK_EXEC_ATWEEKDAY:
		{ // ����xִ��
			ASSERT(m_arX.size() == 1); 
			if (0 == m_arX[0]) // ����������һ���������ǿ���ִ�е�
				return AUTOTASK_RUNNING_STATUS_BADTIMER;
			QTime tmTest = QTime::ParseDate(dwDate);
			for (int iTestCount = 0; iTestCount < 7; iTestCount++)
			{// ����7��֮�ڵ�ִ�����,
				if (m_arX[0] & (0x01<<(tmTest.GetDayOfWeek()-1))) // 0-sunday,1-monday...6-saturday
				{
					dwDate = tmTest.MakeDate();
					return AUTOTASK_RUNNING_STATUS_OK;
				}
				tmTest += QTimeSpan(1,0,0,0);
				if (tmTest.CompareDate(m_tmLifeEnd) > 0)
				{
					return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;
				}
			}
			// ��Ӧ��ִ�е����,��ǰ�������һ��֮�ڱ���һ���ǿ���ִ�е�
			return AUTOTASK_RUNNING_STATUS_APPERROR;
		}
	case AUTOTASK_EXEC_ATMONTHDAY:
		{
			ASSERT(m_arX.size() == 1); 
			if (0 == m_arX[0]) // ����������һ���������ǿ���ִ�е�
				return AUTOTASK_RUNNING_STATUS_BADTIMER;
			QTime tmTest = QTime::ParseDate(dwDate);
			for (int iTestCount = 0; iTestCount < 31; iTestCount++)
			{
				if (m_arX[0] & (0x1 << (tmTest.GetDay())))
				{
					dwDate = tmTest.MakeDate();
					return AUTOTASK_RUNNING_STATUS_OK;
				}
				tmTest += QTimeSpan(1,0,0,0); // ��һ��
				if (tmTest.CompareDate(m_tmLifeEnd) > 0)
				{
					return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;
				}
			}
			// ��Ӧ��ִ�е��������Ϊ��31��֮�ڣ�����һ���ǿ���ִ�е�
			return AUTOTASK_RUNNING_STATUS_APPERROR;
		}
	default:
		{
			ASSERT(FALSE);
			return AUTOTASK_RUNNING_STATUS_APPERROR;
		}
	}
	return AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC;
}

// ���tmExec�����ڴ���tmTest�����ڣ�tmNext�趨Ϊm_arTime[0],����TASK_RUNNING_STATUS_OK
// ���tmExec�����ڵ���tmTest�����ڣ�tmNext�趨Ϊ���ڵ���tmTest.MakeTime()��ֵ��
//		���m_arTime�а���������ʱ�䣬����TASK_RUNNING_STATUS_OK
//		���������������ʱ�䣬����TASK_RUNNING_STATUS_TIMENOTMATCH
// ���tmExec������С��tmTest�����ڣ������߼�������������TASK_RUNNING_STATUS_OVERDUE
ENUM_AUTOTASK_RUNNING_STATUS QTimer::_AbsTime_NextRightTimeFrom(__in const QTime&tmTest,
			__in const QTime& tmExec,__inout DWORD &dwNextExecTime)
{
	ASSERT(!IsRelateTimer());
	ASSERT(m_arTime.size());
	if (m_arTime.size())
	{ // ����ʱ��ֵ
		int iCmp = tmExec.CompareDate(tmTest);
		if ( iCmp > 0)
		{
			dwNextExecTime = m_arTime[0];
			return AUTOTASK_RUNNING_STATUS_OK;			
		}
		else if ( 0 == iCmp)
		{
			int idx = m_arTime.find_first_lgoreq(tmTest.MakeTime());
			if (-1 != idx)
			{
				dwNextExecTime = m_arTime[idx];
				return AUTOTASK_RUNNING_STATUS_OK;
			}
			return AUTOTASK_RUNNING_STATUS_TIMENOTMATCH;
		}
		else
		{
			return AUTOTASK_RUNNING_STATUS_OVERDUE;
		}
	}
	// �����Ӧ�ñ�ִ�е�
	ASSERT(FALSE); 
	return AUTOTASK_RUNNING_STATUS_BADTIMER;
}

BOOL QTimer::SetExp( const QString& sExp )
{
	if (sExp.IsEmpty())
	{
		ASSERT(FALSE);
		return FALSE;
	}
	m_sExpWhen = sExp;
	return ParseExp(sExp);
}

BOOL QTimer::Update(int nTimerID, QTime tmB, QTime tmEnd, LPCWSTR szExpWhen, 
    LPCWSTR szExpRemind, LPCWSTR szExpXFiled)
{
	ASSERT(!IsStarted());
    m_nID = nTimerID;
    m_sXFiledExp = szExpXFiled;
    if ( !SetLifeTime(tmB, tmEnd) )
        return FALSE;

    if (!SetExp(szExpWhen) )
        return FALSE;

    if (!SetRemindExp(szExpRemind))
        return FALSE;

    return TRUE;
}

BOOL QTimer::SetRemindExp(LPCWSTR pszRmdExp)
{
	if (IsStarted())
	{
		ASSERT(FALSE);
		return FALSE;
	}
	m_sExpRemind = pszRmdExp;
    return TRUE;
}

int QTimer::GetExecTimeSpot(__out std::vector<QTime>& vTimes)
{
    if (IsRelateTimer())
    {
        ASSERT(FALSE);
        return 0;
    }
    vTimes.clear();
    for (int i = 0; i < m_arTime.size(); ++i)
    {
        vTimes.push_back(QTime::ParseTime(m_arTime[i]));
    }
    ASSERT(vTimes.size());      // ����Ҫ��һ��ִ��ʱ���
    return vTimes.size();
}

DWORD QTimer::GetExecDate(  )
{
	ASSERT(!IsRelateTimer());
	if (m_arX.size())
	{
		return m_arX[0];
	}
	return 0;
}

BOOL QTimer::GetRemindString(__out QString& sReminderDes)
{
	int nA;
	WCHAR cUnit;
	QString sSound,sMsg;
	BOOL bOK = QTimer::ParseRemindExp(m_sExpRemind,nA,cUnit,sSound,sMsg);
	if (bOK)
	{
		sReminderDes.Format(
            L"<b .back>��ִ��ǰ:</b> <b .yellow>%d %s</b><br/>"
            L"<b .back>��������:</b>%s<br/><b .back>��ʾ��Ϣ:</b>%s",
            nA, QHelper::GetTimeUnitString(cUnit),
            sSound.IsEmpty() ? L"��" : sSound,
            sMsg.IsEmpty() ? L"��" : sMsg);
        return TRUE;
	}
	return FALSE;
}

DWORD QTimer::GetExecSpanSeconds() const
{
	return QHelper::HowManySeconds(m_dwSpan,m_cSpanUnit);
}

DWORD QTimer::GetExecSpanSeconds2() const
{
	return QHelper::HowManySeconds(m_dwSpan2,m_cSpanUnit2);
}

BOOL QTimer::GetWhenDoString(QString &sWhenDoDes)
{
	QString sPart;
	if (IsRelateTimer())
	{
		// within [a~b]
// 		sRet.Format(QGetTextString(L"4"),
// 			m_tmLifeBegin.Format(L"%Y/%m/%d %H:%M"),
// 			m_tmLifeEnd.Format(L"%Y/%m/%d %H:%M"));
		// after
		sWhenDoDes.Format(L"��<b .yellow>[%s] [%d][%s]</b>֮��",
			GetExecFlagText(m_eExecFlag),
			GetExecSpan(),
			QHelper::GetTimeUnitString(GetExecSpanUnit()));
		// then every
		if (IsExecSpan2())
		{
			sPart.Format(L"Ȼ��ÿ <b .yellow>[%d][%s]</b>ִ��",
				GetExecSpan2(),
				QHelper::GetTimeUnitString(GetExecSpanUnit2()));
			sWhenDoDes += L"<br/>" + sPart;
			// after x times stop
			if (IsExecCount())
			{
				sPart.Format(L"�� <b .yellow>[%d]</b> �κ�ֹͣ",
					GetExecCount());
				sWhenDoDes += L"<br />" + sPart;
			}
		}
	}
	else
	{
		QTime tmTest;
        for (int i = 0; i < m_arTime.size(); ++i)
        {
            tmTest = QTime::ParseTime(m_arTime[i]);
            sPart.AppendFormat(tmTest.Format(L"%H:%M:%S,"));
        }
        sPart.TrimRight(L',');
		switch (m_eExecFlag)
		{
		case AUTOTASK_EXEC_ATDATE:
			{
				tmTest = QTime::ParseDate(m_arX[0]);
				sWhenDoDes.Format(L"�� <b .yellow>[%s] [%s]</b>",
                    tmTest.Format(L"%Y/%m/%d"),sPart);
				break;
			}
		case AUTOTASK_EXEC_ATDAILY:
			{
				ASSERT(m_arX.size() == 1);
				sWhenDoDes.Format(L"ÿ <b .yellow>[%d]</b> ��� <b .yellow>[%s]</b>",
                    m_arX[0],sPart);
				break;
			}
		case AUTOTASK_EXEC_ATWEEKDAY:
			{ // ����xִ��
				ASSERT(m_arX.size() == 1); 
				QString sWeekdays,sTemp;
				for (int iWeekday = 0; iWeekday < 7; iWeekday++)
				{// ����7��֮�ڵ�ִ�����,
					if (m_arX[0] & (0x01<<iWeekday)) // 0-sunday,1-monday...6-saturday
					{
						sTemp.Format(L"%d,",iWeekday);
						sWeekdays += sTemp;
					}
				}
				sWeekdays = sWeekdays.Left(sWeekdays.GetLength() - 1);
				sWhenDoDes.Format(L"<b .yellow>����[%s]</b>(0-����,...,6-����)�� [%s]<br />",sWeekdays,sPart);
				break;
			}
		case AUTOTASK_EXEC_ATMONTHDAY:
			{
				ASSERT(m_arX.size() == 1); 
				QString sDays,sTemp;
				for (int iDay = 0; iDay < 31; iDay++)
				{
					if (m_arX[0] & (0x1 << iDay))
					{
						sTemp.Format(L"%d,",iDay);
						sDays += sTemp;
					}
				}
				sDays = sDays.Left(sDays.GetLength() - 1);
				sWhenDoDes.Format(L"ÿ <b .yellow>��[%s]</b> �� [%s]",sDays,sPart);
				break;
			}
        default:
            return FALSE;
		}
	}
	return TRUE;
}

BOOL QTimer::EnableReminder( HANDLE hTimerQueue,int nTaskID,BOOL bEnable/*=TRUE*/ )
{
    BOOL bRet;
	if (bEnable)
	{
		if (IsStarted())
		{
			bRet = SetRemindTimer(hTimerQueue,nTaskID,m_tmNextExec);
		}
		m_dwTimerFlag &= ~TIMER_FLAG_REMINDERDISABLED;
	}
	else
	{
		if (m_hTimerReminder != NULL)
		{
			if (!DeleteTimerQueueTimer(hTimerQueue,m_hTimerReminder,NULL))
			{
				return FALSE;
			}
			m_hTimerReminder = NULL;
		}
		m_dwTimerFlag |= TIMER_FLAG_REMINDERDISABLED;
	    bRet = TRUE;
    }
	return bRet;
}
