


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_HEADERCTRL

#ifndef WX_PRECOMP
    #include "wx/menu.h"
#endif 
#include "wx/headerctrl.h"
#include "wx/rearrangectrl.h"
#include "wx/renderer.h"

namespace
{


const unsigned int wxNO_COLUMN = static_cast<unsigned>(-1);


#if wxUSE_REARRANGECTRL

class wxHeaderColumnsRearrangeDialog : public wxRearrangeDialog
{
public:
    wxHeaderColumnsRearrangeDialog(wxWindow *parent,
                                   const wxArrayInt& order,
                                   const wxArrayString& items)
        : wxRearrangeDialog
          (
            parent,
            _("Please select the columns to show and define their order:"),
            _("Customize Columns"),
            order,
            items
          )
    {
    }
};

#endif 
} 

extern WXDLLIMPEXP_DATA_CORE(const char) wxHeaderCtrlNameStr[] = "wxHeaderCtrl";

wxBEGIN_EVENT_TABLE(wxHeaderCtrlBase, wxControl)
    EVT_HEADER_SEPARATOR_DCLICK(wxID_ANY, wxHeaderCtrlBase::OnSeparatorDClick)
#if wxUSE_MENUS
    EVT_HEADER_RIGHT_CLICK(wxID_ANY, wxHeaderCtrlBase::OnRClick)
#endif wxEND_EVENT_TABLE()

void wxHeaderCtrlBase::ScrollWindow(int dx,
                                    int WXUNUSED_UNLESS_DEBUG(dy),
                                    const wxRect * WXUNUSED_UNLESS_DEBUG(rect))

{
        wxASSERT_MSG( !dy, "header window can't be scrolled vertically" );

            wxASSERT_MSG( !rect, "header window can't be scrolled partially" );

    DoScrollHorz(dx);
}

void wxHeaderCtrlBase::SetColumnCount(unsigned int count)
{
    if ( count != GetColumnCount() )
        OnColumnCountChanging(count);

            DoSetCount(count);
}

int wxHeaderCtrlBase::GetColumnTitleWidth(const wxHeaderColumn& col)
{
    int w = wxWindowBase::GetTextExtent(col.GetTitle()).x;

        w += wxRendererNative::Get().GetHeaderButtonMargin(this);

        wxBitmap bmp = col.GetBitmap();
    if ( bmp.IsOk() )
        w += bmp.GetWidth() + 2;

    return w;
}


void wxHeaderCtrlBase::OnSeparatorDClick(wxHeaderCtrlEvent& event)
{
    const unsigned col = event.GetColumn();
    const wxHeaderColumn& column = GetColumn(col);

    if ( !column.IsResizeable() )
    {
        event.Skip();
        return;
    }

    int w = GetColumnTitleWidth(column);

    if ( !UpdateColumnWidthToFit(col, w) )
        event.Skip();
    else
        UpdateColumn(col);
}

#if wxUSE_MENUS

void wxHeaderCtrlBase::OnRClick(wxHeaderCtrlEvent& event)
{
    if ( !HasFlag(wxHD_ALLOW_HIDE) )
    {
        event.Skip();
        return;
    }

    ShowColumnsMenu(ScreenToClient(wxGetMousePosition()));
}

#endif 

void wxHeaderCtrlBase::SetColumnsOrder(const wxArrayInt& order)
{
    const unsigned count = GetColumnCount();
    wxCHECK_RET( order.size() == count, "wrong number of columns" );

        wxArrayInt seen(count, 0);
    for ( unsigned n = 0; n < count; n++ )
    {
        const unsigned idx = order[n];
        wxCHECK_RET( idx < count, "invalid column index" );
        wxCHECK_RET( !seen[idx], "duplicate column index" );

        seen[idx] = 1;
    }

    DoSetColumnsOrder(order);

    }

void wxHeaderCtrlBase::ResetColumnsOrder()
{
    const unsigned count = GetColumnCount();
    wxArrayInt order(count);
    for ( unsigned n = 0; n < count; n++ )
        order[n] = n;

    DoSetColumnsOrder(order);
}

