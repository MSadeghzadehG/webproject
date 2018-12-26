
#ifndef _WX_DYNAMICLOADER_H__
#define _WX_DYNAMICLOADER_H__


#include "wx/defs.h"

#if wxUSE_DYNAMIC_LOADER

#include "wx/dynlib.h"
#include "wx/hashmap.h"
#include "wx/module.h"

class WXDLLIMPEXP_FWD_BASE wxPluginLibrary;


WX_DECLARE_STRING_HASH_MAP_WITH_DECL(wxPluginLibrary *, wxDLManifest,
                                     class WXDLLIMPEXP_BASE);
typedef wxDLManifest wxDLImports;



class WXDLLIMPEXP_BASE wxPluginLibrary : public wxDynamicLibrary
{
public:

    static wxDLImports* ms_classes;  
    wxPluginLibrary( const wxString &libname, int flags = wxDL_DEFAULT );
    ~wxPluginLibrary();

    wxPluginLibrary  *RefLib();
    bool              UnrefLib();

                                
                                                
    void  RefObj() { ++m_objcount; }
    void  UnrefObj()
    {
        wxASSERT_MSG( m_objcount > 0, wxT("Too many objects deleted??") );
        --m_objcount;
    }

        
    bool  IsLoaded() const { return m_linkcount > 0; }
    void  Unload() { UnrefLib(); }

private:

                const wxClassInfo    *m_ourFirst;     const wxClassInfo    *m_ourLast;  
    size_t          m_linkcount;        size_t          m_objcount;         wxModuleList    m_wxmodules;    
    void    UpdateClasses();            void    RestoreClasses();           void    RegisterModules();          void    UnregisterModules();    
    wxDECLARE_NO_COPY_CLASS(wxPluginLibrary);
};


class WXDLLIMPEXP_BASE wxPluginManager
{
public:

        
    static wxPluginLibrary    *LoadLibrary( const wxString &libname,
                                            int flags = wxDL_DEFAULT );
    static bool                UnloadLibrary(const wxString &libname);

        
    wxPluginManager() : m_entry(NULL) {}
    wxPluginManager(const wxString &libname, int flags = wxDL_DEFAULT)
    {
        Load(libname, flags);
    }
    ~wxPluginManager() { if ( IsLoaded() ) Unload(); }

    bool   Load(const wxString &libname, int flags = wxDL_DEFAULT);
    void   Unload();

    bool   IsLoaded() const { return m_entry && m_entry->IsLoaded(); }
    void  *GetSymbol(const wxString &symbol, bool *success = 0)
    {
        return m_entry->GetSymbol( symbol, success );
    }

    static void CreateManifest() { ms_manifest = new wxDLManifest(wxKEY_STRING); }
    static void ClearManifest() { delete ms_manifest; ms_manifest = NULL; }

private:
            static wxPluginLibrary *FindByName(const wxString& name)
    {
        const wxDLManifest::iterator i = ms_manifest->find(name);

        return i == ms_manifest->end() ? NULL : i->second;
    }

    static wxDLManifest* ms_manifest;      wxPluginLibrary*     m_entry;      
            wxDECLARE_NO_COPY_CLASS(wxPluginManager);
};


#endif  #endif  
