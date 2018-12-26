

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_SOCKETS

#include "wx/socket.h"

#ifndef WX_PRECOMP
    #include "wx/object.h"
    #include "wx/string.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/event.h"
    #include "wx/app.h"
    #include "wx/utils.h"
    #include "wx/timer.h"
    #include "wx/module.h"
    #include "wx/filefn.h"
#endif

#include "wx/apptrait.h"
#include "wx/sckaddr.h"
#include "wx/scopeguard.h"
#include "wx/stopwatch.h"
#include "wx/thread.h"
#include "wx/evtloop.h"
#include "wx/link.h"

#include "wx/private/fd.h"
#include "wx/private/socket.h"

#ifdef __UNIX__
    #include <errno.h>
#endif

#ifdef MSG_NOSIGNAL
    #define wxSOCKET_MSG_NOSIGNAL MSG_NOSIGNAL
#else                     #if defined(__UNIX__) && !defined(SO_NOSIGPIPE)
    extern "C" { typedef void (*wxSigHandler_t)(int); }
    namespace
    {
        class IgnoreSignal
        {
        public:
                        IgnoreSignal(int sig)
                : m_handler(signal(sig, SIG_IGN)),
                  m_sig(sig)
            {
            }

                        ~IgnoreSignal()
            {
                signal(m_sig, m_handler);
            }

        private:
            const wxSigHandler_t m_handler;
            const int m_sig;

            wxDECLARE_NO_COPY_CLASS(IgnoreSignal);
        };
    } 
    #define wxNEEDS_IGNORE_SIGPIPE
    #endif 
    #define wxSOCKET_MSG_NOSIGNAL 0
#endif

#include "wx/build.h"
WX_CHECK_BUILD_OPTIONS("wxNet")


wxDEFINE_EVENT(wxEVT_SOCKET, wxSocketEvent);

#define MAX_DISCARD_SIZE (10 * 1024)

#define wxTRACE_Socket wxT("wxSocket")


wxIMPLEMENT_CLASS(wxSocketBase, wxObject);
wxIMPLEMENT_CLASS(wxSocketServer, wxSocketBase);
wxIMPLEMENT_CLASS(wxSocketClient, wxSocketBase);
wxIMPLEMENT_CLASS(wxDatagramSocket, wxSocketBase);
wxIMPLEMENT_DYNAMIC_CLASS(wxSocketEvent, wxEvent);


namespace
{

void SetTimeValFromMS(timeval& tv, unsigned long ms)
{
    tv.tv_sec  = (ms / 1000);
    tv.tv_usec = (ms % 1000) * 1000;
}

} 

class wxSocketState : public wxObject
{
public:
    wxSocketFlags            m_flags;
    wxSocketEventFlags       m_eventmask;
    bool                     m_notify;
    void                    *m_clientData;

public:
    wxSocketState() : wxObject() {}

    wxDECLARE_NO_COPY_CLASS(wxSocketState);
};

class wxSocketWaitModeChanger
{
public:
            wxSocketWaitModeChanger(wxSocketBase *socket, int flag)
        : m_socket(socket),
          m_oldflags(socket->GetFlags())

    {
                        wxASSERT_MSG( !(flag & wxSOCKET_WAITALL) || !(flag & wxSOCKET_NOWAIT),
                      "not a wait flag" );

                                if ( flag != wxSOCKET_NOWAIT )
            flag |= m_oldflags & wxSOCKET_BLOCK;

        socket->SetFlags(flag);
    }

    ~wxSocketWaitModeChanger()
    {
        m_socket->SetFlags(m_oldflags);
    }

private:
    wxSocketBase * const m_socket;
    const int m_oldflags;

    wxDECLARE_NO_COPY_CLASS(wxSocketWaitModeChanger);
};

class wxSocketReadGuard
{
public:
    wxSocketReadGuard(wxSocketBase *socket)
        : m_socket(socket)
    {
        wxASSERT_MSG( !m_socket->m_reading, "read reentrancy?" );

        m_socket->m_reading = true;
    }

    ~wxSocketReadGuard()
    {
        m_socket->m_reading = false;

                        wxSocketImpl * const impl = m_socket->m_impl;
        if ( impl && impl->m_fd != INVALID_SOCKET )
            impl->ReenableEvents(wxSOCKET_INPUT_FLAG);
    }

private:
    wxSocketBase * const m_socket;

    wxDECLARE_NO_COPY_CLASS(wxSocketReadGuard);
};

class wxSocketWriteGuard
{
public:
    wxSocketWriteGuard(wxSocketBase *socket)
        : m_socket(socket)
    {
        wxASSERT_MSG( !m_socket->m_writing, "write reentrancy?" );

        m_socket->m_writing = true;
    }

    ~wxSocketWriteGuard()
    {
        m_socket->m_writing = false;

        wxSocketImpl * const impl = m_socket->m_impl;
        if ( impl && impl->m_fd != INVALID_SOCKET )
            impl->ReenableEvents(wxSOCKET_OUTPUT_FLAG);
    }

private:
    wxSocketBase * const m_socket;

    wxDECLARE_NO_COPY_CLASS(wxSocketWriteGuard);
};


wxSocketManager *wxSocketManager::ms_manager = NULL;


void wxSocketManager::Set(wxSocketManager *manager)
{
    wxASSERT_MSG( !ms_manager, "too late to set manager now" );

    ms_manager = manager;
}


void wxSocketManager::Init()
{
    wxASSERT_MSG( !ms_manager, "shouldn't be initialized twice" );

    
    wxASSERT_MSG( wxIsMainThread(),
                    "sockets must be initialized from the main thread" );

    wxAppConsole * const app = wxAppConsole::GetInstance();
    wxCHECK_RET( app, "sockets can't be initialized without wxApp" );

    ms_manager = app->GetTraits()->GetSocketManager();
}


