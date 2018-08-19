

#include "wx/wxprec.h"

#if defined(__BORLANDC__)
    #pragma hdrstop
#endif

#if wxUSE_THREADS

#include "wx/thread.h"

#ifndef WX_PRECOMP
    #include "wx/intl.h"
    #include "wx/app.h"
    #include "wx/log.h"
    #include "wx/module.h"
    #include "wx/msgout.h"
#endif

#include "wx/apptrait.h"
#include "wx/scopeguard.h"

#include "wx/msw/private.h"
#include "wx/msw/missing.h"
#include "wx/msw/seh.h"

#include "wx/except.h"

#ifndef _MT
    #define _MT
#endif

#if defined(__BORLANDC__)
    #if !defined(__MT__)
                #define __MT__
    #endif

    #if !defined(__MFC_COMPAT__)
                #define __MFC_COMPAT__
    #endif
#endif 
#if defined(__VISUALC__) || \
    (defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)) || \
    (defined(__GNUG__) && defined(__MSVCRT__))

    #undef wxUSE_BEGIN_THREAD
    #define wxUSE_BEGIN_THREAD

#endif

#ifdef wxUSE_BEGIN_THREAD
        #include <process.h>

            typedef unsigned THREAD_RETVAL;

        #define THREAD_CALLCONV __stdcall
#else
        typedef DWORD THREAD_RETVAL;
    #define THREAD_CALLCONV WINAPI
#endif

static const THREAD_RETVAL THREAD_ERROR_EXIT = (THREAD_RETVAL)-1;


enum wxThreadState
{
    STATE_NEW,              STATE_RUNNING,          STATE_PAUSED,           STATE_CANCELED,         STATE_EXITED        };


static DWORD gs_tlsThisThread = 0xFFFFFFFF;

wxThreadIdType wxThread::ms_idMainThread = 0;

static bool gs_bGuiOwnedByMainThread = true;

static wxCriticalSection *gs_critsectGui = NULL;

static wxCriticalSection *gs_critsectWaitingForGui = NULL;

static wxCriticalSection *gs_critsectThreadDelete = NULL;

static size_t gs_nWaitingForGui = 0;

static bool gs_waitingForThread = false;



wxCriticalSection::wxCriticalSection( wxCriticalSectionType WXUNUSED(critSecType) )
{
    wxCOMPILE_TIME_ASSERT( sizeof(CRITICAL_SECTION) <= sizeof(wxCritSectBuffer),
                           wxCriticalSectionBufferTooSmall );

    ::InitializeCriticalSection((CRITICAL_SECTION *)m_buffer);
}

wxCriticalSection::~wxCriticalSection()
{
    ::DeleteCriticalSection((CRITICAL_SECTION *)m_buffer);
}

void wxCriticalSection::Enter()
{
    ::EnterCriticalSection((CRITICAL_SECTION *)m_buffer);
}

bool wxCriticalSection::TryEnter()
{
    return ::TryEnterCriticalSection((CRITICAL_SECTION *)m_buffer) != 0;
}

void wxCriticalSection::Leave()
{
    ::LeaveCriticalSection((CRITICAL_SECTION *)m_buffer);
}


class wxMutexInternal
{
public:
    wxMutexInternal(wxMutexType mutexType);
    ~wxMutexInternal();

    bool IsOk() const { return m_mutex != NULL; }

    wxMutexError Lock() { return LockTimeout(INFINITE); }
    wxMutexError Lock(unsigned long ms) { return LockTimeout(ms); }
    wxMutexError TryLock();
    wxMutexError Unlock();

private:
    wxMutexError LockTimeout(DWORD milliseconds);

    HANDLE m_mutex;

    unsigned long m_owningThread;
    wxMutexType m_type;

    wxDECLARE_NO_COPY_CLASS(wxMutexInternal);
};

