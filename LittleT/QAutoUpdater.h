#pragma once

#include "BaseType.h"
#include "QBuffer.h"

/** ����Ӧ�ó����ļ�
 *	return:
 *      �޷���
 *	params:
 *		-[in]
 *          wParam      1���и��£� 0����ǰ�޸���
 *          lParam      ��wParam==0������ҲΪ0 
 *                        wParam==1������ΪLPQUI_USERMSGPARAM��
 *  -----------------------------------------
 *          LPQUI_USERMSGPARAM ���ֵΪ��
 *                      wParam  1�����³ɹ�
 *                              0������ʧ��
 *                      sParam  �����صĸ����ļ�·��
 *                      bFreeIt TRUE    ������Ӧ��ɾ���������
 *		-[out]
 *      �����
**/
DECLARE_USER_MESSAGE(QSOFT_MSG_UPDATEAPP);

VOID CALLBACK CheckUpdateCallback(
    __in  PVOID lpParameter,__in  BOOLEAN TimerOrWaitFired);
class QAutoUpdater
{
    friend VOID CALLBACK CheckUpdateCallback(
            __in  PVOID lpParameter,__in  BOOLEAN TimerOrWaitFired);
    
    SINGLETON_ON_DESTRUCTOR(QAutoUpdater)
    {
        if (NULL != m_hTimer)
        {
            DeleteTimerQueueTimer(NULL, m_hTimer, NULL);
        }
    }
public:
    QAutoUpdater(void);

    /** �����Զ����³���
     *	return:
     *      TRUE    �����ɹ�
     *	params:
     *		-[in]
     *          dwCheckPeriod   �����µ�Ƶ�ʣ���λʱ�䣨�֣�
     *                          ��С���Ƶ��Ϊ10����
     *          sURL            �����ַ
     *          szRefer         �ƽ������ �����ã�http://www.sina.com��
     *
    **/
    BOOL Startup( __in const QString& sURL, __in LPCWSTR szRefer,
            __in DWORD dwCheckPeriod = 30);

protected:
    /** �첽��ʽ���������.
     *      ����������ɹ����������󽫻������������ڷ���һ��QSOFT_MSG_UPDATEAPP��Ϣ
     *	return:
     *      TRUE    �����ɹ�
    **/
    BOOL CheckUpdate( );

    // �����̻߳ص�����
    struct _Download_Param 
    {
        QString sURL;	// in
        QString sRefer;	// in
        LPVOID	pData; // in
        QBuffer	bufDown;	// out ���ص�����
    };
    /** �������ݵ��߳�
     *	params:
     *		-[in]
     *          param            _Download_Param* 
    **/
    static uint __stdcall thread_download(void* param);

private:
    //BOOL				m_bEnd;
    HANDLE              m_hTimer;   // �����µ�ʱ�䶨ʱ��
    QString             m_sUrl;
    QString             m_sRefer;   // �����ƽ������
};

