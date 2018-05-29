


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_MDI

#ifndef WX_PRECOMP
    #include "wx/menu.h"
    #include "wx/intl.h"
    #include "wx/log.h"
#endif 
#include "wx/mdi.h"
#include "wx/generic/mdig.h"
#include "wx/notebook.h"
#include "wx/scopeguard.h"

#include "wx/stockitem.h"

enum MDI_MENU_ID
{
    wxWINDOWCLOSE = 4001,
    wxWINDOWCLOSEALL,
    wxWINDOWNEXT,
    wxWINDOWPREV
};


wxIMPLEMENT_DYNAMIC_CLASS(wxGenericMDIParentFrame, wxFrame);

wxBEGIN_EVENT_TABLE(wxGenericMDIParentFrame, wxFrame)
    EVT_CLOSE(wxGenericMDIParentFrame::OnClose)
#if wxUSE_MENUS
    EVT_MENU(wxID_ANY, wxGenericMDIParentFrame::OnWindowMenu)
#endif
wxEND_EVENT_TABLE()

void wxGenericMDIParentFrame::Init()
{
#if wxUSE_MENUS
    m_pMyMenuBar = NULL;
#endif }

wxGenericMDIParentFrame::~wxGenericMDIParentFrame()
{
        wxDELETE(m_clientWindow);

#if wxUSE_MENUS
    wxDELETE(m_pMyMenuBar);

    RemoveWindowMenu(GetMenuBar());
#endif }

bool wxGenericMDIParentFrame::Create(wxWindow *parent,
                              wxWindowID id,
                              const wxString& title,
                              const wxPoint& pos,
                              const wxSize& size,
                              long style,
                              const wxString& name)
{
            if ( !(style & wxFRAME_NO_WINDOW_MENU) )
    {
#if wxUSE_MENUS
        m_windowMenu = new wxMenu;

        m_windowMenu->Append(wxWINDOWCLOSE,    _("Cl&ose"));
        m_windowMenu->Append(wxWINDOWCLOSEALL, _("Close All"));
        m_windowMenu->AppendSeparator();
        m_windowMenu->Append(wxWINDOWNEXT,     _("&Next"));
        m_windowMenu->Append(wxWINDOWPREV,     _("&Previous"));
#endif     }

            style &= ~(wxHSCROLL | wxVSCROLL);

    if ( !wxFrame::Create( parent, id, title, pos, size, style, name ) )
        return false;

    wxGenericMDIClientWindow * const client = OnCreateGenericClient();
    if ( !client->CreateGenericClient(this) )
        return false;

    m_clientWindow = client;

    return true;
}

wxGenericMDIClientWindow *wxGenericMDIParentFrame::OnCreateGenericClient()
{
    return new wxGenericMDIClientWindow;
}

bool wxGenericMDIParentFrame::CloseAll()
{
    wxGenericMDIClientWindow * const client = GetGenericClientWindow();
    if ( !client )
        return true; 
    wxBookCtrlBase * const book = client->GetBookCtrl();
    while ( book->GetPageCount() )
    {
        wxGenericMDIChildFrame * const child = client->GetChild(0);
        if ( !child->Close() )
        {
                        return false;
        }
    }

    return true;
}

#if wxUSE_MENUS
void wxGenericMDIParentFrame::SetWindowMenu(wxMenu* pMenu)
{
        wxMenuBar *pMenuBar = GetMenuBar();

    if (m_windowMenu)
    {
        RemoveWindowMenu(pMenuBar);

        wxDELETE(m_windowMenu);
    }

    if (pMenu)
    {
        m_windowMenu = pMenu;

        AddWindowMenu(pMenuBar);
    }
}

void wxGenericMDIParentFrame::SetMenuBar(wxMenuBar *pMenuBar)
{
        RemoveWindowMenu(GetMenuBar());
        AddWindowMenu(pMenuBar);

    wxFrame::SetMenuBar(pMenuBar);
}
#endif 
void wxGenericMDIParentFrame::WXSetChildMenuBar(wxGenericMDIChildFrame *pChild)
{
#if wxUSE_MENUS
    if (pChild  == NULL)
    {
                SetMenuBar(m_pMyMenuBar);

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

wxGenericMDIClientWindow *
wxGenericMDIParentFrame::GetGenericClientWindow() const
{
    return static_cast<wxGenericMDIClientWindow *>(m_clientWindow);
}

wxBookCtrlBase *wxGenericMDIParentFrame::GetBookCtrl() const
{
    wxGenericMDIClientWindow * const client = GetGenericClientWindow();
    return client ? client->GetBookCtrl() : NULL;
}

void wxGenericMDIParentFrame::AdvanceActive(bool forward)
{
    wxBookCtrlBase * const book = GetBookCtrl();
    if ( book )
        book->AdvanceSelection(forward);
}

void wxGenericMDIParentFrame::WXUpdateChildTitle(wxGenericMDIChildFrame *child)
{
    wxGenericMDIClientWindow * const client = GetGenericClientWindow();

    const int pos = client->FindChild(child);
    if ( pos == wxNOT_FOUND )
        return;

    client->GetBookCtrl()->SetPageText(pos, child->GetTitle());
}

void wxGenericMDIParentFrame::WXActivateChild(wxGenericMDIChildFrame *child)
{
    wxGenericMDIClientWindow * const client = GetGenericClientWindow();

    const int pos = client->FindChild(child);
    if ( pos == wxNOT_FOUND )
        return;

    client->GetBookCtrl()->SetSelection(pos);
}

void wxGenericMDIParentFrame::WXRemoveChild(wxGenericMDIChildFrame *child)
{
    const bool removingActive = WXIsActiveChild(child);
    if ( removingActive )
    {
        SetActiveChild(NULL);
        WXSetChildMenuBar(NULL);
    }

    wxGenericMDIClientWindow * const client = GetGenericClientWindow();
    wxCHECK_RET( client, "should have client window" );

    wxBookCtrlBase * const book = client->GetBookCtrl();

        int pos = client->FindChild(child);
    if ( pos != wxNOT_FOUND )
    {
        if ( book->RemovePage(pos) )
            book->Refresh();
    }

    if ( removingActive )
    {
                const size_t count = book->GetPageCount();
        if ( count > (size_t)pos )
        {
            book->SetSelection(pos);
        }
        else
        {
            if ( count > 0 )
                book->SetSelection(count - 1);
        }
    }
}

bool
wxGenericMDIParentFrame::WXIsActiveChild(wxGenericMDIChildFrame *child) const
{
    return static_cast<wxMDIChildFrameBase *>(GetActiveChild()) == child;
}

#if wxUSE_MENUS
void wxGenericMDIParentFrame::RemoveWindowMenu(wxMenuBar *pMenuBar)
{
    if (pMenuBar && m_windowMenu)
    {
                int pos = pMenuBar->FindMenu(_("&Window"));
        if (pos != wxNOT_FOUND)
        {
            wxASSERT(m_windowMenu == pMenuBar->GetMenu(pos));             pMenuBar->Remove(pos);
        }
    }
}

void wxGenericMDIParentFrame::AddWindowMenu(wxMenuBar *pMenuBar)
{
    if (pMenuBar && m_windowMenu)
    {
        int pos = pMenuBar->FindMenu(wxGetStockLabel(wxID_HELP,false));
        if (pos == wxNOT_FOUND)
        {
            pMenuBar->Append(m_windowMenu, _("&Window"));
        }
        else
        {
            pMenuBar->Insert(pos, m_windowMenu, _("&Window"));
        }
    }
}

void wxGenericMDIParentFrame::OnWindowMenu(wxCommandEvent &event)
{
    switch ( event.GetId() )
    {
        case wxWINDOWCLOSE:
            if ( m_currentChild )
                m_currentChild->Close();
            break;

        case wxWINDOWCLOSEALL:
            CloseAll();
            break;

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
#endif 
void wxGenericMDIParentFrame::OnClose(wxCloseEvent& event)
{
    if ( !CloseAll() )
        event.Veto();
    else
        event.Skip();
}

bool wxGenericMDIParentFrame::ProcessEvent(wxEvent& event)
{
    if ( m_currentChild )
    {
                        const wxEventType eventType = event.GetEventType();
        if ( eventType == wxEVT_MENU ||
             eventType == wxEVT_UPDATE_UI )
        {
                                                m_childHandler = m_currentChild;
            wxON_BLOCK_EXIT_NULL(m_childHandler);

            if ( m_currentChild->ProcessWindowEvent(event) )
                return true;
        }
    }

    return wxMDIParentFrameBase::ProcessEvent(event);
}


wxIMPLEMENT_DYNAMIC_CLASS(wxGenericMDIChildFrame, wxFrame);

wxBEGIN_EVENT_TABLE(wxGenericMDIChildFrame, wxFrame)
    EVT_MENU_HIGHLIGHT_ALL(wxGenericMDIChildFrame::OnMenuHighlight)

    EVT_CLOSE(wxGenericMDIChildFrame::OnClose)
wxEND_EVENT_TABLE()

void wxGenericMDIChildFrame::Init()
{
#if wxUSE_MENUS
    m_pMenuBar = NULL;
#endif 
#if !wxUSE_GENERIC_MDI_AS_NATIVE
    m_mdiParentGeneric = NULL;
#endif
}

wxGenericMDIChildFrame::~wxGenericMDIChildFrame()
{
    wxGenericMDIParentFrame * const parent = GetGenericMDIParent();

            if ( parent )
        parent->WXRemoveChild(this);

#if wxUSE_MENUS
    delete m_pMenuBar;
#endif }

bool wxGenericMDIChildFrame::Create(wxGenericMDIParentFrame *parent,
                                    wxWindowID id,
                                    const wxString& title,
                                    const wxPoint& WXUNUSED(pos),
                                    const wxSize& size,
                                    long WXUNUSED(style),
                                    const wxString& name)
{
        #if wxUSE_GENERIC_MDI_AS_NATIVE
    m_mdiParent = parent;
#else         m_mdiParentGeneric = parent;
#endif

    wxBookCtrlBase * const book = parent->GetBookCtrl();

    wxASSERT_MSG( book, "Missing MDI client window." );

            if ( !wxWindow::Create(book, id, wxDefaultPosition, size, 0, name) )
        return false;

    m_title = title;
    book->AddPage(this, title, true);

    return true;
}

#if wxUSE_MENUS
void wxGenericMDIChildFrame::SetMenuBar( wxMenuBar *menu_bar )
{
    wxMenuBar *pOldMenuBar = m_pMenuBar;
    m_pMenuBar = menu_bar;

    if (m_pMenuBar)
    {
        wxGenericMDIParentFrame *parent = GetGenericMDIParent();

        if ( parent )
        {
            m_pMenuBar->SetParent(parent);

            if ( parent->WXIsActiveChild(this) )
            {
                                if (pOldMenuBar)
                    parent->WXSetChildMenuBar(NULL);
                parent->WXSetChildMenuBar(this);
            }
        }
    }
}

wxMenuBar *wxGenericMDIChildFrame::GetMenuBar() const
{
    return m_pMenuBar;
}
#endif 
void wxGenericMDIChildFrame::SetTitle(const wxString& title)
{
    m_title = title;

    wxGenericMDIParentFrame * const parent = GetGenericMDIParent();
    if ( parent )
        parent->WXUpdateChildTitle(this);
    }

void wxGenericMDIChildFrame::Activate()
{
    wxGenericMDIParentFrame * const parent = GetGenericMDIParent();

    wxCHECK_RET( parent, "can't activate MDI child without parent" );
    parent->WXActivateChild(this);
}

void wxGenericMDIChildFrame::OnMenuHighlight(wxMenuEvent& event)
{
    wxGenericMDIParentFrame * const parent = GetGenericMDIParent();
    if ( parent)
    {
                        parent->OnMenuHighlight(event);
    }
}

void wxGenericMDIChildFrame::OnClose(wxCloseEvent& WXUNUSED(event))
{
        delete this;
}

bool wxGenericMDIChildFrame::TryAfter(wxEvent& event)
{
            wxGenericMDIParentFrame * const parent = GetGenericMDIParent();
    if ( parent && parent->WXIsInsideChildHandler(this) )
        return false;

    return wxTDIChildFrame::TryAfter(event);
}


wxIMPLEMENT_DYNAMIC_CLASS(wxGenericMDIClientWindow, wxWindow);

bool
wxGenericMDIClientWindow::CreateGenericClient(wxWindow *parent)
{
    if ( !wxWindow::Create(parent, wxID_ANY) )
        return false;

    m_notebook = new wxNotebook(this, wxID_ANY);
    m_notebook->Connect
                (
                    wxEVT_NOTEBOOK_PAGE_CHANGED,
                    wxNotebookEventHandler(
                        wxGenericMDIClientWindow::OnPageChanged),
                    NULL,
                    this
                );

        Connect(wxEVT_SIZE, wxSizeEventHandler(wxGenericMDIClientWindow::OnSize));

    return true;
}

wxBookCtrlBase *wxGenericMDIClientWindow::GetBookCtrl() const
{
    return m_notebook;
}

wxGenericMDIChildFrame *wxGenericMDIClientWindow::GetChild(size_t pos) const
{
    return static_cast<wxGenericMDIChildFrame *>(GetBookCtrl()->GetPage(pos));
}

int wxGenericMDIClientWindow::FindChild(wxGenericMDIChildFrame *child) const
{
    wxBookCtrlBase * const book = GetBookCtrl();
    const size_t count = book->GetPageCount();
    for ( size_t pos = 0; pos < count; pos++ )
    {
        if ( book->GetPage(pos) == child )
            return pos;
    }

    return wxNOT_FOUND;
}

void wxGenericMDIClientWindow::PageChanged(int oldSelection, int newSelection)
{
        if (oldSelection == newSelection)
        return;

        if (newSelection != -1)
    {
        wxGenericMDIChildFrame * const child = GetChild(newSelection);

        if ( child->GetGenericMDIParent()->WXIsActiveChild(child) )
            return;
    }

        if (oldSelection != -1)
    {
        wxGenericMDIChildFrame * const oldChild = GetChild(oldSelection);
        if (oldChild)
        {
            wxActivateEvent event(wxEVT_ACTIVATE, false, oldChild->GetId());
            event.SetEventObject( oldChild );
            oldChild->GetEventHandler()->ProcessEvent(event);
        }
    }

        if (newSelection != -1)
    {
        wxGenericMDIChildFrame * const activeChild = GetChild(newSelection);
        if ( activeChild )
        {
            wxActivateEvent event(wxEVT_ACTIVATE, true, activeChild->GetId());
            event.SetEventObject( activeChild );
            activeChild->GetEventHandler()->ProcessEvent(event);

            wxGenericMDIParentFrame * const
                parent = activeChild->GetGenericMDIParent();

            if ( parent )
            {
                                                                                parent->SetActiveChild(
                    reinterpret_cast<wxMDIChildFrame *>(activeChild));
                parent->WXSetChildMenuBar(activeChild);
            }
        }
    }
}

void wxGenericMDIClientWindow::OnPageChanged(wxBookCtrlEvent& event)
{
    PageChanged(event.GetOldSelection(), event.GetSelection());

    event.Skip();
}

void wxGenericMDIClientWindow::OnSize(wxSizeEvent& WXUNUSED(event))
{
    m_notebook->SetSize(GetClientSize());
}

#endif 
