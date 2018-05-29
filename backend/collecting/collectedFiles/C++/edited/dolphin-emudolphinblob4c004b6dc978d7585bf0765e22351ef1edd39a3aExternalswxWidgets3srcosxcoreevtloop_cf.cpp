


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/evtloop.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/app.h"
#endif

#include "wx/evtloopsrc.h"

#include "wx/scopedptr.h"

#include "wx/osx/private.h"
#include "wx/osx/core/cfref.h"
#include "wx/thread.h"

#if wxUSE_GUI
    #include "wx/nonownedwnd.h"
#endif

#include <CoreFoundation/CFSocket.h>


#if wxUSE_EVENTLOOP_SOURCE

void wxCFEventLoopSource::InitSourceSocket(CFSocketRef cfSocket)
{
    wxASSERT_MSG( !m_cfSocket, "shouldn't be called more than once" );

    m_cfSocket = cfSocket;
}

wxCFEventLoopSource::~wxCFEventLoopSource()
{
    if ( m_cfSocket )
    {
        CFSocketInvalidate(m_cfSocket);
        CFRelease(m_cfSocket);
    }
}

#endif 
void wxCFEventLoop::OSXCommonModeObserverCallBack(CFRunLoopObserverRef observer, int activity, void *info)
{
    wxCFEventLoop * eventloop = static_cast<wxCFEventLoop *>(info);
    if ( eventloop && eventloop->IsRunning() )
        eventloop->CommonModeObserverCallBack(observer, activity);
}

void wxCFEventLoop::OSXDefaultModeObserverCallBack(CFRunLoopObserverRef observer, int activity, void *info)
{
    wxCFEventLoop * eventloop = static_cast<wxCFEventLoop *>(info);
    if ( eventloop && eventloop->IsRunning() )
        eventloop->DefaultModeObserverCallBack(observer, activity);
}

void wxCFEventLoop::CommonModeObserverCallBack(CFRunLoopObserverRef WXUNUSED(observer), int activity)
{
    if ( activity & kCFRunLoopBeforeTimers )
    {
                                                        
        if ( wxTheApp && ShouldProcessIdleEvents() )
            wxTheApp->ProcessPendingEvents();
    }

    if ( activity & kCFRunLoopBeforeWaiting )
    {
        if ( ShouldProcessIdleEvents() && ProcessIdle() )
        {
            WakeUp();
        }
        else
        {
#if wxUSE_THREADS
            wxMutexGuiLeaveOrEnter();
#endif
        }
    }
}

void
wxCFEventLoop::DefaultModeObserverCallBack(CFRunLoopObserverRef WXUNUSED(observer),
                                           int WXUNUSED(activity))
{
    
}


wxCFEventLoop::wxCFEventLoop()
{
    m_shouldExit = false;
    m_processIdleEvents = true;

#if wxUSE_UIACTIONSIMULATOR
    m_shouldWaitForEvent = false;
#endif
    
    m_runLoop = CFGetCurrentRunLoop();

    CFRunLoopObserverContext ctxt;
    bzero( &ctxt, sizeof(ctxt) );
    ctxt.info = this;
    m_commonModeRunLoopObserver = CFRunLoopObserverCreate( kCFAllocatorDefault, kCFRunLoopBeforeTimers | kCFRunLoopBeforeWaiting , true , 0,
                                                          (CFRunLoopObserverCallBack) wxCFEventLoop::OSXCommonModeObserverCallBack, &ctxt );
    CFRunLoopAddObserver(m_runLoop, m_commonModeRunLoopObserver, kCFRunLoopCommonModes);

    m_defaultModeRunLoopObserver = CFRunLoopObserverCreate( kCFAllocatorDefault, kCFRunLoopBeforeTimers | kCFRunLoopBeforeWaiting , true , 0,
                                                           (CFRunLoopObserverCallBack) wxCFEventLoop::OSXDefaultModeObserverCallBack, &ctxt );
    CFRunLoopAddObserver(m_runLoop, m_defaultModeRunLoopObserver, kCFRunLoopDefaultMode);
}

wxCFEventLoop::~wxCFEventLoop()
{
    CFRunLoopRemoveObserver(m_runLoop, m_commonModeRunLoopObserver, kCFRunLoopCommonModes);
    CFRunLoopRemoveObserver(m_runLoop, m_defaultModeRunLoopObserver, kCFRunLoopDefaultMode);

    CFRelease(m_defaultModeRunLoopObserver);
    CFRelease(m_commonModeRunLoopObserver);
}


CFRunLoopRef wxCFEventLoop::CFGetCurrentRunLoop() const
{
    return CFRunLoopGetCurrent();
}

void wxCFEventLoop::WakeUp()
{
    CFRunLoopWakeUp(m_runLoop);
}

#if wxUSE_BASE

void wxMacWakeUp()
{
    wxEventLoopBase * const loop = wxEventLoopBase::GetActive();

    if ( loop )
        loop->WakeUp();
}

#endif

void wxCFEventLoop::DoYieldFor(long eventsToProcess)
{
        while ( DoProcessEvents() == 1 )
        ;

    wxEventLoopBase::DoYieldFor(eventsToProcess);
}

bool wxCFEventLoop::Pending() const
{
    return true;
}

int wxCFEventLoop::DoProcessEvents()
{
#if wxUSE_UIACTIONSIMULATOR
    if ( m_shouldWaitForEvent )
    {
        int  handled = DispatchTimeout( 1000 );
        wxASSERT_MSG( handled == 1, "No Event Available");
        m_shouldWaitForEvent = false;
        return handled;
    }
    else
#endif
        return DispatchTimeout( IsYielding() ? 0 : 1000 );
}

