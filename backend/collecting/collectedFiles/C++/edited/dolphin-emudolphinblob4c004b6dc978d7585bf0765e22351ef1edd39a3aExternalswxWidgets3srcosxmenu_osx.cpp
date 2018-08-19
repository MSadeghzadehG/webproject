


#include "wx/wxprec.h"

#if wxUSE_MENUS

#include "wx/menu.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/app.h"
    #include "wx/utils.h"
    #include "wx/frame.h"
    #include "wx/dialog.h"
    #include "wx/menuitem.h"
#endif

#include "wx/osx/private.h"
#include "wx/scopedptr.h"

#include <string.h>

wxIMPLEMENT_ABSTRACT_CLASS(wxMenuImpl, wxObject);

wxMenuImpl::~wxMenuImpl()
{
}

static const int idMenuTitle = -3;

wxScopedPtr<wxMenu> gs_emptyMenuBar;




void wxMenu::Init()
{
    m_doBreak = false;
    m_allowRearrange = true;
    m_noEventsMode = false;

    m_peer = wxMenuImpl::Create( this, wxStripMenuCodes(m_title) );


        if ( !m_title.empty() )
    {
        Append(idMenuTitle, m_title) ;
        AppendSeparator() ;
    }
}

wxMenu::~wxMenu()
{
    delete m_peer;
}

WXHMENU wxMenu::GetHMenu() const
{
    if ( m_peer )
        return m_peer->GetHMenu();
    return NULL;
}

void wxMenu::Break()
{
    }

void wxMenu::SetAllowRearrange( bool allow )
{
    m_allowRearrange = allow;
}

void wxMenu::SetNoEventsMode( bool noEvents )
{
    m_noEventsMode = noEvents;
}

bool wxMenu::DoInsertOrAppend(wxMenuItem *item, size_t pos)
{
    wxASSERT_MSG( item != NULL, wxT("can't append NULL item to the menu") );
    GetPeer()->InsertOrAppend( item, pos );

    bool check = false;

    if ( item->IsSeparator() )
    {
            }
    else
    {
        wxMenu *pSubMenu = item->GetSubMenu() ;
        if ( pSubMenu != NULL )
        {
            wxASSERT_MSG( pSubMenu->GetHMenu() != NULL , wxT("invalid submenu added"));
            pSubMenu->m_menuParent = this ;

            pSubMenu->DoRearrange();
        }
        else if ( item->IsRadio() )
        {
                                                wxMenuItemList& items = GetMenuItems();

            size_t const
                posItem = pos == (size_t)-1 ? items.GetCount() - 1 : pos;

            wxMenuItemList::compatibility_iterator node = items.Item(posItem);
            wxCHECK_MSG( node, false, wxS("New item must have been inserted") );

            bool foundGroup = false;
            if ( node->GetPrevious() )
            {
                wxMenuItem* const prev = node->GetPrevious()->GetData();

                if ( prev->IsRadio() )
                {
                                                                                                    const int groupStart = prev->IsRadioGroupStart()
                                            ? posItem - 1
                                            : prev->GetRadioGroupStart();
                    item->SetRadioGroupStart(groupStart);

                                                            wxMenuItem* const first = items.Item(groupStart)->GetData();
                    first->SetRadioGroupEnd(first->GetRadioGroupEnd() + 1);

                    foundGroup = true;
                }
            }

            if ( !foundGroup && node->GetNext() )
            {
                wxMenuItem* const next = node->GetNext()->GetData();

                if ( next->IsRadio() )
                {
                                                            wxASSERT_MSG( next->IsRadioGroupStart(),
                                  wxS("Where is the start of this group?") );

                                                            item->SetAsRadioGroupStart();
                    item->SetRadioGroupEnd(next->GetRadioGroupEnd() + 1);

                                        next->SetAsRadioGroupStart(false);

                    foundGroup = true;
                }
            }

            if ( !foundGroup )
            {
                                item->SetAsRadioGroupStart();
                item->SetRadioGroupEnd(posItem);

                                check = true;
            }
        }
        else
        {
            if ( item->GetId() == idMenuTitle )
                item->GetMenu()->Enable( idMenuTitle, false );
        }
    }

            if ( pos < GetMenuItemCount() - 1 )     {
        for ( wxMenuItemList::compatibility_iterator
                node = GetMenuItems().Item(pos + 1);
                node;
                node = node->GetNext() )
        {
            wxMenuItem* const item = node->GetData();
            if ( item->IsRadio() )
            {
                if ( item->IsRadioGroupStart() )
                {
                                                                                item->SetRadioGroupEnd(item->GetRadioGroupEnd() + 1);
                }
                else                 {
                                                            const int groupStart = item->GetRadioGroupStart();
                    if ( (size_t)groupStart > pos )
                        item->SetRadioGroupStart(groupStart + 1);
                }
            }
        }
    }

        if ( IsAttached() && GetMenuBar()->IsAttached() )
        GetMenuBar()->Refresh();

    if ( check )
        item->Check(true);

    return true ;
}

