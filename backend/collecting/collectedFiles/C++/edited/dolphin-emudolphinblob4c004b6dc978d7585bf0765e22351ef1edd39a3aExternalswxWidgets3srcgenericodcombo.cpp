


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_ODCOMBOBOX

#include "wx/odcombo.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/combobox.h"
    #include "wx/dcclient.h"
    #include "wx/settings.h"
    #include "wx/dialog.h"
    #include "wx/textctrl.h"
#endif

#include "wx/combo.h"


#define wxODCB_PARTIAL_COMPLETION_TIME 1000



wxBEGIN_EVENT_TABLE(wxVListBoxComboPopup, wxVListBox)
    EVT_MOTION(wxVListBoxComboPopup::OnMouseMove)
    EVT_KEY_DOWN(wxVListBoxComboPopup::OnKey)
    EVT_CHAR(wxVListBoxComboPopup::OnChar)
    EVT_LEFT_UP(wxVListBoxComboPopup::OnLeftClick)
wxEND_EVENT_TABLE()


void wxVListBoxComboPopup::Init()
{
    m_widestWidth = 0;
    m_widestItem = -1;
    m_widthsDirty = false;
    m_findWidest = false;
    m_itemHeight = 0;
    m_value = -1;
    m_itemHover = -1;
    m_clientDataItemsType = wxClientData_None;
    m_partialCompletionString = wxEmptyString;
}

bool wxVListBoxComboPopup::Create(wxWindow* parent)
{
    if ( !wxVListBox::Create(parent,
                             wxID_ANY,
                             wxDefaultPosition,
                             wxDefaultSize,
                             wxBORDER_SIMPLE | wxLB_INT_HEIGHT | wxWANTS_CHARS) )
        return false;

    m_useFont = m_combo->GetFont();

    wxVListBox::SetItemCount(m_strings.GetCount());

        m_itemHeight = GetCharHeight() + 0;

    return true;
}

wxVListBoxComboPopup::~wxVListBoxComboPopup()
{
    Clear();
}

void wxVListBoxComboPopup::SetFocus()
{
            #ifdef __WXMSW__
    #else
    wxVListBox::SetFocus();
#endif
}

bool wxVListBoxComboPopup::LazyCreate()
{
                return true;
}

void wxVListBoxComboPopup::PaintComboControl( wxDC& dc, const wxRect& rect )
{
    if ( !(m_combo->GetWindowStyle() & wxODCB_STD_CONTROL_PAINT) )
    {
        int flags = wxODCB_PAINTING_CONTROL;

        if ( m_combo->ShouldDrawFocus() )
            flags |= wxODCB_PAINTING_SELECTED;

        OnDrawBg(dc, rect, m_value, flags);

        if ( m_value >= 0 )
        {
            OnDrawItem(dc,rect,m_value,flags);
            return;
        }
    }

    wxComboPopup::PaintComboControl(dc,rect);
}

void wxVListBoxComboPopup::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const
{
        dc.SetFont(m_useFont);

    int flags = 0;

        if ( wxVListBox::GetSelection() == (int) n )
    {
        dc.SetTextForeground( wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) );
        flags |= wxODCB_PAINTING_SELECTED;
    }
    else
    {
        dc.SetTextForeground( wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT) );
    }

    OnDrawItem(dc,rect,(int)n,flags);
}

wxCoord wxVListBoxComboPopup::OnMeasureItem(size_t n) const
{
    wxOwnerDrawnComboBox* combo = (wxOwnerDrawnComboBox*) m_combo;

    wxASSERT_MSG( wxDynamicCast(combo, wxOwnerDrawnComboBox),
                  wxT("you must subclass wxVListBoxComboPopup for drawing and measuring methods") );

    wxCoord h = combo->OnMeasureItem(n);
    if ( h < 0 )
        h = m_itemHeight;
    return h;
}

wxCoord wxVListBoxComboPopup::OnMeasureItemWidth(size_t n) const
{
    wxOwnerDrawnComboBox* combo = (wxOwnerDrawnComboBox*) m_combo;

    wxASSERT_MSG( wxDynamicCast(combo, wxOwnerDrawnComboBox),
                  wxT("you must subclass wxVListBoxComboPopup for drawing and measuring methods") );

    return combo->OnMeasureItemWidth(n);
}

