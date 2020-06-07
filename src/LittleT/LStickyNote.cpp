﻿#include "stdafx.h"

#include "LStickyNote.h"
#include "common/QDBHelper.h"
#include "ui/QConfig.h"
#include "ui/WndHelper.h"
#include "ui/QUIDlgs.h"
#include "html/xh_scanner.h"


namespace littlet
{
    LStickyNoteWnd* NewStickyNote(__out TTodoTask& t)
    {
        t.sTask = L"Ctrl+Tab导航便签";
        t.nFlag = TODO_FLAG_STICKYNOTE;
        t.nID = QDBEvents::GetInstance()->TodoTask_Add(&t);
        t.nCateID = INVALID_ID;
        t.nPriority = 1;
        t.eStatus = TODO_STATUS_PROCESSING;

        // 创建便签
        return StickyNoteMan::GetInstance()->Create(t);
    }
}

//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(LStickyNoteWnd, _Base)
    BN_CLICKED_NAME(L"item_note", &LStickyNoteWnd::OnClkNoteItem)
    BN_CLICKED_NAME(L"btn_strikeline", &LStickyNoteWnd::OnStrikeText)
    BN_CLICKED_ID(L"btn_find", &LStickyNoteWnd::OnClkFind)
    BN_CLICKED_ID(L"btn_close", &LStickyNoteWnd::OnClkClose)
    BN_CLICKED_ID(L"btn_editor", &LStickyNoteWnd::OnClkFontEditor)
    BN_CLICKED_ID(L"btn_new", &LStickyNoteWnd::OnclkNewNote)
    BN_STATECHANGED_ID(L"chk_topmost", &LStickyNoteWnd::OnClkPinTop);
    BN_CLICKED_ID(L"id_bar_ok", &LStickyNoteWnd::OnClkSearchOK)
    BN_CLICKED_ID(L"id_bar_cancel", &LStickyNoteWnd::OnClkSearchCancel)
    UNKNOWN_SELECTION_CHANGED_ID(L"color_scheme", &LStickyNoteWnd::OnSelColorSchemeChanged)
QUI_END_EVENT_MAP()

LStickyNoteWnd::LStickyNoteWnd(const TTodoTask& task)
    :taskid_(task.nID)
{
    m_sHtmlPath = L"qabs:main/todo/note.htm";
}

