


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_FONTPICKERCTRL

#include "wx/fontpicker.h"

#ifndef WX_PRECOMP
    #include "wx/textctrl.h"
#endif

#include "wx/fontenum.h"
#include "wx/tokenzr.h"


const char wxFontPickerCtrlNameStr[] = "fontpicker";
const char wxFontPickerWidgetNameStr[] = "fontpickerwidget";

wxDEFINE_EVENT(wxEVT_FONTPICKER_CHANGED, wxFontPickerEvent);
wxIMPLEMENT_DYNAMIC_CLASS(wxFontPickerCtrl, wxPickerBase);
wxIMPLEMENT_DYNAMIC_CLASS(wxFontPickerEvent, wxCommandEvent);


bool wxFontPickerCtrl::Create( wxWindow *parent, wxWindowID id,
                        const wxFont &initial,
                        const wxPoint &pos, const wxSize &size,
                        long style, const wxValidator& validator,
                        const wxString &name )
{
    if (!wxPickerBase::CreateBase(parent, id,
                                  Font2String(initial.IsOk() ? initial
                                                             : *wxNORMAL_FONT),
                                  pos, size, style, validator, name))
        return false;

        m_picker = new wxFontPickerWidget(this, wxID_ANY, initial,
                                      wxDefaultPosition, wxDefaultSize,
                                      GetPickerStyle(style));
        wxPickerBase::PostCreation();

    m_picker->Connect(wxEVT_FONTPICKER_CHANGED,
            wxFontPickerEventHandler(wxFontPickerCtrl::OnFontChange),
            NULL, this);

    return true;
}

wxString wxFontPickerCtrl::Font2String(const wxFont &f)
{
    wxString ret = f.GetNativeFontInfoUserDesc();
#ifdef __WXMSW__
            wxFontEncoding enc = f.GetEncoding();
    if ( enc != wxFONTENCODING_DEFAULT && enc != wxFONTENCODING_SYSTEM )
        ret = ret.BeforeLast(wxT(' '));
#endif
    return ret;
}

wxFont wxFontPickerCtrl::String2Font(const wxString &s)
{
    wxString str(s);
    wxFont ret;
    double n;

            wxString size = str.AfterLast(wxT(' '));
    if (size.ToDouble(&n))
    {
        if (n < 1)
            str = str.Left(str.length() - size.length()) + wxT("1");
        else if (n >= m_nMaxPointSize)
            str = str.Left(str.length() - size.length()) +
                  wxString::Format(wxT("%d"), m_nMaxPointSize);
    }

    if (!ret.SetNativeFontInfoUserDesc(str))
        return wxNullFont;

    return ret;
}

void wxFontPickerCtrl::SetSelectedFont(const wxFont &f)
{
    GetPickerWidget()->SetSelectedFont(f);
    UpdateTextCtrlFromPicker();
}

void wxFontPickerCtrl::UpdatePickerFromTextCtrl()
{
    wxASSERT(m_text);

                    wxFont f = String2Font(m_text->GetValue());
    if (!f.IsOk())
        return;     
    if (GetPickerWidget()->GetSelectedFont() != f)
    {
        GetPickerWidget()->SetSelectedFont(f);

                wxFontPickerEvent event(this, GetId(), f);
        GetEventHandler()->ProcessEvent(event);
    }
}

void wxFontPickerCtrl::UpdateTextCtrlFromPicker()
{
    if (!m_text)
        return;     
            m_text->ChangeValue(Font2String(GetPickerWidget()->GetSelectedFont()));
}




void wxFontPickerCtrl::OnFontChange(wxFontPickerEvent &ev)
{
    UpdateTextCtrlFromPicker();

            wxFontPickerEvent event(this, GetId(), ev.GetFont());
    GetEventHandler()->ProcessEvent(event);
}

#endif  