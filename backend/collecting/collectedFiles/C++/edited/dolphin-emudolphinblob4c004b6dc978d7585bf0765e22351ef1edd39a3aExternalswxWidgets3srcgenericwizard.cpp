


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_WIZARDDLG

#ifndef WX_PRECOMP
    #include "wx/dynarray.h"
    #include "wx/intl.h"
    #include "wx/statbmp.h"
    #include "wx/button.h"
    #include "wx/settings.h"
    #include "wx/sizer.h"
#endif 
#include "wx/statline.h"

#include "wx/scrolwin.h"
#include "wx/wizard.h"
#include "wx/dcmemory.h"


class wxWizardSizer : public wxSizer
{
public:
    wxWizardSizer(wxWizard *owner);

    virtual wxSizerItem *Insert(size_t index, wxSizerItem *item) wxOVERRIDE;

    virtual void RecalcSizes() wxOVERRIDE;
    virtual wxSize CalcMin() wxOVERRIDE;

        wxSize GetMaxChildSize();

            int GetBorder() const;

            void HidePages();

private:
    wxSize SiblingSize(wxSizerItem *child);

    wxWizard *m_owner;
    wxSize m_childSize;
};


wxDEFINE_EVENT( wxEVT_WIZARD_PAGE_CHANGED, wxWizardEvent );
wxDEFINE_EVENT( wxEVT_WIZARD_PAGE_CHANGING, wxWizardEvent );
wxDEFINE_EVENT( wxEVT_WIZARD_BEFORE_PAGE_CHANGED, wxWizardEvent );
wxDEFINE_EVENT( wxEVT_WIZARD_CANCEL, wxWizardEvent );
wxDEFINE_EVENT( wxEVT_WIZARD_FINISHED, wxWizardEvent );
wxDEFINE_EVENT( wxEVT_WIZARD_HELP, wxWizardEvent );
wxDEFINE_EVENT( wxEVT_WIZARD_PAGE_SHOWN, wxWizardEvent );

wxBEGIN_EVENT_TABLE(wxWizard, wxDialog)
    EVT_BUTTON(wxID_CANCEL, wxWizard::OnCancel)
    EVT_BUTTON(wxID_BACKWARD, wxWizard::OnBackOrNext)
    EVT_BUTTON(wxID_FORWARD, wxWizard::OnBackOrNext)
    EVT_BUTTON(wxID_HELP, wxWizard::OnHelp)

    EVT_WIZARD_PAGE_CHANGED(wxID_ANY, wxWizard::OnWizEvent)
    EVT_WIZARD_PAGE_CHANGING(wxID_ANY, wxWizard::OnWizEvent)
    EVT_WIZARD_CANCEL(wxID_ANY, wxWizard::OnWizEvent)
    EVT_WIZARD_FINISHED(wxID_ANY, wxWizard::OnWizEvent)
    EVT_WIZARD_HELP(wxID_ANY, wxWizard::OnWizEvent)
wxEND_EVENT_TABLE()

wxIMPLEMENT_DYNAMIC_CLASS(wxWizard, wxDialog);



wxIMPLEMENT_ABSTRACT_CLASS(wxWizardPage, wxPanel);
wxIMPLEMENT_DYNAMIC_CLASS(wxWizardPageSimple, wxWizardPage);
wxIMPLEMENT_DYNAMIC_CLASS(wxWizardEvent, wxNotifyEvent);



void wxWizardPage::Init()
{
    m_bitmap = wxNullBitmap;
}

wxWizardPage::wxWizardPage(wxWizard *parent,
                           const wxBitmap& bitmap)
{
    Create(parent, bitmap);
}

bool wxWizardPage::Create(wxWizard *parent,
                          const wxBitmap& bitmap)
{
    if ( !wxPanel::Create(parent, wxID_ANY) )
        return false;

    m_bitmap = bitmap;

        Hide();

    return true;
}


wxWizardPage *wxWizardPageSimple::GetPrev() const
{
    return m_prev;
}

wxWizardPage *wxWizardPageSimple::GetNext() const
{
    return m_next;
}


wxWizardSizer::wxWizardSizer(wxWizard *owner)
             : m_owner(owner),
               m_childSize(wxDefaultSize)
{
}