wxMutexInternal::wxMutexInternal(wxMutexType mutexType)
{
        m_mutex = ::CreateMutex
                (
                    NULL,                           FALSE,                          NULL                        );

    m_type = mutexType;
    m_owningThread = 0;

    if ( !m_mutex )
    {
        wxLogLastError(wxT("CreateMutex()"));
    }

}

wxMutexInternal::~wxMutexInternal()
{
    if ( m_mutex )
    {
        if ( !::CloseHandle(m_mutex) )
        {
            wxLogLastError(wxT("CloseHandle(mutex)"));
        }
    }
}

wxMutexError wxMutexInternal::TryLock()
{
    const wxMutexError rc = LockTimeout(0);

        return rc == wxMUTEX_TIMEOUT ? wxMUTEX_BUSY : rc;
}

wxMutexError wxMutexInternal::LockTimeout(DWORD milliseconds)
{
    if (m_type == wxMUTEX_DEFAULT)
    {
                if (m_owningThread != 0)
        {
            if (m_owningThread == wxThread::GetCurrentId())
                return wxMUTEX_DEAD_LOCK;
        }
    }

    DWORD rc = ::WaitForSingleObject(m_mutex, milliseconds);
    switch ( rc )
    {
        case WAIT_ABANDONED:
                                    wxLogDebug(wxT("WaitForSingleObject() returned WAIT_ABANDONED"));
            
        case WAIT_OBJECT_0:
                        break;

        case WAIT_TIMEOUT:
            return wxMUTEX_TIMEOUT;

        default:
            wxFAIL_MSG(wxT("impossible return value in wxMutex::Lock"));
            
        case WAIT_FAILED:
            wxLogLastError(wxT("WaitForSingleObject(mutex)"));
            return wxMUTEX_MISC_ERROR;
    }

    if (m_type == wxMUTEX_DEFAULT)
    {
                m_owningThread = wxThread::GetCurrentId();
    }

    return wxMUTEX_NO_ERROR;
}

wxMutexError wxMutexInternal::Unlock()
{
        m_owningThread = 0;

    if ( !::ReleaseMutex(m_mutex) )
    {
        wxLogLastError(wxT("ReleaseMutex()"));

        return wxMUTEX_MISC_ERROR;
    }

    return wxMUTEX_NO_ERROR;
}


class wxSemaphoreInternal
{
public:
    wxSemaphoreInternal(int initialcount, int maxcount);
    ~wxSemaphoreInternal();

    bool IsOk() const { return m_semaphore != NULL; }

    wxSemaError Wait() { return WaitTimeout(INFINITE); }

    wxSemaError TryWait()
    {
        wxSemaError rc = WaitTimeout(0);
        if ( rc == wxSEMA_TIMEOUT )
            rc = wxSEMA_BUSY;

        return rc;
    }

    wxSemaError WaitTimeout(unsigned long milliseconds);

    wxSemaError Post();

private:
    HANDLE m_semaphore;

    wxDECLARE_NO_COPY_CLASS(wxSemaphoreInternal);
};

wxSemaphoreInternal::wxSemaphoreInternal(int initialcount, int maxcount)
{
    if ( maxcount == 0 )
    {
                maxcount = INT_MAX;
    }

    m_semaphore = ::CreateSemaphore
                    (
                        NULL,                                   initialcount,
                        maxcount,
                        NULL                                );
    if ( !m_semaphore )
    {
        wxLogLastError(wxT("CreateSemaphore()"));
    }
}

wxSemaphoreInternal::~wxSemaphoreInternal()
{
    if ( m_semaphore )
    {
        if ( !::CloseHandle(m_semaphore) )
        {
            wxLogLastError(wxT("CloseHandle(semaphore)"));
        }
    }
}

wxSemaError wxSemaphoreInternal::WaitTimeout(unsigned long milliseconds)
{
    DWORD rc = ::WaitForSingleObject( m_semaphore, milliseconds );

    switch ( rc )
    {
        case WAIT_OBJECT_0:
           return wxSEMA_NO_ERROR;

        case WAIT_TIMEOUT:
           return wxSEMA_TIMEOUT;

        default:
            wxLogLastError(wxT("WaitForSingleObject(semaphore)"));
    }

    return wxSEMA_MISC_ERROR;
}

