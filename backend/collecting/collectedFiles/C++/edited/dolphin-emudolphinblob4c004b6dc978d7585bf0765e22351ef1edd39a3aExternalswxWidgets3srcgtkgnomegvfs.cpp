
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_MIMETYPE && wxUSE_LIBGNOMEVFS

#include "wx/gtk/gnome/gvfs.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/module.h"
#endif

#include "wx/mimetype.h"
#include "wx/dynlib.h"

#include <libgnomevfs/gnome-vfs-mime-handlers.h>

#include "wx/link.h"
wxFORCE_LINK_THIS_MODULE(gnome_vfs)


class wxGnomeVFSLibrary
{
public:
    wxGnomeVFSLibrary();
    ~wxGnomeVFSLibrary();
    bool IsOk();

private:
    bool InitializeMethods();

    wxDynamicLibrary m_libGnomeVFS;

                bool m_ok;

public:
    wxDL_METHOD_DEFINE( gboolean, gnome_vfs_init,
        (), (), FALSE )
    wxDL_VOIDMETHOD_DEFINE( gnome_vfs_shutdown,
        (), () )

    wxDL_METHOD_DEFINE( GnomeVFSResult, gnome_vfs_mime_set_icon,
        (const char *mime_type, const char *filename), (mime_type, filename), GNOME_VFS_OK )
};

wxGnomeVFSLibrary::wxGnomeVFSLibrary()
{
    wxLogNull log;

    m_libGnomeVFS.Load("libgnomevfs-2.so.0");
    m_ok = m_libGnomeVFS.IsLoaded() && InitializeMethods();
}

wxGnomeVFSLibrary::~wxGnomeVFSLibrary()
{
                        m_libGnomeVFS.Detach();
}

bool wxGnomeVFSLibrary::IsOk()
{
    return m_ok;
}

bool wxGnomeVFSLibrary::InitializeMethods()
{
    wxDL_METHOD_LOAD( m_libGnomeVFS, gnome_vfs_init );
    wxDL_METHOD_LOAD( m_libGnomeVFS, gnome_vfs_shutdown );

    return true;
}

static wxGnomeVFSLibrary* gs_lgvfs = NULL;


wxMimeTypesManagerImpl *wxGnomeVFSMimeTypesManagerFactory::CreateMimeTypesManagerImpl()
{
    return new wxGnomeVFSMimeTypesManagerImpl;
}



bool wxGnomeVFSMimeTypesManagerImpl::DoAssociation(const wxString& strType,
                       const wxString& strIcon,
                       wxMimeTypeCommands *entry,
                       const wxArrayString& strExtensions,
                       const wxString& strDesc)
{
    return AddToMimeData
           (
            strType,
            strIcon,
            entry,
            strExtensions,
            strDesc,
            true
           ) != wxNOT_FOUND;
}


class wxGnomeVFSModule: public wxModule
{
public:
    wxGnomeVFSModule() {}
    bool OnInit();
    void OnExit();

private:
    wxDECLARE_DYNAMIC_CLASS(wxGnomeVFSModule);
};

bool wxGnomeVFSModule::OnInit()
{
    gs_lgvfs = new wxGnomeVFSLibrary;
    if (gs_lgvfs->IsOk())
    {
        if (gs_lgvfs->gnome_vfs_init())
            wxMimeTypesManagerFactory::Set( new wxGnomeVFSMimeTypesManagerFactory );
    }
    return true;
}

void wxGnomeVFSModule::OnExit()
{
    if (gs_lgvfs->IsOk())
        gs_lgvfs->gnome_vfs_shutdown();

    delete gs_lgvfs;
}

wxIMPLEMENT_DYNAMIC_CLASS(wxGnomeVFSModule, wxModule);

#endif 