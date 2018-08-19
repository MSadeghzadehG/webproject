
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_FSWATCHER

#include "wx/fswatcher.h"

#ifdef wxHAS_INOTIFY

#include <sys/inotify.h>
#include <unistd.h>
#include "wx/private/fswatcher.h"


WX_DECLARE_HASH_MAP(int, wxFSWatchEntry*, wxIntegerHash, wxIntegerEqual,
                                              wxFSWatchEntryDescriptors);

WX_DECLARE_HASH_MAP(int, inotify_event*, wxIntegerHash, wxIntegerEqual,
                                                      wxInotifyCookies);


class wxFSWatcherImplUnix : public wxFSWatcherImpl
{
public:
    wxFSWatcherImplUnix(wxFileSystemWatcherBase* watcher) :
        wxFSWatcherImpl(watcher),
        m_source(NULL),
        m_ifd(-1)
    {
        m_handler = new wxFSWSourceHandler(this);
    }

    ~wxFSWatcherImplUnix()
    {
                if (IsOk())
        {
            Close();
        }

        delete m_handler;
    }

    bool Init()
    {
        wxCHECK_MSG( !IsOk(), false, "Inotify already initialized" );

        wxEventLoopBase *loop = wxEventLoopBase::GetActive();
        wxCHECK_MSG( loop, false, "File system watcher needs an event loop" );

        m_ifd = inotify_init();
        if ( m_ifd == -1 )
        {
            wxLogSysError( _("Unable to create inotify instance") );
            return false;
        }

        m_source = loop->AddSourceForFD
                         (
                          m_ifd,
                          m_handler,
                          wxEVENT_SOURCE_INPUT | wxEVENT_SOURCE_EXCEPTION
                         );

        return m_source != NULL;
    }

    void Close()
    {
        wxCHECK_RET( IsOk(),
                    "Inotify not initialized or invalid inotify descriptor" );

        wxDELETE(m_source);

        if ( close(m_ifd) != 0 )
        {
            wxLogSysError( _("Unable to close inotify instance") );
        }
    }

    virtual bool DoAdd(wxSharedPtr<wxFSWatchEntryUnix> watch)
    {
        wxCHECK_MSG( IsOk(), false,
                    "Inotify not initialized or invalid inotify descriptor" );

        int wd = DoAddInotify(watch.get());
        if (wd == -1)
        {
            wxLogSysError( _("Unable to add inotify watch") );
            return false;
        }

        wxFSWatchEntryDescriptors::value_type val(wd, watch.get());
        if (!m_watchMap.insert(val).second)
        {
            wxFAIL_MSG( wxString::Format( "Path %s is already watched",
                                           watch->GetPath()) );
            return false;
        }

        return true;
    }

    virtual bool DoRemove(wxSharedPtr<wxFSWatchEntryUnix> watch)
    {
        wxCHECK_MSG( IsOk(), false,
                    "Inotify not initialized or invalid inotify descriptor" );

        int ret = DoRemoveInotify(watch.get());
        if (ret == -1)
        {
            wxLogSysError( _("Unable to remove inotify watch") );
            return false;
        }

        if (m_watchMap.erase(watch->GetWatchDescriptor()) != 1)
        {
            wxFAIL_MSG( wxString::Format("Path %s is not watched",
                                          watch->GetPath()) );
        }
                m_staleDescriptors.Add(watch->GetWatchDescriptor());

        watch->SetWatchDescriptor(-1);
        return true;
    }

    virtual bool RemoveAll()
    {
        wxFSWatchEntries::iterator it = m_watches.begin();
        for ( ; it != m_watches.end(); ++it )
        {
            (void) DoRemove(it->second);
        }
        m_watches.clear();
        return true;
    }

    int ReadEvents()
    {
        wxCHECK_MSG( IsOk(), -1,
                    "Inotify not initialized or invalid inotify descriptor" );

                        char buf[128 * sizeof(inotify_event)];
        int left = ReadEventsToBuf(buf, sizeof(buf));
        if (left == -1)
            return -1;

                char* memory = buf;
        int event_count = 0;
        while (left > 0)         {
            event_count++;
            inotify_event* e = (inotify_event*)memory;

                        ProcessNativeEvent(*e);

            int offset = sizeof(inotify_event) + e->len;
            left -= offset;
            memory += offset;
        }

                ProcessRenames();

        wxLogTrace(wxTRACE_FSWATCHER, "We had %d native events", event_count);
        return event_count;
    }

