#include "stdafx.h"

#include "LStickyNote.h"
#include "common/QDBHelper.h"
#include "ui/QConfig.h"


namespace littlet
{
    LStickyNoteWnd* NewStickyNote(__out TTodoTask& t)
    {
        t.sTask = L"�±�ǩ";
        t.nFlag = TODO_FLAG_STICKYNOTE;
        t.nID = QDBEvents::GetInstance()->TodoTask_Add(&t);
        t.nCateID = INVALID_ID;
        t.nPriority = 1;
        t.eStatus = TODO_STATUS_PROCESSING;

        // ������ǩ
        return StickyNoteMan::GetInstance()->Create(t);
    }
}

//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(LStickyNoteWnd, _Base)
    BN_CLICKED_NAME(L"item_note", &LStickyNoteWnd::OnClkNoteItem)
    BN_CLICKED_NAME(L"del_item", &LStickyNoteWnd::OnClkDelItem)
    BN_CLICKED_ID(L"btn_find", &LStickyNoteWnd::OnClkFind)
    BN_CLICKED_ID(L"btn_showall", &LStickyNoteWnd::OnClkShowAll)
    BN_CLICKED_ID(L"btn_new", &LStickyNoteWnd::OnclkNewNote)
    BN_STATECHANGED_ID(L"chk_topmost", &LStickyNoteWnd::OnClkPinTop);
    BN_CLICKED_ID(L"btn_close", &LStickyNoteWnd::OnClkClose)
    UNKNOWN_SELECTION_CHANGED_ID(L"color_scheme", &LStickyNoteWnd::OnSelColorSchemeChanged)
QUI_END_EVENT_MAP()

LStickyNoteWnd::LStickyNoteWnd(const TTodoTask& task)
    :taskid_(task.nID)
{
    m_sHtmlPath = L"qabs:main/todo/note.htm";
}

void LStickyNoteWnd::OnclkNewNote(HELEMENT he) 
{
    TTodoTask t;
    if (LStickyNoteWnd* w = littlet::NewStickyNote(t))
    {
        // ��ʾ���Լ��Ա�
        CRect rc;
        GetWindowRect(&rc);

        CRect rc_dest;
        w->GetWindowRect(&rc_dest);
        CPoint pt_dest = rc.TopLeft();
        pt_dest.Offset(40, 40);
        rc_dest.MoveToXY(pt_dest);

        w->MoveWindow(&rc_dest, FALSE);
    }
}

void LStickyNoteWnd::OnClkDelItem(HELEMENT he) 
{
}

void LStickyNoteWnd::OnClkNoteItem(HELEMENT he) 
{
    // �༭
}

void LStickyNoteWnd::ShowEditPane(BOOL bShow, TTodoTask* p) 
{
}

ETable LStickyNoteWnd::AddItem(__in TTodoTask* p, __in BOOL bAtFirst) 
{
    return ETable();
}

ETable LStickyNoteWnd::NewItem(__in TTodoTask* p) 
{
    return ETable();
}

BOOL LStickyNoteWnd::UpdateItem(__in ETable& tbl) 
{
    return FALSE;
}

void LStickyNoteWnd::OnClose() 
{
    SaveWindowPos();

    // ��������˳��Ͳ��ر�
    SetMsgHandled(FALSE);
}

void LStickyNoteWnd::OnClkShowAll(HELEMENT he) 
{
}

void LStickyNoteWnd::OnClkFind(HELEMENT he) 
{
    
}

LRESULT LStickyNoteWnd::OnDocumentComplete() 
{
    if (taskid_ == INVALID_ID)
    {
        ASSERT(FALSE);
        return 0;
    }

    RestoreSetting();

    TTodoTask task;
    if (QDBEvents::GetInstance()->TodoTask_Get(taskid_, task))
    {
        _Text().SetValue(task.sTask);
    }

    return 0;
}

void LStickyNoteWnd::NewIt() 
{
    OnclkNewNote(NULL);
}

