


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_FILEPICKERCTRL || wxUSE_DIRPICKERCTRL

#include "wx/filepicker.h"
#include "wx/filename.h"

#ifndef WX_PRECOMP
    #include "wx/textctrl.h"
#endif


const char wxFilePickerCtrlNameStr[] = "filepicker";
const char wxFilePickerWidgetNameStr[] = "filepickerwidget";
const char wxDirPickerCtrlNameStr[] = "dirpicker";
const char wxDirPickerWidgetNameStr[] = "dirpickerwidget";
const char wxFilePickerWidgetLabel[] = wxTRANSLATE("Browse");
const char wxDirPickerWidgetLabel[] = wxTRANSLATE("Browse");

wxDEFINE_EVENT( wxEVT_FILEPICKER_CHANGED, wxFileDirPickerEvent );
wxDEFINE_EVENT( wxEVT_DIRPICKER_CHANGED,  wxFileDirPickerEvent );
wxIMPLEMENT_DYNAMIC_CLASS(wxFileDirPickerEvent, wxCommandEvent);


bool wxFileDirPickerCtrlBase::CreateBase(wxWindow *parent,
                                         wxWindowID id,
                                         const wxString &path,
                                         const wxString &message,
                                         const wxString &wildcard,
                                         const wxPoint &pos,
                                         const wxSize &size,
                                         long style,
                                         const wxValidator& validator,
                                         const wxString &name )
{
    if (!wxPickerBase::CreateBase(parent, id, path, pos, size,
                                   style, validator, name))
        return false;

    if (!HasFlag(wxFLP_OPEN) && !HasFlag(wxFLP_SAVE))
        m_windowStyle |= wxFLP_OPEN;     
        wxASSERT_MSG( !(HasFlag(wxFLP_SAVE) && HasFlag(wxFLP_OPEN)),
                  wxT("can't specify both wxFLP_SAVE and wxFLP_OPEN at once") );

    wxASSERT_MSG( !HasFlag(wxFLP_SAVE) || !HasFlag(wxFLP_FILE_MUST_EXIST),
                   wxT("wxFLP_FILE_MUST_EXIST can't be used with wxFLP_SAVE" ) );

    wxASSERT_MSG( !HasFlag(wxFLP_OPEN) || !HasFlag(wxFLP_OVERWRITE_PROMPT),
                  wxT("wxFLP_OVERWRITE_PROMPT can't be used with wxFLP_OPEN") );

        m_pickerIface = CreatePicker(this, path, message, wildcard);
    if ( !m_pickerIface )
        return false;
    m_picker = m_pickerIface->AsControl();

        wxPickerBase::PostCreation();

    DoConnect( m_picker, this );

            if (m_text) m_text->SetMaxLength(512);

    return true;
}

wxString wxFileDirPickerCtrlBase::GetPath() const
{
    return m_pickerIface->GetPath();
}

void wxFileDirPickerCtrlBase::SetPath(const wxString &path)
{
    m_pickerIface->SetPath(path);
    UpdateTextCtrlFromPicker();
}

void wxFileDirPickerCtrlBase::UpdatePickerFromTextCtrl()
{
    wxASSERT(m_text);

                wxString newpath(GetTextCtrlValue());

                        
    if (m_pickerIface->GetPath() != newpath)
    {
        m_pickerIface->SetPath(newpath);

                                if (IsCwdToUpdate())
            wxSetWorkingDirectory(newpath);

                wxFileDirPickerEvent event(GetEventType(), this, GetId(), newpath);
        GetEventHandler()->ProcessEvent(event);
    }
}

void wxFileDirPickerCtrlBase::UpdateTextCtrlFromPicker()
{
    if (!m_text)
        return;     
                m_text->ChangeValue(m_pickerIface->GetPath());
}




void wxFileDirPickerCtrlBase::OnFileDirChange(wxFileDirPickerEvent &ev)
{
    UpdateTextCtrlFromPicker();

            wxFileDirPickerEvent event(GetEventType(), this, GetId(), ev.GetPath());
    GetEventHandler()->ProcessEvent(event);
}

#endif  

#if wxUSE_FILEPICKERCTRL

wxIMPLEMENT_DYNAMIC_CLASS(wxFilePickerCtrl, wxPickerBase);

bool wxFilePickerCtrl::Create(wxWindow *parent,
                              wxWindowID id,
                              const wxString& path,
                              const wxString& message,
                              const wxString& wildcard,
                              const wxPoint& pos,
                              const wxSize& size,
                              long style,
                              const wxValidator& validator,
                              const wxString& name)
{
    if ( !wxFileDirPickerCtrlBase::CreateBase
                                   (
                                        parent, id, path, message, wildcard,
                                        pos, size, style, validator, name
                                   ) )
        return false;

    if ( HasTextCtrl() )
        GetTextCtrl()->AutoCompleteFileNames();

    return true;
}

wxString wxFilePickerCtrl::GetTextCtrlValue() const
{
        return wxFileName(m_text->GetValue()).GetFullPath();
}

#endif 

#if wxUSE_DIRPICKERCTRL
wxIMPLEMENT_DYNAMIC_CLASS(wxDirPickerCtrl, wxPickerBase);

bool wxDirPickerCtrl::Create(wxWindow *parent,
                             wxWindowID id,
                             const wxString& path,
                             const wxString& message,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style,
                             const wxValidator& validator,
                             const wxString& name)
{
    if ( !wxFileDirPickerCtrlBase::CreateBase
                                   (
                                        parent, id, path, message, wxString(),
                                        pos, size, style, validator, name
                                   ) )
        return false;

    if ( HasTextCtrl() )
        GetTextCtrl()->AutoCompleteDirectories();

    return true;
}

wxString wxDirPickerCtrl::GetTextCtrlValue() const
{
        return wxFileName::DirName(m_text->GetValue()).GetPath();
}

#endif 