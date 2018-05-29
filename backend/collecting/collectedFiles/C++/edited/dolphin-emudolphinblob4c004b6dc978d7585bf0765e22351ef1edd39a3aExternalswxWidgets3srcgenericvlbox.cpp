


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_LISTBOX

#include "wx/vlbox.h"

#ifndef WX_PRECOMP
    #include "wx/settings.h"
    #include "wx/dcclient.h"
    #include "wx/listbox.h"
#endif 
#include "wx/dcbuffer.h"
#include "wx/selstore.h"
#include "wx/renderer.h"


wxBEGIN_EVENT_TABLE(wxVListBox, wxVScrolledWindow)
    EVT_PAINT(wxVListBox::OnPaint)

    EVT_KEY_DOWN(wxVListBox::OnKeyDown)
    EVT_LEFT_DOWN(wxVListBox::OnLeftDown)
    EVT_LEFT_DCLICK(wxVListBox::OnLeftDClick)

    EVT_SET_FOCUS(wxVListBox::OnSetOrKillFocus)
    EVT_KILL_FOCUS(wxVListBox::OnSetOrKillFocus)

    EVT_SIZE(wxVListBox::OnSize)
wxEND_EVENT_TABLE()


wxIMPLEMENT_ABSTRACT_CLASS(wxVListBox, wxVScrolledWindow);
const char wxVListBoxNameStr[] = "wxVListBox";


void wxVListBox::Init()
{
    m_current =
    m_anchor = wxNOT_FOUND;
    m_selStore = NULL;
}

bool wxVListBox::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxPoint& pos,
                        const wxSize& size,
                        long style,
                        const wxString& name)
{
#ifdef __WXMSW__
    if ( (style & wxBORDER_MASK) == wxDEFAULT )
        style |= wxBORDER_THEME;
#endif

    style |= wxWANTS_CHARS | wxFULL_REPAINT_ON_RESIZE;
    if ( !wxVScrolledWindow::Create(parent, id, pos, size, style, name) )
        return false;

    if ( style & wxLB_MULTIPLE )
        m_selStore = new wxSelectionStore;

            SetBackgroundColour(GetBackgroundColour());

            m_colBgSel = wxNullColour;

        SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    return true;
}

wxVListBox::~wxVListBox()
{
    delete m_selStore;
}

void wxVListBox::SetItemCount(size_t count)
{
        if ( m_current != wxNOT_FOUND && (size_t)m_current >= count )
        m_current = count - 1; 
    if ( m_selStore )
    {
                m_selStore->SetItemCount(count);
    }

    SetRowCount(count);
}


bool wxVListBox::IsSelected(size_t line) const
{
    return m_selStore ? m_selStore->IsSelected(line) : (int)line == m_current;
}

bool wxVListBox::Select(size_t item, bool select)
{
    wxCHECK_MSG( m_selStore, false,
                 wxT("Select() may only be used with multiselection listbox") );

    wxCHECK_MSG( item < GetItemCount(), false,
                 wxT("Select(): invalid item index") );

    bool changed = m_selStore->SelectItem(item, select);
    if ( changed )
    {
                RefreshRow(item);
    }

    DoSetCurrent(item);

    return changed;
}

bool wxVListBox::SelectRange(size_t from, size_t to)
{
    wxCHECK_MSG( m_selStore, false,
                 wxT("SelectRange() may only be used with multiselection listbox") );

        if ( from > to )
    {
        size_t tmp = from;
        from = to;
        to = tmp;
    }

    wxCHECK_MSG( to < GetItemCount(), false,
                    wxT("SelectRange(): invalid item index") );

    wxArrayInt changed;
    if ( !m_selStore->SelectRange(from, to, true, &changed) )
    {
                        RefreshRows(from, to);
    }
    else     {
        const size_t count = changed.GetCount();
        if ( !count )
        {
                        return false;
        }

                for ( size_t n = 0; n < count; n++ )
        {
            RefreshRow(changed[n]);
        }
    }

        return true;
}

bool wxVListBox::DoSelectAll(bool select)
{
    wxCHECK_MSG( m_selStore, false,
                 wxT("SelectAll may only be used with multiselection listbox") );

    size_t count = GetItemCount();
    if ( count )
    {
        wxArrayInt changed;
        if ( !m_selStore->SelectRange(0, count - 1, select) ||
                !changed.IsEmpty() )
        {
            Refresh();

                        return true;
        }
    }

    return false;
}

bool wxVListBox::DoSetCurrent(int current)
{
    wxASSERT_MSG( current == wxNOT_FOUND ||
                    (current >= 0 && (size_t)current < GetItemCount()),
                  wxT("wxVListBox::DoSetCurrent(): invalid item index") );

    if ( current == m_current )
    {
                return false;
    }

    if ( m_current != wxNOT_FOUND )
        RefreshRow(m_current);

    m_current = current;

    if ( m_current != wxNOT_FOUND )
    {
                        if ( !IsVisible(m_current) )
        {
            ScrollToRow(m_current);
        }
        else         {
                                                            while ( (size_t)m_current + 1 == GetVisibleRowsEnd() &&
                    (size_t)m_current != GetVisibleRowsBegin() &&
                    ScrollToRow(GetVisibleBegin() + 1) ) ;

                                    RefreshRow(m_current);
        }
    }

    return true;
}