wxSizerItem *wxWizardSizer::Insert(size_t index, wxSizerItem *item)
{
    m_owner->m_usingSizer = true;

    if ( item->IsWindow() )
    {
                                item->GetWindow()->wxWindowBase::Show();
    }

    return wxSizer::Insert(index, item);
}

void wxWizardSizer::HidePages()
{
    for ( wxSizerItemList::compatibility_iterator node = GetChildren().GetFirst();
          node;
          node = node->GetNext() )
    {
        wxSizerItem * const item = node->GetData();
        if ( item->IsWindow() )
            item->GetWindow()->wxWindowBase::Show(false);
    }
}

void wxWizardSizer::RecalcSizes()
{
            if ( m_owner->m_page )
    {
        m_owner->m_page->SetSize(wxRect(m_position, m_size));
    }
}

wxSize wxWizardSizer::CalcMin()
{
    return m_owner->GetPageSize();
}

wxSize wxWizardSizer::GetMaxChildSize()
{
    wxSize maxOfMin;

    for ( wxSizerItemList::compatibility_iterator childNode = m_children.GetFirst();
          childNode;
          childNode = childNode->GetNext() )
    {
        wxSizerItem *child = childNode->GetData();
        maxOfMin.IncTo(child->CalcMin());
        maxOfMin.IncTo(SiblingSize(child));
    }

    if ( m_owner->m_started )
    {
        m_childSize = maxOfMin;
    }

    return maxOfMin;
}

int wxWizardSizer::GetBorder() const
{
    return m_owner->m_border;
}

wxSize wxWizardSizer::SiblingSize(wxSizerItem *child)
{
    wxSize maxSibling;

    if ( child->IsWindow() )
    {
        wxWizardPage *page = wxDynamicCast(child->GetWindow(), wxWizardPage);
        if ( page )
        {
            for ( wxWizardPage *sibling = page->GetNext();
                  sibling;
                  sibling = sibling->GetNext() )
            {
                if ( sibling->GetSizer() )
                {
                    maxSibling.IncTo(sibling->GetSizer()->CalcMin());
                }
            }
        }
    }

    return maxSibling;
}


void wxWizard::Init()
{
    m_posWizard = wxDefaultPosition;
    m_page = NULL;
    m_firstpage = NULL;
    m_btnPrev = m_btnNext = NULL;
    m_statbmp = NULL;
    m_sizerBmpAndPage = NULL;
    m_sizerPage = NULL;
    m_border = 5;
    m_started = false;
    m_wasModal = false;
    m_usingSizer = false;
    m_bitmapBackgroundColour = *wxWHITE;
    m_bitmapPlacement = 0;
    m_bitmapMinimumWidth = 115;
}

bool wxWizard::Create(wxWindow *parent,
                      int id,
                      const wxString& title,
                      const wxBitmap& bitmap,
                      const wxPoint& pos,
                      long style)
{
    bool result = wxDialog::Create(parent,id,title,pos,wxDefaultSize,style);

    m_posWizard = pos;
    m_bitmap = bitmap ;

    DoCreateControls();

    return result;
}

wxWizard::~wxWizard()
{
                if ( !m_usingSizer || !m_started )
        delete m_sizerPage;
}

void wxWizard::AddBitmapRow(wxBoxSizer *mainColumn)
{
    m_sizerBmpAndPage = new wxBoxSizer(wxHORIZONTAL);
    mainColumn->Add(
        m_sizerBmpAndPage,
        1,         wxEXPAND     );
    mainColumn->Add(0,5,
        0,         wxEXPAND     );

#if wxUSE_STATBMP
    if ( m_bitmap.IsOk() )
    {
        wxSize bitmapSize(wxDefaultSize);
        if (GetBitmapPlacement())
            bitmapSize.x = GetMinimumBitmapWidth();

        m_statbmp = new wxStaticBitmap(this, wxID_ANY, m_bitmap, wxDefaultPosition, bitmapSize);
        m_sizerBmpAndPage->Add(
            m_statbmp,
            0,             wxALL,             5         );
        m_sizerBmpAndPage->Add(
            5,0,
            0,             wxEXPAND         );
    }
#endif

        m_sizerPage = new wxWizardSizer(this);
}