wxSocketImpl::wxSocketImpl(wxSocketBase& wxsocket)
    : m_wxsocket(&wxsocket)
{
    m_fd              = INVALID_SOCKET;
    m_error           = wxSOCKET_NOERROR;
    m_server          = false;
    m_stream          = true;

    SetTimeout(wxsocket.GetTimeout() * 1000);

    m_establishing    = false;
    m_reusable        = false;
    m_broadcast       = false;
    m_dobind          = true;
    m_initialRecvBufferSize = -1;
    m_initialSendBufferSize = -1;
}

wxSocketImpl::~wxSocketImpl()
{
    if ( m_fd != INVALID_SOCKET )
        Shutdown();
}

bool wxSocketImpl::PreCreateCheck(const wxSockAddressImpl& addr)
{
    if ( m_fd != INVALID_SOCKET )
    {
        m_error = wxSOCKET_INVSOCK;
        return false;
    }

    if ( !addr.IsOk() )
    {
        m_error = wxSOCKET_INVADDR;
        return false;
    }

    return true;
}

void wxSocketImpl::PostCreation()
{
    #ifdef SO_NOSIGPIPE
    EnableSocketOption(SO_NOSIGPIPE);
#endif

    if ( m_reusable )
        EnableSocketOption(SO_REUSEADDR);

    if ( m_broadcast )
    {
        wxASSERT_MSG( !m_stream, "broadcasting is for datagram sockets only" );

        EnableSocketOption(SO_BROADCAST);
    }

    if ( m_initialRecvBufferSize >= 0 )
        SetSocketOption(SO_RCVBUF, m_initialRecvBufferSize);
    if ( m_initialSendBufferSize >= 0 )
        SetSocketOption(SO_SNDBUF, m_initialSendBufferSize);

            UnblockAndRegisterWithEventLoop();
}

wxSocketError wxSocketImpl::UpdateLocalAddress()
{
    if ( !m_local.IsOk() )
    {
                                m_local.Create(m_peer.GetFamily());
    }

    WX_SOCKLEN_T lenAddr = m_local.GetLen();
    if ( getsockname(m_fd, m_local.GetWritableAddr(), &lenAddr) != 0 )
    {
        Close();
        m_error = wxSOCKET_IOERR;
        return m_error;
    }

    return wxSOCKET_NOERROR;
}

wxSocketError wxSocketImpl::CreateServer()
{
    if ( !PreCreateCheck(m_local) )
        return m_error;

    m_server = true;
    m_stream = true;

        m_fd = socket(m_local.GetFamily(), SOCK_STREAM, 0);

    if ( m_fd == INVALID_SOCKET )
    {
        m_error = wxSOCKET_IOERR;
        return wxSOCKET_IOERR;
    }

    PostCreation();

                if ( bind(m_fd, m_local.GetAddr(), m_local.GetLen()) != 0 )
        m_error = wxSOCKET_IOERR;

    if ( IsOk() )
    {
        if ( listen(m_fd, 5) != 0 )
            m_error = wxSOCKET_IOERR;
    }

    if ( !IsOk() )
    {
        Close();
        return m_error;
    }

        return UpdateLocalAddress();
}

wxSocketError wxSocketImpl::CreateClient(bool wait)
{
    if ( !PreCreateCheck(m_peer) )
        return m_error;

    m_fd = socket(m_peer.GetFamily(), SOCK_STREAM, 0);

    if ( m_fd == INVALID_SOCKET )
    {
        m_error = wxSOCKET_IOERR;
        return wxSOCKET_IOERR;
    }

    PostCreation();

        if ( m_local.IsOk() )
    {
        if ( bind(m_fd, m_local.GetAddr(), m_local.GetLen()) != 0 )
        {
            Close();
            m_error = wxSOCKET_IOERR;
            return m_error;
        }
    }

        int rc = connect(m_fd, m_peer.GetAddr(), m_peer.GetLen());
    if ( rc == SOCKET_ERROR )
    {
        wxSocketError err = GetLastError();
        if ( err == wxSOCKET_WOULDBLOCK )
        {
            m_establishing = true;

                                    if ( wait )
            {
                err = SelectWithTimeout(wxSOCKET_CONNECTION_FLAG)
                        ? wxSOCKET_NOERROR
                        : wxSOCKET_TIMEDOUT;
                m_establishing = false;
            }
        }

        m_error = err;
    }
    else     {
        m_error = wxSOCKET_NOERROR;
    }

    return m_error;
}


wxSocketError wxSocketImpl::CreateUDP()
{
    if ( !PreCreateCheck(m_local) )
        return m_error;

    m_stream = false;
    m_server = false;

    m_fd = socket(m_local.GetFamily(), SOCK_DGRAM, 0);

    if ( m_fd == INVALID_SOCKET )
    {
        m_error = wxSOCKET_IOERR;
        return wxSOCKET_IOERR;
    }

    PostCreation();

    if ( m_dobind )
    {
        if ( bind(m_fd, m_local.GetAddr(), m_local.GetLen()) != 0 )
        {
            Close();
            m_error = wxSOCKET_IOERR;
            return m_error;
        }

        return UpdateLocalAddress();
    }

    return wxSOCKET_NOERROR;
}

wxSocketImpl *wxSocketImpl::Accept(wxSocketBase& wxsocket)
{
    wxSockAddressStorage from;
    WX_SOCKLEN_T fromlen = sizeof(from);
    const wxSOCKET_T fd = accept(m_fd, &from.addr, &fromlen);

    wxScopeGuard closeSocket = wxMakeGuard(wxClose, fd);

            ReenableEvents(wxSOCKET_INPUT_FLAG);

    if ( fd == INVALID_SOCKET )
        return NULL;

    wxSocketManager * const manager = wxSocketManager::Get();
    if ( !manager )
        return NULL;

    wxSocketImpl * const sock = manager->CreateSocket(wxsocket);
    if ( !sock )
        return NULL;

        closeSocket.Dismiss();
    sock->m_fd = fd;
    sock->m_peer = wxSockAddressImpl(from.addr, fromlen);

    sock->UnblockAndRegisterWithEventLoop();

    return sock;
}


