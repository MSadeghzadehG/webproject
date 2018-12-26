

#include "wx/wxprec.h"

#if wxUSE_SOCKETS

#include "wx/private/fd.h"
#include "wx/private/socket.h"
#include "wx/unix/private/sockunix.h"

#include <errno.h>

#include <sys/types.h>

#ifdef HAVE_SYS_SELECT_H
#   include <sys/select.h>
#endif

#ifndef WX_SOCKLEN_T

#ifdef VMS
#  define WX_SOCKLEN_T unsigned int
#else
#  ifdef __GLIBC__
#    if __GLIBC__ == 2
#      define WX_SOCKLEN_T socklen_t
#    endif
#  elif defined(__WXMAC__)
#    define WX_SOCKLEN_T socklen_t
#  else
#    define WX_SOCKLEN_T int
#  endif
#endif

#endif 

#ifndef SOCKOPTLEN_T
    #define SOCKOPTLEN_T WX_SOCKLEN_T
#endif

#ifdef __UNIXWARE__
    #include <sys/filio.h>
#endif


wxSocketError wxSocketImplUnix::GetLastError() const
{
    switch ( errno )
    {
        case 0:
            return wxSOCKET_NOERROR;

        case ENOTSOCK:
            return wxSOCKET_INVSOCK;

                                                                case EAGAIN:
#ifdef EWOULDBLOCK
    #if EWOULDBLOCK != EAGAIN
        case EWOULDBLOCK:
    #endif
#endif         case EINPROGRESS:
            return wxSOCKET_WOULDBLOCK;

        default:
            return wxSOCKET_IOERR;
    }
}

void wxSocketImplUnix::DoEnableEvents(int flags, bool enable)
{
            if ( GetSocketFlags() & wxSOCKET_BLOCK )
        return;

    wxSocketManager * const manager = wxSocketManager::Get();
    if (!manager)
        return;

    if ( enable )
    {
        if ( flags & wxSOCKET_INPUT_FLAG )
            manager->Install_Callback(this, wxSOCKET_INPUT);
        if ( flags & wxSOCKET_OUTPUT_FLAG )
            manager->Install_Callback(this, wxSOCKET_OUTPUT);
    }
    else     {
        if ( flags & wxSOCKET_INPUT_FLAG )
            manager->Uninstall_Callback(this, wxSOCKET_INPUT);
        if ( flags & wxSOCKET_OUTPUT_FLAG )
            manager->Uninstall_Callback(this, wxSOCKET_OUTPUT);
    }
}

int wxSocketImplUnix::CheckForInput()
{
    char c;
    int rc;
    do
    {
        rc = recv(m_fd, &c, 1, MSG_PEEK);
    } while ( rc == -1 && errno == EINTR );

    return rc;
}

void wxSocketImplUnix::OnStateChange(wxSocketNotify event)
{
    NotifyOnStateChange(event);

    if ( event == wxSOCKET_LOST )
        Shutdown();
}

void wxSocketImplUnix::OnReadWaiting()
{
    wxASSERT_MSG( m_fd != INVALID_SOCKET, "invalid socket ready for reading?" );

                                DisableEvents(wxSOCKET_INPUT_FLAG);


        wxSocketNotify notify;

            if ( m_server && m_stream )
    {
        notify = wxSOCKET_CONNECTION;
    }
    else     {
        switch ( CheckForInput() )
        {
            case 1:
                notify = wxSOCKET_INPUT;
                break;

            case 0:
                                                                notify = m_stream ? wxSOCKET_LOST : wxSOCKET_INPUT;
                break;

            default:
                wxFAIL_MSG( "unexpected CheckForInput() return value" );
                wxFALLTHROUGH;

            case -1:
                if ( GetLastError() == wxSOCKET_WOULDBLOCK )
                {
                                        EnableEvents(wxSOCKET_INPUT_FLAG);
                    return;
                }

                notify = wxSOCKET_LOST;
        }
    }

    OnStateChange(notify);
}

void wxSocketImplUnix::OnWriteWaiting()
{
    wxASSERT_MSG( m_fd != INVALID_SOCKET, "invalid socket ready for writing?" );

        DisableEvents(wxSOCKET_OUTPUT_FLAG);


            if ( m_establishing && !m_server )
    {
        m_establishing = false;

                int error;
        SOCKOPTLEN_T len = sizeof(error);

        getsockopt(m_fd, SOL_SOCKET, SO_ERROR, (char*)&error, &len);

        if ( error )
        {
            OnStateChange(wxSOCKET_LOST);
            return;
        }

        OnStateChange(wxSOCKET_CONNECTION);
    }

    OnStateChange(wxSOCKET_OUTPUT);
}

void wxSocketImplUnix::OnExceptionWaiting()
{
                                if ( m_fd != INVALID_SOCKET )
        OnStateChange(wxSOCKET_LOST);
}

#endif  
