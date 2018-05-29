


#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/dc.h"
    #include "wx/sizer.h"
#endif

#include "wx/vscroll.h"

#include "wx/utils.h"   


class WXDLLEXPORT wxVarScrollHelperEvtHandler : public wxEvtHandler
{
public:
    wxVarScrollHelperEvtHandler(wxVarScrollHelperBase *scrollHelper)
    {
        m_scrollHelper = scrollHelper;
    }

    virtual bool ProcessEvent(wxEvent& event) wxOVERRIDE;

private:
    wxVarScrollHelperBase *m_scrollHelper;

    wxDECLARE_NO_COPY_CLASS(wxVarScrollHelperEvtHandler);
};


bool wxVarScrollHelperEvtHandler::ProcessEvent(wxEvent& event)
{
    wxEventType evType = event.GetEventType();

                            bool processed = m_nextHandler->ProcessEvent(event);

                                if ( evType == wxEVT_SIZE )
    {
        m_scrollHelper->HandleOnSize((wxSizeEvent &)event);
        return true;
    }

                if (
#ifndef __WXUNIVERSAL__
                                                            !processed &&
#endif             evType == wxEVT_PAINT )
    {
        m_scrollHelper->HandleOnPaint((wxPaintEvent &)event);
        return true;
    }

            if ( processed )
        return true;

            bool wasSkipped = event.GetSkipped();
    if ( wasSkipped )
        event.Skip(false);

    if ( evType == wxEVT_SCROLLWIN_TOP ||
         evType == wxEVT_SCROLLWIN_BOTTOM ||
         evType == wxEVT_SCROLLWIN_LINEUP ||
         evType == wxEVT_SCROLLWIN_LINEDOWN ||
         evType == wxEVT_SCROLLWIN_PAGEUP ||
         evType == wxEVT_SCROLLWIN_PAGEDOWN ||
         evType == wxEVT_SCROLLWIN_THUMBTRACK ||
         evType == wxEVT_SCROLLWIN_THUMBRELEASE )
    {
        m_scrollHelper->HandleOnScroll((wxScrollWinEvent &)event);
        if ( !event.GetSkipped() )
        {
                                                            processed = true;
            wasSkipped = false;
        }
    }
#if wxUSE_MOUSEWHEEL
    #ifndef __WXGTK20__
    else if ( evType == wxEVT_MOUSEWHEEL )
    {
        m_scrollHelper->HandleOnMouseWheel((wxMouseEvent &)event);
        return true;
    }
#endif
#endif     else if ( evType == wxEVT_CHAR &&
                (m_scrollHelper->GetOrientation() == wxVERTICAL) )
    {
        m_scrollHelper->HandleOnChar((wxKeyEvent &)event);
        if ( !event.GetSkipped() )
        {
            processed = true;
            wasSkipped = false;
        }
    }

    event.Skip(wasSkipped);

                                        event.DidntHonourProcessOnlyIn();

    return processed;
}




wxVarScrollHelperBase::wxVarScrollHelperBase(wxWindow *win)
    : wxAnyScrollHelperBase(win)
{
#if wxUSE_MOUSEWHEEL
    m_sumWheelRotation = 0;
#endif

    m_unitMax = 0;
    m_sizeTotal = 0;
    m_unitFirst = 0;

    m_physicalScrolling = true;
    m_handler = NULL;

        DoSetTargetWindow(win);
}

wxVarScrollHelperBase::~wxVarScrollHelperBase()
{
    DeleteEvtHandler();
}


void
wxVarScrollHelperBase::AssignOrient(wxCoord& x,
                                    wxCoord& y,
                                    wxCoord first,
                                    wxCoord second)
{
    if ( GetOrientation() == wxVERTICAL )
    {
        x = first;
        y = second;
    }
    else     {
        x = second;
        y = first;
    }
}

void
wxVarScrollHelperBase::IncOrient(wxCoord& x, wxCoord& y, wxCoord inc)
{
    if ( GetOrientation() == wxVERTICAL )
        y += inc;
    else
        x += inc;
}

