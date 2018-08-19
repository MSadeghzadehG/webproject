
#include "wx/wxprec.h"

#if wxUSE_SOCKETS

#include "wx/private/socket.h"
#include "wx/unix/private/sockunix.h"
#include "wx/apptrait.h"
#include "wx/link.h"

#include "wx/osx/core/cfstring.h"           
#include <CoreFoundation/CoreFoundation.h>

namespace
{


static CFRunLoopRef gs_mainRunLoop = NULL;


class wxSocketImplMac : public wxSocketImplUnix
{
public:
    wxSocketImplMac(wxSocketBase& wxsocket)
        : wxSocketImplUnix(wxsocket)
    {
        m_socket = NULL;
        m_source = NULL;
    }

    virtual ~wxSocketImplMac()
    {
        wxASSERT_MSG( !m_source && !m_socket, "forgot to call Close()?" );
    }

        CFSocketRef GetSocket() 
    {
        if ( !m_socket )
            Initialize();

        return m_socket;
    }

private:
    virtual void DoClose() wxOVERRIDE
    {
        wxSocketManager * const manager = wxSocketManager::Get();
        if ( manager )
        {
            manager->Uninstall_Callback(this, wxSOCKET_INPUT);
            manager->Uninstall_Callback(this, wxSOCKET_OUTPUT);
        }

                                CFRunLoopRemoveSource(gs_mainRunLoop, m_source, kCFRunLoopCommonModes);
        CFSocketInvalidate(m_socket);

        CFRelease(m_source);
        m_source = NULL;

        CFRelease(m_socket);
        m_socket = NULL;
    }

        bool Initialize()
    {
                if ( m_fd < 0 )
            return false;

        CFSocketContext cont;
        cont.version = 0;                       cont.info = this;                       cont.retain = NULL;                     cont.release = NULL;                    cont.copyDescription = NULL;    
        m_socket = CFSocketCreateWithNative
                   (
                        NULL,                                           m_fd,
                        kCFSocketReadCallBack |
                        kCFSocketWriteCallBack |
                        kCFSocketConnectCallBack,
                        SocketCallback,
                        &cont
                   );
        if ( !m_socket )
            return false;

        m_source = CFSocketCreateRunLoopSource(NULL, m_socket, 0);

        if ( !m_source )
        {
            CFRelease(m_socket);
            m_socket = NULL;

            return false;
        }

        CFRunLoopAddSource(gs_mainRunLoop, m_source, kCFRunLoopCommonModes);

        return true;
    }

    static void SocketCallback(CFSocketRef WXUNUSED(s),
                               CFSocketCallBackType callbackType,
                               CFDataRef WXUNUSED(address),
                               const void* data,
                               void* info)
    {
        wxSocketImplMac * const socket = static_cast<wxSocketImplMac *>(info);

        switch (callbackType)
        {
            case kCFSocketConnectCallBack:
                wxASSERT(!socket->IsServer());
                                                                                                                if (data == NULL)
                    socket->OnWriteWaiting();
                break;

            case kCFSocketReadCallBack:
                socket->OnReadWaiting();
                break;

            case kCFSocketWriteCallBack:
                socket->OnWriteWaiting();
                break;

            default:
                wxFAIL_MSG( "unexpected socket callback" );
        }

                                                wxMacWakeUp();
    }

    CFSocketRef m_socket;
    CFRunLoopSourceRef m_source;

    wxDECLARE_NO_COPY_CLASS(wxSocketImplMac);
};

} 


class wxSocketManagerMac : public wxSocketManager
{
public:
    virtual bool OnInit() wxOVERRIDE;
    virtual void OnExit() wxOVERRIDE;

    virtual wxSocketImpl *CreateSocket(wxSocketBase& wxsocket) wxOVERRIDE
    {
        return new wxSocketImplMac(wxsocket);
    }

    virtual void Install_Callback(wxSocketImpl *socket, wxSocketNotify event) wxOVERRIDE;
    virtual void Uninstall_Callback(wxSocketImpl *socket, wxSocketNotify event) wxOVERRIDE;

private:
                    static int GetCFCallback(wxSocketImpl *socket, wxSocketNotify event);
};

bool wxSocketManagerMac::OnInit()
{
        if (gs_mainRunLoop != NULL)
        return true;

                gs_mainRunLoop = CFRunLoopGetCurrent();
    if ( !gs_mainRunLoop )
        return false;

    CFRetain(gs_mainRunLoop);

    return true;
}

void wxSocketManagerMac::OnExit()
{
        CFRelease(gs_mainRunLoop);
    gs_mainRunLoop = NULL;
}


int wxSocketManagerMac::GetCFCallback(wxSocketImpl *socket, wxSocketNotify event)
{
    switch ( event )
    {
        case wxSOCKET_CONNECTION:
            return socket->IsServer() ? kCFSocketReadCallBack
                                      : kCFSocketConnectCallBack;

        case wxSOCKET_INPUT:
            return kCFSocketReadCallBack;

        case wxSOCKET_OUTPUT:
            return kCFSocketWriteCallBack;

        case wxSOCKET_LOST:
            wxFAIL_MSG( "unexpected wxSocketNotify" );
            return 0;

        default:
            wxFAIL_MSG( "unknown wxSocketNotify" );
            return 0;
    }
}

void wxSocketManagerMac::Install_Callback(wxSocketImpl *socket_,
                                          wxSocketNotify event)
{
    wxSocketImplMac * const socket = static_cast<wxSocketImplMac *>(socket_);

    CFSocketEnableCallBacks(socket->GetSocket(), GetCFCallback(socket, event));
}

void wxSocketManagerMac::Uninstall_Callback(wxSocketImpl *socket_,
                                            wxSocketNotify event)
{
    wxSocketImplMac * const socket = static_cast<wxSocketImplMac *>(socket_);

    CFSocketDisableCallBacks(socket->GetSocket(), GetCFCallback(socket, event));
}

extern WXDLLIMPEXP_BASE wxSocketManager *wxOSXSocketManagerCF;

static struct OSXManagerSetter
{
    OSXManagerSetter()
    {
        static wxSocketManagerMac s_manager;
        wxOSXSocketManagerCF = &s_manager;
    }
} gs_OSXManagerSetter;

wxFORCE_LINK_THIS_MODULE(osxsocket)

#endif 