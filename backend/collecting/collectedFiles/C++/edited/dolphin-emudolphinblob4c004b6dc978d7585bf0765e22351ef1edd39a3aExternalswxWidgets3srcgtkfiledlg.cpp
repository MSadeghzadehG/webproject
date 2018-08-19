
#include "wx/wxprec.h"

#if wxUSE_FILEDLG

#include "wx/filedlg.h"

#ifndef WX_PRECOMP
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/msgdlg.h"
#endif

#include <gtk/gtk.h>
#include "wx/gtk/private.h"

#ifdef __UNIX__
#include <unistd.h> #endif

#include "wx/filename.h" #include "wx/tokenzr.h" #include "wx/filefn.h" #include "wx/modalhook.h"


extern "C" {
static void gtk_filedialog_ok_callback(GtkWidget *widget, wxFileDialog *dialog)
{
    int style = dialog->GetWindowStyle();
    wxGtkString filename(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget)));

    #ifndef __WXGTK3__
#if GTK_CHECK_VERSION(2,7,3)
    if (gtk_check_version(2, 7, 3) != NULL)
#endif
    {
        if ((style & wxFD_SAVE) && (style & wxFD_OVERWRITE_PROMPT))
        {
            if ( g_file_test(filename, G_FILE_TEST_EXISTS) )
            {
                wxString msg;

                msg.Printf(
                    _("File '%s' already exists, do you really want to overwrite it?"),
                    wxString::FromUTF8(filename));

                wxMessageDialog dlg(dialog, msg, _("Confirm"),
                                   wxYES_NO | wxICON_QUESTION);
                if (dlg.ShowModal() != wxID_YES)
                    return;
            }
        }
    }
#endif

    if (style & wxFD_FILE_MUST_EXIST)
    {
        if ( !g_file_test(filename, G_FILE_TEST_EXISTS) )
        {
            wxMessageDialog dlg( dialog, _("Please choose an existing file."),
                                 _("Error"), wxOK| wxICON_ERROR);
            dlg.ShowModal();
            return;
        }
    }

        if (style & wxFD_CHANGE_DIR)
    {
                wxGtkString folder(g_path_get_dirname(filename));
        if ( chdir(folder) != 0 )
        {
            wxLogSysError(_("Changing current directory to \"%s\" failed"),
                          wxString::FromUTF8(folder));
        }
    }

    wxCommandEvent event(wxEVT_BUTTON, wxID_OK);
    event.SetEventObject(dialog);
    dialog->HandleWindowEvent(event);
}
}


extern "C"
{

static void
gtk_filedialog_cancel_callback(GtkWidget * WXUNUSED(w), wxFileDialog *dialog)
{
    wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, wxID_CANCEL);
    event.SetEventObject(dialog);
    dialog->HandleWindowEvent(event);
}

static void gtk_filedialog_response_callback(GtkWidget *w,
                                             gint response,
                                             wxFileDialog *dialog)
{
    if (response == GTK_RESPONSE_ACCEPT)
        gtk_filedialog_ok_callback(w, dialog);
    else            gtk_filedialog_cancel_callback(w, dialog);
}

static void gtk_filedialog_selchanged_callback(GtkFileChooser *chooser,
                                               wxFileDialog *dialog)
{
    wxGtkString filename(gtk_file_chooser_get_preview_filename(chooser));

    dialog->GTKSelectionChanged(wxString::FromUTF8(filename));
}


static void gtk_filedialog_update_preview_callback(GtkFileChooser *chooser,
                                                   gpointer user_data)
{
    GtkWidget *preview = GTK_WIDGET(user_data);

    wxGtkString filename(gtk_file_chooser_get_preview_filename(chooser));

    if ( !filename )
        return;

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(filename, 128, 128, NULL);
    gboolean have_preview = pixbuf != NULL;

    gtk_image_set_from_pixbuf(GTK_IMAGE(preview), pixbuf);
    if ( pixbuf )
        g_object_unref (pixbuf);

    gtk_file_chooser_set_preview_widget_active(chooser, have_preview);
}

} 
void wxFileDialog::AddChildGTK(wxWindowGTK* child)
{
        gtk_widget_set_size_request(
        child->m_widget, child->GetMinWidth(), child->m_height);

    gtk_file_chooser_set_extra_widget(
        GTK_FILE_CHOOSER(m_widget), child->m_widget);
}


