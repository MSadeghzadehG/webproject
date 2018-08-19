

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_SOCKETS


#ifdef _MSC_VER
#   pragma warning(disable:4115) 
#endif

#include "wx/private/socket.h"
#include "wx/msw/private.h"     #include "wx/private/fd.h"
#include "wx/apptrait.h"
#include "wx/thread.h"
#include "wx/dynlib.h"
#include "wx/link.h"

#ifdef _MSC_VER
#  pragma warning(default:4115) 
#endif

#include "wx/msw/private/hiddenwin.h"

#define CLASSNAME  TEXT("_wxSocket_Internal_Window_Class")


#define MAXSOCKETS 1024

#if (MAXSOCKETS > (0x7FFF - WM_USER + 1))
#error "MAXSOCKETS is too big!"
#endif

typedef int (PASCAL *WSAAsyncSelect_t)(SOCKET,HWND,u_int,long);

LRESULT CALLBACK wxSocket_Internal_WinProc(HWND, UINT, WPARAM, LPARAM);



static HWND hWin;
wxCRIT_SECT_DECLARE_MEMBER(gs_critical);
static wxSocketImplMSW *socketList[MAXSOCKETS];
static int firstAvailable;

static WSAAsyncSelect_t gs_WSAAsyncSelect = NULL;


class wxSocketMSWManager : public wxSocketManager
{
public:
    virtual bool OnInit();
    virtual void OnExit();

    virtual wxSocketImpl *CreateSocket(wxSocketBase& wxsocket)
    {
        return new wxSocketImplMSW(wxsocket);
    }
    virtual void Install_Callback(wxSocketImpl *socket,
                                  wxSocketNotify event = wxSOCKET_LOST);
    virtual void Uninstall_Callback(wxSocketImpl *socket,
                                    wxSocketNotify event = wxSOCKET_LOST);

private:
    static wxDynamicLibrary gs_wsock32dll;
};

wxDynamicLibrary wxSocketMSWManager::gs_wsock32dll;

bool wxSocketMSWManager::OnInit()
{
  LPCTSTR pclassname = NULL;
  int i;

  
  hWin = wxCreateHiddenWindow(&pclassname, CLASSNAME, wxSocket_Internal_WinProc);
  if (!hWin)
      return false;

  
  for (i = 0; i < MAXSOCKETS; i++)
  {
    socketList[i] = NULL;
  }
  firstAvailable = 0;

          #define WINSOCK_DLL_NAME wxT("wsock32.dll")

    gs_wsock32dll.Load(WINSOCK_DLL_NAME, wxDL_VERBATIM | wxDL_QUIET);
    if ( !gs_wsock32dll.IsLoaded() )
        return false;

    wxDL_INIT_FUNC(gs_, WSAAsyncSelect, gs_wsock32dll);
    if ( !gs_WSAAsyncSelect )
        return false;

    WSADATA wsaData;
  return WSAStartup((1 << 8) | 1, &wsaData) == 0;
}

void wxSocketMSWManager::OnExit()
{
  
  DestroyWindow(hWin);
  UnregisterClass(CLASSNAME, wxGetInstance());

  WSACleanup();

  gs_wsock32dll.Unload();
}



wxSocketImplMSW::wxSocketImplMSW(wxSocketBase& wxsocket)
    : wxSocketImpl(wxsocket)
{
  
  wxCRIT_SECT_LOCKER(lock, gs_critical);

  int i = firstAvailable;
  while (socketList[i] != NULL)
  {
    i = (i + 1) % MAXSOCKETS;

    if (i == firstAvailable)    
    {
      m_msgnumber = 0;       return;
    }
  }
  socketList[i] = this;
  firstAvailable = (i + 1) % MAXSOCKETS;
  m_msgnumber = (i + WM_USER);
}

