


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_TIPWINDOW

#include "wx/tipwin.h"

#ifndef WX_PRECOMP
    #include "wx/dcclient.h"
    #include "wx/timer.h"
    #include "wx/settings.h"
#endif 

static const wxCoord TEXT_MARGIN_X = 3;
static const wxCoord TEXT_MARGIN_Y = 3;


class WXDLLEXPORT wxTipWindowView : public wxWindow
{
public:
    wxTipWindowView(wxWindow *parent);

        void OnPaint(wxPaintEvent& event);
    void OnMouseClick(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);

#if !wxUSE_POPUPWIN
    void OnKillFocus(wxFocusEvent& event);
#endif 
        void Adjust(const wxString& text, wxCoord maxLength);

private:
    wxTipWindow* m_parent;

#if !wxUSE_POPUPWIN
    long m_creationTime;
#endif 
    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(wxTipWindowView);
};



wxBEGIN_EVENT_TABLE(wxTipWindow, wxTipWindowBase)
    EVT_LEFT_DOWN(wxTipWindow::OnMouseClick)
    EVT_RIGHT_DOWN(wxTipWindow::OnMouseClick)
    EVT_MIDDLE_DOWN(wxTipWindow::OnMouseClick)

#if !wxUSE_POPUPWIN
    EVT_KILL_FOCUS(wxTipWindow::OnKillFocus)
    EVT_ACTIVATE(wxTipWindow::OnActivate)
#endif wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(wxTipWindowView, wxWindow)
    EVT_PAINT(wxTipWindowView::OnPaint)

    EVT_LEFT_DOWN(wxTipWindowView::OnMouseClick)
    EVT_RIGHT_DOWN(wxTipWindowView::OnMouseClick)
    EVT_MIDDLE_DOWN(wxTipWindowView::OnMouseClick)

    EVT_MOTION(wxTipWindowView::OnMouseMove)

#if !wxUSE_POPUPWIN
    EVT_KILL_FOCUS(wxTipWindowView::OnKillFocus)
#endif wxEND_EVENT_TABLE()


wxTipWindow::wxTipWindow(wxWindow *parent,
                         const wxString& text,
                         wxCoord maxLength,
                         wxTipWindow** windowPtr,
                         wxRect *rectBounds)
#if wxUSE_POPUPWIN
           : wxPopupTransientWindow(parent)
#else
           : wxFrame(parent, wxID_ANY, wxEmptyString,
                     wxDefaultPosition, wxDefaultSize,
                     wxNO_BORDER | wxFRAME_NO_TASKBAR )
#endif
{
    SetTipWindowPtr(windowPtr);
    if ( rectBounds )
    {
        SetBoundingRect(*rectBounds);
    }

        SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));

        m_view = new wxTipWindowView(this);
    m_view->Adjust(text, maxLength);
    m_view->SetFocus();

    int x, y;
    wxGetMousePosition(&x, &y);

                        y += wxSystemSettings::GetMetric(wxSYS_CURSOR_Y) / 2;

#if wxUSE_POPUPWIN
    Position(wxPoint(x, y), wxSize(0,0));
    Popup(m_view);
    #ifdef __WXGTK__
        m_view->CaptureMouse();
    #endif
#else
    Move(x, y);
    Show(true);
#endif
}

wxTipWindow::~wxTipWindow()
{
    if ( m_windowPtr )
    {
        *m_windowPtr = NULL;
    }
    #if wxUSE_POPUPWIN
        #ifdef __WXGTK__
            if ( m_view->HasCapture() )
                m_view->ReleaseMouse();
        #endif
    #endif
}

void wxTipWindow::OnMouseClick(wxMouseEvent& WXUNUSED(event))
{
    Close();
}

#if wxUSE_POPUPWIN

void wxTipWindow::OnDismiss()
{
    Close();
}

#else 
void wxTipWindow::OnActivate(wxActivateEvent& event)
{
    if (!event.GetActive())
        Close();
}

void wxTipWindow::OnKillFocus(wxFocusEvent& WXUNUSED(event))
{
            #ifdef __WXGTK__
    Close();
#endif
}

#endif 
void wxTipWindow::SetBoundingRect(const wxRect& rectBound)
{
    m_rectBound = rectBound;
}