wxIMPLEMENT_DYNAMIC_CLASS(wxFileDialog, wxFileDialogBase);

wxBEGIN_EVENT_TABLE(wxFileDialog,wxFileDialogBase)
    EVT_BUTTON(wxID_OK, wxFileDialog::OnFakeOk)
    EVT_SIZE(wxFileDialog::OnSize)
wxEND_EVENT_TABLE()

wxFileDialog::wxFileDialog(wxWindow *parent, const wxString& message,
                           const wxString& defaultDir,
                           const wxString& defaultFileName,
                           const wxString& wildCard,
                           long style, const wxPoint& pos,
                           const wxSize& sz,
                           const wxString& name)
    : wxFileDialogBase()
{
    Create(parent, message, defaultDir, defaultFileName, wildCard, style, pos, sz, name);
}

bool wxFileDialog::Create(wxWindow *parent, const wxString& message,
                           const wxString& defaultDir,
                           const wxString& defaultFileName,
                           const wxString& wildCard,
                           long style, const wxPoint& pos,
                           const wxSize& sz,
                           const wxString& name)
{
    parent = GetParentForModalDialog(parent, style);

    if (!wxFileDialogBase::Create(parent, message, defaultDir, defaultFileName,
                                  wildCard, style, pos, sz, name))
    {
        return false;
    }

    if (!PreCreation(parent, pos, wxDefaultSize) ||
        !CreateBase(parent, wxID_ANY, pos, wxDefaultSize, style,
                wxDefaultValidator, wxT("filedialog")))
    {
        wxFAIL_MSG( wxT("wxFileDialog creation failed") );
        return false;
    }

    GtkFileChooserAction gtk_action;
    GtkWindow* gtk_parent = NULL;
    if (parent)
        gtk_parent = GTK_WINDOW( gtk_widget_get_toplevel(parent->m_widget) );

    const gchar* ok_btn_stock;
    if ( style & wxFD_SAVE )
    {
        gtk_action = GTK_FILE_CHOOSER_ACTION_SAVE;
        ok_btn_stock = GTK_STOCK_SAVE;
    }
    else
    {
        gtk_action = GTK_FILE_CHOOSER_ACTION_OPEN;
        ok_btn_stock = GTK_STOCK_OPEN;
    }

    m_widget = gtk_file_chooser_dialog_new(
                   wxGTK_CONV(m_message),
                   gtk_parent,
                   gtk_action,
                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                   ok_btn_stock, GTK_RESPONSE_ACCEPT,
                   NULL);
    g_object_ref(m_widget);
    GtkFileChooser* file_chooser = GTK_FILE_CHOOSER(m_widget);

    m_fc.SetWidget(file_chooser);

    gtk_dialog_set_default_response(GTK_DIALOG(m_widget), GTK_RESPONSE_ACCEPT);

    if ( style & wxFD_MULTIPLE )
        gtk_file_chooser_set_select_multiple(file_chooser, true);

                            
    g_signal_connect (m_widget, "response",
        G_CALLBACK (gtk_filedialog_response_callback), this);

    g_signal_connect (m_widget, "selection-changed",
        G_CALLBACK (gtk_filedialog_selchanged_callback), this);

        SetWildcard(wildCard);

    wxString defaultFileNameWithExt = defaultFileName;
    if ( !wildCard.empty() && !defaultFileName.empty() &&
            !wxFileName(defaultFileName).HasExt() )
    {
                        const wxFileName fnWC(m_fc.GetCurrentWildCard());
        if ( fnWC.HasExt() )
        {
                                                const wxString& ext = fnWC.GetExt();
            if ( ext.find_first_of("?*") == wxString::npos )
                defaultFileNameWithExt << "." << ext;
        }
    }


                wxFileName fn;
    if ( defaultDir.empty() )
        fn.Assign(defaultFileNameWithExt);
    else if ( !defaultFileNameWithExt.empty() )
        fn.Assign(defaultDir, defaultFileNameWithExt);
    else
        fn.AssignDir(defaultDir);

        fn.MakeAbsolute();     const wxString dir = fn.GetPath();
    if ( !dir.empty() )
    {
        gtk_file_chooser_set_current_folder(file_chooser, wxGTK_CONV_FN(dir));
    }

    const wxString fname = fn.GetFullName();
    if ( style & wxFD_SAVE )
    {
        if ( !fname.empty() )
        {
            gtk_file_chooser_set_current_name(file_chooser, wxGTK_CONV_FN(fname));
        }

#if GTK_CHECK_VERSION(2,7,3)
        if ((style & wxFD_OVERWRITE_PROMPT)
#ifndef __WXGTK3__
            && gtk_check_version(2,7,3) == NULL
#endif
            )
        {
            gtk_file_chooser_set_do_overwrite_confirmation(file_chooser, true);
        }
#endif
    }
    else     {
        if ( !fname.empty() )
        {
            gtk_file_chooser_set_filename(file_chooser,
                                          wxGTK_CONV_FN(fn.GetFullPath()));
        }
    }

    if ( style & wxFD_PREVIEW )
    {
        GtkWidget *previewImage = gtk_image_new();

        gtk_file_chooser_set_preview_widget(file_chooser, previewImage);
        g_signal_connect(m_widget, "update-preview",
                         G_CALLBACK(gtk_filedialog_update_preview_callback),
                         previewImage);
    }

    return true;
}

