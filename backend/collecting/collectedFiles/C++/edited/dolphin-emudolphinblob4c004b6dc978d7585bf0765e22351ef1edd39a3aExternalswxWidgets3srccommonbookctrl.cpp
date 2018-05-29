


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_BOOKCTRL

#include "wx/imaglist.h"

#include "wx/bookctrl.h"



wxIMPLEMENT_ABSTRACT_CLASS(wxBookCtrlBase, wxControl);

wxBEGIN_EVENT_TABLE(wxBookCtrlBase, wxControl)
    EVT_SIZE(wxBookCtrlBase::OnSize)
#if wxUSE_HELP
    EVT_HELP(wxID_ANY, wxBookCtrlBase::OnHelp)
#endif wxEND_EVENT_TABLE()


void wxBookCtrlBase::Init()
{
    m_selection = wxNOT_FOUND;
    m_bookctrl = NULL;
    m_fitToCurrentPage = false;

    m_internalBorder = 5;

    m_controlMargin = 0;
    m_controlSizer = NULL;
}

bool
wxBookCtrlBase::Create(wxWindow *parent,
                       wxWindowID id,
                       const wxPoint& pos,
                       const wxSize& size,
                       long style,
                       const wxString& name)
{
    return wxControl::Create
                     (
                        parent,
                        id,
                        pos,
                        size,
                        style,
                        wxDefaultValidator,
                        name
                     );
}


void wxBookCtrlBase::DoInvalidateBestSize()
{
                    if ( m_bookctrl )
        m_bookctrl->InvalidateBestSize();
    else
        wxControl::InvalidateBestSize();
}

wxSize wxBookCtrlBase::CalcSizeFromPage(const wxSize& sizePage) const
{
        if ( !m_bookctrl || !m_bookctrl->IsShown() )
        return sizePage;

                    const wxSize sizeController = GetControllerSize();

    wxSize size = sizePage;
    if ( IsVertical() )
        size.y += sizeController.y + GetInternalBorder();
    else         size.x += sizeController.x + GetInternalBorder();

    return size;
}

void wxBookCtrlBase::SetPageSize(const wxSize& size)
{
    SetClientSize(CalcSizeFromPage(size));
}

wxSize wxBookCtrlBase::DoGetBestSize() const
{
    wxSize bestSize;

    if (m_fitToCurrentPage && GetCurrentPage())
    {
        bestSize = GetCurrentPage()->GetBestSize();
    }
    else
    {
                const size_t nCount = m_pages.size();
        for ( size_t nPage = 0; nPage < nCount; nPage++ )
        {
            const wxWindow * const pPage = m_pages[nPage];
            if ( pPage )
                bestSize.IncTo(pPage->GetBestSize());
        }
    }

            wxSize best = CalcSizeFromPage(bestSize);
    CacheBestSize(best);
    return best;
}

wxRect wxBookCtrlBase::GetPageRect() const
{
    const wxSize size = GetControllerSize();

    wxPoint pt;
    wxRect rectPage(pt, GetClientSize());

    switch ( GetWindowStyle() & wxBK_ALIGN_MASK )
    {
        default:
            wxFAIL_MSG( wxT("unexpected alignment") );
            wxFALLTHROUGH;

        case wxBK_TOP:
            rectPage.y = size.y + GetInternalBorder();
            wxFALLTHROUGH;

        case wxBK_BOTTOM:
            rectPage.height -= size.y + GetInternalBorder();
            if (rectPage.height < 0)
                rectPage.height = 0;
            break;

        case wxBK_LEFT:
            rectPage.x = size.x + GetInternalBorder();
            wxFALLTHROUGH;

        case wxBK_RIGHT:
            rectPage.width -= size.x + GetInternalBorder();
            if (rectPage.width < 0)
                rectPage.width = 0;
            break;
    }

    return rectPage;
}