wxMenuItem* wxMenu::DoAppend(wxMenuItem *item)
{
    if (wxMenuBase::DoAppend(item) && DoInsertOrAppend(item) )
        return item;

    return NULL;
}

wxMenuItem* wxMenu::DoInsert(size_t pos, wxMenuItem *item)
{
    if (wxMenuBase::DoInsert(pos, item) && DoInsertOrAppend(item, pos))
        return item;

    return NULL;
}

wxMenuItem *wxMenu::DoRemove(wxMenuItem *item)
{
    if ( item->IsRadio() )
    {
                if ( item->IsRadioGroupStart() )
        {
                                    const int endGroup = item->GetRadioGroupEnd();

            wxMenuItemList::compatibility_iterator
                node = GetMenuItems().Item(endGroup);
            wxASSERT_MSG( node, wxS("Should have valid radio group end") );

            while ( node->GetData() != item )
            {
                const wxMenuItemList::compatibility_iterator
                    prevNode = node->GetPrevious();
                wxMenuItem* const prevItem = prevNode->GetData();
                if ( prevItem == item )
                {
                    prevItem->SetAsRadioGroupStart();
                    prevItem->SetRadioGroupEnd(endGroup);
                    break;
                }

                node = prevNode;
            }
        }
    }


    GetPeer()->Remove( item );
        return wxMenuBase::DoRemove(item);
}

void wxMenu::SetTitle(const wxString& label)
{
    m_title = label ;
    GetPeer()->SetTitle( wxStripMenuCodes( label ) );
}

bool wxMenu::ProcessCommand(wxCommandEvent & event)
{
    bool processed = false;

        if (  GetEventHandler())
        processed = GetEventHandler()->SafelyProcessEvent(event);

            wxWindow *win = GetWindow();
    if ( !processed && win )
        processed = win->HandleWindowEvent(event);

    return processed;
}


void wxMenu::DoRearrange()
{
    if ( !AllowRearrange() )
        return;

    wxMenuItem* previousItem = NULL ;
    size_t pos ;
    wxMenuItemList::compatibility_iterator node;
    wxMenuItem *item;

    for (pos = 0, node = GetMenuItems().GetFirst(); node; node = node->GetNext(), pos++)
    {
        item = (wxMenuItem *)node->GetData();
        wxMenu* subMenu = item->GetSubMenu() ;
        if (subMenu)
        {
                    }
        else         {
                                                if ( item->GetId() == wxApp::s_macAboutMenuItemId ||
                    item->GetId() == wxApp::s_macPreferencesMenuItemId ||
                    item->GetId() == wxApp::s_macExitMenuItemId )

            {
                item->GetPeer()->Hide( true );

                                                                                wxMenuItemList::compatibility_iterator nextNode = node->GetNext();
                wxMenuItem *next = nextNode ? nextNode->GetData() : NULL;

                wxMenuItem *sepToHide = 0;
                if ( !previousItem && next && next->IsSeparator() )
                {
                                                            wxASSERT_MSG( pos == 0, wxT("should be the menu start") );
                    sepToHide = next;
                }
                else if ( GetMenuItems().GetCount() == pos + 1 &&
                            previousItem != NULL &&
                                previousItem->IsSeparator() )
                {
                                        sepToHide = previousItem;
                }
                else if ( previousItem && previousItem->IsSeparator() &&
                            next && next->IsSeparator() )
                {
                                        sepToHide = next;
                }

                if ( sepToHide )
                {
                                        sepToHide->GetPeer()->Hide( true );
                }
            }
        }

        previousItem = item ;
    }
}