void LStickyNoteWnd::OnclkNewNote(HELEMENT) 
{
    TTodoTask t;
    if (LStickyNoteWnd* w = littlet::NewStickyNote(t))
    {
        // 显示在自己旁边
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

void LStickyNoteWnd::OnStrikeText(HELEMENT he) 
{
//     _Text().xcall(,)
//     _Text().ReplaceSelection()
}

void LStickyNoteWnd::OnClkNoteItem(HELEMENT he) 
{
    // 编辑
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

    // 如果不是退出就不关闭
    SetMsgHandled(FALSE);
}

void LStickyNoteWnd::OnClkFontEditor(HELEMENT he) 
{
    ECtrl ctl = GetCtrl("#editor_msg");
    if (ctl.IsHasAttribute("xfocus"))
    {
        ctl.remove_attribute("xfocus");
    }
    else
    {
        // 必须加这句，否则toolbar上的按钮都不起作用了
        GetCtrl("#editor_msg>.toolbar").SetFocus();
        ctl.set_attribute("xfocus", L"1");
    }
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

    _Font().SelectItem_Text(L"宋体");

    TTodoTask task;
    if (QDBEvents::GetInstance()->TodoTask_Get(taskid_, TODOTASK_TYPE_STICKNOTE ,task))
    {
        CStdString real_txt;
        real_txt.Format(L"<html>"
            L"<head><style type='text/css'>u{color:gray; text-decoration:line-through;}</style></head>"
            L"<body>%s</body>"
            L"</html>", task.sTask);
        _Text().SetText(real_txt);
    }

    return 0;
}

void LStickyNoteWnd::OnClkClose(HELEMENT he) 
{
    // 关闭窗口认为删除任务
    QDBEvents::GetInstance()->TodoTask_Delete(Task().nID);

    // 关闭窗口
    SendMessage(WM_CLOSE);
}

void LStickyNoteWnd::SetTopMost(bool top)
{
    ModifyStyleEx(0, WS_EX_TOPMOST);

    GetCtrl("#chk_topmost").SetCheck(top);
    WTL::CRect rc(0, 0, 0, 0);
    SetWindowPos(top ? HWND_TOPMOST : HWND_NOTOPMOST, &rc, SWP_SHOWWINDOW | SWP_NOREDRAW | SWP_NOSIZE | SWP_NOMOVE);
    QUIGetConfig()->SetValue(L"StickyNoteTop", CStdString::number(taskid_), top ? L"1" : L"0");
}

bool LStickyNoteWnd::IsTopMost()const
{
    return GetExStyle() & WS_EX_TOPMOST;
}

void LStickyNoteWnd::OnClkPinTop(HELEMENT he) 
{
    SetTopMost(ECheck(he).IsChecked());
}

void LStickyNoteWnd::OnKillFocus(HWND)
{
    TTodoTask task = Task();
    task.sTask = _Text().GetText();
    QDBEvents::GetInstance()->TodoTask_Edit(&task);

    SetShadowSize(2);

    SetMsgHandled(FALSE);
}

void LStickyNoteWnd::OnSetFocus(HWND)
{
    // 输入光标移动到最后一个字符
    _Text().SelectText(0xFFFF);
    // 某些时候，光标不是移动到字符后面，而是移动到边缘，这个时候是无法
    // 输入数据的。不清楚是什么原因。
    _Text().SimulateKeybordEvent(VK_LEFT);
    _Text().SetFocus();

    // 阴影颜色
    // SetShadowColor(0xE80A4B);
    SetShadowSize(4);

    SetMsgHandled(FALSE);
}

void LStickyNoteWnd::OnSelColorSchemeChanged(HELEMENT he, HELEMENT)
{
    CStdString cr = EColorPicker(he).GetColor();
    GetBody().SetBkgndColor(cr);
    // 保存背景色
    QUIGetConfig()->SetValue(L"StickyNoteColor", CStdString::number(taskid_), cr);
}

TTodoTask LStickyNoteWnd::Task() const
{
    TTodoTask task;
    QDBEvents::GetInstance()->TodoTask_Get(TaskID(), TODOTASK_TYPE_STICKNOTE, task);
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

    // 背景色
    CStdString scolor = cfg->GetValue(L"StickyNoteColor", CStdString::number(taskid_));
    if (!scolor.IsEmpty())
    {
        GetBody().SetBkgndColor(scolor);
    }

    // topmost
    if (cfg->GetValue(L"StickyNoteTop", CStdString::number(taskid_)) == L"1")
    {
        SetTopMost(true);
    }

    // 位置
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

    // 最后给它一个wm_move 消息
    SendMessage(WM_EXITSIZEMOVE, 0, 0);

    return TRUE;
}

void LStickyNoteWnd::ShowSearchBar()
{
    HTMLayoutShowPopup(_SearchBar(), _Text(), 2);

    EEdit ctlInput = _SearchBar().find_first("#id_search");
    ctlInput.SetFocus();
}

void LStickyNoteWnd::OnKeyDown(TCHAR ch, UINT n, UINT r)
{
    if (GetKeyState(VK_CONTROL) & 0x8000) // CTRL
    {
        switch (ch)
        {
        case 0x46:  // F
            ShowSearchBar();
            return;

        case 0x4E:  // N
            OnclkNewNote(NULL);
            return;
        }
    }

    if (GetKeyState(VK_F3) & 0x8000)
    {
        // 搜索
        StickyNoteMan::GetInstance()->SearchNext(this);
        return;
    }

    if (GetKeyState(VK_TAB) & 0x8000)
    {
        if (GetKeyState(VK_CONTROL) & 0x8000)
        {
            LStickyNoteWnd* p = nullptr;
            if (GetKeyState(VK_SHIFT) & 0x8000)
            {
                // 向前
                p = StickyNoteMan::GetInstance()->PrevSibling(this);
            }
            else
            {
                // 向后
                p = StickyNoteMan::GetInstance()->NextSibling(this);
            }

            if ((nullptr != p) && (p != this))
            {
                quibase::SetForegroundWindowInternal(p->GetSafeHwnd());
                p->SetFocus();
            }
            return;
        }
        EEdit(_Text()).ReplaceSelection(L"    ", 4);
        return;
    }

    SetMsgHandled(FALSE);
}

bool LStickyNoteWnd::SearchText(const CStdString& txt)const
{
    if (txt.IsEmpty())
    {
        ASSERT(FALSE);
        return false;
    }

    auto Html2Txt = [](CStdString html)->CStdString
    {
        struct str_istream : public markup::instream
        {
            const wchar_t* p;
            const wchar_t* end;
            str_istream(const wchar_t* src) : p(src), end(src + wcslen(src)) {}
            virtual wchar_t get_char() { return p < end ? *p++ : 0; }
        };

        html.Replace(L"\r\n", L"");
        str_istream si(html.c_str());
        markup::scanner sc(si);
        
        CStdString ret;
        while (true)
        {
            bool finished = false; 
            switch (sc.get_token())
            {
            case markup::scanner::TT_ERROR:
            case markup::scanner::TT_EOF: finished = true; break;

//             case markup::scanner::TT_TAG_END:   // 
//             case markup::scanner::TT_WORD:
            case markup::scanner::TT_WORD:
            case markup::scanner::TT_SPACE: ret += sc.get_value(); break;
            }

            if (finished)
                break;
        }
        ret.Replace(L"\r\n ", L"");
        ret.Replace(L"\n", L"");
        return ret;
    };

    int i_start = -1, i_end = -1;
    _Text().GetSelection(i_start, i_end);
    
    CStdString s_txt = Html2Txt(_Text().GetText());
    i_start = s_txt.find(txt, i_end);
    if (-1 != i_start)
    {
        // 先取消选中
        _Text().SelectText(0xffff);
        _Text().SelectText(i_start, i_start + txt.length());
        // _Text().SelectText(8,9);
        return true;
    }
    return false;
}

void LStickyNoteWnd::OnClkSearchCancel(HELEMENT he)
{
    ::HTMLayoutHidePopup(_SearchBar());
}

void LStickyNoteWnd::OnClkSearchOK(HELEMENT he)
{
    EEdit ctlInput = _SearchBar().find_first("#id_search");
    ctlInput.SetFocus();

    auto* man = StickyNoteMan::GetInstance();
    man->s_search_ = ctlInput.GetText();
    man->SearchNext(this);

    ::HTMLayoutHidePopup(_SearchBar());
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
        // 显示桌面便签
        wnd = new LStickyNoteWnd(t);
        wnd->Create(GetDesktopWindow(), WS_POPUP | WS_VISIBLE, WS_EX_TOOLWINDOW,
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

LStickyNoteWnd* StickyNoteMan::PrevSibling(LStickyNoteWnd* p)
{
    auto i = std::find(lst_.begin(), lst_.end(), p);
    if (i == lst_.end())
    {
        return nullptr;
    }

    if (i == lst_.begin())
    {
        return lst_.back();
    }
    return *--i;
}

LStickyNoteWnd* StickyNoteMan::NextSibling(LStickyNoteWnd* p)
{
    auto i = std::find(lst_.begin(), lst_.end(), p);
    if (i == lst_.end())
    {
        return nullptr;
    }

    if (p == lst_.back())
    {
        return lst_.front();
    }
    return *++i;
}

void StickyNoteMan::ShowAll()
{
    for (LStickyNoteWnd* p : lst_)
    {
        if (p->IsWindow() /*&& !p->IsWindowVisible()*/)
        {
            bool is_top = p->IsTopMost();
            // 如果不将其设置为topmost的风格，那么LittleT处于后台运行（非激活状态）
            // 是不能将这些窗口带到最上层的
            p->SetTopMost(true);
            p->ShowWindow(SW_SHOW);
            p->SetTopMost(is_top);  
        }
    }
}

LStickyNoteWnd* StickyNoteMan::SearchNext(LStickyNoteWnd* cur_win)
{
    if (s_search_.IsEmpty())
    {
        return nullptr;
    }

    auto FuncSearch = [=](LstStickyWnd::iterator ib, LstStickyWnd::iterator ie)->LStickyNoteWnd*
    {
        for (ib; ib != ie; ++ib)
        {
            LStickyNoteWnd* p = *ib;
            if (p->SearchText(s_search_))
            {
                p->ShowWindow(true);
                p->SetFocus();

                // 找到了
                return p;
            }
        }
        // 失败了！
        return nullptr;
    };

    auto i = std::find(lst_.begin(), lst_.end(), cur_win);
    // 后半段找
    if (LStickyNoteWnd* p = FuncSearch(i, lst_.end()))
        return p;
    
    // 前半段找
    return FuncSearch(lst_.begin(), i);
}
