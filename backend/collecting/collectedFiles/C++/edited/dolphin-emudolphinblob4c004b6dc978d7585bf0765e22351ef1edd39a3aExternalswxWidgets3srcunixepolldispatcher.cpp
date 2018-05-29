


#include "wx/wxprec.h"

#if wxUSE_EPOLL_DISPATCHER

#include "wx/unix/private/epolldispatcher.h"
#include "wx/unix/private.h"
#include "wx/stopwatch.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/intl.h"
#endif

#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>

#define wxEpollDispatcher_Trace wxT("epolldispatcher")


static uint32_t GetEpollMask(int flags, int fd)
{
    wxUnusedVar(fd); 
    uint32_t ep = 0;

    if ( flags & wxFDIO_INPUT )
    {
        ep |= EPOLLIN;
        wxLogTrace(wxEpollDispatcher_Trace,
                   wxT("Registered fd %d for input events"), fd);
    }

    if ( flags & wxFDIO_OUTPUT )
    {
        ep |= EPOLLOUT;
        wxLogTrace(wxEpollDispatcher_Trace,
                   wxT("Registered fd %d for output events"), fd);
    }

    if ( flags & wxFDIO_EXCEPTION )
    {
        ep |= EPOLLERR | EPOLLHUP;
        wxLogTrace(wxEpollDispatcher_Trace,
                   wxT("Registered fd %d for exceptional events"), fd);
    }

    return ep;
}



wxEpollDispatcher *wxEpollDispatcher::Create()
{
    int epollDescriptor = epoll_create(1024);
    if ( epollDescriptor == -1 )
    {
        wxLogSysError(_("Failed to create epoll descriptor"));
        return NULL;
    }
    wxLogTrace(wxEpollDispatcher_Trace,
                   wxT("Epoll fd %d created"), epollDescriptor);
    return new wxEpollDispatcher(epollDescriptor);
}

wxEpollDispatcher::wxEpollDispatcher(int epollDescriptor)
{
    wxASSERT_MSG( epollDescriptor != -1, wxT("invalid descriptor") );

    m_epollDescriptor = epollDescriptor;
}

wxEpollDispatcher::~wxEpollDispatcher()
{
    if ( close(m_epollDescriptor) != 0 )
    {
        wxLogSysError(_("Error closing epoll descriptor"));
    }
}

bool wxEpollDispatcher::RegisterFD(int fd, wxFDIOHandler* handler, int flags)
{
    epoll_event ev;
    ev.events = GetEpollMask(flags, fd);
    ev.data.ptr = handler;

    const int ret = epoll_ctl(m_epollDescriptor, EPOLL_CTL_ADD, fd, &ev);
    if ( ret != 0 )
    {
        wxLogSysError(_("Failed to add descriptor %d to epoll descriptor %d"),
                      fd, m_epollDescriptor);

        return false;
    }
    wxLogTrace(wxEpollDispatcher_Trace,
               wxT("Added fd %d (handler %p) to epoll %d"), fd, handler, m_epollDescriptor);

    return true;
}

bool wxEpollDispatcher::ModifyFD(int fd, wxFDIOHandler* handler, int flags)
{
    epoll_event ev;
    ev.events = GetEpollMask(flags, fd);
    ev.data.ptr = handler;

    const int ret = epoll_ctl(m_epollDescriptor, EPOLL_CTL_MOD, fd, &ev);
    if ( ret != 0 )
    {
        wxLogSysError(_("Failed to modify descriptor %d in epoll descriptor %d"),
                      fd, m_epollDescriptor);

        return false;
    }

    wxLogTrace(wxEpollDispatcher_Trace,
                wxT("Modified fd %d (handler: %p) on epoll %d"), fd, handler, m_epollDescriptor);
    return true;
}

bool wxEpollDispatcher::UnregisterFD(int fd)
{
    epoll_event ev;
    ev.events = 0;
    ev.data.ptr = NULL;

    if ( epoll_ctl(m_epollDescriptor, EPOLL_CTL_DEL, fd, &ev) != 0 )
    {
        wxLogSysError(_("Failed to unregister descriptor %d from epoll descriptor %d"),
                      fd, m_epollDescriptor);
    }
    wxLogTrace(wxEpollDispatcher_Trace,
                wxT("removed fd %d from %d"), fd, m_epollDescriptor);
    return true;
}

int
wxEpollDispatcher::DoPoll(epoll_event *events, int numEvents, int timeout) const
{
                    wxCOMPILE_TIME_ASSERT( TIMEOUT_INFINITE == -1, UpdateThisCode );

    wxMilliClock_t timeEnd;
    if ( timeout > 0 )
        timeEnd = wxGetLocalTimeMillis();

    int rc;
    for ( ;; )
    {
        rc = epoll_wait(m_epollDescriptor, events, numEvents, timeout);
        if ( rc != -1 || errno != EINTR )
            break;

                if ( timeout > 0 )
        {
            timeout = wxMilliClockToLong(timeEnd - wxGetLocalTimeMillis());
            if ( timeout < 0 )
                return 0;
        }
    }

    return rc;
}

bool wxEpollDispatcher::HasPending() const
{
    epoll_event event;

                return DoPoll(&event, 1, 0) >= 1;
}

int wxEpollDispatcher::Dispatch(int timeout)
{
    epoll_event events[16];

    const int rc = DoPoll(events, WXSIZEOF(events), timeout);

    if ( rc == -1 )
    {
        wxLogSysError(_("Waiting for IO on epoll descriptor %d failed"),
                      m_epollDescriptor);
        return -1;
    }

    int numEvents = 0;
    for ( epoll_event *p = events; p < events + rc; p++ )
    {
        wxFDIOHandler * const handler = (wxFDIOHandler *)(p->data.ptr);
        if ( !handler )
        {
            wxFAIL_MSG( wxT("NULL handler in epoll_event?") );
            continue;
        }

                                        if ( p->events & (EPOLLIN | EPOLLHUP) )
            handler->OnReadWaiting();
        else if ( p->events & EPOLLOUT )
            handler->OnWriteWaiting();
        else if ( p->events & EPOLLERR )
            handler->OnExceptionWaiting();
        else
            continue;

        numEvents++;
    }

    return numEvents;
}

#endif 