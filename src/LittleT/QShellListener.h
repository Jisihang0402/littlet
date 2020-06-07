#pragma once

#include "BaseType.h"
#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include <strsafe.h>

// ����LittleTShell�Ĺܵ���Ϣ

class QShellListener
{
    SINGLETON_ON_DESTRUCTOR(QShellListener)
    {
        Shutdown();
    }

public:
    QShellListener(void);

    // ��������
    BOOL Startup();
    
    // �رռ���
    BOOL Shutdown();

protected:
    // �����̣߳��ڳ��������ڼ�һֱ����
    static UINT_PTR __stdcall ListenThread(void* parm);
   
    /** ����Shell����������Ϣ
     *	return
     *      TRUE    ��Ϣ���ɹ�����
     *	param
     *		-[in]
     *          pMsg        ��Ϣ
     *          nLen        ��Ϣ����
     *		-[out]	
     *          sReply      ����Shell�Ļظ�
    **/
    BOOL HandleShellRequest(__in wchar_t* pMsg, __in DWORD nLen, __out CStdString& sReply);
    /** �ظ�Shell������
     *	return
     *      TRUE    �ɹ��ظ�
     *	param
     *		-[in]
     *          hPipe   �ܵ�
     *          sPeply  ��Ϣ
    **/
    BOOL ReplyShellRequest(__in HANDLE hPipe, __in const CStdString& sReply);
    /** Shell ���ӵ�Pipe
     *	return
     *	param
     *		-[in]
     *		-[out]	
     *
    **/
    BOOL HandleShellConnected(HANDLE hPipe);
private:
    BOOL    m_bStarted;
    BOOL    m_bContinueListen;
};

