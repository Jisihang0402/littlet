#pragma once

#include "ui/QFrame.h"

#pragma comment(lib, "libzmq.lib")

// �û��������ϵͳ
//  1> ʹ��http�������ݵ�web������
//  /// 2> ʹ��smtp+SSL�������ݵ�email������
class QUserFeedbackWnd : public QFrame
{
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
    static BOOL Show(const CStdString& sSubject = L"USER FEEDBACK:",
        const CStdString& sSenderName = APP_NAME);

protected:
    LRESULT OnMSG_Feedback(UINT uMsg, WPARAM w, LPARAM l);

    LRESULT OnMSG_FeedbackFinished(UINT uMsg, WPARAM w, LPARAM l);

    virtual LRESULT OnDocumentComplete();

    void OnclkSubmit(HELEMENT he);
 
    void OnclkWebsite(HELEMENT he);

    void OnMouseupTxtFeed(MOUSE_PARAMS &);

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

    CStdString FormatSendResult(SENDFEEDBACK_RESULT s);

    /**
     *	���ͷ���
     *
     *	@param
     *		-[in]
     *          sFeedback       ���
    **/
    SENDFEEDBACK_RESULT SendFeedback( __in const CStdString& sFeedback);
    
    BOOL CheckMaxFeedbackReached();

    // ��һ���ͼ�¼
    void IncreaseFeedback();

    // ����δ���͵�����
    BOOL BackupFeedback();

    BOOL RestoreFeedback();
   
    CStdString _FeedbackBackpath();

    EEdit _TxtFeed()
    {
        return GetCtrl("#txt_feed");
    }

    EEdit _TxtContact()
    {
        return GetCtrl("#txt_mail");
    }

    BOOL SaveContactInfo();

    CStdString MakeContent(const CStdString& sTxt, const CStdString& sMail);

private:    
    int         max_feed_;  // ÿ������ܷ��Ͷ��ٷ���
    CStdString     feed_content_;
    CStdString     sender_name_;
};

