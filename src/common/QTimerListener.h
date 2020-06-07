#pragma once


typedef struct TASK_REMINDER_PARAM 
{
    int		nSeconds; // ��ǰ����������
    int		nTaskID; // ����id
    QTime	tmExec; // �����ִ��ʱ��
    CStdString sSound;	// �����ļ�·��
    CStdString sMsg; // ��ʾ��Ϣ
}*LPTASK_REMINDER_PARAM;

class QTimerEventHandlerBase
{
public:
    virtual void OnTimerReminderSetted(TASK_REMINDER_PARAM* pParam)
    {

    }
};

