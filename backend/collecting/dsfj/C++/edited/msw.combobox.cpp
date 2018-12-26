


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_COMBOBOX

#include "wx/combobox.h"

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcctl.h"     #include "wx/settings.h"
    #include "wx/log.h"
        #include "wx/textctrl.h"
    #include "wx/app.h"
    #include "wx/brush.h"
#endif

#include "wx/clipbrd.h"
#include "wx/wupdlock.h"
#include "wx/msw/private.h"

#if wxUSE_UXTHEME
    #include "wx/msw/uxtheme.h"
#endif

#if wxUSE_TOOLTIPS
    #include "wx/tooltip.h"
#endif 

wxBEGIN_EVENT_TABLE(wxComboBox, wxControl)
    EVT_MENU(wxID_CUT, wxComboBox::OnCut)
    EVT_MENU(wxID_COPY, wxComboBox::OnCopy)
    EVT_MENU(wxID_PASTE, wxComboBox::OnPaste)
    EVT_MENU(wxID_UNDO, wxComboBox::OnUndo)
    EVT_MENU(wxID_REDO, wxComboBox::OnRedo)
    EVT_MENU(wxID_CLEAR, wxComboBox::OnDelete)
    EVT_MENU(wxID_SELECTALL, wxComboBox::OnSelectAll)

    EVT_UPDATE_UI(wxID_CUT, wxComboBox::OnUpdateCut)
    EVT_UPDATE_UI(wxID_COPY, wxComboBox::OnUpdateCopy)
    EVT_UPDATE_UI(wxID_PASTE, wxComboBox::OnUpdatePaste)
    EVT_UPDATE_UI(wxID_UNDO, wxComboBox::OnUpdateUndo)
    EVT_UPDATE_UI(wxID_REDO, wxComboBox::OnUpdateRedo)
    EVT_UPDATE_UI(wxID_CLEAR, wxComboBox::OnUpdateDelete)
    EVT_UPDATE_UI(wxID_SELECTALL, wxComboBox::OnUpdateSelectAll)
wxEND_EVENT_TABLE()


LRESULT APIENTRY
wxComboEditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


static WNDPROC gs_wndprocEdit = (WNDPROC)NULL;


namespace
{

bool ShouldForwardFromEditToCombo(UINT message)
{
    switch ( message )
    {
        case WM_KEYUP:
        case WM_KEYDOWN:
        case WM_CHAR:
        case WM_SYSCHAR:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
        case WM_CUT:
        case WM_COPY:
        case WM_PASTE:
            return true;
    }

    return false;
}

} 

LRESULT APIENTRY
wxComboEditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND hwndCombo = ::GetParent(hWnd);
    wxWindow *win = wxFindWinFromHandle((WXHWND)hwndCombo);

    if ( ShouldForwardFromEditToCombo(message) )
    {
        wxComboBox *combo = wxDynamicCast(win, wxComboBox);
        if ( !combo )
        {
                                                if ( !win->IsBeingDeleted() )
            {
                wxFAIL_MSG( wxT("should have combo as parent") );
            }
        }
        else if ( combo->MSWProcessEditMsg(message, wParam, lParam) )
        {
                        return 0;
        }
    }
    else if ( message == WM_GETDLGCODE )
    {
        wxCHECK_MSG( win, 0, wxT("should have a parent") );

        if ( win->GetWindowStyle() & wxTE_PROCESS_ENTER )
        {
                        return DLGC_WANTMESSAGE;
        }
    }

    return ::CallWindowProc(CASTWNDPROC gs_wndprocEdit, hWnd, message, wParam, lParam);
}