wxFileDialog::~wxFileDialog()
{
    if (m_extraControl)
    {
                        gtk_file_chooser_set_extra_widget(
            GTK_FILE_CHOOSER(m_widget), NULL);
    }
}

void wxFileDialog::OnFakeOk(wxCommandEvent& WXUNUSED(event))
{
            wxGtkString
        str(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(m_widget)));
    m_dir = wxString::FromUTF8(str);

    EndDialog(wxID_OK);
}

int wxFileDialog::ShowModal()
{
    WX_HOOK_MODAL_DIALOG();

    CreateExtraControl();

    return wxDialog::ShowModal();
}

void wxFileDialog::DoSetSize(int WXUNUSED(x), int WXUNUSED(y),
                             int WXUNUSED(width), int WXUNUSED(height),
                             int WXUNUSED(sizeFlags))
{
}

void wxFileDialog::OnSize(wxSizeEvent&)
{
        }

wxString wxFileDialog::GetPath() const
{
    return m_fc.GetPath();
}

void wxFileDialog::GetFilenames(wxArrayString& files) const
{
    m_fc.GetFilenames( files );
}

void wxFileDialog::GetPaths(wxArrayString& paths) const
{
    m_fc.GetPaths( paths );
}

void wxFileDialog::SetMessage(const wxString& message)
{
    m_message = message;
    SetTitle(message);
}

void wxFileDialog::SetPath(const wxString& path)
{
    wxFileDialogBase::SetPath(path);

                if ( path.empty() )
        return;

                wxFileName fn(path);
    fn.MakeAbsolute(m_dir);
    m_fc.SetPath(fn.GetFullPath());
}

void wxFileDialog::SetDirectory(const wxString& dir)
{
    wxFileDialogBase::SetDirectory(dir);

    m_fc.SetDirectory(dir);
}

void wxFileDialog::SetFilename(const wxString& name)
{
    wxFileDialogBase::SetFilename(name);

    if (HasFdFlag(wxFD_SAVE))
    {
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(m_widget), wxGTK_CONV(name));
    }

    else
    {
        wxString path( GetDirectory() );
        if (path.empty())
        {
                        return;
        }
        SetPath(wxFileName(path, name).GetFullPath());
    }
}

wxString wxFileDialog::GetFilename() const
{
    wxString currentFilename( m_fc.GetFilename() );
    if (currentFilename.empty())
    {
                        currentFilename = m_fileName;
    }
    return currentFilename;
}

void wxFileDialog::SetWildcard(const wxString& wildCard)
{
    wxFileDialogBase::SetWildcard(wildCard);
    m_fc.SetWildcard( GetWildcard() );
}

void wxFileDialog::SetFilterIndex(int filterIndex)
{
    m_fc.SetFilterIndex( filterIndex);
}

int wxFileDialog::GetFilterIndex() const
{
    return m_fc.GetFilterIndex();
}

void wxFileDialog::GTKSelectionChanged(const wxString& filename)
{
    m_currentlySelectedFilename = filename;

    if (m_extraControl)
        m_extraControl->UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

#endif 