void wxSocketImpl::Close()
{
    if ( m_fd != INVALID_SOCKET )
    {
        DoClose();
        m_fd = INVALID_SOCKET;
    }
}

void wxSocketImpl::Shutdown()
{
    if ( m_fd != INVALID_SOCKET )
    {
        shutdown(m_fd, 1 );
        Close();
    }
}


void wxSocketImpl::SetTimeout(unsigned long millis)
{
    SetTimeValFromMS(m_timeout, millis);
}

void wxSocketImpl::NotifyOnStateChange(wxSocketNotify event)
{
    m_wxsocket->OnRequest(event);
}


wxSocketError wxSocketImpl::SetLocal(const wxSockAddressImpl& local)
{
    
    if (m_fd != INVALID_SOCKET && !m_server)
    {
        m_error = wxSOCKET_INVSOCK;
        return wxSOCKET_INVSOCK;
    }

    if ( !local.IsOk() )
    {
        m_error = wxSOCKET_INVADDR;
        return wxSOCKET_INVADDR;
    }

    m_local = local;

    return wxSOCKET_NOERROR;
}

wxSocketError wxSocketImpl::SetPeer(const wxSockAddressImpl& peer)
{
    if ( !peer.IsOk() )
    {
        m_error = wxSOCKET_INVADDR;
        return wxSOCKET_INVADDR;
    }

    m_peer = peer;

    return wxSOCKET_NOERROR;
}

const wxSockAddressImpl& wxSocketImpl::GetLocal()
{
    if ( !m_local.IsOk() )
        UpdateLocalAddress();

    return m_local;
}


#ifdef __UNIX__
    #define DO_WHILE_EINTR( rc, syscall ) \
        do { \
            rc = (syscall); \
        } \
        while ( rc == -1 && errno == EINTR )
#else
    #define DO_WHILE_EINTR( rc, syscall ) rc = (syscall)
#endif

int wxSocketImpl::RecvStream(void *buffer, int size)
{
    int ret;
    DO_WHILE_EINTR( ret, recv(m_fd, static_cast<char *>(buffer), size, 0) );

    if ( !ret )
    {
                                m_establishing = false;
        NotifyOnStateChange(wxSOCKET_LOST);

        Shutdown();

            }

    return ret;
}

int wxSocketImpl::SendStream(const void *buffer, int size)
{
#ifdef wxNEEDS_IGNORE_SIGPIPE
    IgnoreSignal ignore(SIGPIPE);
#endif

    int ret;
    DO_WHILE_EINTR( ret, send(m_fd, static_cast<const char *>(buffer), size,
                              wxSOCKET_MSG_NOSIGNAL) );

    return ret;
}

int wxSocketImpl::RecvDgram(void *buffer, int size)
{
    wxSockAddressStorage from;
    WX_SOCKLEN_T fromlen = sizeof(from);

    int ret;
    DO_WHILE_EINTR( ret, recvfrom(m_fd, static_cast<char *>(buffer), size,
                                  0, &from.addr, &fromlen) );

    if ( ret == SOCKET_ERROR )
        return SOCKET_ERROR;

    m_peer = wxSockAddressImpl(from.addr, fromlen);
    if ( !m_peer.IsOk() )
        return -1;

    return ret;
}

int wxSocketImpl::SendDgram(const void *buffer, int size)
{
    if ( !m_peer.IsOk() )
    {
        m_error = wxSOCKET_INVADDR;
        return -1;
    }

    int ret;
    DO_WHILE_EINTR( ret, sendto(m_fd, static_cast<const char *>(buffer), size,
                                0, m_peer.GetAddr(), m_peer.GetLen()) );

    return ret;
}

int wxSocketImpl::Read(void *buffer, int size)
{
        if ( m_fd == INVALID_SOCKET || m_server )
    {
        m_error = wxSOCKET_INVSOCK;
        return -1;
    }

    int ret = m_stream ? RecvStream(buffer, size)
                       : RecvDgram(buffer, size);

    m_error = ret == SOCKET_ERROR ? GetLastError() : wxSOCKET_NOERROR;

    return ret;
}

int wxSocketImpl::Write(const void *buffer, int size)
{
    if ( m_fd == INVALID_SOCKET || m_server )
    {
        m_error = wxSOCKET_INVSOCK;
        return -1;
    }

    int ret = m_stream ? SendStream(buffer, size)
                       : SendDgram(buffer, size);

    m_error = ret == SOCKET_ERROR ? GetLastError() : wxSOCKET_NOERROR;

    return ret;
}



namespace
{

int gs_socketInitCount = 0;

} 
bool wxSocketBase::IsInitialized()
{
    wxASSERT_MSG( wxIsMainThread(), "unsafe to call from other threads" );

    return gs_socketInitCount != 0;
}

bool wxSocketBase::Initialize()
{
    wxCHECK_MSG( wxIsMainThread(), false,
                 "must be called from the main thread" );

    if ( !gs_socketInitCount )
    {
        wxSocketManager * const manager = wxSocketManager::Get();
        if ( !manager || !manager->OnInit() )
            return false;
    }

    gs_socketInitCount++;

    return true;
}

void wxSocketBase::Shutdown()
{
    wxCHECK_RET( wxIsMainThread(), "must be called from the main thread" );

    wxCHECK_RET( gs_socketInitCount > 0, "too many calls to Shutdown()" );

    if ( !--gs_socketInitCount )
    {
        wxSocketManager * const manager = wxSocketManager::Get();
        wxCHECK_RET( manager, "should have a socket manager" );

        manager->OnExit();
    }
}


