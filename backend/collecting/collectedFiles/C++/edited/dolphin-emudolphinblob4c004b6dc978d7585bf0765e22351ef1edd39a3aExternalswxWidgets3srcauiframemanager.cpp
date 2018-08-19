


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_AUI

#include "wx/aui/framemanager.h"
#include "wx/aui/dockart.h"
#include "wx/aui/floatpane.h"
#include "wx/aui/tabmdi.h"
#include "wx/aui/auibar.h"
#include "wx/mdi.h"

#ifndef WX_PRECOMP
    #include "wx/panel.h"
    #include "wx/settings.h"
    #include "wx/app.h"
    #include "wx/dcclient.h"
    #include "wx/dcscreen.h"
    #include "wx/toolbar.h"
    #include "wx/image.h"
    #include "wx/statusbr.h"
#endif

WX_CHECK_BUILD_OPTIONS("wxAUI")

#include "wx/arrimpl.cpp"
WX_DECLARE_OBJARRAY(wxRect, wxAuiRectArray);
WX_DEFINE_OBJARRAY(wxAuiRectArray)
WX_DEFINE_OBJARRAY(wxAuiDockUIPartArray)
WX_DEFINE_OBJARRAY(wxAuiDockInfoArray)
WX_DEFINE_OBJARRAY(wxAuiPaneButtonArray)
WX_DEFINE_OBJARRAY(wxAuiPaneInfoArray)

wxAuiPaneInfo wxAuiNullPaneInfo;
wxAuiDockInfo wxAuiNullDockInfo;
wxDEFINE_EVENT( wxEVT_AUI_PANE_BUTTON, wxAuiManagerEvent );
wxDEFINE_EVENT( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEvent );
wxDEFINE_EVENT( wxEVT_AUI_PANE_MAXIMIZE, wxAuiManagerEvent );
wxDEFINE_EVENT( wxEVT_AUI_PANE_RESTORE, wxAuiManagerEvent );
wxDEFINE_EVENT( wxEVT_AUI_PANE_ACTIVATED, wxAuiManagerEvent );
wxDEFINE_EVENT( wxEVT_AUI_RENDER, wxAuiManagerEvent );
wxDEFINE_EVENT( wxEVT_AUI_FIND_MANAGER, wxAuiManagerEvent );

#ifdef __WXMAC__
        #define __MAC_OS_X_MEMORY_MANAGER_CLEAN__ 1
    #define __AIFF__
    #include "wx/osx/private.h"
#endif

#ifdef __WXMSW__
    #include "wx/msw/wrapwin.h"
    #include "wx/msw/private.h"
    #include "wx/msw/dc.h"
#endif

wxIMPLEMENT_DYNAMIC_CLASS(wxAuiManagerEvent, wxEvent);
wxIMPLEMENT_CLASS(wxAuiManager, wxEvtHandler);



const int auiToolBarLayer = 10;

#ifndef __WXGTK20__


class wxPseudoTransparentFrame : public wxFrame
{
public:
    wxPseudoTransparentFrame(wxWindow* parent = NULL,
                wxWindowID id = wxID_ANY,
                const wxString& title = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxDEFAULT_FRAME_STYLE,
                const wxString &name = wxT("frame"))
                    : wxFrame(parent, id, title, pos, size, style | wxFRAME_SHAPED, name)
    {
        SetBackgroundStyle(wxBG_STYLE_CUSTOM);
        m_amount=0;
        m_maxWidth=0;
        m_maxHeight=0;
        m_lastWidth=0;
        m_lastHeight=0;
#ifdef __WXGTK__
        m_canSetShape = false; #else
        m_canSetShape = true;
#endif
        m_region = wxRegion(0, 0, 0, 0);
        SetTransparent(0);
    }

    virtual bool SetTransparent(wxByte alpha) wxOVERRIDE
    {
        if (m_canSetShape)
        {
            int w=100;             int h=100;
            GetClientSize(&w, &h);

            m_maxWidth = w;
            m_maxHeight = h;
            m_amount = alpha;
            m_region.Clear();
            if (m_amount)
            {
                for (int y=0; y<m_maxHeight; y++)
                {
                                        int j=((y&8)?1:0)|((y&4)?2:0)|((y&2)?4:0)|((y&1)?8:0);
                    if ((j*16+8)<m_amount)
                        m_region.Union(0, y, m_maxWidth, 1);
                }
            }
            SetShape(m_region);
            Refresh();
        }
        return true;
    }

    void OnPaint(wxPaintEvent& WXUNUSED(event))
    {
        wxPaintDC dc(this);

        if (m_region.IsEmpty())
            return;

#ifdef __WXMAC__
        dc.SetBrush(wxColour(128, 192, 255));
#else
        dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
#endif
        dc.SetPen(*wxTRANSPARENT_PEN);

        wxRegionIterator upd(GetUpdateRegion()); 
        while (upd)
        {
            wxRect rect(upd.GetRect());
            dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);

            ++upd;
        }
    }

#ifdef __WXGTK__
    void OnWindowCreate(wxWindowCreateEvent& WXUNUSED(event))
    {
        m_canSetShape=true;
        SetTransparent(0);
    }
#endif

    void OnSize(wxSizeEvent& event)
    {
                if ((event.GetSize().GetWidth() == m_lastWidth) &&
            (event.GetSize().GetHeight() == m_lastHeight))
        {
            event.Skip();
            return;
        }
        m_lastWidth = event.GetSize().GetWidth();
        m_lastHeight = event.GetSize().GetHeight();

        SetTransparent(m_amount);
        m_region.Intersect(0, 0, event.GetSize().GetWidth(),
                           event.GetSize().GetHeight());
        SetShape(m_region);
        Refresh();
        event.Skip();
    }

private:
    wxByte m_amount;
    int m_maxWidth;
    int m_maxHeight;
    bool m_canSetShape;
    int m_lastWidth,m_lastHeight;

    wxRegion m_region;

    wxDECLARE_DYNAMIC_CLASS(wxPseudoTransparentFrame);
    wxDECLARE_EVENT_TABLE();
};


wxIMPLEMENT_DYNAMIC_CLASS(wxPseudoTransparentFrame, wxFrame);

wxBEGIN_EVENT_TABLE(wxPseudoTransparentFrame, wxFrame)
    EVT_PAINT(wxPseudoTransparentFrame::OnPaint)
    EVT_SIZE(wxPseudoTransparentFrame::OnSize)
#ifdef __WXGTK__
    EVT_WINDOW_CREATE(wxPseudoTransparentFrame::OnWindowCreate)
#endif
wxEND_EVENT_TABLE()


#else
  
#include <gtk/gtk.h>
#include "wx/gtk/private/gtk2-compat.h"

static void
gtk_pseudo_window_realized_callback( GtkWidget *m_widget, void *WXUNUSED(win) )
{
        wxSize disp = wxGetDisplaySize();
        int amount = 128;
        wxRegion region;
        for (int y=0; y<disp.y; y++)
                {
                                        int j=((y&8)?1:0)|((y&4)?2:0)|((y&2)?4:0)|((y&1)?8:0);
                    if ((j*16+8)<amount)
                        region.Union(0, y, disp.x, 1);
                }
        gdk_window_shape_combine_region(gtk_widget_get_window(m_widget), region.GetRegion(), 0, 0);
}


class wxPseudoTransparentFrame: public wxFrame
{
public:
    wxPseudoTransparentFrame(wxWindow* parent = NULL,
                wxWindowID id = wxID_ANY,
                const wxString& title = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxDEFAULT_FRAME_STYLE,
                const wxString &name = wxT("frame"))
    {
         if (!CreateBase( parent, id, pos, size, style, wxDefaultValidator, name ))
            return;

        m_title = title;

        m_widget = gtk_window_new( GTK_WINDOW_POPUP );
        g_object_ref(m_widget);

        if (parent) parent->AddChild(this);

        g_signal_connect( m_widget, "realize",
                      G_CALLBACK (gtk_pseudo_window_realized_callback), this );

                                                        GdkColor col;
        col.red = 128 * 256;
        col.green = 192 * 256;
        col.blue = 255 * 256;

        wxGCC_WARNING_SUPPRESS(deprecated-declarations)
        gtk_widget_modify_bg( m_widget, GTK_STATE_NORMAL, &col );
        wxGCC_WARNING_RESTORE(deprecated-declarations)
    }

    bool SetTransparent(wxByte WXUNUSED(alpha)) wxOVERRIDE
    {
        return true;
    }

protected:
    virtual void DoSetSizeHints( int minW, int minH,
                                 int maxW, int maxH,
                                 int incW, int incH) wxOVERRIDE
    {
                        wxWindow::DoSetSizeHints(minW, minH, maxW, maxH, incW, incH);
    }

private:
    wxDECLARE_DYNAMIC_CLASS(wxPseudoTransparentFrame);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxPseudoTransparentFrame, wxFrame);

#endif
 



static wxBitmap wxPaneCreateStippleBitmap()
{
    unsigned char data[] = { 0,0,0,192,192,192, 192,192,192,0,0,0 };
    wxImage img(2,2,data,true);
    return wxBitmap(img);
}

static void DrawResizeHint(wxDC& dc, const wxRect& rect)
{
    wxBitmap stipple = wxPaneCreateStippleBitmap();
    wxBrush brush(stipple);
    dc.SetBrush(brush);
#ifdef __WXMSW__
    wxMSWDCImpl *impl = (wxMSWDCImpl*) dc.GetImpl();
    PatBlt(GetHdcOf(*impl), rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight(), PATINVERT);
#else
    dc.SetPen(*wxTRANSPARENT_PEN);

    dc.SetLogicalFunction(wxXOR);
    dc.DrawRectangle(rect);
#endif
}




static void CopyDocksAndPanes(wxAuiDockInfoArray& dest_docks,
                              wxAuiPaneInfoArray& dest_panes,
                              const wxAuiDockInfoArray& src_docks,
                              const wxAuiPaneInfoArray& src_panes)
{
    dest_docks = src_docks;
    dest_panes = src_panes;
    int i, j, k, dock_count, pc1, pc2;
    for (i = 0, dock_count = dest_docks.GetCount(); i < dock_count; ++i)
    {
        wxAuiDockInfo& dock = dest_docks.Item(i);
        for (j = 0, pc1 = dock.panes.GetCount(); j < pc1; ++j)
            for (k = 0, pc2 = src_panes.GetCount(); k < pc2; ++k)
                if (dock.panes.Item(j) == &src_panes.Item(k))
                    dock.panes.Item(j) = &dest_panes.Item(k);
    }
}

static int GetMaxLayer(const wxAuiDockInfoArray& docks,
                       int dock_direction)
{
    int i, dock_count, max_layer = 0;
    for (i = 0, dock_count = docks.GetCount(); i < dock_count; ++i)
    {
        wxAuiDockInfo& dock = docks.Item(i);
        if (dock.dock_direction == dock_direction &&
            dock.dock_layer > max_layer && !dock.fixed)
                max_layer = dock.dock_layer;
    }
    return max_layer;
}


static int GetMaxRow(const wxAuiPaneInfoArray& panes, int direction, int layer)
{
    int i, pane_count, max_row = 0;
    for (i = 0, pane_count = panes.GetCount(); i < pane_count; ++i)
    {
        wxAuiPaneInfo& pane = panes.Item(i);
        if (pane.dock_direction == direction &&
            pane.dock_layer == layer &&
            pane.dock_row > max_row)
                max_row = pane.dock_row;
    }
    return max_row;
}



static void DoInsertDockLayer(wxAuiPaneInfoArray& panes,
                              int dock_direction,
                              int dock_layer)
{
    int i, pane_count;
    for (i = 0, pane_count = panes.GetCount(); i < pane_count; ++i)
    {
        wxAuiPaneInfo& pane = panes.Item(i);
        if (!pane.IsFloating() &&
            pane.dock_direction == dock_direction &&
            pane.dock_layer >= dock_layer)
                pane.dock_layer++;
    }
}

static void DoInsertDockRow(wxAuiPaneInfoArray& panes,
                            int dock_direction,
                            int dock_layer,
                            int dock_row)
{
    int i, pane_count;
    for (i = 0, pane_count = panes.GetCount(); i < pane_count; ++i)
    {
        wxAuiPaneInfo& pane = panes.Item(i);
        if (!pane.IsFloating() &&
            pane.dock_direction == dock_direction &&
            pane.dock_layer == dock_layer &&
            pane.dock_row >= dock_row)
                pane.dock_row++;
    }
}

static void DoInsertPane(wxAuiPaneInfoArray& panes,
                         int dock_direction,
                         int dock_layer,
                         int dock_row,
                         int dock_pos)
{
    int i, pane_count;
    for (i = 0, pane_count = panes.GetCount(); i < pane_count; ++i)
    {
        wxAuiPaneInfo& pane = panes.Item(i);
        if (!pane.IsFloating() &&
            pane.dock_direction == dock_direction &&
            pane.dock_layer == dock_layer &&
            pane.dock_row == dock_row &&
            pane.dock_pos >= dock_pos)
                pane.dock_pos++;
    }
}

static void FindDocks(wxAuiDockInfoArray& docks,
                      int dock_direction,
                      int dock_layer,
                      int dock_row,
                      wxAuiDockInfoPtrArray& arr)
{
    int begin_layer = dock_layer;
    int end_layer = dock_layer;
    int begin_row = dock_row;
    int end_row = dock_row;
    int dock_count = docks.GetCount();
    int layer, row, i, max_row = 0, max_layer = 0;

        for (i = 0; i < dock_count; ++i)
    {
        max_row = wxMax(max_row, docks.Item(i).dock_row);
        max_layer = wxMax(max_layer, docks.Item(i).dock_layer);
    }

        if (dock_layer == -1)
    {
        begin_layer = 0;
        end_layer = max_layer;
    }

        if (dock_row == -1)
    {
        begin_row = 0;
        end_row = max_row;
    }

    arr.Clear();

    for (layer = begin_layer; layer <= end_layer; ++layer)
        for (row = begin_row; row <= end_row; ++row)
            for (i = 0; i < dock_count; ++i)
            {
                wxAuiDockInfo& d = docks.Item(i);
                if (dock_direction == -1 || dock_direction == d.dock_direction)
                {
                    if (d.dock_layer == layer && d.dock_row == row)
                        arr.Add(&d);
                }
            }
}

static wxAuiPaneInfo* FindPaneInDock(const wxAuiDockInfo& dock, wxWindow* window)
{
    int i, count = dock.panes.GetCount();
    for (i = 0; i < count; ++i)
    {
        wxAuiPaneInfo* p = dock.panes.Item(i);
        if (p->window == window)
            return p;
    }
    return NULL;
}

static void RemovePaneFromDocks(wxAuiDockInfoArray& docks,
                                wxAuiPaneInfo& pane,
                                wxAuiDockInfo* ex_cept  = NULL  )
{
    int i, dock_count;
    for (i = 0, dock_count = docks.GetCount(); i < dock_count; ++i)
    {
        wxAuiDockInfo& d = docks.Item(i);
        if (&d == ex_cept)
            continue;
        wxAuiPaneInfo* pi = FindPaneInDock(d, pane.window);
        if (pi)
            d.panes.Remove(pi);
    }
}




void wxAuiManager::SetActivePane(wxWindow* active_pane)
{
    int i, pane_count;
    wxAuiPaneInfo* active_paneinfo = NULL;
    for (i = 0, pane_count = m_panes.GetCount(); i < pane_count; ++i)
    {
        wxAuiPaneInfo& pane = m_panes.Item(i);
        pane.state &= ~wxAuiPaneInfo::optionActive;
        if (pane.window == active_pane)
        {
            pane.state |= wxAuiPaneInfo::optionActive;
            active_paneinfo = &pane;
        }
    }

        if ( active_paneinfo )
    {
        wxAuiManagerEvent evt(wxEVT_AUI_PANE_ACTIVATED);
        evt.SetManager(this);
        evt.SetPane(active_paneinfo);
        ProcessMgrEvent(evt);
    }
}