void wxWizard::AddStaticLine(wxBoxSizer *mainColumn)
{
#if wxUSE_STATLINE
    mainColumn->Add(
        new wxStaticLine(this, wxID_ANY),
        0,         wxEXPAND | wxALL,         5     );
    mainColumn->Add(0,5,
        0,         wxEXPAND     );
#else
    (void)mainColumn;
#endif }

void wxWizard::AddBackNextPair(wxBoxSizer *buttonRow)
{
    wxASSERT_MSG( m_btnNext && m_btnPrev,
                  wxT("You must create the buttons before calling ")
                  wxT("wxWizard::AddBackNextPair") );

    wxBoxSizer *backNextPair = new wxBoxSizer(wxHORIZONTAL);
    buttonRow->Add(
        backNextPair,
        0,         wxALL,         5     );

    backNextPair->Add(m_btnPrev);
    backNextPair->Add(10, 0,
        0,         wxEXPAND     );
    backNextPair->Add(m_btnNext);
}

void wxWizard::AddButtonRow(wxBoxSizer *mainColumn)
{
                                    
    bool isPda = (wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA);
    int buttonStyle = isPda ? wxBU_EXACTFIT : 0;

    wxBoxSizer *buttonRow = new wxBoxSizer(wxHORIZONTAL);
#ifdef __WXMAC__
    if (GetExtraStyle() & wxWIZARD_EX_HELPBUTTON)
        mainColumn->Add(
            buttonRow,
            0,             wxEXPAND
            );
    else
#endif
    mainColumn->Add(
        buttonRow,
        0,         wxALIGN_RIGHT     );

            wxButton *btnHelp=0;
#ifdef __WXMAC__
    if (GetExtraStyle() & wxWIZARD_EX_HELPBUTTON)
        btnHelp=new wxButton(this, wxID_HELP, wxEmptyString, wxDefaultPosition, wxDefaultSize, buttonStyle);
#endif

    m_btnNext = new wxButton(this, wxID_FORWARD, _("&Next >"));
    wxButton *btnCancel=new wxButton(this, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, buttonStyle);
#ifndef __WXMAC__
    if (GetExtraStyle() & wxWIZARD_EX_HELPBUTTON)
        btnHelp=new wxButton(this, wxID_HELP, _("&Help"), wxDefaultPosition, wxDefaultSize, buttonStyle);
#endif
    m_btnPrev = new wxButton(this, wxID_BACKWARD, _("< &Back"), wxDefaultPosition, wxDefaultSize, buttonStyle);

    if (btnHelp)
    {
        buttonRow->Add(
            btnHelp,
            0,             wxALL,             5             );
#ifdef __WXMAC__
                buttonRow->Add(0, 0, 1, wxALIGN_CENTRE, 0);
#endif
    }

    AddBackNextPair(buttonRow);

    buttonRow->Add(
        btnCancel,
        0,         wxALL,         5     );
}

void wxWizard::DoCreateControls()
{
        if ( WasCreated() )
        return;

    bool isPda = (wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA);

        int mainColumnSizerFlags = isPda ? wxEXPAND : wxALL|wxEXPAND ;

        wxBoxSizer *windowSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *mainColumn = new wxBoxSizer(wxVERTICAL);
    windowSizer->Add(
        mainColumn,
        1,         mainColumnSizerFlags,
        5     );

    AddBitmapRow(mainColumn);

    if (!isPda)
        AddStaticLine(mainColumn);

    AddButtonRow(mainColumn);

    SetSizer(windowSizer);
}

void wxWizard::SetPageSize(const wxSize& size)
{
    wxCHECK_RET(!m_started, wxT("wxWizard::SetPageSize after RunWizard"));
    m_sizePage = size;
}

void wxWizard::FitToPage(const wxWizardPage *page)
{
    wxCHECK_RET(!m_started, wxT("wxWizard::FitToPage after RunWizard"));

    while ( page )
    {
        wxSize size = page->GetBestSize();

        m_sizePage.IncTo(size);

        page = page->GetNext();
    }
}

bool wxWizard::ShowPage(wxWizardPage *page, bool goingForward)
{
    wxASSERT_MSG( page != m_page, wxT("this is useless") );

    wxSizerFlags flags(1);
    flags.Border(wxALL, m_border).Expand();

    if ( !m_started )
    {
        if ( m_usingSizer )
        {
            m_sizerBmpAndPage->Add(m_sizerPage, flags);

                                    m_sizerPage->HidePages();
        }
    }


        wxBitmap bmpPrev;

        if ( m_page )
    {
                wxWizardEvent event(wxEVT_WIZARD_PAGE_CHANGING, GetId(),
                            goingForward, m_page);
        if ( m_page->GetEventHandler()->ProcessEvent(event) &&
             !event.IsAllowed() )
        {
                        return false;
        }

        m_page->Hide();

        bmpPrev = m_page->GetBitmap();

        if ( !m_usingSizer )
            m_sizerBmpAndPage->Detach(m_page);
    }

        if ( !page )
    {
                if ( IsModal() )
        {
            EndModal(wxID_OK);
        }
        else
        {
            SetReturnCode(wxID_OK);
            Hide();
        }

                        wxWizardEvent event(wxEVT_WIZARD_FINISHED, GetId(), false, m_page);
        (void)GetEventHandler()->ProcessEvent(event);

        m_page = NULL;

        return true;
    }

            m_page = page;

        (void)m_page->TransferDataToWindow();

    if ( m_usingSizer )
    {
                m_sizerPage->RecalcSizes();
    }
    else     {
        m_sizerBmpAndPage->Add(m_page, flags);
        m_sizerBmpAndPage->SetItemMinSize(m_page, GetPageSize());
    }

#if wxUSE_STATBMP
        wxBitmap bmp;
    if ( m_statbmp )
    {
        bmp = m_page->GetBitmap();
        if ( !bmp.IsOk() )
            bmp = m_bitmap;

        if ( !bmpPrev.IsOk() )
            bmpPrev = m_bitmap;

        if (!GetBitmapPlacement())
        {
            if ( !bmp.IsSameAs(bmpPrev) )
                m_statbmp->SetBitmap(bmp);
        }
    }
#endif 

        m_btnPrev->Enable(m_page != m_firstpage);

    const bool hasNext = HasNextPage(m_page);
    const wxString label = hasNext ? _("&Next >") : _("&Finish");
    if ( label != m_btnNext->GetLabel() )
        m_btnNext->SetLabel(label);

    m_btnNext->SetDefault();


        wxWizardEvent event(wxEVT_WIZARD_PAGE_CHANGED, GetId(), goingForward, m_page);
    (void)m_page->GetEventHandler()->ProcessEvent(event);

        m_page->Show();
    m_page->SetFocus();

    if ( !m_usingSizer )
        m_sizerBmpAndPage->Layout();

    if ( !m_started )
    {
        m_started = true;

        DoWizardLayout();
    }

    if (GetBitmapPlacement() && m_statbmp)
    {
        ResizeBitmap(bmp);

        if ( !bmp.IsSameAs(bmpPrev) )
            m_statbmp->SetBitmap(bmp);

        if (m_usingSizer)
            m_sizerPage->RecalcSizes();
    }

    wxWizardEvent pageShownEvent(wxEVT_WIZARD_PAGE_SHOWN, GetId(),
        goingForward, m_page);
    m_page->GetEventHandler()->ProcessEvent(pageShownEvent);

    return true;
}

void wxWizard::DoWizardLayout()
{
    if ( wxSystemSettings::GetScreenType() > wxSYS_SCREEN_PDA )
    {
        if (CanDoLayoutAdaptation())
            DoLayoutAdaptation();
        else
            GetSizer()->SetSizeHints(this);

        if ( m_posWizard == wxDefaultPosition )
            CentreOnScreen();
    }

    SetLayoutAdaptationDone(true);
}

bool wxWizard::RunWizard(wxWizardPage *firstPage)
{
    wxCHECK_MSG( firstPage, false, wxT("can't run empty wizard") );

    m_firstpage = firstPage;

        (void)ShowPage(firstPage, true );

    m_wasModal = true;

    return ShowModal() == wxID_OK;
}

wxWizardPage *wxWizard::GetCurrentPage() const
{
    return m_page;
}

wxSize wxWizard::GetPageSize() const
{
        int DEFAULT_PAGE_WIDTH,
        DEFAULT_PAGE_HEIGHT;
    if ( wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA )
    {
                DEFAULT_PAGE_WIDTH = wxSystemSettings::GetMetric(wxSYS_SCREEN_X) / 2;
        DEFAULT_PAGE_HEIGHT = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y) / 2;
    }
    else     {
        DEFAULT_PAGE_WIDTH =
        DEFAULT_PAGE_HEIGHT = 270;
    }

        wxSize pageSize(DEFAULT_PAGE_WIDTH, DEFAULT_PAGE_HEIGHT);

        pageSize.IncTo(m_sizePage);

    if ( m_statbmp )
    {
                pageSize.IncTo(wxSize(0, m_bitmap.GetHeight()));
    }

    if ( m_usingSizer )
    {
                pageSize.IncTo(m_sizerPage->GetMaxChildSize());
    }

    return pageSize;
}

