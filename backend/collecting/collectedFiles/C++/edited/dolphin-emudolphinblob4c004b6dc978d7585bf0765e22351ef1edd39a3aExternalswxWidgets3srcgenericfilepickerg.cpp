


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_FILEPICKERCTRL || wxUSE_DIRPICKERCTRL

#include "wx/filename.h"
#include "wx/filepicker.h"

#include "wx/scopedptr.h"



wxIMPLEMENT_DYNAMIC_CLASS(wxGenericFileButton, wxButton);
wxIMPLEMENT_DYNAMIC_CLASS(wxGenericDirButton, wxButton);


bool wxGenericFileDirButton::Create(wxWindow *parent,
                                    wxWindowID id,
                                    const wxString& label,
                                    const wxString& path,
                                    const wxString& message,
                                    const wxString& wildcard,
                                    const wxPoint& pos,
                                    const wxSize& size,
                                    long style,
                                    const wxValidator& validator,
                                    const wxString& name)
{
    m_pickerStyle = style;

                long styleButton = 0;
    wxString labelButton;
    if ( m_pickerStyle & wxPB_SMALL )
    {
        labelButton = _("...");
        styleButton = wxBU_EXACTFIT;
    }
    else
    {
        labelButton = label;
    }

        if ( !wxButton::Create(parent, id, labelButton,
                           pos, size, styleButton, validator, name) )
    {
        wxFAIL_MSG( wxT("wxGenericFileButton creation failed") );
        return false;
    }

        Connect(GetId(), wxEVT_BUTTON,
            wxCommandEventHandler(wxGenericFileDirButton::OnButtonClick),
            NULL, this);

        m_path = path;
    m_message = message;
    m_wildcard = wildcard;

    return true;
}

void wxGenericFileDirButton::OnButtonClick(wxCommandEvent& WXUNUSED(ev))
{
    wxScopedPtr<wxDialog> p(CreateDialog());
    if (p->ShowModal() == wxID_OK)
    {
                UpdatePathFromDialog(p.get());

                wxFileDirPickerEvent event(GetEventType(), this, GetId(), m_path);
        GetEventHandler()->ProcessEvent(event);
    }
}

void wxGenericFileDirButton::SetInitialDirectory(const wxString& dir)
{
    m_initialDir = dir;
}


wxDialog *wxGenericFileButton::CreateDialog()
{
                wxFileName fn(m_path);
    wxString initialDir = fn.GetPath();
    if ( initialDir.empty() )
        initialDir = m_initialDir;

    return new wxFileDialog
               (
                    GetDialogParent(),
                    m_message,
                    initialDir,
                    fn.GetFullName(),
                    m_wildcard,
                    GetDialogStyle()
               );
}


wxDialog *wxGenericDirButton::CreateDialog()
{
    wxDirDialog* const dialog = new wxDirDialog
                                    (
                                        GetDialogParent(),
                                        m_message,
                                        m_path.empty() ? m_initialDir : m_path,
                                        GetDialogStyle()
                                    );
    return dialog;
}

#endif      