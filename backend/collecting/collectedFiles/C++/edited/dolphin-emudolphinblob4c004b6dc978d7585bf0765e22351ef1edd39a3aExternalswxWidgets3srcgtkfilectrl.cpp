
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#if wxUSE_FILECTRL && !defined(__WXUNIVERSAL__)

#include "wx/filectrl.h"

#include "wx/gtk/private.h"
#include "wx/filename.h"
#include "wx/scopeguard.h"
#include "wx/tokenzr.h"


void wxGtkFileChooser::SetWidget(GtkFileChooser *w)
{
        wxASSERT( w );
    wxASSERT( GTK_FILE_CHOOSER( w ) );

    this->m_widget = w;
}

wxString wxGtkFileChooser::GetPath() const
{
    wxGtkString str( gtk_file_chooser_get_filename( m_widget ) );

    wxString string;
    if (str)
        string = wxString::FromUTF8(str);
    return string;
}

void wxGtkFileChooser::GetFilenames( wxArrayString& files ) const
{
    GetPaths( files );
    for ( size_t n = 0; n < files.GetCount(); ++n )
    {
        const wxFileName file( files[n] );
        files[n] = file.GetFullName();
    }
}

void wxGtkFileChooser::GetPaths( wxArrayString& paths ) const
{
    paths.Empty();
    if ( gtk_file_chooser_get_select_multiple( m_widget ) )
    {
        GSList *gpathsi = gtk_file_chooser_get_filenames( m_widget );
        GSList *gpaths = gpathsi;
        while ( gpathsi )
        {
            wxString file(wxString::FromUTF8(static_cast<gchar *>(gpathsi->data)));
            paths.Add( file );
            g_free( gpathsi->data );
            gpathsi = gpathsi->next;
        }

        g_slist_free( gpaths );
    }
    else
        paths.Add( GetPath() );
}

bool wxGtkFileChooser::SetPath( const wxString& path )
{
    if ( path.empty() )
        return true;

    switch ( gtk_file_chooser_get_action( m_widget ) )
    {
        case GTK_FILE_CHOOSER_ACTION_SAVE:
            {
                wxFileName fn(path);

                const wxString fname = fn.GetFullName();
                gtk_file_chooser_set_current_name( m_widget, fname.utf8_str() );

                                const wxString dir = fn.GetPath();
                return gtk_file_chooser_set_current_folder( m_widget,
                                                            dir.utf8_str() ) != 0;
            }

        case GTK_FILE_CHOOSER_ACTION_OPEN:
            return gtk_file_chooser_set_filename( m_widget, path.utf8_str() ) != 0;

        case GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER:
        case GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER:
            break;
    }

    wxFAIL_MSG( "Unexpected file chooser type" );

    return false;
}

bool wxGtkFileChooser::SetDirectory( const wxString& dir )
{
    return gtk_file_chooser_set_current_folder( m_widget, dir.utf8_str() ) != 0;
}

wxString wxGtkFileChooser::GetDirectory() const
{
    const wxGtkString str( gtk_file_chooser_get_current_folder( m_widget ) );
    return wxString::FromUTF8(str);
}

wxString wxGtkFileChooser::GetFilename() const
{
    return wxFileName( GetPath() ).GetFullName();
}

void wxGtkFileChooser::SetWildcard( const wxString& wildCard )
{
    m_wildcards.Empty();

        wxArrayString wildDescriptions, wildFilters;

    if ( !wxParseCommonDialogsFilter( wildCard, wildDescriptions, wildFilters ) )
    {
        wxFAIL_MSG( wxT( "wxGtkFileChooser::SetWildcard - bad wildcard string" ) );
    }
    else
    {
                GtkFileChooser* chooser = m_widget;

                GSList* ifilters = gtk_file_chooser_list_filters( chooser );
        GSList* filters = ifilters;

        m_ignoreNextFilterEvent = true;
        wxON_BLOCK_EXIT_SET(m_ignoreNextFilterEvent, false);

        while ( ifilters )
        {
            gtk_file_chooser_remove_filter( chooser, GTK_FILE_FILTER( ifilters->data ) );
            ifilters = ifilters->next;
        }
        g_slist_free( filters );

        if (!wildCard.empty())
        {
                        for ( size_t n = 0; n < wildFilters.GetCount(); ++n )
            {
                GtkFileFilter* filter = gtk_file_filter_new();

                gtk_file_filter_set_name( filter, wxGTK_CONV_SYS( wildDescriptions[n] ) );

                wxStringTokenizer exttok( wildFilters[n], wxT( ";" ) );

                int n1 = 1;
                while ( exttok.HasMoreTokens() )
                {
                    wxString token = exttok.GetNextToken();
                    gtk_file_filter_add_pattern( filter, wxGTK_CONV_SYS( token ) );

                    if (n1 == 1)
                        m_wildcards.Add( token );                     n1++;
                }

                gtk_file_chooser_add_filter( chooser, filter );
            }

                        SetFilterIndex( 0 );
        }
    }
}

