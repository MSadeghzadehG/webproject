


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_HEADERCTRL

#include "wx/headerctrl.h"

#ifdef wxHAS_GENERIC_HEADERCTRL

#include "wx/dcbuffer.h"
#include "wx/renderer.h"


namespace
{

const unsigned COL_NONE = (unsigned)-1;

} 


void wxHeaderCtrl::Init()
{
    m_numColumns = 0;
    m_hover =
    m_colBeingResized =
    m_colBeingReordered = COL_NONE;
    m_dragOffset = 0;
    m_scrollOffset = 0;
}

bool wxHeaderCtrl::Create(wxWindow *parent,
                          wxWindowID id,
                          const wxPoint& pos,
                          const wxSize& size,
                          long style,
                          const wxString& name)
{
    if ( !wxHeaderCtrlBase::Create(parent, id, pos, size,
                                   style, wxDefaultValidator, name) )
        return false;

            SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    return true;
}

wxHeaderCtrl::~wxHeaderCtrl()
{
}


void wxHeaderCtrl::DoSetCount(unsigned int count)
{
        DoResizeColumnIndices(m_colIndices, count);

    m_numColumns = count;

    InvalidateBestSize();
    Refresh();
}

unsigned int wxHeaderCtrl::DoGetCount() const
{
    return m_numColumns;
}

void wxHeaderCtrl::DoUpdate(unsigned int idx)
{
    InvalidateBestSize();

                RefreshColsAfter(idx);
}


void wxHeaderCtrl::DoScrollHorz(int dx)
{
    m_scrollOffset += dx;

        wxControl::ScrollWindow(dx, 0);
}


wxSize wxHeaderCtrl::DoGetBestSize() const
{
    wxWindow *win = GetParent();
    int height = wxRendererNative::Get().GetHeaderButtonHeight( win );

            const wxSize size(IsEmpty() ? wxHeaderCtrlBase::DoGetBestSize().x
                                : GetColEnd(GetColumnCount() - 1),
                                height );     CacheBestSize(size);
    return size;
}

int wxHeaderCtrl::GetColStart(unsigned int idx) const
{
    int pos = m_scrollOffset;
    for ( unsigned n = 0; ; n++ )
    {
        const unsigned i = m_colIndices[n];
        if ( i == idx )
            break;

        const wxHeaderColumn& col = GetColumn(i);
        if ( col.IsShown() )
            pos += col.GetWidth();
    }

    return pos;
}

int wxHeaderCtrl::GetColEnd(unsigned int idx) const
{
    int x = GetColStart(idx);

    return x + GetColumn(idx).GetWidth();
}

unsigned int wxHeaderCtrl::FindColumnAtPoint(int x, bool *onSeparator) const
{
    int pos = 0;
    const unsigned count = GetColumnCount();
    for ( unsigned n = 0; n < count; n++ )
    {
        const unsigned idx = m_colIndices[n];
        const wxHeaderColumn& col = GetColumn(idx);
        if ( col.IsHidden() )
            continue;

        pos += col.GetWidth();

                                        if ( col.IsResizeable() && abs(x - pos) < 8 )
        {
            if ( onSeparator )
                *onSeparator = true;
            return idx;
        }

                if ( x < pos )
        {
            if ( onSeparator )
                *onSeparator = false;
            return idx;
        }
    }

    if ( onSeparator )
        *onSeparator = false;
    return COL_NONE;
}


void wxHeaderCtrl::RefreshCol(unsigned int idx)
{
    wxRect rect = GetClientRect();
    rect.x += GetColStart(idx);
    rect.width = GetColumn(idx).GetWidth();

    RefreshRect(rect);
}

void wxHeaderCtrl::RefreshColIfNotNone(unsigned int idx)
{
    if ( idx != COL_NONE )
        RefreshCol(idx);
}

