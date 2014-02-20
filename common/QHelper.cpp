#include "QHelper.h"

//////////////////////////////////////////////////////////////////////////
// ���Ժ���
/*
const QHelper::_CONTACT_RELATION QHelper::sm_arrRelationMap[]=
{
	{CONTACT_RELSHIP_FAMILY,L"family"},
	{CONTACT_RELSHIP_LOVER,L"lover"},
	{CONTACT_RELSHIP_FRIEND,L"friend"},
	{CONTACT_RELSHIP_MATE,L"mate"},
	{CONTACT_RELSHIP_WORK,L"work"},
	{CONTACT_RELSHIP_OTHER,L"other"},
};

QString QHelper::GetRelationText(ENUM_CONTACT_RELATIONSHIP eShip)
{
	for (int i = 0; i < ARRAYSIZE(sm_arrRelationMap); i++)
	{
		if (sm_arrRelationMap[i].eShip == eShip)
			return sm_arrRelationMap[i].sShip;
	}
	return L"";
}
ENUM_CONTACT_RELATIONSHIP QHelper::GetRelateFlag(const QString& sShip)
{
	for (int i = 0; i < ARRAYSIZE(sm_arrRelationMap); i++)
	{
		if (sm_arrRelationMap[i].sShip == sShip)
			return sm_arrRelationMap[i].eShip;
	}
	return CONTACT_RELSHIP_OTHER;
}
*/

QString QHelper::UniqueString()
{
	QString str;
	QTime tmNow = QTime::GetCurrentTime();
	str.Format(L"%lf",tmNow.GetTime());
	str.Remove(L'.');
	return str;
}

// t2 - t1 �Ĳ�ֵ����������
DWORD QHelper::TimeSpan(WORD t1,WORD t2)
{
	return (QTime::ParseTime(t2) - QTime::ParseTime(t1)).GetTotalSeconds();
}

// ����t1 + dwSecs ��õ���ʱ��ֵ��
// ���������23��59 ,bOverflow Ϊ��
WORD QHelper::TimePlus(WORD t1,DWORD dwSecs,BOOL &bOverflow)
{
	ASSERT(FALSE);
	return 0;
}

QTime QHelper::CombineDateTime(DWORD dwDate,WORD wTime)
{
	QTime tmDate = QTime::ParseDate(dwDate);
	QTime tmTime = QTime::ParseTime(wTime);
	tmDate.SetTime(tmTime.GetHour(),tmTime.GetMinute(),tmTime.GetSecond());
	return tmDate;
}

DWORD QHelper::HowManySeconds( int nX,WCHAR cUnit )
{
	switch (cUnit)
	{
	case L's':case L'S': return nX;
	case L'm':case L'M': return nX * 60;
	case L'h':case L'H': return nX * 3600;
	default:ASSERT(FALSE); return 0;
	}
}

QString QHelper::GetTimeUnitString(WCHAR cUnit)
{
	switch (cUnit)
	{
	case L's':case L'S': return L"��";
	case L'm':case L'M': return L"��";
	case L'h':case L'H': return L"ʱ";
	default:ASSERT(FALSE); return L"��Ч��λ";
	}
}

QString QHelper::MakeReleateExp( ENUM_AUTOTASK_EXECFLAG eExec, 
    int nSpan, wchar_t cUnit, /* �����eExec?��ʱ�� */ 
    int nSpan1 /*= 0*/, wchar_t cUnit1 /*= L's'*/, /* ?������ 0 Ϊ�޼�� */ 
    int nExecCount /*= 0*/ )
{
    QString sExp;
    // R=1;P=20m;Q=22m;C=100;
    // R
    sExp.Format(L"R=%d;", eExec);

    //P - span1
    sExp.AppendFormat(L"P=%d%c;", nSpan, cUnit);

    // Q - span2
    if (nSpan1 > 0)
    { // span2
        sExp.AppendFormat(L"Q=%d%c;",nSpan1, cUnit1);
        // C - exec count
        if (nExecCount > 0)
        {
            sExp.AppendFormat(L"C=%d;",nExecCount);
        }
    }
    return sExp;
}

BOOL QHelper::ParseInt(__in const CStdStringW& src, __out int& nT)
{
    CStdStringW tmp = src;
    if ( tmp.Trim().IsEmpty() )
        return FALSE;

    if (-1 == tmp.find_first_not_of(L"0123456789"))
    {
        nT = StrToInt(tmp);
        return TRUE;
    }
    return FALSE;
}

