
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_DIRDLG

#ifndef WX_PRECOMP
    #include "wx/textctrl.h"
    #include "wx/button.h"
    #include "wx/checkbox.h"
    #include "wx/sizer.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/msgdlg.h"
    #include "wx/bmpbuttn.h"
#endif

#include "wx/statline.h"
#include "wx/dirctrl.h"
#include "wx/generic/dirdlgg.h"
#include "wx/artprov.h"
#include "wx/menu.h"


static const int ID_DIRCTRL = 1000;
static const int ID_TEXTCTRL = 1001;
static const int ID_NEW = 1004;
static const int ID_SHOW_HIDDEN = 1005;
static const int ID_GO_HOME = 1006;


wxIMPLEMENT_DYNAMIC_CLASS(wxGenericDirDialog, wxDialog);

wxBEGIN_EVENT_TABLE(wxGenericDirDialog, wxDialog)
    EVT_CLOSE                (wxGenericDirDialog::OnCloseWindow)
    EVT_BUTTON               (wxID_OK,        wxGenericDirDialog::OnOK)
    EVT_BUTTON               (ID_NEW,         wxGenericDirDialog::OnNew)
    EVT_BUTTON               (ID_GO_HOME,     wxGenericDirDialog::OnGoHome)
    EVT_TREE_KEY_DOWN        (wxID_ANY,       wxGenericDirDialog::OnTreeKeyDown)
    EVT_TREE_SEL_CHANGED     (wxID_ANY,       wxGenericDirDialog::OnTreeSelected)
    EVT_TEXT_ENTER           (ID_TEXTCTRL,    wxGenericDirDialog::OnOK)
    EVT_CHECKBOX             (ID_SHOW_HIDDEN, wxGenericDirDialog::OnShowHidden)
wxEND_EVENT_TABLE()

wxGenericDirDialog::wxGenericDirDialog(wxWindow* parent, const wxString& title,
                                       const wxString& defaultPath, long style,
                                       const wxPoint& pos, const wxSize& sz,
                                       const wxString& name)
{
    Create(parent, title, defaultPath, style, pos, sz, name);
}

bool wxGenericDirDialog::Create(wxWindow* parent,
                                const wxString& title,
                                const wxString& defaultPath, long style,
                                const wxPoint& pos,
                                const wxSize& sz,
                                const wxString& name)
{
    wxBusyCursor cursor;

    parent = GetParentForModalDialog(parent, style);

    if (!wxDirDialogBase::Create(parent, title, defaultPath, style, pos, sz, name))
        return false;

    m_path = defaultPath;
    if (m_path == wxT("~"))
        wxGetHomeDir(&m_path);
    if (m_path == wxT("."))
        m_path = wxGetCwd();

    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

        wxSizer* buttonsizer = new wxBoxSizer( wxHORIZONTAL );

    wxBitmapButton* homeButton =
        new wxBitmapButton(this, ID_GO_HOME,
                           wxArtProvider::GetBitmap(wxART_GO_HOME, wxART_BUTTON));
    buttonsizer->Add( homeButton, 0, wxLEFT|wxRIGHT, 10 );

            if (!HasFlag(wxDD_DIR_MUST_EXIST))
    {
        wxBitmapButton* newButton =
            new wxBitmapButton(this, ID_NEW,
                            wxArtProvider::GetBitmap(wxART_NEW_DIR, wxART_BUTTON));
        buttonsizer->Add( newButton, 0, wxRIGHT, 10 );
#if wxUSE_TOOLTIPS
        newButton->SetToolTip(_("Create new directory"));
#endif
    }

#if wxUSE_TOOLTIPS
    homeButton->SetToolTip(_("Go to home directory"));
#endif

    topsizer->Add( buttonsizer, 0, wxTOP | wxALIGN_RIGHT, 10 );

        m_dirCtrl = NULL;                           long dirStyle = wxDIRCTRL_DIR_ONLY | wxDEFAULT_CONTROL_BORDER;

#ifdef __WXMSW__
    if (!HasFlag(wxDD_DIR_MUST_EXIST))
    {
                        dirStyle |= wxDIRCTRL_EDIT_LABELS;
    }
#endif

    m_dirCtrl = new wxGenericDirCtrl(this, ID_DIRCTRL,
                                     m_path, wxDefaultPosition,
                                     wxSize(300, 200),
                                     dirStyle);

    wxSizerFlags flagsBorder2;
    flagsBorder2.DoubleBorder(wxTOP | wxLEFT | wxRIGHT);

    topsizer->Add(m_dirCtrl, wxSizerFlags(flagsBorder2).Proportion(1).Expand());

        wxCheckBox *
        check = new wxCheckBox(this, ID_SHOW_HIDDEN, _("Show &hidden directories"));
    topsizer->Add(check, wxSizerFlags(flagsBorder2).Right());

        m_input = new wxTextCtrl( this, ID_TEXTCTRL, m_path, wxDefaultPosition );
    topsizer->Add(m_input, wxSizerFlags(flagsBorder2).Expand());

        wxSizer *buttonSizer = CreateSeparatedButtonSizer(wxOK | wxCANCEL);
    if ( buttonSizer )
    {
        topsizer->Add(buttonSizer, wxSizerFlags().Expand().DoubleBorder());
    }

    m_input->SetFocus();

    SetAutoLayout( true );
    SetSizer( topsizer );

    topsizer->SetSizeHints( this );
    topsizer->Fit( this );

    Centre( wxBOTH );

    return true;
}

