
#include "wx/wxprec.h"

#if wxUSE_LISTBOX

#include "wx/listbox.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/intl.h"
    #include "wx/utils.h"
    #include "wx/settings.h"
    #include "wx/arrstr.h"
    #include "wx/dcclient.h"
#endif

wxBEGIN_EVENT_TABLE(wxListBox, wxControl)
wxEND_EVENT_TABLE()

#include "wx/osx/private.h"


wxListBox::wxListBox()
{
}

bool wxListBox::Create(
    wxWindow *parent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    const wxArrayString& choices,
    long style,
    const wxValidator& validator,
    const wxString& name )
{
    wxCArrayString chs(choices);

    return Create(
        parent, id, pos, size, chs.GetCount(), chs.GetStrings(),
        style, validator, name );
}

wxListWidgetImpl* wxListBox::GetListPeer() const
{
    wxListWidgetImpl* impl =  dynamic_cast<wxListWidgetImpl*> ( GetPeer() );
    return impl;
}

bool wxListBox::Create(
    wxWindow *parent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    int n,
    const wxString choices[],
    long style,
    const wxValidator& validator,
    const wxString& name )
{
    DontCreatePeer();
    m_blockEvents = false;

    if ( ! (style & wxNO_BORDER) )
        style = (style & ~wxBORDER_MASK) | wxSUNKEN_BORDER ;

    wxASSERT_MSG( !(style & wxLB_MULTIPLE) || !(style & wxLB_EXTENDED),
                  wxT("only a single listbox selection mode can be specified") );

    if ( !wxListBoxBase::Create( parent, id, pos, size, style & ~(wxHSCROLL | wxVSCROLL), validator, name ) )
        return false;

    if ( IsSorted() )
        m_strings.sorted = new wxSortedArrayString(wxDictionaryStringSortAscending);
    else
        m_strings.unsorted = new wxArrayString;

    SetPeer(wxWidgetImpl::CreateListBox( this, parent, id, pos, size, style, GetExtraStyle() ));

    MacPostControlCreate( pos, size );

    m_textColumn = GetListPeer()->InsertTextColumn(0,wxEmptyString);

    Append(n, choices);

       SetInitialSize( size );

    return true;
}

wxListBox::~wxListBox()
{
    m_blockEvents = true;
    FreeData();
    m_blockEvents = false;

        SetPeer(NULL);

    if ( IsSorted() )
        delete m_strings.sorted;
    else
        delete m_strings.unsorted;

    m_strings.sorted = NULL;
}

void wxListBox::FreeData()
{
    if ( IsSorted() )
        m_strings.sorted->Clear();
    else
        m_strings.unsorted->Clear();

    m_itemsClientData.Clear();

    GetListPeer()->ListClear();
}

void wxListBox::DoSetFirstItem(int n)
{
                    GetListPeer()->ListScrollTo( GetCount()-1 );
    GetListPeer()->ListScrollTo( n );
}

void wxListBox::EnsureVisible(int n)
{
    GetListPeer()->ListScrollTo( n );
}

int wxListBox::GetTopItem() const
{
    return GetListPeer()->ListGetTopItem();
}

void wxListBox::DoDeleteOneItem(unsigned int n)
{
    wxCHECK_RET( IsValid(n), wxT("invalid index in wxListBox::Delete") );

    m_blockEvents = true;
    if ( IsSorted() )
        m_strings.sorted->RemoveAt(n);
    else
        m_strings.unsorted->RemoveAt(n);

    m_itemsClientData.RemoveAt(n);

    GetListPeer()->ListDelete( n );
    m_blockEvents = false;

    UpdateOldSelections();
}

void wxListBox::DoClear()
{
    m_blockEvents = true;
    FreeData();
    m_blockEvents = false;

    UpdateOldSelections();
}


void wxListBox::DoSetSelection(int n, bool select)
{
    wxCHECK_RET( n == wxNOT_FOUND || IsValid(n),
        wxT("invalid index in wxListBox::SetSelection") );

    m_blockEvents = true;

    if ( n == wxNOT_FOUND )
        GetListPeer()->ListDeselectAll();
    else
        GetListPeer()->ListSetSelection( n, select, HasMultipleSelection() );

    m_blockEvents = false;

    UpdateOldSelections();

    if (select)
        EnsureVisible(n);
}

bool wxListBox::IsSelected(int n) const
{
    wxCHECK_MSG( IsValid(n), false, wxT("invalid index in wxListBox::Selected") );

    return GetListPeer()->ListIsSelected( n );
}

int wxListBox::GetSelections(wxArrayInt& aSelections) const
{
    return GetListPeer()->ListGetSelections( aSelections );
}

int wxListBox::GetSelection() const
{
    return GetListPeer()->ListGetSelection();
}

int wxListBox::DoListHitTest(const wxPoint& inpoint) const
{
    return GetListPeer()->DoListHitTest( inpoint );
}


