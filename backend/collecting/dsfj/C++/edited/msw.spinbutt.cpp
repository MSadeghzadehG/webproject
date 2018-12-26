


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcctl.h"     #include "wx/app.h"
#endif

#if wxUSE_SPINBTN

#include "wx/spinbutt.h"

#include "wx/msw/private.h"

#ifndef UDM_SETRANGE32
    #define UDM_SETRANGE32 (WM_USER+111)
#endif

#ifndef UDM_SETPOS32
    #define UDM_SETPOS32 (WM_USER+113)
    #define UDM_GETPOS32 (WM_USER+114)
#endif




bool wxSpinButton::Create(wxWindow *parent,
                          wxWindowID id,
                          const wxPoint& pos,
                          const wxSize& size,
                          long style,
                          const wxString& name)
{
        m_windowId = (id == wxID_ANY) ? NewControlId() : id;

    SetName(name);

    int x = pos.x;
    int y = pos.y;
    int width = size.x;
    int height = size.y;

    m_windowStyle = style;

    SetParent(parent);

        if ( width <= 0 || height <= 0 )
    {
        wxSize bestSize = DoGetBestSize();
        if ( width <= 0 )
            width = bestSize.x;
        if ( height <= 0 )
            height = bestSize.y;
    }

    if ( x < 0 )
        x = 0;
    if ( y < 0 )
        y = 0;

        DWORD wstyle = WS_VISIBLE | WS_CHILD | WS_TABSTOP | 
                   UDS_NOTHOUSANDS |                    UDS_SETBUDDYINT;  
    if ( m_windowStyle & wxCLIP_SIBLINGS )
        wstyle |= WS_CLIPSIBLINGS;
    if ( m_windowStyle & wxSP_HORIZONTAL )
        wstyle |= UDS_HORZ;
    if ( m_windowStyle & wxSP_ARROW_KEYS )
        wstyle |= UDS_ARROWKEYS;
    if ( m_windowStyle & wxSP_WRAP )
        wstyle |= UDS_WRAP;

        m_hWnd = (WXHWND)CreateUpDownControl
                     (
                       wstyle,
                       x, y, width, height,
                       GetHwndOf(parent),
                       m_windowId,
                       wxGetInstance(),
                       NULL,                        m_max, m_min,
                       m_min                      );

    if ( !m_hWnd )
    {
        wxLogLastError(wxT("CreateUpDownControl"));

        return false;
    }

    if ( parent )
    {
        parent->AddChild(this);
    }

    SubclassWin(m_hWnd);

    SetInitialSize(size);

    return true;
}

wxSpinButton::~wxSpinButton()
{
}


wxSize wxSpinButton::DoGetBestSize() const
{
    const bool vert = HasFlag(wxSP_VERTICAL);

    wxSize bestSize(::GetSystemMetrics(vert ? SM_CXVSCROLL : SM_CXHSCROLL),
                    ::GetSystemMetrics(vert ? SM_CYVSCROLL : SM_CYHSCROLL));

    if ( vert )
        bestSize.y *= 2;
    else
        bestSize.x *= 2;

    return bestSize;
}


int wxSpinButton::GetValue() const
{
    int n;
#ifdef UDM_GETPOS32
        n = ::SendMessage(GetHwnd(), UDM_GETPOS32, 0, 0);
#else
        n = (short)LOWORD(::SendMessage(GetHwnd(), UDM_GETPOS, 0, 0));
#endif 
    if (n < m_min) n = m_min;
    if (n > m_max) n = m_max;

    return n;
}

void wxSpinButton::SetValue(int val)
{
    
#ifdef UDM_SETPOS32
        ::SendMessage(GetHwnd(), UDM_SETPOS32, 0, val);
#else
    ::SendMessage(GetHwnd(), UDM_SETPOS, 0, MAKELONG((short) val, 0));
#endif }

void wxSpinButton::NormalizeValue()
{
    SetValue( GetValue() );
}

void wxSpinButton::SetRange(int minVal, int maxVal)
{
    const bool hadRange = m_min < m_max;

    wxSpinButtonBase::SetRange(minVal, maxVal);

#ifdef UDM_SETRANGE32
        ::SendMessage(GetHwnd(), UDM_SETRANGE32, minVal, maxVal);
#else
        ::SendMessage(GetHwnd(), UDM_SETRANGE, 0,
                  (LPARAM) MAKELONG((short)maxVal, (short)minVal));
#endif 
        NormalizeValue();

                if ( hadRange != (m_min < m_max) )
    {
                Refresh();
    }
}

bool wxSpinButton::MSWOnScroll(int WXUNUSED(orientation), WXWORD wParam,
                               WXWORD WXUNUSED(pos), WXHWND control)
{
    wxCHECK_MSG( control, false, wxT("scrolling what?") );

    if ( wParam != SB_THUMBPOSITION )
    {
                return false;
    }

    wxSpinEvent event(wxEVT_SCROLL_THUMBTRACK, m_windowId);
            event.SetPosition(GetValue());
    event.SetEventObject(this);

    return HandleWindowEvent(event);
}

bool wxSpinButton::MSWOnNotify(int WXUNUSED(idCtrl), WXLPARAM lParam, WXLPARAM *result)
{
    NM_UPDOWN *lpnmud = (NM_UPDOWN *)lParam;

    if (lpnmud->hdr.hwndFrom != GetHwnd())         return false;

    wxSpinEvent event(lpnmud->iDelta > 0 ? wxEVT_SCROLL_LINEUP
                                         : wxEVT_SCROLL_LINEDOWN,
                      m_windowId);
    event.SetPosition(lpnmud->iPos + lpnmud->iDelta);
    event.SetEventObject(this);

    bool processed = HandleWindowEvent(event);

    *result = event.IsAllowed() ? 0 : 1;

    return processed;
}

bool wxSpinButton::MSWCommand(WXUINT WXUNUSED(cmd), WXWORD WXUNUSED(id))
{
        return false;
}

#endif 