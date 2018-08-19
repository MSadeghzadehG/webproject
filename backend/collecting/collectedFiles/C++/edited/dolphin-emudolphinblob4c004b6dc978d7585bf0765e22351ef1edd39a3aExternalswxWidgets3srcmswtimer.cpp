
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_TIMER

#include "wx/msw/private/timer.h"

#ifndef WX_PRECOMP
    #include "wx/list.h"
    #include "wx/event.h"
    #include "wx/app.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/hashmap.h"
    #include "wx/module.h"
#endif

#include "wx/msw/private.h"
#include "wx/msw/private/hiddenwin.h"


WX_DECLARE_HASH_MAP(WPARAM, wxMSWTimerImpl *, wxIntegerHash, wxIntegerEqual,
                    wxTimerMap);

static wxTimerMap& TimerMap()
{
    static wxTimerMap s_timerMap;

    return s_timerMap;
}

UINT_PTR GetNewTimerId(wxMSWTimerImpl *t)
{
    static UINT_PTR lastTimerId = 0;

    while (lastTimerId == 0 ||
            TimerMap().find(lastTimerId) != TimerMap().end())
    {
        lastTimerId = lastTimerId + 1;
    }

    TimerMap()[lastTimerId] = t;

    return lastTimerId;
}




LRESULT APIENTRY
wxTimerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


class wxTimerHiddenWindowModule : public wxModule
{
public:
        virtual bool OnInit();
    virtual void OnExit();

        static HWND GetHWND();

private:
        static HWND ms_hwnd;

        static const wxChar *ms_className;

    wxDECLARE_DYNAMIC_CLASS(wxTimerHiddenWindowModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxTimerHiddenWindowModule, wxModule);




bool wxMSWTimerImpl::Start(int milliseconds, bool oneShot)
{
    if ( !wxTimerImpl::Start(milliseconds, oneShot) )
        return false;

    m_id = GetNewTimerId(this);
            UINT_PTR ret = ::SetTimer
             (
              wxTimerHiddenWindowModule::GetHWND(),                m_id,                                                (UINT)m_milli,                                       NULL                                                );

    if ( ret == 0 )
    {
        wxLogSysError(_("Couldn't create a timer"));

        return false;
    }

    return true;
}

void wxMSWTimerImpl::Stop()
{
    ::KillTimer(wxTimerHiddenWindowModule::GetHWND(), m_id);
    TimerMap().erase(m_id);
    m_id = 0;
}


void wxProcessTimer(wxMSWTimerImpl& timer)
{
    wxASSERT_MSG( timer.IsRunning(), wxT("bogus timer id") );

    if ( timer.IsOneShot() )
        timer.Stop();

    timer.Notify();
}


LRESULT APIENTRY
wxTimerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if ( message == WM_TIMER )
    {
        wxTimerMap::iterator node = TimerMap().find(wParam);

        if ( node != TimerMap().end() )
        {
            wxProcessTimer(*(node->second));

            return 0;
        }
                    }

    return ::DefWindowProc(hWnd, message, wParam, lParam);
}



HWND wxTimerHiddenWindowModule::ms_hwnd = NULL;

const wxChar *wxTimerHiddenWindowModule::ms_className = NULL;

bool wxTimerHiddenWindowModule::OnInit()
{
                
    return true;
}

void wxTimerHiddenWindowModule::OnExit()
{
    if ( ms_hwnd )
    {
        if ( !::DestroyWindow(ms_hwnd) )
        {
            wxLogLastError(wxT("DestroyWindow(wxTimerHiddenWindow)"));
        }

        ms_hwnd = NULL;
    }

    if ( ms_className )
    {
        if ( !::UnregisterClass(ms_className, wxGetInstance()) )
        {
            wxLogLastError(wxT("UnregisterClass(\"wxTimerHiddenWindow\")"));
        }

        ms_className = NULL;
    }
}


HWND wxTimerHiddenWindowModule::GetHWND()
{
    static const wxChar *HIDDEN_WINDOW_CLASS = wxT("wxTimerHiddenWindow");
    if ( !ms_hwnd )
    {
        ms_hwnd = wxCreateHiddenWindow(&ms_className, HIDDEN_WINDOW_CLASS,
                                     wxTimerWndProc);
    }

    return ms_hwnd;
}

#endif 