static int PaneSortFunc(wxAuiPaneInfo** p1, wxAuiPaneInfo** p2)
{
    return ((*p1)->dock_pos < (*p2)->dock_pos) ? -1 : 1;
}


bool wxAuiPaneInfo::IsValid() const
{
                wxAuiToolBar* toolbar = wxDynamicCast(window, wxAuiToolBar);
    return !toolbar || toolbar->IsPaneValid(*this);
}



wxBEGIN_EVENT_TABLE(wxAuiManager, wxEvtHandler)
    EVT_AUI_PANE_BUTTON(wxAuiManager::OnPaneButton)
    EVT_AUI_RENDER(wxAuiManager::OnRender)
    EVT_PAINT(wxAuiManager::OnPaint)
    EVT_ERASE_BACKGROUND(wxAuiManager::OnEraseBackground)
    EVT_SIZE(wxAuiManager::OnSize)
    EVT_SET_CURSOR(wxAuiManager::OnSetCursor)
    EVT_LEFT_DOWN(wxAuiManager::OnLeftDown)
    EVT_LEFT_UP(wxAuiManager::OnLeftUp)
    EVT_MOTION(wxAuiManager::OnMotion)
    EVT_LEAVE_WINDOW(wxAuiManager::OnLeaveWindow)
    EVT_MOUSE_CAPTURE_LOST(wxAuiManager::OnCaptureLost)
    EVT_CHILD_FOCUS(wxAuiManager::OnChildFocus)
    EVT_AUI_FIND_MANAGER(wxAuiManager::OnFindManager)
wxEND_EVENT_TABLE()


wxAuiManager::wxAuiManager(wxWindow* managed_wnd, unsigned int flags)
{
    m_action = actionNone;
    m_actionWindow = NULL;
    m_lastMouseMove = wxPoint();
    m_hoverButton = NULL;
    m_art = new wxAuiDefaultDockArt;
    m_hintWnd = NULL;
    m_flags = flags;
    m_skipping = false;
    m_hasMaximized = false;
    m_frame = NULL;
    m_dockConstraintX = 0.3;
    m_dockConstraintY = 0.3;
    m_reserved = NULL;
    m_currentDragItem = -1;

    if (managed_wnd)
    {
        SetManagedWindow(managed_wnd);
    }
}

wxAuiManager::~wxAuiManager()
{
                #if 0
    for ( size_t i = 0; i < m_panes.size(); i++ )
    {
        wxAuiPaneInfo& pinfo = m_panes[i];
        if (pinfo.window && !pinfo.window->GetParent())
            delete pinfo.window;
    }
#endif

    delete m_art;
}

wxAuiFloatingFrame* wxAuiManager::CreateFloatingFrame(wxWindow* parent,
                                                      const wxAuiPaneInfo& paneInfo)
{
    return new wxAuiFloatingFrame(parent, this, paneInfo);
}

bool wxAuiManager::CanDockPanel(const wxAuiPaneInfo & WXUNUSED(p))
{
            return !(wxGetKeyState(WXK_CONTROL) || wxGetKeyState(WXK_ALT));
}


wxAuiPaneInfo& wxAuiManager::GetPane(wxWindow* window)
{
    int i, pane_count;
    for (i = 0, pane_count = m_panes.GetCount(); i < pane_count; ++i)
    {
        wxAuiPaneInfo& p = m_panes.Item(i);
        if (p.window == window)
            return p;
    }
    return wxAuiNullPaneInfo;
}

wxAuiPaneInfo& wxAuiManager::GetPane(const wxString& name)
{
    int i, pane_count;
    for (i = 0, pane_count = m_panes.GetCount(); i < pane_count; ++i)
    {
        wxAuiPaneInfo& p = m_panes.Item(i);
        if (p.name == name)
            return p;
    }
    return wxAuiNullPaneInfo;
}

wxAuiPaneInfoArray& wxAuiManager::GetAllPanes()
{
    return m_panes;
}

wxAuiDockUIPart* wxAuiManager::HitTest(int x, int y)
{
    wxAuiDockUIPart* result = NULL;

    int i, part_count;
    for (i = 0, part_count = m_uiParts.GetCount(); i < part_count; ++i)
    {
        wxAuiDockUIPart* item = &m_uiParts.Item(i);

                                        if (item->type == wxAuiDockUIPart::typeDock)
            continue;

                                if ((item->type == wxAuiDockUIPart::typePane ||
            item->type == wxAuiDockUIPart::typePaneBorder) && result)
            continue;

                if (item->rect.Contains(x,y))
            result = item;
    }

    return result;
}


void wxAuiManager::SetFlags(unsigned int flags)
{
        bool update_hint_wnd = false;
    unsigned int hint_mask = wxAUI_MGR_TRANSPARENT_HINT |
                             wxAUI_MGR_VENETIAN_BLINDS_HINT |
                             wxAUI_MGR_RECTANGLE_HINT;
    if ((flags & hint_mask) != (m_flags & hint_mask))
        update_hint_wnd = true;


        m_flags = flags;

    if (update_hint_wnd)
    {
        UpdateHintWindowConfig();
    }
}

unsigned int wxAuiManager::GetFlags() const
{
    return m_flags;
}

bool wxAuiManager_HasLiveResize(wxAuiManager& manager)
{
        #if defined(__WXMAC__)
    wxUnusedVar(manager);
    return true;
#else
    return (manager.GetFlags() & wxAUI_MGR_LIVE_RESIZE) == wxAUI_MGR_LIVE_RESIZE;
#endif
}

void wxAuiManager::SetFrame(wxFrame* frame)
{
    SetManagedWindow((wxWindow*)frame);
}

wxFrame* wxAuiManager::GetFrame() const
{
    return (wxFrame*)m_frame;
}


wxAuiManager* wxAuiManager::GetManager(wxWindow* window)
{
    wxAuiManagerEvent evt(wxEVT_AUI_FIND_MANAGER);
    evt.SetManager(NULL);
    evt.ResumePropagation(wxEVENT_PROPAGATE_MAX);
    if (!window->GetEventHandler()->ProcessEvent(evt))
        return NULL;

    return evt.GetManager();
}


void wxAuiManager::UpdateHintWindowConfig()
{
        bool can_do_transparent = false;

    wxWindow* w = m_frame;
    while (w)
    {
        if (wxDynamicCast(w, wxFrame))
        {
            wxFrame* f = static_cast<wxFrame*>(w);
            can_do_transparent = f->CanSetTransparent();

            break;
        }

        w = w->GetParent();
    }

        if (m_hintWnd)
    {
        m_hintWnd->Destroy();
        m_hintWnd = NULL;
    }

    m_hintFadeMax = 50;
    m_hintWnd = NULL;

    if ((m_flags & wxAUI_MGR_TRANSPARENT_HINT) && can_do_transparent)
    {
                #if defined(__WXMSW__) || defined(__WXGTK__)
            m_hintWnd = new wxFrame(m_frame, wxID_ANY, wxEmptyString,
                                     wxDefaultPosition, wxSize(1,1),
                                         wxFRAME_TOOL_WINDOW |
                                         wxFRAME_FLOAT_ON_PARENT |
                                         wxFRAME_NO_TASKBAR |
                                         wxNO_BORDER);

            m_hintWnd->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
        #elif defined(__WXMAC__)
                                    m_hintWnd = new wxMiniFrame(m_frame, wxID_ANY, wxEmptyString,
                                         wxDefaultPosition, wxSize(1,1),
                                         wxFRAME_FLOAT_ON_PARENT
                                         | wxFRAME_TOOL_WINDOW );
            m_hintWnd->Connect(wxEVT_ACTIVATE,
                wxActivateEventHandler(wxAuiManager::OnHintActivate), NULL, this);

                        wxPanel* p = new wxPanel(m_hintWnd);

                                                            p->SetBackgroundColour(*wxBLUE);
        #endif

    }
    else
    {
        if ((m_flags & wxAUI_MGR_TRANSPARENT_HINT) != 0 ||
            (m_flags & wxAUI_MGR_VENETIAN_BLINDS_HINT) != 0)
        {
                                    m_hintWnd = new wxPseudoTransparentFrame(m_frame,
                                                      wxID_ANY,
                                                      wxEmptyString,
                                                      wxDefaultPosition,
                                                      wxSize(1,1),
                                                            wxFRAME_TOOL_WINDOW |
                                                            wxFRAME_FLOAT_ON_PARENT |
                                                            wxFRAME_NO_TASKBAR |
                                                            wxNO_BORDER);
            m_hintFadeMax = 128;
        }
    }
}


void wxAuiManager::SetManagedWindow(wxWindow* wnd)
{
    wxASSERT_MSG(wnd, wxT("specified window must be non-NULL"));

    m_frame = wnd;
    m_frame->PushEventHandler(this);

#if wxUSE_MDI
            
    if (wxDynamicCast(m_frame, wxMDIParentFrame))
    {
        wxMDIParentFrame* mdi_frame = (wxMDIParentFrame*)m_frame;
        wxWindow* client_window = mdi_frame->GetClientWindow();

        wxASSERT_MSG(client_window, wxT("Client window is NULL!"));

        AddPane(client_window,
                wxAuiPaneInfo().Name(wxT("mdiclient")).
                CenterPane().PaneBorder(false));
    }
    else if (wxDynamicCast(m_frame, wxAuiMDIParentFrame))
    {
        wxAuiMDIParentFrame* mdi_frame = (wxAuiMDIParentFrame*)m_frame;
        wxAuiMDIClientWindow* client_window = mdi_frame->GetClientWindow();
        wxASSERT_MSG(client_window, wxT("Client window is NULL!"));

        AddPane(client_window,
                wxAuiPaneInfo().Name(wxT("mdiclient")).
                CenterPane().PaneBorder(false));
    }

#endif

    UpdateHintWindowConfig();
}


void wxAuiManager::UnInit()
{
    if (m_frame)
    {
        m_frame->RemoveEventHandler(this);
    }
}

wxWindow* wxAuiManager::GetManagedWindow() const
{
    return m_frame;
}

wxAuiDockArt* wxAuiManager::GetArtProvider() const
{
    return m_art;
}

void wxAuiManager::ProcessMgrEvent(wxAuiManagerEvent& event)
{
        if (m_frame)
    {
        if (m_frame->GetEventHandler()->ProcessEvent(event))
            return;
    }

    ProcessEvent(event);
}

void wxAuiManager::SetArtProvider(wxAuiDockArt* art_provider)
{
        delete m_art;

        m_art = art_provider;
}


bool wxAuiManager::AddPane(wxWindow* window, const wxAuiPaneInfo& paneInfo)
{
    wxASSERT_MSG(window, wxT("NULL window ptrs are not allowed"));

        if (!window)
        return false;

        if (GetPane(paneInfo.window).IsOk())
        return false;

            bool already_exists = false;
    if (!paneInfo.name.empty() && GetPane(paneInfo.name).IsOk())
    {
        wxFAIL_MSG(wxT("A pane with that name already exists in the manager!"));
        already_exists = true;
    }

        if (paneInfo.IsDocked())
        RestoreMaximizedPane();

        wxAuiPaneInfo test(paneInfo);
    wxAuiToolBar* toolbar = wxDynamicCast(window, wxAuiToolBar);
    if (toolbar)
    {
                const unsigned int dockMask = wxAuiPaneInfo::optionLeftDockable |
                                        wxAuiPaneInfo::optionRightDockable |
                                        wxAuiPaneInfo::optionTopDockable |
                                        wxAuiPaneInfo::optionBottomDockable;
        const unsigned int defaultDock = wxAuiPaneInfo().
                                            DefaultPane().state & dockMask;
        if ((test.state & dockMask) == defaultDock)
        {
                        if (toolbar->GetWindowStyleFlag() & wxAUI_TB_VERTICAL)
            {
                test.TopDockable(false).BottomDockable(false);
            }
            else if (toolbar->GetWindowStyleFlag() & wxAUI_TB_HORIZONTAL)
            {
                test.LeftDockable(false).RightDockable(false);
            }
        }
        else
        {
                        test.window = window;
            wxCHECK_MSG(test.IsValid(), false,
                        "toolbar style and pane docking flags are incompatible");
        }
    }

    m_panes.Add(test);

    wxAuiPaneInfo& pinfo = m_panes.Last();

        pinfo.window = window;


        if (pinfo.name.empty() || already_exists)
    {
        pinfo.name.Printf(wxT("%08lx%08x%08x%08lx"),
             (unsigned long)(wxPtrToUInt(pinfo.window) & 0xffffffff),
             (unsigned int)time(NULL),
             (unsigned int)clock(),
             (unsigned long)m_panes.GetCount());
    }

        if (pinfo.dock_proportion == 0)
        pinfo.dock_proportion = 100000;

    if (pinfo.HasMaximizeButton())
    {
        wxAuiPaneButton button;
        button.button_id = wxAUI_BUTTON_MAXIMIZE_RESTORE;
        pinfo.buttons.Add(button);
    }

    if (pinfo.HasPinButton())
    {
        wxAuiPaneButton button;
        button.button_id = wxAUI_BUTTON_PIN;
        pinfo.buttons.Add(button);
    }

    if (pinfo.HasCloseButton())
    {
        wxAuiPaneButton button;
        button.button_id = wxAUI_BUTTON_CLOSE;
        pinfo.buttons.Add(button);
    }

    if (pinfo.HasGripper())
    {
        if (wxDynamicCast(pinfo.window, wxAuiToolBar))
        {
                                                
            wxAuiToolBar* tb = static_cast<wxAuiToolBar*>(pinfo.window);
            pinfo.SetFlag(wxAuiPaneInfo::optionGripper, false);
            tb->SetGripperVisible(true);
        }
    }


    if (pinfo.best_size == wxDefaultSize &&
        pinfo.window)
    {
        pinfo.best_size = pinfo.window->GetClientSize();

#if wxUSE_TOOLBAR
        if (wxDynamicCast(pinfo.window, wxToolBar))
        {
                                                pinfo.best_size = pinfo.window->GetBestSize();
        }
#endif 
        if (pinfo.min_size != wxDefaultSize)
        {
            if (pinfo.best_size.x < pinfo.min_size.x)
                pinfo.best_size.x = pinfo.min_size.x;
            if (pinfo.best_size.y < pinfo.min_size.y)
                pinfo.best_size.y = pinfo.min_size.y;
        }
    }



    return true;
}

bool wxAuiManager::AddPane(wxWindow* window,
                           int direction,
                           const wxString& caption)
{
    wxAuiPaneInfo pinfo;
    pinfo.Caption(caption);
    switch (direction)
    {
        case wxTOP:    pinfo.Top(); break;
        case wxBOTTOM: pinfo.Bottom(); break;
        case wxLEFT:   pinfo.Left(); break;
        case wxRIGHT:  pinfo.Right(); break;
        case wxCENTER: pinfo.CenterPane(); break;
    }
    return AddPane(window, pinfo);
}

bool wxAuiManager::AddPane(wxWindow* window,
                           const wxAuiPaneInfo& paneInfo,
                           const wxPoint& drop_pos)
{
    if (!AddPane(window, paneInfo))
        return false;

    wxAuiPaneInfo& pane = GetPane(window);

    DoDrop(m_docks, m_panes, pane, drop_pos, wxPoint(0,0));

    return true;
}