wxCoord wxVarScrollHelperBase::DoEstimateTotalSize() const
{
                    static const size_t NUM_UNITS_TO_SAMPLE = 10;

    wxCoord sizeTotal;
    if ( m_unitMax < 3*NUM_UNITS_TO_SAMPLE )
    {
                        sizeTotal = GetUnitsSize(0, m_unitMax);
    }
    else     {
                sizeTotal =
            GetUnitsSize(0, NUM_UNITS_TO_SAMPLE) +
                GetUnitsSize(m_unitMax - NUM_UNITS_TO_SAMPLE,
                             m_unitMax) +
                    GetUnitsSize(m_unitMax/2 - NUM_UNITS_TO_SAMPLE/2,
                                 m_unitMax/2 + NUM_UNITS_TO_SAMPLE/2);

                sizeTotal = (wxCoord)
                (((float)sizeTotal / (3*NUM_UNITS_TO_SAMPLE)) * m_unitMax);
    }

    return sizeTotal;
}

wxCoord wxVarScrollHelperBase::GetUnitsSize(size_t unitMin, size_t unitMax) const
{
    if ( unitMin == unitMax )
        return 0;
    else if ( unitMin > unitMax )
        return -GetUnitsSize(unitMax, unitMin);
    
        OnGetUnitsSizeHint(unitMin, unitMax);

        wxCoord size = 0;
    for ( size_t unit = unitMin; unit < unitMax; ++unit )
    {
        size += OnGetUnitSize(unit);
    }

    return size;
}

size_t wxVarScrollHelperBase::FindFirstVisibleFromLast(size_t unitLast, bool full) const
{
    const wxCoord sWindow = GetOrientationTargetSize();

            size_t unitFirst = unitLast;
    wxCoord s = 0;
    for ( ;; )
    {
        s += OnGetUnitSize(unitFirst);

        if ( s > sWindow )
        {
                                                if ( full )
            {
                ++unitFirst;
            }

            break;
        }

        if ( !unitFirst )
            break;

        --unitFirst;
    }

    return unitFirst;
}

size_t wxVarScrollHelperBase::GetNewScrollPosition(wxScrollWinEvent& event) const
{
    wxEventType evtType = event.GetEventType();

    if ( evtType == wxEVT_SCROLLWIN_TOP )
    {
        return 0;
    }
    else if ( evtType == wxEVT_SCROLLWIN_BOTTOM )
    {
        return m_unitMax;
    }
    else if ( evtType == wxEVT_SCROLLWIN_LINEUP )
    {
        return m_unitFirst ? m_unitFirst - 1 : 0;
    }
    else if ( evtType == wxEVT_SCROLLWIN_LINEDOWN )
    {
        return m_unitFirst + 1;
    }
    else if ( evtType == wxEVT_SCROLLWIN_PAGEUP )
    {
                return wxMin(FindFirstVisibleFromLast(m_unitFirst),
                    m_unitFirst ? m_unitFirst - 1 : 0);
    }
    else if ( evtType == wxEVT_SCROLLWIN_PAGEDOWN )
    {
                if ( GetVisibleEnd() )
            return wxMax(GetVisibleEnd() - 1, m_unitFirst + 1);
        else
            return wxMax(GetVisibleEnd(), m_unitFirst + 1);
    }
    else if ( evtType == wxEVT_SCROLLWIN_THUMBRELEASE )
    {
        return event.GetPosition();
    }
    else if ( evtType == wxEVT_SCROLLWIN_THUMBTRACK )
    {
        return event.GetPosition();
    }

        wxFAIL_MSG( wxT("unknown scroll event type?") );
    return 0;
}

void wxVarScrollHelperBase::UpdateScrollbar()
{
        if ( !m_unitMax )
    {
        RemoveScrollbar();
        return;
    }

        const wxCoord sWindow = GetOrientationTargetSize();

        wxCoord s = 0;
    size_t unit;
    for ( unit = m_unitFirst; unit < m_unitMax; ++unit )
    {
        if ( s > sWindow )
            break;

        s += OnGetUnitSize(unit);
    }

    m_nUnitsVisible = unit - m_unitFirst;

    int unitsPageSize = m_nUnitsVisible;
    if ( s > sWindow )
    {
                                --unitsPageSize;
    }

        m_win->SetScrollbar(GetOrientation(), m_unitFirst, unitsPageSize, m_unitMax);
}