    bool IsOk() const
    {
        return m_source != NULL;
    }

protected:
    int DoAddInotify(wxFSWatchEntry* watch)
    {
        int flags = Watcher2NativeFlags(watch->GetFlags());
        int wd = inotify_add_watch(m_ifd, watch->GetPath().fn_str(), flags);
                watch->SetWatchDescriptor(wd);
        return wd;
    }

    int DoRemoveInotify(wxFSWatchEntry* watch)
    {
        return inotify_rm_watch(m_ifd, watch->GetWatchDescriptor());
    }

    void ProcessNativeEvent(const inotify_event& inevt)
    {
        wxLogTrace(wxTRACE_FSWATCHER, InotifyEventToString(inevt));

                        if (inevt.mask & IN_IGNORED)
        {
                                                            const int pos = m_staleDescriptors.Index(inevt.wd);
            if ( pos != wxNOT_FOUND )
            {
                m_staleDescriptors.RemoveAt(static_cast<size_t>(pos));
                wxLogTrace(wxTRACE_FSWATCHER,
                       "Removed wd %i from the stale-wd cache", inevt.wd);
            }
            return;
        }

                wxFSWatchEntryDescriptors::iterator it = m_watchMap.find(inevt.wd);

                if (inevt.wd != -1)
        {
            if (it == m_watchMap.end())
            {
                                if (m_staleDescriptors.Index(inevt.wd) != wxNOT_FOUND)
                {
                    wxLogTrace(wxTRACE_FSWATCHER,
                               "Got an event for stale wd %i", inevt.wd);
                }
                else
                {
                                                                                wxFileSystemWatcherEvent
                        event
                        (
                            wxFSW_EVENT_WARNING,
                            wxFSW_WARNING_GENERAL,
                            wxString::Format
                            (
                             _("Unexpected event for \"%s\": no "
                               "matching watch descriptor."),
                             inevt.len ? inevt.name : ""
                            )
                        );
                    SendEvent(event);

                }

                                                return;
            }
        }

        int nativeFlags = inevt.mask;
        int flags = Native2WatcherFlags(nativeFlags);

                if (flags & wxFSW_EVENT_WARNING || flags & wxFSW_EVENT_ERROR)
        {
            wxFSWWarningType warningType;
            if ( flags & wxFSW_EVENT_WARNING )
            {
                warningType = nativeFlags & IN_Q_OVERFLOW
                                ? wxFSW_WARNING_OVERFLOW
                                : wxFSW_WARNING_GENERAL;
            }
            else             {
                warningType = wxFSW_WARNING_NONE;
            }

            wxFileSystemWatcherEvent event(flags, warningType);
            SendEvent(event);
            return;
        }

        if (inevt.wd == -1)
        {
                                    wxFileSystemWatcherEvent
                event
                (
                    wxFSW_EVENT_WARNING,
                    wxFSW_WARNING_GENERAL,
                    wxString::Format
                    (
                        _("Invalid inotify event for \"%s\""),
                        inevt.len ? inevt.name : ""
                    )
                );
            SendEvent(event);
            return;
        }

        wxFSWatchEntry& watch = *(it->second);

                if (nativeFlags & IN_UNMOUNT)
        {
            wxFileName path = GetEventPath(watch, inevt);
            wxFileSystemWatcherEvent event(wxFSW_EVENT_UNMOUNT, path, path);
            SendEvent(event);
        }
                        else if ((flags == 0) || !(flags & watch.GetFlags()))
        {
            return;
        }

                                                else if ((nativeFlags & IN_CREATE) &&
                 (watch.GetType() == wxFSWPath_Tree) && (inevt.mask & IN_ISDIR))
        {
            wxFileName fn = GetEventPath(watch, inevt);
                        fn.AssignDir(fn.GetFullPath());

            if (m_watcher->AddAny(fn, wxFSW_EVENT_ALL,
                                   wxFSWPath_Tree, watch.GetFilespec()))
            {
                                                if (watch.GetFilespec().empty())
                {
                    wxFileSystemWatcherEvent event(flags, fn, fn);
                    SendEvent(event);
                }
            }
        }

                                                                else if ((nativeFlags & IN_DELETE_SELF) &&
                    ((watch.GetType() == wxFSWPath_Dir) ||
                     (watch.GetType() == wxFSWPath_Tree)))
        {
                                                wxFileName fn = GetEventPath(watch, inevt);
            wxString path(fn.GetPathWithSep());

            if (m_watchMap.erase(inevt.wd) == 1)
            {
                                wxDynamicCast(m_watcher, wxInotifyFileSystemWatcher)->
                                            OnDirDeleted(path);

                                wxFSWatchEntries::iterator wit =
                                        m_watches.find(path);
                if (wit != m_watches.end())
                {
                    m_watches.erase(wit);
                }

                                m_staleDescriptors.Add(inevt.wd);
            }

                                    if (watch.GetFilespec().empty())
            {
                wxFileSystemWatcherEvent event(flags, fn, fn);
                SendEvent(event);
            }
        }

                else if (nativeFlags & IN_MOVE)
        {
                                                                                    
                                                wxInotifyCookies::iterator it2 = m_cookies.find(inevt.cookie);
            if ( it2 == m_cookies.end() )
            {
                int size = sizeof(inevt) + inevt.len;
                inotify_event* e = (inotify_event*) operator new (size);
                memcpy(e, &inevt, size);

                wxInotifyCookies::value_type val(e->cookie, e);
                m_cookies.insert(val);
            }
            else
            {
                inotify_event& oldinevt = *(it2->second);

                                                if ( watch.GetFilespec().empty() )
                {
                                                                                                    wxFSWatchEntry* oldwatch;
                    wxFSWatchEntryDescriptors::iterator oldwatch_it =
                                                m_watchMap.find(oldinevt.wd);
                    if (oldwatch_it != m_watchMap.end())
                    {
                        oldwatch = oldwatch_it->second;
                    }
                    else
                    {
                        wxLogTrace(wxTRACE_FSWATCHER,
                            "oldinevt's watch descriptor not in the watch map");
                                                                        oldwatch = &watch;
                    }

                    wxFileSystemWatcherEvent event(flags);
                    if ( inevt.mask & IN_MOVED_FROM )
                    {
                        event.SetPath(GetEventPath(watch, inevt));
                        event.SetNewPath(GetEventPath(*oldwatch, oldinevt));
                    }
                    else
                    {
                        event.SetPath(GetEventPath(*oldwatch, oldinevt));
                        event.SetNewPath(GetEventPath(watch, inevt));
                    }
                    SendEvent(event);
                }

                m_cookies.erase(it2);
                delete &oldinevt;
            }
        }
                else
        {
            wxFileName path = GetEventPath(watch, inevt);
                        if ( MatchesFilespec(path, watch.GetFilespec()) )
            {
                wxFileSystemWatcherEvent event(flags, path, path);
                SendEvent(event);
            }
        }
    }

