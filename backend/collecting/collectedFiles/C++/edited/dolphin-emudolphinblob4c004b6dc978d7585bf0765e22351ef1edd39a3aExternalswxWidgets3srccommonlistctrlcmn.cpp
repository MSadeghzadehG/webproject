


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_LISTCTRL

#include "wx/listctrl.h"

#ifndef WX_PRECOMP
    #include "wx/dcclient.h"
#endif

const char wxListCtrlNameStr[] = "listCtrl";

wxDEFINE_EVENT( wxEVT_LIST_BEGIN_DRAG, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_BEGIN_RDRAG, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_BEGIN_LABEL_EDIT, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_END_LABEL_EDIT, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_DELETE_ITEM, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_DELETE_ALL_ITEMS, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_ITEM_SELECTED, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_ITEM_DESELECTED, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_KEY_DOWN, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_INSERT_ITEM, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_COL_CLICK, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_COL_RIGHT_CLICK, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_COL_BEGIN_DRAG, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_COL_DRAGGING, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_COL_END_DRAG, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_ITEM_RIGHT_CLICK, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_ITEM_MIDDLE_CLICK, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_ITEM_ACTIVATED, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_ITEM_FOCUSED, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_ITEM_CHECKED, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_ITEM_UNCHECKED, wxListEvent );
wxDEFINE_EVENT( wxEVT_LIST_CACHE_HINT, wxListEvent );


wxDEFINE_FLAGS( wxListCtrlStyle )
wxBEGIN_FLAGS( wxListCtrlStyle )
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

wxFLAGS_MEMBER(wxLC_LIST)
wxFLAGS_MEMBER(wxLC_REPORT)
wxFLAGS_MEMBER(wxLC_ICON)
wxFLAGS_MEMBER(wxLC_SMALL_ICON)
wxFLAGS_MEMBER(wxLC_ALIGN_TOP)
wxFLAGS_MEMBER(wxLC_ALIGN_LEFT)
wxFLAGS_MEMBER(wxLC_AUTOARRANGE)
wxFLAGS_MEMBER(wxLC_USER_TEXT)
wxFLAGS_MEMBER(wxLC_EDIT_LABELS)
wxFLAGS_MEMBER(wxLC_NO_HEADER)
wxFLAGS_MEMBER(wxLC_SINGLE_SEL)
wxFLAGS_MEMBER(wxLC_SORT_ASCENDING)
wxFLAGS_MEMBER(wxLC_SORT_DESCENDING)
wxFLAGS_MEMBER(wxLC_VIRTUAL)
wxEND_FLAGS( wxListCtrlStyle )

#if ((!defined(__WXMSW__) && !defined(__WXQT__) && !(defined(__WXMAC__) && wxOSX_USE_CARBON)) || defined(__WXUNIVERSAL__))
wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxListCtrl, wxGenericListCtrl, "wx/listctrl.h");
#else
wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxListCtrl, wxControl, "wx/listctrl.h");
#endif

wxBEGIN_PROPERTIES_TABLE(wxListCtrl)
wxEVENT_PROPERTY( TextUpdated, wxEVT_TEXT, wxCommandEvent )

wxPROPERTY_FLAGS( WindowStyle, wxListCtrlStyle, long, SetWindowStyleFlag, \
                 GetWindowStyleFlag, wxEMPTY_PARAMETER_VALUE, 0 , \
                 wxT("Helpstring"), wxT("group")) wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxListCtrl)

wxCONSTRUCTOR_5( wxListCtrl, wxWindow*, Parent, wxWindowID, Id, \
                wxPoint, Position, wxSize, Size, long, WindowStyle )



wxIMPLEMENT_DYNAMIC_CLASS(wxListView, wxListCtrl);
wxIMPLEMENT_DYNAMIC_CLASS(wxListItem, wxObject);
wxIMPLEMENT_DYNAMIC_CLASS(wxListEvent, wxNotifyEvent);


long
wxListCtrlBase::AppendColumn(const wxString& heading,
                             wxListColumnFormat format,
                             int width)
{
    return InsertColumn(GetColumnCount(), heading, format, width);
}

long
wxListCtrlBase::InsertColumn(long col,
                             const wxString& heading,
                             int format,
                             int width)
{
    wxListItem item;
    item.m_mask = wxLIST_MASK_TEXT | wxLIST_MASK_FORMAT;
    item.m_text = heading;
    if ( width >= 0
            || width == wxLIST_AUTOSIZE
                || width == wxLIST_AUTOSIZE_USEHEADER )
    {
        item.m_mask |= wxLIST_MASK_WIDTH;
        item.m_width = width;
    }
    item.m_format = format;

    return InsertColumn(col, item);
}

long wxListCtrlBase::InsertColumn(long col, const wxListItem& info)
{
    long rc = DoInsertColumn(col, info);
    if ( rc != -1 )
    {
                        InvalidateBestSize();
    }

    return rc;
}

wxSize wxListCtrlBase::DoGetBestClientSize() const
{
            if ( !InReportView() )
        return wxControl::DoGetBestClientSize();

    int totalWidth;
    wxClientDC dc(const_cast<wxListCtrlBase*>(this));

                                        const int columns = GetColumnCount();
    if ( HasFlag(wxLC_NO_HEADER) || !columns )
    {
                totalWidth = 50*dc.GetCharWidth();
    }
    else     {
        totalWidth = 0;
        for ( int col = 0; col < columns; col++ )
        {
            totalWidth += GetColumnWidth(col);
        }
    }

        return wxSize(totalWidth, 10*dc.GetCharHeight());
}

void wxListCtrlBase::SetAlternateRowColour(const wxColour& colour)
{
    wxASSERT(HasFlag(wxLC_VIRTUAL));
    m_alternateRowColour.SetBackgroundColour(colour);
}

void wxListCtrlBase::EnableAlternateRowColours(bool enable)
{
    if ( enable )
    {
        
                        const wxColour bgColour = GetBackgroundColour();

                        int alpha = bgColour.GetRGB() > 0x808080 ? 97 : 150;
        SetAlternateRowColour(bgColour.ChangeLightness(alpha));
    }
    else     {
        SetAlternateRowColour(wxColour());
    }
}

wxListItemAttr *wxListCtrlBase::OnGetItemAttr(long item) const
{
    return (m_alternateRowColour.GetBackgroundColour().IsOk() && (item % 2))
        ? wxConstCast(&m_alternateRowColour, wxListItemAttr)
        : NULL; }

#endif 