bool wxMenu::HandleCommandUpdateStatus( wxMenuItem* item, wxWindow* senderWindow )
{
    int menuid = item ? item->GetId() : 0;
    wxUpdateUIEvent event(menuid);
    event.SetEventObject( this );

    bool processed = false;

        {
        wxEvtHandler *handler = GetEventHandler();
        if ( handler )
            processed = handler->ProcessEvent(event);
    }

            if ( !processed )
    {
        wxWindow *win = GetWindow();
        if ( win )
            processed = win->HandleWindowEvent(event);
    }

    if ( !processed && senderWindow != NULL)
    {
        processed = senderWindow->HandleWindowEvent(event);
    }

    if ( processed )
    {
                if (event.GetSetText())
            SetLabel(menuid, event.GetText());
        if (event.GetSetChecked())
            Check(menuid, event.GetChecked());
        if (event.GetSetEnabled())
            Enable(menuid, event.GetEnabled());
    }

    return processed;
}

bool wxMenu::HandleCommandProcess( wxMenuItem* item, wxWindow* senderWindow )
{
    int menuid = item ? item->GetId() : 0;
    bool processed = false;
    if (item->IsCheckable())
        item->Check( !item->IsChecked() ) ;

    if ( SendEvent( menuid , item->IsCheckable() ? item->IsChecked() : -1 ) )
        processed = true ;
    else
    {
        if ( senderWindow != NULL )
        {
            wxCommandEvent event(wxEVT_MENU , menuid);
            event.SetEventObject(this);
            event.SetInt(item->IsCheckable() ? item->IsChecked() : -1);

            if ( senderWindow->HandleWindowEvent(event) )
                processed = true ;
        }
    }

    if(!processed && item)
    {
        processed = item->GetPeer()->DoDefault();  
    }
    
    return processed;
}

void wxMenu::HandleMenuItemHighlighted( wxMenuItem* item )
{
    int menuid = item ? item->GetId() : 0;
    wxMenuEvent wxevent(wxEVT_MENU_HIGHLIGHT, menuid, this);
    ProcessMenuEvent(this, wxevent, GetWindow());
}

void wxMenu::DoHandleMenuOpenedOrClosed(wxEventType evtType)
{
        extern wxMenu *wxCurrentPopupMenu;

        int menuid = this == wxCurrentPopupMenu ? wxID_ANY : 0;
    wxMenuEvent wxevent(evtType, menuid, this);
    ProcessMenuEvent(this, wxevent, GetWindow());
}

void wxMenu::HandleMenuOpened()
{
    DoHandleMenuOpenedOrClosed(wxEVT_MENU_OPEN);
}

void wxMenu::HandleMenuClosed()
{
    DoHandleMenuOpenedOrClosed(wxEVT_MENU_CLOSE);
}




wxMenuBar* wxMenuBar::s_macInstalledMenuBar = NULL ;
wxMenuBar* wxMenuBar::s_macCommonMenuBar = NULL ;
bool     wxMenuBar::s_macAutoWindowMenu = true ;
WXHMENU  wxMenuBar::s_macWindowMenuHandle = NULL ;


