
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_FSWATCHER

#include "wx/fswatcher.h"

#ifdef wxHAS_KQUEUE

#include <sys/types.h>
#include <sys/event.h>

#include "wx/dynarray.h"
#include "wx/evtloop.h"
#include "wx/evtloopsrc.h"

#include "wx/private/fswatcher.h"


class wxFSWatcherImplKqueue;


class wxFSWSourceHandler : public wxEventLoopSourceHandler
{
public:
    wxFSWSourceHandler(wxFSWatcherImplKqueue* service) :
        m_service(service)
    {  }

    virtual void OnReadWaiting() wxOVERRIDE;
    virtual void OnWriteWaiting() wxOVERRIDE;
    virtual void OnExceptionWaiting() wxOVERRIDE;

protected:
    wxFSWatcherImplKqueue* m_service;
};



class wxFSWatcherImplKqueue : public wxFSWatcherImpl
{
public:
    wxFSWatcherImplKqueue(wxFileSystemWatcherBase* watcher) :
        wxFSWatcherImpl(watcher),
        m_source(NULL),
        m_kfd(-1)
    {
        m_handler = new wxFSWSourceHandler(this);
    }

    virtual ~wxFSWatcherImplKqueue()
    {
                if (IsOk())
        {
            Close();
        }

        delete m_handler;
    }

    bool Init() wxOVERRIDE
    {
        wxCHECK_MSG( !IsOk(), false,
                     "Kqueue appears to be already initialized" );

        wxEventLoopBase *loop = wxEventLoopBase::GetActive();
        wxCHECK_MSG( loop, false, "File system watcher needs an active loop" );

                m_kfd = kqueue();
        if (m_kfd == -1)
        {
            wxLogSysError(_("Unable to create kqueue instance"));
            return false;
        }

                m_source = loop->AddSourceForFD(m_kfd, m_handler, wxEVENT_SOURCE_INPUT);

        return m_source != NULL;
    }

    void Close()
    {
        wxCHECK_RET( IsOk(),
                    "Kqueue not initialized or invalid kqueue descriptor" );

        if ( close(m_kfd) != 0 )
        {
            wxLogSysError(_("Error closing kqueue instance"));
        }

        wxDELETE(m_source);
    }

    virtual bool DoAdd(wxSharedPtr<wxFSWatchEntryKq> watch) wxOVERRIDE
    {
        wxCHECK_MSG( IsOk(), false,
                    "Kqueue not initialized or invalid kqueue descriptor" );

        struct kevent event;
        int action = EV_ADD | EV_ENABLE | EV_CLEAR | EV_ERROR;
        int flags = Watcher2NativeFlags(watch->GetFlags());
        EV_SET( &event, watch->GetFileDescriptor(), EVFILT_VNODE, action,
                flags, 0, watch.get() );

                        int ret = kevent(m_kfd, &event, 1, NULL, 0, NULL);
        if (ret == -1)
        {
            wxLogSysError(_("Unable to add kqueue watch"));
            return false;
        }

        return true;
    }

    virtual bool DoRemove(wxSharedPtr<wxFSWatchEntryKq> watch) wxOVERRIDE
    {
        wxCHECK_MSG( IsOk(), false,
                    "Kqueue not initialized or invalid kqueue descriptor" );

                                if ( !watch->Close() )
        {
            wxLogSysError(_("Unable to remove kqueue watch"));
            return false;
        }

        return true;
    }

    virtual bool RemoveAll() wxOVERRIDE
    {
        wxFSWatchEntries::iterator it = m_watches.begin();
        for ( ; it != m_watches.end(); ++it )
        {
            (void) DoRemove(it->second);
        }
        m_watches.clear();
        return true;
    }

        bool ReadEvents()
    {
        wxCHECK_MSG( IsOk(), false,
                    "Kqueue not initialized or invalid kqueue descriptor" );

                do
        {
            struct kevent event;
            struct timespec timeout = {0, 0};
            int ret = kevent(m_kfd, NULL, 0, &event, 1, &timeout);
            if (ret == -1)
            {
                wxLogSysError(_("Unable to get events from kqueue"));
                return false;
            }
            else if (ret == 0)
            {
                return true;
            }

                        ProcessNativeEvent(event);
        }
        while (true);

                wxFAIL_MSG( "Never reached" );
        return true;
    }

    bool IsOk() const
    {
        return m_source != NULL;
    }

protected:
                void FindChanges(wxFSWatchEntryKq& watch,
                     wxArrayString& changedFiles,
                     wxArrayInt& changedFlags)
    {
        wxFSWatchEntryKq::wxDirState old = watch.GetLastState();
        watch.RefreshState();
        wxFSWatchEntryKq::wxDirState curr = watch.GetLastState();

                wxArrayString::iterator oit = old.files.begin();
        wxArrayString::iterator cit = curr.files.begin();
        for ( ; oit != old.files.end() && cit != curr.files.end(); )
        {
            if ( *cit == *oit )
            {
                ++cit;
                ++oit;
            }
            else if ( *cit <= *oit )
            {
                changedFiles.push_back(*cit);
                changedFlags.push_back(wxFSW_EVENT_CREATE);
                ++cit;
            }
            else             {
                changedFiles.push_back(*oit);
                changedFlags.push_back(wxFSW_EVENT_DELETE);
                ++oit;
            }
        }

                if ( oit == old.files.end() )
        {
            for ( ; cit != curr.files.end(); ++cit )
            {
                changedFiles.push_back( *cit );
                changedFlags.push_back(wxFSW_EVENT_CREATE);
            }
        }
        else if ( cit == curr.files.end() )
        {
            for ( ; oit != old.files.end(); ++oit )
            {
                changedFiles.push_back( *oit );
                changedFlags.push_back(wxFSW_EVENT_DELETE);
            }
        }

        wxASSERT( changedFiles.size() == changedFlags.size() );

#if 0
        wxLogTrace(wxTRACE_FSWATCHER, "Changed files:");
        wxArrayString::iterator it = changedFiles.begin();
        wxArrayInt::iterator it2 = changedFlags.begin();
        for ( ; it != changedFiles.end(); ++it, ++it2)
        {
            wxString action = (*it2 == wxFSW_EVENT_CREATE) ?
                                "created" : "deleted";
            wxLogTrace(wxTRACE_FSWATCHER, wxString::Format("File: '%s' %s",
                        *it, action));
        }
#endif
    }

