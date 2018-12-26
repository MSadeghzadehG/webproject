


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_CONFIG

#ifndef WX_PRECOMP
#endif 
#include "wx/persist.h"

namespace
{

wxPersistenceManager* gs_manager = NULL;

} 


void wxPersistenceManager::Set(wxPersistenceManager& manager)
{
    gs_manager = &manager;
}


wxPersistenceManager& wxPersistenceManager::Get()
{
    if ( !gs_manager )
    {
        static wxPersistenceManager s_manager;

        gs_manager = &s_manager;
    }

    return *gs_manager;
}

wxPersistenceManager::~wxPersistenceManager()
{
}

wxString
wxPersistenceManager::GetKey(const wxPersistentObject& who,
                             const wxString& name) const
{
    wxString key("Persistent_Options");     key << wxCONFIG_PATH_SEPARATOR << who.GetKind()
        << wxCONFIG_PATH_SEPARATOR << who.GetName()
        << wxCONFIG_PATH_SEPARATOR << name;

    return key;
}

wxPersistentObject *wxPersistenceManager::Find(void *obj) const
{
    const wxPersistentObjectsMap::const_iterator
        it = m_persistentObjects.find(obj);
    return it == m_persistentObjects.end() ? NULL : it->second;
}

wxPersistentObject *
wxPersistenceManager::Register(void *obj, wxPersistentObject *po)
{
    if ( wxPersistentObject *old = Find(obj) )
    {
        wxFAIL_MSG( "object is already registered" );

        delete po;         return old;
    }

    m_persistentObjects[obj] = po;

    return po;
}

void wxPersistenceManager::Unregister(void *obj)
{
    wxPersistentObjectsMap::iterator it = m_persistentObjects.find(obj);
    wxCHECK_RET( it != m_persistentObjects.end(), "not registered" );

    wxPersistentObject * const po = it->second;
    m_persistentObjects.erase(it);
    delete po;
}

void wxPersistenceManager::Save(void *obj)
{
    if ( !m_doSave )
        return;

    wxPersistentObjectsMap::iterator it = m_persistentObjects.find(obj);
    wxCHECK_RET( it != m_persistentObjects.end(), "not registered" );

    it->second->Save();
}

bool wxPersistenceManager::Restore(void *obj)
{
    if ( !m_doRestore )
        return false;

    wxPersistentObjectsMap::iterator it = m_persistentObjects.find(obj);
    wxCHECK_MSG( it != m_persistentObjects.end(), false, "not registered" );

    return it->second->Restore();
}

namespace
{

template <typename T>
inline bool
DoSaveValue(wxConfigBase *conf, const wxString& key, T value)
{
    return conf && conf->Write(key, value);
}

template <typename T>
bool
DoRestoreValue(wxConfigBase *conf, const wxString& key, T *value)
{
    return conf && conf->Read(key, value);
}

} 
#define wxPERSIST_DEFINE_SAVE_RESTORE_FOR(Type)                               \
    bool wxPersistenceManager::SaveValue(const wxPersistentObject& who,       \
                                         const wxString& name,                \
                                         Type value)                          \
    {                                                                         \
        return DoSaveValue(GetConfig(), GetKey(who, name), value);            \
    }                                                                         \
                                                                              \
    bool wxPersistenceManager::RestoreValue(const wxPersistentObject& who,    \
                                            const wxString& name,             \
                                            Type *value)                      \
    {                                                                         \
        return DoRestoreValue(GetConfig(), GetKey(who, name), value);         \
    }

wxPERSIST_DEFINE_SAVE_RESTORE_FOR(bool)
wxPERSIST_DEFINE_SAVE_RESTORE_FOR(int)
wxPERSIST_DEFINE_SAVE_RESTORE_FOR(long)
wxPERSIST_DEFINE_SAVE_RESTORE_FOR(wxString)

#undef wxPERSIST_DEFINE_SAVE_RESTORE_FOR

#endif 