


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_LISTBOOK

#include "wx/listbook.h"

#ifndef WX_PRECOMP
    #include "wx/settings.h"
#endif

#include "wx/listctrl.h"
#include "wx/statline.h"
#include "wx/imaglist.h"


wxIMPLEMENT_DYNAMIC_CLASS(wxListbook, wxBookCtrlBase);

wxDEFINE_EVENT( wxEVT_LISTBOOK_PAGE_CHANGING, wxBookCtrlEvent );
wxDEFINE_EVENT( wxEVT_LISTBOOK_PAGE_CHANGED,  wxBookCtrlEvent );

wxBEGIN_EVENT_TABLE(wxListbook, wxBookCtrlBase)
    EVT_SIZE(wxListbook::OnSize)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, wxListbook::OnListSelected)
wxEND_EVENT_TABLE()



bool
wxListbook::Create(wxWindow *parent,
                   wxWindowID id,
                   const wxPoint& pos,
                   const wxSize& size,
                   long style,
                   const wxString& name)
{
    if ( (style & wxBK_ALIGN_MASK) == wxBK_DEFAULT )
    {
#ifdef __WXMAC__
        style |= wxBK_TOP;
#else         style |= wxBK_LEFT;
#endif     }

            style &= ~wxBORDER_MASK;
    style |= wxBORDER_NONE;

    if ( !wxControl::Create(parent, id, pos, size, style,
                            wxDefaultValidator, name) )
        return false;

    m_bookctrl = new wxListView
                 (
                    this,
                    wxID_ANY,
                    wxDefaultPosition,
                    wxDefaultSize,
                    GetListCtrlFlags()
                 );

    if ( GetListView()->InReportView() )
        GetListView()->InsertColumn(0, wxS("Pages"));

            PostSizeEvent();

    return true;
}


long wxListbook::GetListCtrlFlags() const
{
                                
    long flags = IsVertical() ? wxLC_ALIGN_LEFT : wxLC_ALIGN_TOP;
    if ( GetImageList() )
    {
        flags |= wxLC_ICON;
    }
    else     {
#ifdef __WXMSW__
        if ( !IsVertical() )
        {
                                    flags = wxLC_REPORT | wxLC_NO_HEADER;
        }
        else
#endif         {
            flags |= wxLC_LIST;
        }

#ifdef __WXQT__
        flags |= wxLC_NO_HEADER;
#endif
    }

        return flags | wxLC_SINGLE_SEL;
}


void wxListbook::OnSize(wxSizeEvent& event)
{
                        wxListView * const list = GetListView();
    if ( list )
    {
        list->Arrange();

        const int sel = GetSelection();
        if ( sel != wxNOT_FOUND )
            list->EnsureVisible(sel);
    }

    event.Skip();
}

int wxListbook::HitTest(const wxPoint& pt, long *flags) const
{
    int pagePos = wxNOT_FOUND;

    if ( flags )
        *flags = wxBK_HITTEST_NOWHERE;

        const wxListView * const list = GetListView();
    const wxPoint listPt = list->ScreenToClient(ClientToScreen(pt));

        if ( wxRect(list->GetSize()).Contains(listPt) )
    {
        int flagsList;
        pagePos = list->HitTest(listPt, flagsList);

        if ( flags )
        {
            if ( pagePos != wxNOT_FOUND )
                *flags = 0;

            if ( flagsList & (wxLIST_HITTEST_ONITEMICON |
                              wxLIST_HITTEST_ONITEMSTATEICON ) )
                *flags |= wxBK_HITTEST_ONICON;

            if ( flagsList & wxLIST_HITTEST_ONITEMLABEL )
                *flags |= wxBK_HITTEST_ONLABEL;
        }
    }
    else     {
        if ( flags && GetPageRect().Contains(pt) )
            *flags |= wxBK_HITTEST_ONPAGE;
    }

    return pagePos;
}

void wxListbook::UpdateSize()
{
            wxSizeEvent sz(GetSize(), GetId());
    GetEventHandler()->ProcessEvent(sz);
}


bool wxListbook::SetPageText(size_t n, const wxString& strText)
{
    GetListView()->SetItemText(n, strText);

    return true;
}

wxString wxListbook::GetPageText(size_t n) const
{
    return GetListView()->GetItemText(n);
}

int wxListbook::GetPageImage(size_t n) const
{
    wxListItem item;
    item.SetId(n);

    if (GetListView()->GetItem(item))
    {
       return item.GetImage();
    }
    else
    {
       return wxNOT_FOUND;
    }
}

bool wxListbook::SetPageImage(size_t n, int imageId)
{
    return GetListView()->SetItemImage(n, imageId);
}


void wxListbook::SetImageList(wxImageList *imageList)
{
    const long flagsOld = GetListCtrlFlags();

    wxBookCtrlBase::SetImageList(imageList);

    const long flagsNew = GetListCtrlFlags();

    wxListView * const list = GetListView();

            if ( flagsNew != flagsOld )
    {
                const int oldSel = GetSelection();

        list->SetWindowStyleFlag(flagsNew);
        if ( list->InReportView() )
            list->InsertColumn(0, wxS("Pages"));

                if ( oldSel != wxNOT_FOUND )
            SetSelection(oldSel);
    }

    list->SetImageList(imageList, wxIMAGE_LIST_NORMAL);
}


void wxListbook::UpdateSelectedPage(size_t newsel)
{
    m_selection = newsel;
    GetListView()->Select(newsel);
    GetListView()->Focus(newsel);
}

wxBookCtrlEvent* wxListbook::CreatePageChangingEvent() const
{
    return new wxBookCtrlEvent(wxEVT_LISTBOOK_PAGE_CHANGING, m_windowId);
}

void wxListbook::MakeChangedEvent(wxBookCtrlEvent &event)
{
    event.SetEventType(wxEVT_LISTBOOK_PAGE_CHANGED);
}



bool
wxListbook::InsertPage(size_t n,
                       wxWindow *page,
                       const wxString& text,
                       bool bSelect,
                       int imageId)
{
    if ( !wxBookCtrlBase::InsertPage(n, page, text, bSelect, imageId) )
        return false;

    GetListView()->InsertItem(n, text, imageId);

            if ( int(n) <= m_selection )
    {
                m_selection++;
        GetListView()->Select(m_selection);
        GetListView()->Focus(m_selection);
    }

    if ( !DoSetSelectionAfterInsertion(n, bSelect) )
        page->Hide();

    UpdateSize();

    return true;
}

wxWindow *wxListbook::DoRemovePage(size_t page)
{
    wxWindow *win = wxBookCtrlBase::DoRemovePage(page);

    if ( win )
    {
        GetListView()->DeleteItem(page);

        DoSetSelectionAfterRemoval(page);

        GetListView()->Arrange();
        UpdateSize();
    }

    return win;
}


bool wxListbook::DeleteAllPages()
{
    GetListView()->DeleteAllItems();
    if (!wxBookCtrlBase::DeleteAllPages())
        return false;

    UpdateSize();

    return true;
}


void wxListbook::OnListSelected(wxListEvent& eventList)
{
    if ( eventList.GetEventObject() != m_bookctrl )
    {
        eventList.Skip();
        return;
    }

    const int selNew = eventList.GetIndex();

    if ( selNew == m_selection )
    {
                                return;
    }

    SetSelection(selNew);

        if (m_selection != selNew)
    {
        GetListView()->Select(m_selection);
        GetListView()->Focus(m_selection);
    }
}

#endif 