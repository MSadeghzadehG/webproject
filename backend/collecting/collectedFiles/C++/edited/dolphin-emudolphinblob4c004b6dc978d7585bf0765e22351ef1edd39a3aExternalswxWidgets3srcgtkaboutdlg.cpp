


#include "wx/wxprec.h"

#if wxUSE_ABOUTDLG

#include "wx/aboutdlg.h"

#ifndef WX_PRECOMP
    #include "wx/window.h"
#endif 
#include <gtk/gtk.h>
#include "wx/gtk/private.h"
#include "wx/gtk/private/gtk2-compat.h"


namespace
{

class GtkArray
{
public:
        GtkArray() : m_strings(0), m_count(0)
    {
    }

            GtkArray(const wxArrayString& a)
    {
        m_count = a.size();
        m_strings = new const gchar *[m_count + 1];

        for ( size_t n = 0; n < m_count; n++ )
        {
#if wxUSE_UNICODE
                                                                        m_strings[n] = wxGTK_CONV_SYS(a[n]);
#else                                                 m_strings[n] = wxGTK_CONV_SYS(a[n]).release();
#endif         }

                m_strings[m_count] = NULL;
    }

    operator const gchar **() const { return m_strings; }

    ~GtkArray()
    {
#if !wxUSE_UNICODE
        for ( size_t n = 0; n < m_count; n++ )
            free(const_cast<gchar *>(m_strings[n]));
#endif

        delete [] m_strings;
    }

private:
    const gchar **m_strings;
    size_t m_count;

    wxDECLARE_NO_COPY_CLASS(GtkArray);
};

} 

static GtkAboutDialog *gs_aboutDialog = NULL;

extern "C" {
static void wxGtkAboutDialogOnClose(GtkAboutDialog *about)
{
    gtk_widget_destroy(GTK_WIDGET(about));
    if ( about == gs_aboutDialog )
        gs_aboutDialog = NULL;
}
}

#ifdef __WXGTK3__
extern "C" {
static gboolean activate_link(GtkAboutDialog*, const char* link, void* dontIgnore)
{
    if (dontIgnore)
    {
        wxLaunchDefaultBrowser(wxGTK_CONV_BACK_SYS(link));
        return true;
    }
    return false;
}
}
#else
extern "C" {
static void wxGtkAboutDialogOnLink(GtkAboutDialog*, const char* link, void*)
{
    wxLaunchDefaultBrowser(wxGTK_CONV_BACK_SYS(link));
}
}
#endif

void wxAboutBox(const wxAboutDialogInfo& info, wxWindow* parent)
{
        if ( !gs_aboutDialog )
        gs_aboutDialog = GTK_ABOUT_DIALOG(gtk_about_dialog_new());

    GtkAboutDialog * const dlg = gs_aboutDialog;
    gtk_about_dialog_set_program_name(dlg, wxGTK_CONV_SYS(info.GetName()));
    if ( info.HasVersion() )
        gtk_about_dialog_set_version(dlg, wxGTK_CONV_SYS(info.GetVersion()));
    else
        gtk_about_dialog_set_version(dlg, NULL);
    if ( info.HasCopyright() )
        gtk_about_dialog_set_copyright(dlg, wxGTK_CONV_SYS(info.GetCopyrightToDisplay()));
    else
        gtk_about_dialog_set_copyright(dlg, NULL);
    if ( info.HasDescription() )
        gtk_about_dialog_set_comments(dlg, wxGTK_CONV_SYS(info.GetDescription()));
    else
        gtk_about_dialog_set_comments(dlg, NULL);
    if ( info.HasLicence() )
        gtk_about_dialog_set_license(dlg, wxGTK_CONV_SYS(info.GetLicence()));
    else
        gtk_about_dialog_set_license(dlg, NULL);

    wxIcon icon = info.GetIcon();
    if ( icon.IsOk() )
        gtk_about_dialog_set_logo(dlg, info.GetIcon().GetPixbuf());

    if ( info.HasWebSite() )
    {
#ifdef __WXGTK3__
        g_signal_connect(dlg, "activate-link", G_CALLBACK(activate_link), dlg);
#else
                                gtk_about_dialog_set_url_hook(wxGtkAboutDialogOnLink, NULL, NULL);
#endif

        gtk_about_dialog_set_website(dlg, wxGTK_CONV_SYS(info.GetWebSiteURL()));
        gtk_about_dialog_set_website_label
        (
            dlg,
            wxGTK_CONV_SYS(info.GetWebSiteDescription())
        );
    }
    else
    {
        gtk_about_dialog_set_website(dlg, NULL);
        gtk_about_dialog_set_website_label(dlg, NULL);
#ifdef __WXGTK3__
        g_signal_connect(dlg, "activate-link", G_CALLBACK(activate_link), NULL);
#else
        gtk_about_dialog_set_url_hook(NULL, NULL, NULL);
#endif
    }

    if ( info.HasDevelopers() )
        gtk_about_dialog_set_authors(dlg, GtkArray(info.GetDevelopers()));
    else
        gtk_about_dialog_set_authors(dlg, GtkArray());
    if ( info.HasDocWriters() )
        gtk_about_dialog_set_documenters(dlg, GtkArray(info.GetDocWriters()));
    else
        gtk_about_dialog_set_documenters(dlg, GtkArray());
    if ( info.HasArtists() )
        gtk_about_dialog_set_artists(dlg, GtkArray(info.GetArtists()));
    else
        gtk_about_dialog_set_artists(dlg, GtkArray());

    wxString transCredits;
    if ( info.HasTranslators() )
    {
        const wxArrayString& translators = info.GetTranslators();
        const size_t count = translators.size();
        for ( size_t n = 0; n < count; n++ )
        {
            transCredits << translators[n] << wxT('\n');
        }
    }
    else     {
                wxString translator = _("translator-credits");

                                                        if ( translator != wxT("translator-credits") )             transCredits = translator;
    }

    if ( !transCredits.empty() )
        gtk_about_dialog_set_translator_credits(dlg, wxGTK_CONV_SYS(transCredits));
    else
        gtk_about_dialog_set_translator_credits(dlg, NULL);

    g_signal_connect(dlg, "response",
                        G_CALLBACK(wxGtkAboutDialogOnClose), NULL);

    GtkWindow* gtkParent = NULL;
    if (parent && parent->m_widget)
        gtkParent = (GtkWindow*)gtk_widget_get_ancestor(parent->m_widget, GTK_TYPE_WINDOW);
    gtk_window_set_transient_for(GTK_WINDOW(dlg), gtkParent);

    gtk_window_present(GTK_WINDOW(dlg));
}

#endif 