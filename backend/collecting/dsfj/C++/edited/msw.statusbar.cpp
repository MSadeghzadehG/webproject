


#include "wx/wxprec.h"

#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#if wxUSE_STATUSBAR && wxUSE_NATIVE_STATUSBAR

#include "wx/statusbr.h"

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcctl.h"     #include "wx/frame.h"
    #include "wx/settings.h"
    #include "wx/dcclient.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/control.h"
#endif

#include "wx/msw/private.h"
#include "wx/tooltip.h"
#include <windowsx.h>

#if wxUSE_UXTHEME
    #include "wx/msw/uxtheme.h"
#endif


namespace
{

static const int DEFAULT_FIELD_WIDTH = 25;

} 

#define StatusBar_SetParts(h, n, w) SendMessage(h, SB_SETPARTS, (WPARAM)n, (LPARAM)w)
#define StatusBar_SetText(h, n, t)  SendMessage(h, SB_SETTEXT, (WPARAM)n, (LPARAM)(LPCTSTR)t)
#define StatusBar_GetTextLen(h, n)  LOWORD(SendMessage(h, SB_GETTEXTLENGTH, (WPARAM)n, 0))
#define StatusBar_GetText(h, n, s)  LOWORD(SendMessage(h, SB_GETTEXT, (WPARAM)n, (LPARAM)(LPTSTR)s))



wxStatusBar::wxStatusBar()
{
    SetParent(NULL);
    m_hWnd = 0;
    m_windowId = 0;
    m_pDC = NULL;
}

WXDWORD wxStatusBar::MSWGetStyle(long style, WXDWORD *exstyle) const
{
    WXDWORD msStyle = wxStatusBarBase::MSWGetStyle(style, exstyle);

                wxWindow * const parent = GetParent();
    wxCHECK_MSG( parent, msStyle, wxS("Status bar must have a parent") );
    if ( parent->IsTopLevel() && !parent->HasFlag(wxRESIZE_BORDER) )
        style &= ~wxSTB_SIZEGRIP;

                        if ( !(style & wxSTB_SIZEGRIP) )
    {
        msStyle |= CCS_TOP;
    }
    else
    {
                       msStyle |= SBARS_SIZEGRIP;
    }

    return msStyle;
}

bool wxStatusBar::Create(wxWindow *parent,
                         wxWindowID id,
                         long style,
                         const wxString& name)
{
    if ( !CreateControl(parent, id, wxDefaultPosition, wxDefaultSize,
                        style, wxDefaultValidator, name) )
        return false;

    if ( !MSWCreateControl(STATUSCLASSNAME, wxString(),
                           wxDefaultPosition, wxDefaultSize) )
        return false;

    SetFieldsCount(1);

        m_pDC = new wxClientDC(this);

                            PostSizeEventToParent();

    return true;
}

wxStatusBar::~wxStatusBar()
{
                PostSizeEventToParent();

#if wxUSE_TOOLTIPS
        for (size_t i=0; i<m_tooltips.size(); i++)
    {
        wxDELETE(m_tooltips[i]);
    }
#endif 
    wxDELETE(m_pDC);
}

bool wxStatusBar::SetFont(const wxFont& font)
{
    if (!wxWindow::SetFont(font))
        return false;

    if (m_pDC) m_pDC->SetFont(font);
    return true;
}

void wxStatusBar::SetFieldsCount(int nFields, const int *widths)
{
        wxASSERT_MSG( (nFields > 0) && (nFields < 255), "too many fields" );

    
#if wxUSE_TOOLTIPS
        for (size_t i=0; i<m_tooltips.size(); i++)
    {
        wxDELETE(m_tooltips[i]);
    }

        m_tooltips.resize(nFields, NULL);
#endif 
    wxStatusBarBase::SetFieldsCount(nFields, widths);

    MSWUpdateFieldsWidths();
}

void wxStatusBar::SetStatusWidths(int n, const int widths[])
{
    wxStatusBarBase::SetStatusWidths(n, widths);

    MSWUpdateFieldsWidths();
}

void wxStatusBar::MSWUpdateFieldsWidths()
{
    if ( m_panes.IsEmpty() )
        return;

    const int count = m_panes.GetCount();

    const int extraWidth = MSWGetBorderWidth() + MSWGetMetrics().textMargin;

        int widthAvailable = GetClientSize().x;         widthAvailable -= extraWidth*(count - 1);       widthAvailable -= MSWGetMetrics().textMargin;   
            const int gripWidth = HasFlag(wxSTB_SIZEGRIP) ? MSWGetMetrics().gripWidth
                                                  : 0;
    widthAvailable -= gripWidth;

    
    wxArrayInt widthsAbs = CalculateAbsWidths(widthAvailable);


    
    int *pWidths = new int[count];

    int nCurPos = 0;
    int i;
    for ( i = 0; i < count; i++ )
    {
        nCurPos += widthsAbs[i] + extraWidth;
        pWidths[i] = nCurPos;
    }

                pWidths[count - 1] += gripWidth;

    if ( !StatusBar_SetParts(GetHwnd(), count, pWidths) )
    {
        wxLogLastError("StatusBar_SetParts");
    }

        for ( i = 0; i < count; i++ )
    {
        DoUpdateStatusText(i);
    }

    delete [] pWidths;
}