void wxVarScrollHelperBase::RemoveScrollbar()
{
    m_unitFirst = 0;
    m_nUnitsVisible = m_unitMax;
    m_win->SetScrollbar(GetOrientation(), 0, 0, 0);
}

void wxVarScrollHelperBase::DeleteEvtHandler()
{
        if ( m_win && m_handler )
    {
        if ( m_win->RemoveEventHandler(m_handler) )
        {
            delete m_handler;
        }
                
        m_handler = NULL;
    }
}

void wxVarScrollHelperBase::DoSetTargetWindow(wxWindow *target)
{
    m_targetWindow = target;
#ifdef __WXMAC__
    target->MacSetClipChildren( true ) ;
#endif

                if ( m_targetWindow == m_win )
    {
                DeleteEvtHandler();

        m_handler = new wxVarScrollHelperEvtHandler(this);
        m_targetWindow->PushEventHandler(m_handler);
    }
}


void wxVarScrollHelperBase::SetTargetWindow(wxWindow *target)
{
    wxCHECK_RET( target, wxT("target window must not be NULL") );

    if ( target == m_targetWindow )
        return;

    DoSetTargetWindow(target);
}

void wxVarScrollHelperBase::SetUnitCount(size_t count)
{
        m_unitMax = count;

        m_sizeTotal = EstimateTotalSize();

            size_t oldScrollPos = m_unitFirst;
    DoScrollToUnit(m_unitFirst);
    if ( oldScrollPos == m_unitFirst )
    {
                        UpdateScrollbar();
    }
}

void wxVarScrollHelperBase::RefreshUnit(size_t unit)
{
        if ( !IsVisible(unit) )
    {
                return;
    }

        wxRect rect;
    AssignOrient(rect.width, rect.height,
                 GetNonOrientationTargetSize(), OnGetUnitSize(unit));

    for ( size_t n = GetVisibleBegin(); n < unit; ++n )
    {
        IncOrient(rect.x, rect.y, OnGetUnitSize(n));
    }

        m_targetWindow->RefreshRect(rect);
}

void wxVarScrollHelperBase::RefreshUnits(size_t from, size_t to)
{
    wxASSERT_MSG( from <= to, wxT("RefreshUnits(): empty range") );

            if ( from < GetVisibleBegin() )
        from = GetVisibleBegin();

    if ( to > GetVisibleEnd() )
        to = GetVisibleEnd();

        int orient_size = 0,
        orient_pos = 0;

    int nonorient_size = GetNonOrientationTargetSize();

    for ( size_t nBefore = GetVisibleBegin();
          nBefore < from;
          nBefore++ )
    {
        orient_pos += OnGetUnitSize(nBefore);
    }

    for ( size_t nBetween = from; nBetween <= to; nBetween++ )
    {
        orient_size += OnGetUnitSize(nBetween);
    }

    wxRect rect;
    AssignOrient(rect.x, rect.y, 0, orient_pos);
    AssignOrient(rect.width, rect.height, nonorient_size, orient_size);

        m_targetWindow->RefreshRect(rect);
}

void wxVarScrollHelperBase::RefreshAll()
{
    UpdateScrollbar();

    m_targetWindow->Refresh();
}

bool wxVarScrollHelperBase::ScrollLayout()
{
    if ( m_targetWindow->GetSizer() && m_physicalScrolling )
    {
                
        int x, y;
        AssignOrient(x, y, 0, -GetScrollOffset());

        int w, h;
        m_targetWindow->GetVirtualSize(&w, &h);

        m_targetWindow->GetSizer()->SetDimension(x, y, w, h);
        return true;
    }

        return m_targetWindow->wxWindow::Layout();
}

int wxVarScrollHelperBase::VirtualHitTest(wxCoord coord) const
{
    const size_t unitMax = GetVisibleEnd();
    for ( size_t unit = GetVisibleBegin(); unit < unitMax; ++unit )
    {
        coord -= OnGetUnitSize(unit);
        if ( coord < 0 )
            return unit;
    }

    return wxNOT_FOUND;
}


