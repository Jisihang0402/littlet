#include "stdafx.h"
#include "app.h"
#include "AppHelper.h"
#include <lmerr.h>
#include "file/FileOper.h"
#include "ui/QUIGlobal.h"
#include "ui/WndHelper.h"
#include "qmemfile.h"
#include "sys/singleinst.h"

namespace quibase
{
#ifndef AUTORUN_KEYNAME
#define AUTORUN_KEYNAME  _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")
#endif

	BOOL MakeSureDirExist(LPCWSTR sDir)
	{
		CStdString sTemp = sDir;
		sTemp.Replace(L'/',L'\\');
		HRESULT hr = CWinPathApi::CreateFullPath(sTemp);
		return (hr == S_OK) || (hr == S_FALSE);
	}

	BOOL CreateGUID(__out CStdString& sGUID)
	{
		BOOL bRet = FALSE;
		GUID guid;
		CoInitialize(NULL);
		if (S_OK == ::CoCreateGuid(&guid))
		{
			sGUID.Format(L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
				guid.Data1,
				guid.Data2,
				guid.Data3,
				guid.Data4[0], guid.Data4[1],
				guid.Data4[2], guid.Data4[3],
				guid.Data4[4], guid.Data4[5],
				guid.Data4[6], guid.Data4[7]);
			bRet = TRUE;
		}
		CoUninitialize();
		return bRet;
	}

	CStdString GetModulePath()
	{
		TCHAR path[1024];
		int nSize = ::GetModuleFileName(NULL,path,1024);
		path[nSize] = _T('\0');
		CStdString sRet(path);
		sRet.Replace(_T('\\'),_T('/'));
		int idx = sRet.ReverseFind(_T('/'));
		ATLASSERT(idx != -1);
		sRet = sRet.Left(idx+1);
		return sRet;
	}

    CStdString GetModuleName(BOOL bWithExt/* = FALSE*/)
    {
        TCHAR szFullPath[1024] = {0};
        ::GetModuleFileName(NULL,szFullPath,1024);

        CStdString sRet(szFullPath);
        int idx = sRet.ReverseFind(_T('\\'));
        sRet = sRet.Mid(idx + 1);
        if (!bWithExt)
        {
            idx = sRet.ReverseFind(L'.');
            if (-1 != idx)
            {
                sRet = sRet.Left(idx);
            }
        }
        return sRet;
    }

