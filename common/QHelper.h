#pragma once

#include <list>
#include <algorithm>
#include <iterator>
#include "ConstValues.h"
#include "QTimer.h"

class QHelper
{
public:
	// s/m/h
	static QString GetTimeUnitString(WCHAR cUnit);
	static DWORD HowManySeconds( int nX,WCHAR cUnit );

	// ��ȡһ��Ψһ���ַ���
	static QString UniqueString();

	// t2 - t1 �Ĳ�ֵ����������
	static DWORD TimeSpan(WORD t1,WORD t2);

	// ����t1 + dwSecs ��õ���ʱ��ֵ��
	// ���������23��59 ,bOverflow Ϊ�� 
	static WORD TimePlus(WORD t1,DWORD dwSecs,BOOL &bOverflow);
	// ���Date��Time
	static QTime CombineDateTime(DWORD dwDate,WORD wTime);

    static BOOL ParseInt(__in const CStdStringW& src, __out int& nT);

    static QString MakeReleateExp( ENUM_AUTOTASK_EXECFLAG eExec,
        int nSpan, wchar_t cUnit,   // �����eExecһ��ʱ��
        int nSpan1 = 0, wchar_t cUnit1 = L's', // Ȼ������ 0 Ϊ�޼��
        int nExecCount = 0); // �ܹ���ִ�еĴ���, 0Ϊ������

    static BOOL MakeAbsExp(ENUM_AUTOTASK_EXECFLAG eFlag,QTime& tmB, QTime& tmE, 
            StringArray& arDatePots, /* ִ�����ڵ� */ StringArray& arTimePots, 
            __out QString& sResultExp, __out QString& sError);

    static QString MakeRemindExp( int nA, WCHAR cAUnit, const QString&sSound,const QString&sMsg );

    /** 10h��10s��10m ������ʱ���ʽ����
     *	return:
     *      TRUE    ���ݸ�ʽ��ȷ
     *	params:
     *		-[in]
     *          src     ���������ַ���
     *		-[out]
     *          nT      ���ֲ���
     *          cUnit   ��λ����
    **/
    static BOOL ParseUnitTime(__in const CStdStringW& src, __out int &nT, __out wchar_t& cUnit);
};