bool wxVarScrollHelperBase::DoScrollToUnit(size_t unit)
{
    if ( !m_unitMax )
    {
                return false;
    }

            size_t unitFirstLast = FindFirstVisibleFromLast(m_unitMax - 1, true);
    if ( unit > unitFirstLast )
        unit = unitFirstLast;

        if ( unit == m_unitFirst )
    {
                return false;
    }


        size_t unitFirstOld = GetVisibleBegin(),
           unitLastOld = GetVisibleEnd();

    m_unitFirst = unit;


        UpdateScrollbar();

                if ( m_targetWindow->GetChildren().empty() &&
         (GetVisibleBegin() >= unitLastOld || GetVisibleEnd() <= unitFirstOld) )
    {
                        m_targetWindow->Refresh();
    }
    else     {
        #ifdef __WXMAC__
        if (m_physicalScrolling && m_targetWindow->IsShownOnScreen())
#else
        if ( m_physicalScrolling )
#endif
        {
            wxCoord dx = 0,
                    dy = GetUnitsSize(GetVisibleBegin(), unitFirstOld);

            if ( GetOrientation() == wxHORIZONTAL )
            {
                wxCoord tmp = dx;
                dx = dy;
                dy = tmp;
            }

            m_targetWindow->ScrollWindow(dx, dy);
        }
        else         {
                                                            m_targetWindow->Refresh();
        }
    }

    return true;
}

bool wxVarScrollHelperBase::DoScrollUnits(int units)
{
    units += m_unitFirst;
    if ( units < 0 )
        units = 0;

    return DoScrollToUnit(units);
}

bool wxVarScrollHelperBase::DoScrollPages(int pages)
{
    bool didSomething = false;

    while ( pages )
    {
        int unit;
        if ( pages > 0 )
        {
            unit = GetVisibleEnd();
            if ( unit )
                --unit;
            --pages;
        }
        else         {
            unit = FindFirstVisibleFromLast(GetVisibleEnd());
            ++pages;
        }

        didSomething = DoScrollToUnit(unit);
    }

    return didSomething;
}


void wxVarScrollHelperBase::HandleOnSize(wxSizeEvent& event)
{
    if ( m_unitMax )
    {
                        
                const wxCoord sWindow = GetOrientationTargetSize();
        wxCoord s = 0;
        size_t unit;
        for ( unit = m_unitFirst; unit < m_unitMax; ++unit )
        {
            if ( s > sWindow )
                break;

            s += OnGetUnitSize(unit);
        }
        wxCoord freeSpace = sWindow - s;

                size_t idealUnitFirst;
        for ( idealUnitFirst = m_unitFirst;
              idealUnitFirst > 0;
              idealUnitFirst-- )
        {
            wxCoord us = OnGetUnitSize(idealUnitFirst-1);
            if ( freeSpace < us )
                break;
            freeSpace -= us;
        }
        m_unitFirst = idealUnitFirst;
    }

    UpdateScrollbar();

    event.Skip();
}

