
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_FSWATCHER

#include "wx/fswatcher.h"
#include "wx/private/fswatcher.h"


wxDEFINE_EVENT(wxEVT_FSWATCHER, wxFileSystemWatcherEvent);

static wxString GetFSWEventChangeTypeName(int type)
{
    switch (type)
    {
    case wxFSW_EVENT_CREATE:
        return "CREATE";
    case wxFSW_EVENT_DELETE:
        return "DELETE";
    case wxFSW_EVENT_RENAME:
        return "RENAME";
    case wxFSW_EVENT_MODIFY:
        return "MODIFY";
    case wxFSW_EVENT_ACCESS:
        return "ACCESS";
    case wxFSW_EVENT_ATTRIB:         return "ATTRIBUTE";
#ifdef wxHAS_INOTIFY
    case wxFSW_EVENT_UNMOUNT:         return "UNMOUNT";
#endif
    case wxFSW_EVENT_WARNING:
        return "WARNING";
    case wxFSW_EVENT_ERROR:
        return "ERROR";
    }

        wxFAIL_MSG("Unknown change type");
    return "INVALID_TYPE";
}



wxIMPLEMENT_DYNAMIC_CLASS(wxFileSystemWatcherEvent, wxEvent);

wxString wxFileSystemWatcherEvent::ToString() const
{
    if (IsError())
    {
        return wxString::Format("FSW_EVT type=%d (%s) message='%s'", m_changeType,
            GetFSWEventChangeTypeName(m_changeType), GetErrorDescription());
    }
    return wxString::Format("FSW_EVT type=%d (%s) path='%s'", m_changeType,
            GetFSWEventChangeTypeName(m_changeType), GetPath().GetFullPath());
}



wxFileSystemWatcherBase::wxFileSystemWatcherBase() :
    m_service(0), m_owner(this)
{
}

wxFileSystemWatcherBase::~wxFileSystemWatcherBase()
{
    RemoveAll();
    if (m_service)
    {
        delete m_service;
    }
}

bool wxFileSystemWatcherBase::Add(const wxFileName& path, int events)
{
    wxFSWPathType type = wxFSWPath_None;
    if ( path.FileExists() )
    {
        type = wxFSWPath_File;
    }
    else if ( path.DirExists() )
    {
        type = wxFSWPath_Dir;
    }
    else
    {
                        wxLogTrace(wxTRACE_FSWATCHER,
                   "Can't monitor non-existent path \"%s\" for changes.",
                   path.GetFullPath());
        return false;
    }

    return AddAny(path, events, type);
}

bool
wxFileSystemWatcherBase::AddAny(const wxFileName& path,
                                int events,
                                wxFSWPathType type,
                                const wxString& filespec)
{
    wxString canonical = GetCanonicalPath(path);
    if (canonical.IsEmpty())
        return false;

        wxFSWatchInfo watch(canonical, events, type, filespec);
    if ( !m_service->Add(watch) )
        return false;

                wxFSWatchInfoMap::iterator it = m_watches.find(canonical);
    if ( it == m_watches.end() )
    {
        wxFSWatchInfoMap::value_type val(canonical, watch);
        m_watches.insert(val);
    }
    else
    {
        wxFSWatchInfo& watch2 = it->second;
        const int count = watch2.IncRef();

        wxLogTrace(wxTRACE_FSWATCHER,
                   "'%s' is now watched %d times", canonical, count);

        wxUnusedVar(count);     }
    return true;
}

bool wxFileSystemWatcherBase::Remove(const wxFileName& path)
{
        wxString canonical = GetCanonicalPath(path);
    if (canonical.IsEmpty())
        return false;

    wxFSWatchInfoMap::iterator it = m_watches.find(canonical);
    wxCHECK_MSG(it != m_watches.end(), false,
                wxString::Format("Path '%s' is not watched", canonical));

        bool ret = true;
    wxFSWatchInfo& watch = it->second;
    if ( !watch.DecRef() )
    {
                ret = m_service->Remove(watch);

        m_watches.erase(it);
    }
    return ret;
}

