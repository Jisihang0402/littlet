#ifndef _app_h__
#define _app_h__

#pragma once

#include "AppHelper.h"

namespace quibase
{
    
    /**
     *	ȷ���ļ�Ŀ¼·�����ڣ��������򴴽�	
     *
    **/
	BOOL MakeSureDirExist(LPCWSTR sDir);

    /**
     *	�����ַ�����GUID	
     *
    **/
	BOOL GuidFromString(LPCTSTR szGuid, __out GUID& guid);

    /**
     *	�ж��Ƿ���GUID
     *
    **/
	BOOL IsGuid(LPCTSTR szGuid);

    
    /**
     *	GUIDת�ַ���
     *
    **/
	BOOL GuidToString(const GUID& guid, __out CStdString& sGuid);

    /**
     *	GUID�Ƿ��
     *
    **/
	BOOL GuidIsNull(const GUID& guid);

    /**
     *	���ؿ�GUID
     *
    **/
	void NullGuid(__out GUID& guid);
	
    /**
     *	GUID��ͬ
     *
    **/
    BOOL SameGuids(const GUID& guid1, const GUID& guid2);

    /**
     *	�Ƿ����汻������
     *
    **/
	BOOL IsWorkStationLocked();

    /**
     *	�Ƿ���������ģʽ��
     *
    **/
	BOOL IsScreenSaverActive();

    /**  ��ȡ����·��
     *	return
     *      TRUE    ��ȡ�ɹ�
     *	param
     *		-[in]
     *          csidl       ���磺CSIDL_STARTUP
     *		-[out]
     *			sPath       ��ȡ����·��
    **/
    BOOL GetSpeialPath(__in int csidl, __out CStdString &sPath);

    /**
     *	�Ƿ񿪻��Զ�����	
     *
    **/
    bool IsAutoRun();               
    
    /**
     *	���������Զ�����	
     *
    **/
    void CreateAutoRun();           
    
    /**
     *	�Ƴ������Զ�����	
     *
    **/
    void RemoveAutoRun();           
    
    /**
     *	���ÿ����Զ�����	
     *
    **/
    void SetAutoRun(bool bAutoRun); 

    /**
     *	����Ȩ��	
     *
    **/
    BOOL UpgradeProcessPrivilege(); 
    
    /**
     *	�϶�ϵͳʱ��	
     *
    **/ 
    int  CorrectTime();             
    
    /**
     *	�϶�ʱ�䣬ͨ������Ȩ���ԼӲ����ķ�ʽ���г���ʵ��	
     *
    **/ 
    int  SyncTime();        

    /**
     *	����Դ�ļ��ͷų���	
     *
    **/ 
    BOOL ReleaseRes(LPCTSTR filename,WORD wResID, LPCTSTR filetype);  

    /**
     *	��Ĭ��������򿪣���Ĭ��ʱ��IE��	
     *
    **/ 
    void VisitWebsiteWithDefaultBrowser(LPCWSTR lpszUrl);             

    /**
     *	url�Ƿ�Ϊie��ҳ	
     *
    **/ 
    bool IsIEHomePage(LPCTSTR url);   
    
    /**
     *	����ie��ҳ, ����Ѿ�����ҳ�����ظ�����
     *
    **/ 
    bool SetIEHomePage(LPCTSTR url);  

    /**
     *	��������
     *
    **/ 
    void PinToTaskbar(LPCTSTR lpszDestPath);        

    /**
     *	ɾ����������
     *
    **/ 
    void UnPinFromTaskbar(LPCTSTR lpszDestPath);    

    /**
     *	����������
     *      lpszProgram     ΪNULL�����������ߵ�·����
     *                      �����������߿���ΪDLL
    **/
    BOOL CreateShortcut(LPCSTR lpszDestPath, LPCTSTR lpszProgram, LPCTSTR lpszIco,
        LPCTSTR lpszArguments, LPCTSTR lpszWorkingDir, LPCTSTR lpszDescription);

    /**
     * �Ƿ���PE�ļ�		
     *
    **/
    BOOL IsPEFile(LPCTSTR lpszPath);
};


#endif // apphelper_h__