	BOOL IsFileExist(LPCTSTR pszFile)
	{
		BOOL bRet = FALSE;
		if( pszFile == NULL )
			return bRet;
		if( pszFile[0] == 0 )
			return bRet;

		WIN32_FIND_DATA fd = {0};
		HANDLE hFile = FindFirstFile(pszFile, &fd);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			FindClose(hFile);
			if( !(fd.dwFileAttributes &  FILE_ATTRIBUTE_DIRECTORY) )
				bRet = TRUE;
		}
		return bRet;
	}

	CStdString SysErrorMessage(DWORD dwLastError )
	{
		CStdString strRet(_T("Unknown error"));
		HMODULE hModule = NULL; // default to system source
		LPSTR MessageBuffer;
		DWORD dwBufferLength;

		DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_FROM_SYSTEM ;

		//
		// If dwLastError is in the network range, 
		// load the message source.
		//

		if(dwLastError >= NERR_BASE && dwLastError <= MAX_NERR) {
			hModule = LoadLibraryEx(TEXT("netmsg.dll"),NULL,LOAD_LIBRARY_AS_DATAFILE);
			if(hModule != NULL)
				dwFormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
		}

		//
		// Call FormatMessage() to allow for message 
		// text to be acquired from the system 
		// or from the supplied module handle.
		//
		if(dwBufferLength = FormatMessageA(
			dwFormatFlags,
			hModule, // module to get message from (NULL == system)
			dwLastError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
			(LPSTR) &MessageBuffer,
			0,
			NULL
			))
		{
			//
			// Output message string on stderr.
			//
			strRet=CStdString(MessageBuffer,dwBufferLength);
			//
			// Free the buffer allocated by the system.
			//
			LocalFree(MessageBuffer);
		}

		//
		// If we loaded a message source, unload it.
		//
		if(hModule != NULL)
			FreeLibrary(hModule);
		return strRet;
	}

	void CopyTexttoClipboard(const CStdString& sText, HWND hwnd) 
	{
		if (!::OpenClipboard(hwnd)) 
			return; 

		::EmptyClipboard(); 

		// Allocate a global memory object for the text. 
		HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (sText.GetLength() + 1) * sizeof(TCHAR)); 

		if (!hglbCopy) 
		{ 
			CloseClipboard(); 
			return; 
		} 

		// Lock the handle and copy the text to the buffer. 
		LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy); 

		memcpy(lptstrCopy, (LPVOID)(LPCTSTR)sText, sText.GetLength() * sizeof(TCHAR)); 

		lptstrCopy[sText.GetLength()] = (TCHAR) 0;    // null character 
		GlobalUnlock(hglbCopy); 

		// Place the handle on the clipboard. 
		::SetClipboardData(CF_TEXT, hglbCopy); 

		::CloseClipboard();
	}

	CStdString GetClipboardText(HWND hwnd)
	{
		if (!::OpenClipboard(hwnd)) 
			return ""; 

		HANDLE hData = ::GetClipboardData(CF_TEXT);
		char* buffer = (char*)GlobalLock(hData);

		CStdString sText(buffer);

		::GlobalUnlock(hData);
		::CloseClipboard();

		return sText;
	}

	BOOL IsGuid(LPCTSTR szGuid)
	{
		GUID guid;

		return GuidFromString(szGuid, guid);
	}

	BOOL GuidFromString(LPCTSTR szGuid, GUID& guid)
	{
		#pragma comment(lib, "Rpcrt4.lib")

		RPC_STATUS rpcs = UuidFromString((RPC_WSTR)szGuid, &guid);

		if (rpcs != RPC_S_OK)
		{
			NullGuid(guid);
			return FALSE;
		}

		return TRUE;
	}

	BOOL GuidToString(const GUID& guid, CStdString& sGuid)
	{
		#pragma comment(lib, "Rpcrt4.lib")

		RPC_WSTR pszGuid;
//		unsigned char* pszGuid;

		if (RPC_S_OK == UuidToString((GUID*)&guid, &pszGuid))
			sGuid = CStdString((LPCWSTR)pszGuid);
		else
			sGuid.Empty();

		RpcStringFree(&pszGuid);

		return !sGuid.IsEmpty();
	}

	BOOL GuidIsNull(const GUID& guid)
	{
		static GUID NULLGUID = { 0 };

		return SameGuids(guid, NULLGUID);
	}

	BOOL SameGuids(const GUID& guid1, const GUID& guid2)
	{
		return (memcmp(&guid1, &guid2, sizeof(GUID)) == 0);
	}

	void NullGuid(GUID& guid)
	{
		ZeroMemory(&guid, sizeof(guid));
	}

	BOOL IsWorkStationLocked()
	{
		// note: we can't call OpenInputDesktop directly because it's not
		// available on win 9x
		typedef HDESK (WINAPI *PFNOPENDESKTOP)(LPSTR lpszDesktop, DWORD dwFlags, BOOL fInherit, ACCESS_MASK dwDesiredAccess);
		typedef BOOL (WINAPI *PFNCLOSEDESKTOP)(HDESK hDesk);
		typedef BOOL (WINAPI *PFNSWITCHDESKTOP)(HDESK hDesk);

		// load user32.dll once only
		static HMODULE hUser32 = LoadLibrary(L"user32.dll");

		if (hUser32)
		{
			static PFNOPENDESKTOP fnOpenDesktop = (PFNOPENDESKTOP)GetProcAddress(hUser32, "OpenDesktopA");
			static PFNCLOSEDESKTOP fnCloseDesktop = (PFNCLOSEDESKTOP)GetProcAddress(hUser32, "CloseDesktop");
			static PFNSWITCHDESKTOP fnSwitchDesktop = (PFNSWITCHDESKTOP)GetProcAddress(hUser32, "SwitchDesktop");

			if (fnOpenDesktop && fnCloseDesktop && fnSwitchDesktop)
			{
				HDESK hDesk = fnOpenDesktop("Default", 0, FALSE, DESKTOP_SWITCHDESKTOP);

				if (hDesk)
				{
					BOOL bLocked = !fnSwitchDesktop(hDesk);

					// cleanup
					fnCloseDesktop(hDesk);

					return bLocked;
				}
			}
		}

		// must be win9x
		return FALSE;
	}

#ifndef SPI_GETSCREENSAVERRUNNING
#  define SPI_GETSCREENSAVERRUNNING 114
#endif

	BOOL IsScreenSaverActive()
	{
		BOOL bSSIsRunning = FALSE;
		::SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &bSSIsRunning, 0);

		return bSSIsRunning; 
	}