wxArrayInt wxHeaderCtrlBase::GetColumnsOrder() const
{
    const wxArrayInt order = DoGetColumnsOrder();

    wxASSERT_MSG( order.size() == GetColumnCount(), "invalid order array" );

    return order;
}

unsigned int wxHeaderCtrlBase::GetColumnAt(unsigned int pos) const
{
    wxCHECK_MSG( pos < GetColumnCount(), wxNO_COLUMN, "invalid position" );

    return GetColumnsOrder()[pos];
}

unsigned int wxHeaderCtrlBase::GetColumnPos(unsigned int idx) const
{
    const unsigned count = GetColumnCount();

    wxCHECK_MSG( idx < count, wxNO_COLUMN, "invalid index" );

    const wxArrayInt order = GetColumnsOrder();
    int pos = order.Index(idx);
    wxCHECK_MSG( pos != wxNOT_FOUND, wxNO_COLUMN, "column unexpectedly not displayed at all" );

    return (unsigned int)pos;
}


void wxHeaderCtrlBase::MoveColumnInOrderArray(wxArrayInt& order,
                                              unsigned int idx,
                                              unsigned int pos)
{
    int posOld = order.Index(idx);
    wxASSERT_MSG( posOld != wxNOT_FOUND, "invalid index" );

    if ( pos != (unsigned int)posOld )
    {
        order.RemoveAt(posOld);
        order.Insert(idx, pos);
    }
}

void
wxHeaderCtrlBase::DoResizeColumnIndices(wxArrayInt& colIndices, unsigned int count)
{
        const unsigned countOld = colIndices.size();
    if ( count > countOld )
    {
                for ( unsigned n = countOld; n < count; n++ )
            colIndices.push_back(n);
    }
    else if ( count < countOld )
    {
                        wxArrayInt colIndicesNew;
        colIndicesNew.reserve(count);
        for ( unsigned n = 0; n < countOld; n++ )
        {
            const unsigned idx = colIndices[n];
            if ( idx < count )
                colIndicesNew.push_back(idx);
        }

        colIndices.swap(colIndicesNew);
    }
    
    wxASSERT_MSG( colIndices.size() == count, "logic error" );
}


#if wxUSE_MENUS

void wxHeaderCtrlBase::AddColumnsItems(wxMenu& menu, int idColumnsBase)
{
    const unsigned count = GetColumnCount();
    for ( unsigned n = 0; n < count; n++ )
    {
        const wxHeaderColumn& col = GetColumn(n);
        menu.AppendCheckItem(idColumnsBase + n, col.GetTitle());
        if ( col.IsShown() )
            menu.Check(n, true);
    }
}

bool wxHeaderCtrlBase::ShowColumnsMenu(const wxPoint& pt, const wxString& title)
{
        wxMenu menu;
    if ( !title.empty() )
        menu.SetTitle(title);

    AddColumnsItems(menu);

            const unsigned count = GetColumnCount();
    if ( HasFlag(wxHD_ALLOW_REORDER) )
    {
        menu.AppendSeparator();
        menu.Append(count, _("&Customize..."));
    }

        const int rc = GetPopupMenuSelectionFromUser(menu, pt);
    if ( rc == wxID_NONE )
        return false;

    if ( static_cast<unsigned>(rc) == count )
    {
        return ShowCustomizeDialog();
    }
    else     {
        UpdateColumnVisibility(rc, !GetColumn(rc).IsShown());
    }

    return true;
}

#endif 
bool wxHeaderCtrlBase::ShowCustomizeDialog()
{
#if wxUSE_REARRANGECTRL
        wxArrayInt order = GetColumnsOrder();

    const unsigned count = GetColumnCount();

            wxArrayString titles;
    titles.reserve(count);
    for ( unsigned n = 0; n < count; n++ )
        titles.push_back(GetColumn(n).GetTitle());

        unsigned pos;
    for ( pos = 0; pos < count; pos++ )
    {
        int& idx = order[pos];
        if ( GetColumn(idx).IsHidden() )
        {
                        idx = ~idx;
        }
    }

        wxHeaderColumnsRearrangeDialog dlg(this, order, titles);
    if ( dlg.ShowModal() == wxID_OK )
    {
                order = dlg.GetOrder();
        for ( pos = 0; pos < count; pos++ )
        {
            int& idx = order[pos];
            const bool show = idx >= 0;
            if ( !show )
            {
                                idx = ~idx;
            }

            if ( show != GetColumn(idx).IsShown() )
                UpdateColumnVisibility(idx, show);
        }

        UpdateColumnsOrder(order);
        SetColumnsOrder(order);

        return true;
    }
#endif 
    return false;
}