WXLRESULT wxComboBox::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
        
    switch ( nMsg )
    {
        case WM_SIZE:
                        case WM_STYLECHANGED:
            {
                                                if ( !GetEditHWNDIfAvailable() )
                    break;

                long fromOld, toOld;
                GetSelection(&fromOld, &toOld);

                                                                                wxString textOld;
                if ( !HasFlag(wxCB_READONLY) && GetCurrentSelection() == -1 )
                    textOld = GetValue();

                                wxWindowUpdateLocker lock(this);

                WXLRESULT result = wxChoice::MSWWindowProc(nMsg, wParam, lParam);

                if ( !textOld.empty() && GetValue() != textOld )
                    SetLabel(textOld);

                long fromNew, toNew;
                GetSelection(&fromNew, &toNew);

                if ( fromOld != fromNew || toOld != toNew )
                {
                    SetSelection(fromOld, toOld);
                }

                return result;
            }
    }

    return wxChoice::MSWWindowProc(nMsg, wParam, lParam);
}

bool wxComboBox::MSWProcessEditMsg(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam)
{
    switch ( msg )
    {
        case WM_CHAR:
                                    switch ( wParam )
            {
                case VK_RETURN:
                    {
                        if (SendMessage(GetHwnd(), CB_GETDROPPEDSTATE, 0, 0))
                            return false;

                        wxCommandEvent event(wxEVT_TEXT_ENTER, m_windowId);

                        const int sel = GetSelection();
                        event.SetInt(sel);
                        event.SetString(GetValue());
                        InitCommandEventWithItems(event, sel);

                        if ( ProcessCommand(event) )
                        {
                                                                                                                return true;
                        }
                    }
                    break;

                case VK_TAB:
                                                                                                    if ( !HasFlag(wxTE_PROCESS_TAB) )
                    {
                        int flags = 0;
                        if ( !wxIsShiftDown() )
                            flags |= wxNavigationKeyEvent::IsForward;
                        if ( wxIsCtrlDown() )
                            flags |= wxNavigationKeyEvent::WinChange;
                        if ( Navigate(flags) )
                            return true;
                    }
                    break;
            }
    }

    if ( ShouldForwardFromEditToCombo(msg) )
    {
                        WXLRESULT result;
        return MSWHandleMessage(&result, msg, wParam, lParam);
    }

    return false;
}

bool wxComboBox::MSWCommand(WXUINT param, WXWORD id)
{
    int sel = -1;
    wxString value;

    switch ( param )
    {
        case CBN_DROPDOWN:
                        m_lastAcceptedSelection = GetCurrentSelection();
            {
                wxCommandEvent event(wxEVT_COMBOBOX_DROPDOWN, GetId());
                event.SetEventObject(this);
                ProcessCommand(event);
            }
            break;

        case CBN_CLOSEUP:
                        if ( m_pendingSelection != wxID_NONE )
            {
                SendSelectionChangedEvent(wxEVT_COMBOBOX);
                m_pendingSelection = wxID_NONE;
            }
            {
                wxCommandEvent event(wxEVT_COMBOBOX_CLOSEUP, GetId());
                event.SetEventObject(this);
                ProcessCommand(event);
            }
            break;

        case CBN_SELENDOK:
                                    m_lastAcceptedSelection = wxID_NONE;

                                    sel = GetSelection();
            value = GetStringSelection();

                                                ::SetWindowText(GetHwnd(), value.t_str());

            SendSelectionChangedEvent(wxEVT_COMBOBOX);

                                    
        case CBN_EDITCHANGE:
            if ( m_allowTextEvents )
            {
                wxCommandEvent event(wxEVT_TEXT, GetId());

                                if ( sel == -1 )
                {
                    value = wxGetWindowText(GetHwnd());
                }

                event.SetString(value);
                InitCommandEventWithItems(event, sel);

                ProcessCommand(event);
            }
            break;

        default:
            return wxChoice::MSWCommand(param, id);
    }

            return true;
}

bool wxComboBox::MSWShouldPreProcessMessage(WXMSG *pMsg)
{
            if (wxIsCtrlDown())
    {
        WPARAM vkey = pMsg->wParam;

        switch (vkey)
        {
            case 'C':
            case 'V':
            case 'X':
            case VK_INSERT:
            case VK_DELETE:
            case VK_HOME:
            case VK_END:
                return false;
        }
    }

    return wxChoice::MSWShouldPreProcessMessage(pMsg);
}

