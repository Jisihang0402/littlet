// LittleTShell.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ArgParser.h"
#include "../common/LittleTcmn.h"

#ifdef APP_NAME
#define APP_NAME L"LittleTShell"
#endif

#ifdef _DEBUG
#pragma comment(lib, "quil_d")
#else
#pragma comment(lib, "quil")
#endif

CStdStringW QUIGetAppName()
{
    return L"LittleTShell";
}

#include "AppHelper.h"

void ShowHelp()
{
//    wcout<<L"LittleTShell (-m|-e|-s|-b) [-w] (-r|-b) [-i]";
    wcout<<con::fg_white
         <<L"LittleTShell �������� \n"
         <<con::fg_green
         <<L"    -h      ��ʾ����\n"
         <<con::fg_white
         <<endl;
    wcout<<L"[1] ������𣨱������\n"
         <<L"-------------------------------------------------------------\n"
         <<con::fg_green
         <<L"    -m  \"message\"         ��ʾ��Ϣ\n"
         <<L"    -e  \"�ļ�·��\"        ִ�г���\n"
         <<L"    -s  �ػ����޲�����\n"
         <<L"    -b  ��Ϣһ������޲�����\n"
         <<con::fg_white
         <<endl;
    wcout<<L"[2] ����ִ��ʱ�䣨�����\n"
         <<L"-------------------------------------------------------------\n"
         <<con::fg_green
         <<L"    -r  \"�����;���ʱ��;ִ�д���\" \n"
         <<con::fg_gray
         <<L"        �����[p|s|t]\n"
         <<L"            p   LittleT������  eg: p:20m; LittleT������20��\n"
         <<L"            s   ϵͳ������     eg: s:30s; ����ϵͳ������30��\n"
         <<L"            t   ���񴴽���     eg: t:1h;  ���񴴽���1Сʱ\n"
         <<L"\n"
         <<L"        ���ʱ�䣨��ѡ��   n[h|s|m]\n"
         <<L"            ����: 20m;     ÿ��20��ִ��һ��\n"
         <<L"\n"
         <<L"        ִ�д�������ѡ��  \n"
         <<L"            ����: 30       ִ��30��ֹͣ��\n"
         <<L"\n"
         <<con::fg_cyan
         <<L"    ���磺 -r \"s:30m;1h;100\"     ��ϵͳ������30��ִ������Ȼ��ÿ��1Сʱִ��һ�Σ�ִ��100�κ������"
         <<L"\n"
         <<endl;
    wcout<<con::fg_green
         <<L"    -a  \"��������;����ʱ���\"\n"
         <<con::fg_gray
         <<L"        ��������      [d|s|w|m]\n"
         <<L"            d:date     ��������        eg: d:2013/5/5  ��2013/5/5ִ��\n"
         <<L"            s:n        ÿ����          eg: s:1       ÿһ�춼ִ��\n"
         <<L"            w:1,2...   ���ڼ�          eg: w:1,3    ÿ��1��3ִ�С�0����������\n"
         <<L"            m:1,2...   ÿ�µļ���       eg: m:1,3    ÿ�µ�1��3��ִ�С�\n"
         <<L"\n"
         <<L"        ����ʱ��㣬���ʱ�����Ӣ��','�ָ�\n"
         <<L"            ���磺 10:20,11:30:25      �ڷ������ڵ�10:20��11:30:25ִ��\n"
         <<L"\n"
         <<con::fg_cyan
         <<L"    ���磺-b \"w:0,6;8:00,11:00\" ���������յ�8:00��11:00ִ��"
         <<endl;
    wcout<<con::fg_white
         <<L"\n[3] ������Ч�ڣ���ѡ�"
         <<L"\n-----------------------------------------------------------\n"
         <<con::fg_green
         <<L"    -l  \"��ʼʱ��;����ʱ��\"\n"
         <<con::fg_gray
         <<L"        ��ʼʱ��    eg: 2013/5/5 10:00:00     �������д��Ĭ�ϴӵ�ǰ��ϵͳʱ�俪ʼ\n"
         <<L"        ����ʱ��    eg: 2013/6/2 10:00:00     �������д��Ĭ�ϵ���ʼʱ���3���\n"
         <<con::fg_cyan
         <<L"\n"
         <<L"    ���磺-l \"2013/5/5; 2013/6/5\"    ��Ч��Ϊ��2013/5/5 0:0:1 �� 2013/6/5 23:59:59\n"
         <<endl;
    wcout<<con::fg_white
         <<L"[4] ������ʾ����ѡ���\n"
         <<L"-------------------------------------------------------------\n"
         <<con::fg_green
         <<L"    -w  \"ʱ��;����;��Ϣ\"\n"
         <<con::fg_gray
         <<L"        ʱ���Ǳ������������Ϣ��ͬʱ��������һ��\n"
         <<L"        ѡ��֮����Ӣ��';'�ָ��ָ������Ĭ��Ϊ��\n"
         <<L"        ʱ��      n[s|m|h]        eg:20m\n"
         <<L"        ����      ���ŵ�����      eg:C:\\dir\\sound.wav\n"
         <<L"        ��Ϣ      ��ʾ����Ϣ      eg:���Ͳ���\n"
         <<L"\n"
         <<con::fg_cyan
         <<L"    ���磺 -w \"20m;;�����ػ�\" ��ÿ������ִ��ǰ20������ʾ\"�����ػ�\"\n"
         <<con::fg_white
         <<endl;
    wcout<<con::fg_magenta
         <<L"[5] �������ӣ�\n"
         <<L"-------------------------------------------------------------\n"
         <<con::fg_cyan
         <<L"LittleTShell -s -w \"5m;;�����ػ�\" -a \"s:1;22:00\" -l \";2015/6/5\" \n"
         <<con::fg_gray
         <<L"    ��ʾ�������ڵ�2015/6/5��ÿ������22:00�Զ��ػ������ڹػ�֮ǰ5������ʾ�û�\"�����ػ�!\""
         <<endl;
    wcout<<L"\n"
         <<con::fg_cyan
         <<L"LittleTShell -b -r \"s:1h;1h;\"  \n"
         <<con::fg_gray<<con::bg_black
         <<L"    ��ʾ����ʹ�õ��ԵĹ����У�ÿ��1Сʱ��ʾһ������������ʾ�����ɽ�����"
         <<endl<<endl;
}