void wxSocketBase::Init()
{
    m_impl         = NULL;
    m_type         = wxSOCKET_UNINIT;

        m_flags        = 0;
    m_connected    =
    m_establishing =
    m_reading      =
    m_writing      =
    m_closed       = false;
    m_lcount       = 0;
    m_lcount_read  = 0;
    m_lcount_write = 0;
    m_timeout      = 600;
    m_beingDeleted = false;

        m_unread       = NULL;
    m_unrd_size    = 0;
    m_unrd_cur     = 0;

        m_id           = wxID_ANY;
    m_handler      = NULL;
    m_clientData   = NULL;
    m_notify       = false;
    m_eventmask    =
    m_eventsgot    = 0;

                    if ( wxIsMainThread() )
    {
        if ( !Initialize() )
        {
            wxLogError(_("Cannot initialize sockets"));
        }
    }
}

wxSocketBase::wxSocketBase()
{
    Init();
}

wxSocketBase::wxSocketBase(wxSocketFlags flags, wxSocketType type)
{
    Init();

    SetFlags(flags);

    m_type = type;
}

wxSocketBase::~wxSocketBase()
{
        if (!m_beingDeleted)
        Close();

        delete m_impl;

        free(m_unread);
}

bool wxSocketBase::Destroy()
{
                m_beingDeleted = true;

        Close();

        Notify(false);

                            if ( wxIsMainThread() && wxTheApp )
    {
        wxTheApp->ScheduleForDestruction(this);
    }
    else     {
                delete this;
    }

    return true;
}


void wxSocketBase::SetError(wxSocketError error)
{
    m_impl->m_error = error;
}

wxSocketError wxSocketBase::LastError() const
{
    return m_impl->GetError();
}


bool wxSocketBase::Close()
{
        InterruptWait();

    ShutdownOutput();

    m_connected = false;
    m_establishing = false;
    return true;
}

void wxSocketBase::ShutdownOutput()
{
    if ( m_impl )
        m_impl->Shutdown();
}

wxSocketBase& wxSocketBase::Read(void* buffer, wxUint32 nbytes)
{
    wxSocketReadGuard read(this);

    m_lcount_read = DoRead(buffer, nbytes);
    m_lcount = m_lcount_read;

    return *this;
}

wxUint32 wxSocketBase::DoRead(void* buffer_, wxUint32 nbytes)
{
    wxCHECK_MSG( m_impl, 0, "socket must be valid" );

        char *buffer = static_cast<char *>(buffer_);
    wxCHECK_MSG( buffer, 0, "NULL buffer" );

                wxUint32 total = GetPushback(buffer, nbytes, false);
    nbytes -= total;
    buffer += total;

    while ( nbytes )
    {
                                                        const int ret = !m_impl->m_stream || m_connected
                            ? m_impl->Read(buffer, nbytes)
                            : 0;
        if ( ret == -1 )
        {
            if ( m_impl->GetLastError() == wxSOCKET_WOULDBLOCK )
            {
                                if ( m_flags & wxSOCKET_NOWAIT_READ )
                {
                                        SetError(wxSOCKET_NOERROR);
                    break;
                }

                                                if ( !DoWaitWithTimeout(wxSOCKET_INPUT_FLAG) )
                {
                                        SetError(wxSOCKET_TIMEDOUT);
                    break;
                }

                                continue;
            }
            else             {
                SetError(wxSOCKET_IOERR);
                break;
            }
        }
        else if ( ret == 0 )
        {
                                                            m_closed = true;

                                                if ( (m_flags & wxSOCKET_WAITALL_READ) || !total )
                SetError(wxSOCKET_IOERR);
            break;
        }

        total += ret;

                        if ( !(m_flags & wxSOCKET_WAITALL_READ) )
            break;

        nbytes -= ret;
        buffer += ret;
    }

    return total;
}

wxSocketBase& wxSocketBase::ReadMsg(void* buffer, wxUint32 nbytes)
{
    struct
    {
        unsigned char sig[4];
        unsigned char len[4];
    } msg;

    wxSocketReadGuard read(this);

    wxSocketWaitModeChanger changeFlags(this, wxSOCKET_WAITALL_READ);

    bool ok = false;
    if ( DoRead(&msg, sizeof(msg)) == sizeof(msg) )
    {
        wxUint32 sig = (wxUint32)msg.sig[0];
        sig |= (wxUint32)(msg.sig[1] << 8);
        sig |= (wxUint32)(msg.sig[2] << 16);
        sig |= (wxUint32)(msg.sig[3] << 24);

        if ( sig == 0xfeeddead )
        {
            wxUint32 len = (wxUint32)msg.len[0];
            len |= (wxUint32)(msg.len[1] << 8);
            len |= (wxUint32)(msg.len[2] << 16);
            len |= (wxUint32)(msg.len[3] << 24);

            wxUint32 len2;
            if (len > nbytes)
            {
                len2 = len - nbytes;
                len = nbytes;
            }
            else
                len2 = 0;

                        m_lcount_read = len ? DoRead(buffer, len) : 0;
            m_lcount = m_lcount_read;

            if ( len2 )
            {
                char discard_buffer[MAX_DISCARD_SIZE];
                long discard_len;

                                do
                {
                    discard_len = len2 > MAX_DISCARD_SIZE
                                    ? MAX_DISCARD_SIZE
                                    : len2;
                    discard_len = DoRead(discard_buffer, (wxUint32)discard_len);
                    len2 -= (wxUint32)discard_len;
                }
                while ((discard_len > 0) && len2);
            }

            if ( !len2 && DoRead(&msg, sizeof(msg)) == sizeof(msg) )
            {
                sig = (wxUint32)msg.sig[0];
                sig |= (wxUint32)(msg.sig[1] << 8);
                sig |= (wxUint32)(msg.sig[2] << 16);
                sig |= (wxUint32)(msg.sig[3] << 24);

                if ( sig == 0xdeadfeed )
                    ok = true;
            }
        }
    }

    if ( !ok )
        SetError(wxSOCKET_IOERR);

    return *this;
}

wxSocketBase& wxSocketBase::Peek(void* buffer, wxUint32 nbytes)
{
    wxSocketReadGuard read(this);

        wxSocketWaitModeChanger changeFlags(this, wxSOCKET_NOWAIT);

    m_lcount = DoRead(buffer, nbytes);

    Pushback(buffer, m_lcount);

    return *this;
}