void wxListBox::GetValueCallback( unsigned int n, wxListWidgetColumn* col , wxListWidgetCellValue& value )
{
    if ( col == m_textColumn )
        value.Set( GetString( n ) );
}

void wxListBox::SetValueCallback( unsigned int WXUNUSED(n), wxListWidgetColumn* WXUNUSED(col) , wxListWidgetCellValue& WXUNUSED(value) )
{
}

wxSize wxListBox::DoGetBestSize() const
{
    int lbWidth = 100;      int lbHeight;
    int wLine;

    {
        wxClientDC dc(const_cast<wxListBox*>(this));
        dc.SetFont(GetFont());

                for (unsigned int i = 0; i < GetCount(); i++)
        {
            wxString str( GetString( i ) );

            wxCoord width, height ;
            dc.GetTextExtent( str , &width, &height);
            wLine = width ;
            lbWidth = wxMax( lbWidth, wLine );
        }

                lbWidth += wxSystemSettings::GetMetric( wxSYS_VSCROLL_X );

                int cy = 12;

        wxCoord width, height ;
        dc.GetTextExtent( wxT("XX") , &width, &height);
        int cx = width ;
        lbWidth += cx;

                        lbHeight = wxMax( (cy + 4) * wxMin( wxMax( GetCount(), 3 ), 10 ), 70 );
    }

    return wxSize( lbWidth, lbHeight );
}

void wxListBox::Refresh(bool eraseBack, const wxRect *rect)
{
    wxControl::Refresh( eraseBack, rect );
}

 wxVisualAttributes
wxListBox::GetClassDefaultAttributes(wxWindowVariant WXUNUSED(variant))
{
    wxVisualAttributes attr;

    attr.colFg = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT );
    attr.colBg = wxSystemSettings::GetColour( wxSYS_COLOUR_LISTBOX );
    static wxFont font = wxFont(wxOSX_SYSTEM_FONT_VIEWS);
    attr.font = font;

    return attr;
}



void wxListBox::DoSetItemClientData(unsigned int n, void* clientData)
{
    m_itemsClientData[n] = clientData;
}

void *wxListBox::DoGetItemClientData(unsigned int n) const
{
    return m_itemsClientData[n];
}


unsigned int wxListBox::GetCount() const
{
    return IsSorted() ? m_strings.sorted->size()
                      : m_strings.unsorted->size();
}

wxString wxListBox::GetString(unsigned int n) const
{
    return IsSorted() ? m_strings.sorted->Item(n)
                      : m_strings.unsorted->Item(n);
}

int wxListBox::FindString(const wxString& s, bool bCase) const
{
    return IsSorted() ? m_strings.sorted->Index(s, bCase)
                      : m_strings.unsorted->Index(s, bCase);
}


void wxListBox::OnItemInserted(unsigned int WXUNUSED(pos))
{
}

int wxListBox::DoInsertItems(const wxArrayStringsAdapter& items,
                             unsigned int pos,
                             void **clientData,
                             wxClientDataType type)
{
    int idx = wxNOT_FOUND;
    unsigned int startpos = pos;

    const unsigned int numItems = items.GetCount();
    for ( unsigned int i = 0; i < numItems; ++i )
    {
        const wxString& item = items[i];
        idx = IsSorted() ? m_strings.sorted->Add(item)
                         : (m_strings.unsorted->Insert(item, pos), pos++);

        m_itemsClientData.Insert(NULL, idx);
        AssignNewItemClientData(idx, clientData, i, type);

        GetListPeer()->ListInsert(startpos+i);

        OnItemInserted(idx);
    }

    GetListPeer()->UpdateLineToEnd(startpos);

                            SetFirstItem(startpos);

    UpdateOldSelections();

    return idx;
}

void wxListBox::SetString(unsigned int n, const wxString& s)
{
    wxCHECK_RET( !IsSorted(), wxT("can't set string in sorted listbox") );

    if ( IsSorted() )
        (*m_strings.sorted)[n] = s;
    else
        (*m_strings.unsorted)[n] = s;

    GetListPeer()->UpdateLine(n);
}


void wxListBox::HandleLineEvent( unsigned int n, bool doubleClick )
{
    wxCommandEvent event( doubleClick ? wxEVT_LISTBOX_DCLICK :
        wxEVT_LISTBOX, GetId() );
    event.SetEventObject( this );
    if ( HasClientObjectData() )
        event.SetClientObject( GetClientObject(n) );
    else if ( HasClientUntypedData() )
        event.SetClientData( GetClientData(n) );
    event.SetString( GetString(n) );
    event.SetInt( n );
    event.SetExtraLong( 1 );
    HandleWindowEvent(event);
}


void wxListWidgetCellValue::Check( bool check )
{
    Set( check ? 1 : 0 );
}

bool wxListWidgetCellValue::IsChecked() const
{
    return GetIntValue() != 0;
}



#endif 