HWND GetLittleTMainWnd()
{
    HANDLE hEventMain = QMUF::CreateEvent(NULL,TRUE, 
        FALSE, LITTLET_MAINWND_STARTUP_EVENTNAME);
    if (GetLastError() != ERROR_ALREADY_EXISTS) 
    {   // �����ڻ�û�����������Ƿ��Ѿ������У�
        // ���ȼ��LittleT�Ƿ��Ѿ�������
        CSingleInstance theInst;
        if ( !theInst.InstanceAlreadyRun(LITTLET_APP_NAME) )
        { // ����û�����أ�������
            theInst.RemoveRestrict();

            CStdStringW sWorkDir = quibase::GetModulePath();
            CStdStringW sExe = sWorkDir + L"LittleT.exe";
            if (32 >= (int)ShellExecute(NULL,L"open", sExe, 
                NULL, sWorkDir, SW_SHOWNORMAL))
            {
                // ��������ʧ��
                return NULL;
            }
        }
        // �ȵ�LittleT�������ڴ�������
        if (WAIT_OBJECT_0 != WaitForSingleObject(hEventMain, 30 * 1000)) 
        {// 30��Ӧ�����㹻��
            return NULL;
        }
        // ��ʱ����Ӧ���������ĵģ�����Ҳ����������
        if (theInst.InstanceAlreadyRun(LITTLET_APP_NAME, FALSE))
        {
            return theInst.GetInstanceMainWnd();
        }
    }
    return NULL;
}

BOOL SendRequestToLittleT(ENUM_AUTOTASK_DOWHAT nDoWhat, const CStdStringW& sDoWhatParam,
    const CStdStringW& sWhenDo, const CStdStringW& sRemindexp, QTime tmBegin, QTime tmEnd,
    __out CStdStringW& sError)
{
    HWND hWnd = GetLittleTMainWnd();
    if (NULL == hWnd)
    {
        wcout<<con::fg_red<<L"�����̣�LittleT.exe������ʧ�ܣ�"<<con::fg_white<<endl;
        return FALSE;
    }

    _LittleTShell_CopyData_Param pra;
    ZeroMemory(&pra, sizeof(pra));
    StringCchCopy(pra.szDoWhatParam, MAX_PATH, sDoWhatParam);
    StringCchCopy(pra.szRemindExp, MAX_PATH, sRemindexp);
    StringCchCopy(pra.szWhenDo, MAX_PATH, sWhenDo);
    pra.eDoWhat = nDoWhat;
    pra.tmBegin = tmBegin.m_dt;
    pra.tmEnd = tmEnd.m_dt;

    COPYDATASTRUCT stCDS;
    stCDS.dwData = LITTLET_SHELL_COMMANDCODE_ADDAUTOTASK;
    stCDS.cbData = sizeof(pra);
    stCDS.lpData = &pra;

    if (SendMessage(hWnd , WM_COPYDATA, NULL, (LPARAM)&stCDS))
    {
        sError = pra.szResult;
        return TRUE;
    }
    else
    {
        sError = L"����δ����";
        return FALSE;
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
    ::CoInitialize(NULL);

    // �������
    cout.imbue(locale("chs"));
    wcout.imbue(locale("chs"));

    // �����Բ��ԣ�
    QArgvParser aParser;
    if ( !aParser.SetArgv(argv, argc) )
    {
        ShowHelp();
    }
    else
    {
        // �����Բ��ԣ�
        ENUM_AUTOTASK_DOWHAT eDoWhat; 
        CStdStringW sDoWhatParam, sWhenDo ,sRemindExp;
        QTime tmBegin , tmEnd;
        if (aParser.ParseArgs(eDoWhat, sDoWhatParam, 
                sWhenDo, sRemindExp, tmBegin, tmEnd))
        {
            // �����ɹ���ͨ���ܵ������ݴ���LittleT
            CStdStringW sError;
            if (!SendRequestToLittleT(eDoWhat, sDoWhatParam, 
                sWhenDo, sRemindExp, tmBegin, tmEnd, sError))
            {
                std::wcout<<con::fg_red<<L"����δ������"<<sError<<con::fg_white<<endl;
            }
            else
            {
                std::wcout<<con::fg_red<<L"�����Ѵ���"<<con::fg_white<<endl;
            }
        }
        else
        {
            aParser.EchoErrors();
        }
    }
    ::CoUninitialize();
    return 0;
}