// 
// 	int CompareVersions(LPCTSTR szVersion1, LPCTSTR szVersion2)
// 	{
// 		// if the first character of either string is not a number
// 		// then fall back on a standard string comparison
// 		if (!szVersion1 || !isdigit(szVersion1[0]) || !szVersion2 || !isdigit(szVersion2[0]))
// 			return wcscmp(szVersion1, szVersion2);
// 
// 		CStringArray aVer1, aVer2;
// 
// 		Split(szVersion1, '.', aVer1);
// 		Split(szVersion2, '.', aVer2);
// 
// 		// compare starting from the front
// 		for (int nItem = 0; nItem < aVer1.GetSize() && nItem < aVer2.GetSize(); nItem++)
// 		{
// 			int nThis = _wtoi(aVer1[nItem]);
// 			int nOther = _wtoi(aVer2[nItem]);
// 
// 			if (nThis < nOther)
// 				return -1;
// 
// 			else if (nThis > nOther)
// 				return 1;
// 
// 			// else try next item
// 		}
// 
// 		// if we got here then compare array lengths
// 		if (aVer1.GetSize() < aVer2.GetSize())
// 			return -1;
// 
// 		else if (aVer1.GetSize() > aVer2.GetSize())
// 			return 1;
// 
// 		// else
// 		return 0;
// 	}

	// private method for implementing the bubblesort
// 	BOOL CompareAndSwap(CStringArray& array, int pos, BOOL bAscending)
// 	{
// 		QString temp;
// 		int posFirst = pos;
// 		int posNext = pos + 1;
// 
// 		QString sFirst = array.GetAt(posFirst);
// 		QString sNext = array.GetAt(posNext);
// 
// 		int nCompare = sFirst.CompareNoCase(sNext);
// 
// 		if ((bAscending && nCompare > 0) || (!bAscending && nCompare < 0))
// 		{
// 			array.SetAt(posFirst, sNext);
// 			array.SetAt(posNext, sFirst);
// 
// 			return TRUE;
// 
// 		}
// 		return FALSE;
// 	}
// 
// 	void SortArray(CStringArray& array, BOOL bAscending)
// 	{
// 		BOOL bNotDone = TRUE;
// 
// 		while (bNotDone)
// 		{
// 			bNotDone = FALSE;
// 
// 			for(int pos = 0; pos < array.GetUpperBound(); pos++)
// 				bNotDone |= CompareAndSwap(array, pos, bAscending);
// 		}
// 
// 		/*
// 		#ifdef _DEBUG
// 		TRACE(L"SortArray(%s)\n", bAscending ? "Ascending" : "Descending");
// 
// 		for (int pos = 0; pos < array.GetSize(); pos++)
// 		TRACE(L"%s\n", array[pos]);
// 		#endif
// 		*/
// 	}

    BOOL ShutdownComputer( __in BOOL bReboot,__in LPWSTR pszMsg, 
            __in DWORD dwTimeout,__in BOOL bAskUserCloseApp/*=TRUE*/)
    {
        HANDLE hToken; // handle to process token 
        TOKEN_PRIVILEGES tkp; // pointer to token structure 
        BOOL fResult; // system shutdown flag 
        // Get the current process token handle so we can get shutdown 
        // privilege. 
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        {
            return FALSE;
        }

        // Get the LUID for shutdown privilege. 
        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 
        tkp.PrivilegeCount = 1; // one privilege to set 
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
        // Get shutdown privilege for this process. 
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0); 
        // Cannot test the return value of AdjustTokenPrivileges. 
        if (GetLastError() != ERROR_SUCCESS) 
            return FALSE;

        // Display the shutdown dialog box and start the time-out countdown. 
        fResult = InitiateSystemShutdown( 
            NULL, // shut down local computer 
            pszMsg, // message to user 
            dwTimeout, // time-out period 
            !bAskUserCloseApp, // ask user to close apps 
            bReboot); // reboot after shutdown 
        if (!fResult) 
        {
            return FALSE;
        } 
        // Disable shutdown privilege. 
        tkp.Privileges[0].Attributes = 0; 
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
            (PTOKEN_PRIVILEGES) NULL, 0); 

        return (GetLastError() == ERROR_SUCCESS);
    }

    // ȡ���ػ�
    BOOL PreventSystemShutdown()
    {
        HANDLE hToken;              // handle to process token 
        TOKEN_PRIVILEGES tkp;       // pointer to token structure 

        // Get the current process token handle  so we can get shutdown 
        // privilege. 

        if (!OpenProcessToken(GetCurrentProcess(), 
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
            return FALSE; 

        // Get the LUID for shutdown privilege. 

        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
            &tkp.Privileges[0].Luid); 

        tkp.PrivilegeCount = 1;  // one privilege to set    
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

        // Get shutdown privilege for this process. 

        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
            (PTOKEN_PRIVILEGES)NULL, 0); 

        if (GetLastError() != ERROR_SUCCESS) 
            return FALSE; 

        // Prevent the system from shutting down. 

        if ( !AbortSystemShutdown(NULL) ) 
            return FALSE; 

        // Disable shutdown privilege. 

        tkp.Privileges[0].Attributes = 0; 
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
            (PTOKEN_PRIVILEGES) NULL, 0); 

        return TRUE;
    }

    BOOL GetSpeialPath(__in int csidl, __out CStdString &sPath)
    {
        LPITEMIDLIST  ppidl = NULL;   

        wchar_t szBuf[MAX_PATH * 2] = {0};
        if (SHGetSpecialFolderLocation(NULL, csidl, &ppidl) == S_OK)  
        {  
            BOOL flag = SHGetPathFromIDList(ppidl, szBuf);  
            CoTaskMemFree(ppidl);
            sPath = szBuf;
            return flag;  
        }  

        return FALSE;  
    }

    bool IsAutoRun()
    {
        TCHAR szExeFile[MAX_PATH] = {0};
        ::GetModuleFileName(NULL, szExeFile, MAX_PATH);

        CRegKey regkey;
        if (regkey.Open(HKEY_CURRENT_USER, AUTORUN_KEYNAME) == ERROR_SUCCESS)
        {
            DWORD dwCount = 1024;
            TCHAR szValue[1024] = {0};
            if (regkey.QueryStringValue(APP_NAME, szValue, &dwCount) == ERROR_SUCCESS)
            {
                return _tcsstr(szValue, szExeFile) != NULL;
            }
            regkey.Close();
        }
        return false;
    }

    void CreateAutoRun()
    {
        CStdString strValue = _T("\"");
        TCHAR szExeFile[MAX_PATH] = {0};
        ::GetModuleFileName(NULL, szExeFile, MAX_PATH);
        strValue += szExeFile;
        strValue += _T("\"  /s");

        CRegKey regkey;
        if (regkey.Open(HKEY_CURRENT_USER, AUTORUN_KEYNAME) == ERROR_SUCCESS)
        {
            if (regkey.SetStringValue(APP_NAME, strValue))
            {
                ATLTRACE("Query Success.\n");
            }
            regkey.Close();
        }
    }

    void RemoveAutoRun()
    {
        CRegKey regkey;
        if (regkey.Open(HKEY_CURRENT_USER, AUTORUN_KEYNAME) == ERROR_SUCCESS)
        {
            if (regkey.DeleteValue(APP_NAME))
            {
                ATLTRACE("Query Success.\n");
            }
            regkey.Close();
        }
    }

    void SetAutoRun(bool bAutoRun)
    {
        if (bAutoRun)
        {
            if (!IsAutoRun())
            {
                CreateAutoRun();
            }
        }
        else
        {
            RemoveAutoRun();
        }
    }

    BOOL UpgradeProcessPrivilege()
    {
        HANDLE hToken; 
        TOKEN_PRIVILEGES tkp;

        // Get a token for this process.
        if (!OpenProcessToken(GetCurrentProcess(), 
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
            return FALSE ; 

        // Get the LUID for the shutdown privilege.
        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
            &tkp.Privileges[0].Luid); 

        tkp.PrivilegeCount = 1; // one privilege to set 
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

        // Get the shutdown privilege for this process.
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
            (PTOKEN_PRIVILEGES)NULL, 0); 

        if (GetLastError() != ERROR_SUCCESS) 
            return FALSE; 

        return TRUE;
    }