bool wxAuiManager::InsertPane(wxWindow* window, const wxAuiPaneInfo& paneInfo,
                                int insert_level)
{
    wxASSERT_MSG(window, wxT("NULL window ptrs are not allowed"));

        switch (insert_level)
    {
        case wxAUI_INSERT_PANE:
            DoInsertPane(m_panes,
                 paneInfo.dock_direction,
                 paneInfo.dock_layer,
                 paneInfo.dock_row,
                 paneInfo.dock_pos);
            break;
        case wxAUI_INSERT_ROW:
            DoInsertDockRow(m_panes,
                 paneInfo.dock_direction,
                 paneInfo.dock_layer,
                 paneInfo.dock_row);
            break;
        case wxAUI_INSERT_DOCK:
            DoInsertDockLayer(m_panes,
                 paneInfo.dock_direction,
                 paneInfo.dock_layer);
            break;
    }

            wxAuiPaneInfo& existing_pane = GetPane(window);
    if (!existing_pane.IsOk())
    {
        return AddPane(window, paneInfo);
    }
    else
    {
        if (paneInfo.IsFloating())
        {
            existing_pane.Float();
            if (paneInfo.floating_pos != wxDefaultPosition)
                existing_pane.FloatingPosition(paneInfo.floating_pos);
            if (paneInfo.floating_size != wxDefaultSize)
                existing_pane.FloatingSize(paneInfo.floating_size);
        }
        else
        {
                        RestoreMaximizedPane();

            existing_pane.Direction(paneInfo.dock_direction);
            existing_pane.Layer(paneInfo.dock_layer);
            existing_pane.Row(paneInfo.dock_row);
            existing_pane.Position(paneInfo.dock_pos);
        }
    }

    return true;
}


bool wxAuiManager::DetachPane(wxWindow* window)
{
    wxASSERT_MSG(window, wxT("NULL window ptrs are not allowed"));

    int i, count;
    for (i = 0, count = m_panes.GetCount(); i < count; ++i)
    {
        wxAuiPaneInfo& p = m_panes.Item(i);
        if (p.window == window)
        {
            if (p.frame)
            {
                                
                                p.window->SetSize(1,1);

                if (p.frame->IsShown())
                    p.frame->Show(false);

                                if (m_actionWindow == p.frame)
                {
                    m_actionWindow = NULL;
                }

                p.window->Reparent(m_frame);
                p.frame->SetSizer(NULL);
                p.frame->Destroy();
                p.frame = NULL;
            }

                                                            int pi, part_count;
            for (pi = 0, part_count = (int)m_uiParts.GetCount(); pi < part_count; ++pi)
            {
                wxAuiDockUIPart& part = m_uiParts.Item(pi);
                if (part.pane == &p)
                {
                    m_uiParts.RemoveAt(pi);
                    part_count--;
                    pi--;
                    continue;
                }
            }

            m_panes.RemoveAt(i);
            return true;
        }
    }
    return false;
}

void wxAuiManager::ClosePane(wxAuiPaneInfo& paneInfo)
{
        if (paneInfo.IsMaximized())
    {
        RestorePane(paneInfo);
    }

        if (paneInfo.window && paneInfo.window->IsShown())
    {
        paneInfo.window->Show(false);
    }

        if (paneInfo.window && paneInfo.window->GetParent() != m_frame)
    {
        paneInfo.window->Reparent(m_frame);
    }

        if (paneInfo.frame)
    {
        paneInfo.frame->Destroy();
        paneInfo.frame = NULL;
    }

        if (paneInfo.IsDestroyOnClose())
    {
        wxWindow * window = paneInfo.window;
        DetachPane(window);
        if (window)
        {
            window->Destroy();
        }
    }
    else
    {
        paneInfo.Hide();
    }
}

void wxAuiManager::MaximizePane(wxAuiPaneInfo& paneInfo)
{
    int i, pane_count;

        for (i = 0, pane_count = m_panes.GetCount(); i < pane_count; ++i)
    {
        wxAuiPaneInfo& p = m_panes.Item(i);
        if (!p.IsToolbar() && !p.IsFloating())
        {
            p.Restore();

                        p.SetFlag(wxAuiPaneInfo::savedHiddenState,
                      p.HasFlag(wxAuiPaneInfo::optionHidden));

                                    p.Hide();
        }
    }

        paneInfo.Maximize();
    paneInfo.Show();
    m_hasMaximized = true;

        if (paneInfo.window && !paneInfo.window->IsShown())
    {
        paneInfo.window->Show(true);
    }
}

void wxAuiManager::RestorePane(wxAuiPaneInfo& paneInfo)
{
    int i, pane_count;

        for (i = 0, pane_count = m_panes.GetCount(); i < pane_count; ++i)
    {
        wxAuiPaneInfo& p = m_panes.Item(i);
        if (!p.IsToolbar() && !p.IsFloating())
        {
            p.SetFlag(wxAuiPaneInfo::optionHidden,
                      p.HasFlag(wxAuiPaneInfo::savedHiddenState));
        }
    }

        paneInfo.Restore();
    m_hasMaximized = false;

        if (paneInfo.window && !paneInfo.window->IsShown())
    {
        paneInfo.window->Show(true);
    }
}

void wxAuiManager::RestoreMaximizedPane()
{
    int i, pane_count;

        for (i = 0, pane_count = m_panes.GetCount(); i < pane_count; ++i)
    {
        wxAuiPaneInfo& p = m_panes.Item(i);
        if (p.IsMaximized())
        {
            RestorePane(p);
            break;
        }
    }
}

static wxString EscapeDelimiters(const wxString& s)
{
    wxString result;
    result.Alloc(s.length());
    const wxChar* ch = s.c_str();
    while (*ch)
    {
        if (*ch == wxT(';') || *ch == wxT('|'))
            result += wxT('\\');
        result += *ch;
        ++ch;
    }
    return result;
}

wxString wxAuiManager::SavePaneInfo(wxAuiPaneInfo& pane)
{
    wxString result = wxT("name=");
    result += EscapeDelimiters(pane.name);
    result += wxT(";");

    result += wxT("caption=");
    result += EscapeDelimiters(pane.caption);
    result += wxT(";");

    result += wxString::Format(wxT("state=%u;"), pane.state);
    result += wxString::Format(wxT("dir=%d;"), pane.dock_direction);
    result += wxString::Format(wxT("layer=%d;"), pane.dock_layer);
    result += wxString::Format(wxT("row=%d;"), pane.dock_row);
    result += wxString::Format(wxT("pos=%d;"), pane.dock_pos);
    result += wxString::Format(wxT("prop=%d;"), pane.dock_proportion);
    result += wxString::Format(wxT("bestw=%d;"), pane.best_size.x);
    result += wxString::Format(wxT("besth=%d;"), pane.best_size.y);
    result += wxString::Format(wxT("minw=%d;"), pane.min_size.x);
    result += wxString::Format(wxT("minh=%d;"), pane.min_size.y);
    result += wxString::Format(wxT("maxw=%d;"), pane.max_size.x);
    result += wxString::Format(wxT("maxh=%d;"), pane.max_size.y);
    result += wxString::Format(wxT("floatx=%d;"), pane.floating_pos.x);
    result += wxString::Format(wxT("floaty=%d;"), pane.floating_pos.y);
    result += wxString::Format(wxT("floatw=%d;"), pane.floating_size.x);
    result += wxString::Format(wxT("floath=%d"), pane.floating_size.y);

    return result;
}

void wxAuiManager::LoadPaneInfo(wxString pane_part, wxAuiPaneInfo &pane)
{
            pane_part.Replace(wxT("\\|"), wxT("\a"));
    pane_part.Replace(wxT("\\;"), wxT("\b"));

    while(1)
    {
        wxString val_part = pane_part.BeforeFirst(wxT(';'));
        pane_part = pane_part.AfterFirst(wxT(';'));
        wxString val_name = val_part.BeforeFirst(wxT('='));
        wxString value = val_part.AfterFirst(wxT('='));
        val_name.MakeLower();
        val_name.Trim(true);
        val_name.Trim(false);
        value.Trim(true);
        value.Trim(false);

        if (val_name.empty())
            break;

        if (val_name == wxT("name"))
            pane.name = value;
        else if (val_name == wxT("caption"))
            pane.caption = value;
        else if (val_name == wxT("state"))
            pane.state = (unsigned int)wxAtoi(value.c_str());
        else if (val_name == wxT("dir"))
            pane.dock_direction = wxAtoi(value.c_str());
        else if (val_name == wxT("layer"))
            pane.dock_layer = wxAtoi(value.c_str());
        else if (val_name == wxT("row"))
            pane.dock_row = wxAtoi(value.c_str());
        else if (val_name == wxT("pos"))
            pane.dock_pos = wxAtoi(value.c_str());
        else if (val_name == wxT("prop"))
            pane.dock_proportion = wxAtoi(value.c_str());
        else if (val_name == wxT("bestw"))
            pane.best_size.x = wxAtoi(value.c_str());
        else if (val_name == wxT("besth"))
            pane.best_size.y = wxAtoi(value.c_str());
        else if (val_name == wxT("minw"))
            pane.min_size.x = wxAtoi(value.c_str());
        else if (val_name == wxT("minh"))
            pane.min_size.y = wxAtoi(value.c_str());
        else if (val_name == wxT("maxw"))
            pane.max_size.x = wxAtoi(value.c_str());
        else if (val_name == wxT("maxh"))
            pane.max_size.y = wxAtoi(value.c_str());
        else if (val_name == wxT("floatx"))
            pane.floating_pos.x = wxAtoi(value.c_str());
        else if (val_name == wxT("floaty"))
            pane.floating_pos.y = wxAtoi(value.c_str());
        else if (val_name == wxT("floatw"))
            pane.floating_size.x = wxAtoi(value.c_str());
        else if (val_name == wxT("floath"))
            pane.floating_size.y = wxAtoi(value.c_str());
        else {
            wxFAIL_MSG(wxT("Bad Perspective String"));
        }
    }

            pane.name.Replace(wxT("\a"), wxT("|"));
    pane.name.Replace(wxT("\b"), wxT(";"));
    pane.caption.Replace(wxT("\a"), wxT("|"));
    pane.caption.Replace(wxT("\b"), wxT(";"));
    pane_part.Replace(wxT("\a"), wxT("|"));
    pane_part.Replace(wxT("\b"), wxT(";"));

    return;
}



wxString wxAuiManager::SavePerspective()
{
    wxString result;
    result.Alloc(500);
    result = wxT("layout2|");

    int pane_i, pane_count = m_panes.GetCount();
    for (pane_i = 0; pane_i < pane_count; ++pane_i)
    {
        wxAuiPaneInfo& pane = m_panes.Item(pane_i);
        result += SavePaneInfo(pane)+wxT("|");
    }

    int dock_i, dock_count = m_docks.GetCount();
    for (dock_i = 0; dock_i < dock_count; ++dock_i)
    {
        wxAuiDockInfo& dock = m_docks.Item(dock_i);

        result += wxString::Format(wxT("dock_size(%d,%d,%d)=%d|"),
                                   dock.dock_direction, dock.dock_layer,
                                   dock.dock_row, dock.size);
    }

    return result;
}


bool wxAuiManager::LoadPerspective(const wxString& layout, bool update)
{
    wxString input = layout;
    wxString part;

                part = input.BeforeFirst(wxT('|'));
    input = input.AfterFirst(wxT('|'));
    part.Trim(true);
    part.Trim(false);
    if (part != wxT("layout2"))
        return false;

        int pane_i, pane_count = m_panes.GetCount();
    for (pane_i = 0; pane_i < pane_count; ++pane_i)
    {
        wxAuiPaneInfo& p = m_panes.Item(pane_i);
        if(p.IsDockable())
            p.Dock();
        p.Hide();
    }

        m_docks.Clear();

            input.Replace(wxT("\\|"), wxT("\a"));
    input.Replace(wxT("\\;"), wxT("\b"));

    m_hasMaximized = false;
    while (1)
    {
        wxAuiPaneInfo pane;

        wxString pane_part = input.BeforeFirst(wxT('|'));
        input = input.AfterFirst(wxT('|'));
        pane_part.Trim(true);

                if (pane_part.empty())
            break;

        if (pane_part.Left(9) == wxT("dock_size"))
        {
            wxString val_name = pane_part.BeforeFirst(wxT('='));
            wxString value = pane_part.AfterFirst(wxT('='));

            long dir, layer, row, size;
            wxString piece = val_name.AfterFirst(wxT('('));
            piece = piece.BeforeLast(wxT(')'));
            piece.BeforeFirst(wxT(',')).ToLong(&dir);
            piece = piece.AfterFirst(wxT(','));
            piece.BeforeFirst(wxT(',')).ToLong(&layer);
            piece.AfterFirst(wxT(',')).ToLong(&row);
            value.ToLong(&size);

            wxAuiDockInfo dock;
            dock.dock_direction = dir;
            dock.dock_layer = layer;
            dock.dock_row = row;
            dock.size = size;
            m_docks.Add(dock);
            continue;
        }

                        pane_part.Replace(wxT("\a"), wxT("|"));
        pane_part.Replace(wxT("\b"), wxT(";"));

        LoadPaneInfo(pane_part, pane);

        if ( pane.IsMaximized() )
            m_hasMaximized = true;

        wxAuiPaneInfo& p = GetPane(pane.name);
        if (!p.IsOk())
        {
                                    continue;
        }

        p.SafeSet(pane);
    }

    if (update)
        Update();

    return true;
}

void wxAuiManager::GetPanePositionsAndSizes(wxAuiDockInfo& dock,
                                            wxArrayInt& positions,
                                            wxArrayInt& sizes)
{
    int caption_size = m_art->GetMetric(wxAUI_DOCKART_CAPTION_SIZE);
    int pane_borderSize = m_art->GetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE);
    int gripperSize = m_art->GetMetric(wxAUI_DOCKART_GRIPPER_SIZE);

    positions.Empty();
    sizes.Empty();

    int offset, action_pane = -1;
    int pane_i, pane_count = dock.panes.GetCount();

        for (pane_i = 0; pane_i < pane_count; ++pane_i)
    {
        wxAuiPaneInfo& pane = *(dock.panes.Item(pane_i));

        if (pane.HasFlag(wxAuiPaneInfo::actionPane))
        {
            wxASSERT_MSG(action_pane==-1, wxT("Too many fixed action panes"));
            action_pane = pane_i;
        }
    }

                for (pane_i = 0; pane_i < pane_count; ++pane_i)
    {
        wxAuiPaneInfo& pane = *(dock.panes.Item(pane_i));
        positions.Add(pane.dock_pos);
        int size = 0;

        if (pane.HasBorder())
            size += (pane_borderSize*2);

        if (dock.IsHorizontal())
        {
            if (pane.HasGripper() && !pane.HasGripperTop())
                size += gripperSize;
            size += pane.best_size.x;
        }
        else
        {
            if (pane.HasGripper() && pane.HasGripperTop())
                size += gripperSize;

            if (pane.HasCaption())
                size += caption_size;
            size += pane.best_size.y;
        }

        sizes.Add(size);
    }

            if (action_pane == -1)
        return;

    offset = 0;
    for (pane_i = action_pane-1; pane_i >= 0; --pane_i)
    {
        int amount = positions[pane_i+1] - (positions[pane_i] + sizes[pane_i]);

        if (amount >= 0)
            offset += amount;
        else
            positions[pane_i] -= -amount;

        offset += sizes[pane_i];
    }

            offset = 0;
    for (pane_i = action_pane; pane_i < pane_count; ++pane_i)
    {
        int amount = positions[pane_i] - offset;
        if (amount >= 0)
            offset += amount;
        else
            positions[pane_i] += -amount;

        offset += sizes[pane_i];
    }
}


