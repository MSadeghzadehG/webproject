


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_INFOBAR

#include "wx/infobar.h"

#ifndef WX_PRECOMP
    #include "wx/bmpbuttn.h"
    #include "wx/button.h"
    #include "wx/dcmemory.h"
    #include "wx/settings.h"
    #include "wx/statbmp.h"
    #include "wx/stattext.h"
    #include "wx/sizer.h"
#endif 
#include "wx/artprov.h"
#include "wx/scopeguard.h"

wxBEGIN_EVENT_TABLE(wxInfoBarGeneric, wxInfoBarBase)
    EVT_BUTTON(wxID_ANY, wxInfoBarGeneric::OnButton)
wxEND_EVENT_TABLE()


void wxInfoBarGeneric::Init()
{
    m_icon = NULL;
    m_text = NULL;
    m_button = NULL;

    m_showEffect =
    m_hideEffect = wxSHOW_EFFECT_MAX;

        m_effectDuration = 0;
}

bool wxInfoBarGeneric::Create(wxWindow *parent, wxWindowID winid)
{
            Hide();
    if ( !wxWindow::Create(parent, winid) )
        return false;

        SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));

        
        m_icon = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap);

    m_text = new wxStaticText(this, wxID_ANY, "");
    m_text->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));

    m_button = wxBitmapButton::NewCloseButton(this, wxID_ANY);
    m_button->SetToolTip(_("Hide this notification message."));

                        wxSizer * const sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(m_icon, wxSizerFlags().Centre().Border());
    sizer->Add(m_text, wxSizerFlags().Centre());
    sizer->AddStretchSpacer();
    sizer->Add(m_button, wxSizerFlags().Centre().Border());
    SetSizer(sizer);

    return true;
}

bool wxInfoBarGeneric::SetFont(const wxFont& font)
{
    if ( !wxInfoBarBase::SetFont(font) )
        return false;

        if ( m_text )
        m_text->SetFont(font);

    return true;
}

bool wxInfoBarGeneric::SetForegroundColour(const wxColor& colour)
{
    if ( !wxInfoBarBase::SetForegroundColour(colour) )
        return false;

    if ( m_text )
        m_text->SetForegroundColour(colour);

    return true;
}

wxInfoBarGeneric::BarPlacement wxInfoBarGeneric::GetBarPlacement() const
{
    wxSizer * const sizer = GetContainingSizer();
    if ( !sizer )
        return BarPlacement_Unknown;

    const wxSizerItemList& siblings = sizer->GetChildren();
    if ( siblings.GetFirst()->GetData()->GetWindow() == this )
        return BarPlacement_Top;
    else if ( siblings.GetLast()->GetData()->GetWindow() == this )
        return BarPlacement_Bottom;
    else
        return BarPlacement_Unknown;
}

wxShowEffect wxInfoBarGeneric::GetShowEffect() const
{
    if ( m_showEffect != wxSHOW_EFFECT_MAX )
        return m_showEffect;

    switch ( GetBarPlacement() )
    {
        case BarPlacement_Top:
            return wxSHOW_EFFECT_SLIDE_TO_BOTTOM;

        case BarPlacement_Bottom:
            return wxSHOW_EFFECT_SLIDE_TO_TOP;

        default:
            wxFAIL_MSG( "unknown info bar placement" );
            wxFALLTHROUGH;

        case BarPlacement_Unknown:
            return wxSHOW_EFFECT_NONE;
    }
}

wxShowEffect wxInfoBarGeneric::GetHideEffect() const
{
    if ( m_hideEffect != wxSHOW_EFFECT_MAX )
        return m_hideEffect;

    switch ( GetBarPlacement() )
    {
        case BarPlacement_Top:
            return wxSHOW_EFFECT_SLIDE_TO_TOP;

        case BarPlacement_Bottom:
            return wxSHOW_EFFECT_SLIDE_TO_BOTTOM;

        default:
            wxFAIL_MSG( "unknown info bar placement" );
            wxFALLTHROUGH;

        case BarPlacement_Unknown:
            return wxSHOW_EFFECT_NONE;
    }
}

void wxInfoBarGeneric::UpdateParent()
{
    wxWindow * const parent = GetParent();
    parent->Layout();
}

void wxInfoBarGeneric::DoHide()
{
    HideWithEffect(GetHideEffect(), GetEffectDuration());

    UpdateParent();
}

void wxInfoBarGeneric::DoShow()
{
                    
            wxWindowBase::Show();

        UpdateParent();

            wxWindowBase::Show(false);


        ShowWithEffect(GetShowEffect(), GetEffectDuration());
}