BOOL QHelper::MakeAbsExp(ENUM_AUTOTASK_EXECFLAG eFlag,QTime& tmB, QTime& tmE,
    StringArray& arDatePots, StringArray& arTimePots,
    __out QString& sResultExp, __out QString& sError)
{
    if (arTimePots.empty() || arDatePots.empty())
    {
        ASSERT(FALSE);
        sError = L"��������ָ��һ�����ں�һ��ʱ��";
        return FALSE;
    }

    // �Ȼ�ȡʱ���
    QString sTimePots;
    QTime tTime;
    std::vector<DWORD> vExist;
    for (int i = 0; i < arTimePots.size(); ++i)
    {
        if ( !tTime.ParseDateTime(arTimePots[i], VAR_TIMEVALUEONLY))
        {
            sError = L"ִ��ʱ������";
            return FALSE;
        }
        DWORD dwT = tTime.MakeTime();
        if (std::find(vExist.begin(), vExist.end(), dwT) == vExist.end())
        {
            vExist.push_back(dwT);
            sTimePots.AppendFormat(L"%u,", dwT);
        }
    }
    // ȥ�����һ���ַ�','
    sTimePots.TrimRight(L',');

    // ���ڲ���
    if (AUTOTASK_EXEC_ATDATE == eFlag)
    { // �������ں�ʱ���ִ��
        QTime tmNow = QTime::GetCurrentTime();
        QTime tDate;
        tDate.ParseDateTime(arDatePots[0]);
        TRACE(L"parsedate string:-> %s  result:-> %s\n",
            (LPCWSTR)arDatePots[0], (LPCWSTR)tDate.Format(L"%c"));
        TRACE(L"time now: %s\n", (LPCWSTR)tmNow.Format(L"%c"));
        if (tDate.CompareDate(tmNow) < 0)
        {
            sError = L"ִ�������Ѿ���ȥ��";
            return FALSE;
        }
        // �Զ����������������Ϊ���ʵ�ʱ��
        if ((tmB >= tmE) || (tmE <= tmNow) || (tmB <= tmNow)
            || (tmB > tDate) || (tmE < tDate))
        {
            tmB = tmNow;
            tmE = tDate + QTimeSpan(1,0,0,0);
        }
        sResultExp.Format(L"A=%d;X=%u;T=%s;",AUTOTASK_EXEC_ATDATE, tDate.MakeDate(), sTimePots);
        return TRUE;
    }
    else if (AUTOTASK_EXEC_ATDAILY == eFlag)
    { // ÿ��x��ִ��һ��
        if (arDatePots.size() > 1)
        {
            sError = L"���ռ��ִ��   ����̫��";
            return FALSE;
        }
        int nXDay;
        if (!QHelper::ParseInt(arDatePots[0], nXDay))
        {
            sError = L"���ռ��ִ��   ������������      Ӧ��ָ��һ���������";
        }
        if ((nXDay < 1) || (nXDay > 30))
        {
            sError = L"���ռ��ִ��   �����ΧӦ����[1,30]֮��";
            return FALSE;
        }
        sResultExp.Format(L"A=%d;P=%u;T=%s;", AUTOTASK_EXEC_ATDAILY, nXDay, sTimePots);
        return TRUE;
    }
    else if (AUTOTASK_EXEC_ATWEEKDAY == eFlag)
    { 
        // �ܼ�
        int v , weeks = 0;
        for (int i = 0; i < arDatePots.size(); ++i)
        {
            if (!QHelper::ParseInt(arDatePots[i], v) || (v > 6))
            {
                sError = L"����ִ��   ������������    Ӧ��ָ��һ��[0, 6]������";
                return FALSE;
            }
            weeks |= (0x1<<v);
        }
        if (0 == weeks)
        {
            sError = L"����ִ��   ������������    Ӧ������ָ��һ��[0, 6]������";
            return FALSE;
        }
        sResultExp.Format(L"A=%d;X=%u;T=%s;",AUTOTASK_EXEC_ATWEEKDAY, weeks, sTimePots);
        return TRUE;
    }
    else if (AUTOTASK_EXEC_ATMONTHDAY == eFlag)
    {
        int d, days = 0;
        for (int i = 0; i < arDatePots.size(); ++i)
        {
            if (!QHelper::ParseInt(arDatePots[i], d) || (d > 31))
            {
                sError = L"����ִ��   ������������    ������Χ[0, 31], 0 - ÿ�ն�ִ��";
                return FALSE;
            }
            else if (0 == d)
            {
                days = 0x0fffffff;  // ȫ��ִ��
                break;
            }
            days |= (0x1 << d);
        }
        if (0 == days)
        {
            sError = L"����ִ��   ������������    Ӧ������ָ��һ��[0, 31]������,  0 - ÿ�ն�ִ��";
            return FALSE;
        }
        sResultExp.Format(L"A=%d;X=%u;T=%s;",AUTOTASK_EXEC_ATMONTHDAY,  days, sTimePots);
        return TRUE;
    }
    sError = L"���Ա��ʽ    ִ�в�������";
    return FALSE;
}

QString QHelper::MakeRemindExp( int nA, WCHAR cAUnit, 
    const QString&sSound,const QString&sMsg )
{
    cAUnit = tolower(cAUnit);
    if ((nA <= 0) || ((cAUnit != L's') && (cAUnit != L'm') && (cAUnit != L'h')))
        return L"";
    QString sRet;
    sRet.Format(L"A=%d%c;\nS=%s;\nM=%s;\n",nA,cAUnit,sSound,sMsg);
    return sRet;
}

BOOL QHelper::ParseUnitTime(__in const CStdStringW& src, __out int &nT, __out wchar_t& cUnit)
{
    CStdStringW tmp = src;
    if (tmp.Trim().IsEmpty())
        return FALSE;

    cUnit = tolower(tmp.back());
    if ( (L's' != cUnit) && (L'm' != cUnit) && (L'h' != cUnit))
        return FALSE;
    tmp.pop_back();

    return QHelper::ParseInt(tmp, nT);
}