//     int CorrectTime()
//     {
//         int nRet = 0;
//         if (!UpgradeProcessPrivilege())
//         {
//             return -1;
//         }
// 
//         WSADATA wsaData;
//         //��ʼ��Socket ��
//         WSAStartup(MAKEWORD(2,0),&wsaData);
//         //�����׽��� 
//         SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//         SOCKADDR_IN sa;
//         sa.sin_family = AF_INET;
//         //���÷������˵Ķ˿�
//         //sa.sin_port = htons(IPPORT_SMTP);   //SMTP����������Ķ˿�Ϊ25
//         sa.sin_port = htons(IPPORT_TIMESERVER);  //SNTPУʱ����Ķ˿ں�Ϊ37
//         //��ʱ������ip��ַ
//         sa.sin_addr.S_un.S_addr = inet_addr("223.255.185.2");
//         //���ӷ�����,����������Ƿ�ɹ�
//         if(connect(sock, (SOCKADDR *)&sa, sizeof(sa)) == SOCKET_ERROR)
//         {
//             return -2;
//         }
// #define HIGHTIME 21968699  // Jan 1, 1900 FILETIME.highTime
// #define LOWTIME  4259332096  // Jan 1, 1900 FILETIME.lowtime
// 
//         unsigned long ulTime = 0;
//         recv(sock, (char *)&ulTime, sizeof(unsigned long), 0);
//         //��һ���޷��ų��������������ֽ�˳��ת��Ϊ�����ֽ�˳��
//         ulTime = ntohl(ulTime);
//         SYSTEMTIME st; //ϵͳʱ��ṹ��,������WinBase.h
//         UINT64 uiCurTime, uiBaseTime, uiResult; 
//         uiBaseTime = ((UINT64)HIGHTIME << 32) + LOWTIME; 
//         uiCurTime = (UINT64)ulTime * (UINT64)10000000;
//         uiResult = uiBaseTime + uiCurTime; 
//         //��FILETIME�ṹװ�䵽SYSTEMTIME�ṹ
//         FileTimeToSystemTime((LPFILETIME)&uiResult, &st);
// 
//         //����ϵͳʱ��
//         if (st.wYear > 2012)
//         { // ��ʱȡ��ʱ�䲻��ȷ��2012ֻ����һ���жϿ�ȡ���������ǲ�����ȷ
//             if (FALSE == SetSystemTime(&st))
//             {
//                 nRet = -3;
//             }
//         }
// 
//         //�ر�socket��������WSADATA��Դ 
//         closesocket(sock);
//         WSACleanup();
//         return nRet;
//     }

    BOOL ReleaseRes(LPCTSTR filename,WORD wResID, LPCTSTR filetype)
    {
        DWORD dwWrite=0;
        HANDLE  hFile = CreateFile(filename, GENERIC_WRITE,FILE_SHARE_WRITE,
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile == INVALID_HANDLE_VALUE)
            return FALSE;

        // ������Դ�ļ��С�������Դ���ڴ桢�õ���Դ��С   
        HRSRC hrsc=FindResource(NULL, MAKEINTRESOURCE(wResID), filetype);
        HGLOBAL hG=LoadResource(NULL, hrsc);
        DWORD dwSize=SizeofResource( NULL, hrsc);
        // д���ļ�
        WriteFile(hFile,hG,dwSize,&dwWrite,NULL);
        CloseHandle(hFile);
        return TRUE;
    }

    void VisitWebsiteWithDefaultBrowser(LPCWSTR lpszUrl)
    {
        if (lpszUrl && _tcslen(lpszUrl) > 5)
        {
            SHELLEXECUTEINFOW ShExecInfo = {0};
            ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
            ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
            ShExecInfo.hwnd = NULL;
            ShExecInfo.lpVerb = L"open";
            ShExecInfo.lpFile = lpszUrl;
            ShExecInfo.lpParameters = NULL;
            ShExecInfo.lpDirectory = NULL;
            ShExecInfo.nShow = SW_SHOW;
            ShExecInfo.hInstApp = NULL;
            if (!ShellExecuteExW(&ShExecInfo))
            {
                ShExecInfo.lpFile = L"iexplore";
                ShExecInfo.lpParameters = lpszUrl;
                ShellExecuteExW(&ShExecInfo);
            }
        }
    }

