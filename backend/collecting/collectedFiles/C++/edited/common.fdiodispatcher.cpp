


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/module.h"
#endif 
#include "wx/private/fdiodispatcher.h"

#include "wx/private/selectdispatcher.h"
#ifdef __UNIX__
    #include "wx/unix/private/epolldispatcher.h"
#endif

wxFDIODispatcher *gs_dispatcher = NULL;




wxFDIODispatcher *wxFDIODispatcher::Get()
{
    if ( !gs_dispatcher )
    {
#if wxUSE_EPOLL_DISPATCHER
        gs_dispatcher = wxEpollDispatcher::Create();
        if ( !gs_dispatcher )
#endif #if wxUSE_SELECT_DISPATCHER
            gs_dispatcher = new wxSelectDispatcher();
#endif     }

    wxASSERT_MSG( gs_dispatcher, "failed to create any IO dispatchers" );

    return gs_dispatcher;
}


void wxFDIODispatcher::DispatchPending()
{
    if ( gs_dispatcher )
        gs_dispatcher->Dispatch(0);
}


wxFDIOHandler *wxMappedFDIODispatcher::FindHandler(int fd) const
{
    const wxFDIOHandlerMap::const_iterator it = m_handlers.find(fd);

    return it == m_handlers.end() ? NULL : it->second.handler;
}


bool
wxMappedFDIODispatcher::RegisterFD(int fd, wxFDIOHandler *handler, int flags)
{
    wxCHECK_MSG( handler, false, "handler can't be NULL" );

                wxFDIOHandlerMap::iterator i = m_handlers.find(fd);
    if ( i != m_handlers.end() )
    {
        wxASSERT_MSG( i->second.handler == handler,
                        "registering different handler for the same fd?" );
        wxASSERT_MSG( i->second.flags != flags,
                        "reregistering with the same flags?" );
    }

    m_handlers[fd] = wxFDIOHandlerEntry(handler, flags);

    return true;
}

bool
wxMappedFDIODispatcher::ModifyFD(int fd, wxFDIOHandler *handler, int flags)
{
    wxCHECK_MSG( handler, false, "handler can't be NULL" );

    wxFDIOHandlerMap::iterator i = m_handlers.find(fd);
    wxCHECK_MSG( i != m_handlers.end(), false,
                    "modifying unregistered handler?" );

    i->second = wxFDIOHandlerEntry(handler, flags);

    return true;
}

bool wxMappedFDIODispatcher::UnregisterFD(int fd)
{
    wxFDIOHandlerMap::iterator i = m_handlers.find(fd);
    if ( i == m_handlers.end() )
      return false;

    m_handlers.erase(i);

    return true;
}


class wxFDIODispatcherModule : public wxModule
{
public:
    virtual bool OnInit() wxOVERRIDE { return true; }
    virtual void OnExit() wxOVERRIDE { wxDELETE(gs_dispatcher); }

private:
    wxDECLARE_DYNAMIC_CLASS(wxFDIODispatcherModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxFDIODispatcherModule, wxModule);