void wxTipWindow::Close()
{
    if ( m_windowPtr )
    {
        *m_windowPtr = NULL;
        m_windowPtr = NULL;
    }
        if (!IsShown())
        return;

#if wxUSE_POPUPWIN
    Show(false);
    #ifdef __WXGTK__
        if ( m_view->HasCapture() )
            m_view->ReleaseMouse();
    #endif
            #ifndef __WXOSX__
        Destroy();
    #endif
#else
    wxFrame::Close();
#endif
}


wxTipWindowView::wxTipWindowView(wxWindow *parent)
               : wxWindow(parent, wxID_ANY,
                          wxDefaultPosition, wxDefaultSize,
                          wxNO_BORDER)
{
        SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));

#if !wxUSE_POPUPWIN
    m_creationTime = wxGetLocalTime();
#endif 
    m_parent = (wxTipWindow*)parent;
}

void wxTipWindowView::Adjust(const wxString& text, wxCoord maxLength)
{
    wxClientDC dc(this);
    dc.SetFont(GetFont());

            wxString current;
    wxCoord height, width,
            widthMax = 0;
    m_parent->m_heightLine = 0;

    bool breakLine = false;
    for ( const wxChar *p = text.c_str(); ; p++ )
    {
        if ( *p == wxT('\n') || *p == wxT('\0') )
        {
            dc.GetTextExtent(current, &width, &height);
            if ( width > widthMax )
                widthMax = width;

            if ( height > m_parent->m_heightLine )
                m_parent->m_heightLine = height;

            m_parent->m_textLines.Add(current);

            if ( !*p )
            {
                                break;
            }

            current.clear();
            breakLine = false;
        }
        else if ( breakLine && (*p == wxT(' ') || *p == wxT('\t')) )
        {
                        m_parent->m_textLines.Add(current);
            current.clear();
            breakLine = false;
        }
        else         {
            current += *p;
            dc.GetTextExtent(current, &width, &height);
            if ( width > maxLength )
                breakLine = true;

            if ( width > widthMax )
                widthMax = width;

            if ( height > m_parent->m_heightLine )
                m_parent->m_heightLine = height;
        }
    }

        width  = 2*(TEXT_MARGIN_X + 1) + widthMax;
    height = 2*(TEXT_MARGIN_Y + 1) + wx_truncate_cast(wxCoord, m_parent->m_textLines.GetCount())*m_parent->m_heightLine;
    m_parent->SetClientSize(width, height);
    SetSize(0, 0, width, height);
}

void wxTipWindowView::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);

    wxRect rect;
    wxSize size = GetClientSize();
    rect.width = size.x;
    rect.height = size.y;

        dc.SetBrush(wxBrush(GetBackgroundColour(), wxBRUSHSTYLE_SOLID));
    dc.SetPen(wxPen(GetForegroundColour(), 1, wxPENSTYLE_SOLID));
    dc.DrawRectangle(rect);

        dc.SetTextBackground(GetBackgroundColour());
    dc.SetTextForeground(GetForegroundColour());
    dc.SetFont(GetFont());

    wxPoint pt;
    pt.x = TEXT_MARGIN_X;
    pt.y = TEXT_MARGIN_Y;
    size_t count = m_parent->m_textLines.GetCount();
    for ( size_t n = 0; n < count; n++ )
    {
        dc.DrawText(m_parent->m_textLines[n], pt);

        pt.y += m_parent->m_heightLine;
    }
}

void wxTipWindowView::OnMouseClick(wxMouseEvent& WXUNUSED(event))
{
    m_parent->Close();
}

void wxTipWindowView::OnMouseMove(wxMouseEvent& event)
{
    const wxRect& rectBound = m_parent->m_rectBound;

    if ( rectBound.width &&
            !rectBound.Contains(ClientToScreen(event.GetPosition())) )
    {
                m_parent->Close();
    }
    else
    {
        event.Skip();
    }
}

#if !wxUSE_POPUPWIN
void wxTipWindowView::OnKillFocus(wxFocusEvent& WXUNUSED(event))
{
        if (wxGetLocalTime() > m_creationTime + 1)
        m_parent->Close();
}
#endif 
#endif 