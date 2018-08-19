
#include "wx/wxprec.h"







#if wxUSE_DIRDLG

#include "wx/dirdlg.h"

#ifndef WX_PRECOMP
    #include "wx/intl.h"
    #include "wx/filedlg.h"
#endif

#include "wx/gtk/private.h"

extern "C" {
static void gtk_dirdialog_response_callback(GtkWidget * WXUNUSED(w),
                                             gint response,
                                             wxDirDialog *dialog)
{
    if (response == GTK_RESPONSE_ACCEPT)
        dialog->GTKOnAccept();
    else         dialog->GTKOnCancel();
}
}


wxIMPLEMENT_DYNAMIC_CLASS(wxDirDialog, wxDialog);

wxDirDialog::wxDirDialog(wxWindow* parent,
                         const wxString& title,
                         const wxString& defaultPath,
                         long style,
                         const wxPoint& pos,
                         const wxSize& WXUNUSED(sz),
                         const wxString& WXUNUSED(name))
{
    Create(parent, title, defaultPath, style, pos);
}

bool wxDirDialog::Create(wxWindow* parent,
                         const wxString& title,
                         const wxString& defaultPath,
                         long style,
                         const wxPoint& pos,
                         const wxSize& WXUNUSED(sz),
                         const wxString& WXUNUSED(name))
{
    m_message = title;

    parent = GetParentForModalDialog(parent, style);

    if (!PreCreation(parent, pos, wxDefaultSize) ||
        !CreateBase(parent, wxID_ANY, pos, wxDefaultSize, style,
                wxDefaultValidator, wxT("dirdialog")))
    {
        wxFAIL_MSG( wxT("wxDirDialog creation failed") );
        return false;
    }

    GtkWindow* gtk_parent = NULL;
    if (parent)
        gtk_parent = GTK_WINDOW( gtk_widget_get_toplevel(parent->m_widget) );

    m_widget = gtk_file_chooser_dialog_new(
                   wxGTK_CONV(m_message),
                   gtk_parent,
                   GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                   NULL);
    g_object_ref(m_widget);

    gtk_dialog_set_default_response(GTK_DIALOG(m_widget), GTK_RESPONSE_ACCEPT);
#if GTK_CHECK_VERSION(2,18,0)
#ifndef __WXGTK3__
    if (gtk_check_version(2,18,0) == NULL)
#endif
    {
        gtk_file_chooser_set_create_folders(
            GTK_FILE_CHOOSER(m_widget), (style & wxDD_DIR_MUST_EXIST) == 0);
    }
#endif

                        
    g_signal_connect (m_widget, "response",
        G_CALLBACK (gtk_dirdialog_response_callback), this);

    if ( !defaultPath.empty() )
        SetPath(defaultPath);

    return true;
}

void wxDirDialog::GTKOnAccept()
{
    wxGtkString str(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(m_widget)));
    m_selectedDirectory = wxString::FromUTF8(str);

        if (HasFlag(wxDD_CHANGE_DIR))
    {
        wxSetWorkingDirectory(m_selectedDirectory);
    }

    EndDialog(wxID_OK);
}

void wxDirDialog::GTKOnCancel()
{
    EndDialog(wxID_CANCEL);
}

void wxDirDialog::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
    if (!m_wxwindow)
        return;

    wxDirDialogBase::DoSetSize( x, y, width, height, sizeFlags );
}

void wxDirDialog::SetPath(const wxString& dir)
{
    if (wxDirExists(dir))
    {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(m_widget),
                                            wxGTK_CONV_FN(dir));
    }
}

wxString wxDirDialog::GetPath() const
{
    return m_selectedDirectory;
}

#endif 