WXHWND wxComboBox::GetEditHWNDIfAvailable() const
{
    WinStruct<COMBOBOXINFO> info;
    if ( ::GetComboBoxInfo(GetHwnd(), &info) )
        return info.hwndItem;

    if (HasFlag(wxCB_SIMPLE))
    {
        POINT pt;
        pt.x = pt.y = 4;
        return (WXHWND) ::ChildWindowFromPoint(GetHwnd(), pt);
    }

                
        return (WXHWND)::GetWindow(GetHwnd(), GW_CHILD);
}

WXHWND wxComboBox::GetEditHWND() const
{
            wxASSERT_MSG( !HasFlag(wxCB_READONLY),
                  wxT("read-only combobox doesn't have any edit control") );

    WXHWND hWndEdit = GetEditHWNDIfAvailable();
    wxASSERT_MSG( hWndEdit, wxT("combobox without edit control?") );

    return hWndEdit;
}

wxWindow *wxComboBox::GetEditableWindow()
{
    wxASSERT_MSG( !HasFlag(wxCB_READONLY),
                  wxT("read-only combobox doesn't have any edit control") );

    return this;
}


bool wxComboBox::Create(wxWindow *parent, wxWindowID id,
                        const wxString& value,
                        const wxPoint& pos,
                        const wxSize& size,
                        int n, const wxString choices[],
                        long style,
                        const wxValidator& validator,
                        const wxString& name)
{
                m_isShown = false;

    if ( !CreateAndInit(parent, id, pos, size, n, choices, style,
                        validator, name) )
        return false;

                if ( !value.empty() )
        SetValue(value);

                if ( !(style & wxCB_READONLY) )
    {
        gs_wndprocEdit = wxSetWindowProc((HWND)GetEditHWND(), wxComboEditWndProc);
    }

        Show(true);

    return true;
}

bool wxComboBox::Create(wxWindow *parent, wxWindowID id,
                        const wxString& value,
                        const wxPoint& pos,
                        const wxSize& size,
                        const wxArrayString& choices,
                        long style,
                        const wxValidator& validator,
                        const wxString& name)
{
    wxCArrayString chs(choices);
    return Create(parent, id, value, pos, size, chs.GetCount(),
                  chs.GetStrings(), style, validator, name);
}

WXDWORD wxComboBox::MSWGetStyle(long style, WXDWORD *exstyle) const
{
        WXDWORD msStyle = wxChoice::MSWGetStyle
                      (
                        (style & ~wxBORDER_MASK) | wxBORDER_NONE, exstyle
                      );

                msStyle |= WS_TABSTOP;

        msStyle &= ~CBS_DROPDOWNLIST;

    if ( style & wxCB_READONLY )
        msStyle |= CBS_DROPDOWNLIST;
    else if ( style & wxCB_SIMPLE )
        msStyle |= CBS_SIMPLE;     else
        msStyle |= CBS_DROPDOWN;

        msStyle |= CBS_AUTOHSCROLL;

    
    return msStyle;
}


wxString wxComboBox::GetValue() const
{
    return HasFlag(wxCB_READONLY) ? GetStringSelection()
                                  : wxTextEntry::GetValue();
}

void wxComboBox::SetValue(const wxString& value)
{
    if ( HasFlag(wxCB_READONLY) )
        SetStringSelection(value);
    else
        wxTextEntry::SetValue(value);
}

void wxComboBox::Clear()
{
    wxChoice::Clear();
    if ( !HasFlag(wxCB_READONLY) )
        wxTextEntry::Clear();
}

bool wxComboBox::ContainsHWND(WXHWND hWnd) const
{
    return hWnd == GetEditHWNDIfAvailable();
}

void wxComboBox::GetSelection(long *from, long *to) const
{
    if ( !HasFlag(wxCB_READONLY) )
    {
        wxTextEntry::GetSelection(from, to);
    }
    else     {
        if ( from )
            *from = -1;
        if ( to )
            *to = -1;
    }
}

bool wxComboBox::IsEditable() const
{
    return !HasFlag(wxCB_READONLY) && wxTextEntry::IsEditable();
}