wxSizer *wxWizard::GetPageAreaSizer() const
{
    return m_sizerPage;
}

void wxWizard::SetBorder(int border)
{
    wxCHECK_RET(!m_started, wxT("wxWizard::SetBorder after RunWizard"));

    m_border = border;
}

void wxWizard::OnCancel(wxCommandEvent& WXUNUSED(eventUnused))
{
            wxWindow *win = m_page ? (wxWindow *)m_page : (wxWindow *)this;

    wxWizardEvent event(wxEVT_WIZARD_CANCEL, GetId(), false, m_page);
    if ( !win->GetEventHandler()->ProcessEvent(event) || event.IsAllowed() )
    {
                if(IsModal())
        {
            EndModal(wxID_CANCEL);
        }
        else
        {
            SetReturnCode(wxID_CANCEL);
            Hide();
        }
    }
    }

void wxWizard::OnBackOrNext(wxCommandEvent& event)
{
    wxASSERT_MSG( (event.GetEventObject() == m_btnNext) ||
                  (event.GetEventObject() == m_btnPrev),
                  wxT("unknown button") );

    wxCHECK_RET( m_page, wxT("should have a valid current page") );

                if ( !m_page->Validate() || !m_page->TransferDataFromWindow() )
    {
                return;
    }

    bool forward = event.GetEventObject() == m_btnNext;

        wxWizardEvent eventPreChanged(wxEVT_WIZARD_BEFORE_PAGE_CHANGED, GetId(), forward, m_page);
    (void)m_page->GetEventHandler()->ProcessEvent(eventPreChanged);

    if (!eventPreChanged.IsAllowed())
        return;

    wxWizardPage *page;
    if ( forward )
    {
        page = m_page->GetNext();
    }
    else     {
        page = m_page->GetPrev();

        wxASSERT_MSG( page, wxT("\"<Back\" button should have been disabled") );
    }

        (void)ShowPage(page, forward);
}

