
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/evtloop.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
#endif 
#include "wx/scopeguard.h"
#include "wx/apptrait.h"
#include "wx/private/eventloopsourcesmanager.h"

wxEventLoopBase *wxEventLoopBase::ms_activeLoop = NULL;

wxEventLoopBase::wxEventLoopBase()
{
    m_isInsideRun = false;
    m_shouldExit = false;
    m_yieldLevel = 0;
    m_eventsToProcessInsideYield = wxEVT_CATEGORY_ALL;
}

bool wxEventLoopBase::IsMain() const
{
    if (wxTheApp)
        return wxTheApp->GetMainLoop() == this;
    return false;
}


void wxEventLoopBase::SetActive(wxEventLoopBase* loop)
{
    ms_activeLoop = loop;

    if (wxTheApp)
        wxTheApp->OnEventLoopEnter(loop);
}

int wxEventLoopBase::Run()
{
        wxCHECK_MSG( !IsInsideRun(), -1, wxT("can't reenter a message loop") );

                wxEventLoopActivator activate(this);

            m_shouldExit = false;

        m_isInsideRun = true;
    wxON_BLOCK_EXIT_SET(m_isInsideRun, false);

        return DoRun();
}

void wxEventLoopBase::Exit(int rc)
{
    wxCHECK_RET( IsRunning(), wxS("Use ScheduleExit() on not running loop") );

    ScheduleExit(rc);
}
void wxEventLoopBase::OnExit()
{
    if (wxTheApp)
        wxTheApp->OnEventLoopExit(this);
}

void wxEventLoopBase::WakeUpIdle()
{
    WakeUp();
}

bool wxEventLoopBase::ProcessIdle()
{
    return wxTheApp && wxTheApp->ProcessIdle();
}

bool wxEventLoopBase::Yield(bool onlyIfNeeded)
{
    if ( onlyIfNeeded && IsYielding() )
        return false;

    return YieldFor(wxEVT_CATEGORY_ALL);
}

bool wxEventLoopBase::YieldFor(long eventsToProcess)
{
#if wxUSE_THREADS
    if ( !wxThread::IsMain() )
    {
                return false;
    }
#endif 
        const int yieldLevelOld = m_yieldLevel;
    const long eventsToProcessOld = m_eventsToProcessInsideYield;

    m_yieldLevel++;
    wxON_BLOCK_EXIT_SET(m_yieldLevel, yieldLevelOld);

    m_eventsToProcessInsideYield = eventsToProcess;
    wxON_BLOCK_EXIT_SET(m_eventsToProcessInsideYield, eventsToProcessOld);

#if wxUSE_LOG
            wxLog::Suspend();

        wxON_BLOCK_EXIT0(wxLog::Resume);
#endif

    DoYieldFor(eventsToProcess);

#if wxUSE_EXCEPTIONS
                        if ( wxTheApp )
        wxTheApp->RethrowStoredException();
#endif 
    return true;
}

void wxEventLoopBase::DoYieldFor(long eventsToProcess)
{
                                    if ( eventsToProcess == wxEVT_CATEGORY_ALL )
    {
        if ( wxTheApp )
            wxTheApp->ProcessPendingEvents();

                                        ProcessIdle();
    }
}

#if wxUSE_EVENTLOOP_SOURCE

wxEventLoopSource*
wxEventLoopBase::AddSourceForFD(int fd,
                                wxEventLoopSourceHandler *handler,
                                int flags)
{
#if wxUSE_CONSOLE_EVENTLOOP
        wxEventLoopSourcesManagerBase* const
        manager = wxApp::GetValidTraits().GetEventLoopSourcesManager();
    wxCHECK_MSG( manager, NULL, wxS("Must have wxEventLoopSourcesManager") );

    return manager->AddSourceForFD(fd, handler, flags);
#else     return NULL;
#endif }

#endif #if defined(__WINDOWS__) || defined(__WXDFB__) || ( ( defined(__UNIX__) && !defined(__WXOSX__) ) && wxUSE_BASE)


wxEventLoopManual::wxEventLoopManual()
{
    m_exitcode = 0;
}

bool wxEventLoopManual::ProcessEvents()
{
                                if ( wxTheApp )
    {
        wxTheApp->ProcessPendingEvents();

                                if ( m_shouldExit )
            return false;
    }

    const bool res = Dispatch();

#if wxUSE_EXCEPTIONS
            if ( wxTheApp )
        wxTheApp->RethrowStoredException();
#endif 
    return res;
}

int wxEventLoopManual::DoRun()
{

                    #if wxUSE_EXCEPTIONS
    for ( ;; )
    {
        try
        {
#endif 
                        for ( ;; )
            {
                                OnNextIteration();

                                                while ( !m_shouldExit && !Pending() && ProcessIdle() )
                    ;

                if ( m_shouldExit )

                    break;

                                                                if ( !ProcessEvents() )
                {
                                        break;
                }
            }

                                                                                                            for ( ;; )
            {
                bool hasMoreEvents = false;
                if ( wxTheApp && wxTheApp->HasPendingEvents() )
                {
                    wxTheApp->ProcessPendingEvents();
                    hasMoreEvents = true;
                }

                if ( Pending() )
                {
                    Dispatch();
                    hasMoreEvents = true;
                }

                if ( !hasMoreEvents )
                    break;
            }
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

void wxEventLoopManual::ScheduleExit(int rc)
{
    wxCHECK_RET( IsInsideRun(), wxT("can't call ScheduleExit() if not running") );

    m_exitcode = rc;
    m_shouldExit = true;

    OnExit();

                                    WakeUp();
}

#endif 