void wxVListBoxComboPopup::OnDrawBg( wxDC& dc,
                                     const wxRect& rect,
                                     int item,
                                     int flags ) const
{
    wxOwnerDrawnComboBox* combo = (wxOwnerDrawnComboBox*) m_combo;

    wxASSERT_MSG( wxDynamicCast(combo, wxOwnerDrawnComboBox),
                  wxT("you must subclass wxVListBoxComboPopup for drawing and measuring methods") );

    if ( IsCurrent((size_t)item) && !(flags & wxODCB_PAINTING_CONTROL) )
        flags |= wxODCB_PAINTING_SELECTED;

    combo->OnDrawBackground(dc,rect,item,flags);
}

void wxVListBoxComboPopup::OnDrawBackground(wxDC& dc, const wxRect& rect, size_t n) const
{
    OnDrawBg(dc,rect,(int)n,0);
}

void wxVListBoxComboPopup::OnDrawItem( wxDC& dc, const wxRect& rect, int item, int flags ) const
{
    wxOwnerDrawnComboBox* combo = (wxOwnerDrawnComboBox*) m_combo;

    wxASSERT_MSG( wxDynamicCast(combo, wxOwnerDrawnComboBox),
                  wxT("you must subclass wxVListBoxComboPopup for drawing and measuring methods") );

    combo->OnDrawItem(dc,rect,item,flags);
}

void wxVListBoxComboPopup::DismissWithEvent()
{
    StopPartialCompletion();

    int selection = wxVListBox::GetSelection();

    Dismiss();

    if ( selection != wxNOT_FOUND )
        m_stringValue = m_strings[selection];
    else
        m_stringValue = wxEmptyString;

    if ( m_stringValue != m_combo->GetValue() )
        m_combo->SetValueByUser(m_stringValue);

    m_value = selection;

    SendComboBoxEvent(selection);
}

void wxVListBoxComboPopup::SendComboBoxEvent( int selection )
{
    wxCommandEvent evt(wxEVT_COMBOBOX,m_combo->GetId());

    evt.SetEventObject(m_combo);

    evt.SetInt(selection);

        if ( selection >= 0 && (int)m_clientDatas.GetCount() > selection )
    {
        void* clientData = m_clientDatas[selection];
        if ( m_clientDataItemsType == wxClientData_Object )
            evt.SetClientObject((wxClientData*)clientData);
        else
            evt.SetClientData(clientData);
    }

    m_combo->GetEventHandler()->AddPendingEvent(evt);
}