void wxHeaderCtrl::RefreshColsAfter(unsigned int idx)
{
    wxRect rect = GetClientRect();
    const int ofs = GetColStart(idx);
    rect.x += ofs;
    rect.width -= ofs;

    RefreshRect(rect);
}


bool wxHeaderCtrl::IsResizing() const
{
    return m_colBeingResized != COL_NONE;
}

bool wxHeaderCtrl::IsReordering() const
{
    return m_colBeingReordered != COL_NONE;
}

void wxHeaderCtrl::ClearMarkers()
{
    wxClientDC dc(this);

    wxDCOverlay dcover(m_overlay, &dc);
    dcover.Clear();
}

void wxHeaderCtrl::EndDragging()
{
        if (IsReordering())
    {
        ClearMarkers();
        m_overlay.Reset();
    }

        SetCursor(wxNullCursor);
}

void wxHeaderCtrl::CancelDragging()
{
    wxASSERT_MSG( IsDragging(),
                  "shouldn't be called if we're not dragging anything" );

    EndDragging();

    unsigned int& col = IsResizing() ? m_colBeingResized : m_colBeingReordered;

    wxHeaderCtrlEvent event(wxEVT_HEADER_DRAGGING_CANCELLED, GetId());
    event.SetEventObject(this);
    event.SetColumn(col);

    GetEventHandler()->ProcessEvent(event);

    col = COL_NONE;
}

int wxHeaderCtrl::ConstrainByMinWidth(unsigned int col, int& xPhysical)
{
    const int xStart = GetColStart(col);

            const int xMinEnd = xStart + GetColumn(col).GetMinWidth();

    if ( xPhysical < xMinEnd )
        xPhysical = xMinEnd;

    return xPhysical - xStart;
}

void wxHeaderCtrl::StartOrContinueResizing(unsigned int col, int xPhysical)
{
    wxHeaderCtrlEvent event(IsResizing() ? wxEVT_HEADER_RESIZING
                                         : wxEVT_HEADER_BEGIN_RESIZE,
                            GetId());
    event.SetEventObject(this);
    event.SetColumn(col);

    event.SetWidth(ConstrainByMinWidth(col, xPhysical));

    if ( GetEventHandler()->ProcessEvent(event) && !event.IsAllowed() )
    {
        if ( IsResizing() )
        {
            ReleaseMouse();
            CancelDragging();
        }
            }
    else     {
        if ( !IsResizing() )
        {
            m_colBeingResized = col;
            SetCursor(wxCursor(wxCURSOR_SIZEWE));
            CaptureMouse();
        }
        
    }
}

void wxHeaderCtrl::EndResizing(int xPhysical)
{
    wxASSERT_MSG( IsResizing(), "shouldn't be called if we're not resizing" );

    EndDragging();

    ReleaseMouse();

    wxHeaderCtrlEvent event(wxEVT_HEADER_END_RESIZE, GetId());
    event.SetEventObject(this);
    event.SetColumn(m_colBeingResized);
    event.SetWidth(ConstrainByMinWidth(m_colBeingResized, xPhysical));

    GetEventHandler()->ProcessEvent(event);

    m_colBeingResized = COL_NONE;
}

void wxHeaderCtrl::UpdateReorderingMarker(int xPhysical)
{
    wxClientDC dc(this);

    wxDCOverlay dcover(m_overlay, &dc);
    dcover.Clear();

    dc.SetPen(*wxBLUE);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);

        int x = xPhysical - m_dragOffset;
    int y = GetClientSize().y;
    dc.DrawRectangle(x, 0,
                     GetColumn(m_colBeingReordered).GetWidth(), y);

            unsigned int col = FindColumnAtPoint(xPhysical);
    if ( col != COL_NONE )
    {
        static const int DROP_MARKER_WIDTH = 4;

        dc.SetBrush(*wxBLUE);
        dc.DrawRectangle(GetColEnd(col) - DROP_MARKER_WIDTH/2, 0,
                         DROP_MARKER_WIDTH, y);
    }
}