void wxBookCtrlBase::DoSize()
{
    if ( !m_bookctrl )
    {
                return;
    }

    if (GetSizer())
        Layout();
    else
    {
                const wxSize sizeClient( GetClientSize() ),
                    sizeBorder( m_bookctrl->GetSize() - m_bookctrl->GetClientSize() ),
                    sizeCtrl( GetControllerSize() );

        m_bookctrl->SetClientSize( sizeCtrl.x - sizeBorder.x, sizeCtrl.y - sizeBorder.y );
                wxSize sizeCtrl2 = GetControllerSize();
        if ( sizeCtrl != sizeCtrl2 )
        {
            wxSize sizeBorder2 = m_bookctrl->GetSize() - m_bookctrl->GetClientSize();
            m_bookctrl->SetClientSize( sizeCtrl2.x - sizeBorder2.x, sizeCtrl2.y - sizeBorder2.y );
        }

        const wxSize sizeNew = m_bookctrl->GetSize();
        wxPoint posCtrl;
        switch ( GetWindowStyle() & wxBK_ALIGN_MASK )
        {
            default:
                wxFAIL_MSG( wxT("unexpected alignment") );
                wxFALLTHROUGH;

            case wxBK_TOP:
            case wxBK_LEFT:
                                break;

            case wxBK_BOTTOM:
                posCtrl.y = sizeClient.y - sizeNew.y;
                break;

            case wxBK_RIGHT:
                posCtrl.x = sizeClient.x - sizeNew.x;
                break;
        }

        if ( m_bookctrl->GetPosition() != posCtrl )
            m_bookctrl->Move(posCtrl);
    }

        const wxRect pageRect = GetPageRect();
    const unsigned pagesCount = m_pages.GetCount();
    for ( unsigned int i = 0; i < pagesCount; ++i )
    {
        wxWindow * const page = m_pages[i];
        if ( !page )
        {
            wxASSERT_MSG( AllowNullPage(),
                wxT("Null page in a control that does not allow null pages?") );
            continue;
        }

        page->SetSize(pageRect);
    }
}

void wxBookCtrlBase::OnSize(wxSizeEvent& event)
{
    event.Skip();

    DoSize();
}

wxSize wxBookCtrlBase::GetControllerSize() const
{
                if ( !m_bookctrl || !m_bookctrl->IsShown() )
        return wxSize(0, 0);

    const wxSize sizeClient = GetClientSize();

    wxSize size;

        if ( IsVertical() )
    {
        size.x = sizeClient.x;
        size.y = m_bookctrl->GetBestHeight(sizeClient.x);
    }
    else     {
        size.x = m_bookctrl->GetBestWidth(sizeClient.y);
        size.y = sizeClient.y;
    }

    return size;
}


#if wxUSE_HELP

void wxBookCtrlBase::OnHelp(wxHelpEvent& event)
{
        
                wxWindow *source = wxStaticCast(event.GetEventObject(), wxWindow);
    while ( source && source != this && source->GetParent() != this )
    {
        source = source->GetParent();
    }

    if ( source && m_pages.Index(source) == wxNOT_FOUND )
    {
                        wxWindow *page = NULL;

        if ( event.GetOrigin() == wxHelpEvent::Origin_HelpButton )
        {
                        const int pagePos = HitTest(ScreenToClient(event.GetPosition()));

            if ( pagePos != wxNOT_FOUND)
            {
                page = GetPage((size_t)pagePos);
            }
        }
        else         {
                        page = GetCurrentPage();
        }

        if ( page )
        {
                                    event.SetEventObject(page);

            if ( page->GetEventHandler()->ProcessEvent(event) )
            {
                                return;
            }
        }
    }
    
    event.Skip();
}

#endif 