    void ProcessNativeEvent(const struct kevent& e)
    {
        wxASSERT_MSG(e.udata, "Null user data associated with kevent!");

        wxLogTrace(wxTRACE_FSWATCHER, "Event: ident=%llu, filter=%d, flags=%u, "
                   "fflags=%u, data=%lld, user_data=%lp",
                   e.ident, e.filter, e.flags, e.fflags, e.data, e.udata);

                wxFSWatchEntryKq& w = *(static_cast<wxFSWatchEntry*>(e.udata));
        int nflags = e.fflags;

                nflags &= ~NOTE_REVOKE;

                        while ( nflags )
        {
                        const wxString basepath = w.GetPath();
            if ( nflags & NOTE_WRITE && wxDirExists(basepath) )
            {
                                                                                nflags &= ~(NOTE_WRITE | NOTE_ATTRIB | NOTE_LINK);

                wxArrayString changedFiles;
                wxArrayInt changedFlags;
                FindChanges(w, changedFiles, changedFlags);

                wxArrayString::iterator it = changedFiles.begin();
                wxArrayInt::iterator changeType = changedFlags.begin();
                for ( ; it != changedFiles.end(); ++it, ++changeType )
                {
                    const wxString fullpath = w.GetPath() +
                                                wxFileName::GetPathSeparator() +
                                                  *it;
                    const wxFileName path(wxDirExists(fullpath)
                                            ? wxFileName::DirName(fullpath)
                                            : wxFileName::FileName(fullpath));

                    wxFileSystemWatcherEvent event(*changeType, path, path);
                    SendEvent(event);
                }
            }
            else if ( nflags & NOTE_RENAME )
            {
                                                                nflags &= ~NOTE_RENAME;
                wxFileSystemWatcherEvent event(wxFSW_EVENT_RENAME,
                                        basepath, wxFileName());
                SendEvent(event);
            }
            else if ( nflags & NOTE_WRITE || nflags & NOTE_EXTEND )
            {
                nflags &= ~(NOTE_WRITE | NOTE_EXTEND);
                wxFileSystemWatcherEvent event(wxFSW_EVENT_MODIFY,
                                        basepath, basepath);
                SendEvent(event);
            }
            else if ( nflags & NOTE_DELETE )
            {
                nflags &= ~(NOTE_DELETE);
                wxFileSystemWatcherEvent event(wxFSW_EVENT_DELETE,
                                        basepath, basepath);
                SendEvent(event);
            }
            else if ( nflags & NOTE_ATTRIB )
            {
                nflags &= ~(NOTE_ATTRIB);
                wxFileSystemWatcherEvent event(wxFSW_EVENT_ACCESS,
                                        basepath, basepath);
                SendEvent(event);
            }

                        nflags &= ~(NOTE_LINK);
        }
    }

    void SendEvent(wxFileSystemWatcherEvent& evt)
    {
        m_watcher->GetOwner()->ProcessEvent(evt);
    }

    static int Watcher2NativeFlags(int WXUNUSED(flags))
    {
                return NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND |
               NOTE_ATTRIB | NOTE_LINK | NOTE_RENAME |
               NOTE_REVOKE;
    }

    wxFSWSourceHandler* m_handler;            wxEventLoopSource* m_source;          
        int m_kfd;
};


void wxFSWSourceHandler::OnReadWaiting()
{
    wxLogTrace(wxTRACE_FSWATCHER, "--- OnReadWaiting ---");
    m_service->ReadEvents();
}

void wxFSWSourceHandler::OnWriteWaiting()
{
    wxFAIL_MSG("We never write to kqueue descriptor.");
}

void wxFSWSourceHandler::OnExceptionWaiting()
{
    wxFAIL_MSG("We never receive exceptions on kqueue descriptor.");
}



wxKqueueFileSystemWatcher::wxKqueueFileSystemWatcher()
    : wxFileSystemWatcherBase()
{
    Init();
}

wxKqueueFileSystemWatcher::wxKqueueFileSystemWatcher(const wxFileName& path,
                                                     int events)
    : wxFileSystemWatcherBase()
{
    if (!Init())
    {
        wxDELETE(m_service);
        return;
    }

    Add(path, events);
}

wxKqueueFileSystemWatcher::~wxKqueueFileSystemWatcher()
{
}

bool wxKqueueFileSystemWatcher::Init()
{
    m_service = new wxFSWatcherImplKqueue(this);
    return m_service->Init();
}

#endif 
#endif 