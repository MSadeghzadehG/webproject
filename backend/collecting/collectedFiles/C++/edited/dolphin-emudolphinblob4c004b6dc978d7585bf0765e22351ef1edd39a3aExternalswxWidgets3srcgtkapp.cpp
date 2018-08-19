
#include "wx/wxprec.h"

#include "wx/app.h"

#ifndef WX_PRECOMP
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/utils.h"
    #include "wx/memory.h"
    #include "wx/font.h"
#endif

#include "wx/thread.h"

#ifdef __WXGPE__
    #include <gpe/init.h>
#endif

#include "wx/apptrait.h"
#include "wx/fontmap.h"

#include <gtk/gtk.h>
#include "wx/gtk/private.h"


#if wxUSE_MIMETYPE && wxUSE_LIBGNOMEVFS
    #include "wx/link.h"
    wxFORCE_LINK_MODULE(gnome_vfs)
#endif


extern "C" {
static gboolean
wx_emission_hook(GSignalInvocationHint*, guint, const GValue*, gpointer data)
{
    wxApp* app = wxTheApp;
    if (app != NULL)
        app->WakeUpIdle();
    bool* hook_installed = (bool*)data;
        *hook_installed = false;
        return false;
}
}

static void wx_add_idle_hooks()
{
        {
        static bool hook_installed;
        if (!hook_installed)
        {
            static guint sig_id;
            if (sig_id == 0)
                sig_id = g_signal_lookup("event", GTK_TYPE_WIDGET);
            hook_installed = true;
            g_signal_add_emission_hook(
                sig_id, 0, wx_emission_hook, &hook_installed, NULL);
        }
    }
                {
        static bool hook_installed;
        if (!hook_installed)
        {
            static guint sig_id;
            if (sig_id == 0)
                sig_id = g_signal_lookup("size_allocate", GTK_TYPE_WIDGET);
            hook_installed = true;
            g_signal_add_emission_hook(
                sig_id, 0, wx_emission_hook, &hook_installed, NULL);
        }
    }
}

extern "C" {
static gboolean wxapp_idle_callback(gpointer)
{
    return wxTheApp->DoIdle();
}
}

static int gs_focusChange;

extern "C" {
static gboolean
wx_focus_event_hook(GSignalInvocationHint*, unsigned, const GValue* param_values, void* data)
{
        if (GTK_IS_WINDOW(g_value_peek_pointer(param_values)))
        gs_focusChange = GPOINTER_TO_INT(data);

    return true;
}
}

bool wxApp::DoIdle()
{
    guint id_save;
    {
                        #if wxUSE_THREADS
        wxMutexLocker lock(m_idleMutex);
#endif
        id_save = m_idleSourceId;
        m_idleSourceId = 0;
        wx_add_idle_hooks();

#if wxDEBUG_LEVEL
                        if (m_isInAssert)
            return false;
#endif
    }

    gdk_threads_enter();

    if (gs_focusChange) {
        SetActive(gs_focusChange == 1, NULL);
        gs_focusChange = 0;
    }

    bool needMore;
    do {
        ProcessPendingEvents();

        needMore = ProcessIdle();
    } while (needMore && gtk_events_pending() == 0);
    gdk_threads_leave();

#if wxUSE_THREADS
    wxMutexLocker lock(m_idleMutex);
#endif

    bool keepSource = false;
            if (m_idleSourceId == 0)
    {
                if (needMore || HasPendingEvents())
        {
                        m_idleSourceId = id_save;
            keepSource = true;
        }
        else             wx_add_idle_hooks();
    }
        
    return keepSource;
}


wxIMPLEMENT_DYNAMIC_CLASS(wxApp,wxEvtHandler);

wxApp::wxApp()
{
    m_isInAssert = false;
    m_idleSourceId = 0;
}

wxApp::~wxApp()
{
}

