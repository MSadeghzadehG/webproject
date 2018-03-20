


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_SELECT_DISPATCHER

#include "wx/private/selectdispatcher.h"
#include "wx/unix/private.h"

#ifndef WX_PRECOMP
    #include "wx/hash.h"
    #include "wx/log.h"
    #include "wx/intl.h"
#endif

#include <errno.h>

#define wxSelectDispatcher_Trace wxT("selectdispatcher")



int wxSelectSets::ms_flags[wxSelectSets::Max] =
{
    wxFDIO_INPUT,
    wxFDIO_OUTPUT,
    wxFDIO_EXCEPTION,
};

const char *wxSelectSets::ms_names[wxSelectSets::Max] =
{
    "input",
    "output",
    "exceptional",
};

wxSelectSets::Callback wxSelectSets::ms_handlers[wxSelectSets::Max] =
{
    &wxFDIOHandler::OnReadWaiting,
    &wxFDIOHandler::OnWriteWaiting,
    &wxFDIOHandler::OnExceptionWaiting,
};

wxSelectSets::wxSelectSets()
{
    for ( int n = 0; n < Max; n++ )
    {
        wxFD_ZERO(&m_fds[n]);
    }
}

bool wxSelectSets::HasFD(int fd) const
{
    for ( int n = 0; n < Max; n++ )
    {
        if ( wxFD_ISSET(fd, (fd_set*) &m_fds[n]) )
            return true;
    }

    return false;
}

bool wxSelectSets::SetFD(int fd, int flags)
{
    wxCHECK_MSG( fd >= 0, false, wxT("invalid descriptor") );

    for ( int n = 0; n < Max; n++ )
    {
        if ( flags & ms_flags[n] )
        {
            wxFD_SET(fd, &m_fds[n]);
        }
        else if ( wxFD_ISSET(fd,  (fd_set*) &m_fds[n]) )
        {
            wxFD_CLR(fd, &m_fds[n]);
        }
    }

    return true;
}

int wxSelectSets::Select(int nfds, struct timeval *tv)
{
    return select(nfds, &m_fds[Read], &m_fds[Write], &m_fds[Except], tv);
}

bool wxSelectSets::Handle(int fd, wxFDIOHandler& handler) const
{
    for ( int n = 0; n < Max; n++ )
    {
        if ( wxFD_ISSET(fd, (fd_set*) &m_fds[n]) )
        {
            wxLogTrace(wxSelectDispatcher_Trace,
                       wxT("Got %s event on fd %d"), ms_names[n], fd);
            (handler.*ms_handlers[n])();
                                    return true;
        }
    }

    return false;
}


bool wxSelectDispatcher::RegisterFD(int fd, wxFDIOHandler *handler, int flags)
{
    wxCRIT_SECT_LOCKER(lock, m_cs);

    if ( !wxMappedFDIODispatcher::RegisterFD(fd, handler, flags) )
        return false;

    if ( !m_sets.SetFD(fd, flags) )
       return false;

    if ( fd > m_maxFD )
      m_maxFD = fd;

    wxLogTrace(wxSelectDispatcher_Trace,
                wxT("Registered fd %d: input:%d, output:%d, exceptional:%d"), fd, (flags & wxFDIO_INPUT) == wxFDIO_INPUT, (flags & wxFDIO_OUTPUT), (flags & wxFDIO_EXCEPTION) == wxFDIO_EXCEPTION);
    return true;
}

bool wxSelectDispatcher::ModifyFD(int fd, wxFDIOHandler *handler, int flags)
{
    wxCRIT_SECT_LOCKER(lock, m_cs);

    if ( !wxMappedFDIODispatcher::ModifyFD(fd, handler, flags) )
        return false;

    wxASSERT_MSG( fd <= m_maxFD, wxT("logic error: registered fd > m_maxFD?") );

    wxLogTrace(wxSelectDispatcher_Trace,
                wxT("Modified fd %d: input:%d, output:%d, exceptional:%d"), fd, (flags & wxFDIO_INPUT) == wxFDIO_INPUT, (flags & wxFDIO_OUTPUT) == wxFDIO_OUTPUT, (flags & wxFDIO_EXCEPTION) == wxFDIO_EXCEPTION);
    return m_sets.SetFD(fd, flags);
}

bool wxSelectDispatcher::UnregisterFD(int fd)
{
    wxCRIT_SECT_LOCKER(lock, m_cs);

    m_sets.ClearFD(fd);

    if ( !wxMappedFDIODispatcher::UnregisterFD(fd) )
        return false;

        if ( !m_sets.HasFD(fd) )
    {
        if ( fd == m_maxFD )
        {
                        m_maxFD = -1;
            for ( wxFDIOHandlerMap::const_iterator it = m_handlers.begin();
                  it != m_handlers.end();
                  ++it )
            {
                if ( it->first > m_maxFD )
                {
                    m_maxFD = it->first;
                }
            }
        }
    }

    wxLogTrace(wxSelectDispatcher_Trace,
                wxT("Removed fd %d, current max: %d"), fd, m_maxFD);
    return true;
}

int wxSelectDispatcher::ProcessSets(const wxSelectSets& sets)
{
    int numEvents = 0;
    for ( int fd = 0; fd <= m_maxFD; fd++ )
    {
        if ( !sets.HasFD(fd) )
            continue;

        wxFDIOHandler * const handler = FindHandler(fd);
        if ( !handler )
        {
            wxFAIL_MSG( wxT("NULL handler in wxSelectDispatcher?") );
            continue;
        }

        if ( sets.Handle(fd, *handler) )
            numEvents++;
    }

    return numEvents;
}

int wxSelectDispatcher::DoSelect(wxSelectSets& sets, int timeout) const
{
    struct timeval tv,
                  *ptv;
    if ( timeout != TIMEOUT_INFINITE )
    {
        ptv = &tv;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000)*1000;
    }
    else     {
        ptv = NULL;
    }

    int ret = sets.Select(m_maxFD + 1, ptv);

            if ( ret == -1 && errno == EINTR )
        ret = 0;

    return ret;
}

bool wxSelectDispatcher::HasPending() const
{
    wxSelectSets sets(m_sets);
    return DoSelect(sets, 0) > 0;
}

int wxSelectDispatcher::Dispatch(int timeout)
{
    wxSelectSets sets(m_sets);
    switch ( DoSelect(sets, timeout) )
    {
        case -1:
            wxLogSysError(_("Failed to monitor I/O channels"));
            return -1;

        case 0:
                        return 0;

        default:
            return ProcessSets(sets);
    }
}

#endif 