bool wxCFEventLoop::Dispatch()
{
    return DoProcessEvents() != 0;
}

int wxCFEventLoop::DispatchTimeout(unsigned long timeout)
{
    if ( !wxTheApp )
        return 0;

    int status = DoDispatchTimeout(timeout);

    switch( status )
    {
        case 0:
            break;
        case -1:
            if ( m_shouldExit )
                return 0;

            break;
        case 1:
            break;
    }

    return status;
}

int wxCFEventLoop::DoDispatchTimeout(unsigned long timeout)
{
    SInt32 status = CFRunLoopRunInMode(kCFRunLoopDefaultMode, timeout / 1000.0 , true);
    switch( status )
    {
        case kCFRunLoopRunFinished:
            wxFAIL_MSG( "incorrect run loop state" );
            break;
        case kCFRunLoopRunStopped:
            return 0;
            break;
        case kCFRunLoopRunTimedOut:
            return -1;
            break;
        case kCFRunLoopRunHandledSource:
        default:
            break;
    }
    return 1;
}

void wxCFEventLoop::OSXDoRun()
{
    for ( ;; )
    {
        OnNextIteration();

                        DoProcessEvents();

                                if ( m_shouldExit )
        {
            while ( DoProcessEvents() == 1 )
                ;

            break;
        }
    }
}

void wxCFEventLoop::OSXDoStop()
{
    CFRunLoopStop(CFGetCurrentRunLoop());
}

int wxCFEventLoop::DoRun()
{
                    #if wxUSE_EXCEPTIONS
    for ( ;; )
    {
        try
        {
#endif 
            OSXDoRun();

#if wxUSE_EXCEPTIONS
                        break;
        }
        catch ( ... )
        {
            try
            {
                if ( !wxTheApp || !wxTheApp->OnExceptionInMainLoop() )
                {
                    OnExit();
                    break;
                }
                            }
            catch ( ... )
            {
                                                                OnExit();
                throw;
            }
        }
    }
#endif 
    return m_exitcode;
}

void wxCFEventLoop::ScheduleExit(int rc)
{
    m_exitcode = rc;
    m_shouldExit = true;
    OSXDoStop();
}

wxCFEventLoopPauseIdleEvents::wxCFEventLoopPauseIdleEvents()
{
    wxCFEventLoop* cfl = dynamic_cast<wxCFEventLoop*>(wxEventLoopBase::GetActive());
    if ( cfl )
    {
        m_formerState = cfl->ShouldProcessIdleEvents();
        cfl->SetProcessIdleEvents(false);
    }
    else
        m_formerState = true;
}

wxCFEventLoopPauseIdleEvents::~wxCFEventLoopPauseIdleEvents()
{
    wxCFEventLoop* cfl = dynamic_cast<wxCFEventLoop*>(wxEventLoopBase::GetActive());
    if ( cfl )
        cfl->SetProcessIdleEvents(m_formerState);
}


#if wxUSE_THREADS


static bool gs_bGuiOwnedByMainThread = true;

static wxCriticalSection *gs_critsectGui = NULL;

static wxCriticalSection *gs_critsectWaitingForGui = NULL;

static size_t gs_nWaitingForGui = 0;

void wxOSXThreadModuleOnInit()
{
    gs_critsectWaitingForGui = new wxCriticalSection();
    gs_critsectGui = new wxCriticalSection();
    gs_critsectGui->Enter();
}


void wxOSXThreadModuleOnExit()
{
    if ( gs_critsectGui )
    {
        if ( !wxGuiOwnedByMainThread() )
        {
            gs_critsectGui->Enter();
            gs_bGuiOwnedByMainThread = true;
        }

        gs_critsectGui->Leave();
        wxDELETE(gs_critsectGui);
    }

    wxDELETE(gs_critsectWaitingForGui);
}


void WXDLLIMPEXP_BASE wxWakeUpMainThread()
{
    wxMacWakeUp();
}

void wxMutexGuiEnterImpl()
{
        wxASSERT_MSG( !wxThread::IsMain(),
                 wxT("main thread doesn't want to block in wxMutexGuiEnter()!") );

    
        {
        wxCriticalSectionLocker enter(*gs_critsectWaitingForGui);

        gs_nWaitingForGui++;
    }

    wxWakeUpMainThread();

            gs_critsectGui->Enter();
}

void wxMutexGuiLeaveImpl()
{
    wxCriticalSectionLocker enter(*gs_critsectWaitingForGui);

    if ( wxThread::IsMain() )
    {
        gs_bGuiOwnedByMainThread = false;
    }
    else
    {
                wxASSERT_MSG( gs_nWaitingForGui > 0,
                     wxT("calling wxMutexGuiLeave() without entering it first?") );

        gs_nWaitingForGui--;

        wxWakeUpMainThread();
    }

    gs_critsectGui->Leave();
}

void WXDLLIMPEXP_BASE wxMutexGuiLeaveOrEnter()
{
    wxASSERT_MSG( wxThread::IsMain(),
                 wxT("only main thread may call wxMutexGuiLeaveOrEnter()!") );

    if ( !gs_critsectWaitingForGui )
        return;

    wxCriticalSectionLocker enter(*gs_critsectWaitingForGui);

    if ( gs_nWaitingForGui == 0 )
    {
                        if ( !wxGuiOwnedByMainThread() )
        {
            gs_critsectGui->Enter();

            gs_bGuiOwnedByMainThread = true;
        }
            }
    else
    {
                if ( wxGuiOwnedByMainThread() )
            wxMutexGuiLeave();
            }
}

bool WXDLLIMPEXP_BASE wxGuiOwnedByMainThread()
{
    return gs_bGuiOwnedByMainThread;
}

#endif