const int firstMenuPos = 1; 
static wxMenu *CreateAppleMenu()
{
    wxMenu *appleMenu = new wxMenu();
    appleMenu->SetAllowRearrange(false);

                if ( wxApp::s_macAboutMenuItemId != wxID_NONE )
    {
        wxString aboutLabel;
        if ( wxTheApp )
            aboutLabel.Printf(_("About %s"), wxTheApp->GetAppDisplayName());
        else
            aboutLabel = _("About...");
        appleMenu->Append( wxApp::s_macAboutMenuItemId, aboutLabel);
        appleMenu->AppendSeparator();
    }

    if ( wxApp::s_macPreferencesMenuItemId != wxID_NONE )
    {
        appleMenu->Append( wxApp::s_macPreferencesMenuItemId,
                           _("Preferences...") + "\tCtrl+," );
        appleMenu->AppendSeparator();
    }

    appleMenu->Append(wxID_OSX_SERVICES, _("Services"), new wxMenu());
    appleMenu->AppendSeparator();

        wxString hideLabel;
    hideLabel = wxString::Format(_("Hide %s"), wxTheApp ? wxTheApp->GetAppDisplayName() : _("Application"));
    appleMenu->Append( wxID_OSX_HIDE, hideLabel + "\tCtrl+H" );
    appleMenu->Append( wxID_OSX_HIDEOTHERS, _("Hide Others")+"\tAlt+Ctrl+H" );
    appleMenu->Append( wxID_OSX_SHOWALL, _("Show All") );
    appleMenu->AppendSeparator();
    
        wxString quitLabel;
    quitLabel = wxString::Format(_("Quit %s"), wxTheApp ? wxTheApp->GetAppDisplayName() : _("Application"));
    appleMenu->Append( wxApp::s_macExitMenuItemId, quitLabel + "\tCtrl+Q" );

    return appleMenu;
}

void wxMenuBar::Init()
{
    if ( !gs_emptyMenuBar )
    {
        gs_emptyMenuBar.reset( new wxMenu() );
        gs_emptyMenuBar->AppendSubMenu(CreateAppleMenu(), "\x14") ;
    }
    
    m_eventHandler = this;
    m_menuBarFrame = NULL;
    m_rootMenu = new wxMenu();
    m_rootMenu->Attach(this);

    m_appleMenu = CreateAppleMenu();
    m_rootMenu->AppendSubMenu(m_appleMenu, "\x14") ;
}

wxMenuBar::wxMenuBar()
{
    Init();
}

wxMenuBar::wxMenuBar( long WXUNUSED(style) )
{
    Init();
}

wxMenuBar::wxMenuBar(size_t count, wxMenu *menus[], const wxString titles[], long WXUNUSED(style))
{
    Init();

    for ( size_t i = 0; i < count; i++ )
    {
        m_menus.Append(menus[i]);

        menus[i]->Attach(this);
        Append( menus[i], titles[i] );
    }
}

wxMenuBar::~wxMenuBar()
{
    if (s_macCommonMenuBar == this)
        s_macCommonMenuBar = NULL;

    MacUninstallMenuBar();
    wxDELETE( m_rootMenu );
        m_appleMenu = NULL;

            m_menus.clear();
}

void wxMenuBar::Refresh(bool WXUNUSED(eraseBackground), const wxRect *WXUNUSED(rect))
{
    wxCHECK_RET( IsAttached(), wxT("can't refresh unatteched menubar") );
}

void wxMenuBar::MacUninstallMenuBar()
{
  if (s_macInstalledMenuBar == this)
  {
    gs_emptyMenuBar->GetPeer()->MakeRoot();
    s_macInstalledMenuBar = NULL;
  }
}