wxSocketBase& wxSocketBase::Write(const void *buffer, wxUint32 nbytes)
{
    wxSocketWriteGuard write(this);

    m_lcount_write = DoWrite(buffer, nbytes);
    m_lcount = m_lcount_write;

    return *this;
}

wxUint32 wxSocketBase::DoWrite(const void *buffer_, wxUint32 nbytes)
{
    wxCHECK_MSG( m_impl, 0, "socket must be valid" );

    const char *buffer = static_cast<const char *>(buffer_);
    wxCHECK_MSG( buffer, 0, "NULL buffer" );

    wxUint32 total = 0;
    while ( nbytes )
    {
        if ( m_impl->m_stream && !m_connected )
        {
            if ( (m_flags & wxSOCKET_WAITALL_WRITE) || !total )
                SetError(wxSOCKET_IOERR);
            break;
        }

        const int ret = m_impl->Write(buffer, nbytes);
        if ( ret == -1 )
        {
            if ( m_impl->GetLastError() == wxSOCKET_WOULDBLOCK )
            {
                if ( m_flags & wxSOCKET_NOWAIT_WRITE )
                    break;

                if ( !DoWaitWithTimeout(wxSOCKET_OUTPUT_FLAG) )
                {
                    SetError(wxSOCKET_TIMEDOUT);
                    break;
                }

                continue;
            }
            else             {
                SetError(wxSOCKET_IOERR);
                break;
            }
        }

        total += ret;

        if ( !(m_flags & wxSOCKET_WAITALL_WRITE) )
            break;

        nbytes -= ret;
        buffer += ret;
    }

    return total;
}

wxSocketBase& wxSocketBase::WriteMsg(const void *buffer, wxUint32 nbytes)
{
    struct
    {
        unsigned char sig[4];
        unsigned char len[4];
    } msg;

    wxSocketWriteGuard write(this);

    wxSocketWaitModeChanger changeFlags(this, wxSOCKET_WAITALL_WRITE);

    msg.sig[0] = (unsigned char) 0xad;
    msg.sig[1] = (unsigned char) 0xde;
    msg.sig[2] = (unsigned char) 0xed;
    msg.sig[3] = (unsigned char) 0xfe;

    msg.len[0] = (unsigned char) (nbytes & 0xff);
    msg.len[1] = (unsigned char) ((nbytes >> 8) & 0xff);
    msg.len[2] = (unsigned char) ((nbytes >> 16) & 0xff);
    msg.len[3] = (unsigned char) ((nbytes >> 24) & 0xff);

    bool ok = false;
    if ( DoWrite(&msg, sizeof(msg)) == sizeof(msg) )
    {
        m_lcount_write = DoWrite(buffer, nbytes);
        m_lcount = m_lcount_write;
        if ( m_lcount_write == nbytes )
        {
            msg.sig[0] = (unsigned char) 0xed;
            msg.sig[1] = (unsigned char) 0xfe;
            msg.sig[2] = (unsigned char) 0xad;
            msg.sig[3] = (unsigned char) 0xde;
            msg.len[0] =
            msg.len[1] =
            msg.len[2] =
            msg.len[3] = (char) 0;

            if ( DoWrite(&msg, sizeof(msg)) == sizeof(msg))
                ok = true;
        }
    }

    if ( !ok )
        SetError(wxSOCKET_IOERR);

    return *this;
}

wxSocketBase& wxSocketBase::Unread(const void *buffer, wxUint32 nbytes)
{
    if (nbytes != 0)
        Pushback(buffer, nbytes);

    SetError(wxSOCKET_NOERROR);
    m_lcount = nbytes;

    return *this;
}

wxSocketBase& wxSocketBase::Discard()
{
    char *buffer = new char[MAX_DISCARD_SIZE];
    wxUint32 ret;
    wxUint32 total = 0;

    wxSocketReadGuard read(this);

    wxSocketWaitModeChanger changeFlags(this, wxSOCKET_NOWAIT);

    do
    {
        ret = DoRead(buffer, MAX_DISCARD_SIZE);
        total += ret;
    }
    while (ret == MAX_DISCARD_SIZE);

    delete[] buffer;
    m_lcount = total;
    SetError(wxSOCKET_NOERROR);

    return *this;
}



wxSocketEventFlags wxSocketImpl::Select(wxSocketEventFlags flags,
                                        const timeval *timeout)
{
    if ( m_fd == INVALID_SOCKET )
        return (wxSOCKET_LOST_FLAG & flags);

    struct timeval tv;
    if ( timeout )
        tv = *timeout;
    else
        tv.tv_sec = tv.tv_usec = 0;

        fd_set
        readfds, *preadfds = NULL,
        writefds, *pwritefds = NULL,
        exceptfds;                      
    if ( flags & wxSOCKET_INPUT_FLAG )
        preadfds = &readfds;

    if ( flags & wxSOCKET_OUTPUT_FLAG )
        pwritefds = &writefds;

                    if ( flags & wxSOCKET_CONNECTION_FLAG )
    {
        if ( m_server )
            preadfds = &readfds;
        else
            pwritefds = &writefds;
    }

    if ( preadfds )
    {
        wxFD_ZERO(preadfds);
        wxFD_SET(m_fd, preadfds);
    }

    if ( pwritefds )
    {
        wxFD_ZERO(pwritefds);
        wxFD_SET(m_fd, pwritefds);
    }

    wxFD_ZERO(&exceptfds);
    wxFD_SET(m_fd, &exceptfds);

    const int rc = select(m_fd + 1, preadfds, pwritefds, &exceptfds, &tv);

        if ( rc == -1 || wxFD_ISSET(m_fd, &exceptfds) )
    {
        m_establishing = false;

        return wxSOCKET_LOST_FLAG & flags;
    }

    if ( rc == 0 )
        return 0;

    wxASSERT_MSG( rc == 1, "unexpected select() return value" );

    wxSocketEventFlags detected = 0;
    if ( preadfds && wxFD_ISSET(m_fd, preadfds) )
    {
                if ( m_server && (flags & wxSOCKET_CONNECTION_FLAG) )
        {
            int error;
            SOCKOPTLEN_T len = sizeof(error);
            m_establishing = false;
            getsockopt(m_fd, SOL_SOCKET, SO_ERROR, (char*)&error, &len);

            if ( error )
                detected = wxSOCKET_LOST_FLAG;
            else
                detected |= wxSOCKET_CONNECTION_FLAG;
        }
        else         {
            detected |= wxSOCKET_INPUT_FLAG;
        }
    }

    if ( pwritefds && wxFD_ISSET(m_fd, pwritefds) )
    {
                if ( m_establishing && !m_server )
        {
            int error;
            SOCKOPTLEN_T len = sizeof(error);
            m_establishing = false;
            getsockopt(m_fd, SOL_SOCKET, SO_ERROR, (char*)&error, &len);

            if ( error )
                detected = wxSOCKET_LOST_FLAG;
            else
                detected |= wxSOCKET_CONNECTION_FLAG;
        }
        else         {
            detected |= wxSOCKET_OUTPUT_FLAG;
        }
    }

    return detected & flags;
}

