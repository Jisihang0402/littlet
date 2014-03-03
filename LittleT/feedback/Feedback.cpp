#include "Feedback.h"
#include "inet/CSmtp.h"
#include "ui/QUIDlgs.h"
#include "sys/SyncInvoker.h"
#include "ui/QConfig.h"
#include "FileBackup.h"

//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(QUserFeedbackWnd, QFrame)
    BN_CLICKED_ID(L"btn_submit", &QUserFeedbackWnd::OnclkSubmit)
    BN_CLICKED_ID(L"a_website", &QUserFeedbackWnd::OnclkWebsite)
//    MOUSE_UP_ID(L"txt_feed", &QUserFeedbackWnd::OnMouseupTxtFeed)
QUI_END_EVENT_MAP()

QUserFeedbackWnd::QUserFeedbackWnd( void )
    :QFrame(L"qabs:common/feedback.htm")
{
    addrs_ = QUIConnectCenter::GetInstance()->GetQiuInfo();

    // Ĭ��ÿ������ܷ���10������
    max_feed_ = 10; 
}

void QUserFeedbackWnd::OnclkSubmit( HELEMENT he )
{
    QString sContent = _TxtFeed().GetText();
    QString sContact = _TxtContact().GetText();

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

BOOL QUserFeedbackWnd::Show( const QString& sSubject , const QString& sSenderName)
{
    QUserFeedbackWnd* pThis = QUserFeedbackWnd::GetInstance();
    if (!pThis->IsWindow())
    {
        pThis->Create(QUIGetMainWnd(), WS_POPUP | WS_VISIBLE, 
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST);
    
        // ������ʾ
        pThis->CenterWindow();

        pThis->subject_ = sSubject;
        pThis->sender_name_ = sSenderName;
        if (pThis->subject_.Trim().IsEmpty())
        {
            pThis->subject_ = L"�������";
        }
        if (pThis->sender_name_.Trim().IsEmpty())
        {
            pThis->sender_name_ = APP_NAME;
        }
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
    __in const QString& sFeedback, __in BOOL bUseMail /*= TRUE*/ )
{
    // ��ʱֻ��ʹ��mail�ķ�ʽ
    ASSERT(bUseMail);

    LRESULT lRet = 0;
    if (bUseMail)
    {
        SyncInvoke(QUserFeedbackWnd, AsyncSendMail, NULL, lRet);
    }
    else
    {
        SyncInvoke(QUserFeedbackWnd, AsyncWebPost, NULL, lRet);
    }
    return (SENDFEEDBACK_RESULT)lRet;
}

LRESULT QUserFeedbackWnd::AsyncSendMail( LPVOID )
{
    // ��ȡclient�ʼ���ַ
    _CLIENT_MAIL cm;
    if (!addrs_->GetClientMail(cm))
        return SENDFEEDBACK_RESULT_NOCLIENTMAIL;

    // ��ȡserver mail
    QString sTo;
    if (!addrs_->GetMailAddr(sTo))
        return SENDFEEDBACK_RESULT_NOSERVERMAIL;

    if (littlet::SMTP_SendMail_SSL(cm, sender_name_, sTo, 
        L"Ziyue_yulu", subject_, feed_content_ ))
    {
        return SENDFEEDBACK_RESULT_OK;
    }

    return SENDFEEDBACK_RESULT_FAIL;
}

LRESULT QUserFeedbackWnd::AsyncWebPost( LPVOID )
{
    return SENDFEEDBACK_RESULT_FAIL;
}

void QUserFeedbackWnd::OnclkWebsite( HELEMENT he )
{
    QString url = ECtrl(he).get_attribute("url_x");
    quibase::VisitWebsiteWithDefaultBrowser(url);
}

QString QUserFeedbackWnd::FormatSendResult( SENDFEEDBACK_RESULT s )
{
    QString sRet;
    switch (s)
    {
    case SENDFEEDBACK_RESULT_MAXSENDREACHED:// = -2,    // �ﵽÿ������ܷ��͵ķ�����Ŀ
    case SENDFEEDBACK_RESULT_OK:// = 0,
        sRet.Format(L"�������");
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
        QString str;
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
    QString str = _TxtFeed().GetText();
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
        QString str;

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

QString QUserFeedbackWnd::MakeContent( const QString& sTxt, const QString& sMail )
{
    QString str;
    str = L"�����\r\n";
    str += sTxt;
    str += L"\r\n��ϵ��ʽ��\r\n";
    str += sMail;
    return str;
}