void wxVListBox::InitEvent(wxCommandEvent& event, int n)
{
    event.SetEventObject(this);
    event.SetInt(n);
}

void wxVListBox::SendSelectedEvent()
{
    wxASSERT_MSG( m_current != wxNOT_FOUND,
                    wxT("SendSelectedEvent() shouldn't be called") );

    wxCommandEvent event(wxEVT_LISTBOX, GetId());
    InitEvent(event, m_current);
    (void)GetEventHandler()->ProcessEvent(event);
}

void wxVListBox::SetSelection(int selection)
{
    wxCHECK_RET( selection == wxNOT_FOUND ||
                  (selection >= 0 && (size_t)selection < GetItemCount()),
                  wxT("wxVListBox::SetSelection(): invalid item index") );

    if ( HasMultipleSelection() )
    {
        if (selection != wxNOT_FOUND)
            Select(selection);
        else
            DeselectAll();
        m_anchor = selection;
    }

    DoSetCurrent(selection);
}

size_t wxVListBox::GetSelectedCount() const
{
    return m_selStore ? m_selStore->GetSelectedCount()
                      : m_current == wxNOT_FOUND ? 0 : 1;
}

int wxVListBox::GetFirstSelected(unsigned long& cookie) const
{
    cookie = 0;

    return GetNextSelected(cookie);
}

int wxVListBox::GetNextSelected(unsigned long& cookie) const
{
    wxCHECK_MSG( m_selStore, wxNOT_FOUND,
                  wxT("GetFirst/NextSelected() may only be used with multiselection listboxes") );

    while ( cookie < GetItemCount() )
    {
        if ( IsSelected(cookie++) )
            return cookie - 1;
    }

    return wxNOT_FOUND;
}

void wxVListBox::RefreshSelected()
{
        for ( size_t n = GetVisibleBegin(), end = GetVisibleEnd(); n < end; n++ )
    {
        if ( IsSelected(n) )
            RefreshRow(n);
    }
}

wxRect wxVListBox::GetItemRect(size_t n) const
{
    wxRect itemrect;

        const size_t lineMax = GetVisibleEnd();
    if ( n >= lineMax )
        return itemrect;
    size_t line = GetVisibleBegin();
    if ( n < line )
        return itemrect;

    while ( line <= n )
    {
        itemrect.y += itemrect.height;
        itemrect.height = OnGetRowHeight(line);

        line++;
    }

    itemrect.width = GetClientSize().x;

    return itemrect;
}


void wxVListBox::SetMargins(const wxPoint& pt)
{
    if ( pt != m_ptMargins )
    {
        m_ptMargins = pt;

        Refresh();
    }
}

void wxVListBox::SetSelectionBackground(const wxColour& col)
{
    m_colBgSel = col;
}


wxCoord wxVListBox::OnGetRowHeight(size_t line) const
{
    return OnMeasureItem(line) + 2*m_ptMargins.y;
}

void wxVListBox::OnDrawSeparator(wxDC& WXUNUSED(dc),
                                 wxRect& WXUNUSED(rect),
                                 size_t WXUNUSED(n)) const
{
}

bool
wxVListBox::DoDrawSolidBackground(const wxColour& col,
                                  wxDC& dc,
                                  const wxRect& rect,
                                  size_t n) const
{
    if ( !col.IsOk() )
        return false;

        const bool isSelected = IsSelected(n),
               isCurrent = IsCurrent(n);
    if ( isSelected || isCurrent )
    {
        if ( isSelected )
        {
            dc.SetBrush(wxBrush(col, wxBRUSHSTYLE_SOLID));
        }
        else         {
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
        }
        dc.SetPen(*(isCurrent ? wxBLACK_PEN : wxTRANSPARENT_PEN));
        dc.DrawRectangle(rect);
    }
    
    return true;
}

void wxVListBox::OnDrawBackground(wxDC& dc, const wxRect& rect, size_t n) const
{
        if ( !DoDrawSolidBackground(m_colBgSel, dc, rect, n) )
    {
        int flags = 0;
        if ( IsSelected(n) )
            flags |= wxCONTROL_SELECTED;
        if ( IsCurrent(n) )
            flags |= wxCONTROL_CURRENT;
        if ( wxWindow::FindFocus() == const_cast<wxVListBox*>(this) )
            flags |= wxCONTROL_FOCUSED;

        wxRendererNative::Get().DrawItemSelectionRect(
            const_cast<wxVListBox *>(this), dc, rect, flags);
    }
}

