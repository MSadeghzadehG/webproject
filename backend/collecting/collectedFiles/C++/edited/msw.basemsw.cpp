


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/event.h"
#endif 
#include "wx/apptrait.h"
#include "wx/evtloop.h"
#include "wx/msw/private/timer.h"
#include "wx/recguard.h"

#include "wx/crt.h"
#include "wx/msw/private.h"


#if wxUSE_THREADS
WXDWORD wxAppTraits::DoSimpleWaitForThread(WXHANDLE hThread)
{
    return ::WaitForSingleObject((HANDLE)hThread, INFINITE);
}
#endif 

void *wxConsoleAppTraits::BeforeChildWaitLoop()
{
        return NULL;
}

void wxConsoleAppTraits::AfterChildWaitLoop(void * WXUNUSED(data))
{
    }

#if wxUSE_THREADS
bool wxConsoleAppTraits::DoMessageFromThreadWait()
{
        return true;
}

WXDWORD wxConsoleAppTraits::WaitForThread(WXHANDLE hThread, int WXUNUSED(flags))
{
    return DoSimpleWaitForThread(hThread);
}
#endif 
#if wxUSE_TIMER

wxTimerImpl *wxConsoleAppTraits::CreateTimerImpl(wxTimer *timer)
{
    return new wxMSWTimerImpl(timer);
}

#endif 
wxEventLoopBase *wxConsoleAppTraits::CreateEventLoop()
{
#if wxUSE_CONSOLE_EVENTLOOP
    return new wxEventLoop();
#else     return NULL;
#endif }


bool wxConsoleAppTraits::WriteToStderr(const wxString& text)
{
    return wxFprintf(stderr, "%s", text) != -1;
}