    void ProcessRenames()
    {
                        wxInotifyCookies::iterator it = m_cookies.begin();
        while ( it != m_cookies.end() )
        {
            inotify_event& inevt = *(it->second);

            wxLogTrace(wxTRACE_FSWATCHER, "Processing pending rename events");
            wxLogTrace(wxTRACE_FSWATCHER, InotifyEventToString(inevt));

                        wxFSWatchEntryDescriptors::iterator wit = m_watchMap.find(inevt.wd);
            if (wit == m_watchMap.end())
            {
                wxLogTrace(wxTRACE_FSWATCHER,
                            "Watch descriptor not present in the watch map!");
            }
            else
            {
                                                wxFSWatchEntry& watch = *(wit->second);
                if ( watch.GetFilespec().empty() )
                {
                    int flags = Native2WatcherFlags(inevt.mask);
                    wxFileName path = GetEventPath(watch, inevt);
                    {
                        wxFileSystemWatcherEvent event(flags, path, path);
                        SendEvent(event);
                    }
                }
            }

            m_cookies.erase(it);
            delete &inevt;
            it = m_cookies.begin();
        }
    }

    void SendEvent(wxFileSystemWatcherEvent& evt)
    {
        wxLogTrace(wxTRACE_FSWATCHER, evt.ToString());
        m_watcher->GetOwner()->ProcessEvent(evt);
    }

    int ReadEventsToBuf(char* buf, int size)
    {
        wxCHECK_MSG( IsOk(), false,
                    "Inotify not initialized or invalid inotify descriptor" );

        memset(buf, 0, size);
        ssize_t left = read(m_ifd, buf, size);
        if (left == -1)
        {
            wxLogSysError(_("Unable to read from inotify descriptor"));
            return -1;
        }
        else if (left == 0)
        {
            wxLogWarning(_("EOF while reading from inotify descriptor"));
            return -1;
        }

        return left;
    }

    static wxString InotifyEventToString(const inotify_event& inevt)
    {
        wxString mask = (inevt.mask & IN_ISDIR) ?
                        wxString::Format("IS_DIR | %u", inevt.mask & ~IN_ISDIR) :
                        wxString::Format("%u", inevt.mask);
        const char* name = "";
        if (inevt.len)
            name = inevt.name;
        return wxString::Format("Event: wd=%d, mask=%s, cookie=%u, len=%u, "
                                "name=%s", inevt.wd, mask, inevt.cookie,
                                inevt.len, name);
    }

