
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/module.h"

#ifndef WX_PRECOMP
    #include "wx/hash.h"
    #include "wx/intl.h"
    #include "wx/log.h"
#endif

#include "wx/listimpl.cpp"

#define TRACE_MODULE wxT("module")

WX_DEFINE_LIST(wxModuleList)

wxIMPLEMENT_ABSTRACT_CLASS(wxModule, wxObject)

wxModuleList wxModule::m_modules;

void wxModule::RegisterModule(wxModule* module)
{
    module->m_state = State_Registered;
    m_modules.Append(module);
}

void wxModule::UnregisterModule(wxModule* module)
{
    m_modules.DeleteObject(module);
    delete module;
}

void wxModule::RegisterModules()
{
    for (wxClassInfo::const_iterator it  = wxClassInfo::begin_classinfo(),
                                     end = wxClassInfo::end_classinfo();
         it != end; ++it)
    {
        const wxClassInfo* classInfo = *it;

        if ( classInfo->IsKindOf(wxCLASSINFO(wxModule)) &&
             (classInfo != (& (wxModule::ms_classInfo))) )
        {
            wxLogTrace(TRACE_MODULE, wxT("Registering module %s"),
                       classInfo->GetClassName());
            wxModule* module = (wxModule *)classInfo->CreateObject();
            wxModule::RegisterModule(module);
        }
    }
}

bool wxModule::DoInitializeModule(wxModule *module,
                                  wxModuleList &initializedModules)
{
    if ( module->m_state == State_Initializing )
    {
        wxLogError(_("Circular dependency involving module \"%s\" detected."),
                   module->GetClassInfo()->GetClassName());
        return false;
    }

    module->m_state = State_Initializing;

        if ( !module->ResolveNamedDependencies() )
      return false;

    const wxArrayClassInfo& dependencies = module->m_dependencies;

        for ( unsigned int i = 0; i < dependencies.size(); ++i )
    {
        wxClassInfo * cinfo = dependencies[i];

                wxModuleList::compatibility_iterator node;
        for ( node = initializedModules.GetFirst(); node; node = node->GetNext() )
        {
            if ( node->GetData()->GetClassInfo() == cinfo )
                break;
        }

        if ( node )
        {
                        continue;
        }

                for ( node = m_modules.GetFirst(); node; node = node->GetNext() )
        {
            wxModule *moduleDep = node->GetData();
            if ( moduleDep->GetClassInfo() == cinfo )
            {
                if ( !DoInitializeModule(moduleDep, initializedModules ) )
                {
                                        return false;
                }

                break;
            }
        }

        if ( !node )
        {
            wxLogError(_("Dependency \"%s\" of module \"%s\" doesn't exist."),
                       cinfo->GetClassName(),
                       module->GetClassInfo()->GetClassName());
            return false;
        }
    }

    if ( !module->Init() )
    {
        wxLogError(_("Module \"%s\" initialization failed"),
                   module->GetClassInfo()->GetClassName());
        return false;
    }

    wxLogTrace(TRACE_MODULE, wxT("Module \"%s\" initialized"),
               module->GetClassInfo()->GetClassName());

    module->m_state = State_Initialized;
    initializedModules.Append(module);

    return true;
}

bool wxModule::InitializeModules()
{
    wxModuleList initializedModules;

    for ( wxModuleList::compatibility_iterator node = m_modules.GetFirst();
          node;
          node = node->GetNext() )
    {
        wxModule *module = node->GetData();

                        if ( module->m_state == State_Registered )
        {
            if ( !DoInitializeModule( module, initializedModules ) )
            {
                                                DoCleanUpModules(initializedModules);

                return false;
            }
        }
    }

        m_modules = initializedModules;

    return true;
}

void wxModule::DoCleanUpModules(const wxModuleList& modules)
{
            for ( wxModuleList::compatibility_iterator node = modules.GetLast();
          node;
          node = node->GetPrevious() )
    {
        wxLogTrace(TRACE_MODULE, wxT("Cleanup module %s"),
                   node->GetData()->GetClassInfo()->GetClassName());

        wxModule * module = node->GetData();

        wxASSERT_MSG( module->m_state == State_Initialized,
                        wxT("not initialized module being cleaned up") );

        module->Exit();
        module->m_state = State_Registered;
    }

        WX_CLEAR_LIST(wxModuleList, m_modules);
}

bool wxModule::ResolveNamedDependencies()
{
        for ( size_t i = 0; i < m_namedDependencies.size(); ++i )
    {
        wxClassInfo *info = wxClassInfo::FindClass(m_namedDependencies[i]);

        if ( !info )
        {
                        return false;
        }

                                m_dependencies.Add(info);
    }

    return true;
}