void wxInfoBarGeneric::ShowMessage(const wxString& msg, int flags)
{
        const int icon = flags & wxICON_MASK;
    if ( !icon || (icon == wxICON_NONE) )
    {
        m_icon->Hide();
    }
    else     {
        m_icon->SetBitmap(wxArtProvider::GetBitmap(
                            wxArtProvider::GetMessageBoxIconId(flags),
                          wxART_BUTTON));
        m_icon->Show();
    }

            m_text->SetLabel(wxControl::EscapeMnemonics(msg));


        if ( !IsShown() )
    {
        DoShow();
    }
    else     {
                Layout();
    }
}

void wxInfoBarGeneric::Dismiss()
{
    DoHide();
}

void wxInfoBarGeneric::AddButton(wxWindowID btnid, const wxString& label)
{
    wxSizer * const sizer = GetSizer();
    wxCHECK_RET( sizer, "must be created first" );

            if ( sizer->Detach(m_button) )
    {
        m_button->Hide();
    }

    wxButton * const button = new wxButton(this, btnid, label);

#ifdef __WXMAC__
        button->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif 
    sizer->Add(button, wxSizerFlags().Centre().DoubleBorder());
}

size_t wxInfoBarGeneric::GetButtonCount() const
{
    size_t count = 0;
    wxSizer * const sizer = GetSizer();
    if ( !sizer )
        return 0;

        const wxSizerItemList& items = sizer->GetChildren();
    for ( wxSizerItemList::compatibility_iterator node = items.GetLast();
          node != items.GetFirst();
          node = node->GetPrevious() )
    {
        const wxSizerItem * const item = node->GetData();

                        if ( item->IsSpacer() )
            break;

                if ( item->GetWindow() == m_button )
            return 0;

        ++count;
    }

    return count;
}

wxWindowID wxInfoBarGeneric::GetButtonId(size_t idx) const
{
    wxCHECK_MSG( idx < GetButtonCount(), wxID_NONE,
                 "Invalid infobar button position" );

    wxSizer * const sizer = GetSizer();
    if ( !sizer )
        return wxID_NONE;

    bool foundSpacer = false;

    size_t count = 0;
    const wxSizerItemList& items = sizer->GetChildren();
    for ( wxSizerItemList::compatibility_iterator node = items.GetLast();
          node != items.GetFirst() || node != items.GetLast();
          )
    {
        const wxSizerItem * const item = node->GetData();

        if ( item->IsSpacer() )
            foundSpacer = true;

        if ( foundSpacer )
        {
            if ( !item->IsSpacer() )
            {
                if ( count == idx )
                {
                  if ( item->GetWindow() != m_button )
                    return item->GetWindow()->GetId();
                }

                ++count;
            }

            node = node->GetNext();
        }
        else
        {
            node = node->GetPrevious();
        }
    }

    return wxID_NONE;
}

bool wxInfoBarGeneric::HasButtonId(wxWindowID btnid) const
{
    wxSizer * const sizer = GetSizer();
    if ( !sizer )
        return false;

            const wxSizerItemList& items = sizer->GetChildren();
    for ( wxSizerItemList::compatibility_iterator node = items.GetLast();
          node != items.GetFirst();
          node = node->GetPrevious() )
    {
        const wxSizerItem * const item = node->GetData();

                        if ( item->IsSpacer() )
            return false;

                if ( item->GetWindow()->GetId() == btnid )
            return true;
    }

    return false;
}

void wxInfoBarGeneric::RemoveButton(wxWindowID btnid)
{
    wxSizer * const sizer = GetSizer();
    wxCHECK_RET( sizer, "must be created first" );

                const wxSizerItemList& items = sizer->GetChildren();
    for ( wxSizerItemList::compatibility_iterator node = items.GetLast();
          node != items.GetFirst();
          node = node->GetPrevious() )
    {
        const wxSizerItem * const item = node->GetData();

                                if ( item->IsSpacer() )
        {
            wxFAIL_MSG( wxString::Format("button with id %d not found", btnid) );
            return;
        }

                if ( item->GetWindow()->GetId() == btnid )
        {
            delete item->GetWindow();
            break;
        }
    }

        if ( sizer->GetChildren().GetLast()->GetData()->IsSpacer() )
    {
                        sizer->Add(m_button, wxSizerFlags().Centre().DoubleBorder());
        m_button->Show();
    }
}

void wxInfoBarGeneric::OnButton(wxCommandEvent& WXUNUSED(event))
{
    DoHide();
}

#endif 