void wxWizard::OnHelp(wxCommandEvent& WXUNUSED(event))
{
            if(m_page != NULL)
    {
                                wxWizardEvent eventHelp(wxEVT_WIZARD_HELP, GetId(), true, m_page);
        (void)m_page->GetEventHandler()->ProcessEvent(eventHelp);
    }
}

void wxWizard::OnWizEvent(wxWizardEvent& event)
{
            if ( !(GetExtraStyle() & wxWS_EX_BLOCK_EVENTS) )
    {
                event.Skip();
    }
    else
    {
        wxWindow *parent = GetParent();

        if ( !parent || !parent->GetEventHandler()->ProcessEvent(event) )
        {
            event.Skip();
        }
    }

    if ( ( !m_wasModal ) &&
         event.IsAllowed() &&
         ( event.GetEventType() == wxEVT_WIZARD_FINISHED ||
           event.GetEventType() == wxEVT_WIZARD_CANCEL
         )
       )
    {
        Destroy();
    }
}

void wxWizard::SetBitmap(const wxBitmap& bitmap)
{
    m_bitmap = bitmap;
    if (m_statbmp)
        m_statbmp->SetBitmap(m_bitmap);
}


wxWizardEvent::wxWizardEvent(wxEventType type, int id, bool direction, wxWizardPage* page)
             : wxNotifyEvent(type, id)
{
            m_direction = direction;
    m_page = page;
}