void wxMenuBar::MacInstallMenuBar()
{
    if ( s_macInstalledMenuBar == this )
        return ;

    m_rootMenu->GetPeer()->MakeRoot();
    
        
    wxMenuItem* appleItem = NULL;
    wxMenuItem* wxItem = NULL;

    int menuid = wxApp::s_macAboutMenuItemId;
    appleItem = m_appleMenu->FindItem(menuid);
    wxItem = FindItem(menuid);
    if ( appleItem != NULL )
    {
        if ( wxItem == NULL )
            appleItem->GetPeer()->Hide();
        else 
            appleItem->SetItemLabel(wxItem->GetItemLabel());
    }
    
    menuid = wxApp::s_macPreferencesMenuItemId;
    appleItem = m_appleMenu->FindItem(menuid);
    wxItem = FindItem(menuid);
    if ( appleItem != NULL )
    {
        if ( wxItem == NULL )
            appleItem->GetPeer()->Hide();
        else 
            appleItem->SetItemLabel(wxItem->GetItemLabel());
    }
    
        
#if 0

        MenuHandle helpMenuHandle ;
    MenuItemIndex firstUserHelpMenuItem ;

    if ( UMAGetHelpMenuDontCreate( &helpMenuHandle , &firstUserHelpMenuItem) == noErr )
    {
        for ( int i = CountMenuItems( helpMenuHandle ) ; i >= firstUserHelpMenuItem ; --i )
            DeleteMenuItem( helpMenuHandle , i ) ;
    }
    else
    {
        helpMenuHandle = NULL ;
    }

    if ( wxApp::s_macPreferencesMenuItemId)
    {
        wxMenuItem *item = FindItem( wxApp::s_macPreferencesMenuItemId , NULL ) ;
        if ( item == NULL || !(item->IsEnabled()) )
            DisableMenuCommand( NULL , kHICommandPreferences ) ;
        else
            EnableMenuCommand( NULL , kHICommandPreferences ) ;
    }

                    if ( wxApp::s_macExitMenuItemId)
    {
        wxMenuItem *item = FindItem( wxApp::s_macExitMenuItemId , NULL ) ;
        if ( item != NULL && !(item->IsEnabled()) )
            DisableMenuCommand( NULL , kHICommandQuit ) ;
        else
            EnableMenuCommand( NULL , kHICommandQuit ) ;
    }

    wxString strippedHelpMenuTitle = wxStripMenuCodes( wxApp::s_macHelpMenuTitleName ) ;
    wxString strippedTranslatedHelpMenuTitle = wxStripMenuCodes( wxString( _("&Help") ) ) ;
    wxMenuList::compatibility_iterator menuIter = m_menus.GetFirst();
    for (size_t i = 0; i < m_menus.GetCount(); i++, menuIter = menuIter->GetNext())
    {
        wxMenuItemList::compatibility_iterator node;
        wxMenuItem *item;
        wxMenu* menu = menuIter->GetData() , *subMenu = NULL ;
        wxString strippedMenuTitle = wxStripMenuCodes(m_titles[i]);

        if ( strippedMenuTitle == wxT("?") || strippedMenuTitle == strippedHelpMenuTitle || strippedMenuTitle == strippedTranslatedHelpMenuTitle )
        {
            for (node = menu->GetMenuItems().GetFirst(); node; node = node->GetNext())
            {
                item = (wxMenuItem *)node->GetData();
                subMenu = item->GetSubMenu() ;
                if (subMenu)
                {
                    UMAAppendMenuItem(mh, wxStripMenuCodes(item->GetText()) , wxFont::GetDefaultEncoding() );
                    MenuItemIndex position = CountMenuItems(mh);
                    ::SetMenuItemHierarchicalMenu(mh, position, MAC_WXHMENU(subMenu->GetHMenu()));
                }
                else
                {
                    if ( item->GetId() != wxApp::s_macAboutMenuItemId )
                    {
                                                                        if ( helpMenuHandle == NULL )
                        {
                            if ( UMAGetHelpMenu( &helpMenuHandle , &firstUserHelpMenuItem) != noErr )
                            {
                                helpMenuHandle = NULL ;
                                break ;
                            }
                        }
                    }

                    if ( item->IsSeparator() )
                    {
                        if ( helpMenuHandle )
                            AppendMenuItemTextWithCFString( helpMenuHandle,
                                CFSTR(""), kMenuItemAttrSeparator, 0,NULL);
                    }
                    else
                    {
                        wxAcceleratorEntry*
                            entry = wxAcceleratorEntry::Create( item->GetItemLabel() ) ;

                        if ( item->GetId() == wxApp::s_macAboutMenuItemId )
                        {
                                                    }
                        else
                        {
                            if ( helpMenuHandle )
                            {
                                UMAAppendMenuItem(helpMenuHandle, wxStripMenuCodes(item->GetItemLabel()) , wxFont::GetDefaultEncoding(), entry);
                                SetMenuItemCommandID( helpMenuHandle , CountMenuItems(helpMenuHandle) , wxIdToMacCommand ( item->GetId() ) ) ;
                                SetMenuItemRefCon( helpMenuHandle , CountMenuItems(helpMenuHandle) , (URefCon) item ) ;
                            }
                        }

                        delete entry ;
                    }
                }
            }
        }

        else if ( ( m_titles[i] == wxT("Window") || m_titles[i] == wxT("&Window") )
                && GetAutoWindowMenu() )
        {
            if ( MacGetWindowMenuHMenu() == NULL )
            {
                CreateStandardWindowMenu( 0 , (MenuHandle*) &s_macWindowMenuHandle ) ;
            }

            MenuRef wm = (MenuRef)MacGetWindowMenuHMenu();
            if ( wm == NULL )
                break;

                        MenuItemIndex winListStart;
            GetIndMenuItemWithCommandID(wm,
                        kHICommandWindowListSeparator, 1, NULL, &winListStart);

                                    OSStatus err = GetIndMenuItemWithCommandID(wm,
                        'WXWM', 1, NULL, NULL);

            if ( err == menuItemNotFoundErr )
            {
                InsertMenuItemTextWithCFString( wm,
                        CFSTR(""), winListStart-1, kMenuItemAttrSeparator, 'WXWM');
            }

            wxInsertMenuItemsInMenu(menu, wm, winListStart);
        }
        else
        {
            UMASetMenuTitle( MAC_WXHMENU(menu->GetHMenu()) , m_titles[i], GetFont().GetEncoding()  ) ;
            menu->MacBeforeDisplay(false) ;

            ::InsertMenu(MAC_WXHMENU(GetMenu(i)->GetHMenu()), 0);
        }
    }

        {
        wxMenu* aboutMenu ;
        wxMenuItem *aboutMenuItem = FindItem(wxApp::s_macAboutMenuItemId , &aboutMenu) ;
        if ( aboutMenuItem )
        {
            wxAcceleratorEntry*
                entry = wxAcceleratorEntry::Create( aboutMenuItem->GetItemLabel() ) ;
            UMASetMenuItemText( GetMenuHandle( kwxMacAppleMenuId ) , 1 , wxStripMenuCodes ( aboutMenuItem->GetItemLabel() ) , wxFont::GetDefaultEncoding() );
            UMAEnableMenuItem( GetMenuHandle( kwxMacAppleMenuId ) , 1 , true );
            SetMenuItemCommandID( GetMenuHandle( kwxMacAppleMenuId ) , 1 , kHICommandAbout ) ;
            SetMenuItemRefCon(GetMenuHandle( kwxMacAppleMenuId ) , 1 , (URefCon)aboutMenuItem ) ;
            UMASetMenuItemShortcut( GetMenuHandle( kwxMacAppleMenuId ) , 1 , entry ) ;
            delete entry;
        }
    }

    if ( GetAutoWindowMenu() )
    {
        if ( MacGetWindowMenuHMenu() == NULL )
            CreateStandardWindowMenu( 0 , (MenuHandle*) &s_macWindowMenuHandle ) ;

        InsertMenu( (MenuHandle) MacGetWindowMenuHMenu() , 0 ) ;
    }

    ::DrawMenuBar() ;
#endif

    s_macInstalledMenuBar = this;
}