wxSemaError wxSemaphoreInternal::Post()
{
    if ( !::ReleaseSemaphore(m_semaphore, 1, NULL ) )
    {
        if ( GetLastError() == ERROR_TOO_MANY_POSTS )
        {
            return wxSEMA_OVERFLOW;
        }
        else
        {
            wxLogLastError(wxT("ReleaseSemaphore"));
            return wxSEMA_MISC_ERROR;
        }
    }

    return wxSEMA_NO_ERROR;
}



class wxThreadInternal
{
public:
    wxThreadInternal(wxThread *thread)
    {
        m_thread = thread;
        m_hThread = 0;
        m_state = STATE_NEW;
        m_priority = wxPRIORITY_DEFAULT;
        m_nRef = 1;
    }

    ~wxThreadInternal()
    {
        Free();
    }

    void Free()
    {
        if ( m_hThread )
        {
            if ( !::CloseHandle(m_hThread) )
            {
                wxLogLastError(wxT("CloseHandle(thread)"));
            }

            m_hThread = 0;
        }
    }

        bool Create(wxThread *thread, unsigned int stackSize);

            wxThreadError WaitForTerminate(wxCriticalSection& cs,
                                   wxThread::ExitCode *pRc,
                                   wxThreadWait waitMode,
                                   wxThread *threadToDelete = NULL);

        wxThreadError Kill();

        bool Suspend();
    bool Resume();
    void Cancel() { m_state = STATE_CANCELED; }

        void SetState(wxThreadState state) { m_state = state; }
    wxThreadState GetState() const { return m_state; }

        void SetPriority(unsigned int priority);
    unsigned int GetPriority() const { return m_priority; }

        HANDLE GetHandle() const { return m_hThread; }
    DWORD  GetId() const { return m_tid; }

        static THREAD_RETVAL THREAD_CALLCONV WinThreadStart(void *thread);

        static THREAD_RETVAL DoThreadStart(wxThread *thread);

        static void DoThreadOnExit(wxThread *thread);


    void KeepAlive()
    {
        if ( m_thread->IsDetached() )
            ::InterlockedIncrement(&m_nRef);
    }

    void LetDie()
    {
        if ( m_thread->IsDetached() && !::InterlockedDecrement(&m_nRef) )
            delete m_thread;
    }

private:
        wxThread *m_thread;

    HANDLE        m_hThread;        wxThreadState m_state;          unsigned int  m_priority;       DWORD         m_tid;        
            LONG m_nRef;

    wxDECLARE_NO_COPY_CLASS(wxThreadInternal);
};

class wxThreadKeepAlive
{
public:
    wxThreadKeepAlive(wxThreadInternal& thrImpl) : m_thrImpl(thrImpl)
        { m_thrImpl.KeepAlive(); }
    ~wxThreadKeepAlive()
        { m_thrImpl.LetDie(); }

private:
    wxThreadInternal& m_thrImpl;
};


void wxThreadInternal::DoThreadOnExit(wxThread *thread)
{
    wxTRY
    {
        thread->OnExit();
    }
    wxCATCH_ALL( wxTheApp->OnUnhandledException(); )
}


THREAD_RETVAL wxThreadInternal::DoThreadStart(wxThread *thread)
{
    wxON_BLOCK_EXIT1(DoThreadOnExit, thread);

    THREAD_RETVAL rc = THREAD_ERROR_EXIT;

    wxTRY
    {
                if ( !::TlsSetValue(gs_tlsThisThread, thread) )
        {
            wxLogSysError(_("Cannot start thread: error writing TLS."));

            return THREAD_ERROR_EXIT;
        }

        rc = wxPtrToUInt(thread->CallEntry());
    }
    wxCATCH_ALL( wxTheApp->OnUnhandledException(); )

    return rc;
}


