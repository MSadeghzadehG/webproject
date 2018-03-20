


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/frame.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/menu.h"
    #include "wx/menuitem.h"
    #include "wx/dcclient.h"
    #include "wx/toolbar.h"
    #include "wx/statusbr.h"
#endif 
extern WXDLLEXPORT_DATA(const char) wxFrameNameStr[] = "frame";
extern WXDLLEXPORT_DATA(const char) wxStatusLineNameStr[] = "status_line";


#if wxUSE_MENUS

#if wxUSE_STATUSBAR
wxBEGIN_EVENT_TABLE(wxFrameBase, wxTopLevelWindow)
    EVT_MENU_OPEN(wxFrameBase::OnMenuOpen)
    EVT_MENU_CLOSE(wxFrameBase::OnMenuClose)

    EVT_MENU_HIGHLIGHT_ALL(wxFrameBase::OnMenuHighlight)
wxEND_EVENT_TABLE()
#endif 

bool wxFrameBase::ShouldUpdateMenuFromIdle()
{
                    #ifdef __WXGTK20__
    if ( wxApp::GTKIsUsingGlobalMenu() )
        return true;
#endif 
    return wxUSE_IDLEMENUUPDATES != 0;
}

#endif 


wxDEFINE_FLAGS( wxFrameStyle )
wxBEGIN_FLAGS( wxFrameStyle )
wxFLAGS_MEMBER(wxBORDER_SIMPLE)
wxFLAGS_MEMBER(wxBORDER_SUNKEN)
wxFLAGS_MEMBER(wxBORDER_DOUBLE)
wxFLAGS_MEMBER(wxBORDER_RAISED)
wxFLAGS_MEMBER(wxBORDER_STATIC)
wxFLAGS_MEMBER(wxBORDER_NONE)

wxFLAGS_MEMBER(wxSIMPLE_BORDER)
wxFLAGS_MEMBER(wxSUNKEN_BORDER)
wxFLAGS_MEMBER(wxDOUBLE_BORDER)
wxFLAGS_MEMBER(wxRAISED_BORDER)
wxFLAGS_MEMBER(wxSTATIC_BORDER)
wxFLAGS_MEMBER(wxBORDER)

wxFLAGS_MEMBER(wxTAB_TRAVERSAL)
wxFLAGS_MEMBER(wxCLIP_CHILDREN)
wxFLAGS_MEMBER(wxTRANSPARENT_WINDOW)
wxFLAGS_MEMBER(wxWANTS_CHARS)
wxFLAGS_MEMBER(wxFULL_REPAINT_ON_RESIZE)
wxFLAGS_MEMBER(wxALWAYS_SHOW_SB )
wxFLAGS_MEMBER(wxVSCROLL)
wxFLAGS_MEMBER(wxHSCROLL)

wxFLAGS_MEMBER(wxSTAY_ON_TOP)
wxFLAGS_MEMBER(wxCAPTION)
wxFLAGS_MEMBER(wxSYSTEM_MENU)
wxFLAGS_MEMBER(wxRESIZE_BORDER)
wxFLAGS_MEMBER(wxCLOSE_BOX)
wxFLAGS_MEMBER(wxMAXIMIZE_BOX)
wxFLAGS_MEMBER(wxMINIMIZE_BOX)

wxFLAGS_MEMBER(wxFRAME_TOOL_WINDOW)
wxFLAGS_MEMBER(wxFRAME_FLOAT_ON_PARENT)

wxFLAGS_MEMBER(wxFRAME_SHAPED)
wxEND_FLAGS( wxFrameStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxFrame, wxTopLevelWindow, "wx/frame.h");

wxBEGIN_PROPERTIES_TABLE(wxFrame)
wxEVENT_PROPERTY( Menu, wxEVT_MENU, wxCommandEvent)

wxPROPERTY( Title,wxString, SetTitle, GetTitle, wxString(), 0 , \
           wxT("Helpstring"), wxT("group"))
wxPROPERTY_FLAGS( WindowStyle, wxFrameStyle, long, SetWindowStyleFlag, \
                 GetWindowStyleFlag, wxEMPTY_PARAMETER_VALUE, 0 , \
                 wxT("Helpstring"), wxT("group")) #if wxUSE_MENUS
wxPROPERTY( MenuBar, wxMenuBar *, SetMenuBar, GetMenuBar, wxEMPTY_PARAMETER_VALUE, \
           0 , wxT("Helpstring"), wxT("group"))
#endif
wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxFrame)

wxCONSTRUCTOR_6( wxFrame, wxWindow*, Parent, wxWindowID, Id, wxString, Title, \
                wxPoint, Position, wxSize, Size, long, WindowStyle)


wxFrameBase::wxFrameBase()
{
#if wxUSE_MENUS
    m_frameMenuBar = NULL;
#endif 
#if wxUSE_TOOLBAR
    m_frameToolBar = NULL;
#endif 
#if wxUSE_STATUSBAR
    m_frameStatusBar = NULL;
#endif 
    m_statusBarPane = 0;
}

wxFrameBase::~wxFrameBase()
{
    SendDestroyEvent();

    DeleteAllBars();
}

