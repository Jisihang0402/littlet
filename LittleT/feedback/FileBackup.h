#pragma once

#include "UserInfo.h"
#include "inet/CSmtp.h"

// enum ENUM_AUTOTASK_DOWHAT_CUSTOM
// {
//     // �ļ�����
//     AUTOTASK_DOWHAT_CUSTOM_AUTOBACKUP = AUTOTASK_DO_USERCUSTOM + 1,
// };

// class YuluATaskEventHandler : public DATaskEventHandlerBase
// {
// public:
//     virtual BOOL OnTaskFiredRequest(DATaskItem *pTask)
//     {
//         if (!pTask->IsUserCustomTask())
//             return TRUE;
// 
//         switch (pTask->DoWhat())
//         {
//         case AUTOTASK_DOWHAT_CUSTOM_AUTOBACKUP:
//             {
//                 break;
//             }
//         }
// 
//         // �����Լ�����
//         return FALSE;
//     }
// };

namespace littlet
{
    /*
     *	ʹ��SMTPЭ�� + SSL �����ʼ�
     *      
     *      ����ֵ����
     *          ECSmtp::CSmtpError
     */
    LRESULT SMTP_SendMail_SSL( 
        __in _CLIENT_MAIL& cmFrom, __in const QString& sSenderName,
        __in const QString& sTo, __in const QString& sToName, 
        __in const QString& sSubject, __in const QString& sContent,
        __in const QString& sAttachFile = L"");

    /**
     *  �����ض����͵��Զ����� 
     *      ENUM_AUTOTASK_DOWHAT/ ENUM_AUTOTASK_DOWHAT_CUSTOM
    **/
/*    DATaskItem* FindAutoTask(__in int nDoWhat);*/

//     inline DATaskItem* FindAutoTask_AUTOBACKUP()
//     {
//         return FindAutoTask(AUTOTASK_DOWHAT_CUSTOM_AUTOBACKUP);
//     }

}

// ʹ�����������ݱ���
class CFileBackup
{
public:
    CFileBackup(void);
    ~CFileBackup(void);

    /*
     *	��������
     */
    BOOL BackupNow();

    /*
     *  �����Զ���������
     *	    dwPeriodMinute  ��С30
     */
    void SetAutoBackup(__in BOOL bAuto = TRUE, __in DWORD dwPeriodMinute = 30);

protected:
public:
    /*
     *	�����ļ�������
     *      sFile   �ļ�ȫ·��
     */
    BOOL BackupToUserMail(__in const QString& sFile);
private:
    DWORD       period_;    
    BOOL        auto_backup_;
};