wxSocketImplMSW::~wxSocketImplMSW()
{
  
  wxCRIT_SECT_LOCKER(lock, gs_critical);

  if ( m_msgnumber )
  {
                        MSG msg;
      while ( ::PeekMessage(&msg, hWin, m_msgnumber, m_msgnumber, PM_REMOVE) )
          ;

      socketList[m_msgnumber - WM_USER] = NULL;
  }
  }



LRESULT CALLBACK wxSocket_Internal_WinProc(HWND hWnd,
                                           UINT uMsg,
                                           WPARAM wParam,
                                           LPARAM lParam)
{
    if ( uMsg < WM_USER || uMsg > (WM_USER + MAXSOCKETS - 1))
        return DefWindowProc(hWnd, uMsg, wParam, lParam);

    wxSocketImplMSW *socket;
    wxSocketNotify event = (wxSocketNotify)-1;
    {
        wxCRIT_SECT_LOCKER(lock, gs_critical);

        socket = socketList[(uMsg - WM_USER)];
        if ( !socket )
            return 0;

                        if ( socket->m_fd == INVALID_SOCKET )
            return 0;

        wxASSERT_MSG( socket->m_fd == (SOCKET)wParam,
                      "mismatch between message and socket?" );

        switch ( WSAGETSELECTEVENT(lParam) )
        {
            case FD_READ:
                                                                                                {
                    fd_set fds;
                    timeval tv = { 0, 0 };

                    wxFD_ZERO(&fds);
                    wxFD_SET(socket->m_fd, &fds);

                    if ( select(socket->m_fd + 1, &fds, NULL, NULL, &tv) != 1 )
                        return 0;
                }

                event = wxSOCKET_INPUT;
                break;

            case FD_WRITE:
                event = wxSOCKET_OUTPUT;
                break;

            case FD_ACCEPT:
                event = wxSOCKET_CONNECTION;
                break;

            case FD_CONNECT:
                event = WSAGETSELECTERROR(lParam) ? wxSOCKET_LOST
                                                  : wxSOCKET_CONNECTION;
                break;

            case FD_CLOSE:
                event = wxSOCKET_LOST;
                break;

            default:
                wxFAIL_MSG( "unexpected socket notification" );
                return 0;
        }
    } 
    socket->NotifyOnStateChange(event);

    return 0;
}


void wxSocketMSWManager::Install_Callback(wxSocketImpl *socket_,
                                         wxSocketNotify WXUNUSED(event))
{
    wxSocketImplMSW * const socket = static_cast<wxSocketImplMSW *>(socket_);

  if (socket->m_fd != INVALID_SOCKET)
  {
    
    long lEvent = socket->m_server?
                  FD_ACCEPT : (FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE);
    gs_WSAAsyncSelect(socket->m_fd, hWin, socket->m_msgnumber, lEvent);
  }
}


void wxSocketMSWManager::Uninstall_Callback(wxSocketImpl *socket_,
                                            wxSocketNotify WXUNUSED(event))
{
    wxSocketImplMSW * const socket = static_cast<wxSocketImplMSW *>(socket_);

  if (socket->m_fd != INVALID_SOCKET)
  {
    gs_WSAAsyncSelect(socket->m_fd, hWin, socket->m_msgnumber, 0);
  }
}

static struct ManagerSetter
{
    ManagerSetter()
    {
        static wxSocketMSWManager s_manager;
        wxAppTraits::SetDefaultSocketManager(&s_manager);
    }
} gs_managerSetter;

wxFORCE_LINK_THIS_MODULE( mswsocket );


void wxSocketImplMSW::DoClose()
{
    wxSocketManager::Get()->Uninstall_Callback(this);

    closesocket(m_fd);
}

wxSocketError wxSocketImplMSW::GetLastError() const
{
    switch ( WSAGetLastError() )
    {
        case 0:
            return wxSOCKET_NOERROR;

        case WSAENOTSOCK:
            return wxSOCKET_INVSOCK;

        case WSAEWOULDBLOCK:
            return wxSOCKET_WOULDBLOCK;

        default:
            return wxSOCKET_IOERR;
    }
}

#endif  