void wxAuiManager::LayoutAddPane(wxSizer* cont,
                                 wxAuiDockInfo& dock,
                                 wxAuiPaneInfo& pane,
                                 wxAuiDockUIPartArray& uiparts,
                                 bool spacer_only)
{
    wxAuiDockUIPart part;
    wxSizerItem* sizer_item;

    int caption_size = m_art->GetMetric(wxAUI_DOCKART_CAPTION_SIZE);
    int gripperSize = m_art->GetMetric(wxAUI_DOCKART_GRIPPER_SIZE);
    int pane_borderSize = m_art->GetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE);
    int pane_button_size = m_art->GetMetric(wxAUI_DOCKART_PANE_BUTTON_SIZE);

            int orientation;
    if (dock.IsHorizontal())
        orientation = wxHORIZONTAL;
    else
        orientation = wxVERTICAL;

            int pane_proportion = pane.dock_proportion;

    wxBoxSizer* horz_pane_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* vert_pane_sizer = new wxBoxSizer(wxVERTICAL);

    if (pane.HasGripper())
    {
        if (pane.HasGripperTop())
            sizer_item = vert_pane_sizer ->Add(1, gripperSize, 0, wxEXPAND);
        else
            sizer_item = horz_pane_sizer ->Add(gripperSize, 1, 0, wxEXPAND);

        part.type = wxAuiDockUIPart::typeGripper;
        part.dock = &dock;
        part.pane = &pane;
        part.button = NULL;
        part.orientation = orientation;
        part.cont_sizer = horz_pane_sizer;
        part.sizer_item = sizer_item;
        uiparts.Add(part);
    }

    if (pane.HasCaption())
    {
                wxBoxSizer* caption_sizer = new wxBoxSizer(wxHORIZONTAL);

        sizer_item = caption_sizer->Add(1, caption_size, 1, wxEXPAND);

        part.type = wxAuiDockUIPart::typeCaption;
        part.dock = &dock;
        part.pane = &pane;
        part.button = NULL;
        part.orientation = orientation;
        part.cont_sizer = vert_pane_sizer;
        part.sizer_item = sizer_item;
        int caption_part_idx = uiparts.GetCount();
        uiparts.Add(part);

                int i, button_count;
        for (i = 0, button_count = pane.buttons.GetCount();
             i < button_count; ++i)
        {
            wxAuiPaneButton& button = pane.buttons.Item(i);

            sizer_item = caption_sizer->Add(pane_button_size,
                                            caption_size,
                                            0, wxEXPAND);

            part.type = wxAuiDockUIPart::typePaneButton;
            part.dock = &dock;
            part.pane = &pane;
            part.button = &button;
            part.orientation = orientation;
            part.cont_sizer = caption_sizer;
            part.sizer_item = sizer_item;
            uiparts.Add(part);
        }

                        if (button_count >= 1)
        {
            caption_sizer->Add(3,1);
        }

                sizer_item = vert_pane_sizer->Add(caption_sizer, 0, wxEXPAND);

        uiparts.Item(caption_part_idx).sizer_item = sizer_item;
    }

        if (spacer_only)
    {
        sizer_item = vert_pane_sizer->Add(1, 1, 1, wxEXPAND);
    }
    else
    {
        sizer_item = vert_pane_sizer->Add(pane.window, 1, wxEXPAND);
                                vert_pane_sizer->SetItemMinSize(pane.window, 1, 1);
    }

    part.type = wxAuiDockUIPart::typePane;
    part.dock = &dock;
    part.pane = &pane;
    part.button = NULL;
    part.orientation = orientation;
    part.cont_sizer = vert_pane_sizer;
    part.sizer_item = sizer_item;
    uiparts.Add(part);


            
    wxSize min_size = pane.min_size;
    if (pane.IsFixed())
    {
        if (min_size == wxDefaultSize)
        {
            min_size = pane.best_size;
            pane_proportion = 0;
        }
    }

    if (min_size != wxDefaultSize)
    {
        vert_pane_sizer->SetItemMinSize(
                        vert_pane_sizer->GetChildren().GetCount()-1,
                        min_size.x, min_size.y);
    }


            horz_pane_sizer->Add(vert_pane_sizer, 1, wxEXPAND);

    
    if (pane.HasBorder())
    {
                sizer_item = cont->Add(horz_pane_sizer, pane_proportion,
                               wxEXPAND | wxALL, pane_borderSize);

        part.type = wxAuiDockUIPart::typePaneBorder;
        part.dock = &dock;
        part.pane = &pane;
        part.button = NULL;
        part.orientation = orientation;
        part.cont_sizer = cont;
        part.sizer_item = sizer_item;
        uiparts.Add(part);
    }
    else
    {
        sizer_item = cont->Add(horz_pane_sizer, pane_proportion, wxEXPAND);
    }
}

void wxAuiManager::LayoutAddDock(wxSizer* cont,
                                 wxAuiDockInfo& dock,
                                 wxAuiDockUIPartArray& uiparts,
                                 bool spacer_only)
{
    wxSizerItem* sizer_item;
    wxAuiDockUIPart part;

    int sashSize = m_art->GetMetric(wxAUI_DOCKART_SASH_SIZE);
    int orientation = dock.IsHorizontal() ? wxHORIZONTAL : wxVERTICAL;

        if (!m_hasMaximized && !dock.fixed && (dock.dock_direction == wxAUI_DOCK_BOTTOM ||
                        dock.dock_direction == wxAUI_DOCK_RIGHT))
    {
        sizer_item = cont->Add(sashSize, sashSize, 0, wxEXPAND);

        part.type = wxAuiDockUIPart::typeDockSizer;
        part.orientation = orientation;
        part.dock = &dock;
        part.pane = NULL;
        part.button = NULL;
        part.cont_sizer = cont;
        part.sizer_item = sizer_item;
        uiparts.Add(part);
    }

        wxSizer* dock_sizer = new wxBoxSizer(orientation);

        bool has_maximized_pane = false;
    int pane_i, pane_count = dock.panes.GetCount();

    if (dock.fixed)
    {
        wxArrayInt pane_positions, pane_sizes;

                        GetPanePositionsAndSizes(dock, pane_positions, pane_sizes);

        int offset = 0;
        for (pane_i = 0; pane_i < pane_count; ++pane_i)
        {
            wxAuiPaneInfo& pane = *(dock.panes.Item(pane_i));
            int pane_pos = pane_positions.Item(pane_i);

            if (pane.IsMaximized())
                has_maximized_pane = true;


            int amount = pane_pos - offset;
            if (amount > 0)
            {
                if (dock.IsVertical())
                    sizer_item = dock_sizer->Add(1, amount, 0, wxEXPAND);
                else
                    sizer_item = dock_sizer->Add(amount, 1, 0, wxEXPAND);

                part.type = wxAuiDockUIPart::typeBackground;
                part.dock = &dock;
                part.pane = NULL;
                part.button = NULL;
                part.orientation = (orientation==wxHORIZONTAL) ? wxVERTICAL:wxHORIZONTAL;
                part.cont_sizer = dock_sizer;
                part.sizer_item = sizer_item;
                uiparts.Add(part);

                offset += amount;
            }

            LayoutAddPane(dock_sizer, dock, pane, uiparts, spacer_only);

            offset += pane_sizes.Item(pane_i);
        }

                sizer_item = dock_sizer->Add(0,0, 1, wxEXPAND);

        part.type = wxAuiDockUIPart::typeBackground;
        part.dock = &dock;
        part.pane = NULL;
        part.button = NULL;
        part.orientation = orientation;
        part.cont_sizer = dock_sizer;
        part.sizer_item = sizer_item;
        uiparts.Add(part);
    }
    else
    {
        for (pane_i = 0; pane_i < pane_count; ++pane_i)
        {
            wxAuiPaneInfo& pane = *(dock.panes.Item(pane_i));

            if (pane.IsMaximized())
                has_maximized_pane = true;

                                    if (!m_hasMaximized && pane_i > 0)
            {
                sizer_item = dock_sizer->Add(sashSize, sashSize, 0, wxEXPAND);

                part.type = wxAuiDockUIPart::typePaneSizer;
                part.dock = &dock;
                part.pane = dock.panes.Item(pane_i-1);
                part.button = NULL;
                part.orientation = (orientation==wxHORIZONTAL) ? wxVERTICAL:wxHORIZONTAL;
                part.cont_sizer = dock_sizer;
                part.sizer_item = sizer_item;
                uiparts.Add(part);
            }

            LayoutAddPane(dock_sizer, dock, pane, uiparts, spacer_only);
        }
    }

    if (dock.dock_direction == wxAUI_DOCK_CENTER || has_maximized_pane)
        sizer_item = cont->Add(dock_sizer, 1, wxEXPAND);
    else
        sizer_item = cont->Add(dock_sizer, 0, wxEXPAND);

    part.type = wxAuiDockUIPart::typeDock;
    part.dock = &dock;
    part.pane = NULL;
    part.button = NULL;
    part.orientation = orientation;
    part.cont_sizer = cont;
    part.sizer_item = sizer_item;
    uiparts.Add(part);

    if (dock.IsHorizontal())
        cont->SetItemMinSize(dock_sizer, 0, dock.size);
    else
        cont->SetItemMinSize(dock_sizer, dock.size, 0);

        if (!m_hasMaximized &&
        !dock.fixed &&
          (dock.dock_direction == wxAUI_DOCK_TOP ||
           dock.dock_direction == wxAUI_DOCK_LEFT))
    {
        sizer_item = cont->Add(sashSize, sashSize, 0, wxEXPAND);

        part.type = wxAuiDockUIPart::typeDockSizer;
        part.dock = &dock;
        part.pane = NULL;
        part.button = NULL;
        part.orientation = orientation;
        part.cont_sizer = cont;
        part.sizer_item = sizer_item;
        uiparts.Add(part);
    }
}

wxSizer* wxAuiManager::LayoutAll(wxAuiPaneInfoArray& panes,
                                 wxAuiDockInfoArray& docks,
                                 wxAuiDockUIPartArray& uiparts,
                                 bool spacer_only)
{
    wxBoxSizer* container = new wxBoxSizer(wxVERTICAL);

    int pane_borderSize = m_art->GetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE);
    int caption_size = m_art->GetMetric(wxAUI_DOCKART_CAPTION_SIZE);
    wxSize cli_size = m_frame->GetClientSize();
    int i, dock_count, pane_count;


        for (i = 0, dock_count = docks.GetCount(); i < dock_count; ++i)
    {
        wxAuiDockInfo& dock = docks.Item(i);

                dock.panes.Empty();

        if (dock.fixed)
        {
                                    dock.size = 0;
        }
    }


                for (i = 0, pane_count = panes.GetCount(); i < pane_count; ++i)
    {
        wxAuiPaneInfo& p = panes.Item(i);

                        wxAuiDockInfo* dock;
        wxAuiDockInfoPtrArray arr;
        FindDocks(docks, p.dock_direction, p.dock_layer, p.dock_row, arr);

        if (arr.GetCount() > 0)
        {
                        dock = arr.Item(0);
        }
        else
        {
                        wxAuiDockInfo d;
            d.dock_direction = p.dock_direction;
            d.dock_layer = p.dock_layer;
            d.dock_row = p.dock_row;
            docks.Add(d);
            dock = &docks.Last();
        }


        if (p.IsDocked() && p.IsShown())
        {
                        RemovePaneFromDocks(docks, p, dock);

                                    if (!FindPaneInDock(*dock, p.window))
                dock->panes.Add(&p);
        }
        else
        {
                        RemovePaneFromDocks(docks, p);
        }

    }

        for (i = docks.GetCount()-1; i >= 0; --i)
    {
        if (docks.Item(i).panes.GetCount() == 0)
            docks.RemoveAt(i);
    }

        for (i = 0, dock_count = docks.GetCount(); i < dock_count; ++i)
    {
        wxAuiDockInfo& dock = docks.Item(i);
        int j, dock_pane_count = dock.panes.GetCount();

                        dock.panes.Sort(PaneSortFunc);

                if (dock.size == 0)
        {
            int size = 0;

            for (j = 0; j < dock_pane_count; ++j)
            {
                wxAuiPaneInfo& pane = *dock.panes.Item(j);
                wxSize pane_size = pane.best_size;
                if (pane_size == wxDefaultSize)
                    pane_size = pane.min_size;
                if (pane_size == wxDefaultSize)
                    pane_size = pane.window->GetSize();

                if (dock.IsHorizontal())
                    size = wxMax(pane_size.y, size);
                else
                    size = wxMax(pane_size.x, size);
            }

                                    for (j = 0; j < dock_pane_count; ++j)
            {
                if (dock.panes.Item(j)->HasBorder())
                {
                    size += (pane_borderSize*2);
                    break;
                }
            }

                                    if (dock.IsHorizontal())
            {
                for (j = 0; j < dock_pane_count; ++j)
                {
                    if (dock.panes.Item(j)->HasCaption())
                    {
                        size += caption_size;
                        break;
                    }
                }
            }


                        
            int max_dock_x_size = (int)(m_dockConstraintX * ((double)cli_size.x));
            int max_dock_y_size = (int)(m_dockConstraintY * ((double)cli_size.y));

            if (dock.IsHorizontal())
                size = wxMin(size, max_dock_y_size);
            else
                size = wxMin(size, max_dock_x_size);

                        if (size < 10)
                size = 10;

            dock.size = size;
        }


                bool plus_border = false;
        bool plus_caption = false;
        int dock_min_size = 0;
        for (j = 0; j < dock_pane_count; ++j)
        {
            wxAuiPaneInfo& pane = *dock.panes.Item(j);
            if (pane.min_size != wxDefaultSize)
            {
                if (pane.HasBorder())
                    plus_border = true;
                if (pane.HasCaption())
                    plus_caption = true;
                if (dock.IsHorizontal())
                {
                    if (pane.min_size.y > dock_min_size)
                        dock_min_size = pane.min_size.y;
                }
                else
                {
                    if (pane.min_size.x > dock_min_size)
                        dock_min_size = pane.min_size.x;
                }
            }
        }

        if (plus_border)
            dock_min_size += (pane_borderSize*2);
        if (plus_caption && dock.IsHorizontal())
            dock_min_size += (caption_size);

        dock.min_size = dock_min_size;


                        if (dock.size < dock.min_size)
            dock.size = dock.min_size;


                        bool action_pane_marked = false;
        dock.fixed = true;
        dock.toolbar = true;
        for (j = 0; j < dock_pane_count; ++j)
        {
            wxAuiPaneInfo& pane = *dock.panes.Item(j);
            if (!pane.IsFixed())
                dock.fixed = false;
            if (!pane.IsToolbar())
                dock.toolbar = false;
            if (pane.HasFlag(wxAuiPaneInfo::optionDockFixed))
                dock.fixed = true;
            if (pane.HasFlag(wxAuiPaneInfo::actionPane))
                action_pane_marked = true;
        }


                                if (!dock.fixed)
        {
            for (j = 0; j < dock_pane_count; ++j)
            {
                wxAuiPaneInfo& pane = *dock.panes.Item(j);
                pane.dock_pos = j;
            }
        }

                                        if (dock.fixed && !action_pane_marked)
        {
            wxArrayInt pane_positions, pane_sizes;
            GetPanePositionsAndSizes(dock, pane_positions, pane_sizes);

            int offset = 0;
            for (j = 0; j < dock_pane_count; ++j)
            {
                wxAuiPaneInfo& pane = *(dock.panes.Item(j));
                pane.dock_pos = pane_positions[j];

                int amount = pane.dock_pos - offset;
                if (amount >= 0)
                    offset += amount;
                else
                    pane.dock_pos += -amount;

                offset += pane_sizes[j];
            }
        }
    }

        int max_layer = 0;
    for (i = 0; i < dock_count; ++i)
        max_layer = wxMax(max_layer, docks.Item(i).dock_layer);


        uiparts.Empty();

            wxSizer* cont = NULL;
    wxSizer* middle = NULL;
    int layer = 0;
    int row, row_count;

    for (layer = 0; layer <= max_layer; ++layer)
    {
        wxAuiDockInfoPtrArray arr;

                FindDocks(docks, -1, layer, -1, arr);

                if (arr.IsEmpty())
            continue;

        wxSizer* old_cont = cont;

                        cont = new wxBoxSizer(wxVERTICAL);


                FindDocks(docks, wxAUI_DOCK_TOP, layer, -1, arr);
        if (!arr.IsEmpty())
        {
            for (row = 0, row_count = arr.GetCount(); row < row_count; ++row)
                LayoutAddDock(cont, *arr.Item(row), uiparts, spacer_only);
        }


                
        middle = new wxBoxSizer(wxHORIZONTAL);

                FindDocks(docks, wxAUI_DOCK_LEFT, layer, -1, arr);
        if (!arr.IsEmpty())
        {
            for (row = 0, row_count = arr.GetCount(); row < row_count; ++row)
                LayoutAddDock(middle, *arr.Item(row), uiparts, spacer_only);
        }

                        if (!old_cont)
        {
                        FindDocks(docks, wxAUI_DOCK_CENTER, -1, -1, arr);
            if (!arr.IsEmpty())
            {
                for (row = 0,row_count = arr.GetCount(); row<row_count; ++row)
                   LayoutAddDock(middle, *arr.Item(row), uiparts, spacer_only);
            }
            else if (!m_hasMaximized)
            {
                                wxSizerItem* sizer_item = middle->Add(1,1, 1, wxEXPAND);
                wxAuiDockUIPart part;
                part.type = wxAuiDockUIPart::typeBackground;
                part.pane = NULL;
                part.dock = NULL;
                part.button = NULL;
                part.cont_sizer = middle;
                part.sizer_item = sizer_item;
                uiparts.Add(part);
            }
        }
        else
        {
            middle->Add(old_cont, 1, wxEXPAND);
        }

                FindDocks(docks, wxAUI_DOCK_RIGHT, layer, -1, arr);
        if (!arr.IsEmpty())
        {
            for (row = arr.GetCount()-1; row >= 0; --row)
                LayoutAddDock(middle, *arr.Item(row), uiparts, spacer_only);
        }

        if (middle->GetChildren().GetCount() > 0)
            cont->Add(middle, 1, wxEXPAND);
             else
            delete middle;



                FindDocks(docks, wxAUI_DOCK_BOTTOM, layer, -1, arr);
        if (!arr.IsEmpty())
        {
            for (row = arr.GetCount()-1; row >= 0; --row)
                LayoutAddDock(cont, *arr.Item(row), uiparts, spacer_only);
        }

    }

    if (!cont)
    {
                        cont = new wxBoxSizer(wxVERTICAL);
        wxSizerItem* sizer_item = cont->Add(1,1, 1, wxEXPAND);
        wxAuiDockUIPart part;
        part.type = wxAuiDockUIPart::typeBackground;
        part.pane = NULL;
        part.dock = NULL;
        part.button = NULL;
        part.cont_sizer = middle;
        part.sizer_item = sizer_item;
        uiparts.Add(part);
    }

    container->Add(cont, 1, wxEXPAND);
    return container;
}