void wxGtkFileChooser::SetFilterIndex( int filterIndex )
{
    gpointer filter;
    GtkFileChooser *chooser = m_widget;
    GSList *filters = gtk_file_chooser_list_filters( chooser );

    filter = g_slist_nth_data( filters, filterIndex );

    if ( filter != NULL )
    {
        gtk_file_chooser_set_filter( chooser, GTK_FILE_FILTER( filter ) );
    }
    else
    {
        wxFAIL_MSG( wxT( "wxGtkFileChooser::SetFilterIndex - bad filter index" ) );
    }

    g_slist_free( filters );
}

int wxGtkFileChooser::GetFilterIndex() const
{
    GtkFileChooser *chooser = m_widget;
    GtkFileFilter *filter = gtk_file_chooser_get_filter( chooser );
    GSList *filters = gtk_file_chooser_list_filters( chooser );
    const gint index = g_slist_index( filters, filter );
    g_slist_free( filters );

    if ( index == -1 )
    {
        wxFAIL_MSG( wxT( "wxGtkFileChooser::GetFilterIndex - bad filter index returned by gtk+" ) );
        return 0;
    }
    else
        return index;
}

bool wxGtkFileChooser::HasFilterChoice() const
{
    return gtk_file_chooser_get_filter( m_widget ) != NULL;
}


#if wxUSE_FILECTRL


extern "C"
{
    static void
    gtkfilechooserwidget_file_activated_callback( GtkWidget *WXUNUSED( widget ), wxGtkFileCtrl *fileCtrl )
    {
        GenerateFileActivatedEvent( fileCtrl, fileCtrl );
    }
}

extern "C"
{
    static void
    gtkfilechooserwidget_selection_changed_callback( GtkWidget *WXUNUSED( widget ), wxGtkFileCtrl *fileCtrl )
    {
                        if ( fileCtrl->m_checkNextSelEvent )
        {
            wxArrayString filenames;
            fileCtrl->GetFilenames( filenames );

            if ( filenames.Count() != 0 )
                fileCtrl->m_checkNextSelEvent = false;
        }

        if ( !fileCtrl->m_checkNextSelEvent )
            GenerateSelectionChangedEvent( fileCtrl, fileCtrl );
    }
}

extern "C"
{
    static void
    gtkfilechooserwidget_folder_changed_callback( GtkWidget *WXUNUSED( widget ), wxGtkFileCtrl *fileCtrl )
    {
        if ( fileCtrl->m_ignoreNextFolderChangeEvent )
        {
            fileCtrl->m_ignoreNextFolderChangeEvent = false;
        }
        else
        {
            GenerateFolderChangedEvent( fileCtrl, fileCtrl );
        }

        fileCtrl->m_checkNextSelEvent = true;
    }
}

extern "C"
{
    static void
    gtkfilechooserwidget_notify_callback( GObject *WXUNUSED( gobject ), GParamSpec *arg1, wxGtkFileCtrl *fileCtrl )
    {
        const char *name = g_param_spec_get_name (arg1);
        if ( strcmp( name, "filter" ) == 0 &&
             fileCtrl->HasFilterChoice() &&
             !fileCtrl->GTKShouldIgnoreNextFilterEvent() )
        {
            GenerateFilterChangedEvent( fileCtrl, fileCtrl );
        }
    }
}


wxIMPLEMENT_DYNAMIC_CLASS(wxGtkFileCtrl, wxControl);

wxGtkFileCtrl::~wxGtkFileCtrl()
{
    if (m_fcWidget)
        GTKDisconnect(m_fcWidget);
}

void wxGtkFileCtrl::Init()
{
    m_checkNextSelEvent = false;

        m_ignoreNextFolderChangeEvent = true;
}

