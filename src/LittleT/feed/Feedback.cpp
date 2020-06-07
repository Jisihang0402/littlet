#include "Feedback.h"
#include "ui/QUIDlgs.h"
#include "sys/SyncInvoker.h"
#include "ui/QConfig.h"
#include "AppHelper.h"
#include "deps/zmq.hpp"
#include "include/misc.h"

//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(QUserFeedbackWnd, QFrame)
    BN_CLICKED_ID(L"btn_submit", &QUserFeedbackWnd::OnclkSubmit)
    BN_CLICKED_ID(L"a_website", &QUserFeedbackWnd::OnclkWebsite)
//    MOUSE_UP_ID(L"txt_feed", &QUserFeedbackWnd::OnMouseupTxtFeed)
QUI_END_EVENT_MAP()

QUserFeedbackWnd::QUserFeedbackWnd( void )
    :QFrame(L"qabs:common/feedback.htm")
{
    // Ĭ��ÿ������ܷ���10������
    max_feed_ = 100; 
}

void QUserFeedbackWnd::OnclkSubmit( HELEMENT he )
{
    CStdString sContent = _TxtFeed().GetText();
    CStdString sContact = _TxtContact().GetText();

    if (sContent.Trim().GetLength() < 10)
    {
        XMsgBox::OkMsgBox(L"���д��10�����¡�");

        return;
    }

    feed_content_ = MakeContent(sContent, sContact);
    // �����û�����ϰ��ʽ
    SaveContactInfo();

    SENDFEEDBACK_RESULT lRet = SENDFEEDBACK_RESULT_OK;
    // ��������
    ECtrl tblInput = GetCtrl("#tbl_input");
    tblInput.EnableCtrl(FALSE);
    if ( CheckMaxFeedbackReached() )
    {
        // �Ѵﵽ�������ƣ���װ�ڷ��ͣ�Ȼ����ʾ�ɹ��ˣ�������
        LRESULT nouse = 0;
        SyncInvoke(QUserFeedbackWnd, AsyncFakeSend, NULL, nouse);
    }
    else
    {
        lRet = SendFeedback(feed_content_);
    }

    XMsgBox::OkMsgBox(FormatSendResult(lRet));
    switch(lRet)
    {
    case SENDFEEDBACK_RESULT_MAXSENDREACHED:// = -2,    // �ﵽÿ������ܷ��͵ķ�����Ŀ
        break;
    case SENDFEEDBACK_RESULT_OK:// = 0,
        IncreaseFeedback();
        // ɾ���������ļ�
        ::DeleteFile(_FeedbackBackpath());
        break;

    case SENDFEEDBACK_RESULT_NOCLIENTMAIL://,
    case SENDFEEDBACK_RESULT_NOSERVERMAIL://,
    case SENDFEEDBACK_RESULT_FAIL:// = -1,
        // ��¼��feedback�����л����ٷ���
        BackupFeedback();
        break;
    }
    // ���÷��Ͱ�ť
    tblInput.EnableCtrl(TRUE);
}

BOOL QUserFeedbackWnd::Show( const CStdString& sSubject , const CStdString& sSenderName)
{
    QUserFeedbackWnd* pThis = QUserFeedbackWnd::GetInstance();
    if (!pThis->IsWindow())
    {
        pThis->Create(QUIGetMainWnd(), WS_POPUP | WS_VISIBLE, 
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST);
    
        // ������ʾ
        pThis->CenterWindow();

        pThis->sender_name_ = sSenderName;
    }

    pThis->ShowWindow(SW_SHOW);

    return TRUE;
}

LRESULT QUserFeedbackWnd::OnDocumentComplete()
{
    RestoreFeedback();

    return 0;
}

QUserFeedbackWnd::SENDFEEDBACK_RESULT QUserFeedbackWnd::SendFeedback( 
    __in const CStdString& sFeedback)
{
    LRESULT lRet = 0;
    
    SyncInvoke(QUserFeedbackWnd, AsyncWebPost, NULL, lRet);
    
    return (SENDFEEDBACK_RESULT)lRet;
}