bool
wxBookCtrlBase::InsertPage(size_t nPage,
                           wxWindow *page,
                           const wxString& WXUNUSED(text),
                           bool WXUNUSED(bSelect),
                           int WXUNUSED(imageId))
{
    wxCHECK_MSG( page || AllowNullPage(), false,
                 wxT("NULL page in wxBookCtrlBase::InsertPage()") );
    wxCHECK_MSG( nPage <= m_pages.size(), false,
                 wxT("invalid page index in wxBookCtrlBase::InsertPage()") );

    m_pages.Insert(page, nPage);
    if ( page )
        page->SetSize(GetPageRect());

    DoInvalidateBestSize();

    return true;
}

bool wxBookCtrlBase::DeletePage(size_t nPage)
{
    wxWindow *page = DoRemovePage(nPage);
    if ( !(page || AllowNullPage()) )
        return false;

        delete page;

    return true;
}

wxWindow *wxBookCtrlBase::DoRemovePage(size_t nPage)
{
    wxCHECK_MSG( nPage < m_pages.size(), NULL,
                 wxT("invalid page index in wxBookCtrlBase::DoRemovePage()") );

    wxWindow *pageRemoved = m_pages[nPage];
    m_pages.RemoveAt(nPage);
    DoInvalidateBestSize();

    return pageRemoved;
}

int wxBookCtrlBase::GetNextPage(bool forward) const
{
    int nPage;

    int nMax = GetPageCount();
    if ( nMax-- )     {
        int nSel = GetSelection();

                nPage = forward ? nSel == nMax ? 0
                                       : nSel + 1
                        : nSel == 0 ? nMax
                                    : nSel - 1;
    }
    else     {
        nPage = wxNOT_FOUND;
    }

    return nPage;
}

int wxBookCtrlBase::FindPage(const wxWindow* page) const
{
    const size_t nCount = m_pages.size();
    for ( size_t nPage = 0; nPage < nCount; nPage++ )
    {
        if ( m_pages[nPage] == page )
            return (int)nPage;
    }

    return wxNOT_FOUND;
}

bool wxBookCtrlBase::DoSetSelectionAfterInsertion(size_t n, bool bSelect)
{
    if ( bSelect )
        SetSelection(n);
    else if ( m_selection == wxNOT_FOUND )
        ChangeSelection(0);
    else         return false;

        return true;
}

void wxBookCtrlBase::DoSetSelectionAfterRemoval(size_t n)
{
    if ( m_selection >= (int)n )
    {
                int sel;
        if ( GetPageCount() == 0 )
            sel = wxNOT_FOUND;
        else
            sel = m_selection ? m_selection - 1 : 0;

                m_selection = m_selection == (int)n ? wxNOT_FOUND
                                            : m_selection - 1;

        if ( sel != wxNOT_FOUND && sel != m_selection )
            SetSelection(sel);
    }
}

int wxBookCtrlBase::DoSetSelection(size_t n, int flags)
{
    wxCHECK_MSG( n < GetPageCount(), wxNOT_FOUND,
                 wxT("invalid page index in wxBookCtrlBase::DoSetSelection()") );

    const int oldSel = GetSelection();

    if ( n != (size_t)oldSel )
    {
        wxBookCtrlEvent *event = CreatePageChangingEvent();
        bool allowed = false;

        if ( flags & SetSelection_SendEvent )
        {
            event->SetSelection(n);
            event->SetOldSelection(oldSel);
            event->SetEventObject(this);

            allowed = !GetEventHandler()->ProcessEvent(*event) || event->IsAllowed();
        }

        if ( !(flags & SetSelection_SendEvent) || allowed)
        {
            if ( oldSel != wxNOT_FOUND )
                DoShowPage(m_pages[oldSel], false);

            wxWindow *page = m_pages[n];
            page->SetSize(GetPageRect());
            DoShowPage(page, true);

                        UpdateSelectedPage(n);

            if ( flags & SetSelection_SendEvent )
            {
                                MakeChangedEvent(*event);
                (void)GetEventHandler()->ProcessEvent(*event);
            }
        }

        delete event;
    }

    return oldSel;
}

wxIMPLEMENT_DYNAMIC_CLASS(wxBookCtrlEvent, wxNotifyEvent);

#endif 