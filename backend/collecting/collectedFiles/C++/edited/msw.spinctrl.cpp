


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_SPINCTRL

#include "wx/spinctrl.h"

#ifndef WX_PRECOMP
    #include "wx/hashmap.h"
    #include "wx/msw/wrapcctl.h"     #include "wx/event.h"
    #include "wx/textctrl.h"
    #include "wx/wxcrtvararg.h"
#endif

#include "wx/msw/private.h"

#if wxUSE_TOOLTIPS
    #include "wx/tooltip.h"
#endif 
#include <limits.h>         

wxBEGIN_EVENT_TABLE(wxSpinCtrl, wxSpinButton)
    EVT_CHAR(wxSpinCtrl::OnChar)
    EVT_SET_FOCUS(wxSpinCtrl::OnSetFocus)
    EVT_KILL_FOCUS(wxSpinCtrl::OnKillFocus)
wxEND_EVENT_TABLE()

#define GetBuddyHwnd()      (HWND)(m_hwndBuddy)


static const int MARGIN_BETWEEN = 1;



namespace
{

WX_DECLARE_HASH_MAP(HWND, wxSpinCtrl *,
                    wxPointerHash, wxPointerEqual,
                    SpinForTextCtrl);

SpinForTextCtrl gs_spinForTextCtrl;

} 


LRESULT APIENTRY
wxBuddyTextWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    wxSpinCtrl * const spin = wxSpinCtrl::GetSpinForTextCtrl(hwnd);

        switch ( message )
    {
        case WM_SETFOCUS:
                                    if ( (WXHWND)wParam == spin->GetHWND() )
                break;
            
        case WM_KILLFOCUS:
        case WM_CHAR:
        case WM_DEADCHAR:
        case WM_KEYUP:
        case WM_KEYDOWN:
#ifdef WM_HELP
                                case WM_HELP:
#endif
            {
                WXLRESULT result;
                if ( spin->MSWHandleMessage(&result, message, wParam, lParam) )
                {
                                                                                                    return 0;
                }

                                if ( !::IsWindow(hwnd) )
                    return 0;
            }
            break;

        case WM_GETDLGCODE:
            if ( spin->HasFlag(wxTE_PROCESS_ENTER) )
            {
                long dlgCode = ::CallWindowProc
                                 (
                                    CASTWNDPROC spin->GetBuddyWndProc(),
                                    hwnd,
                                    message,
                                    wParam,
                                    lParam
                                 );
                dlgCode |= DLGC_WANTMESSAGE;
                return dlgCode;
            }
            break;
    }

    return ::CallWindowProc(CASTWNDPROC spin->GetBuddyWndProc(),
                            hwnd, message, wParam, lParam);
}


wxSpinCtrl *wxSpinCtrl::GetSpinForTextCtrl(WXHWND hwndBuddy)
{
    const SpinForTextCtrl::const_iterator
        it = gs_spinForTextCtrl.find(hwndBuddy);
    if ( it == gs_spinForTextCtrl.end() )
        return NULL;

    wxSpinCtrl * const spin = it->second;

        wxASSERT_MSG( spin->m_hwndBuddy == hwndBuddy,
                  wxT("wxSpinCtrl has incorrect buddy HWND!") );

    return spin;
}

bool wxSpinCtrl::ProcessTextCommand(WXWORD cmd, WXWORD WXUNUSED(id))
{
    if ( (cmd == EN_CHANGE) && (!m_blockEvent ))
    {
        wxCommandEvent event(wxEVT_TEXT, GetId());
        event.SetEventObject(this);
        wxString val = wxGetWindowText(m_hwndBuddy);
        event.SetString(val);
        event.SetInt(GetValue());
        return HandleWindowEvent(event);
    }

        return false;
}

void wxSpinCtrl::OnChar(wxKeyEvent& event)
{
    switch ( event.GetKeyCode() )
    {
        case WXK_RETURN:
            {
                wxCommandEvent evt(wxEVT_TEXT_ENTER, m_windowId);
                InitCommandEvent(evt);
                wxString val = wxGetWindowText(m_hwndBuddy);
                evt.SetString(val);
                evt.SetInt(GetValue());
                if ( HandleWindowEvent(evt) )
                    return;
                break;
            }

        case WXK_TAB:
                                                            {
                wxNavigationKeyEvent eventNav;
                eventNav.SetDirection(!event.ShiftDown());
                eventNav.SetWindowChange(event.ControlDown());
                eventNav.SetEventObject(this);

                if ( GetParent()->HandleWindowEvent(eventNav) )
                    return;
            }
            break;
    }

        event.Skip();
}

void wxSpinCtrl::OnKillFocus(wxFocusEvent& event)
{
        NormalizeValue();
    event.Skip();
}