void wxMenuBar::EnableTop(size_t pos, bool enable)
{
    wxCHECK_RET( IsAttached(), wxT("doesn't work with unattached menubars") );

    m_rootMenu->FindItemByPosition(pos+firstMenuPos)->Enable(enable);

    Refresh();
}

bool wxMenuBar::IsEnabledTop(size_t pos) const
{
    wxCHECK_MSG( IsAttached(), true,
                 wxT("doesn't work with unattached menubars") );

    wxMenuItem* const item = m_rootMenu->FindItemByPosition(pos+firstMenuPos);
    wxCHECK_MSG( item, false, wxT("invalid menu index") );

    return item->IsEnabled();
}

bool wxMenuBar::Enable(bool enable)
{
    wxCHECK_MSG( IsAttached(), false, wxT("doesn't work with unattached menubars") );

    size_t i;
    for (i = 0; i < GetMenuCount(); i++)
        EnableTop(i, enable);

    return true;
}

void wxMenuBar::SetMenuLabel(size_t pos, const wxString& label)
{
    wxCHECK_RET( pos < GetMenuCount(), wxT("invalid menu index") );

    GetMenu(pos)->SetTitle( label ) ;
}

wxString wxMenuBar::GetMenuLabel(size_t pos) const
{
    wxCHECK_MSG( pos < GetMenuCount(), wxEmptyString,
                 wxT("invalid menu index in wxMenuBar::GetMenuLabel") );

    return GetMenu(pos)->GetTitle();
}