//     int SyncTime()
//     {
//         TCHAR szExeFile[MAX_PATH] = {0};
//         ::GetModuleFileName(NULL, szExeFile, MAX_PATH);
// 
//         QString strVerb;
//         NONCLIENTMETRICS info = { 0 };
//         OSVERSIONINFO ovi = { sizeof(OSVERSIONINFO) };
//         ::GetVersionEx(&ovi);
//         strVerb =  (ovi.dwMajorVersion < 6) ? _T("open") : _T("runas");
// 
//         SHELLEXECUTEINFO ShExecInfo;
//         memset(&ShExecInfo, 0, sizeof(ShExecInfo));
//         ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
//         ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
//         ShExecInfo.hwnd = NULL;
//         ShExecInfo.lpVerb = strVerb;
//         ShExecInfo.lpFile = szExeFile;
//         ShExecInfo.lpParameters = _T("/correcttime");
//         ShExecInfo.lpDirectory = NULL;
//         ShExecInfo.nShow = SW_SHOW;
//         ShExecInfo.hInstApp = NULL;
//         return ::ShellExecuteEx(&ShExecInfo);
//     }

    bool IsIEHomePage(LPCTSTR url)
    {
        CRegKey regkey;
        if (regkey.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Internet Explorer\\Main")) == ERROR_SUCCESS)
        {
            DWORD dwCount = 1024;
            TCHAR szValue[1024] = {0};
            if (regkey.QueryStringValue(_T("Start Page"), szValue, &dwCount) == ERROR_SUCCESS)
            {
                regkey.Close();
                return _tcsstr(szValue, url) != NULL;
            }
            regkey.Close();
        }
        return false;
    }

    bool SetIEHomePage(LPCTSTR url)
    {
        if (IsIEHomePage(url))
        {
            return true;
        }
        CRegKey regkey;
        if (regkey.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Internet Explorer\\Main")) == ERROR_SUCCESS)
        {
            if (regkey.SetStringValue(_T("Start Page"), url) == ERROR_SUCCESS)
            {
                ATLTRACE("Query Success.\n");
                regkey.Close();
                return true;
            }
            regkey.Close();
        }
        return false;
    }

    void PinToTaskbar(LPCTSTR lpszDestPath)
    {
        OSVERSIONINFO ovi = { sizeof(OSVERSIONINFO) };
        ::GetVersionEx(&ovi);
        if (ovi.dwMajorVersion < 6)
        {
            return;
        }
        TCHAR szPath[MAX_PATH] = {0};
        _tcscpy_s(szPath, MAX_PATH, lpszDestPath);
        _tcscat_s(szPath, MAX_PATH, _T(".lnk"));

        ::ShellExecute(NULL, _T("taskbarpin"), szPath, NULL, NULL, 0);
    }

    void UnPinFromTaskbar(LPCTSTR lpszDestPath)
    {
        OSVERSIONINFO ovi = { sizeof(OSVERSIONINFO) };
        ::GetVersionEx(&ovi);
        if (ovi.dwMajorVersion < 6)
        {
            return;
        }
        TCHAR szPath[MAX_PATH] = {0};
        _tcscpy_s(szPath, MAX_PATH, lpszDestPath);
        _tcscat_s(szPath, MAX_PATH, _T(".lnk"));

        ::ShellExecute(NULL, _T("taskbarunpin"), szPath, NULL, NULL, 0);
    }

    BOOL CreateShortcut(LPCSTR lpszDestPath, LPCTSTR lpszProgram, LPCTSTR lpszIco,
        LPCTSTR lpszArguments, LPCTSTR lpszWorkingDir, LPCTSTR lpszDescription)
    {
        char szPath[MAX_PATH] = {0};
        strcpy_s(szPath, MAX_PATH, lpszDestPath);
        CStdString name = CPath(lpszProgram).GetFileTitlex();
        strcat_s(szPath, MAX_PATH, ATL::CW2AEX<128>(name));
        strcat_s(szPath, MAX_PATH, ".lnk");

        HRESULT hr = S_OK;
        IShellLink *psl = NULL;     //IShellLink�ӿ�ָ��
        IPersistFile* ppf = NULL;   //IPersistFile�ӿ�ָ��
        BOOL bChk = FALSE;

        ::CoInitialize(NULL);
        //�õ�CLSID_ShellLink��ʶ��COM�����IShellLink�ӿ�
        hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
        if(SUCCEEDED(hr))
        {
            //��ѯIPersistFile�ӿ��Խ��п�ݷ�ʽ�Ĵ洢����
            hr = psl->QueryInterface (IID_IPersistFile, (void **)&ppf);
            if(SUCCEEDED(hr))
            {
                psl->SetDescription(lpszDescription);    //���ñ�ע
                if (NULL == lpszProgram)
                    psl->SetPath(quibase::GetModuleName(TRUE));               //����Դ�ļ���ַ
                else
                    psl->SetPath(lpszProgram);               //����Դ�ļ���ַ

                if (NULL != lpszWorkingDir)
                    psl->SetWorkingDirectory(lpszWorkingDir);
    
                if (NULL != lpszIco)
                    psl->SetIconLocation(lpszIco, 0);         //���ÿ�ݷ�ʽ��ͼ��
    
                if (NULL != lpszArguments)
                   psl->SetArguments(lpszArguments);         //���ò���

                WORD wsz[MAX_PATH] = {0};   //Unicode�ַ����Ļ����ַ

                // ��ANSI�ַ���ת��ΪUnicode�ַ���
                MultiByteToWideChar(CP_ACP, 0, szPath, -1, (LPWSTR)wsz, MAX_PATH);
                hr = ppf->Save((LPWSTR)wsz, FALSE);         //����Save�������д洢
                ppf->Release();
            }
            psl->Release();
        }
        ::CoUninitialize();

        return SUCCEEDED(hr);
    }

    BOOL CreateFileShortcut(LPCWSTR lpszFileName, LPCWSTR lpszLnkFileDir, 
        LPCWSTR lpszLnkFileName, LPCWSTR lpszWorkDir, WORD wHotkey, 
        LPCWSTR lpszDescription, int iShowCmd)  
    {  
        if (lpszLnkFileDir == NULL)  
            return FALSE;  

        HRESULT hr;  
        IShellLink     *pLink;  //IShellLink����ָ��  
        IPersistFile   *ppf; //IPersisFil����ָ��  

        //����IShellLink����  
        hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pLink);  
        if (FAILED(hr))  
            return FALSE;  

        //��IShellLink�����л�ȡIPersistFile�ӿ�  
        hr = pLink->QueryInterface(IID_IPersistFile, (void**)&ppf);  
        if (FAILED(hr))  
        {  
            pLink->Release();  
            return FALSE;  
        }  

        //Ŀ��  
        if (lpszFileName == NULL)  
            pLink->SetPath(_wpgmptr);  
        else  
            pLink->SetPath(lpszFileName);  

        //����Ŀ¼  
        if (lpszWorkDir != NULL)  
            pLink->SetPath(lpszWorkDir);  

        //��ݼ�  
        if (wHotkey != 0)  
            pLink->SetHotkey(wHotkey);  

        //��ע  
        if (lpszDescription != NULL)  
            pLink->SetDescription(lpszDescription);  

        //��ʾ��ʽ  
        pLink->SetShowCmd(iShowCmd);  


        //��ݷ�ʽ��·�� + ����  
        wchar_t szBuffer[MAX_PATH] = {0};  
        if (lpszLnkFileName != NULL) //ָ���˿�ݷ�ʽ������  
            swprintf_s(szBuffer,MAX_PATH, L"%s\\%s", lpszLnkFileDir, lpszLnkFileName);  
        else     
        {  
            //û��ָ�����ƣ��ʹ�ȡָ���ļ����ļ�����Ϊ��ݷ�ʽ���ơ�  
            wchar_t szTmp[MAX_PATH] = {0};
            const wchar_t *pstr = NULL;  
            if (lpszFileName != NULL)  
                pstr = wcsrchr(lpszFileName, L'\\');  
            else //if (0 == _get_wpgmptr((wchar_t**)&szTmp))
                pstr = wcsrchr(GetModuleName(TRUE), L'\\');  
            if (pstr == NULL)  
            { 
                ppf->Release();  
                pLink->Release();  
                return FALSE;  
            }  
            //ע���׺��Ҫ��.exe��Ϊ.lnk  
            swprintf_s(szBuffer,MAX_PATH, L"%s\\%s", lpszLnkFileDir, pstr);  
            int nLen = wcslen(szBuffer);  
            szBuffer[nLen - 3] = L'l';  
            szBuffer[nLen - 2] = L'n';  
            szBuffer[nLen - 1] = L'k';  
        }  
        //�����ݷ�ʽ��ָ��Ŀ¼��  
        hr = ppf->Save(szBuffer, TRUE);  

        ppf->Release();  
        pLink->Release();  
        return SUCCEEDED(hr);  
    }  

    BOOL IsPEFile(LPCTSTR lpszPath)
    {
        //�򿪼���ļ�
        HANDLE hFile=::CreateFile(lpszPath,GENERIC_READ,
            FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

        if(hFile==INVALID_HANDLE_VALUE)
        {
            return FALSE;
        }

        //����PE�ļ��е�DOSͷ��NTͷ
        IMAGE_DOS_HEADER dosHeader;
        IMAGE_NT_HEADERS ntHeader;

        //��֤����
        BOOL bValid = FALSE;
        DWORD dwRead = NULL;

        //��ȡDOSͷ
        ::ReadFile(hFile,&dosHeader,sizeof(dosHeader),&dwRead,NULL);
        if (dwRead==sizeof(dosHeader))
        {
            if (dosHeader.e_magic==IMAGE_DOS_SIGNATURE)
            {
                if (::SetFilePointer(hFile,dosHeader.e_lfanew,NULL,FILE_BEGIN))
                {
                    ReadFile(hFile,&ntHeader,sizeof(ntHeader),&dwRead,NULL);
                    if (dwRead==sizeof(ntHeader))
                    {
                        if(ntHeader.Signature==IMAGE_NT_SIGNATURE)
                            bValid=TRUE;
                    }
                }
            }
        }
        return bValid;
    }
};

//////////////////////////////////////////////////////////////////////////
QMemFile::LstME        QMemFile::ms_LstMe;

BOOL QMemFile::SetEventData(HANDLE hEvent,DWORD dwData)  
{  
	MeItr itr = FindEvent(hEvent);
	if (!IsEndItr(itr))
	{
		ME memFile = *itr;
		if(memFile.pMapBuf == NULL)  
		{
			return FALSE;  
		}  
		//����ֵ�������ڴ���  
		memcpy(memFile.pMapBuf,&dwData,sizeof(DWORD));  
	}
	return FALSE;
}  

DWORD QMemFile::GetEventData(HANDLE hEvent)  
{  
	MeItr itr = FindEvent(hEvent);
	if (!IsEndItr(itr))
	{
		ME memFile = *itr;
		if(memFile.pMapBuf == NULL)  
		{  
			return 0;  
		}  
		//���ڴ��л�ȡDWORD����
		DWORD dwVal = 0;  
		memcpy(&dwVal,memFile.pMapBuf,4); 
		return dwVal;
	}
	return 0;  
}  

QMemFile::MeItr QMemFile::FindEvent(HANDLE hEvent)
{
	for ( MeItr itr = ms_LstMe.begin(); itr != ms_LstMe.end(); ++itr)
	{
		if (itr->hEvent == hEvent)
			return itr;
	}
	return ms_LstMe.end();
}

BOOL QMemFile::CloseHandle(HANDLE hObject)  
{  
	MeItr itr;
	if(hObject == NULL || ( itr = FindEvent(hObject)) == ms_LstMe.end())  
		return FALSE;

	::CloseHandle(itr->hFileMap);
	htmlayout::mutex lck;
	lck.lock();
	ms_LstMe.erase(itr);
	lck.unlock();
	return ::CloseHandle(hObject);
} 

BOOL QMemFile::GetMemFile(LPCTSTR lpEventName, ME &memFile) 
{
	memFile.sEventName = GetEventName(lpEventName);
	// ����ӳ���ļ�  
	HANDLE hFileMap = ::OpenFileMapping(FILE_MAP_ALL_ACCESS,TRUE,memFile.sEventName);
	if (hFileMap == NULL
		&& (hFileMap = ::CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,4,memFile.sEventName)) == NULL)
	{
		return FALSE;  
	}
	//��ӳ���ļ������÷�����ڴ�ռ�  
	VOID *pMapBuf = MapViewOfFile(hFileMap,FILE_MAP_ALL_ACCESS,0,0,0);   
	if(pMapBuf != NULL)  
	{  
		//����ֵ���浽�ṹ����  
		memFile.hFileMap = hFileMap;  
		memFile.pMapBuf = pMapBuf; 
		return TRUE;
	} 
	CloseHandle(hFileMap);
	return FALSE;
}

