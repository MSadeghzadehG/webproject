
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/frame.h"
#endif

#include "wx/laywin.h"
#include "wx/mdi.h"


wxIMPLEMENT_DYNAMIC_CLASS(wxQueryLayoutInfoEvent, wxEvent);
wxIMPLEMENT_DYNAMIC_CLASS(wxCalculateLayoutEvent, wxEvent);

wxDEFINE_EVENT( wxEVT_QUERY_LAYOUT_INFO, wxQueryLayoutInfoEvent );
wxDEFINE_EVENT( wxEVT_CALCULATE_LAYOUT, wxCalculateLayoutEvent );



#if wxUSE_SASH
wxIMPLEMENT_CLASS(wxSashLayoutWindow, wxSashWindow);
wxBEGIN_EVENT_TABLE(wxSashLayoutWindow, wxSashWindow)
    EVT_CALCULATE_LAYOUT(wxSashLayoutWindow::OnCalculateLayout)
    EVT_QUERY_LAYOUT_INFO(wxSashLayoutWindow::OnQueryLayoutInfo)
wxEND_EVENT_TABLE()

bool wxSashLayoutWindow::Create(wxWindow *parent, wxWindowID id, const wxPoint& pos,
        const wxSize& size, long style, const wxString& name)
{
    return wxSashWindow::Create(parent, id, pos, size, style, name);
}

void wxSashLayoutWindow::Init()
{
    m_orientation = wxLAYOUT_HORIZONTAL;
    m_alignment = wxLAYOUT_TOP;
#ifdef __WXMAC__
    MacSetClipChildren( true ) ;
#endif
}

void wxSashLayoutWindow::OnQueryLayoutInfo(wxQueryLayoutInfoEvent& event)
{
      int requestedLength = event.GetRequestedLength();

    event.SetOrientation(m_orientation);
    event.SetAlignment(m_alignment);

    if (m_orientation == wxLAYOUT_HORIZONTAL)
        event.SetSize(wxSize(requestedLength, m_defaultSize.y));
    else
        event.SetSize(wxSize(m_defaultSize.x, requestedLength));
}


void wxSashLayoutWindow::OnCalculateLayout(wxCalculateLayoutEvent& event)
{
    wxRect clientSize(event.GetRect());

    int flags = event.GetFlags();

    if (!IsShown())
        return;

                        
    wxRect thisRect;

        int length = (GetOrientation() == wxLAYOUT_HORIZONTAL) ? clientSize.width : clientSize.height;
    wxLayoutOrientation orient = GetOrientation();

                int whichDimension = (GetOrientation() == wxLAYOUT_HORIZONTAL) ? wxLAYOUT_LENGTH_X : wxLAYOUT_LENGTH_Y;

    wxQueryLayoutInfoEvent infoEvent(GetId());
    infoEvent.SetEventObject(this);
    infoEvent.SetRequestedLength(length);
    infoEvent.SetFlags(orient | whichDimension);

    if (!GetEventHandler()->ProcessEvent(infoEvent))
        return;

    wxSize sz = infoEvent.GetSize();

    if (sz.x == 0 && sz.y == 0)         return;

            switch (GetAlignment())
    {
        case wxLAYOUT_TOP:
        {
            thisRect.x = clientSize.x; thisRect.y = clientSize.y;
            thisRect.width = sz.x; thisRect.height = sz.y;
            clientSize.y += thisRect.height;
            clientSize.height -= thisRect.height;
            break;
        }
        case wxLAYOUT_LEFT:
        {
            thisRect.x = clientSize.x; thisRect.y = clientSize.y;
            thisRect.width = sz.x; thisRect.height = sz.y;
            clientSize.x += thisRect.width;
            clientSize.width -= thisRect.width;
            break;
        }
        case wxLAYOUT_RIGHT:
        {
            thisRect.x = clientSize.x + (clientSize.width - sz.x); thisRect.y = clientSize.y;
            thisRect.width = sz.x; thisRect.height = sz.y;
            clientSize.width -= thisRect.width;
            break;
        }
        case wxLAYOUT_BOTTOM:
        {
            thisRect.x = clientSize.x; thisRect.y = clientSize.y + (clientSize.height - sz.y);
            thisRect.width = sz.x; thisRect.height = sz.y;
            clientSize.height -= thisRect.height;
            break;
        }
        case wxLAYOUT_NONE:
        {
            break;
        }

    }

    if ((flags & wxLAYOUT_QUERY) == 0)
    {
                        wxSize sz2 = GetSize();
        wxPoint pos = GetPosition();
        SetSize(thisRect.x, thisRect.y, thisRect.width, thisRect.height);

                if ((pos.x != thisRect.x || pos.y != thisRect.y || sz2.x != thisRect.width || sz2.y != thisRect.height) &&
            (GetSashVisible(wxSASH_TOP) || GetSashVisible(wxSASH_RIGHT) || GetSashVisible(wxSASH_BOTTOM) || GetSashVisible(wxSASH_LEFT)))
            Refresh(true);

    }

    event.SetRect(clientSize);
}
#endif 