void wxGenericDirDialog::EndModal(int retCode)
{
        if (retCode == wxID_OK && HasFlag(wxDD_CHANGE_DIR))
        wxSetWorkingDirectory(m_path);

    wxDialog::EndModal(retCode);
}

void wxGenericDirDialog::OnCloseWindow(wxCloseEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void wxGenericDirDialog::OnOK(wxCommandEvent& WXUNUSED(event))
{
    m_path = m_input->GetValue();

        if (wxDirExists(m_path))
    {
                EndModal(wxID_OK);
        return;
    }

        wxString msg;
    msg.Printf(_("The directory '%s' does not exist\nCreate it now?"),
               m_path.c_str());
    wxMessageDialog dialog(this, msg, _("Directory does not exist"),
                           wxYES_NO | wxICON_WARNING);

    if ( dialog.ShowModal() == wxID_YES )
    {
                wxLogNull log;
        if (wxMkdir(m_path))
        {
                        EndModal(wxID_OK);
            return;
        }
        else
        {
                        msg.Printf(_("Failed to create directory '%s'\n(Do you have the required permissions?)"),
                       m_path.c_str());
            wxMessageDialog errmsg(this, msg, _("Error creating directory"), wxOK | wxICON_ERROR);
            errmsg.ShowModal();
                    }
    }
    }

void wxGenericDirDialog::SetPath(const wxString& path)
{
    m_dirCtrl->SetPath(path);
    m_path = path;
}

wxString wxGenericDirDialog::GetPath(void) const
{
    return m_path;
}

int wxGenericDirDialog::ShowModal()
{
    m_input->SetValue( m_path );
    return wxDialog::ShowModal();
}

void wxGenericDirDialog::OnTreeSelected( wxTreeEvent &event )
{
    if (!m_dirCtrl)
        return;

    wxTreeItemId item = event.GetItem();

    wxDirItemData *data = NULL;

    if(item.IsOk())
        data = (wxDirItemData*)m_dirCtrl->GetTreeCtrl()->GetItemData(item);

    if (data)
       m_input->SetValue( data->m_path );
}

void wxGenericDirDialog::OnTreeKeyDown( wxTreeEvent &WXUNUSED(event) )
{
    if (!m_dirCtrl)
        return;

    wxDirItemData *data = (wxDirItemData*)m_dirCtrl->GetTreeCtrl()->GetItemData(m_dirCtrl->GetTreeCtrl()->GetSelection());
    if (data)
        m_input->SetValue( data->m_path );
}

void wxGenericDirDialog::OnShowHidden( wxCommandEvent& event )
{
    if (!m_dirCtrl)
        return;

    m_dirCtrl->ShowHidden( event.GetInt() != 0 );
}

void wxGenericDirDialog::OnNew( wxCommandEvent& WXUNUSED(event) )
{
    wxTreeItemId id = m_dirCtrl->GetTreeCtrl()->GetSelection();
    if ((id == m_dirCtrl->GetTreeCtrl()->GetRootItem()) ||
        (m_dirCtrl->GetTreeCtrl()->GetItemParent(id) == m_dirCtrl->GetTreeCtrl()->GetRootItem()))
    {
        wxMessageDialog msg(this, _("You cannot add a new directory to this section."),
                            _("Create directory"), wxOK | wxICON_INFORMATION );
        msg.ShowModal();
        return;
    }

    wxTreeItemId parent = id ;     wxDirItemData *data = (wxDirItemData*)m_dirCtrl->GetTreeCtrl()->GetItemData( parent );
    wxASSERT( data );

    wxString new_name( _("NewName") );
    wxString path( data->m_path );
    if (!wxEndsWithPathSeparator(path))
        path += wxFILE_SEP_PATH;
    path += new_name;
    if (wxDirExists(path))
    {
                int i = 0;
        do {
            new_name = _("NewName");
            wxString num;
            num.Printf( wxT("%d"), i );
            new_name += num;

            path = data->m_path;
            if (!wxEndsWithPathSeparator(path))
                path += wxFILE_SEP_PATH;
            path += new_name;
            i++;
        } while (wxDirExists(path));
    }

    wxLogNull log;
    if (!wxMkdir(path))
    {
        wxMessageDialog dialog(this, _("Operation not permitted."), _("Error"), wxOK | wxICON_ERROR );
        dialog.ShowModal();
        return;
    }

    wxDirItemData *new_data = new wxDirItemData( path, new_name, true );

            wxTreeItemId new_id = m_dirCtrl->GetTreeCtrl()->AppendItem( parent, new_name, 0, 0, new_data );
    m_dirCtrl->GetTreeCtrl()->EnsureVisible( new_id );
    m_dirCtrl->GetTreeCtrl()->EditLabel( new_id );
}

void wxGenericDirDialog::OnGoHome(wxCommandEvent& WXUNUSED(event))
{
    SetPath(wxGetUserHome());
}

#endif 