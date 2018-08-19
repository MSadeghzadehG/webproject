


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_CHOICEBOOK

#include "wx/choicebk.h"

#ifndef WX_PRECOMP
    #include "wx/settings.h"
    #include "wx/choice.h"
    #include "wx/sizer.h"
#endif

#include "wx/imaglist.h"


wxIMPLEMENT_DYNAMIC_CLASS(wxChoicebook, wxBookCtrlBase);

wxDEFINE_EVENT( wxEVT_CHOICEBOOK_PAGE_CHANGING, wxBookCtrlEvent );
wxDEFINE_EVENT( wxEVT_CHOICEBOOK_PAGE_CHANGED,  wxBookCtrlEvent );

wxBEGIN_EVENT_TABLE(wxChoicebook, wxBookCtrlBase)
    EVT_CHOICE(wxID_ANY, wxChoicebook::OnChoiceSelected)
wxEND_EVENT_TABLE()



bool
wxChoicebook::Create(wxWindow *parent,
                     wxWindowID id,
                     const wxPoint& pos,
                     const wxSize& size,
                     long style,
                     const wxString& name)
{
    if ( (style & wxBK_ALIGN_MASK) == wxBK_DEFAULT )
    {
        style |= wxBK_TOP;
    }

            style &= ~wxBORDER_MASK;
    style |= wxBORDER_NONE;

    if ( !wxControl::Create(parent, id, pos, size, style,
                            wxDefaultValidator, name) )
        return false;

    m_bookctrl = new wxChoice
                 (
                    this,
                    wxID_ANY,
                    wxDefaultPosition,
                    wxDefaultSize
                 );

    wxSizer* mainSizer = new wxBoxSizer(IsVertical() ? wxVERTICAL : wxHORIZONTAL);

    if (style & wxBK_RIGHT || style & wxBK_BOTTOM)
        mainSizer->Add(0, 0, 1, wxEXPAND, 0);

    m_controlSizer = new wxBoxSizer(IsVertical() ? wxHORIZONTAL : wxVERTICAL);
    m_controlSizer->Add(m_bookctrl, wxSizerFlags(1).Expand());
    wxSizerFlags flags;
    if ( IsVertical() )
        flags.Expand();
    else
        flags.CentreVertical();
    mainSizer->Add(m_controlSizer, flags.Border(wxALL, m_controlMargin));
    SetSizer(mainSizer);
    return true;
}


bool wxChoicebook::SetPageText(size_t n, const wxString& strText)
{
    GetChoiceCtrl()->SetString(n, strText);

    return true;
}

wxString wxChoicebook::GetPageText(size_t n) const
{
    return GetChoiceCtrl()->GetString(n);
}

int wxChoicebook::GetPageImage(size_t WXUNUSED(n)) const
{
    return wxNOT_FOUND;
}

bool wxChoicebook::SetPageImage(size_t WXUNUSED(n), int WXUNUSED(imageId))
{
                    
    return false;
}


void wxChoicebook::DoSetWindowVariant(wxWindowVariant variant)
{
    wxBookCtrlBase::DoSetWindowVariant(variant);
    if (m_bookctrl)
        m_bookctrl->SetWindowVariant(variant);
}

void wxChoicebook::SetImageList(wxImageList *imageList)
{
    
    wxBookCtrlBase::SetImageList(imageList);
}


wxBookCtrlEvent* wxChoicebook::CreatePageChangingEvent() const
{
    return new wxBookCtrlEvent(wxEVT_CHOICEBOOK_PAGE_CHANGING, m_windowId);
}

void wxChoicebook::MakeChangedEvent(wxBookCtrlEvent &event)
{
    event.SetEventType(wxEVT_CHOICEBOOK_PAGE_CHANGED);
}


bool
wxChoicebook::InsertPage(size_t n,
                         wxWindow *page,
                         const wxString& text,
                         bool bSelect,
                         int imageId)
{
    if ( !wxBookCtrlBase::InsertPage(n, page, text, bSelect, imageId) )
        return false;

    GetChoiceCtrl()->Insert(text, n);

            if ( int(n) <= m_selection )
    {
                m_selection++;
        GetChoiceCtrl()->Select(m_selection);
    }

    if ( !DoSetSelectionAfterInsertion(n, bSelect) )
        page->Hide();

    return true;
}

wxWindow *wxChoicebook::DoRemovePage(size_t page)
{
    wxWindow *win = wxBookCtrlBase::DoRemovePage(page);

    if ( win )
    {
        GetChoiceCtrl()->Delete(page);

        DoSetSelectionAfterRemoval(page);
    }

    return win;
}


bool wxChoicebook::DeleteAllPages()
{
    GetChoiceCtrl()->Clear();
    return wxBookCtrlBase::DeleteAllPages();
}


void wxChoicebook::OnChoiceSelected(wxCommandEvent& eventChoice)
{
    if ( eventChoice.GetEventObject() != m_bookctrl )
    {
        eventChoice.Skip();
        return;
    }

    const int selNew = eventChoice.GetSelection();

    if ( selNew == m_selection )
    {
                                return;
    }

    SetSelection(selNew);

        if (m_selection != selNew)
        GetChoiceCtrl()->Select(m_selection);
}

#endif 