CStdString QMemFile::GetEventName(LPCTSTR lpName,HANDLE hEvent/*=NULL*/)
{
	TCHAR buf[MAX_PATH] = {0};
	//���ж�������Ƿ�ֻ���ڲ�ʹ�á���ν���ڲ�ʹ�ã�ָ����û�����ֵ��¼�������ͨ�����������ʹ�����⣬�޷�ͨ���ٴδ򿪻�á�  
	if(lpName != NULL && _tcslen(lpName) > 0)  
	{  
		//��Ϊ�ڴ�ӳ���ļ����¼�����ͬһ�������ռ䣬���������ߵ����ֲ�����ͬ��������Ҫ�������ڴ�ӳ���ļ���Ϊ��EVENT_ǰ׺ + �¼�����  
		_stprintf_s(buf,MAX_PATH,TEXT("EVENT_%s\0"),lpName);
	}  
	else  
	{  
		//������¼�Ϊ�ڲ�ʹ�ã���ôҲ����ζ�����ڴ�ӳ���ļ�Ҳ���ڲ�ʹ�á��ʲ��ó�����������+�¼����ķ�ʽ�����ڴ�ӳ���ļ�������ȷ����  
		_stprintf_s(buf,MAX_PATH,TEXT("%ld_%ld\0"),(DWORD)GetModuleHandle(NULL),(DWORD)hEvent);
	}  
	return buf;
}