int
wxSocketBase::DoWait(long seconds, long milliseconds, wxSocketEventFlags flags)
{
                    const long timeout = seconds == -1 ? m_timeout * 1000
                                       : seconds * 1000 + milliseconds;

    return DoWait(timeout, flags);
}

int
wxSocketBase::DoWait(long timeout, wxSocketEventFlags flags)
{
    wxCHECK_MSG( m_impl, -1, "can't wait on invalid socket" );

                if ( !m_impl->IsServer() &&
            m_impl->m_stream && !m_connected && !m_establishing )
        return -1;

        m_interrupt = false;


    const wxMilliClock_t timeEnd = wxGetLocalTimeMillis() + timeout;

                wxEventLoopBase *eventLoop;
    if ( !(m_flags & wxSOCKET_BLOCK) && wxIsMainThread() )
    {
        eventLoop = wxEventLoop::GetActive();
    }
    else     {
                eventLoop = NULL;
    }

                                                m_impl->ReenableEvents(flags & (wxSOCKET_INPUT_FLAG | wxSOCKET_OUTPUT_FLAG));


                int rc = 0;
    for ( bool firstTime = true; !m_interrupt; firstTime = false )
    {
        long timeLeft = wxMilliClockToLong(timeEnd - wxGetLocalTimeMillis());
        if ( timeLeft < 0 )
        {
            if ( !firstTime )
                break;   
            timeLeft = 0;
        }

        wxSocketEventFlags events;
        if ( eventLoop )
        {
                        m_eventsgot = 0;

            eventLoop->DispatchTimeout(timeLeft);

            events = m_eventsgot;
        }
        else         {
                        timeval tv;
            SetTimeValFromMS(tv, timeLeft);
            events = m_impl->Select(flags | wxSOCKET_LOST_FLAG, &tv);
        }

                                if ( events & wxSOCKET_LOST_FLAG )
        {
            m_connected = false;
            m_establishing = false;
            rc = -1;
            break;
        }

                events &= flags;

                if ( events & wxSOCKET_CONNECTION_FLAG )
        {
            m_connected = true;
            m_establishing = false;
            rc = true;
            break;
        }

                if ( (events & wxSOCKET_INPUT_FLAG) || (events & wxSOCKET_OUTPUT_FLAG) )
        {
            rc = true;
            break;
        }
    }

    return rc;
}

bool wxSocketBase::Wait(long seconds, long milliseconds)
{
    return DoWait(seconds, milliseconds,
                  wxSOCKET_INPUT_FLAG |
                  wxSOCKET_OUTPUT_FLAG |
                  wxSOCKET_CONNECTION_FLAG) != 0;
}

bool wxSocketBase::WaitForRead(long seconds, long milliseconds)
{
        if ( m_unread )
        return true;

                    if ( m_impl->Select(wxSOCKET_INPUT_FLAG) )
        return true;

    return DoWait(seconds, milliseconds, wxSOCKET_INPUT_FLAG) != 0;
}


bool wxSocketBase::WaitForWrite(long seconds, long milliseconds)
{
    if ( m_impl->Select(wxSOCKET_OUTPUT_FLAG) )
        return true;

    return DoWait(seconds, milliseconds, wxSOCKET_OUTPUT_FLAG) != 0;
}

bool wxSocketBase::WaitForLost(long seconds, long milliseconds)
{
    return DoWait(seconds, milliseconds, wxSOCKET_LOST_FLAG) == -1;
}



bool wxSocketBase::GetPeer(wxSockAddress& addr) const
{
    wxCHECK_MSG( m_impl, false, "invalid socket" );

    const wxSockAddressImpl& peer = m_impl->GetPeer();
    if ( !peer.IsOk() )
        return false;

    addr.SetAddress(peer);

    return true;
}

bool wxSocketBase::GetLocal(wxSockAddress& addr) const
{
    wxCHECK_MSG( m_impl, false, "invalid socket" );

    const wxSockAddressImpl& local = m_impl->GetLocal();
    if ( !local.IsOk() )
        return false;

    addr.SetAddress(local);

    return true;
}


void wxSocketBase::SaveState()
{
    wxSocketState *state;

    state = new wxSocketState();

    state->m_flags      = m_flags;
    state->m_notify     = m_notify;
    state->m_eventmask  = m_eventmask;
    state->m_clientData = m_clientData;

    m_states.Append(state);
}

void wxSocketBase::RestoreState()
{
    wxList::compatibility_iterator node;
    wxSocketState *state;

    node = m_states.GetLast();
    if (!node)
        return;

    state = (wxSocketState *)node->GetData();

    m_flags      = state->m_flags;
    m_notify     = state->m_notify;
    m_eventmask  = state->m_eventmask;
    m_clientData = state->m_clientData;

    m_states.Erase(node);
    delete state;
}