    static wxFileName GetEventPath(const wxFSWatchEntry& watch,
                                   const inotify_event& inevt)
    {
                wxFileName path = watch.GetPath();
        if (path.IsDir() && inevt.len)
        {
            path = wxFileName(path.GetPath(), inevt.name);
        }
        return path;
    }

    static int Watcher2NativeFlags(int flags)
    {
                if (flags == wxFSW_EVENT_ALL)
        {
            return IN_ALL_EVENTS;
        }

        static const int flag_mapping[][2] = {
            { wxFSW_EVENT_ACCESS, IN_ACCESS   },
            { wxFSW_EVENT_MODIFY, IN_MODIFY   },
            { wxFSW_EVENT_ATTRIB, IN_ATTRIB   },
            { wxFSW_EVENT_RENAME, IN_MOVE     },
            { wxFSW_EVENT_CREATE, IN_CREATE   },
            { wxFSW_EVENT_DELETE, IN_DELETE|IN_DELETE_SELF|IN_MOVE_SELF },
            { wxFSW_EVENT_UNMOUNT, IN_UNMOUNT }
                    };

        int native_flags = 0;
        for ( unsigned int i=0; i < WXSIZEOF(flag_mapping); ++i)
        {
            if (flags & flag_mapping[i][0])
                native_flags |= flag_mapping[i][1];
        }

        return native_flags;
    }

    static int Native2WatcherFlags(int flags)
    {
        static const int flag_mapping[][2] = {
            { IN_ACCESS,        wxFSW_EVENT_ACCESS },             { IN_MODIFY,        wxFSW_EVENT_MODIFY },
            { IN_ATTRIB,        wxFSW_EVENT_ATTRIB },
            { IN_CLOSE_WRITE,   0 },
            { IN_CLOSE_NOWRITE, 0 },
            { IN_OPEN,          0 },
            { IN_MOVED_FROM,    wxFSW_EVENT_RENAME },
            { IN_MOVED_TO,      wxFSW_EVENT_RENAME },
            { IN_CREATE,        wxFSW_EVENT_CREATE },
            { IN_DELETE,        wxFSW_EVENT_DELETE },
            { IN_DELETE_SELF,   wxFSW_EVENT_DELETE },
            { IN_MOVE_SELF,     wxFSW_EVENT_DELETE },

            { IN_UNMOUNT,       wxFSW_EVENT_UNMOUNT},
            { IN_Q_OVERFLOW,    wxFSW_EVENT_WARNING},

                        { IN_IGNORED,        0 }
        };

        unsigned int i=0;
        for ( ; i < WXSIZEOF(flag_mapping); ++i) {
                        if (flags & flag_mapping[i][0])
                return flag_mapping[i][1];
        }

                wxFAIL_MSG(wxString::Format("Unknown inotify event mask %u", flags));
        return -1;
    }

    wxFSWSourceHandler* m_handler;            wxFSWatchEntryDescriptors m_watchMap;     wxArrayInt m_staleDescriptors;            wxInotifyCookies m_cookies;               wxEventLoopSource* m_source;          
        int m_ifd;
};



void wxFSWSourceHandler::OnReadWaiting()
{
    wxLogTrace(wxTRACE_FSWATCHER, "--- OnReadWaiting ---");
    m_service->ReadEvents();
}

void wxFSWSourceHandler::OnWriteWaiting()
{
    wxFAIL_MSG("We never write to inotify descriptor.");
}

void wxFSWSourceHandler::OnExceptionWaiting()
{
    wxFAIL_MSG("We never receive exceptions on inotify descriptor.");
}



wxInotifyFileSystemWatcher::wxInotifyFileSystemWatcher()
    : wxFileSystemWatcherBase()
{
    Init();
}

wxInotifyFileSystemWatcher::wxInotifyFileSystemWatcher(const wxFileName& path,
                                                       int events)
    : wxFileSystemWatcherBase()
{
    if (!Init())
    {
        if (m_service)
            delete m_service;
        return;
    }

    Add(path, events);
}

wxInotifyFileSystemWatcher::~wxInotifyFileSystemWatcher()
{
}

bool wxInotifyFileSystemWatcher::Init()
{
    m_service = new wxFSWatcherImplUnix(this);
    return m_service->Init();
}

void wxInotifyFileSystemWatcher::OnDirDeleted(const wxString& path)
{
    if (!path.empty())
    {
        wxFSWatchInfoMap::iterator it = m_watches.find(path);
        wxCHECK_RET(it != m_watches.end(),
                    wxString::Format("Path '%s' is not watched", path));

                m_watches.erase(it);
    }
}

#endif 
#endif 