HANDLE QMemFile::CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,BOOL bManualReset,
	BOOL bInitialState,LPCTSTR lpName)
{  
	HANDLE hEvent = ::CreateEvent(lpEventAttributes,bManualReset,bInitialState,lpName);  
	if(hEvent != NULL)  
	{
		ME memFile;
		if (GetMemFile(lpName,memFile))
		{
			memFile.hEvent = hEvent;
			ms_LstMe.push_back(memFile);
			return hEvent;  
		}
	}
	::CloseHandle(hEvent);
	return NULL;
}  


BOOL CSingleInstance::InstanceAlreadyRun( LPCTSTR pszName,BOOL bBringLastTop/*=TRUE*/ )
{ 
    m_hEvent = QMemFile::CreateEvent(NULL,TRUE, FALSE, pszName);
    ASSERT (m_hEvent != NULL);
    if (NULL != m_hEvent)
    {
        SetEvent(m_hEvent);
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) // �Ѿ���һ��ʵ����������
    {
        if (bBringLastTop)
        {
            HWND hWnd = (HWND)QMemFile::GetEventData(m_hEvent);
            if (hWnd != NULL)
            {
                ShowWindow(hWnd,SW_HIDE);
                ShowWindow(hWnd,SW_RESTORE);
                quibase::SetForegroundWindowInternal(hWnd);
            }
        }
        return TRUE;
    }
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////
void CSingleInstance::RemoveRestrict()
{
    if (m_hEvent != NULL) 
    {
        QMemFile::CloseHandle(m_hEvent);
        m_hEvent = NULL;
    } 
}

HWND CSingleInstance::GetInstanceMainWnd()
{
    return (HWND)QMemFile::GetEventData(m_hEvent);
}

BOOL CSingleInstance::SetInstanceMainWnd( HWND hWnd )
{
    if (::IsWindow(hWnd))
    {
        QMemFile::SetEventData(m_hEvent,(DWORD)hWnd);
        return TRUE;
    }
    return FALSE;
}