void wxAuiManager::SetDockSizeConstraint(double width_pct, double height_pct)
{
    m_dockConstraintX = wxMax(0.0, wxMin(1.0, width_pct));
    m_dockConstraintY = wxMax(0.0, wxMin(1.0, height_pct));
}

void wxAuiManager::GetDockSizeConstraint(double* width_pct, double* height_pct) const
{
    if (width_pct)
        *width_pct = m_dockConstraintX;

    if (height_pct)
        *height_pct = m_dockConstraintY;
}




void wxAuiManager::Update()
{
    m_hoverButton = NULL;
    m_actionPart = NULL;

    wxSizer* sizer;
    int i, pane_count = m_panes.GetCount();


            for (i = 0; i < pane_count; ++i)
    {
        wxAuiPaneInfo& p = m_panes.Item(i);

        if (!p.IsFloating() && p.frame)
        {
                        
                        p.window->SetSize(1,1);


                                                                                    if (m_actionWindow == p.frame)
            {
                if (wxWindow::GetCapture() == m_frame)
                    m_frame->ReleaseMouse();
                m_action = actionNone;
                m_actionWindow = NULL;
            }

                        if (p.frame->IsShown())
                p.frame->Show(false);

                        if (m_actionWindow == p.frame)
            {
                m_actionWindow = NULL;
            }

            p.window->Reparent(m_frame);
            p.frame->SetSizer(NULL);
            p.frame->Destroy();
            p.frame = NULL;
        }
    }


        m_frame->SetSizer(NULL);

        sizer = LayoutAll(m_panes, m_docks, m_uiParts, false);

            for (i = 0; i < pane_count; ++i)
    {
        wxAuiPaneInfo& p = m_panes.Item(i);

        if (p.IsFloating())
        {
            if (p.frame == NULL)
            {
                                                wxAuiFloatingFrame* frame = CreateFloatingFrame(m_frame, p);

                                                                if (m_action == actionDragFloatingPane &&
                    (m_flags & wxAUI_MGR_TRANSPARENT_DRAG))
                        frame->SetTransparent(150);

                frame->SetPaneWindow(p);
                p.frame = frame;

                if (p.IsShown() && !frame->IsShown())
                    frame->Show();
            }
            else
            {
                                                if ((p.frame->GetPosition() != p.floating_pos) || (p.frame->GetSize() != p.floating_size))
                {
                    p.frame->SetSize(p.floating_pos.x, p.floating_pos.y,
                                     p.floating_size.x, p.floating_size.y,
                                     wxSIZE_USE_EXISTING);
                
                }

                                long style = p.frame->GetWindowStyleFlag();
                if (p.IsFixed())
                    style &= ~wxRESIZE_BORDER;
                else
                    style |= wxRESIZE_BORDER;
                p.frame->SetWindowStyleFlag(style);

                if (p.frame->GetLabel() != p.caption)
                    p.frame->SetLabel(p.caption);

                if (p.frame->IsShown() != p.IsShown())
                    p.frame->Show(p.IsShown());
            }
        }
        else
        {
            if (p.window->IsShown() != p.IsShown())
                p.window->Show(p.IsShown());
        }

                        if ((m_flags & wxAUI_MGR_ALLOW_ACTIVE_PANE) == 0)
        {
            p.state &= ~wxAuiPaneInfo::optionActive;
        }
    }


            wxAuiRectArray old_pane_rects;
    for (i = 0; i < pane_count; ++i)
    {
        wxRect r;
        wxAuiPaneInfo& p = m_panes.Item(i);

        if (p.window && p.IsShown() && p.IsDocked())
            r = p.rect;

        old_pane_rects.Add(r);
    }




        m_frame->SetSizer(sizer);
    m_frame->SetAutoLayout(false);
    DoFrameLayout();



                    for (i = 0; i < pane_count; ++i)
    {
        wxAuiPaneInfo& p = m_panes.Item(i);
        if (p.window && p.window->IsShown() && p.IsDocked())
        {
            if (p.rect != old_pane_rects[i])
            {
                p.window->Refresh();
                p.window->Update();
            }
        }
    }


    Repaint();

    

}



void wxAuiManager::DoFrameLayout()
{
    m_frame->Layout();

    int i, part_count;
    for (i = 0, part_count = m_uiParts.GetCount(); i < part_count; ++i)
    {
        wxAuiDockUIPart& part = m_uiParts.Item(i);

                                                                                

        part.rect = part.sizer_item->GetRect();
        int flag = part.sizer_item->GetFlag();
        int border = part.sizer_item->GetBorder();
        if (flag & wxTOP)
        {
            part.rect.y -= border;
            part.rect.height += border;
        }
        if (flag & wxLEFT)
        {
            part.rect.x -= border;
            part.rect.width += border;
        }
        if (flag & wxBOTTOM)
            part.rect.height += border;
        if (flag & wxRIGHT)
            part.rect.width += border;


        if (part.type == wxAuiDockUIPart::typeDock)
            part.dock->rect = part.rect;
        if (part.type == wxAuiDockUIPart::typePane)
            part.pane->rect = part.rect;
    }
}


wxAuiDockUIPart* wxAuiManager::GetPanePart(wxWindow* wnd)
{
    int i, part_count;
    for (i = 0, part_count = m_uiParts.GetCount(); i < part_count; ++i)
    {
        wxAuiDockUIPart& part = m_uiParts.Item(i);
        if (part.type == wxAuiDockUIPart::typePaneBorder &&
            part.pane && part.pane->window == wnd)
                return &part;
    }
    for (i = 0, part_count = m_uiParts.GetCount(); i < part_count; ++i)
    {
        wxAuiDockUIPart& part = m_uiParts.Item(i);
        if (part.type == wxAuiDockUIPart::typePane &&
            part.pane && part.pane->window == wnd)
                return &part;
    }
    return NULL;
}




int wxAuiManager::GetDockPixelOffset(wxAuiPaneInfo& test)
{
        
    int i, part_count, dock_count;
    wxAuiDockInfoArray docks;
    wxAuiPaneInfoArray panes;
    wxAuiDockUIPartArray uiparts;
    CopyDocksAndPanes(docks, panes, m_docks, m_panes);
    panes.Add(test);

    wxSizer* sizer = LayoutAll(panes, docks, uiparts, true);
    wxSize client_size = m_frame->GetClientSize();
    sizer->SetDimension(0, 0, client_size.x, client_size.y);
    sizer->Layout();

    for (i = 0, part_count = uiparts.GetCount(); i < part_count; ++i)
    {
        wxAuiDockUIPart& part = uiparts.Item(i);
        part.rect = wxRect(part.sizer_item->GetPosition(),
                           part.sizer_item->GetSize());
        if (part.type == wxAuiDockUIPart::typeDock)
            part.dock->rect = part.rect;
    }

    delete sizer;

    for (i = 0, dock_count = docks.GetCount(); i < dock_count; ++i)
    {
        wxAuiDockInfo& dock = docks.Item(i);
        if (test.dock_direction == dock.dock_direction &&
            test.dock_layer==dock.dock_layer && test.dock_row==dock.dock_row)
        {
            if (dock.IsVertical())
                return dock.rect.y;
            else
                return dock.rect.x;
        }
    }

    return 0;
}




bool wxAuiManager::ProcessDockResult(wxAuiPaneInfo& target,
                                     const wxAuiPaneInfo& new_pos)
{
    bool allowed = false;
    switch (new_pos.dock_direction)
    {
        case wxAUI_DOCK_TOP:    allowed = target.IsTopDockable();    break;
        case wxAUI_DOCK_BOTTOM: allowed = target.IsBottomDockable(); break;
        case wxAUI_DOCK_LEFT:   allowed = target.IsLeftDockable();   break;
        case wxAUI_DOCK_RIGHT:  allowed = target.IsRightDockable();  break;
    }

    if (allowed)
    {
        target = new_pos;
                                wxAuiToolBar* toolbar = wxDynamicCast(target.window, wxAuiToolBar);
        if (toolbar)
        {
            wxSize hintSize = toolbar->GetHintSize(target.dock_direction);
            if (target.best_size != hintSize)
            {
                target.best_size = hintSize;
                target.floating_size = wxDefaultSize;
            }
        }
    }

    return allowed;
}



const int auiInsertRowPixels = 10;
const int auiNewRowPixels = 40;
const int auiLayerInsertPixels = 40;
const int auiLayerInsertOffset = 5;