void wxHeaderCtrl::StartReordering(unsigned int col, int xPhysical)
{
    wxHeaderCtrlEvent event(wxEVT_HEADER_BEGIN_REORDER, GetId());
    event.SetEventObject(this);
    event.SetColumn(col);

    if ( GetEventHandler()->ProcessEvent(event) && !event.IsAllowed() )
    {
                return;
    }

    m_dragOffset = xPhysical - GetColStart(col);

    m_colBeingReordered = col;
    SetCursor(wxCursor(wxCURSOR_HAND));
    CaptureMouse();

            }

bool wxHeaderCtrl::EndReordering(int xPhysical)
{
    wxASSERT_MSG( IsReordering(), "shouldn't be called if we're not reordering" );

    EndDragging();

    ReleaseMouse();

    const int colOld = m_colBeingReordered,
              colNew = FindColumnAtPoint(xPhysical);

    m_colBeingReordered = COL_NONE;

    if ( xPhysical - GetColStart(colOld) == m_dragOffset )
        return false;

    if ( colNew != colOld )
    {
        wxHeaderCtrlEvent event(wxEVT_HEADER_END_REORDER, GetId());
        event.SetEventObject(this);
        event.SetColumn(colOld);

        const unsigned pos = GetColumnPos(FindColumnAtPoint(xPhysical));
        event.SetNewOrder(pos);

        if ( !GetEventHandler()->ProcessEvent(event) || event.IsAllowed() )
        {
                        DoMoveCol(colOld, pos);
        }
    }

            return true;
}


void wxHeaderCtrl::DoSetColumnsOrder(const wxArrayInt& order)
{
    m_colIndices = order;
    Refresh();
}

wxArrayInt wxHeaderCtrl::DoGetColumnsOrder() const
{
    return m_colIndices;
}

void wxHeaderCtrl::DoMoveCol(unsigned int idx, unsigned int pos)
{
    MoveColumnInOrderArray(m_colIndices, idx, pos);

    Refresh();
}


wxBEGIN_EVENT_TABLE(wxHeaderCtrl, wxHeaderCtrlBase)
    EVT_PAINT(wxHeaderCtrl::OnPaint)

    EVT_MOUSE_EVENTS(wxHeaderCtrl::OnMouse)

    EVT_MOUSE_CAPTURE_LOST(wxHeaderCtrl::OnCaptureLost)

    EVT_KEY_DOWN(wxHeaderCtrl::OnKeyDown)
wxEND_EVENT_TABLE()

void wxHeaderCtrl::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    int w, h;
    GetClientSize(&w, &h);

#ifdef __WXGTK__
#endif

    wxAutoBufferedPaintDC dc(this);

    dc.SetBackground(GetBackgroundColour());
    dc.Clear();

        dc.SetDeviceOrigin(m_scrollOffset, 0);

    const unsigned int count = m_numColumns;
    int xpos = 0;
    for ( unsigned int i = 0; i < count; i++ )
    {
        const unsigned idx = m_colIndices[i];
        const wxHeaderColumn& col = GetColumn(idx);
        if ( col.IsHidden() )
            continue;

        int colWidth = col.GetWidth();

        wxHeaderSortIconType sortArrow;
        if ( col.IsSortKey() )
        {
            sortArrow = col.IsSortOrderAscending() ? wxHDR_SORT_ICON_UP
                                                   : wxHDR_SORT_ICON_DOWN;
        }
        else         {
            sortArrow = wxHDR_SORT_ICON_NONE;
        }

        int state = 0;
        if ( IsEnabled() )
        {
            if ( idx == m_hover )
                state = wxCONTROL_CURRENT;
        }
        else         {
            state = wxCONTROL_DISABLED;
        }

        if (i == 0)
           state |= wxCONTROL_SPECIAL;

        wxHeaderButtonParams params;
        params.m_labelText = col.GetTitle();
        params.m_labelBitmap = col.GetBitmap();
        params.m_labelAlignment = col.GetAlignment();

#ifdef __WXGTK__
        if (i == count-1)
        {
            state |= wxCONTROL_DIRTY;
        }
#endif

        wxRendererNative::Get().DrawHeaderButton
                                (
                                    this,
                                    dc,
                                    wxRect(xpos, 0, colWidth, h),
                                    state,
                                    sortArrow,
                                    &params
                                );

        xpos += colWidth;
    }
}