wxMenu *wxMenuBar::Replace(size_t pos, wxMenu *menu, const wxString& title)
{
    wxMenu *menuOld = wxMenuBarBase::Replace(pos, menu, title);
    if ( !menuOld )
        return NULL;

    wxMenuItem* item = m_rootMenu->FindItemByPosition(pos+firstMenuPos);
    m_rootMenu->Remove(item);
    m_rootMenu->Insert( pos+firstMenuPos, wxMenuItem::New( m_rootMenu, wxID_ANY, title, "", wxITEM_NORMAL, menu ) );

    return menuOld;
}

bool wxMenuBar::Insert(size_t pos, wxMenu *menu, const wxString& title)
{
    if ( !wxMenuBarBase::Insert(pos, menu, title) )
        return false;

    m_rootMenu->Insert( pos+firstMenuPos, wxMenuItem::New( m_rootMenu, wxID_ANY, title, "", wxITEM_NORMAL, menu ) );
    menu->SetTitle(title);

    return true;
}

wxMenu *wxMenuBar::Remove(size_t pos)
{
    wxMenu *menu = wxMenuBarBase::Remove(pos);
    if ( !menu )
        return NULL;

    wxMenuItem* item = m_rootMenu->FindItemByPosition(pos+firstMenuPos);
    m_rootMenu->Remove(item);

    return menu;
}

bool wxMenuBar::Append(wxMenu *menu, const wxString& title)
{
    WXHMENU submenu = menu ? menu->GetHMenu() : 0;
        wxCHECK_MSG( submenu, false, wxT("can't append invalid menu to menubar") );

    if ( !wxMenuBarBase::Append(menu, title) )
        return false;

    m_rootMenu->AppendSubMenu(menu, title);
    menu->SetTitle(title);

    return true;
}

void wxMenuBar::Detach()
{
    wxMenuBarBase::Detach() ;
}

void wxMenuBar::Attach(wxFrame *frame)
{
    wxMenuBarBase::Attach( frame ) ;
}

void wxMenuBar::DoGetPosition(int *x, int *y) const
{
    int _x,_y,_width,_height;

    m_rootMenu->GetPeer()->GetMenuBarDimensions(_x, _y, _width, _height);

    if (x)
        *x = _x;
    if (y)
        *y = _y;
}

void wxMenuBar::DoGetSize(int *width, int *height) const
{
    int _x,_y,_width,_height;

    m_rootMenu->GetPeer()->GetMenuBarDimensions(_x, _y, _width, _height);

    if (width)
        *width = _width;
    if (height)
        *height = _height;
}

void wxMenuBar::DoGetClientSize(int *width, int *height) const
{
    DoGetSize(width, height);
}


#endif 