bool wxAuiManager::DoDrop(wxAuiDockInfoArray& docks,
                          wxAuiPaneInfoArray& panes,
                          wxAuiPaneInfo& target,
                          const wxPoint& pt,
                          const wxPoint& offset)
{
    wxSize cli_size = m_frame->GetClientSize();

    wxAuiPaneInfo drop = target;


        drop.Show();


        

    int layer_insert_offset = auiLayerInsertOffset;
    if (drop.IsToolbar())
        layer_insert_offset = 0;


    if (pt.x < layer_insert_offset &&
        pt.x > layer_insert_offset-auiLayerInsertPixels &&
        pt.y > 0 &&
        pt.y < cli_size.y)
    {
        int new_layer = wxMax(wxMax(GetMaxLayer(docks, wxAUI_DOCK_LEFT),
                                GetMaxLayer(docks, wxAUI_DOCK_BOTTOM)),
                                GetMaxLayer(docks, wxAUI_DOCK_TOP)) + 1;

        if (drop.IsToolbar())
            new_layer = auiToolBarLayer;

        drop.Dock().Left().
             Layer(new_layer).
             Row(0).
             Position(pt.y - GetDockPixelOffset(drop) - offset.y);
        return ProcessDockResult(target, drop);
    }
    else if (pt.y < layer_insert_offset &&
             pt.y > layer_insert_offset-auiLayerInsertPixels &&
             pt.x > 0 &&
             pt.x < cli_size.x)
    {
        int new_layer = wxMax(wxMax(GetMaxLayer(docks, wxAUI_DOCK_TOP),
                                GetMaxLayer(docks, wxAUI_DOCK_LEFT)),
                                GetMaxLayer(docks, wxAUI_DOCK_RIGHT)) + 1;

        if (drop.IsToolbar())
            new_layer = auiToolBarLayer;

        drop.Dock().Top().
             Layer(new_layer).
             Row(0).
             Position(pt.x - GetDockPixelOffset(drop) - offset.x);
        return ProcessDockResult(target, drop);
    }
    else if (pt.x >= cli_size.x - layer_insert_offset &&
             pt.x < cli_size.x - layer_insert_offset + auiLayerInsertPixels &&
             pt.y > 0 &&
             pt.y < cli_size.y)
    {
        int new_layer = wxMax(wxMax(GetMaxLayer(docks, wxAUI_DOCK_RIGHT),
                                GetMaxLayer(docks, wxAUI_DOCK_TOP)),
                                GetMaxLayer(docks, wxAUI_DOCK_BOTTOM)) + 1;

        if (drop.IsToolbar())
            new_layer = auiToolBarLayer;

        drop.Dock().Right().
             Layer(new_layer).
             Row(0).
             Position(pt.y - GetDockPixelOffset(drop) - offset.y);
        return ProcessDockResult(target, drop);
    }
    else if (pt.y >= cli_size.y - layer_insert_offset &&
             pt.y < cli_size.y - layer_insert_offset + auiLayerInsertPixels &&
             pt.x > 0 &&
             pt.x < cli_size.x)
    {
        int new_layer = wxMax( wxMax( GetMaxLayer(docks, wxAUI_DOCK_BOTTOM),
                                      GetMaxLayer(docks, wxAUI_DOCK_LEFT)),
                                      GetMaxLayer(docks, wxAUI_DOCK_RIGHT)) + 1;

        if (drop.IsToolbar())
            new_layer = auiToolBarLayer;

        drop.Dock().Bottom().
             Layer(new_layer).
             Row(0).
             Position(pt.x - GetDockPixelOffset(drop) - offset.x);
        return ProcessDockResult(target, drop);
    }


    wxAuiDockUIPart* part = HitTest(pt.x, pt.y);


    if (drop.IsToolbar())
    {
        if (!part || !part->dock)
            return false;

                        int dock_drop_offset = 0;
        if (part->dock->IsHorizontal())
            dock_drop_offset = pt.x - part->dock->rect.x - offset.x;
        else
            dock_drop_offset = pt.y - part->dock->rect.y - offset.y;


                                if (!part->dock->fixed || part->dock->dock_direction == wxAUI_DOCK_CENTER ||
            pt.x >= cli_size.x || pt.x <= 0 || pt.y >= cli_size.y || pt.y <= 0)
        {
            if (m_lastRect.IsEmpty() || m_lastRect.Contains(pt.x, pt.y ))
            {
                m_skipping = true;
            }
            else
            {
                if ((m_flags & wxAUI_MGR_ALLOW_FLOATING) && drop.IsFloatable())
                {
                    drop.Float();
                }

                m_skipping = false;

                return ProcessDockResult(target, drop);
            }

            drop.Position(pt.x - GetDockPixelOffset(drop) - offset.x);

            return ProcessDockResult(target, drop);
        }

        m_skipping = false;

        m_lastRect = part->dock->rect;
        m_lastRect.Inflate( 15, 15 );

        drop.Dock().
             Direction(part->dock->dock_direction).
             Layer(part->dock->dock_layer).
             Row(part->dock->dock_row).
             Position(dock_drop_offset);

        if ((
            ((pt.y < part->dock->rect.y + 1) && part->dock->IsHorizontal()) ||
            ((pt.x < part->dock->rect.x + 1) && part->dock->IsVertical())
            ) && part->dock->panes.GetCount() > 1)
        {
            if ((part->dock->dock_direction == wxAUI_DOCK_TOP) ||
                (part->dock->dock_direction == wxAUI_DOCK_LEFT))
            {
                int row = drop.dock_row;
                DoInsertDockRow(panes, part->dock->dock_direction,
                                part->dock->dock_layer,
                                part->dock->dock_row);
                drop.dock_row = row;
            }
            else
            {
                DoInsertDockRow(panes, part->dock->dock_direction,
                                part->dock->dock_layer,
                                part->dock->dock_row+1);
                drop.dock_row = part->dock->dock_row+1;
            }
        }

        if ((
            ((pt.y > part->dock->rect.y + part->dock->rect.height - 2 ) && part->dock->IsHorizontal()) ||
            ((pt.x > part->dock->rect.x + part->dock->rect.width - 2 ) && part->dock->IsVertical())
            ) && part->dock->panes.GetCount() > 1)
        {
            if ((part->dock->dock_direction == wxAUI_DOCK_TOP) ||
                (part->dock->dock_direction == wxAUI_DOCK_LEFT))
            {
                DoInsertDockRow(panes, part->dock->dock_direction,
                                part->dock->dock_layer,
                                part->dock->dock_row+1);
                drop.dock_row = part->dock->dock_row+1;
            }
            else
            {
                int row = drop.dock_row;
                DoInsertDockRow(panes, part->dock->dock_direction,
                                part->dock->dock_layer,
                                part->dock->dock_row);
                drop.dock_row = row;
            }
        }

        return ProcessDockResult(target, drop);
    }




    if (!part)
        return false;

    if (part->type == wxAuiDockUIPart::typePaneBorder ||
        part->type == wxAuiDockUIPart::typeCaption ||
        part->type == wxAuiDockUIPart::typeGripper ||
        part->type == wxAuiDockUIPart::typePaneButton ||
        part->type == wxAuiDockUIPart::typePane ||
        part->type == wxAuiDockUIPart::typePaneSizer ||
        part->type == wxAuiDockUIPart::typeDockSizer ||
        part->type == wxAuiDockUIPart::typeBackground)
    {
        if (part->type == wxAuiDockUIPart::typeDockSizer)
        {
            if (part->dock->panes.GetCount() != 1)
                return false;
            part = GetPanePart(part->dock->panes.Item(0)->window);
            if (!part)
                return false;
        }



                                        if (part->dock && part->dock->toolbar)
        {
            int layer = 0;

            switch (part->dock->dock_direction)
            {
                case wxAUI_DOCK_LEFT:
                    layer = wxMax(wxMax(GetMaxLayer(docks, wxAUI_DOCK_LEFT),
                                      GetMaxLayer(docks, wxAUI_DOCK_BOTTOM)),
                                      GetMaxLayer(docks, wxAUI_DOCK_TOP));
                    break;
                case wxAUI_DOCK_TOP:
                    layer = wxMax(wxMax(GetMaxLayer(docks, wxAUI_DOCK_TOP),
                                      GetMaxLayer(docks, wxAUI_DOCK_LEFT)),
                                      GetMaxLayer(docks, wxAUI_DOCK_RIGHT));
                    break;
                case wxAUI_DOCK_RIGHT:
                    layer = wxMax(wxMax(GetMaxLayer(docks, wxAUI_DOCK_RIGHT),
                                      GetMaxLayer(docks, wxAUI_DOCK_TOP)),
                                      GetMaxLayer(docks, wxAUI_DOCK_BOTTOM));
                    break;
                case wxAUI_DOCK_BOTTOM:
                    layer = wxMax(wxMax(GetMaxLayer(docks, wxAUI_DOCK_BOTTOM),
                                      GetMaxLayer(docks, wxAUI_DOCK_LEFT)),
                                      GetMaxLayer(docks, wxAUI_DOCK_RIGHT));
                    break;
            }

            DoInsertDockRow(panes, part->dock->dock_direction,
                            layer, 0);
            drop.Dock().
                 Direction(part->dock->dock_direction).
                 Layer(layer).Row(0).Position(0);
            return ProcessDockResult(target, drop);
        }


        if (!part->pane)
            return false;

        part = GetPanePart(part->pane->window);
        if (!part)
            return false;

        bool insert_dock_row = false;
        int insert_row = part->pane->dock_row;
        int insert_dir = part->pane->dock_direction;
        int insert_layer = part->pane->dock_layer;

        switch (part->pane->dock_direction)
        {
            case wxAUI_DOCK_TOP:
                if (pt.y >= part->rect.y &&
                    pt.y < part->rect.y+auiInsertRowPixels)
                        insert_dock_row = true;
                break;
            case wxAUI_DOCK_BOTTOM:
                if (pt.y > part->rect.y+part->rect.height-auiInsertRowPixels &&
                    pt.y <= part->rect.y + part->rect.height)
                        insert_dock_row = true;
                break;
            case wxAUI_DOCK_LEFT:
                if (pt.x >= part->rect.x &&
                    pt.x < part->rect.x+auiInsertRowPixels)
                        insert_dock_row = true;
                break;
            case wxAUI_DOCK_RIGHT:
                if (pt.x > part->rect.x+part->rect.width-auiInsertRowPixels &&
                    pt.x <= part->rect.x+part->rect.width)
                        insert_dock_row = true;
                break;
            case wxAUI_DOCK_CENTER:
            {
                                                int new_row_pixels_x = auiNewRowPixels;
                int new_row_pixels_y = auiNewRowPixels;

                if (new_row_pixels_x > (part->rect.width*20)/100)
                    new_row_pixels_x = (part->rect.width*20)/100;

                if (new_row_pixels_y > (part->rect.height*20)/100)
                    new_row_pixels_y = (part->rect.height*20)/100;


                                                
                insert_layer = 0;
                insert_dock_row = true;
                const wxRect& pr = part->rect;
                if (pt.x >= pr.x && pt.x < pr.x + new_row_pixels_x)
                    insert_dir = wxAUI_DOCK_LEFT;
                else if (pt.y >= pr.y && pt.y < pr.y + new_row_pixels_y)
                    insert_dir = wxAUI_DOCK_TOP;
                else if (pt.x >= pr.x + pr.width - new_row_pixels_x &&
                         pt.x < pr.x + pr.width)
                    insert_dir = wxAUI_DOCK_RIGHT;
                else if (pt.y >= pr.y+ pr.height - new_row_pixels_y &&
                         pt.y < pr.y + pr.height)
                    insert_dir = wxAUI_DOCK_BOTTOM;
                else
                    return false;

                insert_row = GetMaxRow(panes, insert_dir, insert_layer) + 1;
            }
        }

        if (insert_dock_row)
        {
            DoInsertDockRow(panes, insert_dir, insert_layer, insert_row);
            drop.Dock().Direction(insert_dir).
                        Layer(insert_layer).
                        Row(insert_row).
                        Position(0);
            return ProcessDockResult(target, drop);
        }

                
        int mouseOffset, size;

        if (part->orientation == wxVERTICAL)
        {
            mouseOffset = pt.y - part->rect.y;
            size = part->rect.GetHeight();
        }
        else
        {
            mouseOffset = pt.x - part->rect.x;
            size = part->rect.GetWidth();
        }

        int drop_position = part->pane->dock_pos;

                        if (mouseOffset <= size/2)
        {
            drop_position = part->pane->dock_pos;
            DoInsertPane(panes,
                         part->pane->dock_direction,
                         part->pane->dock_layer,
                         part->pane->dock_row,
                         part->pane->dock_pos);
        }

                        if (mouseOffset > size/2)
        {
            drop_position = part->pane->dock_pos+1;
            DoInsertPane(panes,
                         part->pane->dock_direction,
                         part->pane->dock_layer,
                         part->pane->dock_row,
                         part->pane->dock_pos+1);
        }

        drop.Dock().
             Direction(part->dock->dock_direction).
             Layer(part->dock->dock_layer).
             Row(part->dock->dock_row).
             Position(drop_position);
        return ProcessDockResult(target, drop);
    }

    return false;
}


void wxAuiManager::OnHintFadeTimer(wxTimerEvent& WXUNUSED(event))
{
    if (!m_hintWnd || m_hintFadeAmt >= m_hintFadeMax)
    {
        m_hintFadeTimer.Stop();
        Disconnect(m_hintFadeTimer.GetId(), wxEVT_TIMER,
                   wxTimerEventHandler(wxAuiManager::OnHintFadeTimer));
        return;
    }

    m_hintFadeAmt += 4;
    m_hintWnd->SetTransparent(m_hintFadeAmt);
}

void wxAuiManager::ShowHint(const wxRect& rect)
{
    if (m_hintWnd)
    {
                if (m_lastHint == rect)
            return;
        m_lastHint = rect;

        m_hintFadeAmt = m_hintFadeMax;

        if ((m_flags & wxAUI_MGR_HINT_FADE)
            && !((wxDynamicCast(m_hintWnd, wxPseudoTransparentFrame)) &&
                 (m_flags & wxAUI_MGR_NO_VENETIAN_BLINDS_FADE))
            )
            m_hintFadeAmt = 0;

        m_hintWnd->SetSize(rect);
        m_hintWnd->SetTransparent(m_hintFadeAmt);

        if (!m_hintWnd->IsShown())
            m_hintWnd->Show();

                        if (m_action == actionDragFloatingPane && m_actionWindow)
            m_actionWindow->SetFocus();

        m_hintWnd->Raise();


        if (m_hintFadeAmt != m_hintFadeMax)         {
                        m_hintFadeTimer.SetOwner(this);
            m_hintFadeTimer.Start(5);
            Connect(m_hintFadeTimer.GetId(), wxEVT_TIMER,
                    wxTimerEventHandler(wxAuiManager::OnHintFadeTimer));
        }
    }
    else      {
        if (!(m_flags & wxAUI_MGR_RECTANGLE_HINT))
            return;

        if (m_lastHint != rect)
        {
                        m_lastHint = rect;
            m_frame->Refresh();
            m_frame->Update();
        }

        wxScreenDC screendc;
        wxRegion clip(1, 1, 10000, 10000);

                int i, pane_count;
        for (i = 0, pane_count = m_panes.GetCount(); i < pane_count; ++i)
        {
            wxAuiPaneInfo& pane = m_panes.Item(i);

            if (pane.IsFloating() &&
                    pane.frame &&
                        pane.frame->IsShown())
            {
                wxRect r = pane.frame->GetRect();
#ifdef __WXGTK__
                                r.width += 15;
                r.height += 35;
                r.Inflate(5);
#endif

                clip.Subtract(r);
            }
        }

                                clip.Intersect(m_frame->GetRect());

        screendc.SetDeviceClippingRegion(clip);

        wxBitmap stipple = wxPaneCreateStippleBitmap();
        wxBrush brush(stipple);
        screendc.SetBrush(brush);
        screendc.SetPen(*wxTRANSPARENT_PEN);

        screendc.DrawRectangle(rect.x, rect.y, 5, rect.height);
        screendc.DrawRectangle(rect.x+5, rect.y, rect.width-10, 5);
        screendc.DrawRectangle(rect.x+rect.width-5, rect.y, 5, rect.height);
        screendc.DrawRectangle(rect.x+5, rect.y+rect.height-5, rect.width-10, 5);
    }
}

void wxAuiManager::HideHint()
{
        if (m_hintWnd)
    {
        if (m_hintWnd->IsShown())
            m_hintWnd->Show(false);
        m_hintWnd->SetTransparent(0);
        m_hintFadeTimer.Stop();
                        Disconnect(m_hintFadeTimer.GetId(), wxEVT_TIMER,
                   wxTimerEventHandler(wxAuiManager::OnHintFadeTimer));
        m_lastHint = wxRect();
        return;
    }

        if (!m_lastHint.IsEmpty())
    {
        m_frame->Refresh();
        m_frame->Update();
        m_lastHint = wxRect();
    }
}

void wxAuiManager::OnHintActivate(wxActivateEvent& WXUNUSED(event))
{
    
                    }



void wxAuiManager::StartPaneDrag(wxWindow* pane_window,
                                 const wxPoint& offset)
{
    wxAuiPaneInfo& pane = GetPane(pane_window);
    if (!pane.IsOk())
        return;

    if (pane.IsToolbar())
    {
        m_action = actionDragToolbarPane;
    }
    else
    {
        m_action = actionDragFloatingPane;
    }

    m_actionWindow = pane_window;
    m_actionOffset = offset;
    m_frame->CaptureMouse();

    if (pane.frame)
    {
        wxRect window_rect = pane.frame->GetRect();
        wxRect client_rect = pane.frame->GetClientRect();
        wxPoint client_pt = pane.frame->ClientToScreen(client_rect.GetTopLeft());
        wxPoint origin_pt = client_pt - window_rect.GetTopLeft();
        m_actionOffset += origin_pt;
    }
}



wxRect wxAuiManager::CalculateHintRect(wxWindow* pane_window,
                                       const wxPoint& pt,
                                       const wxPoint& offset)
{
    wxRect rect;

                
    int i, pane_count, part_count;
    wxAuiDockInfoArray docks;
    wxAuiPaneInfoArray panes;
    wxAuiDockUIPartArray uiparts;
    wxAuiPaneInfo hint = GetPane(pane_window);
    hint.name = wxT("__HINT__");
    hint.PaneBorder(true);
    hint.Show();

    if (!hint.IsOk())
        return rect;

    CopyDocksAndPanes(docks, panes, m_docks, m_panes);

            for (i = 0, pane_count = panes.GetCount(); i < pane_count; ++i)
    {
        if (panes.Item(i).window == pane_window)
        {
            RemovePaneFromDocks(docks, panes.Item(i));
            panes.RemoveAt(i);
            break;
        }
    }

        if (!DoDrop(docks, panes, hint, pt, offset))
    {
        return rect;
    }

    panes.Add(hint);

    wxSizer* sizer = LayoutAll(panes, docks, uiparts, true);
    wxSize client_size = m_frame->GetClientSize();
    sizer->SetDimension(0, 0, client_size.x, client_size.y);
    sizer->Layout();

    for (i = 0, part_count = uiparts.GetCount();
         i < part_count; ++i)
    {
        wxAuiDockUIPart& part = uiparts.Item(i);

        if (part.type == wxAuiDockUIPart::typePaneBorder &&
            part.pane && part.pane->name == wxT("__HINT__"))
        {
            rect = wxRect(part.sizer_item->GetPosition(),
                          part.sizer_item->GetSize());
            break;
        }
    }

    delete sizer;

    if (rect.IsEmpty())
    {
        return rect;
    }

        m_frame->ClientToScreen(&rect.x, &rect.y);

    if ( m_frame->GetLayoutDirection() == wxLayout_RightToLeft )
    {
                rect.x -= rect.GetWidth();
    }

    return rect;
}

