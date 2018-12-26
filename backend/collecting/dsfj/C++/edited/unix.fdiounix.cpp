


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_SOCKETS

#include "wx/apptrait.h"
#include "wx/log.h"
#include "wx/private/fdiodispatcher.h"
#include "wx/unix/private/fdiounix.h"


int wxFDIOManagerUnix::AddInput(wxFDIOHandler *handler, int fd, Direction d)
{
    wxFDIODispatcher * const dispatcher = wxFDIODispatcher::Get();
    wxCHECK_MSG( dispatcher, -1, "can't monitor FDs without FD IO dispatcher" );

        const int flag = d == INPUT ? wxFDIO_INPUT : wxFDIO_OUTPUT;

                bool ok;
    const int regmask = handler->GetRegisteredEvents();
    if ( !regmask )
    {
        ok = dispatcher->RegisterFD(fd, handler, flag);
    }
    else
    {
        ok = dispatcher->ModifyFD(fd, handler, regmask | flag);
    }

    if ( !ok )
        return -1;

        handler->SetRegisteredEvent(flag);

    return fd;
}

void wxFDIOManagerUnix::RemoveInput(wxFDIOHandler *handler, int fd, Direction d)
{
    wxFDIODispatcher * const dispatcher = wxFDIODispatcher::Get();
    if ( !dispatcher )
        return;

    const int flag = d == INPUT ? wxFDIO_INPUT : wxFDIO_OUTPUT;

            bool ok;
    const int regmask = handler->GetRegisteredEvents();
    if ( regmask == flag )
    {
        ok = dispatcher->UnregisterFD(fd);
    }
    else
    {
        ok = dispatcher->ModifyFD(fd, handler, regmask & ~flag);
    }

    if ( !ok )
    {
        wxLogDebug("Failed to unregister %d in direction %d", fd, d);
    }

        handler->ClearRegisteredEvent(flag);
}

wxFDIOManager *wxAppTraits::GetFDIOManager()
{
    static wxFDIOManagerUnix s_manager;
    return &s_manager;
}

#endif 