LRESULT QUserFeedbackWnd::AsyncWebPost( LPVOID )
{
    if (littlet::SendWebRequest(L"feed", feed_content_))
        return SENDFEEDBACK_RESULT_OK;
    return SENDFEEDBACK_RESULT_FAIL;
}

void QUserFeedbackWnd::OnclkWebsite( HELEMENT he )
{
    CStdString url = ECtrl(he).get_attribute("url_x");
    quibase::VisitWebsiteWithDefaultBrowser(url);
}

CStdString QUserFeedbackWnd::FormatSendResult( SENDFEEDBACK_RESULT s )
{
    CStdString sRet;
    switch (s)
    {
    case SENDFEEDBACK_RESULT_MAXSENDREACHED:// = -2,    // �ﵽÿ������ܷ��͵ķ�����Ŀ
    case SENDFEEDBACK_RESULT_OK:// = 0,
        sRet.Format(L"������ɣ�лл���Ľ��顣");
        break;

    case SENDFEEDBACK_RESULT_NOCLIENTMAIL://,
    case SENDFEEDBACK_RESULT_NOSERVERMAIL://,
    case SENDFEEDBACK_RESULT_FAIL:// = -1,
        sRet.Format(L"����ʧ��");
        break;
    }
    return sRet;
}

LRESULT QUserFeedbackWnd::AsyncFakeSend( LPVOID )
{
    // ˯��5�룬��װ���ڷ���
    Sleep(5000);

    return SENDFEEDBACK_RESULT_MAXSENDREACHED;
}

BOOL QUserFeedbackWnd::CheckMaxFeedbackReached()
{
    QConfig* pCfg = QUIGetConfig();
    QTime tmNow = QTime::GetCurrentTime();
    QTime tmDay = pCfg->GetTimeValue(L"APP", L"fbday");
    if (!tmDay.IsValid() || !tmNow.CompareDate(tmDay) )
    {
        CStdString str;
        str.Format(L"%lf", tmNow.SQLDateTime());
        pCfg->SetValue(L"APP", L"fbday", str);
        pCfg->SetValue(L"APP", L"fbsend", 0);

        return FALSE;
    }

    return pCfg->GetIntValue(L"APP", L"fbsend") >= max_feed_;
}

void QUserFeedbackWnd::IncreaseFeedback()
{
    QConfig* p = QUIGetConfig();
    p->SetValue(L"APP", L"fbsend", 
        p->GetIntValue(L"APP", L"fbsend") + 1);
}

BOOL QUserFeedbackWnd::BackupFeedback()
{
    CStdString str = _TxtFeed().GetText();
    if (str.GetLength() < 10)
        return FALSE;

    CTextFileWrite f(_FeedbackBackpath(), CTextFileBase::UTF_8);
    if (f.IsOpen())
    {
        f.Write(str);
        f.Close();

        return TRUE;
    }
    return FALSE;
}

BOOL QUserFeedbackWnd::RestoreFeedback()
{
    _TxtContact().SetText(
        QUIGetConfig()->GetValue(L"APP", L"user_contact"));

    CTextFileRead f(_FeedbackBackpath());
    if (f.IsOpen())
    {
        CStdString str;

        f.Read(str);
        f.Close();
       
        _TxtFeed().SetText(str);

        return TRUE;
    }


    return FALSE;
}

BOOL QUserFeedbackWnd::SaveContactInfo()
{
    QUIConfig* pCfg = QUIGetConfig();

    pCfg->SetValue(L"APP", L"user_contact", _TxtContact().GetText());

    return TRUE;
}

CStdString QUserFeedbackWnd::MakeContent( const CStdString& sTxt, const CStdString& sMail )
{
    CStdString str;
    str = L"�����\r\n";
    str += sTxt;
    str += L"\r\n��ϵ��ʽ��\r\n";
    str += sMail;
    return str;
}

CStdString QUserFeedbackWnd::_FeedbackBackpath()
{
    return quibase::GetModulePath() + L"feedback.dat";
}