bool wxApp::SetNativeTheme(const wxString& theme)
{
#ifdef __WXGTK3__
    wxUnusedVar(theme);
    return false;
#else
    wxString path;
    path = gtk_rc_get_theme_dir();
    path += "/";
    path += theme.utf8_str();
    path += "/gtk-2.0/gtkrc";

    if ( wxFileExists(path.utf8_str()) )
        gtk_rc_add_default_file(path.utf8_str());
    else if ( wxFileExists(theme.utf8_str()) )
        gtk_rc_add_default_file(theme.utf8_str());
    else
    {
        wxLogWarning("Theme \"%s\" not available.", theme);

        return false;
    }

    gtk_rc_reparse_all_for_settings(gtk_settings_get_default(), TRUE);

    return true;
#endif
}

bool wxApp::OnInitGui()
{
    if ( !wxAppBase::OnInitGui() )
        return false;

#ifndef __WXGTK3__
            if ( GetXVisualInfo() )
    {
        GdkVisual* vis = gtk_widget_get_default_visual();

        GdkColormap *colormap = gdk_colormap_new( vis, FALSE );
        gtk_widget_set_default_colormap( colormap );
    }
    else
    {
                        if (m_useBestVisual)
        {
            if (m_forceTrueColour)
            {
                GdkVisual* visual = gdk_visual_get_best_with_both( 24, GDK_VISUAL_TRUE_COLOR );
                if (!visual)
                {
                    wxLogError(wxT("Unable to initialize TrueColor visual."));
                    return false;
                }
                GdkColormap *colormap = gdk_colormap_new( visual, FALSE );
                gtk_widget_set_default_colormap( colormap );
            }
            else
            {
                if (gdk_visual_get_best() != gdk_visual_get_system())
                {
                    GdkVisual* visual = gdk_visual_get_best();
                    GdkColormap *colormap = gdk_colormap_new( visual, FALSE );
                    gtk_widget_set_default_colormap( colormap );
                }
            }
        }
    }
#endif

    return true;
}

bool wxApp::Initialize(int& argc_, wxChar **argv_)
{
    if ( !wxAppBase::Initialize(argc_, argv_) )
        return false;

#if wxUSE_THREADS
    if (!g_thread_supported())
    {
                                        wxGCC_WARNING_SUPPRESS(deprecated-declarations)
        g_thread_init(NULL);
        wxGCC_WARNING_RESTORE()
        gdk_threads_init();
    }
#endif 
        wxConvCurrent = &wxConvUTF8;

#ifdef __UNIX__
    
            wxString encName(wxGetenv(wxT("G_FILENAME_ENCODING")));
    encName = encName.BeforeFirst(wxT(','));
    if (encName.CmpNoCase(wxT("@locale")) == 0)
        encName.clear();
    encName.MakeUpper();
    if (encName.empty())
    {
#if wxUSE_INTL
                        encName = wxLocale::GetSystemEncodingName().Upper();

                if ( !encName.empty() )
        {
#if wxUSE_FONTMAP
            wxFontEncoding enc = wxFontMapperBase::GetEncodingFromName(encName);
            if ( enc == wxFONTENCODING_DEFAULT )
#else             if ( encName == wxT("US-ASCII") )
#endif             {
                                encName.clear();
            }
        }
#endif 
                if ( encName.empty() )
            encName = wxT("UTF-8");
        wxSetEnv(wxT("G_FILENAME_ENCODING"), encName);
    }

    static wxConvBrokenFileNames fileconv(encName);
    wxConvFileName = &fileconv;
#endif 

    bool init_result;
    int i;

#if wxUSE_UNICODE
        char **argvGTK = new char *[argc_ + 1];
    for ( i = 0; i < argc_; i++ )
    {
        argvGTK[i] = wxStrdupA(wxConvUTF8.cWX2MB(argv_[i]));
    }

    argvGTK[argc_] = NULL;

    int argcGTK = argc_;

                gtk_disable_setlocale();

#ifdef __WXGPE__
    init_result = true;      gpe_application_init( &argcGTK, &argvGTK );
#else
    init_result = gtk_init_check( &argcGTK, &argvGTK ) != 0;