THREAD_RETVAL THREAD_CALLCONV wxThreadInternal::WinThreadStart(void *param)
{
    THREAD_RETVAL rc = THREAD_ERROR_EXIT;

    wxThread * const thread = (wxThread *)param;

        DisableAutomaticSETranslator();

        
            thread->m_critsect.Enter();
    const bool hasExited = thread->m_internal->GetState() == STATE_EXITED;
    thread->m_critsect.Leave();

        wxSEH_TRY
    {
        if ( hasExited )
            DoThreadOnExit(thread);
        else
            rc = DoThreadStart(thread);
    }
    wxSEH_HANDLE(THREAD_ERROR_EXIT)


            const bool isDetached = thread->IsDetached();
    if ( !hasExited )
    {
        thread->m_critsect.Enter();
        thread->m_internal->SetState(STATE_EXITED);
        thread->m_critsect.Leave();
    }

        if ( isDetached )
        thread->m_internal->LetDie();

    return rc;
}

void wxThreadInternal::SetPriority(unsigned int priority)
{
    m_priority = priority;

        int win_priority;
    if (m_priority <= 20)
        win_priority = THREAD_PRIORITY_LOWEST;
    else if (m_priority <= 40)
        win_priority = THREAD_PRIORITY_BELOW_NORMAL;
    else if (m_priority <= 60)
        win_priority = THREAD_PRIORITY_NORMAL;
    else if (m_priority <= 80)
        win_priority = THREAD_PRIORITY_ABOVE_NORMAL;
    else if (m_priority <= 100)
        win_priority = THREAD_PRIORITY_HIGHEST;
    else
    {
        wxFAIL_MSG(wxT("invalid value of thread priority parameter"));
        win_priority = THREAD_PRIORITY_NORMAL;
    }

    if ( !::SetThreadPriority(m_hThread, win_priority) )
    {
        wxLogSysError(_("Can't set thread priority"));
    }
}

bool wxThreadInternal::Create(wxThread *thread, unsigned int stackSize)
{
    wxASSERT_MSG( m_state == STATE_NEW && !m_hThread,
                    wxT("Create()ing thread twice?") );

            #ifdef wxUSE_BEGIN_THREAD
    m_hThread = (HANDLE)_beginthreadex
                        (
                          NULL,                                                       stackSize,
                          wxThreadInternal::WinThreadStart,                           thread,
                          CREATE_SUSPENDED,
                          (unsigned int *)&m_tid
                        );
#else     m_hThread = ::CreateThread
                  (
                    NULL,                                                   stackSize,                                              wxThreadInternal::WinThreadStart,                       (LPVOID)thread,                                         CREATE_SUSPENDED,                                       &m_tid                                                );
#endif 
    if ( m_hThread == NULL )
    {
        wxLogSysError(_("Can't create thread"));

        return false;
    }

    if ( m_priority != wxPRIORITY_DEFAULT )
    {
        SetPriority(m_priority);
    }

    return true;
}

wxThreadError wxThreadInternal::Kill()
{
    m_thread->OnKill();

    if ( !::TerminateThread(m_hThread, THREAD_ERROR_EXIT) )
    {
        wxLogSysError(_("Couldn't terminate thread"));

        return wxTHREAD_MISC_ERROR;
    }

    Free();

    return wxTHREAD_NO_ERROR;
}