void wxSocketBase::SetTimeout(long seconds)
{
    m_timeout = seconds;

    if (m_impl)
        m_impl->SetTimeout(m_timeout * 1000);
}

void wxSocketBase::SetFlags(wxSocketFlags flags)
{
            wxASSERT_MSG( !(flags & wxSOCKET_NOWAIT) ||
                  !(flags & (wxSOCKET_WAITALL | wxSOCKET_BLOCK)),
                  "Using wxSOCKET_WAITALL or wxSOCKET_BLOCK with "
                  "wxSOCKET_NOWAIT doesn't make sense" );

    m_flags = flags;
}



void wxSocketBase::OnRequest(wxSocketNotify notification)
{
    wxSocketEventFlags flag = 0;
    switch ( notification )
    {
        case wxSOCKET_INPUT:
            flag = wxSOCKET_INPUT_FLAG;
            break;

        case wxSOCKET_OUTPUT:
            flag = wxSOCKET_OUTPUT_FLAG;
            break;

        case wxSOCKET_CONNECTION:
            flag = wxSOCKET_CONNECTION_FLAG;

                        m_connected = true;
            m_establishing = false;

                                    SetError(wxSOCKET_NOERROR);
            break;

        case wxSOCKET_LOST:
            flag = wxSOCKET_LOST_FLAG;

                                    m_connected = false;
            m_closed = true;
            break;

        default:
            wxFAIL_MSG( "unknown wxSocket notification" );
    }

            m_eventsgot |= flag;

        if ( m_notify && (m_eventmask & flag) && m_handler )
    {
                                if ( (notification == wxSOCKET_INPUT && m_reading) ||
                (notification == wxSOCKET_OUTPUT && m_writing) )
        {
            return;
        }

        wxSocketEvent event(m_id);
        event.m_event      = notification;
        event.m_clientData = m_clientData;
        event.SetEventObject(this);

        m_handler->AddPendingEvent(event);
    }
}

void wxSocketBase::Notify(bool notify)
{
    m_notify = notify;
}

void wxSocketBase::SetNotify(wxSocketEventFlags flags)
{
    m_eventmask = flags;
}

void wxSocketBase::SetEventHandler(wxEvtHandler& handler, int id)
{
    m_handler = &handler;
    m_id      = id;
}


void wxSocketBase::Pushback(const void *buffer, wxUint32 size)
{
    if (!size) return;

    if (m_unread == NULL)
        m_unread = malloc(size);
    else
    {
        void *tmp;

        tmp = malloc(m_unrd_size + size);
        memcpy((char *)tmp + size, m_unread, m_unrd_size);
        free(m_unread);

        m_unread = tmp;
    }

    m_unrd_size += size;

    memcpy(m_unread, buffer, size);
}

wxUint32 wxSocketBase::GetPushback(void *buffer, wxUint32 size, bool peek)
{
    wxCHECK_MSG( buffer, 0, "NULL buffer" );

    if (!m_unrd_size)
        return 0;

    if (size > (m_unrd_size-m_unrd_cur))
        size = m_unrd_size-m_unrd_cur;

    memcpy(buffer, (char *)m_unread + m_unrd_cur, size);

    if (!peek)
    {
        m_unrd_cur += size;
        if (m_unrd_size == m_unrd_cur)
        {
            free(m_unread);
            m_unread = NULL;
            m_unrd_size = 0;
            m_unrd_cur  = 0;
        }
    }

    return size;
}




wxSocketServer::wxSocketServer(const wxSockAddress& addr,
                               wxSocketFlags flags)
              : wxSocketBase(flags, wxSOCKET_SERVER)
{
    wxLogTrace( wxTRACE_Socket, wxT("Opening wxSocketServer") );

    wxSocketManager * const manager = wxSocketManager::Get();
    m_impl = manager ? manager->CreateSocket(*this) : NULL;

    if (!m_impl)
    {
        wxLogTrace( wxTRACE_Socket, wxT("*** Failed to create m_impl") );
        return;
    }

        m_impl->SetLocal(addr.GetAddress());

    if (GetFlags() & wxSOCKET_REUSEADDR) {
        m_impl->SetReusable();
    }
    if (GetFlags() & wxSOCKET_BROADCAST) {
        m_impl->SetBroadcast();
    }
    if (GetFlags() & wxSOCKET_NOBIND) {
        m_impl->DontDoBind();
    }

    if (m_impl->CreateServer() != wxSOCKET_NOERROR)
    {
        wxDELETE(m_impl);

        wxLogTrace( wxTRACE_Socket, wxT("*** CreateServer() failed") );
        return;
    }

                wxLogTrace( wxTRACE_Socket, wxT("wxSocketServer on fd %u"),
                static_cast<unsigned>(m_impl->m_fd) );
}


bool wxSocketServer::AcceptWith(wxSocketBase& sock, bool wait)
{
    if ( !m_impl || (m_impl->m_fd == INVALID_SOCKET) || !m_impl->IsServer() )
    {
        wxFAIL_MSG( "can only be called for a valid server socket" );

        SetError(wxSOCKET_INVSOCK);

        return false;
    }

    if ( wait )
    {
                if ( !m_impl->SelectWithTimeout(wxSOCKET_INPUT_FLAG) )
        {
            SetError(wxSOCKET_TIMEDOUT);

            return false;
        }
    }

    sock.m_impl = m_impl->Accept(sock);

    if ( !sock.m_impl )
    {
        SetError(m_impl->GetLastError());

        return false;
    }

    sock.m_type = wxSOCKET_BASE;
    sock.m_connected = true;

    return true;
}

wxSocketBase *wxSocketServer::Accept(bool wait)
{
    wxSocketBase* sock = new wxSocketBase();

    sock->SetFlags(m_flags);

    if (!AcceptWith(*sock, wait))
    {
        sock->Destroy();
        sock = NULL;
    }

    return sock;
}