bool wxWizard::DoLayoutAdaptation()
{
    wxWindowList windows;
    wxWindowList pages;

        for ( wxSizerItemList::compatibility_iterator node = m_sizerPage->GetChildren().GetFirst(); node; node = node->GetNext() )
    {
        wxSizerItem * const item = node->GetData();
        if ( item->IsWindow() )
        {
            wxWizardPage* page = wxDynamicCast(item->GetWindow(), wxWizardPage);
            if (page)
            {
                while (page)
                {
                    if (!pages.Find(page) && page->GetSizer())
                    {
                                                wxScrolledWindow* scrolledWindow = new wxScrolledWindow(page, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxVSCROLL|wxHSCROLL|wxBORDER_NONE);
                        wxSizer* oldSizer = page->GetSizer();

                        wxSizer* newSizer = new wxBoxSizer(wxVERTICAL);
                        newSizer->Add(scrolledWindow,1, wxEXPAND, 0);

                        page->SetSizer(newSizer, false );

                        scrolledWindow->SetSizer(oldSizer);

                        wxStandardDialogLayoutAdapter::DoReparentControls(page, scrolledWindow);

                        pages.Append(page);
                        windows.Append(scrolledWindow);
                    }
                    page = page->GetNext();
                }
            }
        }
    }

    wxStandardDialogLayoutAdapter::DoFitWithScrolling(this, windows);

        DoLayout();

    SetLayoutAdaptationDone(true);

    return true;
}

bool wxWizard::ResizeBitmap(wxBitmap& bmp)
{
    if (!GetBitmapPlacement())
        return false;

    if (bmp.IsOk())
    {
        wxSize pageSize = m_sizerPage->GetSize();
        if (pageSize == wxSize(0,0))
            pageSize = GetPageSize();
        int bitmapWidth = wxMax(bmp.GetWidth(), GetMinimumBitmapWidth());
        int bitmapHeight = pageSize.y;

        if (!m_statbmp->GetBitmap().IsOk() || m_statbmp->GetBitmap().GetHeight() != bitmapHeight)
        {
            wxBitmap bitmap(bitmapWidth, bitmapHeight);
            {
                wxMemoryDC dc;
                dc.SelectObject(bitmap);
                dc.SetBackground(wxBrush(m_bitmapBackgroundColour));
                dc.Clear();

                if (GetBitmapPlacement() & wxWIZARD_TILE)
                {
                    TileBitmap(wxRect(0, 0, bitmapWidth, bitmapHeight), dc, bmp);
                }
                else
                {
                    int x, y;

                    if (GetBitmapPlacement() & wxWIZARD_HALIGN_LEFT)
                        x = 0;
                    else if (GetBitmapPlacement() & wxWIZARD_HALIGN_RIGHT)
                        x = bitmapWidth - bmp.GetWidth();
                    else
                        x = (bitmapWidth - bmp.GetWidth())/2;

                    if (GetBitmapPlacement() & wxWIZARD_VALIGN_TOP)
                        y = 0;
                    else if (GetBitmapPlacement() & wxWIZARD_VALIGN_BOTTOM)
                        y = bitmapHeight - bmp.GetHeight();
                    else
                        y = (bitmapHeight - bmp.GetHeight())/2;

                    dc.DrawBitmap(bmp, x, y, true);
                    dc.SelectObject(wxNullBitmap);
                }
            }

            bmp = bitmap;
        }
    }

    return true;
}

bool wxWizard::TileBitmap(const wxRect& rect, wxDC& dc, const wxBitmap& bitmap)
{
    int w = bitmap.GetWidth();
    int h = bitmap.GetHeight();

    wxMemoryDC dcMem;

    dcMem.SelectObjectAsSource(bitmap);

    int i, j;
    for (i = rect.x; i < rect.x + rect.width; i += w)
    {
        for (j = rect.y; j < rect.y + rect.height; j+= h)
            dc.Blit(i, j, bitmap.GetWidth(), bitmap.GetHeight(), & dcMem, 0, 0);
    }
    dcMem.SelectObject(wxNullBitmap);

    return true;
}

#endif 