void wxSpinCtrl::OnSetFocus(wxFocusEvent& event)
{
            ::SetFocus((HWND)m_hwndBuddy);

    event.Skip();
}

void wxSpinCtrl::NormalizeValue()
{
    const int value = GetValue();
    const bool changed = value != m_oldValue;

                SetValue(value);

    if ( changed )
    {
        SendSpinUpdate(value);
    }
}


void wxSpinCtrl::Init()
{
    m_blockEvent = false;
    m_hwndBuddy = NULL;
    m_wndProcBuddy = NULL;
    m_oldValue = INT_MIN;
}

bool wxSpinCtrl::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxString& value,
                        const wxPoint& pos,
                        const wxSize& size,
                        long style,
                        int min, int max, int initial,
                        const wxString& name)
{
                style |= wxSP_VERTICAL;

    if ( (style & wxBORDER_MASK) == wxBORDER_DEFAULT )
        style |= wxBORDER_SUNKEN;

    SetWindowStyle(style);

    WXDWORD exStyle = 0;
    WXDWORD msStyle = MSWGetStyle(GetWindowStyle(), & exStyle) ;

            msStyle |= ES_AUTOHSCROLL;

        if ( style & wxALIGN_RIGHT )
        msStyle |= ES_RIGHT;
    else if ( style & wxALIGN_CENTER )
        msStyle |= ES_CENTER;

            wxSize sizeText(size), sizeBtn(size);
    sizeBtn.x = wxSpinButton::DoGetBestSize().x;
    if ( sizeText.x <= 0 )
    {
                sizeText.x = FromDIP(DEFAULT_ITEM_WIDTH) + MARGIN_BETWEEN + sizeBtn.x;
    }

    sizeText.x -= sizeBtn.x + MARGIN_BETWEEN;
    if ( sizeText.x <= 0 )
    {
        wxLogDebug(wxS("wxSpinCtrl \"%s\": initial width %d is too small, ")
                   wxS("at least %d pixels needed."),
                   name, size.x, sizeBtn.x + MARGIN_BETWEEN + 1);
    }

    wxPoint posBtn(pos);
    posBtn.x += sizeText.x + MARGIN_BETWEEN;

                    
    
    m_hwndBuddy = (WXHWND)::CreateWindowEx
                    (
                     exStyle,                                     wxT("EDIT"),                                  NULL,                                        msStyle,                                     pos.x, pos.y,                                0, 0,                                        GetHwndOf(parent),                           (HMENU)-1,                                   wxGetInstance(),                             NULL                                        );

    if ( !m_hwndBuddy )
    {
        wxLogLastError(wxT("CreateWindow(buddy text window)"));

        return false;
    }


        if ( !wxSpinButton::Create(parent, id, posBtn, sizeBtn, style, name) )
    {
        return false;
    }

    wxSpinButtonBase::SetRange(min, max);

        gs_spinForTextCtrl[GetBuddyHwnd()] = this;

    m_wndProcBuddy = (WXFARPROC)wxSetWindowProc(GetBuddyHwnd(),
                                                wxBuddyTextWndProc);

        InheritAttributes();
    if (!m_hasFont)
        SetFont(GetDefaultAttributes().font);

            if ( sizeText.y <= 0 )
    {
        int cx, cy;
        wxGetCharSize(GetHWND(), &cx, &cy, GetFont());

        sizeText.y = EDIT_HEIGHT_FROM_CHAR_HEIGHT(cy);
    }

    SetInitialSize(size);

    (void)::ShowWindow(GetBuddyHwnd(), SW_SHOW);

        (void)::SendMessage(GetHwnd(), UDM_SETBUDDY, (WPARAM)m_hwndBuddy, 0);

            long initialFromText;
    if ( value.ToLong(&initialFromText) )
        initial = initialFromText;

                SetRange(min, max);
    SetValue(initial);

            m_blockEvent = true;
    if ( !value.empty() )
        SetValue(value);
    m_blockEvent = false;

    return true;
}

wxSpinCtrl::~wxSpinCtrl()
{
            ::DestroyWindow( GetBuddyHwnd() );

    gs_spinForTextCtrl.erase(GetBuddyHwnd());
}


int wxSpinCtrl::GetBase() const
{
    return ::SendMessage(GetHwnd(), UDM_GETBASE, 0, 0);
}

bool wxSpinCtrl::SetBase(int base)
{
    if ( !::SendMessage(GetHwnd(), UDM_SETBASE, base, 0) )
        return false;

            UpdateBuddyStyle();

    return true;
}