void wxAuiManager::DrawHintRect(wxWindow* pane_window,
                                const wxPoint& pt,
                                const wxPoint& offset)
{
    wxRect rect = CalculateHintRect(pane_window, pt, offset);

    if (rect.IsEmpty())
    {
        HideHint();
    }
    else
    {
        ShowHint(rect);
    }
}

void wxAuiManager::OnFloatingPaneMoveStart(wxWindow* wnd)
{
        wxAuiPaneInfo& pane = GetPane(wnd);
    wxASSERT_MSG(pane.IsOk(), wxT("Pane window not found"));

    if(!pane.frame)
        return;

    if (m_flags & wxAUI_MGR_TRANSPARENT_DRAG)
        pane.frame->SetTransparent(150);
}

void wxAuiManager::OnFloatingPaneMoving(wxWindow* wnd, wxDirection dir)
{
        wxAuiPaneInfo& pane = GetPane(wnd);
    wxASSERT_MSG(pane.IsOk(), wxT("Pane window not found"));

    if(!pane.frame)
        return;

    wxPoint pt = ::wxGetMousePosition();

#if 0
        if (dir == wxNORTH)
    {
                wxPoint pos( 0,0 );
        pos = wnd->ClientToScreen( pos );
        pt.y = pos.y;
                pt.y -= 5;
    }
    else if (dir == wxWEST)
    {
                wxPoint pos( 0,0 );
        pos = wnd->ClientToScreen( pos );
        pt.x = pos.x;
    }
    else if (dir == wxEAST)
    {
                wxPoint pos( wnd->GetSize().x, 0 );
        pos = wnd->ClientToScreen( pos );
        pt.x = pos.x;
    }
    else if (dir == wxSOUTH)
    {
                wxPoint pos( 0, wnd->GetSize().y );
        pos = wnd->ClientToScreen( pos );
        pt.y = pos.y;
    }
#else
    wxUnusedVar(dir);
#endif

    wxPoint client_pt = m_frame->ScreenToClient(pt);

            wxPoint frame_pos = pane.frame->GetPosition();
    wxPoint action_offset(pt.x-frame_pos.x, pt.y-frame_pos.y);

        if (pane.IsToolbar() && m_action == actionDragFloatingPane)
    {
        wxAuiDockInfoArray docks;
        wxAuiPaneInfoArray panes;
        wxAuiDockUIPartArray uiparts;
        wxAuiPaneInfo hint = pane;

        CopyDocksAndPanes(docks, panes, m_docks, m_panes);

                if (!DoDrop(docks, panes, hint, client_pt))
            return;
        if (hint.IsFloating())
            return;

        pane = hint;
        m_action = actionDragToolbarPane;
        m_actionWindow = pane.window;

        Update();

        return;
    }


            if (!CanDockPanel(pane))
    {
        HideHint();
        return;
    }


    DrawHintRect(wnd, client_pt, action_offset);

#ifdef __WXGTK__
                #endif


        m_frame->Update();
}

void wxAuiManager::OnFloatingPaneMoved(wxWindow* wnd, wxDirection dir)
{
        wxAuiPaneInfo& pane = GetPane(wnd);
    wxASSERT_MSG(pane.IsOk(), wxT("Pane window not found"));

    if(!pane.frame)
        return;

    wxPoint pt = ::wxGetMousePosition();

#if 0
        if (dir == wxNORTH)
    {
                wxPoint pos( 0,0 );
        pos = wnd->ClientToScreen( pos );
        pt.y = pos.y;
                pt.y -= 10;
    }
    else if (dir == wxWEST)
    {
                wxPoint pos( 0,0 );
        pos = wnd->ClientToScreen( pos );
        pt.x = pos.x;
    }
    else if (dir == wxEAST)
    {
                wxPoint pos( wnd->GetSize().x, 0 );
        pos = wnd->ClientToScreen( pos );
        pt.x = pos.x;
    }
    else if (dir == wxSOUTH)
    {
                wxPoint pos( 0, wnd->GetSize().y );
        pos = wnd->ClientToScreen( pos );
        pt.y = pos.y;
    }
#else
    wxUnusedVar(dir);
#endif

    wxPoint client_pt = m_frame->ScreenToClient(pt);

            wxPoint frame_pos = pane.frame->GetPosition();
    wxPoint action_offset(pt.x-frame_pos.x, pt.y-frame_pos.y);

            if (CanDockPanel(pane))
    {
                DoDrop(m_docks, m_panes, pane, client_pt, action_offset);
    }

            if (pane.IsFloating())
    {
        pane.floating_pos = pane.frame->GetPosition();

        if (m_flags & wxAUI_MGR_TRANSPARENT_DRAG)
            pane.frame->SetTransparent(255);
    }
    else if (m_hasMaximized)
    {
        RestoreMaximizedPane();
    }

    Update();

    HideHint();
}

void wxAuiManager::OnFloatingPaneResized(wxWindow* wnd, const wxRect& rect)
{
        wxAuiPaneInfo& pane = GetPane(wnd);
    wxASSERT_MSG(pane.IsOk(), wxT("Pane window not found"));

    pane.FloatingSize(rect.GetWidth(), rect.GetHeight());

        pane.FloatingPosition(rect.x, rect.y);
}


void wxAuiManager::OnFloatingPaneClosed(wxWindow* wnd, wxCloseEvent& evt)
{
        wxAuiPaneInfo& pane = GetPane(wnd);
    wxASSERT_MSG(pane.IsOk(), wxT("Pane window not found"));


        wxAuiManagerEvent e(wxEVT_AUI_PANE_CLOSE);
    e.SetPane(&pane);
    e.SetCanVeto(evt.CanVeto());
    ProcessMgrEvent(e);

    if (e.GetVeto())
    {
        evt.Veto();
        return;
    }
    else
    {
                        
        wxAuiPaneInfo& check = GetPane(wnd);
        if (check.IsOk())
        {
            ClosePane(pane);
        }
    }
}



void wxAuiManager::OnFloatingPaneActivated(wxWindow* wnd)
{
    if ((GetFlags() & wxAUI_MGR_ALLOW_ACTIVE_PANE) && GetPane(wnd).IsOk())
    {
        SetActivePane(wnd);
        Repaint();
    }
}


void wxAuiManager::OnRender(wxAuiManagerEvent& evt)
{
        if (!m_frame || wxPendingDelete.Member(m_frame))
        return;

    wxDC* dc = evt.GetDC();

#ifdef __WXMAC__
    dc->Clear() ;
#endif
    int i, part_count;
    for (i = 0, part_count = m_uiParts.GetCount();
         i < part_count; ++i)
    {
        wxAuiDockUIPart& part = m_uiParts.Item(i);

                if (part.sizer_item &&
                ((!part.sizer_item->IsWindow() &&
                  !part.sizer_item->IsSpacer() &&
                  !part.sizer_item->IsSizer()) ||
                  !part.sizer_item->IsShown() ||
                   part.rect.IsEmpty()))
            continue;

        switch (part.type)
        {
            case wxAuiDockUIPart::typeDockSizer:
            case wxAuiDockUIPart::typePaneSizer:
                m_art->DrawSash(*dc, m_frame, part.orientation, part.rect);
                break;
            case wxAuiDockUIPart::typeBackground:
                m_art->DrawBackground(*dc, m_frame, part.orientation, part.rect);
                break;
            case wxAuiDockUIPart::typeCaption:
                m_art->DrawCaption(*dc, m_frame, part.pane->caption, part.rect, *part.pane);
                break;
            case wxAuiDockUIPart::typeGripper:
                m_art->DrawGripper(*dc, m_frame, part.rect, *part.pane);
                break;
            case wxAuiDockUIPart::typePaneBorder:
                m_art->DrawBorder(*dc, m_frame, part.rect, *part.pane);
                break;
            case wxAuiDockUIPart::typePaneButton:
                m_art->DrawPaneButton(*dc, m_frame, part.button->button_id,
                        wxAUI_BUTTON_STATE_NORMAL, part.rect, *part.pane);
                break;
        }
    }
}



void wxAuiManager::Render(wxDC* dc)
{
    wxAuiManagerEvent e(wxEVT_AUI_RENDER);
    e.SetManager(this);
    e.SetDC(dc);
    ProcessMgrEvent(e);
}

void wxAuiManager::Repaint(wxDC* dc)
{
#ifdef __WXMAC__ 
    if ( dc == NULL )
    {
        m_frame->Refresh() ;
        m_frame->Update() ;
        return ;
    }
#endif
    int w, h;
    m_frame->GetClientSize(&w, &h);

                wxClientDC* client_dc = NULL;
    if (!dc)
    {
        client_dc = new wxClientDC(m_frame);
        dc = client_dc;
    }

            wxPoint pt = m_frame->GetClientAreaOrigin();
    if (pt.x != 0 || pt.y != 0)
        dc->SetDeviceOrigin(pt.x, pt.y);

        Render(dc);

        if (client_dc)
        delete client_dc;
}

void wxAuiManager::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(m_frame);
    Repaint(&dc);
}

void wxAuiManager::OnEraseBackground(wxEraseEvent& event)
{
#ifdef __WXMAC__
    event.Skip() ;
#else
    wxUnusedVar(event);
#endif
}

void wxAuiManager::OnSize(wxSizeEvent& event)
{
    if (m_frame)
    {
        DoFrameLayout();
        Repaint();

#if wxUSE_MDI
        if (wxDynamicCast(m_frame, wxMDIParentFrame))
        {
                                                            return;
        }
#endif
    }
    event.Skip();
}

void wxAuiManager::OnFindManager(wxAuiManagerEvent& evt)
{
        wxWindow* window = GetManagedWindow();
    if (!window)
    {
        evt.SetManager(NULL);
        return;
    }

        if (wxDynamicCast(window, wxAuiFloatingFrame))
    {
        wxAuiFloatingFrame* float_frame = static_cast<wxAuiFloatingFrame*>(window);
        evt.SetManager(float_frame->GetOwnerManager());
        return;
    }

        evt.SetManager(this);
}

void wxAuiManager::OnSetCursor(wxSetCursorEvent& event)
{
        wxAuiDockUIPart* part = HitTest(event.GetX(), event.GetY());
    wxCursor cursor = wxNullCursor;

    if (part)
    {
        if (part->type == wxAuiDockUIPart::typeDockSizer ||
            part->type == wxAuiDockUIPart::typePaneSizer)
        {
                                    if (part->type == wxAuiDockUIPart::typeDockSizer && part->dock &&
                part->dock->panes.GetCount() == 1 &&
                part->dock->panes.Item(0)->IsFixed())
                    return;

                        if (part->pane && part->pane->IsFixed())
                return;

            if (part->orientation == wxVERTICAL)
                cursor = wxCursor(wxCURSOR_SIZEWE);
            else
                cursor = wxCursor(wxCURSOR_SIZENS);
        }
        else if (part->type == wxAuiDockUIPart::typeGripper)
        {
            cursor = wxCursor(wxCURSOR_SIZING);
        }
    }

    event.SetCursor(cursor);
}



void wxAuiManager::UpdateButtonOnScreen(wxAuiDockUIPart* button_ui_part,
                                        const wxMouseEvent& event)
{
    wxAuiDockUIPart* hit_test = HitTest(event.GetX(), event.GetY());
    if (!hit_test || !button_ui_part)
        return;

    int state = wxAUI_BUTTON_STATE_NORMAL;

    if (hit_test == button_ui_part)
    {
        if (event.LeftDown())
            state = wxAUI_BUTTON_STATE_PRESSED;
        else
            state = wxAUI_BUTTON_STATE_HOVER;
    }
    else
    {
        if (event.LeftDown())
            state = wxAUI_BUTTON_STATE_HOVER;
    }

        wxClientDC cdc(m_frame);

            wxPoint pt = m_frame->GetClientAreaOrigin();
    if (pt.x != 0 || pt.y != 0)
        cdc.SetDeviceOrigin(pt.x, pt.y);

    if (hit_test->pane)
    {
        m_art->DrawPaneButton(cdc, m_frame,
                  button_ui_part->button->button_id,
                  state,
                  button_ui_part->rect,
                  *hit_test->pane);
    }
}

void wxAuiManager::OnLeftDown(wxMouseEvent& event)
{
    m_currentDragItem = -1;

    wxAuiDockUIPart* part = HitTest(event.GetX(), event.GetY());
    if (part)
    {
        if (part->type == wxAuiDockUIPart::typeDockSizer ||
            part->type == wxAuiDockUIPart::typePaneSizer)
        {
                                    
                                    if (part->type == wxAuiDockUIPart::typeDockSizer && part->dock &&
                part->dock->panes.GetCount() == 1 &&
                part->dock->panes.Item(0)->IsFixed())
                    return;

                        if (part->pane && part->pane->IsFixed())
                return;

            m_action = actionResize;
            m_actionPart = part;
            m_actionHintRect = wxRect();
            m_actionStart = wxPoint(event.m_x, event.m_y);
            m_actionOffset = wxPoint(event.m_x - part->rect.x,
                                      event.m_y - part->rect.y);
            m_frame->CaptureMouse();
        }
        else if (part->type == wxAuiDockUIPart::typePaneButton)
        {
            m_action = actionClickButton;
            m_actionPart = part;
            m_actionStart = wxPoint(event.m_x, event.m_y);
            m_frame->CaptureMouse();

            UpdateButtonOnScreen(part, event);
        }
        else if (part->type == wxAuiDockUIPart::typeCaption ||
                  part->type == wxAuiDockUIPart::typeGripper)
        {
                                                wxWindow* managed_wnd = GetManagedWindow();

            if (part->pane &&
                part->pane->window &&
                managed_wnd &&
                wxDynamicCast(managed_wnd, wxAuiFloatingFrame))
            {
                wxAuiFloatingFrame* floating_frame = (wxAuiFloatingFrame*)managed_wnd;
                wxAuiManager* owner_mgr = floating_frame->GetOwnerManager();
                owner_mgr->StartPaneDrag(part->pane->window,
                                             wxPoint(event.m_x - part->rect.x,
                                                     event.m_y - part->rect.y));
                return;
            }



            if (GetFlags() & wxAUI_MGR_ALLOW_ACTIVE_PANE)
            {
                                SetActivePane(part->pane->window);
                Repaint();
            }

            if (part->dock && part->dock->dock_direction == wxAUI_DOCK_CENTER)
                return;

            m_action = actionClickCaption;
            m_actionPart = part;
            m_actionStart = wxPoint(event.m_x, event.m_y);
            m_actionOffset = wxPoint(event.m_x - part->rect.x,
                                      event.m_y - part->rect.y);
            m_frame->CaptureMouse();
        }
#ifdef __WXMAC__
        else
        {
            event.Skip();
        }
#endif
    }
#ifdef __WXMAC__
    else
    {
        event.Skip();
    }
#else
    event.Skip();
#endif
}

