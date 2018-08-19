


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_SOCKETS

#include "wx/app.h"
#include "wx/apptrait.h"
#include "wx/private/socket.h"
#include "wx/link.h"


bool wxSocketFDBasedManager::OnInit()
{
    wxAppTraits * const traits = wxApp::GetTraitsIfExists();
    if ( !traits )
        return false;

    m_fdioManager = traits->GetFDIOManager();
    return m_fdioManager != NULL;
}

void wxSocketFDBasedManager::Install_Callback(wxSocketImpl *socket_,
                                              wxSocketNotify event)
{
    wxSocketImplUnix * const
        socket = static_cast<wxSocketImplUnix *>(socket_);

    wxCHECK_RET( socket->m_fd != -1,
                    "shouldn't be called on invalid socket" );

    const wxFDIOManager::Direction d = GetDirForEvent(socket, event);

    int& fd = FD(socket, d);
    if ( fd != -1 )
        m_fdioManager->RemoveInput(socket, fd, d);

    fd = m_fdioManager->AddInput(socket, socket->m_fd, d);
}

void wxSocketFDBasedManager::Uninstall_Callback(wxSocketImpl *socket_,
                                                wxSocketNotify event)
{
    wxSocketImplUnix * const
        socket = static_cast<wxSocketImplUnix *>(socket_);

    const wxFDIOManager::Direction d = GetDirForEvent(socket, event);

    int& fd = FD(socket, d);
    if ( fd != -1 )
    {
        m_fdioManager->RemoveInput(socket, fd, d);
        fd = -1;
    }
}

wxFDIOManager::Direction
wxSocketFDBasedManager::GetDirForEvent(wxSocketImpl *socket,
                                       wxSocketNotify event)
{
    switch ( event )
    {
        default:
            wxFAIL_MSG( "unknown socket event" );
            return wxFDIOManager::INPUT; 
        case wxSOCKET_LOST:
            wxFAIL_MSG( "unexpected socket event" );
            return wxFDIOManager::INPUT; 
        case wxSOCKET_INPUT:
            return wxFDIOManager::INPUT;

        case wxSOCKET_OUTPUT:
            return wxFDIOManager::OUTPUT;

        case wxSOCKET_CONNECTION:
                                                                                    return socket->IsServer() ? wxFDIOManager::INPUT
                                      : wxFDIOManager::OUTPUT;
    }
}

static struct ManagerSetter
{
    ManagerSetter()
    {
        static wxSocketFDBasedManager s_manager;
        wxAppTraits::SetDefaultSocketManager(&s_manager);
    }
} gs_managerSetter;


wxFORCE_LINK_THIS_MODULE( socketiohandler );

#endif 