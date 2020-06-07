#pragma once

#include "resource.h"

// xmsgbox
#ifndef QUI_USE_COLORBTN
#   define QUI_USE_COLORBTN
#endif

#include "ui/QApp.h"
//#include "LMainWnd.h"
#include "LittleTFrame.h"
#include "AppHelper.h"

#ifdef _DEBUG
#pragma comment(lib, "quil_d.lib")
#else
#pragma comment(lib, "quil.lib")
#endif

class LittleTApp : public QApp
{
public:
    LittleTApp();

    ~LittleTApp();

    virtual BOOL InitRun();

    virtual LPCWSTR GetAppVersion()const;

    BOOL CheckUpdate();

public:
    BOOL        m_bStartUpdater;
    CStdString     m_sUpdateFile;

protected:
    void StartUpdateExe();
    // ����"���³�����û�и���
    void CheckUpdaterExe();

private:
    //    LMainWnd    m_MainFrm;
    LittleTFrame    m_frame;

    // �����ڴ��������󣬴�������¼����˺�������ں���������ͨ��
    HANDLE      m_hEventMainWnd;
};


#include "ui/QConfig.h"

class LittleTConfig : public QUIConfig
{
public:
    // ��Ϣһ�����Ĭ�ϲ����ļ�Ŀ¼
    CStdString  GetPicFolder()
    {
        CStdString sRet = GetValue(L"setting",L"pic_folder");
        if (sRet.Find(L':') == -1)
        {   // ���·����ת��Ϊ����·��
            return quibase::GetModulePath() + sRet;
        }
        return sRet;
    }
    // ��Ϣһ�����Ĭ�ϲ����ļ����ʱ��
    int GetPicSec()
    {
        return GetIntValue(L"setting",L"pic_sec");
    }

    // ��Ϣһ�����Ĭ����ʾʱ��
    int GetBreakSec()
    {
        return GetIntValue(L"setting",L"break_sec");
    }
    
    // ��Ϣһ�����Ĭ��͸����
    int GetPicAlpha()
    {
        return GetIntValue(L"setting",L"pic_alpha");
    }

    // ��Ϣһ�����Ĭ�ϱ���ɫ
    DWORD GetPicBkcolor()
    {
        return GetIntValue(L"setting",L"pic_bkcolor");
    }

    // ��ȡͼ������ͼ��Ŀ¼
    CStdString GetIconsDir()
    {
        return quibase::GetModulePath() + L"icons/";
    }

public:
    void SetBreakFolder(LPCWSTR szFolder)
    {
        SetValue(L"setting", L"pic_folder", szFolder);
    }

    void SetBreakSec(int nSec)
    {
        SetValue(L"setting", L"break_sec", nSec);
    }

    void SetBreakSpan(int nSec)
    {
        SetValue(L"setting", L"pic_sec", nSec);
    }

    void SetBreakBkcolor(DWORD dwClr)
    {
        SetValue(L"setting", L"pic_bkcolor", dwClr);
    }

    void SetBreakAlpha(int nAlpha)
    {
        SetValue(L"setting", L"pic_alpha", nAlpha);
    }
};