void wxHeaderCtrl::OnCaptureLost(wxMouseCaptureLostEvent& WXUNUSED(event))
{
    if ( IsDragging() )
        CancelDragging();
}

void wxHeaderCtrl::OnKeyDown(wxKeyEvent& event)
{
    if ( event.GetKeyCode() == WXK_ESCAPE )
    {
        if ( IsDragging() )
        {
            ReleaseMouse();
            CancelDragging();

            return;
        }
    }

    event.Skip();
}

void wxHeaderCtrl::OnMouse(wxMouseEvent& mevent)
{
            mevent.Skip();


        const int xPhysical = mevent.GetX();
    const int xLogical = xPhysical - m_scrollOffset;

            if ( IsResizing() )
    {
        if ( mevent.LeftUp() )
            EndResizing(xPhysical);
        else             StartOrContinueResizing(m_colBeingResized, xPhysical);

        return;
    }

    if ( IsReordering() )
    {
        if ( !mevent.LeftUp() )
        {
                        UpdateReorderingMarker(xPhysical);

            return;
        }

                        if ( EndReordering(xPhysical) )
            return;
    }


        bool onSeparator;
    const unsigned col = mevent.Leaving()
                            ? (onSeparator = false, COL_NONE)
                            : FindColumnAtPoint(xLogical, &onSeparator);


        if ( col != m_hover )
    {
        const unsigned hoverOld = m_hover;
        m_hover = col;

        RefreshColIfNotNone(hoverOld);
        RefreshColIfNotNone(m_hover);
    }

        if ( mevent.Moving() )
    {
        SetCursor(onSeparator ? wxCursor(wxCURSOR_SIZEWE) : wxNullCursor);
        return;
    }

        if ( col == COL_NONE )
        return;


        if ( mevent.LeftDown() )
    {
        if ( onSeparator )
        {
                        wxASSERT_MSG( !IsResizing(), "reentering column resize mode?" );
            StartOrContinueResizing(col, xPhysical);
        }
        else         {
                        wxASSERT_MSG( !IsReordering(), "reentering column move mode?" );

            StartReordering(col, xPhysical);
        }

        return;
    }

        wxEventType evtType = wxEVT_NULL;
    const bool click = mevent.ButtonUp(),
               dblclk = mevent.ButtonDClick();
    if ( click || dblclk )
    {
        switch ( mevent.GetButton() )
        {
            case wxMOUSE_BTN_LEFT:
                                if ( onSeparator && dblclk )
                {
                    evtType = wxEVT_HEADER_SEPARATOR_DCLICK;
                }
                else                 {
                    evtType = click ? wxEVT_HEADER_CLICK
                                    : wxEVT_HEADER_DCLICK;
                }
                break;

            case wxMOUSE_BTN_RIGHT:
                evtType = click ? wxEVT_HEADER_RIGHT_CLICK
                                : wxEVT_HEADER_RIGHT_DCLICK;
                break;

            case wxMOUSE_BTN_MIDDLE:
                evtType = click ? wxEVT_HEADER_MIDDLE_CLICK
                                : wxEVT_HEADER_MIDDLE_DCLICK;
                break;

            default:
                                ;
        }
    }

    if ( evtType == wxEVT_NULL )
        return;

    wxHeaderCtrlEvent event(evtType, GetId());
    event.SetEventObject(this);
    event.SetColumn(col);

    if ( GetEventHandler()->ProcessEvent(event) )
        mevent.Skip(false);
}

#endif 
#endif 