void wxHeaderCtrlSimple::Init()
{
    m_sortKey = wxNO_COLUMN;
}

const wxHeaderColumn& wxHeaderCtrlSimple::GetColumn(unsigned int idx) const
{
    return m_cols[idx];
}

void wxHeaderCtrlSimple::DoInsert(const wxHeaderColumnSimple& col, unsigned int idx)
{
    m_cols.insert(m_cols.begin() + idx, col);

    UpdateColumnCount();
}

void wxHeaderCtrlSimple::DoDelete(unsigned int idx)
{
    m_cols.erase(m_cols.begin() + idx);
    if ( idx == m_sortKey )
        m_sortKey = wxNO_COLUMN;

    UpdateColumnCount();
}

void wxHeaderCtrlSimple::DeleteAllColumns()
{
    m_cols.clear();
    m_sortKey = wxNO_COLUMN;

    UpdateColumnCount();
}


void wxHeaderCtrlSimple::DoShowColumn(unsigned int idx, bool show)
{
    if ( show != m_cols[idx].IsShown() )
    {
        m_cols[idx].SetHidden(!show);

        UpdateColumn(idx);
    }
}

void wxHeaderCtrlSimple::DoShowSortIndicator(unsigned int idx, bool ascending)
{
    RemoveSortIndicator();

    m_cols[idx].SetSortOrder(ascending);
    m_sortKey = idx;

    UpdateColumn(idx);
}

void wxHeaderCtrlSimple::RemoveSortIndicator()
{
    if ( m_sortKey != wxNO_COLUMN )
    {
        const unsigned sortOld = m_sortKey;
        m_sortKey = wxNO_COLUMN;

        m_cols[sortOld].UnsetAsSortKey();

        UpdateColumn(sortOld);
    }
}

bool
wxHeaderCtrlSimple::UpdateColumnWidthToFit(unsigned int idx, int widthTitle)
{
    const int widthContents = GetBestFittingWidth(idx);
    if ( widthContents == -1 )
        return false;

    m_cols[idx].SetWidth(wxMax(widthContents, widthTitle));

    return true;
}


wxIMPLEMENT_DYNAMIC_CLASS(wxHeaderCtrlEvent, wxNotifyEvent);

wxDEFINE_EVENT( wxEVT_HEADER_CLICK, wxHeaderCtrlEvent);
wxDEFINE_EVENT( wxEVT_HEADER_RIGHT_CLICK, wxHeaderCtrlEvent);
wxDEFINE_EVENT( wxEVT_HEADER_MIDDLE_CLICK, wxHeaderCtrlEvent);

wxDEFINE_EVENT( wxEVT_HEADER_DCLICK, wxHeaderCtrlEvent);
wxDEFINE_EVENT( wxEVT_HEADER_RIGHT_DCLICK, wxHeaderCtrlEvent);
wxDEFINE_EVENT( wxEVT_HEADER_MIDDLE_DCLICK, wxHeaderCtrlEvent);

wxDEFINE_EVENT( wxEVT_HEADER_SEPARATOR_DCLICK, wxHeaderCtrlEvent);

wxDEFINE_EVENT( wxEVT_HEADER_BEGIN_RESIZE, wxHeaderCtrlEvent);
wxDEFINE_EVENT( wxEVT_HEADER_RESIZING, wxHeaderCtrlEvent);
wxDEFINE_EVENT( wxEVT_HEADER_END_RESIZE, wxHeaderCtrlEvent);

wxDEFINE_EVENT( wxEVT_HEADER_BEGIN_REORDER, wxHeaderCtrlEvent);
wxDEFINE_EVENT( wxEVT_HEADER_END_REORDER, wxHeaderCtrlEvent);

wxDEFINE_EVENT( wxEVT_HEADER_DRAGGING_CANCELLED, wxHeaderCtrlEvent);

#endif 