wxFrame *wxFrameBase::New(wxWindow *parent,
                          wxWindowID id,
                          const wxString& title,
                          const wxPoint& pos,
                          const wxSize& size,
                          long style,
                          const wxString& name)
{
    return new wxFrame(parent, id, title, pos, size, style, name);
}

void wxFrameBase::DeleteAllBars()
{
#if wxUSE_MENUS
    wxDELETE(m_frameMenuBar);
#endif 
#if wxUSE_STATUSBAR
    wxDELETE(m_frameStatusBar);
#endif 
#if wxUSE_TOOLBAR
    wxDELETE(m_frameToolBar);
#endif }

bool wxFrameBase::IsOneOfBars(const wxWindow *win) const
{
#if wxUSE_MENUS
    if ( win == GetMenuBar() )
        return true;
#endif 
#if wxUSE_STATUSBAR
    if ( win == GetStatusBar() )
        return true;
#endif 
#if wxUSE_TOOLBAR
    if ( win == GetToolBar() )
        return true;
#endif 
    wxUnusedVar(win);

    return false;
}


wxPoint wxFrameBase::GetClientAreaOrigin() const
{
    wxPoint pt = wxTopLevelWindow::GetClientAreaOrigin();

#if wxUSE_TOOLBAR && !defined(__WXUNIVERSAL__)
    wxToolBar *toolbar = GetToolBar();
    if ( toolbar && toolbar->IsShown() )
    {
        int w, h;
        toolbar->GetSize(&w, &h);

        if ( toolbar->GetWindowStyleFlag() & wxTB_VERTICAL )
        {
            pt.x += w;
        }
        else
        {
            pt.y += h;
        }
    }
#endif 
    return pt;
}


#if wxUSE_MENUS

bool wxFrameBase::ProcessCommand(int id)
{
    wxMenuItem* const item = FindItemInMenuBar(id);
    if ( !item )
        return false;

    return ProcessCommand(item);
}

bool wxFrameBase::ProcessCommand(wxMenuItem *item)
{
    wxCHECK_MSG( item, false, wxS("Menu item can't be NULL") );

    if (!item->IsEnabled())
        return true;

    if ((item->GetKind() == wxITEM_RADIO) && item->IsChecked() )
        return true;

    int checked;
    if (item->IsCheckable())
    {
        item->Toggle();

                checked = item->IsChecked();
    }
    else     {
        checked = -1;
    }

    wxMenu* const menu = item->GetMenu();
    wxCHECK_MSG( menu, false, wxS("Menu item should be attached to a menu") );

    return menu->SendEvent(item->GetId(), checked);
}

#endif 
void wxFrameBase::UpdateWindowUI(long flags)
{
    wxWindowBase::UpdateWindowUI(flags);

#if wxUSE_TOOLBAR
    if (GetToolBar())
        GetToolBar()->UpdateWindowUI(flags);
#endif

#if wxUSE_MENUS
    if (GetMenuBar())
    {
                                if ( !(flags & wxUPDATE_UI_FROMIDLE) || ShouldUpdateMenuFromIdle() )
            DoMenuUpdates();
    }
#endif }


#if wxUSE_MENUS && wxUSE_STATUSBAR

void wxFrameBase::OnMenuHighlight(wxMenuEvent& event)
{
    event.Skip();

#if wxUSE_STATUSBAR
    (void)ShowMenuHelp(event.GetMenuId());
#endif }

void wxFrameBase::OnMenuOpen(wxMenuEvent& event)
{
    event.Skip();

    if ( !ShouldUpdateMenuFromIdle() )
    {
                DoMenuUpdates(event.GetMenu());
    }
}

void wxFrameBase::OnMenuClose(wxMenuEvent& event)
{
    event.Skip();

    DoGiveHelp(wxEmptyString, false);
}

#endif 
void wxFrameBase::OnInternalIdle()
{
    wxTopLevelWindow::OnInternalIdle();

#if wxUSE_MENUS
    if ( ShouldUpdateMenuFromIdle() && wxUpdateUIEvent::CanUpdate(this) )
        DoMenuUpdates();
#endif
}


#if wxUSE_STATUSBAR

wxStatusBar* wxFrameBase::CreateStatusBar(int number,
                                          long style,
                                          wxWindowID id,
                                          const wxString& name)
{
            wxCHECK_MSG( !m_frameStatusBar, NULL,
                 wxT("recreating status bar in wxFrame") );

    SetStatusBar(OnCreateStatusBar(number, style, id, name));

    return m_frameStatusBar;
}

wxStatusBar *wxFrameBase::OnCreateStatusBar(int number,
                                            long style,
                                            wxWindowID id,
                                            const wxString& name)
{
    wxStatusBar *statusBar = new wxStatusBar(this, id, style, name);

    statusBar->SetFieldsCount(number);

    return statusBar;
}

void wxFrameBase::SetStatusText(const wxString& text, int number)
{
    wxCHECK_RET( m_frameStatusBar != NULL, wxT("no statusbar to set text for") );

    m_frameStatusBar->SetStatusText(text, number);
}

void wxFrameBase::SetStatusWidths(int n, const int widths_field[] )
{
    wxCHECK_RET( m_frameStatusBar != NULL, wxT("no statusbar to set widths for") );

    m_frameStatusBar->SetStatusWidths(n, widths_field);

    PositionStatusBar();
}

void wxFrameBase::PushStatusText(const wxString& text, int number)
{
    wxCHECK_RET( m_frameStatusBar != NULL, wxT("no statusbar to set text for") );

    m_frameStatusBar->PushStatusText(text, number);
}

void wxFrameBase::PopStatusText(int number)
{
    wxCHECK_RET( m_frameStatusBar != NULL, wxT("no statusbar to set text for") );

    m_frameStatusBar->PopStatusText(number);
}

bool wxFrameBase::ShowMenuHelp(int menuId)
{
#if wxUSE_MENUS
                wxString helpString;
    if ( menuId != wxID_SEPARATOR && menuId != wxID_NONE )
    {
        const wxMenuItem * const item = FindItemInMenuBar(menuId);
        if ( item && !item->IsSeparator() )
            helpString = item->GetHelp();

                    }

    DoGiveHelp(helpString, true);

    return !helpString.empty();
#else     return false;
#endif }

void wxFrameBase::SetStatusBar(wxStatusBar *statBar)
{
    bool hadBar = m_frameStatusBar != NULL;
    m_frameStatusBar = statBar;

    if ( (m_frameStatusBar != NULL) != hadBar )
    {
        PositionStatusBar();

        DoLayout();
    }
}

#endif 
#if wxUSE_MENUS || wxUSE_TOOLBAR
void wxFrameBase::DoGiveHelp(const wxString& help, bool show)
{
#if wxUSE_STATUSBAR
    if ( m_statusBarPane < 0 )
    {
                return;
    }

    wxStatusBar *statbar = GetStatusBar();
    if ( !statbar )
        return;

    wxString text;
    if ( show )
    {
                                                                                                if ( m_oldStatusText.empty() )
        {
            m_oldStatusText = statbar->GetStatusText(m_statusBarPane);
            if ( m_oldStatusText.empty() )
            {
                                m_oldStatusText += wxT('\0');
            }
        }

        m_lastHelpShown =
        text = help;
    }
    else     {
                wxString lastHelpShown;
        lastHelpShown.swap(m_lastHelpShown);

                        text.swap(m_oldStatusText);

        if ( statbar->GetStatusText(m_statusBarPane) != lastHelpShown )
        {
                                                            return;
        }
    }

    statbar->SetStatusText(text, m_statusBarPane);
#else
    wxUnusedVar(help);
    wxUnusedVar(show);
#endif }
#endif 


#if wxUSE_TOOLBAR

wxToolBar* wxFrameBase::CreateToolBar(long style,
                                      wxWindowID id,
                                      const wxString& name)
{
            wxCHECK_MSG( !m_frameToolBar, NULL,
                 wxT("recreating toolbar in wxFrame") );

    if ( style == -1 )
    {
                                                                style = wxTB_DEFAULT_STYLE;
    }

    SetToolBar(OnCreateToolBar(style, id, name));

    return m_frameToolBar;
}

wxToolBar* wxFrameBase::OnCreateToolBar(long style,
                                        wxWindowID id,
                                        const wxString& name)
{
    return new wxToolBar(this, id,
                         wxDefaultPosition, wxDefaultSize,
                         style, name);
}

void wxFrameBase::SetToolBar(wxToolBar *toolbar)
{
    if ( (toolbar != NULL) != (m_frameToolBar != NULL) )
    {
                        
        if ( toolbar )
        {
                                    m_frameToolBar = toolbar;
            PositionToolBar();
        }
                        

                                        if ( m_frameToolBar )
            m_frameToolBar->Hide();

        DoLayout();

        if ( m_frameToolBar )
            m_frameToolBar->Show();
    }

            m_frameToolBar = toolbar;
}

#endif 

#if wxUSE_MENUS

void wxFrameBase::DoMenuUpdates(wxMenu* menu)
{
    if (menu)
    {
        menu->UpdateUI();
    }
    else
    {
        wxMenuBar* bar = GetMenuBar();
        if (bar != NULL)
            bar->UpdateMenus();
    }
}

void wxFrameBase::DetachMenuBar()
{
    if ( m_frameMenuBar )
    {
        m_frameMenuBar->Detach();
        m_frameMenuBar = NULL;
    }
}

void wxFrameBase::AttachMenuBar(wxMenuBar *menubar)
{
    if ( menubar )
    {
        menubar->Attach((wxFrame *)this);
        m_frameMenuBar = menubar;
    }
}

void wxFrameBase::SetMenuBar(wxMenuBar *menubar)
{
    if ( menubar == GetMenuBar() )
    {
                return;
    }

    DetachMenuBar();

    this->AttachMenuBar(menubar);
}

wxMenuItem *wxFrameBase::FindItemInMenuBar(int menuId) const
{
    const wxMenuBar * const menuBar = GetMenuBar();

    return menuBar ? menuBar->FindItem(menuId) : NULL;
}

#endif 