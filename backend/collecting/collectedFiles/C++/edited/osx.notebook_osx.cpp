
#include "wx/wxprec.h"

#if wxUSE_NOTEBOOK

#include "wx/notebook.h"

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/log.h"
    #include "wx/app.h"
    #include "wx/image.h"
#endif

#include "wx/string.h"
#include "wx/imaglist.h"
#include "wx/osx/private.h"


#define IS_VALID_PAGE(nPage) ((nPage) < GetPageCount())

wxBEGIN_EVENT_TABLE(wxNotebook, wxBookCtrlBase)
    EVT_SIZE(wxNotebook::OnSize)
    EVT_SET_FOCUS(wxNotebook::OnSetFocus)
    EVT_NAVIGATION_KEY(wxNotebook::OnNavigationKey)
wxEND_EVENT_TABLE()

bool wxNotebook::Create( wxWindow *parent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name )
{    
    DontCreatePeer();
    
    if (! (style & wxBK_ALIGN_MASK))
        style |= wxBK_TOP;

    if ( !wxNotebookBase::Create( parent, id, pos, size, style, name ) )
        return false;

    SetPeer(wxWidgetImpl::CreateTabView(this,parent, id, pos, size, style, GetExtraStyle() ));

    MacPostControlCreate( pos, size );

    return true ;
}

wxNotebook::~wxNotebook()
{
}


void wxNotebook::SetPadding(const wxSize& WXUNUSED(padding))
{
    }

void wxNotebook::SetTabSize(const wxSize& WXUNUSED(sz))
{
    }

void wxNotebook::SetPageSize(const wxSize& size)
{
    SetSize( CalcSizeFromPage( size ) );
}

wxSize wxNotebook::CalcSizeFromPage(const wxSize& sizePage) const
{
    return DoGetSizeFromClientSize( sizePage );
}

int wxNotebook::DoSetSelection(size_t nPage, int flags)
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), wxNOT_FOUND, wxT("DoSetSelection: invalid notebook page") );

    if ( m_selection == wxNOT_FOUND || nPage != (size_t)m_selection )
    {
        if ( flags & SetSelection_SendEvent )
        {
            if ( !SendPageChangingEvent(nPage) )
            {
                                return m_selection;
            }
                    }

                        int oldSelection = m_selection;
        ChangePage(oldSelection, nPage);

        if ( flags & SetSelection_SendEvent )
            SendPageChangedEvent(oldSelection, nPage);
    }
    
    return m_selection;
}

bool wxNotebook::SetPageText(size_t nPage, const wxString& strText)
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), false, wxT("SetPageText: invalid notebook page") );

    wxNotebookPage *page = m_pages[nPage];
    page->SetLabel(wxStripMenuCodes(strText));
    MacSetupTabs();

    return true;
}

wxString wxNotebook::GetPageText(size_t nPage) const
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), wxEmptyString, wxT("GetPageText: invalid notebook page") );

    wxNotebookPage *page = m_pages[nPage];

    return page->GetLabel();
}

int wxNotebook::GetPageImage(size_t nPage) const
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), wxNOT_FOUND, wxT("GetPageImage: invalid notebook page") );

    return m_images[nPage];
}

bool wxNotebook::SetPageImage(size_t nPage, int nImage)
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), false,
        wxT("SetPageImage: invalid notebook page") );
    wxCHECK_MSG( HasImageList() && nImage < GetImageList()->GetImageCount(), false,
        wxT("SetPageImage: invalid image index") );

    if ( nImage != m_images[nPage] )
    {
                                m_images[nPage] = nImage;

        MacSetupTabs() ;
    }

    return true;
}


wxNotebookPage* wxNotebook::DoRemovePage(size_t nPage)
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), NULL,
        wxT("DoRemovePage: invalid notebook page") );

    wxNotebookPage* page = m_pages[nPage] ;
    m_pages.RemoveAt(nPage);
    m_images.RemoveAt(nPage);

    MacSetupTabs();

    if ( m_selection >= (int)nPage )
    {
        if ( GetPageCount() == 0 )
            m_selection = wxNOT_FOUND;
        else
            m_selection = m_selection ? m_selection - 1 : 0;

        GetPeer()->SetValue( m_selection + 1 ) ;
    }

    if (m_selection >= 0)
        m_pages[m_selection]->Show(true);

    InvalidateBestSize();

    return page;
}

bool wxNotebook::DeleteAllPages()
{
    WX_CLEAR_ARRAY(m_pages);
    m_images.clear();
    MacSetupTabs();
    m_selection = wxNOT_FOUND ;
    InvalidateBestSize();

    return true;
}