#endif

    if ( argcGTK != argc_ )
    {
                for ( i = 0; i < argcGTK; i++ )
        {
            while ( strcmp(wxConvUTF8.cWX2MB(argv_[i]), argvGTK[i]) != 0 )
            {
                memmove(argv_ + i, argv_ + i + 1, (argc_ - i)*sizeof(*argv_));
            }
        }

        argc_ = argcGTK;
        argv_[argc_] = NULL;
    }
    
        for ( i = 0; i < argcGTK; i++ )
    {
        free(argvGTK[i]);
    }

    delete [] argvGTK;
#else             init_result = gtk_init_check( &argc_, &argv_ );
#endif 
        this->argc = argc_;
    this->argv = argv_;

    if ( m_traits )
    {
                                        wxArrayString opt, desc;
        m_traits->GetStandardCmdLineOptions(opt, desc);

        for ( i = 0; i < argc_; i++ )
        {
                        const wxString str = wxString(argv_[i]).BeforeFirst('=');

            for ( size_t j = 0; j < opt.size(); j++ )
            {
                                                if ( opt[j].Trim(false).BeforeFirst('=') == str )
                {
                                                                                                    wxLogError(_("Invalid GTK+ command line option, use \"%s --help\""),
                               argv_[0]);
                    return false;
                }
            }
        }
    }

    if ( !init_result )
    {
        wxLogError(_("Unable to initialize GTK+, is DISPLAY set properly?"));
        return false;
    }

        gdk_threads_enter();

#if wxUSE_INTL
    wxFont::SetDefaultEncoding(wxLocale::GetSystemEncoding());
#endif

        const GType widgetType = GTK_TYPE_WIDGET;
    g_type_class_ref(widgetType);

        g_signal_add_emission_hook(
        g_signal_lookup("focus_in_event", widgetType),
        0, wx_focus_event_hook, GINT_TO_POINTER(1), NULL);
    g_signal_add_emission_hook(
        g_signal_lookup("focus_out_event", widgetType),
        0, wx_focus_event_hook, GINT_TO_POINTER(2), NULL);

    WakeUpIdle();

    return true;
}

void wxApp::CleanUp()
{
    if (m_idleSourceId != 0)
        g_source_remove(m_idleSourceId);

        gpointer gt = g_type_class_peek(GTK_TYPE_WIDGET);
    if (gt != NULL)
        g_type_class_unref(gt);

    gdk_threads_leave();

    wxAppBase::CleanUp();
}

void wxApp::WakeUpIdle()
{
#if wxUSE_THREADS
    wxMutexLocker lock(m_idleMutex);
#endif
    if (m_idleSourceId == 0)
        m_idleSourceId = g_idle_add_full(G_PRIORITY_LOW, wxapp_idle_callback, NULL, NULL);
}

bool wxApp::EventsPending()
{
#if wxUSE_THREADS
    wxMutexLocker lock(m_idleMutex);
#endif
    if (m_idleSourceId != 0)
    {
        g_source_remove(m_idleSourceId);
        m_idleSourceId = 0;
        wx_add_idle_hooks();
    }
    return gtk_events_pending() != 0;
}

void wxApp::OnAssertFailure(const wxChar *file,
                            int line,
                            const wxChar* func,
                            const wxChar* cond,
                            const wxChar *msg)
{
        #if wxDEBUG_LEVEL
        m_isInAssert = true;

    wxAppBase::OnAssertFailure(file, line, func, cond, msg);

    m_isInAssert = false;
#else     wxUnusedVar(file);
    wxUnusedVar(line);
    wxUnusedVar(func);
    wxUnusedVar(cond);
    wxUnusedVar(msg);
#endif }

#if wxUSE_THREADS
void wxGUIAppTraits::MutexGuiEnter()
{
    gdk_threads_enter();
}

void wxGUIAppTraits::MutexGuiLeave()
{
    gdk_threads_leave();
}
#endif 

bool wxApp::GTKIsUsingGlobalMenu()
{
    static int s_isUsingGlobalMenu = -1;
    if ( s_isUsingGlobalMenu == -1 )
    {
                                                wxString proxy;
        s_isUsingGlobalMenu = wxGetEnv("UBUNTU_MENUPROXY", &proxy) &&
                                !proxy.empty() && proxy != "0";
    }

    return s_isUsingGlobalMenu == 1;
}
