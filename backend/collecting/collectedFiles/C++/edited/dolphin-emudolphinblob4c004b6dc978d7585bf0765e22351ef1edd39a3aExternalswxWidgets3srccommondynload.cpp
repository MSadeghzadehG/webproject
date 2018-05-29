

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_DYNAMIC_LOADER

#ifdef __WINDOWS__
    #include "wx/msw/private.h"
#endif

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/intl.h"
    #include "wx/hash.h"
    #include "wx/utils.h"
    #include "wx/module.h"
#endif

#include "wx/strconv.h"

#include "wx/dynload.h"




wxDLImports*  wxPluginLibrary::ms_classes = NULL;

class wxPluginLibraryModule : public wxModule
{
public:
    wxPluginLibraryModule() { }

        virtual bool OnInit() wxOVERRIDE
    {
        wxPluginLibrary::ms_classes = new wxDLImports;
        wxPluginManager::CreateManifest();
        return true;
    }

    virtual void OnExit() wxOVERRIDE
    {
        wxDELETE(wxPluginLibrary::ms_classes);
        wxPluginManager::ClearManifest();
    }

private:
    wxDECLARE_DYNAMIC_CLASS(wxPluginLibraryModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxPluginLibraryModule, wxModule);


wxPluginLibrary::wxPluginLibrary(const wxString &libname, int flags)
        : m_linkcount(1)
        , m_objcount(0)
{
    const wxClassInfo* const oldFirst = wxClassInfo::GetFirst();
    Load( libname, flags );

                m_ourFirst = wxClassInfo::GetFirst();

                if ( m_ourFirst != oldFirst )
    {
        for ( const wxClassInfo* info = m_ourFirst; ; info = info->GetNext() )
        {
            if ( info->GetNext() == oldFirst )
            {
                m_ourLast = info;
                break;
            }
        }
    }
    else     {
        m_ourFirst =
        m_ourLast = NULL;
    }

    if( m_handle != 0 )
    {
        UpdateClasses();
        RegisterModules();
    }
    else
    {
                --m_linkcount;
    }
}

wxPluginLibrary::~wxPluginLibrary()
{
    if( m_handle != 0 )
    {
        UnregisterModules();
        RestoreClasses();
    }
}

wxPluginLibrary *wxPluginLibrary::RefLib()
{
    wxCHECK_MSG( m_linkcount > 0, NULL,
                 wxT("Library had been already deleted!") );

    ++m_linkcount;
    return this;
}

bool wxPluginLibrary::UnrefLib()
{
    wxASSERT_MSG( m_objcount == 0,
                  wxT("Library unloaded before all objects were destroyed") );

    if ( m_linkcount == 0 || --m_linkcount == 0 )
    {
        delete this;
        return true;
    }

    return false;
}


void wxPluginLibrary::UpdateClasses()
{
    if ( !m_ourFirst )
        return;

    for ( const wxClassInfo *info = m_ourFirst; ; info = info->GetNext() )
    {
        if( info->GetClassName() )
        {
                                    (*ms_classes)[info->GetClassName()] = this;
        }

        if ( info == m_ourLast )
            break;
    }
}

void wxPluginLibrary::RestoreClasses()
{
        if (!ms_classes)
        return;

    if ( !m_ourFirst )
        return;

    for ( const wxClassInfo *info = m_ourFirst; ; info = info->GetNext() )
    {
        ms_classes->erase(ms_classes->find(info->GetClassName()));

        if ( info == m_ourLast )
            break;
    }
}

void wxPluginLibrary::RegisterModules()
{
                            
    wxASSERT_MSG( m_linkcount == 1,
                  wxT("RegisterModules should only be called for the first load") );

    if ( m_ourFirst )
    {
        for ( const wxClassInfo *info = m_ourFirst; ; info = info->GetNext() )
        {
            if( info->IsKindOf(wxCLASSINFO(wxModule)) )
            {
                wxModule *m = wxDynamicCast(info->CreateObject(), wxModule);

                wxASSERT_MSG( m, wxT("wxDynamicCast of wxModule failed") );

                m_wxmodules.push_back(m);
                wxModule::RegisterModule(m);
            }

            if ( info == m_ourLast )
                break;
        }
    }

    
    for ( wxModuleList::iterator it = m_wxmodules.begin();
          it != m_wxmodules.end();
          ++it)
    {
        if( !(*it)->Init() )
        {
            wxLogDebug(wxT("wxModule::Init() failed for wxPluginLibrary"));

                        
                                    
            wxModuleList::iterator oldNode = m_wxmodules.end();
            do {
                ++it;
                if( oldNode != m_wxmodules.end() )
                    m_wxmodules.erase(oldNode);
                wxModule::UnregisterModule( *it );
                oldNode = it;
            } while( it != m_wxmodules.end() );

            --m_linkcount;                 break;
        }
    }
}

void wxPluginLibrary::UnregisterModules()
{
    wxModuleList::iterator it;

    for ( it = m_wxmodules.begin(); it != m_wxmodules.end(); ++it )
        (*it)->Exit();

    for ( it = m_wxmodules.begin(); it != m_wxmodules.end(); ++it )
        wxModule::UnregisterModule( *it );

        m_wxmodules.clear();
}



wxDLManifest*   wxPluginManager::ms_manifest = NULL;


wxPluginLibrary *
wxPluginManager::LoadLibrary(const wxString &libname, int flags)
{
    wxString realname(libname);

    if( !(flags & wxDL_VERBATIM) )
        realname += wxDynamicLibrary::GetDllExt(wxDL_MODULE);

    wxPluginLibrary *entry;

    if ( flags & wxDL_NOSHARE )
    {
        entry = NULL;
    }
    else
    {
        entry = FindByName(realname);
    }

    if ( entry )
    {
        wxLogTrace(wxT("dll"),
                   wxT("LoadLibrary(%s): already loaded."), realname.c_str());

        entry->RefLib();
    }
    else
    {
        entry = new wxPluginLibrary( libname, flags );

        if ( entry->IsLoaded() )
        {
            (*ms_manifest)[realname] = entry;

            wxLogTrace(wxT("dll"),
                       wxT("LoadLibrary(%s): loaded ok."), realname.c_str());

        }
        else
        {
            wxLogTrace(wxT("dll"),
                       wxT("LoadLibrary(%s): failed to load."), realname.c_str());

                        if ( !entry->UnrefLib() )
            {
                                wxFAIL_MSG( wxT("Currently linked library is not loaded?") );
            }

            entry = NULL;
        }
    }

    return entry;
}

bool wxPluginManager::UnloadLibrary(const wxString& libname)
{
    wxString realname = libname;

    wxPluginLibrary *entry = FindByName(realname);

    if ( !entry )
    {
        realname += wxDynamicLibrary::GetDllExt(wxDL_MODULE);

        entry = FindByName(realname);
    }

    if ( !entry )
    {
        wxLogDebug(wxT("Attempt to unload library '%s' which is not loaded."),
                   libname.c_str());

        return false;
    }

    wxLogTrace(wxT("dll"), wxT("UnloadLibrary(%s)"), realname.c_str());

    if ( !entry->UnrefLib() )
    {
                return false;
    }

    ms_manifest->erase(ms_manifest->find(realname));

    return true;
}


bool wxPluginManager::Load(const wxString &libname, int flags)
{
    m_entry = wxPluginManager::LoadLibrary(libname, flags);

    return IsLoaded();
}

void wxPluginManager::Unload()
{
    wxCHECK_RET( m_entry, wxT("unloading an invalid wxPluginManager?") );

    for ( wxDLManifest::iterator i = ms_manifest->begin();
          i != ms_manifest->end();
          ++i )
    {
        if ( i->second == m_entry )
        {
            ms_manifest->erase(i);
            break;
        }
    }

    m_entry->UnrefLib();

    m_entry = NULL;
}

#endif  