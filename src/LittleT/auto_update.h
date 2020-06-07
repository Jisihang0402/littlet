#pragma once

#include "BaseType.h"
#include "QBuffer.h"
#include "AppHelper.h"

struct _Url 
{
    CStdString     url_;
    CStdString     domain_;
};

#define UPDATION_TEMP_FILE L"__temp_update.zip"

inline CStdString __UpdationTempFile()
{
    return quibase::GetModulePath() + UPDATION_TEMP_FILE;
}

#define BASEDATA_CACHE_FILE L"__basedata.dat"

inline CStdString __BasedataCacheFile()
{
    return quibase::GetModulePath() + BASEDATA_CACHE_FILE;
}

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
*                      lParam  1��baseData ����
*                              0��Ӧ�ó������
*                      sParam  �����صĸ����ļ�·��
*                      bFreeIt TRUE    ������Ӧ��ɾ���������
*		-[out]
*      �����
**/
DECLARE_USER_MESSAGE(QSOFT_MSG_UPDATEAPP);

class QAutoUpdater
{
    static VOID CALLBACK CheckUpdateCallback(
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
    BOOL Startup(__in _Url& urlUpdation, __in _Url& urlBaseData, __in DWORD dwCheckPeriod = 30);
    //BOOL Startup(__in DWORD dwCheckPeriod = 30);
protected:
    /** �첽��ʽ���������.
    *      ����������ɹ����������󽫻������������ڷ���һ��QSOFT_MSG_UPDATEAPP��Ϣ
    *	return:
    *      TRUE    �����ɹ�
    **/
    BOOL CheckUpdate( );
    // BOOL CheckBaseData( );

    enum ENUM_TASK_TYPE 
    {
        TASK_TYPE_UPDATION = 1,
        TASK_TYPE_BASEDATA = 2,
    };

    // �����̻߳ص�����
    struct _Download_Param 
    {
        _Download_Param (ENUM_TASK_TYPE t)
            :pData(nullptr), type_(t)
        {

        }

        CStdString sURL;	// in
        CStdString sRefer;	// in
        LPVOID	pData; // in
        QBuffer	bufDown;	// out ���ص�����

        inline BOOL IsUpdationTask()const
        {
            return TASK_TYPE_UPDATION == type_; 
        }

    private:
        ENUM_TASK_TYPE type_;   // ��������
    };
    /** �������ݵ��߳�
    *	params:
    *		-[in]
    *          param            _Download_Param* 
    **/
    static UINT_PTR __stdcall thread_download(void* param);
private:
    //BOOL				m_bEnd;
    HANDLE              m_hTimer;   // �����µ�ʱ�䶨ʱ��
    //     QString             m_sUrl;
    //     QString             m_sRefer;   // �����ƽ������
    _Url        url_updation_;
    // _Url        url_basedata_;
};

