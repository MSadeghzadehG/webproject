


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_AUI
#if wxUSE_MDI

#include "wx/aui/tabmdi.h"

#ifndef WX_PRECOMP
    #include "wx/panel.h"
    #include "wx/menu.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/settings.h"
#endif 
#include "wx/stockitem.h"
#include "wx/aui/dockart.h"

enum MDI_MENU_ID
{
    wxWINDOWCLOSE = 4001,
    wxWINDOWCLOSEALL,
    wxWINDOWNEXT,
    wxWINDOWPREV
};


wxIMPLEMENT_DYNAMIC_CLASS(wxAuiMDIParentFrame, wxFrame);

wxBEGIN_EVENT_TABLE(wxAuiMDIParentFrame, wxFrame)
#if wxUSE_MENUS
    EVT_MENU (wxID_ANY, wxAuiMDIParentFrame::DoHandleMenu)
    EVT_UPDATE_UI (wxID_ANY, wxAuiMDIParentFrame::DoHandleUpdateUI)
#endif
wxEND_EVENT_TABLE()

wxAuiMDIParentFrame::wxAuiMDIParentFrame()
{
    Init();
}

wxAuiMDIParentFrame::wxAuiMDIParentFrame(wxWindow *parent,
                                         wxWindowID id,
                                         const wxString& title,
                                         const wxPoint& pos,
                                         const wxSize& size,
                                         long style,
                                         const wxString& name)
{
    Init();
    (void)Create(parent, id, title, pos, size, style, name);
}

