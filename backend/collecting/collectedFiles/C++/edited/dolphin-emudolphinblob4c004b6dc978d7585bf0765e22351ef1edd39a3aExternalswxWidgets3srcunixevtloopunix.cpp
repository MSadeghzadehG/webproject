


#include "wx/wxprec.h"

#if wxUSE_CONSOLE_EVENTLOOP

#include "wx/evtloop.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/log.h"
#endif

#include "wx/apptrait.h"
#include "wx/scopedptr.h"
#include "wx/thread.h"
#include "wx/module.h"
#include "wx/unix/private/timer.h"
#include "wx/unix/private/epolldispatcher.h"
#include "wx/unix/private/wakeuppipe.h"
#include "wx/private/selectdispatcher.h"
#include "wx/private/eventloopsourcesmanager.h"
#include "wx/private/fdioeventloopsourcehandler.h"
#include "wx/private/eventloopsourcesmanager.h"

#if wxUSE_EVENTLOOP_SOURCE
    #include "wx/evtloopsrc.h"
#endif 


wxConsoleEventLoop::wxConsoleEventLoop()
{
        m_dispatcher = NULL;
    m_wakeupPipe = NULL;
    m_wakeupSource = NULL;

        wxScopedPtr<wxWakeUpPipeMT> wakeupPipe(new wxWakeUpPipeMT);
    const int pipeFD = wakeupPipe->GetReadFd();
    if ( pipeFD == wxPipe::INVALID_FD )
        return;

        m_wakeupSource = wxEventLoopBase::AddSourceForFD
                                      (
                                        pipeFD,
                                        wakeupPipe.get(),
                                        wxFDIO_INPUT
                                      );

    if ( !m_wakeupSource )
        return;

                            m_dispatcher = wxFDIODispatcher::Get();

    m_wakeupPipe = wakeupPipe.release();
}

wxConsoleEventLoop::~wxConsoleEventLoop()
{
    if ( m_wakeupPipe )
    {
        delete m_wakeupSource;

        delete m_wakeupPipe;
    }
}


#if wxUSE_EVENTLOOP_SOURCE

class wxConsoleEventLoopSourcesManager : public wxEventLoopSourcesManagerBase
{
public:
    wxEventLoopSource* AddSourceForFD( int fd,
                                       wxEventLoopSourceHandler *handler,
                                       int flags) wxOVERRIDE
    {
        wxCHECK_MSG( fd != -1, NULL, "can't monitor invalid fd" );

        wxLogTrace(wxTRACE_EVT_SOURCE,
                    "Adding event loop source for fd=%d", fd);

                                wxScopedPtr<wxFDIOHandler>
            fdioHandler(new wxFDIOEventLoopSourceHandler(handler));

        if ( !wxFDIODispatcher::Get()->RegisterFD(fd, fdioHandler.get(), flags) )
            return NULL;

        return new wxUnixEventLoopSource(wxFDIODispatcher::Get(), fdioHandler.release(),
                                         fd, handler, flags);
    }
};

wxEventLoopSourcesManagerBase* wxAppTraits::GetEventLoopSourcesManager()
{
    static wxConsoleEventLoopSourcesManager s_eventLoopSourcesManager;

    return &s_eventLoopSourcesManager;
}

wxUnixEventLoopSource::~wxUnixEventLoopSource()
{
    wxLogTrace(wxTRACE_EVT_SOURCE,
               "Removing event loop source for fd=%d", m_fd);

    m_dispatcher->UnregisterFD(m_fd);

    delete m_fdioHandler;
}

#endif 

bool wxConsoleEventLoop::Pending() const
{
    if ( m_dispatcher->HasPending() )
        return true;

#if wxUSE_TIMER
    wxUsecClock_t nextTimer;
    if ( wxTimerScheduler::Get().GetNext(&nextTimer) &&
            !wxMilliClockToLong(nextTimer) )
        return true;
#endif 
    return false;
}

bool wxConsoleEventLoop::Dispatch()
{
    DispatchTimeout(static_cast<unsigned long>(
        wxFDIODispatcher::TIMEOUT_INFINITE));

    return true;
}

int wxConsoleEventLoop::DispatchTimeout(unsigned long timeout)
{
#if wxUSE_TIMER
        wxUsecClock_t nextTimer;
    if ( wxTimerScheduler::Get().GetNext(&nextTimer) )
    {
        unsigned long timeUntilNextTimer = wxMilliClockToLong(nextTimer / 1000);
        if ( timeUntilNextTimer < timeout )
            timeout = timeUntilNextTimer;
    }
#endif 
    bool hadEvent = m_dispatcher->Dispatch(timeout) > 0;

#if wxUSE_TIMER
    if ( wxTimerScheduler::Get().NotifyExpired() )
        hadEvent = true;
#endif 
    return hadEvent ? 1 : -1;
}

void wxConsoleEventLoop::WakeUp()
{
#if wxUSE_THREADS
    m_wakeupPipe->WakeUp();
#else
    m_wakeupPipe->WakeUpNoLock();
#endif
}

void wxConsoleEventLoop::OnNextIteration()
{
        wxTheApp->CheckSignal();
}

void wxConsoleEventLoop::DoYieldFor(long eventsToProcess)
{
    wxEventLoopBase::DoYieldFor(eventsToProcess);
}

wxEventLoopBase *wxConsoleAppTraits::CreateEventLoop()
{
    return new wxEventLoop();
}

#endif 