void LStickyNoteWnd::EditIt(__in TTodoTask* p) 
{

}

void LStickyNoteWnd::OnClkClose(HELEMENT he) 
{
    TTodoTask task = Task();
    // ����Ϊ�����
    task.eStatus = TODO_STATUS_FINISH;
    // �رմ�����Ϊȥ��stick��־
    _RemoveFlag(task.nFlag, TODO_FLAG_STICKYNOTE);

    // ����
    QDBEvents::GetInstance()->TodoTask_Edit(&task);

    // �رմ���
    SendMessage(WM_CLOSE);
}

void LStickyNoteWnd::OnClkPinTop(HELEMENT he) 
{
    if (ECheck(he).IsChecked())
    {
        SetWindowPos(HWND_TOPMOST, NULL, SWP_NOREDRAW | SWP_NOSIZE | SWP_NOMOVE);
        QUIGetConfig()->SetValue(L"StickyNoteTop", CStdString::number(taskid_), L"1");
    }
    else
    {
        SetWindowPos(HWND_TOP, NULL, SWP_NOREDRAW | SWP_NOSIZE | SWP_NOMOVE);
        QUIGetConfig()->SetValue(L"StickyNoteTop", CStdString::number(taskid_), L"0");
    }
}

void LStickyNoteWnd::Show(BOOL bNew, BOOL bUseDate, QTime tmDate /*= QTime::GetCurrentTime() */) 
{
}

void LStickyNoteWnd::ShowAndEdit(TTodoTask* p) 
{
}

void LStickyNoteWnd::OnKillFocus(HWND)
{
    GetCtrl("#editor_msg").remove_attribute("focus");

    TTodoTask task = Task();
    task.sTask = _Text().get_value().to_string();
    QDBEvents::GetInstance()->TodoTask_Edit(&task);
}

void LStickyNoteWnd::OnSetFocus(HWND)
{
    GetCtrl("#editor_msg").set_attribute("focus", L"1");
}

void LStickyNoteWnd::OnSelColorSchemeChanged(HELEMENT he, HELEMENT)
{
    CStdString cr = EColorPicker(he).GetColor();
    GetBody().SetBkgndColor(cr);
    // ���汳��ɫ
    QUIGetConfig()->SetValue(L"StickyNoteColor", CStdString::number(taskid_), cr);
}

TTodoTask LStickyNoteWnd::Task() const
{
    TTodoTask task;
    QDBEvents::GetInstance()->TodoTask_Get(TaskID(), task);
    return task;
}

void LStickyNoteWnd::SaveWindowPos()
{
    WINDOWPLACEMENT wp = { 0 };
    wp.length = sizeof(WINDOWPLACEMENT);

    // get window position and iconized/maximized status
    GetWindowPlacement(&wp);

    wp.flags = 0;
    wp.showCmd = SW_SHOWNORMAL;

    TCHAR tmp[200];
    WTL::SecureHelper::sprintf_x(tmp, 200, _T("%u,%u,%d,%d,%d,%d"),
        wp.flags,
        wp.showCmd,
        wp.rcNormalPosition.left,
        wp.rcNormalPosition.top,
        wp.rcNormalPosition.right,
        wp.rcNormalPosition.bottom);

    // write position to INI file
    QUIGetConfig()->SetValue(L"StickyNote", CStdString::number(taskid_), tmp);
}

