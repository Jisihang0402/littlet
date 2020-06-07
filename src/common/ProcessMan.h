#pragma once

#include "BaseType.h"
#include "time/QTime.h"
#include <Windows.h>
#include <Psapi.h>
#include <vector>
#include <tlhelp32.h>

#pragma comment(lib,"Psapi")

// ϵͳ����ö����
class QProcessMan
{
public:
	QProcessMan(void);
	~QProcessMan(void);

	// ö��֮
	BOOL SnapShot();
	// ������Ŀ
	inline DWORD GetPsNumber()const { return pids_.size(); }
	// ��������ID
	DWORD NextID();
	
	static BOOL GetPsPath( __in DWORD nProcessID, 
		__out CStdString &sPath ,__out QTime* pStart=NULL);
	static BOOL GetPsPath( __in HANDLE hProcess ,
		__out CStdString &sPath,__out QTime* pStart=NULL);
	static BOOL GetPsPath( __in HWND hWnd,
		__out CStdString &sPath,__out QTime* pStart);
	static BOOL GetProcessStartupTime( __in HANDLE hProcess ,
		__out QTime* pStart);
	static QTime GetCurrentProcessStartupTime();
	static QTime GetSystemStartupTime() { return sm_tmSystemStart; }
	static BOOL DebugPrivilege(BOOL bEnable);
	static BOOL IsExeRun(const CStdString & sExePath,__out QTime &tmRun);

private:
	typedef std::vector<DWORD> PIDs;
	PIDs	pids_;
	int		idx_;
	static const QTime		sm_tmSystemStart; // ϵͳ����ʱ��
	static QTime		sm_tmAppStart;	// ��Ӧ�ó�������ʱ��
};

