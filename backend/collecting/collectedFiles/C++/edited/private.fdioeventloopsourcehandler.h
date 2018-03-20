
#ifndef _WX_PRIVATE_FDIO_EVENT_LOOP_SOURCE_HANDLER_H
#define _WX_PRIVATE_FDIO_EVENT_LOOP_SOURCE_HANDLER_H

#include "wx/evtloopsrc.h"

class wxFDIOEventLoopSourceHandler : public wxFDIOHandler
{
public:
    wxEXPLICIT wxFDIOEventLoopSourceHandler(wxEventLoopSourceHandler* handler)
        : m_handler(handler)
    {
    }

        virtual void OnReadWaiting() wxOVERRIDE { m_handler->OnReadWaiting(); }
    virtual void OnWriteWaiting() wxOVERRIDE { m_handler->OnWriteWaiting(); }
    virtual void OnExceptionWaiting() wxOVERRIDE { m_handler->OnExceptionWaiting(); }

protected:
    wxEventLoopSourceHandler* const m_handler;

    wxDECLARE_NO_COPY_CLASS(wxFDIOEventLoopSourceHandler);
};

#endif 