bool wxSocketServer::WaitForAccept(long seconds, long milliseconds)
{
    return DoWait(seconds, milliseconds, wxSOCKET_CONNECTION_FLAG) == 1;
}

wxSOCKET_T wxSocketBase::GetSocket() const
{
    wxASSERT_MSG( m_impl, wxS("Socket not initialised") );

    return m_impl->m_fd;
}


bool wxSocketBase::GetOption(int level, int optname, void *optval, int *optlen)
{
    wxASSERT_MSG( m_impl, wxT("Socket not initialised") );

    SOCKOPTLEN_T lenreal = *optlen;
    if ( getsockopt(m_impl->m_fd, level, optname,
                    static_cast<char *>(optval), &lenreal) != 0 )
        return false;

    *optlen = lenreal;

    return true;
}

bool
wxSocketBase::SetOption(int level, int optname, const void *optval, int optlen)
{
    wxASSERT_MSG( m_impl, wxT("Socket not initialised") );

    return setsockopt(m_impl->m_fd, level, optname,
                      static_cast<const char *>(optval), optlen) == 0;
}

bool wxSocketBase::SetLocal(const wxIPV4address& local)
{
    m_localAddress = local;

    return true;
}



wxSocketClient::wxSocketClient(wxSocketFlags flags)
              : wxSocketBase(flags, wxSOCKET_CLIENT)
{
    m_initialRecvBufferSize =
    m_initialSendBufferSize = -1;
}


bool wxSocketClient::DoConnect(const wxSockAddress& remote,
                               const wxSockAddress* local,
                               bool wait)
{
    if ( m_impl )
    {
                Close();
        delete m_impl;
    }

    m_connected = false;
    m_establishing = false;

        wxSocketManager * const manager = wxSocketManager::Get();
    m_impl = manager ? manager->CreateSocket(*this) : NULL;
    if ( !m_impl )
        return false;

        if (GetFlags() & wxSOCKET_REUSEADDR)
        m_impl->SetReusable();
    if (GetFlags() & wxSOCKET_BROADCAST)
        m_impl->SetBroadcast();
    if (GetFlags() & wxSOCKET_NOBIND)
        m_impl->DontDoBind();

            if ( !local && m_localAddress.GetAddress().IsOk() )
        local = &m_localAddress;

    if ( local )
        m_impl->SetLocal(local->GetAddress());

    m_impl->SetInitialSocketBuffers(m_initialRecvBufferSize, m_initialSendBufferSize);

    m_impl->SetPeer(remote.GetAddress());

        const wxSocketError err = m_impl->CreateClient(wait);

    if ( err != wxSOCKET_NOERROR )
    {
        if ( err == wxSOCKET_WOULDBLOCK )
        {
            wxASSERT_MSG( !wait, "shouldn't get this for blocking connect" );

            m_establishing = true;
        }

        return false;
    }

    m_connected = true;
    return true;
}

bool wxSocketClient::Connect(const wxSockAddress& remote, bool wait)
{
    return DoConnect(remote, NULL, wait);
}

bool wxSocketClient::Connect(const wxSockAddress& remote,
                             const wxSockAddress& local,
                             bool wait)
{
    return DoConnect(remote, &local, wait);
}

bool wxSocketClient::WaitOnConnect(long seconds, long milliseconds)
{
    if ( m_connected )
    {
                        return true;
    }

    wxCHECK_MSG( m_establishing && m_impl, false,
                 "No connection establishment attempt in progress" );

                return DoWait(seconds, milliseconds, wxSOCKET_CONNECTION_FLAG) != 0;
}


wxDatagramSocket::wxDatagramSocket( const wxSockAddress& addr,
                                    wxSocketFlags flags )
                : wxSocketBase( flags, wxSOCKET_DATAGRAM )
{
        wxSocketManager * const manager = wxSocketManager::Get();
    m_impl = manager ? manager->CreateSocket(*this) : NULL;

    if (!m_impl)
        return;

        m_impl->SetLocal(addr.GetAddress());
    if (flags & wxSOCKET_REUSEADDR)
    {
        m_impl->SetReusable();
    }
    if (GetFlags() & wxSOCKET_BROADCAST)
    {
        m_impl->SetBroadcast();
    }
    if (GetFlags() & wxSOCKET_NOBIND)
    {
        m_impl->DontDoBind();
    }

    if ( m_impl->CreateUDP() != wxSOCKET_NOERROR )
    {
        wxDELETE(m_impl);
        return;
    }

        m_connected = false;
    m_establishing = false;
}

wxDatagramSocket& wxDatagramSocket::RecvFrom( wxSockAddress& addr,
                                              void* buf,
                                              wxUint32 nBytes )
{
    Read(buf, nBytes);
    GetPeer(addr);
    return (*this);
}

wxDatagramSocket& wxDatagramSocket::SendTo( const wxSockAddress& addr,
                                            const void* buf,
                                            wxUint32 nBytes )
{
    wxASSERT_MSG( m_impl, wxT("Socket not initialised") );

    m_impl->SetPeer(addr.GetAddress());
    Write(buf, nBytes);
    return (*this);
}


class wxSocketModule : public wxModule
{
public:
    virtual bool OnInit() wxOVERRIDE
    {
                        return true;
    }

    virtual void OnExit() wxOVERRIDE
    {
        if ( wxSocketBase::IsInitialized() )
            wxSocketBase::Shutdown();
    }

private:
    wxDECLARE_DYNAMIC_CLASS(wxSocketModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxSocketModule, wxModule);

#if defined(wxUSE_SELECT_DISPATCHER) && wxUSE_SELECT_DISPATCHER
wxFORCE_LINK_MODULE( socketiohandler )
#endif

#ifdef __WINDOWS__
    wxFORCE_LINK_MODULE( mswsocket )
#endif

#ifdef __WXOSX__
    wxFORCE_LINK_MODULE( osxsocket )
#endif

#endif 