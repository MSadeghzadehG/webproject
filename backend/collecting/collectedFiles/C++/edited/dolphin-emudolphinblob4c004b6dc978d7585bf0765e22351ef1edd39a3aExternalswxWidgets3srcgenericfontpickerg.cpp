


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_FONTPICKERCTRL

#include "wx/fontpicker.h"

#include "wx/fontdlg.h"



wxIMPLEMENT_DYNAMIC_CLASS(wxGenericFontButton, wxButton);


bool wxGenericFontButton::Create( wxWindow *parent, wxWindowID id,
                        const wxFont &initial, const wxPoint &pos,
                        const wxSize &size, long style,
                        const wxValidator& validator, const wxString &name)
{
    wxString label = (style & wxFNTP_FONTDESC_AS_LABEL) ?
                        wxString() :                         _("Choose font");

        if (!wxButton::Create( parent, id, label, pos,
                           size, style, validator, name ))
    {
        wxFAIL_MSG( wxT("wxGenericFontButton creation failed") );
        return false;
    }

        Connect(GetId(), wxEVT_BUTTON,
            wxCommandEventHandler(wxGenericFontButton::OnButtonClick),
            NULL, this);

    InitFontData();

    m_selectedFont = initial.IsOk() ? initial : *wxNORMAL_FONT;
    UpdateFont();

    return true;
}

void wxGenericFontButton::InitFontData()
{
    m_data.SetAllowSymbols(true);
    m_data.SetColour(*wxBLACK);
    m_data.EnableEffects(true);
}

void wxGenericFontButton::OnButtonClick(wxCommandEvent& WXUNUSED(ev))
{
        m_data.SetInitialFont(m_selectedFont);

        wxFontDialog dlg(this, m_data);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_data = dlg.GetFontData();
        SetSelectedFont(m_data.GetChosenFont());

                wxFontPickerEvent event(this, GetId(), m_selectedFont);
        GetEventHandler()->ProcessEvent(event);
    }
}

void wxGenericFontButton::UpdateFont()
{
    if ( !m_selectedFont.IsOk() )
        return;

    SetForegroundColour(m_data.GetColour());

    if (HasFlag(wxFNTP_USEFONT_FOR_LABEL))
    {
                wxButton::SetFont(m_selectedFont);
    }

    if (HasFlag(wxFNTP_FONTDESC_AS_LABEL))
    {
        SetLabel(wxString::Format(wxT("%s, %d"),
                 m_selectedFont.GetFaceName().c_str(),
                 m_selectedFont.GetPointSize()));
    }
}

#endif      