void wxStatusBar::DoUpdateStatusText(int nField)
{
    if (!m_pDC)
        return;

        int style;
    switch(m_panes[nField].GetStyle())
    {
    case wxSB_RAISED:
        style = SBT_POPOUT;
        break;
    case wxSB_FLAT:
        style = SBT_NOBORDERS;
        break;

    case wxSB_SUNKEN:
    case wxSB_NORMAL:
    default:
        style = 0;
        break;
    }

    wxRect rc;
    GetFieldRect(nField, rc);

    const int maxWidth = rc.GetWidth() - MSWGetMetrics().textMargin;

    wxString text = GetStatusText(nField);

        wxEllipsizeMode ellmode = (wxEllipsizeMode)-1;
    if (HasFlag(wxSTB_ELLIPSIZE_START)) ellmode = wxELLIPSIZE_START;
    else if (HasFlag(wxSTB_ELLIPSIZE_MIDDLE)) ellmode = wxELLIPSIZE_MIDDLE;
    else if (HasFlag(wxSTB_ELLIPSIZE_END)) ellmode = wxELLIPSIZE_END;

    if (ellmode == (wxEllipsizeMode)-1)
    {
                        if (HasFlag(wxSTB_SHOW_TIPS))
            SetEllipsizedFlag(nField, m_pDC->GetTextExtent(text).GetWidth() > maxWidth);
    }
    else
    {
        text = wxControl::Ellipsize(text,
                                     *m_pDC,
                                     ellmode,
                                     maxWidth,
                                     wxELLIPSIZE_FLAGS_EXPAND_TABS);

                                SetEllipsizedFlag(nField, text != GetStatusText(nField));
    }

            if ( !StatusBar_SetText(GetHwnd(), nField | style, text.t_str()) )
    {
        wxLogLastError("StatusBar_SetText");
    }

#if wxUSE_TOOLTIPS
    if (HasFlag(wxSTB_SHOW_TIPS))
    {
        wxASSERT(m_tooltips.size() == m_panes.GetCount());

        if (m_tooltips[nField])
        {
            if (GetField(nField).IsEllipsized())
            {
                                m_tooltips[nField]->SetRect(rc);

                                m_tooltips[nField]->SetTip(GetStatusText(nField));
            }
            else
            {
                                wxDELETE(m_tooltips[nField]);
            }
        }
        else
        {
                        if (GetField(nField).IsEllipsized())
                m_tooltips[nField] = new wxToolTip(this, nField, GetStatusText(nField), rc);
                    }
    }
#endif }

wxStatusBar::MSWBorders wxStatusBar::MSWGetBorders() const
{
    int aBorders[3];
    SendMessage(GetHwnd(), SB_GETBORDERS, 0, (LPARAM)aBorders);

    MSWBorders borders;
    borders.horz = aBorders[0];
    borders.vert = aBorders[1];
    borders.between = aBorders[2];
    return borders;
}

int wxStatusBar::GetBorderX() const
{
    return MSWGetBorders().horz;
}

int wxStatusBar::GetBorderY() const
{
    return MSWGetBorders().vert;
}

int wxStatusBar::MSWGetBorderWidth() const
{
    return MSWGetBorders().between;
}


const wxStatusBar::MSWMetrics& wxStatusBar::MSWGetMetrics()
{
    static MSWMetrics s_metrics = { 0, 0 };
    if ( !s_metrics.textMargin )
    {
                                                        #if wxUSE_UXTHEME
        if ( wxUxThemeEngine::GetIfActive() )
        {
            s_metrics.gripWidth = 20;
            s_metrics.textMargin = 8;
        }
        else #endif         {
            s_metrics.gripWidth = 18;
            s_metrics.textMargin = 4;
        }
    }

    return s_metrics;
}

void wxStatusBar::SetMinHeight(int height)
{
                    height += 4*GetBorderY();

                SetSize(-1, height);

    SendMessage(GetHwnd(), SB_SETMINHEIGHT, height, 0);

        SendMessage(GetHwnd(), WM_SIZE, 0, 0);
}

bool wxStatusBar::GetFieldRect(int i, wxRect& rect) const
{
    wxCHECK_MSG( (i >= 0) && ((size_t)i < m_panes.GetCount()), false,
                 "invalid statusbar field index" );

    RECT r;
    if ( !::SendMessage(GetHwnd(), SB_GETRECT, i, (LPARAM)&r) )
    {
        wxLogLastError("SendMessage(SB_GETRECT)");
    }

#if wxUSE_UXTHEME
    wxUxThemeHandle theme(const_cast<wxStatusBar*>(this), L"Status");
    if ( theme )
    {
                        if ( i != 0 )
        {
            r.left -= 2;
        }

        wxUxThemeEngine::Get()->GetThemeBackgroundContentRect(theme, NULL,
                                                              1 , 0,
                                                              &r, &r);
    }
#endif

    wxCopyRECTToRect(r, rect);

    return true;
}