wxThreadError
wxThreadInternal::WaitForTerminate(wxCriticalSection& cs,
                                   wxThread::ExitCode *pRc,
                                   wxThreadWait waitMode,
                                   wxThread *threadToDelete)
{
            wxThreadKeepAlive keepAlive(*this);


            bool shouldDelete = threadToDelete != NULL;

    DWORD rc = 0;

        bool shouldResume = false;

            {
        wxCriticalSectionLocker lock(cs);

        if ( m_state == STATE_NEW )
        {
            if ( shouldDelete )
            {
                                                                m_state = STATE_EXITED;

                                                                                shouldResume = true;
                shouldDelete = false;
            }
                                }
        else         {
            shouldResume = m_state == STATE_PAUSED;
        }
    }

        if ( shouldResume )
        Resume();

        if ( shouldDelete )
    {
        wxCriticalSectionLocker lock(cs);

        Cancel();
    }

    if ( threadToDelete )
        threadToDelete->OnDelete();

        if ( wxThread::IsMain() )
    {
                gs_waitingForThread = true;
    }

    wxAppTraits& traits = wxApp::GetValidTraits();

                        DWORD result wxDUMMY_INITIALIZE(0);
    do
    {
        if ( wxThread::IsMain() )
        {
                                    if ( (gs_nWaitingForGui > 0) && wxGuiOwnedByMainThread() )
            {
                wxMutexGuiLeave();
            }
        }

                        result = traits.WaitForThread(m_hThread, waitMode);

        switch ( result )
        {
            case 0xFFFFFFFF:
                                wxLogSysError(_("Cannot wait for thread termination"));
                Kill();
                return wxTHREAD_KILLED;

            case WAIT_OBJECT_0:
                                break;

            case WAIT_OBJECT_0 + 1:
            case WAIT_OBJECT_0 + 2:
                                                                                                                                                if ( wxThread::IsMain() )
                {
                    if ( !traits.DoMessageFromThreadWait() )
                    {
                                                Kill();

                        return wxTHREAD_KILLED;
                    }
                }
                break;

            default:
                wxFAIL_MSG(wxT("unexpected result of MsgWaitForMultipleObject"));
        }
    } while ( result != WAIT_OBJECT_0 );

    if ( wxThread::IsMain() )
    {
        gs_waitingForThread = false;
    }


                for ( ;; )
    {
        if ( !::GetExitCodeThread(m_hThread, &rc) )
        {
            wxLogLastError(wxT("GetExitCodeThread"));

            rc = THREAD_ERROR_EXIT;

            break;
        }

        if ( rc != STILL_ACTIVE )
            break;

                        ::Sleep(1);
    }

    if ( pRc )
        *pRc = wxUIntToPtr(rc);

        Free();


    return rc == THREAD_ERROR_EXIT ? wxTHREAD_MISC_ERROR : wxTHREAD_NO_ERROR;
}

bool wxThreadInternal::Suspend()
{
    DWORD nSuspendCount = ::SuspendThread(m_hThread);
    if ( nSuspendCount == (DWORD)-1 )
    {
        wxLogSysError(_("Cannot suspend thread %lx"),
                      static_cast<unsigned long>(wxPtrToUInt(m_hThread)));

        return false;
    }

    m_state = STATE_PAUSED;

    return true;
}

bool wxThreadInternal::Resume()
{
    DWORD nSuspendCount = ::ResumeThread(m_hThread);
    if ( nSuspendCount == (DWORD)-1 )
    {
        wxLogSysError(_("Cannot resume thread %lx"),
                      static_cast<unsigned long>(wxPtrToUInt(m_hThread)));

        return false;
    }

                if ( m_state != STATE_EXITED )
    {
        m_state = STATE_RUNNING;
    }

    return true;
}


wxThread *wxThread::This()
{
    wxThread *thread = (wxThread *)::TlsGetValue(gs_tlsThisThread);

        if ( !thread && (::GetLastError() != NO_ERROR) )
    {
        wxLogSysError(_("Couldn't get the current thread pointer"));

            }

    return thread;
}

void wxThread::Yield()
{
            ::Sleep(0);
}

int wxThread::GetCPUCount()
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    return si.dwNumberOfProcessors;
}

unsigned long wxThread::GetCurrentId()
{
    return (unsigned long)::GetCurrentThreadId();
}

