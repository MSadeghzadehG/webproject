
#include "wx/wxprec.h"

#if wxUSE_MENUS

#include "wx/menuitem.h"
#include "wx/stockitem.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/menu.h"
#endif 
#include "wx/osx/private.h"

wxIMPLEMENT_ABSTRACT_CLASS(wxMenuItemImpl, wxObject);

wxMenuItemImpl::~wxMenuItemImpl()
{
}

wxMenuItem::wxMenuItem(wxMenu *pParentMenu,
                       int id,
                       const wxString& t,
                       const wxString& strHelp,
                       wxItemKind kind,
                       wxMenu *pSubMenu)
           :wxMenuItemBase(pParentMenu, id, t, strHelp, kind, pSubMenu)
{
    wxASSERT_MSG( id != 0 || pSubMenu != NULL , wxT("A MenuItem ID of Zero does not work under Mac") ) ;

            if (pParentMenu != NULL && !pParentMenu->GetNoEventsMode())
        if ( wxStripMenuCodes(m_text).Upper() == wxT("EXIT") )
            m_text = wxT("Quit\tCtrl+Q") ;

    m_radioGroup.start = -1;
    m_isRadioGroupStart = false;

    wxString text = wxStripMenuCodes(m_text, (pParentMenu != NULL && pParentMenu->GetNoEventsMode()) ? wxStrip_Accel : wxStrip_All);
    if (text.IsEmpty() && !IsSeparator())
    {
        wxASSERT_MSG(wxIsStockID(GetId()), wxT("A non-stock menu item with an empty label?"));
        text = wxGetStockLabel(GetId(), wxSTOCK_WITH_ACCELERATOR|wxSTOCK_WITH_MNEMONIC);
    }

    wxAcceleratorEntry *entry = wxAcceleratorEntry::Create( m_text ) ;
        m_peer = wxMenuItemImpl::Create( this, pParentMenu, GetId(), text, entry, strHelp, GetKind(), pSubMenu );
    delete entry;
}

wxMenuItem::~wxMenuItem()
{
    delete m_peer;
}


void wxMenuItem::SetBitmap(const wxBitmap& bitmap)
{
      m_bitmap = bitmap;
      UpdateItemBitmap();
}

void wxMenuItem::Enable(bool bDoEnable)
{
    if (( m_isEnabled != bDoEnable
                           )
               || (   GetId() == wxApp::s_macPreferencesMenuItemId
             || GetId() == wxApp::s_macExitMenuItemId
             || GetId() == wxApp::s_macAboutMenuItemId
         ))
    {
        wxMenuItemBase::Enable( bDoEnable ) ;
        UpdateItemStatus() ;
    }
}

void wxMenuItem::UncheckRadio()
{
    if ( m_isChecked )
    {
        wxMenuItemBase::Check( false ) ;
        UpdateItemStatus() ;
    }
}

void wxMenuItem::Check(bool bDoCheck)
{
    wxCHECK_RET( IsCheckable() && !IsSeparator(), wxT("only checkable items may be checked") );

    if ( m_isChecked != bDoCheck )
    {
        if ( GetKind() == wxITEM_RADIO )
        {
            if ( bDoCheck )
            {
                wxMenuItemBase::Check( bDoCheck ) ;
                UpdateItemStatus() ;

                                const wxMenuItemList& items = m_parentMenu->GetMenuItems();
                int pos = items.IndexOf(this);
                wxCHECK_RET( pos != wxNOT_FOUND,
                             wxT("menuitem not found in the menu items list?") );

                                int start, end;

                if ( m_isRadioGroupStart )
                {
                                        start = pos;
                    end = m_radioGroup.end;
                }
                else                 {
                                        start = m_radioGroup.start;
                    end = items.Item(start)->GetData()->m_radioGroup.end;
                }

                                wxMenuItemList::compatibility_iterator node = items.Item(start);
                for ( int n = start; n <= end && node; n++ )
                {
                    if ( n != pos )
                        ((wxMenuItem*)node->GetData())->UncheckRadio();

                    node = node->GetNext();
                }
            }
        }
        else
        {
            wxMenuItemBase::Check( bDoCheck ) ;
            UpdateItemStatus() ;
        }
    }
}

void wxMenuItem::SetItemLabel(const wxString& text)
{
        if ( m_text == text )
        return;

    wxMenuItemBase::SetItemLabel(text);

    UpdateItemText() ;
}


void wxMenuItem::UpdateItemBitmap()
{
    if ( !m_parentMenu )
        return;

    if ( m_bitmap.IsOk() )
    {
        GetPeer()->SetBitmap( m_bitmap );
    }
}

void wxMenuItem::UpdateItemStatus()
{
    if ( !m_parentMenu )
        return ;

    if ( IsSeparator() )
        return ;

    if ( IsCheckable() && IsChecked() )
        GetPeer()->Check( true );
    else
        GetPeer()->Check( false );

    GetPeer()->Enable( IsEnabled() );
}

void wxMenuItem::UpdateItemText()
{
    if ( !m_parentMenu )
        return ;

    wxString text = wxStripMenuCodes(m_text, m_parentMenu != NULL && m_parentMenu->GetNoEventsMode() ? wxStrip_Accel : wxStrip_All);
    if (text.IsEmpty() && !IsSeparator())
    {
        wxASSERT_MSG(wxIsStockID(GetId()), wxT("A non-stock menu item with an empty label?"));
        text = wxGetStockLabel(GetId(), wxSTOCK_WITH_ACCELERATOR|wxSTOCK_WITH_MNEMONIC);
    }

    wxAcceleratorEntry *entry = wxAcceleratorEntry::Create( m_text ) ;
    GetPeer()->SetLabel( text, entry );
    delete entry ;
}


void wxMenuItem::SetAsRadioGroupStart(bool start)
{
    m_isRadioGroupStart = start;
}

void wxMenuItem::SetRadioGroupStart(int start)
{
    wxASSERT_MSG( !m_isRadioGroupStart,
                  wxT("should only be called for the next radio items") );

    m_radioGroup.start = start;
}

void wxMenuItem::SetRadioGroupEnd(int end)
{
    wxASSERT_MSG( m_isRadioGroupStart,
                  wxT("should only be called for the first radio item") );

    m_radioGroup.end = end;
}

bool wxMenuItem::IsRadioGroupStart() const
{
    return m_isRadioGroupStart;
}

int wxMenuItem::GetRadioGroupStart() const
{
    wxASSERT_MSG( !m_isRadioGroupStart,
                  wxS("shouldn't be called for the first radio item") );

    return m_radioGroup.start;
}

int wxMenuItem::GetRadioGroupEnd() const
{
    wxASSERT_MSG( m_isRadioGroupStart,
                  wxS("shouldn't be called for the first radio item") );

    return m_radioGroup.end;
}


wxMenuItem *wxMenuItemBase::New(wxMenu *parentMenu,
                                int id,
                                const wxString& name,
                                const wxString& help,
                                wxItemKind kind,
                                wxMenu *subMenu)
{
    return new wxMenuItem(parentMenu, id, name, help, kind, subMenu);
}

#endif
