#pragma once

#include "ui/QConfig.h"
#include "file/rjson/inc.h"
#include "ui/QWindow.h"
#include "QUIConnectCenter.h"

// �û��������ϵͳ
//  1> ʹ��http�������ݵ�web������
//  2> ʹ��smtp+SSL�������ݵ�email������

class QUserFeedbackWnd : public QFrame
{
    typedef QUIConnectCenter::_QiuInfo _QiuInfo;

    QUI_DECLARE_EVENT_MAP;

    SINGLETON_ON_DESTRUCTOR(QUserFeedbackWnd)
    {

    }

    BEGIN_MSG_MAP_EX(QUserFeedbackWnd)
        CHAIN_MSG_MAP(QFrame)
    END_MSG_MAP()

public:
    QUserFeedbackWnd(void);
    
    /*
     *	sSubject    ����
     *  sSenderName ���ͷ�����
     *  
     */
    static BOOL Show(const QString& sSubject = L"USER FEEDBACK:",
        const QString& sSenderName = APP_NAME);

protected:
    LRESULT OnMSG_Feedback(UINT uMsg, WPARAM w, LPARAM l);

    LRESULT OnMSG_FeedbackFinished(UINT uMsg, WPARAM w, LPARAM l);

    virtual LRESULT OnDocumentComplete();

    void OnclkSubmit(HELEMENT he);
 
    void OnclkWebsite(HELEMENT he);

    void OnMouseupTxtFeed(MOUSE_PARAMS &);

    // �첽��ʽ�����ʼ�
    LRESULT AsyncSendMail(LPVOID);
    // �첽��ʽPost������
    LRESULT AsyncWebPost(LPVOID);
    // ��װ���ڷ�����
    LRESULT AsyncFakeSend(LPVOID);

    enum SENDFEEDBACK_RESULT
    {
        SENDFEEDBACK_RESULT_MAXSENDREACHED = -2,    // �ﵽÿ������ܷ��͵ķ�����Ŀ
        SENDFEEDBACK_RESULT_FAIL = -1,
        SENDFEEDBACK_RESULT_OK = 0,
        SENDFEEDBACK_RESULT_NOCLIENTMAIL,
        SENDFEEDBACK_RESULT_NOSERVERMAIL,
    };

    QString FormatSendResult(SENDFEEDBACK_RESULT s);

    /**
     *	���ͷ���
     *
     *	@param
     *		-[in]
     *          sFeedback       ���
     *          bUseMail        �Ƿ�ʹ���ʼ�����Ļ���post��web��
    **/
    SENDFEEDBACK_RESULT SendFeedback(__in const QString& sFeedback, __in BOOL bUseMail = TRUE);

    BOOL CheckMaxFeedbackReached();

    // ��һ���ͼ�¼
    void IncreaseFeedback();

    // ����δ���͵�����
    BOOL BackupFeedback();

    BOOL RestoreFeedback();
   
    QString _FeedbackBackpath()
    {
        return quibase::GetModulePath() + L"feedback.dat";
    }

    EEdit _TxtFeed()
    {
        return GetCtrl("#txt_feed");
    }

    EEdit _TxtContact()
    {
        return GetCtrl("#txt_mail");
    }

    BOOL SaveContactInfo();

    QString MakeContent(const QString& sTxt, const QString& sMail);

private:    
    int         max_feed_;  // ÿ������ܷ��Ͷ��ٷ���
    QUIConnectCenter::_QiuInfo*   addrs_;    // ��ַ
    QString     subject_;   // �ʼ����⡢���
    QString     feed_content_;
    QString     sender_name_;
};