void wxComboBox::OnCut(wxCommandEvent& WXUNUSED(event))
{
    Cut();
}

void wxComboBox::OnCopy(wxCommandEvent& WXUNUSED(event))
{
    Copy();
}

void wxComboBox::OnPaste(wxCommandEvent& WXUNUSED(event))
{
    Paste();
}

void wxComboBox::OnUndo(wxCommandEvent& WXUNUSED(event))
{
    Undo();
}

void wxComboBox::OnRedo(wxCommandEvent& WXUNUSED(event))
{
    Redo();
}

void wxComboBox::OnDelete(wxCommandEvent& WXUNUSED(event))
{
    RemoveSelection();
}

void wxComboBox::OnSelectAll(wxCommandEvent& WXUNUSED(event))
{
    SelectAll();
}

void wxComboBox::OnUpdateCut(wxUpdateUIEvent& event)
{
    event.Enable( CanCut() );
}

void wxComboBox::OnUpdateCopy(wxUpdateUIEvent& event)
{
    event.Enable( CanCopy() );
}

void wxComboBox::OnUpdatePaste(wxUpdateUIEvent& event)
{
    event.Enable( CanPaste() );
}

void wxComboBox::OnUpdateUndo(wxUpdateUIEvent& event)
{
    event.Enable( IsEditable() && CanUndo() );
}

void wxComboBox::OnUpdateRedo(wxUpdateUIEvent& event)
{
    event.Enable( IsEditable() && CanRedo() );
}

void wxComboBox::OnUpdateDelete(wxUpdateUIEvent& event)
{
    event.Enable(IsEditable() && HasSelection());
}

void wxComboBox::OnUpdateSelectAll(wxUpdateUIEvent& event)
{
    event.Enable(IsEditable() && !wxTextEntry::IsEmpty());
}

#if wxUSE_TOOLTIPS

void wxComboBox::DoSetToolTip(wxToolTip *tip)
{
    wxChoice::DoSetToolTip(tip);

    if ( tip && !HasFlag(wxCB_READONLY) )
        tip->AddOtherWindow(GetEditHWND());
}

#endif 
#if wxUSE_UXTHEME

bool wxComboBox::SetHint(const wxString& hintOrig)
{
    wxString hint(hintOrig);
    if ( wxUxThemeEngine::GetIfActive() )
    {
                                hint.insert(0, " ");
    }

    return wxTextEntry::SetHint(hint);
}

#endif 
wxSize wxComboBox::DoGetSizeFromTextSize(int xlen, int ylen) const
{
    wxSize tsize( wxChoice::DoGetSizeFromTextSize(xlen, ylen) );

    if ( !HasFlag(wxCB_READONLY) )
    {
                wxPoint marg( GetMargins() );
        marg.x = wxMax(0, marg.x);
        marg.y = wxMax(0, marg.y);
        tsize.IncBy( marg );
    }

    return tsize;
}

wxWindow *wxComboBox::MSWFindItem(long id, WXHWND hWnd) const
{
                                if ( id == GetId() && hWnd && hWnd != GetHWND() )
    {
                return NULL;
    }

    return wxChoice::MSWFindItem(id, hWnd);
}

void wxComboBox::SetLayoutDirection(wxLayoutDirection dir)
{
    
                    if ( !HasFlag(wxCB_READONLY) )
    {
        if ( dir == wxLayout_RightToLeft )
        {
            wxUpdateLayoutDirection(GetEditHWND(), dir);
        }
        else
        {
            LONG_PTR style = ::GetWindowLongPtr(GetEditHWND(), GWL_STYLE);
            if ( !(style & ES_CENTER) )
            {
                style &= ~ES_RIGHT;
                ::SetWindowLongPtr(GetEditHWND(), GWL_STYLE, style);
            }
        }
    }

        WinStruct<COMBOBOXINFO> info;
    if ( ::GetComboBoxInfo(GetHwnd(), &info) )
    {
        wxUpdateLayoutDirection(info.hwndList, dir);
    }

    wxChoice::SetLayoutDirection(dir);
}

#endif 