void wxSpinCtrl::SetValue(const wxString& text)
{
    if ( !::SetWindowText(GetBuddyHwnd(), text.c_str()) )
    {
        wxLogLastError(wxT("SetWindowText(buddy)"));
    }
}

void  wxSpinCtrl::SetValue(int val)
{
    m_blockEvent = true;

    wxSpinButton::SetValue(val);

                const wxString text = wxGetWindowText(m_hwndBuddy);

                if ( text.empty() && !val )
    {
        ::SetWindowText(GetBuddyHwnd(), wxT("0"));
    }

                if ( GetBase() == 16 &&
            (text.length() < 3 || text[0] != '0' ||
                (text[1] != 'x' && text[1] != 'X')) )
    {
        ::SetWindowText(GetBuddyHwnd(),
                        wxPrivate::wxSpinCtrlFormatAsHex(val, m_max).t_str());
    }

    m_oldValue = GetValue();

    m_blockEvent = false;
}

int wxSpinCtrl::GetValue() const
{
    const wxString val = wxGetWindowText(m_hwndBuddy);

    long n;
    if ( !val.ToLong(&n, GetBase()) )
        n = INT_MIN;

    if ( n < m_min )
        n = m_min;
    if ( n > m_max )
        n = m_max;

    return n;
}

void wxSpinCtrl::SetSelection(long from, long to)
{
            if ( (from == -1) && (to == -1) )
    {
        from = 0;
    }

    ::SendMessage(GetBuddyHwnd(), EM_SETSEL, (WPARAM)from, (LPARAM)to);
}

void wxSpinCtrl::SetLayoutDirection(wxLayoutDirection dir)
{
            wxUpdateEditLayoutDirection(GetBuddyHwnd(), dir);

    wxSpinButton::SetLayoutDirection(dir);

        SetSize(-1, -1, -1, -1, wxSIZE_AUTO | wxSIZE_FORCE);
}


void wxSpinCtrl::SetRange(int minVal, int maxVal)
{
                if ( minVal <= maxVal )
    {
        if ( m_oldValue < minVal )
            m_oldValue = minVal;
        else if ( m_oldValue > maxVal )
            m_oldValue = maxVal;
    }
    else     {
        if ( m_oldValue > minVal )
            m_oldValue = minVal;
        else if ( m_oldValue < maxVal )
            m_oldValue = maxVal;
    }

    wxSpinButton::SetRange(minVal, maxVal);

    UpdateBuddyStyle();
}

void wxSpinCtrl::UpdateBuddyStyle()
{
                        const DWORD styleOld = ::GetWindowLong(GetBuddyHwnd(), GWL_STYLE);
    DWORD styleNew;
    if ( m_min < 0 || GetBase() != 10 )
        styleNew = styleOld & ~ES_NUMBER;
    else
        styleNew = styleOld | ES_NUMBER;

    if ( styleNew != styleOld )
        ::SetWindowLong(GetBuddyHwnd(), GWL_STYLE, styleNew);
}


bool wxSpinCtrl::SetFont(const wxFont& font)
{
    if ( !wxWindowBase::SetFont(font) )
    {
                return false;
    }

    WXHANDLE hFont = GetFont().GetResourceHandle();
    (void)::SendMessage(GetBuddyHwnd(), WM_SETFONT, (WPARAM)hFont, TRUE);

    return true;
}

bool wxSpinCtrl::Show(bool show)
{
    if ( !wxControl::Show(show) )
    {
        return false;
    }

    ::ShowWindow(GetBuddyHwnd(), show ? SW_SHOW : SW_HIDE);

    return true;
}

bool wxSpinCtrl::Reparent(wxWindowBase *newParent)
{
                    
            const wxRect rect = GetRect();

    if ( !wxWindowBase::Reparent(newParent) )
        return false;

    newParent->GetChildren().DeleteObject(this);

            const HWND hwndOld = GetHwnd();
    UnsubclassWin();
    if ( !::DestroyWindow(hwndOld) )
    {
        wxLogLastError(wxT("DestroyWindow"));
    }

        if ( !wxSpinButton::Create(GetParent(), GetId(),
                               rect.GetPosition(), rect.GetSize(),
                               GetWindowStyle(), GetName()) )
        return false;

        wxSpinButton::SetValue(GetValue());
    SetRange(m_min, m_max);

            SetSize(rect, wxSIZE_ALLOW_MINUS_ONE);

        ::SetParent(GetBuddyHwnd(), GetHwndOf(GetParent()));
    (void)::SendMessage(GetHwnd(), UDM_SETBUDDY, (WPARAM)GetBuddyHwnd(), 0);

    return true;
}

bool wxSpinCtrl::Enable(bool enable)
{
    if ( !wxControl::Enable(enable) )
    {
        return false;
    }

    MSWEnableHWND(GetBuddyHwnd(), enable);

    return true;
}