bool wxThread::SetConcurrency(size_t level)
{
    wxASSERT_MSG( IsMain(), wxT("should only be called from the main thread") );

        if ( level == 0 )
        return 0;

        HANDLE hProcess = ::GetCurrentProcess();
    DWORD_PTR dwProcMask, dwSysMask;
    if ( ::GetProcessAffinityMask(hProcess, &dwProcMask, &dwSysMask) == 0 )
    {
        wxLogLastError(wxT("GetProcessAffinityMask"));

        return false;
    }

        if ( dwSysMask == 1 )
    {
                        return level == 1;
    }

                DWORD bit = 1;
    while ( bit )
    {
        if ( dwSysMask & bit )
        {
                        dwProcMask |= bit;

                        if ( --level == 0 )
            {
                                break;
            }
        }

                bit <<= 1;
    }

        if ( level != 0 )
    {
        wxLogDebug(wxT("bad level %u in wxThread::SetConcurrency()"), level);

        return false;
    }

    if ( ::SetProcessAffinityMask(hProcess, dwProcMask) == 0 )
    {
        wxLogLastError(wxT("SetProcessAffinityMask"));

        return false;
    }

    return true;
}


wxThread::wxThread(wxThreadKind kind)
{
    m_internal = new wxThreadInternal(this);

    m_isDetached = kind == wxTHREAD_DETACHED;
}

wxThread::~wxThread()
{
    delete m_internal;
}


wxThreadError wxThread::Create(unsigned int stackSize)
{
    wxCriticalSectionLocker lock(m_critsect);

    if ( !m_internal->Create(this, stackSize) )
        return wxTHREAD_NO_RESOURCE;

    return wxTHREAD_NO_ERROR;
}

wxThreadError wxThread::Run()
{
    wxCriticalSectionLocker lock(m_critsect);

            if ( !m_internal->GetHandle() )
    {
        if ( !m_internal->Create(this, 0) )
            return wxTHREAD_NO_RESOURCE;
    }

    wxCHECK_MSG( m_internal->GetState() == STATE_NEW, wxTHREAD_RUNNING,
             wxT("thread may only be started once after Create()") );

        return Resume();
}


wxThreadError wxThread::Pause()
{
    wxCriticalSectionLocker lock(m_critsect);

    return m_internal->Suspend() ? wxTHREAD_NO_ERROR : wxTHREAD_MISC_ERROR;
}

wxThreadError wxThread::Resume()
{
    wxCriticalSectionLocker lock(m_critsect);

    return m_internal->Resume() ? wxTHREAD_NO_ERROR : wxTHREAD_MISC_ERROR;
}


wxThread::ExitCode wxThread::Wait(wxThreadWait waitMode)
{
    ExitCode rc = wxUIntToPtr(THREAD_ERROR_EXIT);

            wxCHECK_MSG( !IsDetached(), rc,
                 wxT("wxThread::Wait(): can't wait for detached thread") );

    (void)m_internal->WaitForTerminate(m_critsect, &rc, waitMode);

    return rc;
}

wxThreadError wxThread::Delete(ExitCode *pRc, wxThreadWait waitMode)
{
    return m_internal->WaitForTerminate(m_critsect, pRc, waitMode, this);
}

wxThreadError wxThread::Kill()
{
    if ( !IsRunning() )
        return wxTHREAD_NOT_RUNNING;

    wxThreadError rc = m_internal->Kill();

    if ( IsDetached() )
    {
        delete this;
    }
    else     {
                wxCriticalSectionLocker lock(m_critsect);
        m_internal->SetState(STATE_EXITED);
    }

    return rc;
}

void wxThread::Exit(ExitCode status)
{
    wxThreadInternal::DoThreadOnExit(this);

    m_internal->Free();

    if ( IsDetached() )
    {
        delete this;
    }
    else     {
                wxCriticalSectionLocker lock(m_critsect);
        m_internal->SetState(STATE_EXITED);
    }

#ifdef wxUSE_BEGIN_THREAD
    _endthreadex(wxPtrToUInt(status));
#else     ::ExitThread(wxPtrToUInt(status));
#endif 
    wxFAIL_MSG(wxT("Couldn't return from ExitThread()!"));
}


void wxThread::SetPriority(unsigned int prio)
{
    wxCriticalSectionLocker lock(m_critsect);

    m_internal->SetPriority(prio);
}