#if wxUSE_MDI_ARCHITECTURE

bool wxLayoutAlgorithm::LayoutMDIFrame(wxMDIParentFrame* frame, wxRect* r)
{
    int cw, ch;
    frame->GetClientSize(& cw, & ch);

    wxRect rect(0, 0, cw, ch);
    if (r)
        rect = * r;

    wxCalculateLayoutEvent event;
    event.SetRect(rect);

    wxWindowList::compatibility_iterator node = frame->GetChildren().GetFirst();
    while (node)
    {
        wxWindow* win = node->GetData();

        event.SetId(win->GetId());
        event.SetEventObject(win);
        event.SetFlags(0); 
        win->GetEventHandler()->ProcessEvent(event);

        node = node->GetNext();
    }

    wxWindow* clientWindow = frame->GetClientWindow();

    rect = event.GetRect();

    clientWindow->SetSize(rect.x, rect.y, rect.width, rect.height);

    return true;
}

#endif 
bool wxLayoutAlgorithm::LayoutFrame(wxFrame* frame, wxWindow* mainWindow)
{
    return LayoutWindow(frame, mainWindow);
}

bool wxLayoutAlgorithm::LayoutWindow(wxWindow* parent, wxWindow* mainWindow)
{
        
    int leftMargin = 0, rightMargin = 0, topMargin = 0, bottomMargin = 0;
#if wxUSE_SASH
    if (wxDynamicCast(parent, wxSashWindow))
    {
        wxSashWindow* sashWindow = (wxSashWindow*) parent;

        leftMargin = sashWindow->GetExtraBorderSize();
        rightMargin = sashWindow->GetExtraBorderSize();
        topMargin = sashWindow->GetExtraBorderSize();
        bottomMargin = sashWindow->GetExtraBorderSize();

        if (sashWindow->GetSashVisible(wxSASH_LEFT))
            leftMargin += sashWindow->GetDefaultBorderSize();
        if (sashWindow->GetSashVisible(wxSASH_RIGHT))
            rightMargin += sashWindow->GetDefaultBorderSize();
        if (sashWindow->GetSashVisible(wxSASH_TOP))
            topMargin += sashWindow->GetDefaultBorderSize();
        if (sashWindow->GetSashVisible(wxSASH_BOTTOM))
            bottomMargin += sashWindow->GetDefaultBorderSize();
    }
#endif 
    int cw, ch;
    parent->GetClientSize(& cw, & ch);

    wxRect rect(leftMargin, topMargin, cw - leftMargin - rightMargin, ch - topMargin - bottomMargin);

    wxCalculateLayoutEvent event;
    event.SetRect(rect);

            wxWindow *lastAwareWindow = NULL;
    wxWindowList::compatibility_iterator node = parent->GetChildren().GetFirst();

    while (node)
    {
        wxWindow* win = node->GetData();

        if (win->IsShown())
        {
            wxCalculateLayoutEvent tempEvent(win->GetId());
            tempEvent.SetEventObject(win);
            tempEvent.SetFlags(wxLAYOUT_QUERY);
            tempEvent.SetRect(event.GetRect());
            if (win->GetEventHandler()->ProcessEvent(tempEvent))
                lastAwareWindow = win;
        }

        node = node->GetNext();
    }

        node = parent->GetChildren().GetFirst();
    while (node)
    {
        wxWindow* win = node->GetData();

                        if (win->IsShown() && (win != mainWindow) && (mainWindow != NULL || win != lastAwareWindow))
        {
            event.SetId(win->GetId());
            event.SetEventObject(win);
            event.SetFlags(wxLAYOUT_QUERY);

            win->GetEventHandler()->ProcessEvent(event);
        }

        node = node->GetNext();
    }

    if (event.GetRect().GetWidth() < 0 || event.GetRect().GetHeight() < 0)
        return false;

    event.SetRect(rect);

    node = parent->GetChildren().GetFirst();
    while (node)
    {
        wxWindow* win = node->GetData();

                        if (win->IsShown() && (win != mainWindow) && (mainWindow != NULL || win != lastAwareWindow))
        {
            event.SetId(win->GetId());
            event.SetEventObject(win);
            event.SetFlags(0); 
            win->GetEventHandler()->ProcessEvent(event);
        }

        node = node->GetNext();
    }

    rect = event.GetRect();

    if (mainWindow)
        mainWindow->SetSize(rect.x, rect.y, wxMax(0, rect.width), wxMax(0, rect.height));
    else if (lastAwareWindow)
    {
                lastAwareWindow->SetSize(rect.x, rect.y, wxMax(0, rect.width), wxMax(0, rect.height));
    }

    return true;
}