wxSize wxStatusBar::DoGetBestSize() const
{
    const MSWBorders borders = MSWGetBorders();

        int width = 0;
    for ( size_t i = 0; i < m_panes.GetCount(); ++i )
    {
        int widthField =
            m_bSameWidthForAllPanes ? DEFAULT_FIELD_WIDTH : m_panes[i].GetWidth();
        if ( widthField >= 0 )
        {
            width += widthField;
        }
        else
        {
                                    width += -widthField*DEFAULT_FIELD_WIDTH;
        }

                width += borders.between;
    }

    if ( !width )
    {
                width = 2*DEFAULT_FIELD_WIDTH;
    }

            int height = GetCharHeight();
    height += 4*borders.vert;

    wxSize best(width, height);
    CacheBestSize(best);
    return best;
}

void wxStatusBar::DoMoveWindow(int x, int y, int width, int height)
{
    if ( GetParent()->IsSizeDeferred() )
    {
        wxWindowMSW::DoMoveWindow(x, y, width, height);
    }
    else
    {
                                        ::SetWindowPos(GetHwnd(), NULL, x, y, width, height,
                       SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE
                       | SWP_NOCOPYBITS | SWP_NOSENDCHANGING
                       );
    }

                if ( m_children.GetCount() > 0 )
    {
        wxSizeEvent event(GetClientSize(), m_windowId);
        event.SetEventObject(this);
        HandleWindowEvent(event);
    }
}

void wxStatusBar::SetStatusStyles(int n, const int styles[])
{
    wxStatusBarBase::SetStatusStyles(n, styles);

    if (n != (int)m_panes.GetCount())
        return;

    for (int i = 0; i < n; i++)
    {
        int style;
        switch(styles[i])
        {
        case wxSB_RAISED:
            style = SBT_POPOUT;
            break;
        case wxSB_FLAT:
            style = SBT_NOBORDERS;
            break;
        case wxSB_SUNKEN:
        case wxSB_NORMAL:
        default:
            style = 0;
            break;
        }

                                wxString text = GetStatusText(i);
        if (!StatusBar_SetText(GetHwnd(), style | i, text.t_str()))
        {
            wxLogLastError("StatusBar_SetText");
        }
    }
}

WXLRESULT
wxStatusBar::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
    if ( nMsg == WM_WINDOWPOSCHANGING )
    {
        WINDOWPOS *lpPos = (WINDOWPOS *)lParam;
        int x, y, w, h;
        GetPosition(&x, &y);
        GetSize(&w, &h);

                AdjustForParentClientOrigin(x, y);

        lpPos->x  = x;
        lpPos->y  = y;
        lpPos->cx = w;
        lpPos->cy = h;

        return 0;
    }

    if ( nMsg == WM_NCLBUTTONDOWN )
    {
                        if ( wParam == HTBOTTOMRIGHT )
        {
            wxWindow *win;

            for ( win = GetParent(); win; win = win->GetParent() )
            {
                if ( win->IsTopLevel() )
                {
                    SendMessage(GetHwndOf(win), WM_NCLBUTTONDOWN,
                                wParam, lParam);

                    return 0;
                }
            }
        }
    }

    if ( nMsg == WM_SIZE )
    {
        MSWUpdateFieldsWidths();

        if ( HasFlag(wxSTB_ELLIPSIZE_START) ||
                HasFlag(wxSTB_ELLIPSIZE_MIDDLE) ||
                    HasFlag(wxSTB_ELLIPSIZE_END) )
        {
            for (int i=0; i<GetFieldsCount(); i++)
            {
                                                DoUpdateStatusText(i);
            }
        }
    }

    return wxStatusBarBase::MSWWindowProc(nMsg, wParam, lParam);
}

#if wxUSE_TOOLTIPS
bool wxStatusBar::MSWProcessMessage(WXMSG* pMsg)
{
    if ( HasFlag(wxSTB_SHOW_TIPS) )
    {
                                                MSG *msg = (MSG *)pMsg;
        if ( msg->message == WM_MOUSEMOVE )
            wxToolTip::RelayEvent(pMsg);
    }

    return wxWindow::MSWProcessMessage(pMsg);
}

bool wxStatusBar::MSWOnNotify(int WXUNUSED(idCtrl), WXLPARAM lParam, WXLPARAM* WXUNUSED(result))
{
    if ( HasFlag(wxSTB_SHOW_TIPS) )
    {
                        
        NMHDR* hdr = (NMHDR *)lParam;

        wxString str;
        if (hdr->idFrom < m_tooltips.size() && m_tooltips[hdr->idFrom])
            str = m_tooltips[hdr->idFrom]->GetTip();

        if ( HandleTooltipNotify(hdr->code, lParam, str))
        {
                        return true;
        }
    }

    return false;
}
#endif 
#endif 