bool wxGtkFileCtrl::Create( wxWindow *parent,
                            wxWindowID id,
                            const wxString& defaultDirectory,
                            const wxString& defaultFileName,
                            const wxString& wildCard,
                            long style,
                            const wxPoint& pos,
                            const wxSize& size,
                            const wxString& name )
{
    if ( !PreCreation( parent, pos, size ) ||
            !CreateBase( parent, id, pos, size, style, wxDefaultValidator, name ) )
    {
        wxFAIL_MSG( wxT( "wxGtkFileCtrl creation failed" ) );
        return false;
    }

    GtkFileChooserAction gtkAction = GTK_FILE_CHOOSER_ACTION_OPEN;

    if ( style & wxFC_SAVE )
        gtkAction = GTK_FILE_CHOOSER_ACTION_SAVE;

    m_widget =  gtk_alignment_new ( 0, 0, 1, 1 );
    g_object_ref(m_widget);
    m_fcWidget = GTK_FILE_CHOOSER( gtk_file_chooser_widget_new(gtkAction) );
    gtk_widget_show ( GTK_WIDGET( m_fcWidget ) );
    gtk_container_add ( GTK_CONTAINER ( m_widget ), GTK_WIDGET( m_fcWidget ) );

    m_focusWidget = GTK_WIDGET( m_fcWidget );

    g_signal_connect ( m_fcWidget, "file-activated",
                       G_CALLBACK ( gtkfilechooserwidget_file_activated_callback ),
                       this );

    g_signal_connect ( m_fcWidget, "current-folder-changed",
                       G_CALLBACK ( gtkfilechooserwidget_folder_changed_callback ),
                       this );

    g_signal_connect ( m_fcWidget, "selection-changed",
                       G_CALLBACK ( gtkfilechooserwidget_selection_changed_callback ),
                       this );

    g_signal_connect ( m_fcWidget, "notify",
                       G_CALLBACK ( gtkfilechooserwidget_notify_callback ),
                       this );

    m_fc.SetWidget( m_fcWidget );

    if ( style & wxFC_MULTIPLE )
        gtk_file_chooser_set_select_multiple( m_fcWidget, true );

    SetWildcard( wildCard );

                wxFileName fn;
    if ( defaultDirectory.empty() )
        fn.Assign( defaultFileName );
    else if ( !defaultFileName.empty() )
        fn.Assign( defaultDirectory, defaultFileName );
    else
        fn.AssignDir( defaultDirectory );

        const wxString dir = fn.GetPath();
    if ( !dir.empty() )
    {
        gtk_file_chooser_set_current_folder( m_fcWidget,
                                             wxGTK_CONV_FN(dir) );
    }

    const wxString fname = fn.GetFullName();
    if ( style & wxFC_SAVE )
    {
        if ( !fname.empty() )
        {
            gtk_file_chooser_set_current_name( m_fcWidget,
                                               wxGTK_CONV_FN(fname) );
        }
    }
    else     {
        if ( !fname.empty() )
        {
            gtk_file_chooser_set_filename( m_fcWidget,
                                           wxGTK_CONV_FN(fn.GetFullPath()) );
        }
    }

    m_parent->DoAddChild( this );

    PostCreation( size );

    return TRUE;
}

bool wxGtkFileCtrl::SetPath( const wxString& path )
{
    return m_fc.SetPath( path );
}

bool wxGtkFileCtrl::SetDirectory( const wxString& dir )
{
    return m_fc.SetDirectory( dir );
}

bool wxGtkFileCtrl::SetFilename( const wxString& name )
{
    if ( HasFlag( wxFC_SAVE ) )
    {
        gtk_file_chooser_set_current_name( m_fcWidget, wxGTK_CONV( name ) );
        return true;
    }
    else
        return SetPath( wxFileName( GetDirectory(), name ).GetFullPath() );
}

void wxGtkFileCtrl::SetWildcard( const wxString& wildCard )
{
    m_wildCard = wildCard;

    m_fc.SetWildcard( wildCard );
}

void wxGtkFileCtrl::SetFilterIndex( int filterIndex )
{
    m_fc.SetFilterIndex( filterIndex );
}

wxString wxGtkFileCtrl::GetPath() const
{
    return m_fc.GetPath();
}

void wxGtkFileCtrl::GetPaths( wxArrayString& paths ) const
{
    m_fc.GetPaths( paths );
}

wxString wxGtkFileCtrl::GetDirectory() const
{
    return m_fc.GetDirectory();
}

wxString wxGtkFileCtrl::GetFilename() const
{
    return m_fc.GetFilename();
}

void wxGtkFileCtrl::GetFilenames( wxArrayString& files ) const
{
    m_fc.GetFilenames( files );
}

void wxGtkFileCtrl::ShowHidden(bool show)
{
    gtk_file_chooser_set_show_hidden(m_fcWidget, show);
}

#endif 
#endif 