unsigned int wxThread::GetPriority() const
{
    wxCriticalSectionLocker lock(const_cast<wxCriticalSection &>(m_critsect));

    return m_internal->GetPriority();
}

unsigned long wxThread::GetId() const
{
    wxCriticalSectionLocker lock(const_cast<wxCriticalSection &>(m_critsect));

    return (unsigned long)m_internal->GetId();
}

WXHANDLE wxThread::MSWGetHandle() const
{
    wxCriticalSectionLocker lock(const_cast<wxCriticalSection &>(m_critsect));

    return m_internal->GetHandle();
}

bool wxThread::IsRunning() const
{
    wxCriticalSectionLocker lock(const_cast<wxCriticalSection &>(m_critsect));

    return m_internal->GetState() == STATE_RUNNING;
}

bool wxThread::IsAlive() const
{
    wxCriticalSectionLocker lock(const_cast<wxCriticalSection &>(m_critsect));

    return (m_internal->GetState() == STATE_RUNNING) ||
           (m_internal->GetState() == STATE_PAUSED);
}

bool wxThread::IsPaused() const
{
    wxCriticalSectionLocker lock(const_cast<wxCriticalSection &>(m_critsect));

    return m_internal->GetState() == STATE_PAUSED;
}

bool wxThread::TestDestroy()
{
    wxCriticalSectionLocker lock(const_cast<wxCriticalSection &>(m_critsect));

    return m_internal->GetState() == STATE_CANCELED;
}


class wxThreadModule : public wxModule
{
public:
    virtual bool OnInit();
    virtual void OnExit();

private:
    wxDECLARE_DYNAMIC_CLASS(wxThreadModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxThreadModule, wxModule);

bool wxThreadModule::OnInit()
{
        gs_tlsThisThread = ::TlsAlloc();
    if ( gs_tlsThisThread == 0xFFFFFFFF )
    {
                                wxLogSysError(_("Thread module initialization failed: impossible to allocate index in thread local storage"));

        return false;
    }

            if ( !::TlsSetValue(gs_tlsThisThread, (LPVOID)0) )
    {
        ::TlsFree(gs_tlsThisThread);
        gs_tlsThisThread = 0xFFFFFFFF;

        wxLogSysError(_("Thread module initialization failed: cannot store value in thread local storage"));

        return false;
    }

    gs_critsectWaitingForGui = new wxCriticalSection();

    gs_critsectGui = new wxCriticalSection();
    gs_critsectGui->Enter();

    gs_critsectThreadDelete = new wxCriticalSection;

    wxThread::ms_idMainThread = wxThread::GetCurrentId();

    return true;
}

void wxThreadModule::OnExit()
{
    if ( !::TlsFree(gs_tlsThisThread) )
    {
        wxLogLastError(wxT("TlsFree failed."));
    }

    wxDELETE(gs_critsectThreadDelete);

    if ( gs_critsectGui )
    {
        gs_critsectGui->Leave();
        wxDELETE(gs_critsectGui);
    }

    wxDELETE(gs_critsectWaitingForGui);
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
        {
            wxMutexGuiLeave();
        }
            }
}

bool WXDLLIMPEXP_BASE wxGuiOwnedByMainThread()
{
    return gs_bGuiOwnedByMainThread;
}

void WXDLLIMPEXP_BASE wxWakeUpMainThread()
{
        if ( !::PostThreadMessage(wxThread::GetMainId(), WM_NULL, 0, 0) )
    {
                                const unsigned long ec = wxSysErrorCode();
        wxMessageOutputDebug().Printf
        (
            wxS("Failed to wake up main thread: PostThreadMessage(WM_NULL) ")
            wxS("failed with error 0x%08lx (%s)."),
            ec,
            wxSysErrorMsg(ec)
        );
    }
}

bool WXDLLIMPEXP_BASE wxIsWaitingForThread()
{
    return gs_waitingForThread;
}


#include "wx/thrimpl.cpp"

#endif 