BOOL LStickyNoteWnd::RestoreSetting()
{
    // read window position from INI file
    // MainWindow format =0,1,395,198,1012,642
    auto* cfg = QUIGetConfig();

    // ����ɫ
    CStdString scolor = cfg->GetValue(L"StickyNoteColor", CStdString::number(taskid_));
    if (!scolor.IsEmpty())
    {
        GetBody().SetBkgndColor(scolor);
    }

    // topmost
    if (cfg->GetValue(L"StickyNoteTop", CStdString::number(taskid_)) == L"1")
    {
        GetCtrl("#chk_topmost").SetCheck(TRUE);
        SetWindowPos(HWND_TOPMOST, NULL, SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW);
    }

    // λ��
    CStdString sPos = cfg->GetValue(L"StickyNote", CStdString::number(taskid_));
    if (sPos.IsEmpty())
    {
        CenterWindow(::GetDesktopWindow());
        return FALSE;
    }

    TCHAR tmp[256];
    _tcsncpy_s(tmp, sPos, _countof(tmp) - 2);

    // get WINDOWPLACEMENT info
    WINDOWPLACEMENT wp = { 0 };
    wp.length = sizeof(WINDOWPLACEMENT);
    wp.ptMaxPosition = CPoint(-::GetSystemMetrics(SM_CXBORDER), -::GetSystemMetrics(SM_CYBORDER));
    TCHAR *cp = 0;
    wp.rcNormalPosition = CRect(200, 100, 850, 550);
    if ((cp = _tcstok(tmp, _T(",\r\n"))) != NULL)
        wp.flags = _ttoi(cp);
    if ((cp = _tcstok(NULL, _T(",\r\n"))) != NULL)
        wp.showCmd = _ttoi(cp);
    if ((cp = _tcstok(NULL, _T(",\r\n"))) != NULL)
        wp.rcNormalPosition.left = _ttoi(cp);
    if ((cp = _tcstok(NULL, _T(",\r\n"))) != NULL)
        wp.rcNormalPosition.top = _ttoi(cp);
    if ((cp = _tcstok(NULL, _T(",\r\n"))) != NULL)
        wp.rcNormalPosition.right = _ttoi(cp);
    if ((cp = _tcstok(NULL, _T(",\r\n"))) != NULL)
        wp.rcNormalPosition.bottom = _ttoi(cp);

    // sets window's position and iconized/maximized status
    SetWindowPlacement(&wp);

    // ������һ��wm_move ��Ϣ
    SendMessage(WM_EXITSIZEMOVE, 0, 0);

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
LStickyNoteWnd* StickyNoteMan::Create(const TTodoTask& t)
{
    auto*db = QDBEvents::GetInstance();
    db->TodoTask_SetFlag(t.nID, t.nFlag | TODO_FLAG_STICKYNOTE);
    db->TodoTask_SetStatus(t.nID, TODO_STATUS_PROCESSING);
    
    LStickyNoteWnd* wnd = Find(t.nID);
    if (nullptr == wnd)
    {
        // ��ʾ�����ǩ
        wnd = new LStickyNoteWnd(t);
        wnd->Create(NULL, WS_POPUP | WS_VISIBLE, WS_EX_TOOLWINDOW,
            WS_QEX_WNDSHADOW | WS_QEX_THICKFRAME);
        
        lst_.push_back(wnd);
    }
    return wnd;
}

void StickyNoteMan::Shutdown()
{
    for (LStickyNoteWnd* p : lst_)
    {
        if (p->IsWindow())
        {
            p->SendMessage(WM_CLOSE);
        }
        delete p;
    }
    lst_.clear();
}

void StickyNoteMan::Startup()
{
    if (!lst_.empty())
    {
        ASSERT(FALSE);
        return;
    }

    TodoTaskList lst;
    if (QDBEvents::GetInstance()->TodoTask_GetAllStickyNote(lst))
    {
        for (auto itm : lst )
        {
            Create(itm);
        }
    }
}

LStickyNoteWnd* StickyNoteMan::Find(int taskid)
{
    for (auto i = lst_.begin(); i != lst_.end();)
    {
        if (!(*i)->IsWindow())
        {
            delete *i;
            i = lst_.erase(i);
        }
        else
        {
            if ((*i)->TaskID() == taskid)
            {
                return *i;
            }
            ++i;
        }
    }
    return nullptr;
}

void StickyNoteMan::Remove(int taskid)
{
    if (auto* p = Find(taskid))
    {
        p->SendMessage(WM_CLOSE);
    }
}