bool wxAuiManager::DoEndResizeAction(wxMouseEvent& event)
{
        if (m_actionPart && m_actionPart->type==wxAuiDockUIPart::typeDockSizer)
    {
                int sashSize = m_art->GetMetric(wxAUI_DOCKART_SASH_SIZE);

        int used_width = 0, used_height = 0;

        wxSize client_size = m_frame->GetClientSize();

        size_t dock_i, dock_count = m_docks.GetCount();
        for (dock_i = 0; dock_i < dock_count; ++dock_i)
        {
            wxAuiDockInfo& dock = m_docks.Item(dock_i);
            if (dock.dock_direction == wxAUI_DOCK_TOP ||
                dock.dock_direction == wxAUI_DOCK_BOTTOM)
            {
                used_height += dock.size;
            }
            if (dock.dock_direction == wxAUI_DOCK_LEFT ||
                dock.dock_direction == wxAUI_DOCK_RIGHT)
            {
                used_width += dock.size;
            }
            if (dock.resizable)
                used_width += sashSize;
        }


        int available_width = client_size.GetWidth() - used_width;
        int available_height = client_size.GetHeight() - used_height;


#if wxUSE_STATUSBAR
                        if (wxDynamicCast(m_frame, wxFrame))
        {
            wxFrame* frame = static_cast<wxFrame*>(m_frame);
            wxStatusBar* status = frame->GetStatusBar();
            if (status)
            {
                wxSize status_client_size = status->GetClientSize();
                available_height -= status_client_size.GetHeight();
            }
        }
#endif

        wxRect& rect = m_actionPart->dock->rect;

        wxPoint new_pos(event.m_x - m_actionOffset.x,
            event.m_y - m_actionOffset.y);
        int new_size, old_size = m_actionPart->dock->size;

        switch (m_actionPart->dock->dock_direction)
        {
        case wxAUI_DOCK_LEFT:
            new_size = new_pos.x - rect.x;
            if (new_size-old_size > available_width)
                new_size = old_size+available_width;
            m_actionPart->dock->size = new_size;
            break;
        case wxAUI_DOCK_TOP:
            new_size = new_pos.y - rect.y;
            if (new_size-old_size > available_height)
                new_size = old_size+available_height;
            m_actionPart->dock->size = new_size;
            break;
        case wxAUI_DOCK_RIGHT:
            new_size = rect.x + rect.width - new_pos.x -
                       m_actionPart->rect.GetWidth();
            if (new_size-old_size > available_width)
                new_size = old_size+available_width;
            m_actionPart->dock->size = new_size;
            break;
        case wxAUI_DOCK_BOTTOM:
            new_size = rect.y + rect.height -
                new_pos.y - m_actionPart->rect.GetHeight();
            if (new_size-old_size > available_height)
                new_size = old_size+available_height;
            m_actionPart->dock->size = new_size;
            break;
        }

        Update();
        Repaint(NULL);
    }
    else if (m_actionPart &&
        m_actionPart->type == wxAuiDockUIPart::typePaneSizer)
    {
        wxAuiDockInfo& dock = *m_actionPart->dock;
        wxAuiPaneInfo& pane = *m_actionPart->pane;

        int total_proportion = 0;
        int dock_pixels = 0;
        int new_pixsize = 0;

        int caption_size = m_art->GetMetric(wxAUI_DOCKART_CAPTION_SIZE);
        int pane_borderSize = m_art->GetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE);
        int sashSize = m_art->GetMetric(wxAUI_DOCKART_SASH_SIZE);

        wxPoint new_pos(event.m_x - m_actionOffset.x,
            event.m_y - m_actionOffset.y);

                wxAuiDockUIPart* pane_part = GetPanePart(pane.window);
        wxASSERT_MSG(pane_part,
            wxT("Pane border part not found -- shouldn't happen"));

                        if (dock.IsHorizontal())
            new_pixsize = new_pos.x - pane_part->rect.x;
        else
            new_pixsize = new_pos.y - pane_part->rect.y;

                if (dock.IsHorizontal())
            dock_pixels = dock.rect.GetWidth();
        else
            dock_pixels = dock.rect.GetHeight();

                                int i, dock_pane_count = dock.panes.GetCount();
        int pane_position = -1;
        for (i = 0; i < dock_pane_count; ++i)
        {
            wxAuiPaneInfo& p = *dock.panes.Item(i);
            if (p.window == pane.window)
                pane_position = i;

                                                if (i > 0)
                dock_pixels -= sashSize;

                                                if (p.IsFixed())
            {
                if (dock.IsHorizontal())
                    dock_pixels -= p.best_size.x;
                else
                    dock_pixels -= p.best_size.y;
            }
            else
            {
                total_proportion += p.dock_proportion;
            }
        }

                if (new_pixsize > dock_pixels)
            new_pixsize = dock_pixels;


                                        int borrow_pane = -1;
        for (i = pane_position+1; i < dock_pane_count; ++i)
        {
            wxAuiPaneInfo& p = *dock.panes.Item(i);
            if (!p.IsFixed())
            {
                borrow_pane = i;
                break;
            }
        }


                        wxASSERT_MSG(pane_position != -1, wxT("Pane not found in dock"));

                if (dock_pixels == 0 || total_proportion == 0 || borrow_pane == -1)
        {
            m_action = actionNone;
            return false;
        }

                int new_proportion = (new_pixsize*total_proportion)/dock_pixels;

                int min_size = 0;

                                        if (pane.min_size.IsFullySpecified())
        {
            min_size = 0;

            if (pane.HasBorder())
                min_size += (pane_borderSize*2);

                        if (pane_part->orientation == wxVERTICAL)
            {
                min_size += pane.min_size.y;
                if (pane.HasCaption())
                    min_size += caption_size;
            }
            else
            {
                min_size += pane.min_size.x;
            }
        }


                                        min_size++;

        int min_proportion = (min_size*total_proportion)/dock_pixels;

        if (new_proportion < min_proportion)
            new_proportion = min_proportion;



        int prop_diff = new_proportion - pane.dock_proportion;

                                int prop_borrow = dock.panes.Item(borrow_pane)->dock_proportion;

        if (prop_borrow - prop_diff < 0)
        {
                                    prop_borrow = min_proportion;
        }
         else
        {
            prop_borrow -= prop_diff;
        }


        dock.panes.Item(borrow_pane)->dock_proportion = prop_borrow;
        pane.dock_proportion = new_proportion;


                Update();
        Repaint(NULL);
    }

    return true;
}

void wxAuiManager::OnLeftUp(wxMouseEvent& event)
{
    if (m_action == actionResize)
    {
        m_frame->ReleaseMouse();

        if (!wxAuiManager_HasLiveResize(*this))
        {
                        wxScreenDC dc;
            DrawResizeHint(dc, m_actionHintRect);
        }
        if (m_currentDragItem != -1 && wxAuiManager_HasLiveResize(*this))
            m_actionPart = & (m_uiParts.Item(m_currentDragItem));

        DoEndResizeAction(event);

        m_currentDragItem = -1;

    }
    else if (m_action == actionClickButton)
    {
        m_hoverButton = NULL;
        m_frame->ReleaseMouse();

        if (m_actionPart)
        {
            UpdateButtonOnScreen(m_actionPart, event);

                        if (m_actionPart == HitTest(event.GetX(), event.GetY()))
            {
                                wxAuiManagerEvent e(wxEVT_AUI_PANE_BUTTON);
                e.SetManager(this);
                e.SetPane(m_actionPart->pane);
                e.SetButton(m_actionPart->button->button_id);
                ProcessMgrEvent(e);
            }
        }
    }
    else if (m_action == actionClickCaption)
    {
        m_frame->ReleaseMouse();
    }
    else if (m_action == actionDragFloatingPane)
    {
        m_frame->ReleaseMouse();
    }
    else if (m_action == actionDragToolbarPane)
    {
        m_frame->ReleaseMouse();

        wxAuiPaneInfo& pane = GetPane(m_actionWindow);
        wxASSERT_MSG(pane.IsOk(), wxT("Pane window not found"));

                wxAuiDockInfoPtrArray docks;
        FindDocks(m_docks, pane.dock_direction,
                  pane.dock_layer, pane.dock_row, docks);
        if (docks.GetCount() == 1)
        {
            wxAuiDockInfo& dock = *docks.Item(0);

            wxArrayInt pane_positions, pane_sizes;
            GetPanePositionsAndSizes(dock, pane_positions, pane_sizes);

            int i, dock_pane_count = dock.panes.GetCount();
            for (i = 0; i < dock_pane_count; ++i)
                dock.panes.Item(i)->dock_pos = pane_positions[i];
        }

        pane.state &= ~wxAuiPaneInfo::actionPane;
        Update();
    }
    else
    {
        event.Skip();
    }

    m_action = actionNone;
    m_lastMouseMove = wxPoint(); }


void wxAuiManager::OnMotion(wxMouseEvent& event)
{
                    
    wxPoint mouse_pos = event.GetPosition();
    if (m_lastMouseMove == mouse_pos)
        return;
    m_lastMouseMove = mouse_pos;


    if (m_action == actionResize)
    {
                        if (m_currentDragItem != -1)
            m_actionPart = & (m_uiParts.Item(m_currentDragItem));
        else
            m_currentDragItem = m_uiParts.Index(* m_actionPart);

        if (m_actionPart)
        {
            wxPoint pos = m_actionPart->rect.GetPosition();
            if (m_actionPart->orientation == wxHORIZONTAL)
                pos.y = wxMax(0, event.m_y - m_actionOffset.y);
            else
                pos.x = wxMax(0, event.m_x - m_actionOffset.x);

            if (wxAuiManager_HasLiveResize(*this))
            {
                m_frame->ReleaseMouse();
                DoEndResizeAction(event);
                m_frame->CaptureMouse();
            }
            else
            {
                wxRect rect(m_frame->ClientToScreen(pos),
                    m_actionPart->rect.GetSize());
                wxScreenDC dc;

                if (!m_actionHintRect.IsEmpty())
                {
                                        DrawResizeHint(dc, m_actionHintRect);
                    m_actionHintRect = wxRect();
                }

                                wxRect frameScreenRect = m_frame->GetScreenRect();
                if (frameScreenRect.Contains(rect))
                {
                    DrawResizeHint(dc, rect);
                    m_actionHintRect = rect;
                }
            }
        }
    }
    else if (m_action == actionClickCaption)
    {
        int drag_x_threshold = wxSystemSettings::GetMetric(wxSYS_DRAG_X);
        int drag_y_threshold = wxSystemSettings::GetMetric(wxSYS_DRAG_Y);

                                if (m_actionPart &&
            (abs(event.m_x - m_actionStart.x) > drag_x_threshold ||
             abs(event.m_y - m_actionStart.y) > drag_y_threshold))
        {
            wxAuiPaneInfo* paneInfo = m_actionPart->pane;

            if (!paneInfo->IsToolbar())
            {
                if ((m_flags & wxAUI_MGR_ALLOW_FLOATING) &&
                    paneInfo->IsFloatable())
                {
                    m_action = actionDragFloatingPane;

                                        wxPoint pt = m_frame->ClientToScreen(event.GetPosition());
                    paneInfo->floating_pos = wxPoint(pt.x - m_actionOffset.x,
                                                      pt.y - m_actionOffset.y);

                                        if (paneInfo->IsMaximized())
                        RestorePane(*paneInfo);
                    paneInfo->Float();
                    Update();

                    m_actionWindow = paneInfo->frame;

                                                                                                                        wxSize frame_size = m_actionWindow->GetSize();
                    if (frame_size.x <= m_actionOffset.x)
                        m_actionOffset.x = 30;
                }
            }
            else
            {
                m_action = actionDragToolbarPane;
                m_actionWindow = paneInfo->window;
            }
        }
    }
    else if (m_action == actionDragFloatingPane)
    {
        if (m_actionWindow)
        {
                                                if (!wxDynamicCast(m_actionWindow, wxAuiFloatingFrame))
            {
                wxAuiPaneInfo& pane = GetPane(m_actionWindow);
                m_actionWindow = pane.frame;
            }

            wxPoint pt = m_frame->ClientToScreen(event.GetPosition());
            m_actionWindow->Move(pt.x - m_actionOffset.x,
                                pt.y - m_actionOffset.y);
        }
    }
    else if (m_action == actionDragToolbarPane)
    {
        wxAuiPaneInfo& pane = GetPane(m_actionWindow);
        wxASSERT_MSG(pane.IsOk(), wxT("Pane window not found"));

        pane.SetFlag(wxAuiPaneInfo::actionPane, true);

        wxPoint point = event.GetPosition();
        DoDrop(m_docks, m_panes, pane, point, m_actionOffset);

                        if (pane.IsFloating())
        {
            wxPoint pt = m_frame->ClientToScreen(event.GetPosition());
            pane.floating_pos = wxPoint(pt.x - m_actionOffset.x,
                                        pt.y - m_actionOffset.y);
        }

                                        Update();

                                if (pane.IsFloating())
        {
            pane.state &= ~wxAuiPaneInfo::actionPane;
            m_action = actionDragFloatingPane;
            m_actionWindow = pane.frame;
        }
    }
    else
    {
        wxAuiDockUIPart* part = HitTest(event.GetX(), event.GetY());
        if (part && part->type == wxAuiDockUIPart::typePaneButton)
        {
            if (part != m_hoverButton)
            {
                                if (m_hoverButton)
                {
                    UpdateButtonOnScreen(m_hoverButton, event);
                    Repaint();
                }

                                                UpdateButtonOnScreen(part, event);
                m_hoverButton = part;

            }
        }
        else
        {
            if (m_hoverButton)
            {
                m_hoverButton = NULL;
                Repaint();
            }
            else
            {
                event.Skip();
            }
        }
    }
}

void wxAuiManager::OnLeaveWindow(wxMouseEvent& WXUNUSED(event))
{
    if (m_hoverButton)
    {
        m_hoverButton = NULL;
        Repaint();
    }
}

void wxAuiManager::OnCaptureLost(wxMouseCaptureLostEvent& WXUNUSED(event))
{
        if ( m_action != actionNone )
    {
        m_action = actionNone;
        HideHint();
    }
}

void wxAuiManager::OnChildFocus(wxChildFocusEvent& event)
{
                if (GetFlags() & wxAUI_MGR_ALLOW_ACTIVE_PANE)
    {
        wxAuiPaneInfo& pane = GetPane(event.GetWindow());
        if (pane.IsOk() && (pane.state & wxAuiPaneInfo::optionActive) == 0)
        {
            SetActivePane(event.GetWindow());
            m_frame->Refresh();
        }
    }

    event.Skip();
}


void wxAuiManager::OnPaneButton(wxAuiManagerEvent& evt)
{
    wxASSERT_MSG(evt.pane, wxT("Pane Info passed to wxAuiManager::OnPaneButton must be non-null"));

    wxAuiPaneInfo& pane = *(evt.pane);

    if (evt.button == wxAUI_BUTTON_CLOSE)
    {
                wxAuiManagerEvent e(wxEVT_AUI_PANE_CLOSE);
        e.SetManager(this);
        e.SetPane(evt.pane);
        ProcessMgrEvent(e);

        if (!e.GetVeto())
        {
                                    
            wxAuiPaneInfo& check = GetPane(pane.window);
            if (check.IsOk())
            {
                ClosePane(pane);
            }

            Update();
        }
    }
    else if (evt.button == wxAUI_BUTTON_MAXIMIZE_RESTORE && !pane.IsMaximized())
    {
                wxAuiManagerEvent e(wxEVT_AUI_PANE_MAXIMIZE);
        e.SetManager(this);
        e.SetPane(evt.pane);
        ProcessMgrEvent(e);

        if (!e.GetVeto())
        {
            MaximizePane(pane);
            Update();
        }
    }
    else if (evt.button == wxAUI_BUTTON_MAXIMIZE_RESTORE && pane.IsMaximized())
    {
                wxAuiManagerEvent e(wxEVT_AUI_PANE_RESTORE);
        e.SetManager(this);
        e.SetPane(evt.pane);
        ProcessMgrEvent(e);

        if (!e.GetVeto())
        {
            RestorePane(pane);
            Update();
        }
    }
    else if (evt.button == wxAUI_BUTTON_PIN &&
                (m_flags & wxAUI_MGR_ALLOW_FLOATING) && pane.IsFloatable())
    {
        if (pane.IsMaximized())
        {
                                                wxAuiManagerEvent e(wxEVT_AUI_PANE_RESTORE);
            e.SetManager(this);
            e.SetPane(evt.pane);
            ProcessMgrEvent(e);

            if (e.GetVeto())
            {
                                return;
            }

            RestorePane(pane);
        }

        pane.Float();
        Update();
    }
}

#endif 