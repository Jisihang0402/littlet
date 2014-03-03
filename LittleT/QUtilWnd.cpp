#include "QUtilWnd.h"

QScreenWnd::QScreenWnd()
    :QFrame(L"qabs:dlgs/picwnd.htm")
{
}

BOOL QScreenWnd::Show( LPCWSTR pszPicFolder, int nAlpha /*= 200*/,
    COLORREF crBk /*= RGB(88,93,245)*/,int nSeconds/* = 60*/,
    int nPicPeriod /*= 3*/)
{
	QScreenWnd *pWnd = GetInstance();
    if (pWnd->IsWindow())
    {
        return TRUE;
    }

    return pWnd->_Show(pszPicFolder,nAlpha,crBk,nSeconds, nPicPeriod);
}

BOOL QScreenWnd::_Show( LPCWSTR pszPicFolder, int nAlpha /*= 200*/, 
    COLORREF crBk /*= RGB(88,93,245)*/,int nSeconds /*= QSCREENWND_MIN_LIFESEC*/,
    int nPicPeriod/* = 3*/)
{
    BOOL bOK = Create(QUIGetMainWnd(),WS_POPUP|WS_VISIBLE,
        WS_EX_TOPMOST|WS_EX_TOOLWINDOW|WS_EX_LAYERED, 0);
    if (bOK)
    {
        quibase::MoveToFullScreen(GetSafeHwnd());

        m_nLifeSec = max(nSeconds,QSCREENWND_MIN_LIFESEC);
        // ����ɫ
        m_ctlPic = (HELEMENT)GetCtrl("#div_pic");
        m_ctlPic.SetBkgndColor(COLORREF2HtmlColor(crBk));
        GetBody().SetBkgndColor(COLORREF2HtmlColor(crBk));
        m_ctlPic.SetFrgndImage(_ImgPath());
        // ͸����
        SetLayeredWindowAttributes(m_hWnd,0,nAlpha,LWA_ALPHA);
        // ��ʱ�Զ��ر�
        SetTimer(QSCREENWND_LIFETIMER_ID,QSCREENWND_LIFETIMER_PERIOD,NULL);
        // �Զ�ѭ������ͼƬ
        m_nPeriodSec = max(2,nPicPeriod);   // Ĭ������С2��ѭ��
        int nPics = m_pic.LoadFolder(pszPicFolder);
        m_pic.HtmlayoutPlay(GetSafeHwnd(),L"file://" + _ImgPath(),
            m_nPeriodSec,TRUE,TRUE);
        if (nPics > 0)
        {
            // �������һ��
            OnPlayerTimerFired(nPics - 1);
        }
        SetForegroundWindow(GetSafeHwnd());
        SetFocus();
    }
    return bOK;
}

void QScreenWnd::OnPlayerTimerFired( LPARAM lp )
{
    m_pic.PlayPicture((int)lp);
    
    m_ctlPic.SetCheck(TRUE);
    // ����
    m_iAnimation = m_nPeriodSec * 1000 / QSCREENWND_ANIMATIONTIMER_PERIOD;
    SetTimer(QSCREENWND_ANIMATIONTIMER_ID,QSCREENWND_ANIMATIONTIMER_PERIOD,NULL);
}

void QScreenWnd::OnTimer( UINT_PTR nTimerID )
{
    if (QSCREENWND_LIFETIMER_ID == nTimerID)
    {
        if (m_nLifeSec-- <= 0)
        {
            PostMessage(WM_CLOSE);
        }
    }
    else if (QSCREENWND_ANIMATIONTIMER_ID == nTimerID)
    {
        //TRACE(L"timer:%d\n",m_iAnimation);
        if (2 == m_iAnimation--)
        {
            KillTimer(QSCREENWND_ANIMATIONTIMER_ID);
            m_ctlPic.SetCheck(FALSE);
        }
    }
}

void QScreenWnd::OnClose()
{
    if (m_nLifeSec <= 0)
    {
        // �ر�
        m_pic.StopPlay();
        SetMsgHandled(FALSE);
    }
    // ������û�н������ر�
}

void QScreenWnd::OnKeyDown( TCHAR ch, UINT ,UINT )
{
    if (VK_ESCAPE == ch)
    {
        m_nLifeSec = 0;
        PostMessage(WM_CLOSE);
    }
}
