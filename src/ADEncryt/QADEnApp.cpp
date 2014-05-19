#include "QADEnApp.h"
#include "apphelper.h"
#include "ui/QUIGlobal.h"
#include "../common/LittleTUIcmn.h"

QADEnApp theApp;
QADEnApp::QADEnApp(void)
{
	
}

QADEnApp::~QADEnApp(void)
{
}

BOOL QADEnApp::InitRun()
{
	SetMainWnd(&m_MainWnd);

	if (!m_MainWnd.Startup())
	{
		XMsgBox::ErrorMsgBox(L"���崴��ʧ�ܣ�");
		return FALSE;
	}

	m_MainWnd.ShowWindow(SW_SHOW);
	return TRUE;
}

CStdString QADEnApp::GetConfigPath()
{
	return quibase::GetModulePath() + L"Config.ini";
}


//////////////////////////////////////////////////////////////////////////
QADEnCfg theConfig;
QADEnCfg::QADEnCfg(void)
{
}

QADEnCfg::~QADEnCfg(void)
{
}

CStdString QADEnCfg::GetMainQSS()
{
	return L"ADEn.zip";
	
}