void wxVListBox::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxSize clientSize = GetClientSize();

    wxAutoBufferedPaintDC dc(this);

        wxRect rectUpdate = GetUpdateClientRect();

        dc.SetBackground(GetBackgroundColour());
    dc.Clear();

        wxRect rectRow;
    rectRow.width = clientSize.x;

        const size_t lineMax = GetVisibleEnd();
    for ( size_t line = GetVisibleBegin(); line < lineMax; line++ )
    {
        const wxCoord hRow = OnGetRowHeight(line);

        rectRow.height = hRow;

                if ( rectRow.Intersects(rectUpdate) )
        {
                        wxDCClipper clip(dc, rectRow);

            wxRect rect = rectRow;
            OnDrawBackground(dc, rect, line);

            OnDrawSeparator(dc, rect, line);

            rect.Deflate(m_ptMargins.x, m_ptMargins.y);
            OnDrawItem(dc, rect, line);
        }
        else         {
            if ( rectRow.GetTop() > rectUpdate.GetBottom() )
            {
                                                break;
            }
                    }

        rectRow.y += hRow;
    }
}

void wxVListBox::OnSetOrKillFocus(wxFocusEvent& WXUNUSED(event))
{
                RefreshSelected();
}

void wxVListBox::OnSize(wxSizeEvent& event)
{
    UpdateScrollbar();
    event.Skip();
}


void wxVListBox::DoHandleItemClick(int item, int flags)
{
        bool notify = false;

    if ( HasMultipleSelection() )
    {
                bool select = true;

                                if ( flags & ItemClick_Shift )
        {
            if ( m_current != wxNOT_FOUND )
            {
                if ( m_anchor == wxNOT_FOUND )
                    m_anchor = m_current;

                select = false;

                                                if ( DeselectAll() )
                    notify = true;

                if ( SelectRange(m_anchor, item) )
                    notify = true;
            }
                    }
        else         {
            m_anchor = item;

            if ( flags & ItemClick_Ctrl )
            {
                select = false;

                if ( !(flags & ItemClick_Kbd) )
                {
                    Toggle(item);

                                        notify = true;
                }
                            }
                    }

        if ( select )
        {
                        if ( DeselectAll() )
                notify = true;

            if ( Select(item) )
                notify = true;
        }
    }

        if ( DoSetCurrent(item) )
    {
        if ( !HasMultipleSelection() )
        {
                        notify = true;
        }
    }

    if ( notify )
    {
                SendSelectedEvent();
    }
    }


void wxVListBox::OnKeyDown(wxKeyEvent& event)
{
        int flags = ItemClick_Kbd;

    int current;
    switch ( event.GetKeyCode() )
    {
        case WXK_HOME:
        case WXK_NUMPAD_HOME:
            current = 0;
            break;

        case WXK_END:
        case WXK_NUMPAD_END:
            current = GetRowCount() - 1;
            break;

        case WXK_DOWN:
        case WXK_NUMPAD_DOWN:
            if ( m_current == (int)GetRowCount() - 1 )
                return;

            current = m_current + 1;
            break;

        case WXK_UP:
        case WXK_NUMPAD_UP:
            if ( m_current == wxNOT_FOUND )
                current = GetRowCount() - 1;
            else if ( m_current != 0 )
                current = m_current - 1;
            else                 return;
            break;

        case WXK_PAGEDOWN:
        case WXK_NUMPAD_PAGEDOWN:
            PageDown();
            current = GetVisibleBegin();
            break;

        case WXK_PAGEUP:
        case WXK_NUMPAD_PAGEUP:
            if ( m_current == (int)GetVisibleBegin() )
            {
                PageUp();
            }

            current = GetVisibleBegin();
            break;

        case WXK_SPACE:
                                                flags &= ~ItemClick_Kbd;
            current = m_current;
            break;

#ifdef __WXMSW__
        case WXK_TAB:
                                    HandleAsNavigationKey(event);
            #endif
        default:
            event.Skip();
            current = 0;             wxUnusedVar(current);
            return;
    }

    if ( event.ShiftDown() )
       flags |= ItemClick_Shift;
    if ( event.ControlDown() )
        flags |= ItemClick_Ctrl;

    DoHandleItemClick(current, flags);
}


void wxVListBox::OnLeftDown(wxMouseEvent& event)
{
    SetFocus();

    int item = VirtualHitTest(event.GetPosition().y);

    if ( item != wxNOT_FOUND )
    {
        int flags = 0;
        if ( event.ShiftDown() )
           flags |= ItemClick_Shift;

        if ( event.ControlDown() )
            flags |= ItemClick_Ctrl;

        DoHandleItemClick(item, flags);
    }
}

void wxVListBox::OnLeftDClick(wxMouseEvent& eventMouse)
{
    int item = VirtualHitTest(eventMouse.GetPosition().y);
    if ( item != wxNOT_FOUND )
    {

                        if ( item == m_current )
        {
            wxCommandEvent event(wxEVT_LISTBOX_DCLICK, GetId());
            InitEvent(event, item);
            (void)GetEventHandler()->ProcessEvent(event);
        }
        else
        {
            OnLeftDown(eventMouse);
        }

    }
}



wxVisualAttributes
wxVListBox::GetClassDefaultAttributes(wxWindowVariant variant)
{
    return wxListBox::GetClassDefaultAttributes(variant);
}

#endif