bool wxVListBoxComboPopup::HandleKey( int keycode, bool saturate, wxChar keychar )
{
    const int itemCount = GetCount();

            if ( !itemCount )
        return false;

    int value = m_value;
    int comboStyle = m_combo->GetWindowStyle();

    if ( keychar > 0 )
    {
                        if ( !wxIsprint(keychar) )
            keychar = 0;
    }

    const bool readOnly = (comboStyle & wxCB_READONLY) != 0;

    if ( keycode == WXK_DOWN || keycode == WXK_NUMPAD_DOWN || ( keycode == WXK_RIGHT && readOnly ) )
    {
        value++;
        StopPartialCompletion();
    }
    else if ( keycode == WXK_UP || keycode == WXK_NUMPAD_UP || ( keycode == WXK_LEFT && readOnly ) )
    {
        value--;
        StopPartialCompletion();
    }
    else if ( keycode == WXK_PAGEDOWN || keycode == WXK_NUMPAD_PAGEDOWN )
    {
        value+=10;
        StopPartialCompletion();
    }
    else if ( keycode == WXK_PAGEUP || keycode == WXK_NUMPAD_PAGEUP )
    {
        value-=10;
        StopPartialCompletion();
    }
    else if ( ( keycode == WXK_HOME || keycode == WXK_NUMPAD_HOME ) && readOnly )
    {
        value=0;
        StopPartialCompletion();
    }
    else if ( ( keycode == WXK_END || keycode == WXK_NUMPAD_END ) && readOnly )
    {
        value=itemCount-1;
        StopPartialCompletion();
    }
    else if ( keychar && readOnly )
    {
        
        #if wxUSE_TIMER
        if (m_partialCompletionTimer.IsRunning())
            m_partialCompletionString+=wxString(keychar);
        else
#endif             m_partialCompletionString=wxString(keychar);

                int found = -1;
        unsigned int length=m_partialCompletionString.length();
        int i;
        for (i=0; i<itemCount; i++)
        {
            wxString item=GetString(i);
            if (( item.length() >= length) && (!  m_partialCompletionString.CmpNoCase(item.Left(length))))
            {
                found=i;
                break;
            }
        }

        if (found<0)
        {
            StopPartialCompletion();
            ::wxBell();
            return true;         }
        else
        {
            value=i;
#if wxUSE_TIMER
            m_partialCompletionTimer.Start(wxODCB_PARTIAL_COMPLETION_TIME, true);
#endif         }
    }
    else
        return false;

    if ( saturate )
    {
        if ( value >= itemCount )
            value = itemCount - 1;
        else if ( value < 0 )
            value = 0;
    }
    else
    {
        if ( value >= itemCount )
            value -= itemCount;
        else if ( value < 0 )
            value += itemCount;
    }

    if ( value == m_value )
                        return true;

    if ( value >= 0 )
        m_combo->ChangeValue(m_strings[value]);

                    m_value = value;

    SendComboBoxEvent(m_value);

    return true;
}

void wxVListBoxComboPopup::StopPartialCompletion()
{
    m_partialCompletionString = wxEmptyString;
#if wxUSE_TIMER
    m_partialCompletionTimer.Stop();
#endif }

void wxVListBoxComboPopup::OnComboDoubleClick()
{
        if ( !::wxGetKeyState(WXK_SHIFT) )
        HandleKey(WXK_DOWN,false);
    else
        HandleKey(WXK_UP,false);
}

void wxVListBoxComboPopup::OnComboKeyEvent( wxKeyEvent& event )
{
        if ( !HandleKey(event.GetKeyCode(), true) )
        event.Skip();
}

void wxVListBoxComboPopup::OnComboCharEvent( wxKeyEvent& event )
{
        #if wxUSE_UNICODE
    const wxChar charcode = event.GetUnicodeKey();
#else
    const wxChar charcode = (wxChar)event.GetKeyCode();
#endif

    if ( !HandleKey(event.GetKeyCode(), true, charcode) )
        event.Skip();
}

void wxVListBoxComboPopup::OnPopup()
{
        wxVListBox::SetSelection(m_value);
}

void wxVListBoxComboPopup::OnMouseMove(wxMouseEvent& event)
{
    event.Skip();

    
    int y = event.GetPosition().y;
    int fromBottom = GetClientSize().y - y;

                const size_t lineMax = GetVisibleEnd();
    for ( size_t line = GetVisibleBegin(); line < lineMax; line++ )
    {
        y -= OnGetRowHeight(line);
        if ( y < 0 )
        {
                        if ( (y + fromBottom) >= 0 )
            {
                wxVListBox::SetSelection((int)line);
                return;
            }
        }
    }
}

void wxVListBoxComboPopup::OnLeftClick(wxMouseEvent& WXUNUSED(event))
{
    DismissWithEvent();
}

void wxVListBoxComboPopup::OnKey(wxKeyEvent& event)
{
        if ( m_combo->IsKeyPopupToggle(event) )
    {
        StopPartialCompletion();
        Dismiss();
    }
    else if ( event.AltDown() )
    {
                                return;
    }
        else if ( event.GetKeyCode() == WXK_RETURN || event.GetKeyCode() == WXK_NUMPAD_ENTER )
    {
        DismissWithEvent();
    }
    else
    {
                event.Skip();
    }
}

void wxVListBoxComboPopup::OnChar(wxKeyEvent& event)
{
    if ( m_combo->GetWindowStyle() & wxCB_READONLY )
    {
                #if wxUSE_UNICODE
        const wxChar charcode = event.GetUnicodeKey();
#else
        const wxChar charcode = (wxChar)event.GetKeyCode();
#endif
        if ( wxIsprint(charcode) )
        {
            OnComboCharEvent(event);
            SetSelection(m_value);             return;         }
    }

    event.Skip();
}

void wxVListBoxComboPopup::Insert( const wxString& item, int pos )
{
        wxString strValue;
    if ( !(m_combo->GetWindowStyle() & wxCB_READONLY) &&
         m_combo->GetValue() == item )
    {
        m_value = pos;
    }

    m_strings.Insert(item,pos);
    if ( (int)m_clientDatas.size() >= pos )
        m_clientDatas.Insert(NULL, pos);

    m_widths.Insert(-1,pos);
    m_widthsDirty = true;

    if ( IsCreated() )
        wxVListBox::SetItemCount( wxVListBox::GetItemCount()+1 );
}

int wxVListBoxComboPopup::Append(const wxString& item)
{
    int pos = (int)m_strings.GetCount();

    if ( m_combo->GetWindowStyle() & wxCB_SORT )
    {
                        wxArrayString strings = m_strings;
        unsigned int i;

        for ( i=0; i<strings.GetCount(); i++ )
        {
            if ( item.CmpNoCase(strings.Item(i)) <= 0 )
            {
                pos = (int)i;
                break;
            }
        }
    }

    Insert(item,pos);

    return pos;
}

void wxVListBoxComboPopup::Clear()
{
    wxASSERT(m_combo);

    m_strings.Empty();
    m_widths.Empty();

    m_widestWidth = 0;
    m_widestItem = -1;

    ClearClientDatas();

    m_value = wxNOT_FOUND;

    if ( IsCreated() )
        wxVListBox::SetItemCount(0);
}

void wxVListBoxComboPopup::ClearClientDatas()
{
    if ( m_clientDataItemsType == wxClientData_Object )
    {
        size_t i;
        for ( i=0; i<m_clientDatas.GetCount(); i++ )
            delete (wxClientData*) m_clientDatas[i];
    }

    m_clientDatas.Empty();
    m_clientDataItemsType = wxClientData_None;
}

void wxVListBoxComboPopup::SetItemClientData( unsigned int n,
                                              void* clientData,
                                              wxClientDataType clientDataItemsType )
{
        m_clientDataItemsType = clientDataItemsType;

    m_clientDatas[n] = clientData;

    ItemWidthChanged(n);
}

void* wxVListBoxComboPopup::GetItemClientData(unsigned int n) const
{
    if ( m_clientDatas.GetCount() > n )
        return m_clientDatas[n];

    return NULL;
}

void wxVListBoxComboPopup::Delete( unsigned int item )
{
        if ( m_clientDatas.GetCount() )
    {
        if ( m_clientDataItemsType == wxClientData_Object )
            delete (wxClientData*) m_clientDatas[item];

        m_clientDatas.RemoveAt(item);
    }

    m_strings.RemoveAt(item);
    m_widths.RemoveAt(item);

    if ( (int)item == m_widestItem )
        m_findWidest = true;

    int sel = GetSelection();

    if ( IsCreated() )
        wxVListBox::SetItemCount( wxVListBox::GetItemCount()-1 );

        if ( (int)item < sel )
        SetSelection(sel-1);
    else if ( (int)item == sel )
        SetSelection(wxNOT_FOUND);
}

int wxVListBoxComboPopup::FindString(const wxString& s, bool bCase) const
{
    return m_strings.Index(s, bCase);
}

bool wxVListBoxComboPopup::FindItem(const wxString& item, wxString* trueItem)
{
    int idx = m_strings.Index(item, false);
    if ( idx == wxNOT_FOUND )
        return false;
    if ( trueItem != NULL )
        *trueItem = m_strings[idx];
    return true;
}

unsigned int wxVListBoxComboPopup::GetCount() const
{
    return m_strings.GetCount();
}

wxString wxVListBoxComboPopup::GetString( int item ) const
{
    return m_strings[item];
}

void wxVListBoxComboPopup::SetString( int item, const wxString& str )
{
    m_strings[item] = str;
    ItemWidthChanged(item);
}

wxString wxVListBoxComboPopup::GetStringValue() const
{
    return m_stringValue;
}

void wxVListBoxComboPopup::SetSelection( int item )
{
    wxCHECK_RET( item == wxNOT_FOUND || ((unsigned int)item < GetCount()),
                 wxT("invalid index in wxVListBoxComboPopup::SetSelection") );

    m_value = item;

    if ( item >= 0 )
        m_stringValue = m_strings[item];
    else
        m_stringValue = wxEmptyString;

    if ( IsCreated() )
        wxVListBox::SetSelection(item);
}

int wxVListBoxComboPopup::GetSelection() const
{
    return m_value;
}

void wxVListBoxComboPopup::SetStringValue( const wxString& value )
{
    int index = m_strings.Index(value);

    m_stringValue = value;

    if ( index >= 0 && index < (int)wxVListBox::GetItemCount() )
    {
        wxVListBox::SetSelection(index);
        m_value = index;
    }
}

void wxVListBoxComboPopup::CalcWidths()
{
    bool doFindWidest = m_findWidest;

        if ( m_widthsDirty )
    {
        unsigned int i;
        unsigned int n = m_widths.GetCount();
        int dirtyHandled = 0;
        wxArrayInt& widths = m_widths;

                                wxClientDC dc(m_combo);
        if ( !m_useFont.IsOk() )
            m_useFont = m_combo->GetFont();
        dc.SetFont(m_useFont);

        for ( i=0; i<n; i++ )
        {
            if ( widths[i] < 0 )
            {
                wxCoord x = OnMeasureItemWidth(i);

                if ( x < 0 )
                {
                    const wxString& text = m_strings[i];

                                                                                if ( dirtyHandled < 1024 )
                    {
                        wxCoord y;
                        dc.GetTextExtent(text, &x, &y, 0, 0);
                        x += 4;
                    }
                    else
                    {
                        x = text.length() * (dc.GetCharWidth()+1);
                    }
                }

                widths[i] = x;

                if ( x >= m_widestWidth )
                {
                    m_widestWidth = x;
                    m_widestItem = (int)i;
                }
                else if ( (int)i == m_widestItem )
                {
                                                            doFindWidest = true;
                }

                dirtyHandled++;
            }
        }

        m_widthsDirty = false;
    }

    if ( doFindWidest )
    {
        unsigned int i;
        unsigned int n = m_widths.GetCount();

        int bestWidth = -1;
        int bestIndex = -1;

        for ( i=0; i<n; i++ )
        {
            int w = m_widths[i];
            if ( w > bestWidth )
            {
                bestIndex = (int)i;
                bestWidth = w;
            }
        }

        m_widestWidth = bestWidth;
        m_widestItem = bestIndex;

        m_findWidest = false;
    }
}

wxSize wxVListBoxComboPopup::GetAdjustedSize( int minWidth, int prefHeight, int maxHeight )
{
    int height = 250;

    maxHeight -= 2;  
    if ( m_strings.GetCount() )
    {
        if ( prefHeight > 0 )
            height = prefHeight;

        if ( height > maxHeight )
            height = maxHeight;

        int totalHeight = GetTotalHeight(); 
        #if defined(__WXMAC__)
        totalHeight += 2;
#endif
        if ( height >= totalHeight )
        {
            height = totalHeight;
        }
        else
        {
                                                int fih = GetLineHeight(0);
            height -= height % fih;
        }
    }
    else
        height = 50;

    CalcWidths();

        int widestWidth = m_widestWidth + wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
    return wxSize(minWidth > widestWidth ? minWidth : widestWidth,
                  height+2);
}

void wxVListBoxComboPopup::Populate( const wxArrayString& choices )
{
    int i;

    int n = choices.GetCount();

    for ( i=0; i<n; i++ )
    {
        const wxString& item = choices.Item(i);
        m_strings.Add(item);
    }

    m_widths.SetCount(n,-1);
    m_widthsDirty = true;

    if ( IsCreated() )
        wxVListBox::SetItemCount(n);

        if ( m_combo->GetWindowStyle() & wxCB_SORT )
        m_strings.Sort();

        wxString strValue = m_combo->GetValue();
    if ( !strValue.empty() )
        m_value = m_strings.Index(strValue);
}



wxBEGIN_EVENT_TABLE(wxOwnerDrawnComboBox, wxComboCtrl)
wxEND_EVENT_TABLE()

void wxOwnerDrawnComboBox::Init()
{
}

bool wxOwnerDrawnComboBox::Create(wxWindow *parent,
                                  wxWindowID id,
                                  const wxString& value,
                                  const wxPoint& pos,
                                  const wxSize& size,
                                  long style,
                                  const wxValidator& validator,
                                  const wxString& name)
{
    return wxComboCtrl::Create(parent,id,value,pos,size,style,validator,name);
}

wxOwnerDrawnComboBox::wxOwnerDrawnComboBox(wxWindow *parent,
                                           wxWindowID id,
                                           const wxString& value,
                                           const wxPoint& pos,
                                           const wxSize& size,
                                           const wxArrayString& choices,
                                           long style,
                                           const wxValidator& validator,
                                           const wxString& name)
{
    Init();

    Create(parent,id,value,pos,size,choices,style, validator, name);
}

bool wxOwnerDrawnComboBox::Create(wxWindow *parent,
                                  wxWindowID id,
                                  const wxString& value,
                                  const wxPoint& pos,
                                  const wxSize& size,
                                  const wxArrayString& choices,
                                  long style,
                                  const wxValidator& validator,
                                  const wxString& name)
{
    m_initChs = choices;
    
            return Create(parent, id, value, pos, size, 0,
                  NULL, style, validator, name);
}

bool wxOwnerDrawnComboBox::Create(wxWindow *parent,
                                  wxWindowID id,
                                  const wxString& value,
                                  const wxPoint& pos,
                                  const wxSize& size,
                                  int n,
                                  const wxString choices[],
                                  long style,
                                  const wxValidator& validator,
                                  const wxString& name)
{

    if ( !Create(parent, id, value, pos, size, style,
                 validator, name) )
    {
        return false;
    }

    int i;
    for ( i=0; i<n; i++ )
        m_initChs.Add(choices[i]);

    return true;
}

wxOwnerDrawnComboBox::~wxOwnerDrawnComboBox()
{
    if ( m_popupInterface )
        GetVListBoxComboPopup()->ClearClientDatas();
}

void wxOwnerDrawnComboBox::DoSetPopupControl(wxComboPopup* popup)
{
    if ( !popup )
    {
        popup = new wxVListBoxComboPopup();
    }

    wxComboCtrl::DoSetPopupControl(popup);

    wxASSERT(popup);

        if ( !GetVListBoxComboPopup()->GetCount() )
    {
        GetVListBoxComboPopup()->Populate(m_initChs);
        m_initChs.Clear();
    }
}


void wxOwnerDrawnComboBox::DoClear()
{
    EnsurePopupControl();

    GetVListBoxComboPopup()->Clear();

            SetValue(wxEmptyString);
}

void wxOwnerDrawnComboBox::Clear()
{
    DoClear();
    SetClientDataType(wxClientData_None);
}

void wxOwnerDrawnComboBox::DoDeleteOneItem(unsigned int n)
{
    wxCHECK_RET( IsValid(n), wxT("invalid index in wxOwnerDrawnComboBox::Delete") );

    if ( GetSelection() == (int) n )
        ChangeValue(wxEmptyString);

    GetVListBoxComboPopup()->Delete(n);
}

unsigned int wxOwnerDrawnComboBox::GetCount() const
{
    if ( !m_popupInterface )
        return m_initChs.GetCount();

    return GetVListBoxComboPopup()->GetCount();
}

wxString wxOwnerDrawnComboBox::GetString(unsigned int n) const
{
    wxCHECK_MSG( IsValid(n), wxEmptyString, wxT("invalid index in wxOwnerDrawnComboBox::GetString") );

    if ( !m_popupInterface )
        return m_initChs.Item(n);

    return GetVListBoxComboPopup()->GetString(n);
}

void wxOwnerDrawnComboBox::SetString(unsigned int n, const wxString& s)
{
    EnsurePopupControl();

    wxCHECK_RET( IsValid(n), wxT("invalid index in wxOwnerDrawnComboBox::SetString") );

    GetVListBoxComboPopup()->SetString(n,s);
}

int wxOwnerDrawnComboBox::FindString(const wxString& s, bool bCase) const
{
    if ( !m_popupInterface )
        return m_initChs.Index(s, bCase);

    return GetVListBoxComboPopup()->FindString(s, bCase);
}

void wxOwnerDrawnComboBox::Select(int n)
{
    EnsurePopupControl();

    wxCHECK_RET( (n == wxNOT_FOUND) || IsValid(n), wxT("invalid index in wxOwnerDrawnComboBox::Select") );

    GetVListBoxComboPopup()->SetSelection(n);

    wxString str;
    if ( n >= 0 )
        str = GetVListBoxComboPopup()->GetString(n);

        if ( m_text )
        m_text->ChangeValue( str );
    else
        m_valueString = str;

    Refresh();
}

int wxOwnerDrawnComboBox::GetSelection() const
{
    if ( !m_popupInterface )
        return m_initChs.Index(m_valueString);

    return GetVListBoxComboPopup()->GetSelection();
}

void wxOwnerDrawnComboBox::GetSelection(long *from, long *to) const
{
    wxComboCtrl::GetSelection(from, to);
}

int wxOwnerDrawnComboBox::DoInsertItems(const wxArrayStringsAdapter& items,
                                        unsigned int pos,
                                        void **clientData,
                                        wxClientDataType type)
{
    EnsurePopupControl();

    const unsigned int count = items.GetCount();

    if ( HasFlag(wxCB_SORT) )
    {
        int n = pos;

        for ( unsigned int i = 0; i < count; ++i )
        {
            n = GetVListBoxComboPopup()->Append(items[i]);
            AssignNewItemClientData(n, clientData, i, type);
        }

        return n;
    }
    else
    {
        for ( unsigned int i = 0; i < count; ++i, ++pos )
        {
            GetVListBoxComboPopup()->Insert(items[i], pos);
            AssignNewItemClientData(pos, clientData, i, type);
        }

        return pos - 1;
    }
}

void wxOwnerDrawnComboBox::DoSetItemClientData(unsigned int n, void* clientData)
{
    EnsurePopupControl();

    GetVListBoxComboPopup()->SetItemClientData(n, clientData,
                                               GetClientDataType());
}

void* wxOwnerDrawnComboBox::DoGetItemClientData(unsigned int n) const
{
    if ( !m_popupInterface )
        return NULL;

    return GetVListBoxComboPopup()->GetItemClientData(n);
}


void wxOwnerDrawnComboBox::OnDrawItem( wxDC& dc,
                                       const wxRect& rect,
                                       int item,
                                       int flags ) const
{
    if ( flags & wxODCB_PAINTING_CONTROL )
    {
        wxString text;

        if ( !ShouldUseHintText() )
        {
            text = GetValue();
        }
        else
        {
            text = GetHint();
            wxColour col = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
            dc.SetTextForeground(col);
        }

        dc.DrawText( text,
                     rect.x + GetMargins().x,
                     (rect.height-dc.GetCharHeight())/2 + rect.y );
    }
    else
    {
        dc.DrawText( GetVListBoxComboPopup()->GetString(item), rect.x + 2, rect.y );
    }
}

wxCoord wxOwnerDrawnComboBox::OnMeasureItem( size_t WXUNUSED(item) ) const
{
    return -1;
}

wxCoord wxOwnerDrawnComboBox::OnMeasureItemWidth( size_t WXUNUSED(item) ) const
{
    return -1;
}

wxSize wxOwnerDrawnComboBox::DoGetBestSize() const
{
    if ( GetCount() == 0 )
        return wxComboCtrlBase::DoGetBestSize();

    wxOwnerDrawnComboBox* odc = const_cast<wxOwnerDrawnComboBox*>(this);
                return GetSizeFromTextSize(odc->GetWidestItemWidth());
}

void wxOwnerDrawnComboBox::OnDrawBackground(wxDC& dc,
                                            const wxRect& rect,
                                            int WXUNUSED(item),
                                            int flags) const
{
            
    if ( (flags & wxODCB_PAINTING_SELECTED) ||
         ((flags & wxODCB_PAINTING_CONTROL) && HasFlag(wxCB_READONLY)) )
    {
        int bgFlags = wxCONTROL_SELECTED;

        if ( !(flags & wxODCB_PAINTING_CONTROL) )
            bgFlags |= wxCONTROL_ISSUBMENU;

        PrepareBackground(dc, rect, bgFlags);
    }
}

#endif 