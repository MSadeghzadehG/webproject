


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_COLOURPICKERCTRL

#include "wx/clrpicker.h"

#ifndef WX_PRECOMP
    #include "wx/textctrl.h"
#endif

const char wxColourPickerCtrlNameStr[] = "colourpicker";
const char wxColourPickerWidgetNameStr[] = "colourpickerwidget";


wxDEFINE_EVENT(wxEVT_COLOURPICKER_CHANGED, wxColourPickerEvent);
wxIMPLEMENT_DYNAMIC_CLASS(wxColourPickerCtrl, wxPickerBase);
wxIMPLEMENT_DYNAMIC_CLASS(wxColourPickerEvent, wxEvent);


#define M_PICKER     ((wxColourPickerWidget*)m_picker)

bool wxColourPickerCtrl::Create( wxWindow *parent, wxWindowID id,
                        const wxColour &col,
                        const wxPoint &pos, const wxSize &size,
                        long style, const wxValidator& validator,
                        const wxString &name )
{
    if (!wxPickerBase::CreateBase(parent, id, col.GetAsString(), pos, size,
                                  style, validator, name))
        return false;

            m_picker = new wxColourPickerWidget(this, wxID_ANY, col,
                                        wxDefaultPosition, wxDefaultSize,
                                        GetPickerStyle(style));

        wxPickerBase::PostCreation();

    m_picker->Connect(wxEVT_COLOURPICKER_CHANGED,
            wxColourPickerEventHandler(wxColourPickerCtrl::OnColourChange),
            NULL, this);

    return true;
}

void wxColourPickerCtrl::SetColour(const wxColour &col)
{
    M_PICKER->SetColour(col);
    UpdateTextCtrlFromPicker();
}

bool wxColourPickerCtrl::SetColour(const wxString &text)
{
    wxColour col(text);         if ( !col.IsOk() )
        return false;
    M_PICKER->SetColour(col);
    UpdateTextCtrlFromPicker();

    return true;
}

void wxColourPickerCtrl::UpdatePickerFromTextCtrl()
{
    wxASSERT(m_text);

        wxColour col(m_text->GetValue());
    if ( !col.IsOk() )
        return;     
    if (M_PICKER->GetColour() != col)
    {
        M_PICKER->SetColour(col);

                wxColourPickerEvent event(this, GetId(), col);
        GetEventHandler()->ProcessEvent(event);
    }
}

void wxColourPickerCtrl::UpdateTextCtrlFromPicker()
{
    if (!m_text)
        return;     
            m_text->ChangeValue(M_PICKER->GetColour().GetAsString());
}




void wxColourPickerCtrl::OnColourChange(wxColourPickerEvent &ev)
{
    UpdateTextCtrlFromPicker();

            wxColourPickerEvent event(this, GetId(), ev.GetColour());
    GetEventHandler()->ProcessEvent(event);
}

#endif  