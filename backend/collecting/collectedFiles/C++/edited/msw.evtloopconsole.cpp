


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/msw/private.h"
#endif 
#include "wx/evtloop.h"



wxMSWEventLoopBase::wxMSWEventLoopBase()
{
    m_shouldExit = false;
    m_exitcode = 0;

        m_heventWake = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    if ( !m_heventWake )
        wxLogLastError(wxS("CreateEvent(wake)"));
}

wxMSWEventLoopBase::~wxMSWEventLoopBase()
{
    if ( m_heventWake && !::CloseHandle(m_heventWake) )
        wxLogLastError(wxS("CloseHandle(wake)"));
}


bool wxMSWEventLoopBase::Pending() const
{
    MSG msg;
    return ::PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE) != 0;
}

void wxMSWEventLoopBase::WakeUp()
{
    if ( !::SetEvent(m_heventWake) )
        wxLogLastError(wxS("SetEvent(wake)"));
}

#if wxUSE_THREADS

WXDWORD wxMSWEventLoopBase::MSWWaitForThread(WXHANDLE hThread)
{
                HANDLE handles[2] = { hThread, m_heventWake };
    return ::MsgWaitForMultipleObjects
             (
               WXSIZEOF(handles),                  handles,                            false,                              INFINITE,                           QS_ALLINPUT |                       QS_ALLPOSTMESSAGE
             );
}

#endif 
bool wxMSWEventLoopBase::GetNextMessage(WXMSG* msg)
{
    return GetNextMessageTimeout(msg, INFINITE) == TRUE;
}

int wxMSWEventLoopBase::GetNextMessageTimeout(WXMSG *msg, unsigned long timeout)
{
                while ( !::PeekMessage(msg, 0, 0, 0, PM_REMOVE) )
    {
        DWORD rc = ::MsgWaitForMultipleObjects
                     (
                        1, &m_heventWake,
                        FALSE,
                        timeout,
                        QS_ALLINPUT | QS_ALLPOSTMESSAGE
                     );

        switch ( rc )
        {
            default:
                wxLogDebug("unexpected MsgWaitForMultipleObjects() return "
                           "value %lu", rc);
                
            case WAIT_TIMEOUT:
                return -1;

            case WAIT_OBJECT_0:
                                                                                wxZeroMemory(*msg);
                return TRUE;

            case WAIT_OBJECT_0 + 1:
                                                                break;
        }
    }

    return msg->message != WM_QUIT;
}


#if wxUSE_CONSOLE_EVENTLOOP

void wxConsoleEventLoop::ProcessMessage(WXMSG *msg)
{
    ::DispatchMessage(msg);
}

bool wxConsoleEventLoop::Dispatch()
{
    MSG msg;
    if ( !GetNextMessage(&msg) )
        return false;

    ProcessMessage(&msg);

    return !m_shouldExit;
}

int wxConsoleEventLoop::DispatchTimeout(unsigned long timeout)
{
    MSG msg;
    int rc = GetNextMessageTimeout(&msg, timeout);
    if ( rc != 1 )
        return rc;

    ProcessMessage(&msg);

    return !m_shouldExit;
}

void wxConsoleEventLoop::DoYieldFor(long eventsToProcess)
{
    wxEventLoopBase::DoYieldFor(eventsToProcess);
}

#endif 