wxAuiMDIParentFrame::~wxAuiMDIParentFrame()
{
        SendDestroyEvent();
        wxDELETE(m_pClientWindow);

#if wxUSE_MENUS
    wxDELETE(m_pMyMenuBar);
    RemoveWindowMenu(GetMenuBar());
    wxDELETE(m_pWindowMenu);
#endif }

bool wxAuiMDIParentFrame::Create(wxWindow *parent,
                                 wxWindowID id,
                                 const wxString& title,
                                 const wxPoint& pos,
                                 const wxSize& size,
                                 long style,
                                 const wxString& name)
{
#if wxUSE_MENUS
            if (!(style & wxFRAME_NO_WINDOW_MENU))
    {
        m_pWindowMenu = new wxMenu;
        m_pWindowMenu->Append(wxWINDOWCLOSE,    _("Cl&ose"));
        m_pWindowMenu->Append(wxWINDOWCLOSEALL, _("Close All"));
        m_pWindowMenu->AppendSeparator();
        m_pWindowMenu->Append(wxWINDOWNEXT,     _("&Next"));
        m_pWindowMenu->Append(wxWINDOWPREV,     _("&Previous"));
    }
#endif 
    if ( !wxFrame::Create(parent, id, title, pos, size, style, name) )
        return false;

    m_pClientWindow = OnCreateClient();
    return m_pClientWindow != NULL;
}


void wxAuiMDIParentFrame::SetArtProvider(wxAuiTabArt* provider)
{
    if (m_pClientWindow)
    {
        m_pClientWindow->SetArtProvider(provider);
    }
}

wxAuiTabArt* wxAuiMDIParentFrame::GetArtProvider()
{
    if (!m_pClientWindow)
        return NULL;

    return m_pClientWindow->GetArtProvider();
}

wxAuiNotebook* wxAuiMDIParentFrame::GetNotebook() const
{
    return static_cast<wxAuiNotebook*>(m_pClientWindow);
}



#if wxUSE_MENUS
void wxAuiMDIParentFrame::SetWindowMenu(wxMenu* pMenu)
{
        wxMenuBar *pMenuBar = GetMenuBar();

    if (m_pWindowMenu)
    {
        RemoveWindowMenu(pMenuBar);
        wxDELETE(m_pWindowMenu);
    }

    if (pMenu)
    {
        m_pWindowMenu = pMenu;
        AddWindowMenu(pMenuBar);
    }
}

void wxAuiMDIParentFrame::SetMenuBar(wxMenuBar* pMenuBar)
{
        RemoveWindowMenu(GetMenuBar());

        AddWindowMenu(pMenuBar);

    wxFrame::SetMenuBar(pMenuBar);
    }
#endif 
void wxAuiMDIParentFrame::SetChildMenuBar(wxAuiMDIChildFrame* pChild)
{
#if wxUSE_MENUS
    if (!pChild)
    {
                if (m_pMyMenuBar)
            SetMenuBar(m_pMyMenuBar);
        else
            SetMenuBar(GetMenuBar());

                m_pMyMenuBar = NULL;
    }
    else
    {
        if (pChild->GetMenuBar() == NULL)
            return;

                if (m_pMyMenuBar == NULL)
            m_pMyMenuBar = GetMenuBar();

        SetMenuBar(pChild->GetMenuBar());
    }
#endif }

bool wxAuiMDIParentFrame::ProcessEvent(wxEvent& event)
{
        if (m_pLastEvt == &event)
        return false;
    m_pLastEvt = &event;

        bool res = false;
    wxAuiMDIChildFrame* pActiveChild = GetActiveChild();
    if (pActiveChild &&
        event.IsCommandEvent() &&
        event.GetEventObject() != m_pClientWindow &&
           !(event.GetEventType() == wxEVT_ACTIVATE ||
             event.GetEventType() == wxEVT_SET_FOCUS ||
             event.GetEventType() == wxEVT_KILL_FOCUS ||
             event.GetEventType() == wxEVT_CHILD_FOCUS ||
             event.GetEventType() == wxEVT_COMMAND_SET_FOCUS ||
             event.GetEventType() == wxEVT_COMMAND_KILL_FOCUS )
       )
    {
        res = pActiveChild->GetEventHandler()->ProcessEvent(event);
    }

    if (!res)
    {
                                res = wxEvtHandler::ProcessEvent(event);
    }

    m_pLastEvt = NULL;

    return res;
}

wxAuiMDIChildFrame *wxAuiMDIParentFrame::GetActiveChild() const
{
            wxAuiMDIClientWindow* const client = GetClientWindow();
    return client ? client->GetActiveChild() : NULL;
}

void wxAuiMDIParentFrame::SetActiveChild(wxAuiMDIChildFrame* pChildFrame)
{
    wxAuiMDIClientWindow* const client = GetClientWindow();
    if (client && client->GetActiveChild() != pChildFrame)
    {
        client->SetActiveChild(pChildFrame);
    }
}

wxAuiMDIClientWindow *wxAuiMDIParentFrame::GetClientWindow() const
{
    return m_pClientWindow;
}

wxAuiMDIClientWindow *wxAuiMDIParentFrame::OnCreateClient()
{
    return new wxAuiMDIClientWindow( this );
}

void wxAuiMDIParentFrame::ActivateNext()
{
    if (m_pClientWindow && m_pClientWindow->GetSelection() != wxNOT_FOUND)
    {
        size_t active = m_pClientWindow->GetSelection() + 1;
        if (active >= m_pClientWindow->GetPageCount())
            active = 0;

        m_pClientWindow->SetSelection(active);
    }
}

void wxAuiMDIParentFrame::ActivatePrevious()
{
    if (m_pClientWindow && m_pClientWindow->GetSelection() != wxNOT_FOUND)
    {
        int active = m_pClientWindow->GetSelection() - 1;
        if (active < 0)
            active = m_pClientWindow->GetPageCount() - 1;

        m_pClientWindow->SetSelection(active);
    }
}

void wxAuiMDIParentFrame::Init()
{
    m_pLastEvt = NULL;
    m_pClientWindow = NULL;
#if wxUSE_MENUS
    m_pWindowMenu = NULL;
    m_pMyMenuBar = NULL;
#endif }

#if wxUSE_MENUS
void wxAuiMDIParentFrame::RemoveWindowMenu(wxMenuBar* pMenuBar)
{
    if (pMenuBar && m_pWindowMenu)
    {
                int pos = pMenuBar->FindMenu(_("&Window"));
        if (pos != wxNOT_FOUND)
        {
                        wxASSERT(m_pWindowMenu == pMenuBar->GetMenu(pos));
            pMenuBar->Remove(pos);
        }
    }
}

void wxAuiMDIParentFrame::AddWindowMenu(wxMenuBar *pMenuBar)
{
    if (pMenuBar && m_pWindowMenu)
    {
        int pos = pMenuBar->FindMenu(wxGetStockLabel(wxID_HELP,wxSTOCK_NOFLAGS));
        if (pos == wxNOT_FOUND)
            pMenuBar->Append(m_pWindowMenu, _("&Window"));
        else
            pMenuBar->Insert(pos, m_pWindowMenu, _("&Window"));
    }
}

void wxAuiMDIParentFrame::DoHandleMenu(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case wxWINDOWCLOSE:
        {
            wxAuiMDIChildFrame* pActiveChild = GetActiveChild();
            if (pActiveChild)
                pActiveChild->Close();
            break;
        }
        case wxWINDOWCLOSEALL:
        {
            wxAuiMDIChildFrame* pActiveChild;
            while ((pActiveChild = GetActiveChild()) != NULL)
            {
                if (!pActiveChild->Close())
                {
                    return;                 }
            }
            break;
        }
        case wxWINDOWNEXT:
            ActivateNext();
            break;
        case wxWINDOWPREV:
            ActivatePrevious();
            break;
        default:
            event.Skip();
    }
}

void wxAuiMDIParentFrame::DoHandleUpdateUI(wxUpdateUIEvent& event)
{
    switch (event.GetId())
    {
        case wxWINDOWCLOSE:
        case wxWINDOWCLOSEALL:
        {
            wxAuiMDIClientWindow* client_window = GetClientWindow();
            wxCHECK_RET(client_window, wxS("Missing MDI Client Window"));
            size_t pages = client_window->GetPageCount();
            event.Enable(pages >= 1);
            break;
        }

        case wxWINDOWNEXT:
        case wxWINDOWPREV:
        {
            wxAuiMDIClientWindow* client_window = GetClientWindow();
            wxCHECK_RET(client_window, wxS("Missing MDI Client Window"));
            size_t pages = client_window->GetPageCount();
            event.Enable(pages >= 2);
            break;
        }

        default:
            event.Skip();
    }
}
#endif 
void wxAuiMDIParentFrame::DoGetClientSize(int* width, int* height) const
{
    wxFrame::DoGetClientSize(width, height);
}

void wxAuiMDIParentFrame::Tile(wxOrientation orient)
{
    wxAuiMDIClientWindow* client_window = GetClientWindow();
    wxASSERT_MSG(client_window, wxT("Missing MDI Client Window"));

    int cur_idx = client_window->GetSelection();
    if (cur_idx == -1)
        return;

    if (orient == wxVERTICAL)
    {
        client_window->Split(cur_idx, wxLEFT);
    }
    else if (orient == wxHORIZONTAL)
    {
        client_window->Split(cur_idx, wxTOP);
    }
}



wxIMPLEMENT_DYNAMIC_CLASS(wxAuiMDIChildFrame, wxPanel);

wxBEGIN_EVENT_TABLE(wxAuiMDIChildFrame, wxPanel)
    EVT_MENU_HIGHLIGHT_ALL(wxAuiMDIChildFrame::OnMenuHighlight)
    EVT_ACTIVATE(wxAuiMDIChildFrame::OnActivate)
    EVT_CLOSE(wxAuiMDIChildFrame::OnCloseWindow)
wxEND_EVENT_TABLE()

wxAuiMDIChildFrame::wxAuiMDIChildFrame()
{
    Init();
}

wxAuiMDIChildFrame::wxAuiMDIChildFrame(wxAuiMDIParentFrame *parent,
                                       wxWindowID id,
                                       const wxString& title,
                                       const wxPoint& WXUNUSED(pos),
                                       const wxSize& size,
                                       long style,
                                       const wxString& name)
{
    Init();

                                if (style & wxMINIMIZE)
        m_activateOnCreate = false;

    Create(parent, id, title, wxDefaultPosition, size, 0, name);
}

wxAuiMDIChildFrame::~wxAuiMDIChildFrame()
{
    wxAuiMDIParentFrame* pParentFrame = GetMDIParentFrame();
    if (pParentFrame)
    {
        if (pParentFrame->GetActiveChild() == this)
        {
            pParentFrame->SetActiveChild(NULL);
            pParentFrame->SetChildMenuBar(NULL);
        }
        wxAuiMDIClientWindow* pClientWindow = pParentFrame->GetClientWindow();
        wxASSERT(pClientWindow);
        int idx = pClientWindow->GetPageIndex(this);
        if (idx != wxNOT_FOUND)
        {
            pClientWindow->RemovePage(idx);
        }
    }

#if wxUSE_MENUS
    wxDELETE(m_pMenuBar);
#endif }

bool wxAuiMDIChildFrame::Create(wxAuiMDIParentFrame* parent,
                                wxWindowID id,
                                const wxString& title,
                                const wxPoint& WXUNUSED(pos),
                                const wxSize& size,
                                long style,
                                const wxString& name)
{
    wxAuiMDIClientWindow* pClientWindow = parent->GetClientWindow();
    wxASSERT_MSG((pClientWindow != NULL), wxT("Missing MDI client window."));

        if (style & wxMINIMIZE)
        m_activateOnCreate = false;

    wxSize cli_size = pClientWindow->GetClientSize();

        wxPanel::Create(pClientWindow,
                    id,
                    wxPoint(cli_size.x+1, cli_size.y+1),
                    size,
                    wxNO_BORDER, name);

    DoShow(false);

    SetMDIParentFrame(parent);

    m_title = title;

    pClientWindow->AddPage(this, title, m_activateOnCreate);

                    wxASSERT_MSG
    (
        (m_activateOnCreate || pClientWindow->GetPageCount() == 1)
            == (parent->GetActiveChild() == this),
        wxS("Logic error: child [not] activated when it should [not] have been.")
    );

    pClientWindow->Refresh();

    return true;
}

bool wxAuiMDIChildFrame::Destroy()
{
    wxAuiMDIParentFrame* pParentFrame = GetMDIParentFrame();
    wxASSERT_MSG(pParentFrame, wxT("Missing MDI Parent Frame"));

    wxAuiMDIClientWindow* pClientWindow = pParentFrame->GetClientWindow();
    wxASSERT_MSG(pClientWindow, wxT("Missing MDI Client Window"));

    if (pParentFrame->GetActiveChild() == this)
    {
                wxActivateEvent event(wxEVT_ACTIVATE, false, GetId());
        event.SetEventObject(this);
        GetEventHandler()->ProcessEvent(event);

        pParentFrame->SetChildMenuBar(NULL);
    }

    size_t page_count = pClientWindow->GetPageCount();
    for (size_t pos = 0; pos < page_count; pos++)
    {
        if (pClientWindow->GetPage(pos) == this)
            return pClientWindow->DeletePage(pos);
    }

    return false;
}

#if wxUSE_MENUS
void wxAuiMDIChildFrame::SetMenuBar(wxMenuBar *menu_bar)
{
    wxMenuBar *pOldMenuBar = m_pMenuBar;
    m_pMenuBar = menu_bar;

    if (m_pMenuBar)
    {
        wxAuiMDIParentFrame* pParentFrame = GetMDIParentFrame();
        wxASSERT_MSG(pParentFrame, wxT("Missing MDI Parent Frame"));

        m_pMenuBar->SetParent(pParentFrame);
        if (pParentFrame->GetActiveChild() == this)
        {
                        if (pOldMenuBar)
                pParentFrame->SetChildMenuBar(NULL);
            pParentFrame->SetChildMenuBar(this);
        }
    }
}

wxMenuBar *wxAuiMDIChildFrame::GetMenuBar() const
{
    return m_pMenuBar;
}
#endif 
void wxAuiMDIChildFrame::SetTitle(const wxString& title)
{
    m_title = title;

    wxAuiMDIParentFrame* pParentFrame = GetMDIParentFrame();
    wxASSERT_MSG(pParentFrame, wxT("Missing MDI Parent Frame"));

    wxAuiMDIClientWindow* pClientWindow = pParentFrame->GetClientWindow();
    if (pClientWindow != NULL)
    {
        size_t pos;
        for (pos = 0; pos < pClientWindow->GetPageCount(); pos++)
        {
            if (pClientWindow->GetPage(pos) == this)
            {
                pClientWindow->SetPageText(pos, m_title);
                break;
            }
        }
    }
}

wxString wxAuiMDIChildFrame::GetTitle() const
{
    return m_title;
}

void wxAuiMDIChildFrame::SetIcons(const wxIconBundle& icons)
{
        SetIcon(icons.GetIcon(-1));
    m_iconBundle = icons;
}

const wxIconBundle& wxAuiMDIChildFrame::GetIcons() const
{
    return m_iconBundle;
}

void wxAuiMDIChildFrame::SetIcon(const wxIcon& icon)
{
    wxAuiMDIParentFrame* pParentFrame = GetMDIParentFrame();
    wxASSERT_MSG(pParentFrame, wxT("Missing MDI Parent Frame"));

    m_icon = icon;

    wxBitmap bmp;
    bmp.CopyFromIcon(m_icon);

    wxAuiMDIClientWindow* pClientWindow = pParentFrame->GetClientWindow();
    if (pClientWindow != NULL)
    {
        int idx = pClientWindow->GetPageIndex(this);

        if (idx != -1)
        {
            pClientWindow->SetPageBitmap((size_t)idx, bmp);
        }
    }
}

const wxIcon& wxAuiMDIChildFrame::GetIcon() const
{
    return m_icon;
}


void wxAuiMDIChildFrame::Activate()
{
    wxAuiMDIParentFrame* pParentFrame = GetMDIParentFrame();
    wxASSERT_MSG(pParentFrame, wxT("Missing MDI Parent Frame"));

    wxAuiMDIClientWindow* pClientWindow = pParentFrame->GetClientWindow();

    if (pClientWindow != NULL)
    {
        size_t pos;
        for (pos = 0; pos < pClientWindow->GetPageCount(); pos++)
        {
            if (pClientWindow->GetPage(pos) == this)
            {
                pClientWindow->SetSelection(pos);
                break;
            }
        }
    }
}

void wxAuiMDIChildFrame::OnMenuHighlight(wxMenuEvent& event)
{
#if wxUSE_STATUSBAR
    if (m_pMDIParentFrame)
    {
                        m_pMDIParentFrame->OnMenuHighlight(event);
    }
#else
    wxUnusedVar(event);
#endif }

void wxAuiMDIChildFrame::OnActivate(wxActivateEvent& WXUNUSED(event))
{
    }

void wxAuiMDIChildFrame::OnCloseWindow(wxCloseEvent& WXUNUSED(event))
{
    Destroy();
}

void wxAuiMDIChildFrame::SetMDIParentFrame(wxAuiMDIParentFrame* parentFrame)
{
    m_pMDIParentFrame = parentFrame;
}

wxAuiMDIParentFrame* wxAuiMDIChildFrame::GetMDIParentFrame() const
{
    return m_pMDIParentFrame;
}

void wxAuiMDIChildFrame::Init()
{
    m_activateOnCreate = true;
    m_pMDIParentFrame = NULL;
#if wxUSE_MENUS
    m_pMenuBar = NULL;
#endif }

bool wxAuiMDIChildFrame::Show(bool show)
{
                        wxCHECK_MSG( !GetHandle(), false,
                 wxS("Show() has no effect after Create(). Do you mean Activate()?") );

    m_activateOnCreate = show;

        return true;
}

void wxAuiMDIChildFrame::DoShow(bool show)
{
    wxWindow::Show(show);
}

void wxAuiMDIChildFrame::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
    m_mdiNewRect = wxRect(x, y, width, height);
#ifdef __WXGTK__
    wxPanel::DoSetSize(x,y,width, height, sizeFlags);
#else
    wxUnusedVar(sizeFlags);
#endif
}

void wxAuiMDIChildFrame::DoMoveWindow(int x, int y, int width, int height)
{
    m_mdiNewRect = wxRect(x, y, width, height);
}

void wxAuiMDIChildFrame::ApplyMDIChildFrameRect()
{
    if (m_mdiCurRect != m_mdiNewRect)
    {
        wxPanel::DoMoveWindow(m_mdiNewRect.x, m_mdiNewRect.y,
                              m_mdiNewRect.width, m_mdiNewRect.height);
        m_mdiCurRect = m_mdiNewRect;
    }
}



wxIMPLEMENT_DYNAMIC_CLASS(wxAuiMDIClientWindow, wxAuiNotebook);

wxBEGIN_EVENT_TABLE(wxAuiMDIClientWindow, wxAuiNotebook)
    EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY, wxAuiMDIClientWindow::OnPageChanged)
    EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, wxAuiMDIClientWindow::OnPageClose)
    EVT_SIZE(wxAuiMDIClientWindow::OnSize)
wxEND_EVENT_TABLE()

wxAuiMDIClientWindow::wxAuiMDIClientWindow()
{
}

wxAuiMDIClientWindow::wxAuiMDIClientWindow(wxAuiMDIParentFrame* parent, long style)
{
    CreateClient(parent, style);
}

bool wxAuiMDIClientWindow::CreateClient(wxAuiMDIParentFrame* parent, long style)
{
    SetWindowStyleFlag(style);

    wxSize caption_icon_size =
            wxSize(wxSystemSettings::GetMetric(wxSYS_SMALLICON_X),
                   wxSystemSettings::GetMetric(wxSYS_SMALLICON_Y));
    SetUniformBitmapSize(caption_icon_size);

    if (!wxAuiNotebook::Create(parent,
                               wxID_ANY,
                               wxPoint(0,0),
                               wxSize(100, 100),
                               wxAUI_NB_DEFAULT_STYLE | wxNO_BORDER))
    {
        return false;
    }

    wxColour bkcolour = wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE);
    SetOwnBackgroundColour(bkcolour);

    m_mgr.GetArtProvider()->SetColour(wxAUI_DOCKART_BACKGROUND_COLOUR, bkcolour);

    return true;
}

int wxAuiMDIClientWindow::SetSelection(size_t nPage)
{
    return wxAuiNotebook::SetSelection(nPage);
}

wxAuiMDIChildFrame* wxAuiMDIClientWindow::GetActiveChild()
{
    const int sel = GetSelection();
    if ( sel == wxNOT_FOUND )
        return NULL;

    return wxStaticCast(GetPage(sel), wxAuiMDIChildFrame);
}

void wxAuiMDIClientWindow::PageChanged(int old_selection, int new_selection)
{
        if (old_selection == new_selection)
        return;

    


        if ((old_selection != -1) && (old_selection < (int)GetPageCount()))
    {
        wxAuiMDIChildFrame* old_child = (wxAuiMDIChildFrame*)GetPage(old_selection);
        wxASSERT_MSG(old_child, wxT("wxAuiMDIClientWindow::PageChanged - null page pointer"));

        wxActivateEvent event(wxEVT_ACTIVATE, false, old_child->GetId());
        event.SetEventObject(old_child);
        old_child->GetEventHandler()->ProcessEvent(event);
    }

        if (new_selection != -1)
    {
        wxAuiMDIChildFrame* active_child = (wxAuiMDIChildFrame*)GetPage(new_selection);
        wxASSERT_MSG(active_child, wxT("wxAuiMDIClientWindow::PageChanged - null page pointer"));

        wxActivateEvent event(wxEVT_ACTIVATE, true, active_child->GetId());
        event.SetEventObject(active_child);
        active_child->GetEventHandler()->ProcessEvent(event);

        if (active_child->GetMDIParentFrame())
        {
            active_child->GetMDIParentFrame()->SetActiveChild(active_child);
            active_child->GetMDIParentFrame()->SetChildMenuBar(active_child);
        }
    }


}

void wxAuiMDIClientWindow::OnPageClose(wxAuiNotebookEvent& evt)
{
    wxAuiMDIChildFrame* wnd;
    wnd = static_cast<wxAuiMDIChildFrame*>(GetPage(evt.GetSelection()));

    wnd->Close();

                evt.Veto();
}

void wxAuiMDIClientWindow::OnPageChanged(wxAuiNotebookEvent& evt)
{
    PageChanged(evt.GetOldSelection(), evt.GetSelection());
}

void wxAuiMDIClientWindow::OnSize(wxSizeEvent& evt)
{
    wxAuiNotebook::OnSize(evt);

    for (size_t pos = 0; pos < GetPageCount(); pos++)
        ((wxAuiMDIChildFrame *)GetPage(pos))->ApplyMDIChildFrameRect();
}

#endif #endif 