bool wxNotebook::InsertPage(size_t nPage,
    wxNotebookPage *pPage,
    const wxString& strText,
    bool bSelect,
    int imageId )
{
    if ( !wxNotebookBase::InsertPage( nPage, pPage, strText, bSelect, imageId ) )
        return false;

    wxASSERT_MSG( pPage->GetParent() == this, wxT("notebook pages must have notebook as parent") );

        pPage->Show( false ) ;

    pPage->SetLabel( wxStripMenuCodes(strText) );

    m_images.Insert( imageId, nPage );

    MacSetupTabs();

    wxRect rect = GetPageRect() ;
    pPage->SetSize( rect );
    if ( pPage->GetAutoLayout() )
        pPage->Layout();

        
        
    if ( int(nPage) <= m_selection )
    {
        m_selection++;

                GetPeer()->SetValue( m_selection + 1 ) ;
    }

    DoSetSelectionAfterInsertion(nPage, bSelect);

    InvalidateBestSize();

    return true;
}

int wxNotebook::HitTest(const wxPoint& pt, long *flags) const
{
    return GetPeer()->TabHitTest(pt,flags);
}

void wxNotebook::MacSetupTabs()
{
    GetPeer()->SetupTabs(*this);
    Refresh();
}

wxRect wxNotebook::GetPageRect() const
{
    wxSize size = GetClientSize() ;

    return wxRect( 0 , 0 , size.x , size.y ) ;
}


void wxNotebook::OnSize(wxSizeEvent& event)
{
    unsigned int nCount = m_pages.Count();
    wxRect rect = GetPageRect() ;

    for ( unsigned int nPage = 0; nPage < nCount; nPage++ )
    {
        wxNotebookPage *pPage = m_pages[nPage];
        pPage->SetSize(rect, wxSIZE_FORCE_EVENT);
    }

#if 0                     if ( m_selection != wxNOT_FOUND )
    {
        wxNotebookPage *pPage = m_pages[m_selection];
        if ( !pPage->IsShown() )
        {
            pPage->Show( true );
            pPage->SetFocus();
        }
    }
#endif

        event.Skip();
}

void wxNotebook::OnSetFocus(wxFocusEvent& event)
{
        if ( m_selection != wxNOT_FOUND )
        m_pages[m_selection]->SetFocus();

    event.Skip();
}

void wxNotebook::OnNavigationKey(wxNavigationKeyEvent& event)
{
    if ( event.IsWindowChange() )
    {
                AdvanceSelection( event.GetDirection() );
    }
    else
    {
                                                                                                                wxWindow *parent = GetParent();

                if ( ((wxWindow*)event.GetEventObject()) == parent )
        {
                        if ( m_selection != wxNOT_FOUND )
            {
                                                event.SetEventObject( this );

                wxWindow *page = m_pages[m_selection];
                if ( !page->HandleWindowEvent( event ) )
                {
                    page->SetFocus();
                }
                            }
            else
            {
                                SetFocus();
            }
        }
        else
        {
                        if ( parent )
            {
                event.SetCurrentFocus( this );
                parent->HandleWindowEvent( event );
            }
        }
    }
}


#if wxUSE_CONSTRAINTS


void wxNotebook::SetConstraintSizes(bool WXUNUSED(recurse))
{
        wxControl::SetConstraintSizes( false );
}

bool wxNotebook::DoPhase(int WXUNUSED(nPhase))
{
    return true;
}

#endif 
void wxNotebook::Command(wxCommandEvent& WXUNUSED(event))
{
    wxFAIL_MSG(wxT("wxNotebook::Command not implemented"));
}


void wxNotebook::ChangePage(int nOldSel, int nSel)
{
    if (nOldSel == nSel)
        return;

    if ( nOldSel != wxNOT_FOUND )
        m_pages[nOldSel]->Show( false );

    if ( nSel != wxNOT_FOUND )
    {
        wxNotebookPage *pPage = m_pages[nSel];
#if 0         if ( IsShownOnScreen() )
        {
            pPage->Show( true );
            pPage->SetFocus();
        }
        else
        {
                                                                                }
#else
        pPage->Show( true );
        pPage->SetFocus();
#endif
    }

    m_selection = nSel;
    GetPeer()->SetValue( m_selection + 1 ) ;
}

bool wxNotebook::OSXHandleClicked( double WXUNUSED(timestampsec) )
{
    bool status = false ;

    SInt32 newSel = GetPeer()->GetValue() - 1 ;
    if ( newSel != m_selection )
    {
        if ( DoSetSelection(newSel, SetSelection_SendEvent ) != newSel )
            GetPeer()->SetValue( m_selection + 1 ) ;

        status = true ;
    }

    return status ;
}

#endif
