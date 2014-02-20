#pragma once

//////////////////////////////////////////////////////////////////////////
// һ���޴��ڵĶ�ʱ��ʵ��
// ʹ�÷�����
//		1������ȫ�ֺ��� ��StkSetTimer���趨��ʱ��
//		2������ȫ�ֺ��� ��StkKillTimer��ɾ����ʱ��
//////////////////////////////////////////////////////////////////////////

#include <Windows.h>

class StkTimerMan
{
private:
	StkTimerMan() 
	{
		InitTimerQueue();
	}
	~StkTimerMan()
    {
        Destroy();
    }

protected:
	BOOL InitTimerQueue()
	{
		if (NULL == m_hTimerQueue)
		{
			m_hTimerQueue = CreateTimerQueue();
		}
		return NULL != m_hTimerQueue;
	}

public:
	static StkTimerMan* GetInstance()
	{
		static StkTimerMan _theInstance;
		return &_theInstance;
	}
	void Destroy()
	{
		if (NULL != m_hTimerQueue)
		{
			// ���ɾ����־�����ȴ�����ִ�еĻص���ɣ���������
			DeleteTimerQueueEx(m_hTimerQueue,NULL);
            m_hTimerQueue = NULL;
		}
	}

public:
	/** �趨��ʱ����MSDN��By default, the callback function is queued to a non-I/O worker thread.
	 *	-return:	
	 *			HANDLE		����ɹ������ض�ʱ����������ʧ�ܣ�����NULL
	 *						�൱�ڴ��ڣ�HWND����ʱ����WM_TIMER���Ķ�ʱ��ID
	 *	-params:	
	 *		-[in]	dwMillsec	��ʱ���������λ���� ��ms��
	 *				pCallback	�ص�����������ԭ��Ϊ��
	 *					VOID CALLBACK WaitOrTimerCallback(
								 __in  PVOID lpParameter,
								 __in  BOOLEAN TimerOrWaitFired );
					pData	�ص�������������ʱ�򣬴���ĵ�һ������ֵ���������ԭ��
	 **/
	HANDLE SetTimer(DWORD dwMillsec,WAITORTIMERCALLBACK pCallback,LPVOID pData)
	{
		InitTimerQueue();

		HANDLE hTimer;
		BOOL bOK = CreateTimerQueueTimer(&hTimer,m_hTimerQueue,pCallback,
			pData,dwMillsec,dwMillsec,WT_EXECUTEDEFAULT);
		return bOK ? hTimer : NULL;
	}
	
	void KillTimer(HANDLE hTimer,BOOL bWaitForComplete=FALSE)
	{
		if ((NULL != hTimer) && (NULL != m_hTimerQueue))
		{
            __try
            {
                DeleteTimerQueueTimer(
                    m_hTimerQueue,
                    hTimer,
                    bWaitForComplete ? INVALID_HANDLE_VALUE : NULL);
            }
            __except(EXCEPTION_CONTINUE_EXECUTION)
            {
                // �ڶ�ʱ���ص���ִ��ɾ����ʱ�������п��ܻ�����쳣
                //....
            }
		}
	}

private:
	HANDLE	m_hTimerQueue;
};

/** �趨��ʱ����MSDN��By default, the callback function is queued to a non-I/O worker thread.
	*	-return:	
	*			HANDLE		����ɹ������ض�ʱ����������ʧ�ܣ�����NULL
	*						�൱�ڴ��ڣ�HWND����ʱ����WM_TIMER���Ķ�ʱ��ID
	*	-params:	
	*		-[in]	dwMillsec	��ʱ���������λ���� ��ms��
	*				pCallback	�ص�����������ԭ��Ϊ��
	*					VOID CALLBACK WaitOrTimerCallback(
								__in  PVOID lpParameter,
								__in  BOOLEAN TimerOrWaitFired );
				pData	�ص�������������ʱ�򣬴���ĵ�һ������ֵ���������ԭ��
	**/
inline HANDLE StkSetTimer(DWORD dwMillsec,WAITORTIMERCALLBACK pCallback,LPVOID pData=NULL)
{
	return StkTimerMan::GetInstance()->SetTimer(dwMillsec,pCallback,pData);
}

/** ɾ����ʱ��������ص�����ִ��
	 *	-params:	
	 *		-[in]	hTimer	��ҪKill�Ķ�ʱ�����
	 *						��������Ҫȷ���˾��ΪSetTimer���õķ���ֵ
	 *						
	 *				bWaitForComplete	
	 *					�˲���ָʾ���ô˺�����ʱ���Ƿ�ȴ��Ѽ����Ļص�����ٷ���
	 *					���ΪTRUE����ô�˺����������ȴ��ص���ɺ󷵻�
	 *					���ΪFALSE��������������
	 **/
inline void StkKillTimer(HANDLE hTimer,BOOL bWaitForComplete=FALSE)
{
	StkTimerMan::GetInstance()->KillTimer(hTimer,bWaitForComplete);
}