bool wxFileSystemWatcherBase::AddTree(const wxFileName& path, int events,
                                      const wxString& filespec)
{
    if (!path.DirExists())
        return false;

            class AddTraverser : public wxDirTraverser
    {
    public:
        AddTraverser(wxFileSystemWatcherBase* watcher, int events,
                     const wxString& filespec) :
            m_watcher(watcher), m_events(events), m_filespec(filespec)
        {
        }

        virtual wxDirTraverseResult OnFile(const wxString& WXUNUSED(filename)) wxOVERRIDE
        {
                                    return wxDIR_CONTINUE;
        }

        virtual wxDirTraverseResult OnDir(const wxString& dirname) wxOVERRIDE
        {
            if ( m_watcher->AddAny(wxFileName::DirName(dirname),
                                   m_events, wxFSWPath_Tree, m_filespec) )
            {
                wxLogTrace(wxTRACE_FSWATCHER,
                   "--- AddTree adding directory '%s' ---", dirname);
            }
            return wxDIR_CONTINUE;
        }

    private:
        wxFileSystemWatcherBase* m_watcher;
        int m_events;
        wxString m_filespec;
    };

    wxDir dir(path.GetFullPath());
        int flags = wxDIR_DIRS;
    if ( !path.ShouldFollowLink() )
    {
        flags |= wxDIR_NO_FOLLOW;
    }
    AddTraverser traverser(this, events, filespec);
    dir.Traverse(traverser, filespec, flags);

        AddAny(path.GetPathWithSep(), events, wxFSWPath_Tree, filespec);

    return true;
}

bool wxFileSystemWatcherBase::RemoveTree(const wxFileName& path)
{
    if (!path.DirExists())
        return false;

            class RemoveTraverser : public wxDirTraverser
    {
    public:
        RemoveTraverser(wxFileSystemWatcherBase* watcher,
                        const wxString& filespec) :
            m_watcher(watcher), m_filespec(filespec)
        {
        }

        virtual wxDirTraverseResult OnFile(const wxString& WXUNUSED(filename)) wxOVERRIDE
        {
                                    return wxDIR_CONTINUE;
        }

        virtual wxDirTraverseResult OnDir(const wxString& dirname) wxOVERRIDE
        {
            m_watcher->Remove(wxFileName::DirName(dirname));
            return wxDIR_CONTINUE;
        }

    private:
        wxFileSystemWatcherBase* m_watcher;
        wxString m_filespec;
    };

        wxString canonical = GetCanonicalPath(path);
    wxFSWatchInfoMap::iterator it = m_watches.find(canonical);
    wxCHECK_MSG( it != m_watches.end(), false,
                 wxString::Format("Path '%s' is not watched", canonical) );
    wxFSWatchInfo watch = it->second;
    const wxString filespec = watch.GetFilespec();

#if defined(__WINDOWS__)
            if (filespec.empty())
    {
        return Remove(path);
    }
    #endif 
    wxDir dir(path.GetFullPath());
                    int flags = wxDIR_DIRS;
    if ( !path.ShouldFollowLink() )
    {
        flags |= wxDIR_NO_FOLLOW;
    }
    RemoveTraverser traverser(this, filespec);
    dir.Traverse(traverser, filespec, flags);

        Remove(path);

    return true;
}

bool wxFileSystemWatcherBase::RemoveAll()
{
    const bool ret = m_service->RemoveAll();
    m_watches.clear();
    return ret;
}

int wxFileSystemWatcherBase::GetWatchedPathsCount() const
{
    return m_watches.size();
}

int wxFileSystemWatcherBase::GetWatchedPaths(wxArrayString* paths) const
{
    wxCHECK_MSG( paths != NULL, -1, "Null array passed to retrieve paths");

    wxFSWatchInfoMap::const_iterator it = m_watches.begin();
    for ( ; it != m_watches.end(); ++it)
    {
        paths->push_back(it->first);
    }

    return m_watches.size();
}

#endif 