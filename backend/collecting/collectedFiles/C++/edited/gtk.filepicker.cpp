


#include "wx/wxprec.h"

#if wxUSE_FILEPICKERCTRL

#ifndef WX_PRECOMP
    #include "wx/log.h"
#endif

#include "wx/filepicker.h"
#include "wx/tooltip.h"

#include <gtk/gtk.h>
#include "wx/gtk/private.h"



wxIMPLEMENT_DYNAMIC_CLASS(wxFileButton, wxButton);

bool wxFileButton::Create( wxWindow *parent, wxWindowID id,
                        const wxString &label, const wxString &path,
                        const wxString &message, const wxString &wildcard,
                        const wxPoint &pos, const wxSize &size,
                        long style, const wxValidator& validator,
                        const wxString &name )
{
            if (!(style & wxFLP_SAVE) && !(style & wxFLP_USE_TEXTCTRL))
    {
                
        if (!PreCreation( parent, pos, size ) ||
            !wxControl::CreateBase(parent, id, pos, size, style & wxWINDOW_STYLE_MASK,
                                    validator, name))
        {
            wxFAIL_MSG( wxT("wxFileButton creation failed") );
            return false;
        }

                                SetWindowStyle(style);
        m_path = path;
        m_message = message;
        m_wildcard = wildcard;
        if ((m_dialog = CreateDialog()) == NULL)
            return false;

                                                                                g_signal_connect(m_dialog->m_widget, "show", G_CALLBACK(gtk_grab_add), NULL);
        g_signal_connect(m_dialog->m_widget, "hide", G_CALLBACK(gtk_grab_remove), NULL);

                m_widget = gtk_file_chooser_button_new_with_dialog( m_dialog->m_widget );
        g_object_ref(m_widget);

                                m_dialog->Connect(wxEVT_BUTTON,
                wxCommandEventHandler(wxFileButton::OnDialogOK),
                NULL, this);

        m_parent->DoAddChild( this );

        PostCreation(size);
        SetInitialSize(size);
    }
    else
        return wxGenericFileButton::Create(parent, id, label, path, message, wildcard,
                                           pos, size, style, validator, name);
    return true;
}

wxFileButton::~wxFileButton()
{
    if ( m_dialog )
    {
                                                        gtk_widget_destroy(m_widget);
        delete m_dialog;
    }
}

void wxFileButton::OnDialogOK(wxCommandEvent& ev)
{
            if (ev.GetId() == wxID_OK)
    {
                UpdatePathFromDialog(m_dialog);

                wxFileDirPickerEvent event(wxEVT_FILEPICKER_CHANGED, this, GetId(), m_path);
        HandleWindowEvent(event);
    }
}

void wxFileButton::SetPath(const wxString &str)
{
    m_path = str;

    if (m_dialog)
        UpdateDialogPath(m_dialog);
}

void wxFileButton::SetInitialDirectory(const wxString& dir)
{
    if (m_dialog)
    {
                        if ( m_path.find_first_of(wxFileName::GetPathSeparators()) ==
                wxString::npos )
        {
            static_cast<wxFileDialog*>(m_dialog)->SetDirectory(dir);
        }
    }
    else
        wxGenericFileButton::SetInitialDirectory(dir);
}

#endif 
#if wxUSE_DIRPICKERCTRL

#ifdef __UNIX__
#include <unistd.h> #endif


extern "C" {
static void file_set(GtkFileChooser* widget, wxDirButton* p)
{
            wxGtkString filename(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget)));
    p->GTKUpdatePath(filename);

                        if (p->HasFlag(wxDIRP_CHANGE_DIR))
    {
        if ( chdir(filename) != 0 )
        {
            wxLogSysError(_("Changing current directory to \"%s\" failed"),
                          wxString::FromUTF8(filename));
        }
    }

        wxFileDirPickerEvent event(wxEVT_DIRPICKER_CHANGED, p, p->GetId(), p->GetPath());
    p->HandleWindowEvent(event);
}
}


extern "C" {
static void selection_changed(GtkFileChooser* chooser, wxDirButton* win)
{
    char* filename = gtk_file_chooser_get_filename(chooser);

    if (wxString::FromUTF8(filename) == win->GetPath())
        win->m_bIgnoreNextChange = false;
    else if (!win->m_bIgnoreNextChange)
        file_set(chooser, win);

    g_free(filename);
}
}


wxIMPLEMENT_DYNAMIC_CLASS(wxDirButton, wxButton);

bool wxDirButton::Create( wxWindow *parent, wxWindowID id,
                        const wxString &label, const wxString &path,
                        const wxString &message, const wxString &wildcard,
                        const wxPoint &pos, const wxSize &size,
                        long style, const wxValidator& validator,
                        const wxString &name )
{
    if (!(style & wxDIRP_USE_TEXTCTRL))
    {
                
        if (!PreCreation( parent, pos, size ) ||
            !wxControl::CreateBase(parent, id, pos, size, style & wxWINDOW_STYLE_MASK,
                                    validator, name))
        {
            wxFAIL_MSG( wxT("wxDirButtonGTK creation failed") );
            return false;
        }

                SetWindowStyle(style);
        m_message = message;
        m_wildcard = wildcard;
        if ((m_dialog = CreateDialog()) == NULL)
            return false;

                                                                                g_signal_connect(m_dialog->m_widget, "show", G_CALLBACK(gtk_grab_add), NULL);
        g_signal_connect(m_dialog->m_widget, "hide", G_CALLBACK(gtk_grab_remove), NULL);


                        m_widget = gtk_file_chooser_button_new_with_dialog( m_dialog->m_widget );
        g_object_ref(m_widget);
        SetPath(path);

#ifdef __WXGTK3__
        if (gtk_check_version(3,8,0) == NULL)
            g_signal_connect(m_widget, "file_set", G_CALLBACK(file_set), this);
        else
#endif
        {
                                    g_signal_connect(m_widget, "selection_changed",
                G_CALLBACK(selection_changed), this);
        }

        m_parent->DoAddChild( this );

        PostCreation(size);
        SetInitialSize(size);
    }
    else
        return wxGenericDirButton::Create(parent, id, label, path, message, wildcard,
                                          pos, size, style, validator, name);
    return true;
}

wxDirButton::~wxDirButton()
{
    if (m_dialog)
    {
                gtk_widget_destroy(m_widget);
        delete m_dialog;
    }
}

void wxDirButton::GTKUpdatePath(const char *gtkpath)
{
    m_path = wxString::FromUTF8(gtkpath);
}
void wxDirButton::SetPath(const wxString& str)
{
    if ( m_path == str )
        return;

    m_path = str;

    m_bIgnoreNextChange = true;

    if (GTK_IS_FILE_CHOOSER(m_widget))
        gtk_file_chooser_set_filename((GtkFileChooser*)m_widget, str.utf8_str());
    else if (m_dialog)
        UpdateDialogPath(m_dialog);
}

void wxDirButton::SetInitialDirectory(const wxString& dir)
{
    if (m_dialog)
    {
        if (m_path.empty())
            static_cast<wxDirDialog*>(m_dialog)->SetPath(dir);
    }
    else
        wxGenericDirButton::SetInitialDirectory(dir);
}

#endif 