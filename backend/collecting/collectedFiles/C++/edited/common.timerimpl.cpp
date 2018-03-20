


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_TIMER

#include "wx/private/timer.h"
#include "wx/utils.h"               #include "wx/thread.h"

wxTimerImpl::wxTimerImpl(wxTimer *timer)
{
    m_timer = timer;
    m_owner = NULL;
    m_idTimer = wxID_ANY;
    m_milli = 0;
    m_oneShot = false;
}

void wxTimerImpl::SetOwner(wxEvtHandler *owner, int timerid)
{
    m_owner = owner;
    m_idTimer = timerid == wxID_ANY ? wxNewId() : timerid;
}

void wxTimerImpl::SendEvent()
{
    wxTimerEvent event(*m_timer);
    (void)m_owner->SafelyProcessEvent(event);
}

bool wxTimerImpl::Start(int milliseconds, bool oneShot)
{
        #if wxUSE_THREADS
    wxASSERT_MSG( wxThread::IsMain(),
                  wxT("timer can only be started from the main thread") );
#endif 
    if ( IsRunning() )
    {
                                Stop();
    }

    if ( milliseconds != -1 )
    {
        m_milli = milliseconds;
    }

    m_oneShot = oneShot;

    return true;
}


#endif 