void wxSpinCtrl::SetFocus()
{
    ::SetFocus(GetBuddyHwnd());
}

#if wxUSE_TOOLTIPS

void wxSpinCtrl::DoSetToolTip(wxToolTip *tip)
{
    wxSpinButton::DoSetToolTip(tip);

    if ( tip )
        tip->AddOtherWindow(m_hwndBuddy);
}

#endif 

void wxSpinCtrl::SendSpinUpdate(int value)
{
    wxSpinEvent event(wxEVT_SPINCTRL, GetId());
    event.SetEventObject(this);
    event.SetInt(value);

    (void)HandleWindowEvent(event);

    m_oldValue = value;
}

bool wxSpinCtrl::MSWOnScroll(int WXUNUSED(orientation), WXWORD wParam,
                             WXWORD WXUNUSED(pos), WXHWND control)
{
    wxCHECK_MSG( control, false, wxT("scrolling what?") );

    if ( wParam != SB_THUMBPOSITION )
    {
                return false;
    }

            int new_value = GetValue();
    if (m_oldValue != new_value)
       SendSpinUpdate( new_value );

    return true;
}

bool wxSpinCtrl::MSWOnNotify(int WXUNUSED(idCtrl), WXLPARAM lParam, WXLPARAM *result)
{
    NM_UPDOWN *lpnmud = (NM_UPDOWN *)lParam;

    if (lpnmud->hdr.hwndFrom != GetHwnd())         return false;

    *result = 0;  
    return TRUE;
}



wxSize wxSpinCtrl::DoGetBestSize() const
{
    return DoGetSizeFromTextSize(DEFAULT_ITEM_WIDTH);
}

wxSize wxSpinCtrl::DoGetSizeFromTextSize(int xlen, int ylen) const
{
    wxSize sizeBtn = wxSpinButton::DoGetBestSize();

    int y;
    wxGetCharSize(GetHWND(), NULL, &y, GetFont());
                
    wxSize tsize(xlen + sizeBtn.x + MARGIN_BETWEEN + 3*y/10 + 10,
                 EDIT_HEIGHT_FROM_CHAR_HEIGHT(y));

        if ( ylen > 0 )
        tsize.IncBy(0, ylen - y);

    return tsize;
}

void wxSpinCtrl::DoMoveWindow(int x, int y, int width, int height)
{
    int widthBtn = wxSpinButton::DoGetBestSize().x;
    int widthText = width - widthBtn - MARGIN_BETWEEN;
    if ( widthText < 0 )
    {
                                                                        widthText = 0;
    }

                if ( GetParent()->GetLayoutDirection() == GetLayoutDirection() )
    {
                        DoMoveSibling(m_hwndBuddy, x, y, widthText, height);

                if ( widthText > 0 )
            x += widthText + MARGIN_BETWEEN;
        wxSpinButton::DoMoveWindow(x, y, widthBtn, height);
    }
    else
    {
                        wxSpinButton::DoMoveWindow(x, y, widthBtn, height);

                x += widthBtn + MARGIN_BETWEEN;
        DoMoveSibling(m_hwndBuddy, x, y, widthText, height);
    }
}

void wxSpinCtrl::DoGetSize(int *x, int *y) const
{
    RECT spinrect, textrect, ctrlrect;
    GetWindowRect(GetHwnd(), &spinrect);
    GetWindowRect(GetBuddyHwnd(), &textrect);
    UnionRect(&ctrlrect,&textrect, &spinrect);

    if ( x )
        *x = ctrlrect.right - ctrlrect.left;
    if ( y )
        *y = ctrlrect.bottom - ctrlrect.top;
}

void wxSpinCtrl::DoGetClientSize(int *x, int *y) const
{
    RECT spinrect = wxGetClientRect(GetHwnd());
    RECT textrect = wxGetClientRect(GetBuddyHwnd());
    RECT ctrlrect;
    UnionRect(&ctrlrect,&textrect, &spinrect);

    if ( x )
        *x = ctrlrect.right - ctrlrect.left;
    if ( y )
        *y = ctrlrect.bottom - ctrlrect.top;
}

void wxSpinCtrl::DoGetPosition(int *x, int *y) const
{
                                    
        int xBuddy;
    WXHWND hWnd = GetHWND();
    wxConstCast(this, wxSpinCtrl)->m_hWnd = m_hwndBuddy;
    wxSpinButton::DoGetPosition(&xBuddy, y);

    int xText;
    wxConstCast(this, wxSpinCtrl)->m_hWnd = hWnd;
    wxSpinButton::DoGetPosition(&xText, y);

    *x = wxMin(xBuddy, xText);
}

#endif 