void wxVarScrollHelperBase::HandleOnScroll(wxScrollWinEvent& event)
{
    if (GetOrientation() != event.GetOrientation())
    {
        event.Skip();
        return;
    }

    DoScrollToUnit(GetNewScrollPosition(event));

#ifdef __WXMAC__
    UpdateMacScrollWindow();
#endif }

void wxVarScrollHelperBase::DoPrepareDC(wxDC& dc)
{
    if ( m_physicalScrolling )
    {
        wxPoint pt = dc.GetDeviceOrigin();

        IncOrient(pt.x, pt.y, -GetScrollOffset());

        dc.SetDeviceOrigin(pt.x, pt.y);
    }
}

int wxVarScrollHelperBase::DoCalcScrolledPosition(int coord) const
{
    return coord - GetScrollOffset();
}

int wxVarScrollHelperBase::DoCalcUnscrolledPosition(int coord) const
{
    return coord + GetScrollOffset();
}

#if wxUSE_MOUSEWHEEL

void wxVarScrollHelperBase::HandleOnMouseWheel(wxMouseEvent& event)
{
                    if ( GetOrientation() != wxVERTICAL )
        return;

    m_sumWheelRotation += event.GetWheelRotation();
    int delta = event.GetWheelDelta();

        int units_to_scroll = -(m_sumWheelRotation/delta);
    if ( !units_to_scroll )
        return;

    m_sumWheelRotation += units_to_scroll*delta;

    if ( !event.IsPageScroll() )
        DoScrollUnits( units_to_scroll*event.GetLinesPerAction() );
    else         DoScrollPages( units_to_scroll );
}

#endif 



void wxVarHVScrollHelper::SetRowColumnCount(size_t rowCount, size_t columnCount)
{
    SetRowCount(rowCount);
    SetColumnCount(columnCount);
}

bool wxVarHVScrollHelper::ScrollToRowColumn(size_t row, size_t column)
{
    bool result = false;
    result |= ScrollToRow(row);
    result |= ScrollToColumn(column);
    return result;
}

void wxVarHVScrollHelper::RefreshRowColumn(size_t row, size_t column)
{
        if ( !IsRowVisible(row) || !IsColumnVisible(column) )
    {
                return;
    }

        wxRect v_rect, h_rect;
    v_rect.height = OnGetRowHeight(row);
    h_rect.width = OnGetColumnWidth(column);

    size_t n;

    for ( n = GetVisibleRowsBegin(); n < row; n++ )
    {
        v_rect.y += OnGetRowHeight(n);
    }

    for ( n = GetVisibleColumnsBegin(); n < column; n++ )
    {
        h_rect.x += OnGetColumnWidth(n);
    }

        if ( wxVarVScrollHelper::GetTargetWindow() == wxVarHScrollHelper::GetTargetWindow() )
    {
        v_rect.x = h_rect.x;
        v_rect.width = h_rect.width;
        wxVarVScrollHelper::GetTargetWindow()->RefreshRect(v_rect);
    }
    else
    {
        v_rect.x = 0;
        v_rect.width = wxVarVScrollHelper::GetNonOrientationTargetSize();
        h_rect.y = 0;
        h_rect.width = wxVarHScrollHelper::GetNonOrientationTargetSize();

        wxVarVScrollHelper::GetTargetWindow()->RefreshRect(v_rect);
        wxVarHScrollHelper::GetTargetWindow()->RefreshRect(h_rect);
    }
}

void wxVarHVScrollHelper::RefreshRowsColumns(size_t fromRow, size_t toRow,
                                             size_t fromColumn, size_t toColumn)
{
    wxASSERT_MSG( fromRow <= toRow || fromColumn <= toColumn,
        wxT("RefreshRowsColumns(): empty range") );

            if ( fromRow < GetVisibleRowsBegin() )
        fromRow = GetVisibleRowsBegin();

    if ( toRow > GetVisibleRowsEnd() )
        toRow = GetVisibleRowsEnd();

    if ( fromColumn < GetVisibleColumnsBegin() )
        fromColumn = GetVisibleColumnsBegin();

    if ( toColumn > GetVisibleColumnsEnd() )
        toColumn = GetVisibleColumnsEnd();

        wxRect v_rect, h_rect;
    size_t nBefore, nBetween;

    for ( nBefore = GetVisibleRowsBegin();
          nBefore < fromRow;
          nBefore++ )
    {
        v_rect.y += OnGetRowHeight(nBefore);
    }

    for ( nBetween = fromRow; nBetween <= toRow; nBetween++ )
    {
        v_rect.height += OnGetRowHeight(nBetween);
    }

    for ( nBefore = GetVisibleColumnsBegin();
          nBefore < fromColumn;
          nBefore++ )
    {
        h_rect.x += OnGetColumnWidth(nBefore);
    }

    for ( nBetween = fromColumn; nBetween <= toColumn; nBetween++ )
    {
        h_rect.width += OnGetColumnWidth(nBetween);
    }

        if ( wxVarVScrollHelper::GetTargetWindow() == wxVarHScrollHelper::GetTargetWindow() )
    {
        v_rect.x = h_rect.x;
        v_rect.width = h_rect.width;
        wxVarVScrollHelper::GetTargetWindow()->RefreshRect(v_rect);
    }
    else
    {
        v_rect.x = 0;
        v_rect.width = wxVarVScrollHelper::GetNonOrientationTargetSize();
        h_rect.y = 0;
        h_rect.width = wxVarHScrollHelper::GetNonOrientationTargetSize();

        wxVarVScrollHelper::GetTargetWindow()->RefreshRect(v_rect);
        wxVarHScrollHelper::GetTargetWindow()->RefreshRect(h_rect);
    }
}

wxPosition wxVarHVScrollHelper::VirtualHitTest(wxCoord x, wxCoord y) const
{
    return wxPosition(wxVarVScrollHelper::VirtualHitTest(y),
                      wxVarHScrollHelper::VirtualHitTest(x));
}

void wxVarHVScrollHelper::DoPrepareDC(wxDC& dc)
{
    wxVarVScrollHelper::DoPrepareDC(dc);
    wxVarHScrollHelper::DoPrepareDC(dc);
}

bool wxVarHVScrollHelper::ScrollLayout()
{
    bool layout_result = false;
    layout_result |= wxVarVScrollHelper::ScrollLayout();
    layout_result |= wxVarHScrollHelper::ScrollLayout();
    return layout_result;
}

wxSize wxVarHVScrollHelper::GetRowColumnCount() const
{
    return wxSize(GetColumnCount(), GetRowCount());
}

wxPosition wxVarHVScrollHelper::GetVisibleBegin() const
{
    return wxPosition(GetVisibleRowsBegin(), GetVisibleColumnsBegin());
}

wxPosition wxVarHVScrollHelper::GetVisibleEnd() const
{
    return wxPosition(GetVisibleRowsEnd(), GetVisibleColumnsEnd());
}

bool wxVarHVScrollHelper::IsVisible(size_t row, size_t column) const
{
    return IsRowVisible(row) && IsColumnVisible(column);
}



wxIMPLEMENT_ABSTRACT_CLASS(wxVScrolledWindow, wxPanel);
wxIMPLEMENT_ABSTRACT_CLASS(wxHScrolledWindow, wxPanel);
wxIMPLEMENT_ABSTRACT_CLASS(wxHVScrolledWindow, wxPanel);


#if WXWIN_COMPATIBILITY_2_8


size_t wxVarVScrollLegacyAdaptor::GetFirstVisibleLine() const
{ return GetVisibleRowsBegin(); }

size_t wxVarVScrollLegacyAdaptor::GetLastVisibleLine() const
{ return GetVisibleRowsEnd() - 1; }

size_t wxVarVScrollLegacyAdaptor::GetLineCount() const
{ return GetRowCount(); }

void wxVarVScrollLegacyAdaptor::SetLineCount(size_t count)
{ SetRowCount(count); }

void wxVarVScrollLegacyAdaptor::RefreshLine(size_t line)
{ RefreshRow(line); }

void wxVarVScrollLegacyAdaptor::RefreshLines(size_t from, size_t to)
{ RefreshRows(from, to); }

bool wxVarVScrollLegacyAdaptor::ScrollToLine(size_t line)
{ return ScrollToRow(line); }

bool wxVarVScrollLegacyAdaptor::ScrollLines(int lines)
{ return ScrollRows(lines); }

bool wxVarVScrollLegacyAdaptor::ScrollPages(int pages)
{ return ScrollRowPages(pages); }

wxCoord wxVarVScrollLegacyAdaptor::OnGetLineHeight(size_t WXUNUSED(n)) const
{
    wxFAIL_MSG( wxT("OnGetLineHeight() must be overridden if OnGetRowHeight() isn't!") );
    return -1;
}

void wxVarVScrollLegacyAdaptor::OnGetLinesHint(size_t WXUNUSED(lineMin),
                                               size_t WXUNUSED(lineMax)) const
{
}

wxCoord wxVarVScrollLegacyAdaptor::OnGetRowHeight(size_t n) const
{
    return OnGetLineHeight(n);
}

void wxVarVScrollLegacyAdaptor::OnGetRowsHeightHint(size_t rowMin,
                                                    size_t rowMax) const
{
    OnGetLinesHint(rowMin, rowMax);
}

#endif 