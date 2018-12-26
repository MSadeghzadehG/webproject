


#include "wx/wxprec.h"

#if wxUSE_THREADS

#include "wx/thread.h"
#include "wx/except.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/dynarray.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/utils.h"
    #include "wx/timer.h"
    #include "wx/stopwatch.h"
    #include "wx/module.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>           #ifdef HAVE_SCHED_H
    #include <sched.h>
#endif

#ifdef HAVE_THR_SETCONCURRENCY
    #include <thread.h>
#endif

#ifdef HAVE_ABI_FORCEDUNWIND
    #include <cxxabi.h>
#endif

#ifdef HAVE_SETPRIORITY
    #include <sys/resource.h>   #endif

#ifdef __LINUX__
    #include "wx/ffile.h"
#endif

#define THR_ID_CAST(id)  (reinterpret_cast<void*>(id))
#define THR_ID(thr)      THR_ID_CAST((thr)->GetId())


enum wxThreadState
{
    STATE_NEW,              STATE_RUNNING,          STATE_PAUSED,           STATE_EXITED        };

static const wxThread::ExitCode EXITCODE_CANCELLED = (wxThread::ExitCode)-1;

#define TRACE_THREADS   wxT("thread")

#define TRACE_SEMA      wxT("semaphore")


static void ScheduleThreadForDeletion();
static void DeleteThread(wxThread *This);


WX_DEFINE_ARRAY_PTR(wxThread *, wxArrayThread);



static wxArrayThread gs_allThreads;

static wxMutex *gs_mutexAllThreads = NULL;

wxThreadIdType wxThread::ms_idMainThread = 0;

static pthread_key_t gs_keySelf;

static size_t gs_nThreadsBeingDeleted = 0;

static wxMutex *gs_mutexDeleteThread = NULL;

static wxCondition *gs_condAllDeleted = NULL;

#ifndef __WXOSX__
static wxMutex *gs_mutexGui = NULL;
#endif




class wxMutexInternal
{
public:
    wxMutexInternal(wxMutexType mutexType);
    ~wxMutexInternal();

    wxMutexError Lock();
    wxMutexError Lock(unsigned long ms);
    wxMutexError TryLock();
    wxMutexError Unlock();

    bool IsOk() const { return m_isOk; }

private:
        wxMutexError HandleLockResult(int err);

private:
    pthread_mutex_t m_mutex;
    bool m_isOk;
    wxMutexType m_type;
    unsigned long m_owningThread;

        friend class wxConditionInternal;
};

#if defined(HAVE_PTHREAD_MUTEXATTR_T) && \
        wxUSE_UNIX && !defined(HAVE_PTHREAD_MUTEXATTR_SETTYPE_DECL)
extern "C" int pthread_mutexattr_settype(pthread_mutexattr_t *, int);
#endif

wxMutexInternal::wxMutexInternal(wxMutexType mutexType)
{
    m_type = mutexType;
    m_owningThread = 0;

    int err;
    switch ( mutexType )
    {
        case wxMUTEX_RECURSIVE:
                                                            #ifdef HAVE_PTHREAD_MUTEXATTR_T
            {
                pthread_mutexattr_t attr;
                pthread_mutexattr_init(&attr);
                pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

                err = pthread_mutex_init(&m_mutex, &attr);
            }
#elif defined(HAVE_PTHREAD_RECURSIVE_MUTEX_INITIALIZER)
                                                {
                pthread_mutex_t mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
                m_mutex = mutex;
            }
#else             err = EINVAL;
#endif             break;

        default:
            wxFAIL_MSG( wxT("unknown mutex type") );
            wxFALLTHROUGH;

        case wxMUTEX_DEFAULT:
            err = pthread_mutex_init(&m_mutex, NULL);
            break;
    }

    m_isOk = err == 0;
    if ( !m_isOk )
    {
        wxLogApiError( wxT("pthread_mutex_init()"), err);
    }
}

wxMutexInternal::~wxMutexInternal()
{
    if ( m_isOk )
    {
        int err = pthread_mutex_destroy(&m_mutex);
        if ( err != 0 )
        {
            wxLogApiError( wxT("pthread_mutex_destroy()"), err);
        }
    }
}

wxMutexError wxMutexInternal::Lock()
{
    if ((m_type == wxMUTEX_DEFAULT) && (m_owningThread != 0))
    {
        if (m_owningThread == wxThread::GetCurrentId())
           return wxMUTEX_DEAD_LOCK;
    }

    return HandleLockResult(pthread_mutex_lock(&m_mutex));
}

wxMutexError wxMutexInternal::Lock(unsigned long ms)
{
#ifdef HAVE_PTHREAD_MUTEX_TIMEDLOCK
    static const long MSEC_IN_SEC   = 1000;
    static const long NSEC_IN_MSEC  = 1000000;
    static const long NSEC_IN_USEC  = 1000;
    static const long NSEC_IN_SEC   = MSEC_IN_SEC * NSEC_IN_MSEC;

    time_t seconds = ms/MSEC_IN_SEC;
    long nanoseconds = (ms % MSEC_IN_SEC) * NSEC_IN_MSEC;
    timespec ts = { 0, 0 };

                    #if 0
    if ( clock_gettime(CLOCK_REALTIME, &ts) == 0 )
    {
    }
#else
    struct timeval tv;
    if ( wxGetTimeOfDay(&tv) != -1 )
    {
        ts.tv_sec = tv.tv_sec;
        ts.tv_nsec = tv.tv_usec*NSEC_IN_USEC;
    }
#endif
    else     {
        ts.tv_sec = time(NULL);
    }

    ts.tv_sec += seconds;
    ts.tv_nsec += nanoseconds;
    if ( ts.tv_nsec > NSEC_IN_SEC )
    {
        ts.tv_sec += 1;
        ts.tv_nsec -= NSEC_IN_SEC;
    }

    return HandleLockResult(pthread_mutex_timedlock(&m_mutex, &ts));
#else     wxUnusedVar(ms);

    return wxMUTEX_MISC_ERROR;
#endif }

wxMutexError wxMutexInternal::HandleLockResult(int err)
{
    
    switch ( err )
    {
        case EDEADLK:
                                    wxFAIL_MSG( wxT("mutex deadlock prevented") );
            return wxMUTEX_DEAD_LOCK;

        case EINVAL:
            wxLogDebug(wxT("pthread_mutex_[timed]lock(): mutex not initialized"));
            break;

        case ETIMEDOUT:
            return wxMUTEX_TIMEOUT;

        case 0:
            if (m_type == wxMUTEX_DEFAULT)
                m_owningThread = wxThread::GetCurrentId();
            return wxMUTEX_NO_ERROR;

        default:
            wxLogApiError(wxT("pthread_mutex_[timed]lock()"), err);
    }

    return wxMUTEX_MISC_ERROR;
}


wxMutexError wxMutexInternal::TryLock()
{
    int err = pthread_mutex_trylock(&m_mutex);
    switch ( err )
    {
        case EBUSY:
                                    return wxMUTEX_BUSY;

        case EINVAL:
            wxLogDebug(wxT("pthread_mutex_trylock(): mutex not initialized."));
            break;

        case 0:
            if (m_type == wxMUTEX_DEFAULT)
                m_owningThread = wxThread::GetCurrentId();
            return wxMUTEX_NO_ERROR;

        default:
            wxLogApiError(wxT("pthread_mutex_trylock()"), err);
    }

    return wxMUTEX_MISC_ERROR;
}

wxMutexError wxMutexInternal::Unlock()
{
    m_owningThread = 0;

    int err = pthread_mutex_unlock(&m_mutex);
    switch ( err )
    {
        case EPERM:
                        return wxMUTEX_UNLOCKED;

        case EINVAL:
            wxLogDebug(wxT("pthread_mutex_unlock(): mutex not initialized."));
            break;

        case 0:
            return wxMUTEX_NO_ERROR;

        default:
            wxLogApiError(wxT("pthread_mutex_unlock()"), err);
    }

    return wxMUTEX_MISC_ERROR;
}



class wxConditionInternal
{
public:
    wxConditionInternal(wxMutex& mutex);
    ~wxConditionInternal();

    bool IsOk() const { return m_isOk && m_mutex.IsOk(); }

    wxCondError Wait();
    wxCondError WaitTimeout(unsigned long milliseconds);

    wxCondError Signal();
    wxCondError Broadcast();

private:
        pthread_mutex_t *GetPMutex() const { return &m_mutex.m_internal->m_mutex; }

    wxMutex& m_mutex;
    pthread_cond_t m_cond;

    bool m_isOk;
};

wxConditionInternal::wxConditionInternal(wxMutex& mutex)
                   : m_mutex(mutex)
{
    int err = pthread_cond_init(&m_cond, NULL );

    m_isOk = err == 0;

    if ( !m_isOk )
    {
        wxLogApiError(wxT("pthread_cond_init()"), err);
    }
}

wxConditionInternal::~wxConditionInternal()
{
    if ( m_isOk )
    {
        int err = pthread_cond_destroy(&m_cond);
        if ( err != 0 )
        {
            wxLogApiError(wxT("pthread_cond_destroy()"), err);
        }
    }
}

wxCondError wxConditionInternal::Wait()
{
    int err = pthread_cond_wait(&m_cond, GetPMutex());
    if ( err != 0 )
    {
        wxLogApiError(wxT("pthread_cond_wait()"), err);

        return wxCOND_MISC_ERROR;
    }

    return wxCOND_NO_ERROR;
}

wxCondError wxConditionInternal::WaitTimeout(unsigned long milliseconds)
{
    wxLongLong curtime = wxGetUTCTimeMillis();
    curtime += milliseconds;
    wxLongLong temp = curtime / 1000;
    int sec = temp.GetLo();
    temp *= 1000;
    temp = curtime - temp;
    int millis = temp.GetLo();

    timespec tspec;

    tspec.tv_sec = sec;
    tspec.tv_nsec = millis * 1000L * 1000L;

    int err = pthread_cond_timedwait( &m_cond, GetPMutex(), &tspec );
    switch ( err )
    {
        case ETIMEDOUT:
            return wxCOND_TIMEOUT;

        case 0:
            return wxCOND_NO_ERROR;

        default:
            wxLogApiError(wxT("pthread_cond_timedwait()"), err);
    }

    return wxCOND_MISC_ERROR;
}

wxCondError wxConditionInternal::Signal()
{
    int err = pthread_cond_signal(&m_cond);
    if ( err != 0 )
    {
        wxLogApiError(wxT("pthread_cond_signal()"), err);

        return wxCOND_MISC_ERROR;
    }

    return wxCOND_NO_ERROR;
}

wxCondError wxConditionInternal::Broadcast()
{
    int err = pthread_cond_broadcast(&m_cond);
    if ( err != 0 )
    {
        wxLogApiError(wxT("pthread_cond_broadcast()"), err);

        return wxCOND_MISC_ERROR;
    }

    return wxCOND_NO_ERROR;
}



class wxSemaphoreInternal
{
public:
    wxSemaphoreInternal(int initialcount, int maxcount);

    bool IsOk() const { return m_isOk; }

    wxSemaError Wait();
    wxSemaError TryWait();
    wxSemaError WaitTimeout(unsigned long milliseconds);

    wxSemaError Post();

private:
    wxMutex m_mutex;
    wxCondition m_cond;

    size_t m_count,
           m_maxcount;

    bool m_isOk;
};

wxSemaphoreInternal::wxSemaphoreInternal(int initialcount, int maxcount)
                   : m_cond(m_mutex)
{

    if ( (initialcount < 0 || maxcount < 0) ||
            ((maxcount > 0) && (initialcount > maxcount)) )
    {
        wxFAIL_MSG( wxT("wxSemaphore: invalid initial or maximal count") );

        m_isOk = false;
    }
    else
    {
        m_maxcount = (size_t)maxcount;
        m_count = (size_t)initialcount;
    }

    m_isOk = m_mutex.IsOk() && m_cond.IsOk();
}

wxSemaError wxSemaphoreInternal::Wait()
{
    wxMutexLocker locker(m_mutex);

    while ( m_count == 0 )
    {
        wxLogTrace(TRACE_SEMA,
                   wxT("Thread %p waiting for semaphore to become signalled"),
                   THR_ID_CAST(wxThread::GetCurrentId()));

        if ( m_cond.Wait() != wxCOND_NO_ERROR )
            return wxSEMA_MISC_ERROR;

        wxLogTrace(TRACE_SEMA,
                   wxT("Thread %p finished waiting for semaphore, count = %lu"),
                   THR_ID_CAST(wxThread::GetCurrentId()), (unsigned long)m_count);
    }

    m_count--;

    return wxSEMA_NO_ERROR;
}

wxSemaError wxSemaphoreInternal::TryWait()
{
    wxMutexLocker locker(m_mutex);

    if ( m_count == 0 )
        return wxSEMA_BUSY;

    m_count--;

    return wxSEMA_NO_ERROR;
}

wxSemaError wxSemaphoreInternal::WaitTimeout(unsigned long milliseconds)
{
    wxMutexLocker locker(m_mutex);

    wxLongLong startTime = wxGetLocalTimeMillis();

    while ( m_count == 0 )
    {
        wxLongLong elapsed = wxGetLocalTimeMillis() - startTime;
        long remainingTime = (long)milliseconds - (long)elapsed.GetLo();
        if ( remainingTime <= 0 )
        {
                        return wxSEMA_TIMEOUT;
        }

        switch ( m_cond.WaitTimeout(remainingTime) )
        {
            case wxCOND_TIMEOUT:
                return wxSEMA_TIMEOUT;

            default:
                return wxSEMA_MISC_ERROR;

            case wxCOND_NO_ERROR:
                ;
        }
    }

    m_count--;

    return wxSEMA_NO_ERROR;
}

wxSemaError wxSemaphoreInternal::Post()
{
    wxMutexLocker locker(m_mutex);

    if ( m_maxcount > 0 && m_count == m_maxcount )
    {
        return wxSEMA_OVERFLOW;
    }

    m_count++;

    wxLogTrace(TRACE_SEMA,
               wxT("Thread %p about to signal semaphore, count = %lu"),
               THR_ID_CAST(wxThread::GetCurrentId()), (unsigned long)m_count);

    return m_cond.Signal() == wxCOND_NO_ERROR ? wxSEMA_NO_ERROR
                                              : wxSEMA_MISC_ERROR;
}


extern "C"
{

#ifdef wxHAVE_PTHREAD_CLEANUP
        void wxPthreadCleanup(void *ptr);
#endif 
void *wxPthreadStart(void *ptr);

} 

class wxThreadInternal
{
public:
    wxThreadInternal();
    ~wxThreadInternal();

        static void *PthreadStart(wxThread *thread);

                 wxThreadError Create(wxThread *thread, unsigned int stackSize);
            wxThreadError Run();
            void SignalRun() { m_semRun.Post(); }
            void Wait();
            void Pause();
            void Resume();

                int GetPriority() const { return m_prio; }
    void SetPriority(int prio) { m_prio = prio; }
            wxThreadState GetState() const { return m_state; }
    void SetState(wxThreadState state)
    {
#if wxUSE_LOG_TRACE
        static const wxChar *const stateNames[] =
        {
            wxT("NEW"),
            wxT("RUNNING"),
            wxT("PAUSED"),
            wxT("EXITED"),
        };

        wxLogTrace(TRACE_THREADS, wxT("Thread %p: %s => %s."),
                   THR_ID(this), stateNames[m_state], stateNames[state]);
#endif 
        m_state = state;
    }
            pthread_t GetId() const { return m_threadId; }
    pthread_t *GetIdPtr() { return &m_threadId; }
            bool WasCreated() const { return m_created; }
            void SetCancelFlag() { m_cancelled = true; }
    bool WasCancelled() const { return m_cancelled; }
            void SetExitCode(wxThread::ExitCode exitcode) { m_exitcode = exitcode; }
    wxThread::ExitCode GetExitCode() const { return m_exitcode; }

            void SetReallyPaused(bool paused) { m_isPaused = paused; }
    bool IsReallyPaused() const { return m_isPaused; }

            void Detach()
    {
        wxCriticalSectionLocker lock(m_csJoinFlag);

        m_shouldBeJoined = false;
        m_isDetached = true;
    }

#ifdef wxHAVE_PTHREAD_CLEANUP
        static void Cleanup(wxThread *thread);
#endif 
private:
    pthread_t     m_threadId;       wxThreadState m_state;          int           m_prio;       
        bool m_created;

        bool m_cancelled;

        bool m_isPaused;

            wxThread::ExitCode m_exitcode;

            wxCriticalSection m_csJoinFlag;
    bool m_shouldBeJoined;
    bool m_isDetached;

            wxSemaphore m_semRun;

            wxSemaphore m_semSuspend;
};


void *wxPthreadStart(void *ptr)
{
    return wxThreadInternal::PthreadStart((wxThread *)ptr);
}

void *wxThreadInternal::PthreadStart(wxThread *thread)
{
    wxThreadInternal *pthread = thread->m_internal;

    wxLogTrace(TRACE_THREADS, wxT("Thread %p started."), THR_ID(pthread));

            int rc = pthread_setspecific(gs_keySelf, thread);
    if ( rc != 0 )
    {
        wxLogSysError(rc, _("Cannot start thread: error writing TLS."));

        return (void *)-1;
    }

            bool dontRunAtAll;

#ifdef wxHAVE_PTHREAD_CLEANUP
            pthread_cleanup_push(wxPthreadCleanup, thread);
#endif 
        pthread->m_semRun.Wait();

            {
        wxCriticalSectionLocker lock(thread->m_critsect);

        dontRunAtAll = pthread->GetState() == STATE_NEW &&
                       pthread->WasCancelled();
    }

    if ( !dontRunAtAll )
    {
                wxLogTrace(TRACE_THREADS,
                   wxT("Thread %p about to enter its Entry()."),
                   THR_ID(pthread));

        wxTRY
        {
            pthread->m_exitcode = thread->CallEntry();

            wxLogTrace(TRACE_THREADS,
                       wxT("Thread %p Entry() returned %lu."),
                       THR_ID(pthread), wxPtrToUInt(pthread->m_exitcode));
        }
#ifndef wxNO_EXCEPTIONS
#ifdef HAVE_ABI_FORCEDUNWIND
                                        catch ( abi::__forced_unwind& )
        {
            wxCriticalSectionLocker lock(thread->m_critsect);
            pthread->SetState(STATE_EXITED);
            throw;
        }
#endif         catch ( ... )
        {
            wxTheApp->OnUnhandledException();
        }
#endif 
        {
            wxCriticalSectionLocker lock(thread->m_critsect);

                                                pthread->SetState(STATE_EXITED);
        }
    }

        #ifdef wxHAVE_PTHREAD_CLEANUP
    #ifdef __DECCXX
                #pragma message save
        #pragma message disable(declbutnotref)
    #endif

        pthread_cleanup_pop(FALSE);

    #ifdef __DECCXX
        #pragma message restore
    #endif
#endif 
    if ( dontRunAtAll )
    {
                delete thread;

        return EXITCODE_CANCELLED;
    }
    else
    {
                thread->Exit(pthread->m_exitcode);

        wxFAIL_MSG(wxT("wxThread::Exit() can't return."));

        return NULL;
    }
}

#ifdef wxHAVE_PTHREAD_CLEANUP

extern "C" void wxPthreadCleanup(void *ptr)
{
    wxThreadInternal::Cleanup((wxThread *)ptr);
}

void wxThreadInternal::Cleanup(wxThread *thread)
{
    if (pthread_getspecific(gs_keySelf) == 0) return;
    {
        wxCriticalSectionLocker lock(thread->m_critsect);
        if ( thread->m_internal->GetState() == STATE_EXITED )
        {
                        return;
        }
    }

        thread->Exit(EXITCODE_CANCELLED);
}

#endif 

wxThreadInternal::wxThreadInternal()
{
    m_state = STATE_NEW;
    m_created = false;
    m_cancelled = false;
    m_prio = wxPRIORITY_DEFAULT;
    m_threadId = 0;
    m_exitcode = 0;

        m_isPaused = false;

        m_shouldBeJoined = true;
    m_isDetached = false;
}

wxThreadInternal::~wxThreadInternal()
{
}

#ifdef HAVE_PTHREAD_ATTR_SETSTACKSIZE
    #define WXUNUSED_STACKSIZE(identifier)  identifier
#else
    #define WXUNUSED_STACKSIZE(identifier)  WXUNUSED(identifier)
#endif

wxThreadError wxThreadInternal::Create(wxThread *thread,
                                       unsigned int WXUNUSED_STACKSIZE(stackSize))
{
    if ( GetState() != STATE_NEW )
    {
                return wxTHREAD_RUNNING;
    }

        pthread_attr_t attr;
    pthread_attr_init(&attr);

#ifdef HAVE_PTHREAD_ATTR_SETSTACKSIZE
    if (stackSize)
      pthread_attr_setstacksize(&attr, stackSize);
#endif

#ifdef HAVE_THREAD_PRIORITY_FUNCTIONS
    int policy;
    if ( pthread_attr_getschedpolicy(&attr, &policy) != 0 )
    {
        wxLogError(_("Cannot retrieve thread scheduling policy."));
    }

#ifdef __VMS__
   
# undef sched_get_priority_max
#undef sched_get_priority_min
#define sched_get_priority_max(_pol_) \
     (_pol_ == SCHED_OTHER ? PRI_FG_MAX_NP : PRI_FIFO_MAX)
#define sched_get_priority_min(_pol_) \
     (_pol_ == SCHED_OTHER ? PRI_FG_MIN_NP : PRI_FIFO_MIN)
#endif

    int max_prio = sched_get_priority_max(policy),
        min_prio = sched_get_priority_min(policy),
        prio = GetPriority();

    if ( min_prio == -1 || max_prio == -1 )
    {
        wxLogError(_("Cannot get priority range for scheduling policy %d."),
                   policy);
    }
    else if ( max_prio == min_prio )
    {
        if ( prio != wxPRIORITY_DEFAULT )
        {
                        wxLogWarning(_("Thread priority setting is ignored."));
        }
        
            }
    else
    {
        struct sched_param sp;
        if ( pthread_attr_getschedparam(&attr, &sp) != 0 )
        {
            wxFAIL_MSG(wxT("pthread_attr_getschedparam() failed"));
        }

        sp.sched_priority = min_prio + (prio*(max_prio - min_prio))/100;

        if ( pthread_attr_setschedparam(&attr, &sp) != 0 )
        {
            wxFAIL_MSG(wxT("pthread_attr_setschedparam(priority) failed"));
        }
    }
#endif 
#ifdef HAVE_PTHREAD_ATTR_SETSCOPE
        if ( pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM) != 0 )
    {
        wxFAIL_MSG(wxT("pthread_attr_setscope(PTHREAD_SCOPE_SYSTEM) failed"));
    }
#endif 
                if ( thread->IsDetached() )
    {
        if ( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0 )
        {
            wxFAIL_MSG(wxT("pthread_attr_setdetachstate(DETACHED) failed"));
        }

                Detach();
    }
    
        int rc = pthread_create
             (
                GetIdPtr(),
                &attr,
                wxPthreadStart,
                (void *)thread
             );

    if ( pthread_attr_destroy(&attr) != 0 )
    {
        wxFAIL_MSG(wxT("pthread_attr_destroy() failed"));
    }

    if ( rc != 0 )
    {
        SetState(STATE_EXITED);

        return wxTHREAD_NO_RESOURCE;
    }

    m_created = true;
    return wxTHREAD_NO_ERROR;
}

wxThreadError wxThreadInternal::Run()
{
    wxCHECK_MSG( GetState() == STATE_NEW, wxTHREAD_RUNNING,
                 wxT("thread may only be started once after Create()") );

    SetState(STATE_RUNNING);

        SignalRun();

    return wxTHREAD_NO_ERROR;
}

void wxThreadInternal::Wait()
{
    wxCHECK_RET( !m_isDetached, wxT("can't wait for a detached thread") );

            if ( wxThread::IsMain() )
    {
#ifdef __WXOSX__
                                if ( wxGuiOwnedByMainThread() )
            wxMutexGuiLeave();
#else
        wxMutexGuiLeave();
#endif
    }

    wxLogTrace(TRACE_THREADS,
               wxT("Starting to wait for thread %p to exit."),
               THR_ID(this));

            {
        wxCriticalSectionLocker lock(m_csJoinFlag);

        if ( m_shouldBeJoined )
        {
                                                            if ( pthread_join(GetId(), &m_exitcode) != 0 )
            {
                                                                                wxLogError(_("Failed to join a thread, potential memory leak detected - please restart the program"));
            }

            m_shouldBeJoined = false;
        }
    }

#ifndef __WXOSX__
        if ( wxThread::IsMain() )
        wxMutexGuiEnter();
#endif
}

void wxThreadInternal::Pause()
{
            wxCHECK_RET( m_state == STATE_PAUSED,
                 wxT("thread must first be paused with wxThread::Pause().") );

   wxLogTrace(TRACE_THREADS,
              wxT("Thread %p goes to sleep."), THR_ID(this));

        m_semSuspend.Wait();
}

void wxThreadInternal::Resume()
{
    wxCHECK_RET( m_state == STATE_PAUSED,
                 wxT("can't resume thread which is not suspended.") );

            if ( IsReallyPaused() )
    {
       wxLogTrace(TRACE_THREADS,
                  wxT("Waking up thread %p"), THR_ID(this));

                m_semSuspend.Post();

                SetReallyPaused(false);
    }
    else
    {
        wxLogTrace(TRACE_THREADS,
                   wxT("Thread %p is not yet really paused"), THR_ID(this));
    }

    SetState(STATE_RUNNING);
}


wxThread *wxThread::This()
{
    return (wxThread *)pthread_getspecific(gs_keySelf);
}

void wxThread::Yield()
{
#ifdef HAVE_SCHED_YIELD
    sched_yield();
#endif
}

int wxThread::GetCPUCount()
{
#if defined(_SC_NPROCESSORS_ONLN)
        int rc = sysconf(_SC_NPROCESSORS_ONLN);
    if ( rc != -1 )
    {
        return rc;
    }
#elif defined(__LINUX__) && wxUSE_FFILE
            wxLogNull nolog;

    wxFFile file(wxT("/proc/cpuinfo"));
    if ( file.IsOpened() )
    {
                wxString s;
        if ( file.ReadAll(&s) )
        {
                        size_t count = s.Replace(wxT("processor\t:"), wxT(""));
            if ( count > 0 )
            {
                return count;
            }

            wxLogDebug(wxT("failed to parse /proc/cpuinfo"));
        }
        else
        {
            wxLogDebug(wxT("failed to read /proc/cpuinfo"));
        }
    }
#endif 
        return -1;
}

wxThreadIdType wxThread::GetCurrentId()
{
    return (wxThreadIdType)pthread_self();
}


bool wxThread::SetConcurrency(size_t level)
{
#ifdef HAVE_PTHREAD_SET_CONCURRENCY
    int rc = pthread_setconcurrency( level );
#elif defined(HAVE_THR_SETCONCURRENCY)
    int rc = thr_setconcurrency(level);
#else         int rc = level == 0 ? 0 : -1;
#endif 
    if ( rc != 0 )
    {
        wxLogSysError(rc, _("Failed to set thread concurrency level to %lu"),
                      static_cast<unsigned long>(level));
        return false;
    }

    return true;
}


wxThread::wxThread(wxThreadKind kind)
{
        {
        wxMutexLocker lock(*gs_mutexAllThreads);

        gs_allThreads.Add(this);
    }

    m_internal = new wxThreadInternal();

    m_isDetached = kind == wxTHREAD_DETACHED;
}

wxThreadError wxThread::Create(unsigned int stackSize)
{
    wxCriticalSectionLocker lock(m_critsect);

    return m_internal->Create(this, stackSize);
}

wxThreadError wxThread::Run()
{
    wxCriticalSectionLocker lock(m_critsect);

            if ( !m_internal->WasCreated() )
    {
        wxThreadError rv = m_internal->Create(this, 0);
        if ( rv != wxTHREAD_NO_ERROR )
            return rv;
    }

    return m_internal->Run();
}


void wxThread::SetPriority(unsigned int prio)
{
    wxCHECK_RET( wxPRIORITY_MIN <= prio && prio <= wxPRIORITY_MAX,
                 wxT("invalid thread priority") );

    wxCriticalSectionLocker lock(m_critsect);

    switch ( m_internal->GetState() )
    {
        case STATE_NEW:
                        m_internal->SetPriority(prio);
            break;

        case STATE_RUNNING:
        case STATE_PAUSED:
            {
#ifdef HAVE_THREAD_PRIORITY_FUNCTIONS
                                                                                                                                                                                                struct sched_param sparam = { 0 };

                                                int policy = SCHED_OTHER;
#ifdef SCHED_IDLE
                if ( prio <= 20 )
                    policy = SCHED_IDLE;
#endif
#ifdef SCHED_BATCH
                if ( 20 < prio && prio <= 40 )
                    policy = SCHED_BATCH;
#endif
#ifdef SCHED_RR
                if ( 60 < prio && prio <= 80 )
                    policy = SCHED_RR;
#endif
#ifdef SCHED_FIFO
                if ( 80 < prio )
                    policy = SCHED_FIFO;
#endif

                                                if ( prio > 60 && policy != SCHED_OTHER )
                {
                                                                                                    
                                        sparam.sched_priority = ((prio - 61) % 20)*5 + 1;

                                                            if ( sparam.sched_priority == 96 )
                        sparam.sched_priority = 99;
                }

                if ( pthread_setschedparam(m_internal->GetId(),
                                           policy, &sparam) != 0 )
                {
                    wxLogError(_("Failed to set thread priority %d."), prio);
                }
#endif             }
            break;

        case STATE_EXITED:
        default:
            wxFAIL_MSG(wxT("impossible to set thread priority in this state"));
    }
}

unsigned int wxThread::GetPriority() const
{
    wxCriticalSectionLocker lock((wxCriticalSection &)m_critsect);

    return m_internal->GetPriority();
}

wxThreadIdType wxThread::GetId() const
{
    return (wxThreadIdType) m_internal->GetId();
}


wxThreadError wxThread::Pause()
{
    wxCHECK_MSG( This() != this, wxTHREAD_MISC_ERROR,
                 wxT("a thread can't pause itself") );

    wxCriticalSectionLocker lock(m_critsect);

    if ( m_internal->GetState() != STATE_RUNNING )
    {
        wxLogDebug(wxT("Can't pause thread which is not running."));

        return wxTHREAD_NOT_RUNNING;
    }

            m_internal->SetState(STATE_PAUSED);

    return wxTHREAD_NO_ERROR;
}

wxThreadError wxThread::Resume()
{
    wxCHECK_MSG( This() != this, wxTHREAD_MISC_ERROR,
                 wxT("a thread can't resume itself") );

    wxCriticalSectionLocker lock(m_critsect);

    wxThreadState state = m_internal->GetState();

    switch ( state )
    {
        case STATE_PAUSED:
            wxLogTrace(TRACE_THREADS, wxT("Thread %p suspended, resuming."),
                       THR_ID(this));

            m_internal->Resume();

            return wxTHREAD_NO_ERROR;

        case STATE_EXITED:
            wxLogTrace(TRACE_THREADS, wxT("Thread %p exited, won't resume."),
                       THR_ID(this));
            return wxTHREAD_NO_ERROR;

        default:
            wxLogDebug(wxT("Attempt to resume a thread which is not paused."));

            return wxTHREAD_MISC_ERROR;
    }
}


wxThread::ExitCode wxThread::Wait(wxThreadWait WXUNUSED(waitMode))
{
    wxCHECK_MSG( This() != this, (ExitCode)-1,
                 wxT("a thread can't wait for itself") );

    wxCHECK_MSG( !m_isDetached, (ExitCode)-1,
                 wxT("can't wait for detached thread") );

    m_internal->Wait();

    return m_internal->GetExitCode();
}

wxThreadError wxThread::Delete(ExitCode *rc, wxThreadWait WXUNUSED(waitMode))
{
    wxCHECK_MSG( This() != this, wxTHREAD_MISC_ERROR,
                 wxT("a thread can't delete itself") );

    bool isDetached = m_isDetached;

    m_critsect.Enter();
    wxThreadState state = m_internal->GetState();

        m_internal->SetCancelFlag();

    m_critsect.Leave();

    OnDelete();

    switch ( state )
    {
        case STATE_NEW:
                                                m_internal->SignalRun();

            wxFALLTHROUGH;

        case STATE_EXITED:
                        break;

        case STATE_PAUSED:
                        m_internal->Resume();

            wxFALLTHROUGH;

        default:
            if ( !isDetached )
            {
                                m_internal->Wait();

                if ( rc )
                {
                                        *rc = m_internal->GetExitCode();
                }
            }
                }

    if (state == STATE_NEW)
        return wxTHREAD_MISC_ERROR;
                        
    return wxTHREAD_NO_ERROR;
}

wxThreadError wxThread::Kill()
{
    wxCHECK_MSG( This() != this, wxTHREAD_MISC_ERROR,
                 wxT("a thread can't kill itself") );

    OnKill();

    switch ( m_internal->GetState() )
    {
        case STATE_NEW:
        case STATE_EXITED:
            return wxTHREAD_NOT_RUNNING;

        case STATE_PAUSED:
                        Resume();

            wxFALLTHROUGH;

        default:
#ifdef HAVE_PTHREAD_CANCEL
            if ( pthread_cancel(m_internal->GetId()) != 0 )
#endif             {
                wxLogError(_("Failed to terminate a thread."));

                return wxTHREAD_MISC_ERROR;
            }

#ifdef HAVE_PTHREAD_CANCEL
            if ( m_isDetached )
            {
                                #ifndef wxHAVE_PTHREAD_CLEANUP
                ScheduleThreadForDeletion();

                                
                DeleteThread(this);
#endif             }
            else
            {
                m_internal->SetExitCode(EXITCODE_CANCELLED);
            }

            return wxTHREAD_NO_ERROR;
#endif     }
}

void wxThread::Exit(ExitCode status)
{
    wxASSERT_MSG( This() == this,
                  wxT("wxThread::Exit() can only be called in the context of the same thread") );

    if ( m_isDetached )
    {
                                ScheduleThreadForDeletion();
    }

                    wxTRY
    {
        OnExit();
    }
    wxCATCH_ALL( wxTheApp->OnUnhandledException(); )

            if ( m_isDetached )
    {
                                                        DeleteThread(this);
        pthread_setspecific(gs_keySelf, 0);
    }
    else
    {
        m_critsect.Enter();
        m_internal->SetState(STATE_EXITED);
        m_critsect.Leave();
    }

        pthread_exit(status);

    wxFAIL_MSG(wxT("pthread_exit() failed"));
}

bool wxThread::TestDestroy()
{
    wxASSERT_MSG( This() == this,
                  wxT("wxThread::TestDestroy() can only be called in the context of the same thread") );

    m_critsect.Enter();

    if ( m_internal->GetState() == STATE_PAUSED )
    {
        m_internal->SetReallyPaused(true);

                                m_critsect.Leave();

        m_internal->Pause();
    }
    else
    {
                m_critsect.Leave();
    }

    return m_internal->WasCancelled();
}

wxThread::~wxThread()
{
    m_critsect.Enter();

        if ( m_internal->GetState() != STATE_EXITED &&
         m_internal->GetState() != STATE_NEW )
    {
        wxLogDebug(wxT("The thread %p is being destroyed although it is still running! The application may crash."),
                   THR_ID(this));
    }

    m_critsect.Leave();

    delete m_internal;

        {
        wxMutexLocker lock(*gs_mutexAllThreads);

        gs_allThreads.Remove(this);
    }
}


bool wxThread::IsRunning() const
{
    wxCriticalSectionLocker lock((wxCriticalSection &)m_critsect);

    return m_internal->GetState() == STATE_RUNNING;
}

bool wxThread::IsAlive() const
{
    wxCriticalSectionLocker lock((wxCriticalSection&)m_critsect);

    switch ( m_internal->GetState() )
    {
        case STATE_RUNNING:
        case STATE_PAUSED:
            return true;

        default:
            return false;
    }
}

bool wxThread::IsPaused() const
{
    wxCriticalSectionLocker lock((wxCriticalSection&)m_critsect);

    return (m_internal->GetState() == STATE_PAUSED);
}


#ifdef __WXOSX__
void wxOSXThreadModuleOnInit();
void wxOSXThreadModuleOnExit();
#endif

class wxThreadModule : public wxModule
{
public:
    virtual bool OnInit() wxOVERRIDE;
    virtual void OnExit() wxOVERRIDE;

private:
    wxDECLARE_DYNAMIC_CLASS(wxThreadModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxThreadModule, wxModule);

bool wxThreadModule::OnInit()
{
    int rc = pthread_key_create(&gs_keySelf, NULL );
    if ( rc != 0 )
    {
        wxLogSysError(rc, _("Thread module initialization failed: failed to create thread key"));

        return false;
    }

    wxThread::ms_idMainThread = wxThread::GetCurrentId();

    gs_mutexAllThreads = new wxMutex();

#ifdef __WXOSX__
    wxOSXThreadModuleOnInit();
#else
    gs_mutexGui = new wxMutex();
    gs_mutexGui->Lock();
#endif

    gs_mutexDeleteThread = new wxMutex();
    gs_condAllDeleted = new wxCondition(*gs_mutexDeleteThread);

    return true;
}

void wxThreadModule::OnExit()
{
    wxASSERT_MSG( wxThread::IsMain(), wxT("only main thread can be here") );

        size_t nThreadsBeingDeleted;

    {
        wxMutexLocker lock( *gs_mutexDeleteThread );
        nThreadsBeingDeleted = gs_nThreadsBeingDeleted;

        if ( nThreadsBeingDeleted > 0 )
        {
            wxLogTrace(TRACE_THREADS,
                       wxT("Waiting for %lu threads to disappear"),
                       (unsigned long)nThreadsBeingDeleted);

                        gs_condAllDeleted->Wait();
        }
    }

    size_t count;

    {
        wxMutexLocker lock(*gs_mutexAllThreads);

                count = gs_allThreads.GetCount();
        if ( count != 0u )
        {
            wxLogDebug(wxT("%lu threads were not terminated by the application."),
                       (unsigned long)count);
        }
    } 
    for ( size_t n = 0u; n < count; n++ )
    {
                        gs_allThreads[0]->Delete();
    }

    delete gs_mutexAllThreads;

#ifdef __WXOSX__
    wxOSXThreadModuleOnExit();
#else
        gs_mutexGui->Unlock();
    delete gs_mutexGui;
#endif

        (void)pthread_key_delete(gs_keySelf);

    delete gs_condAllDeleted;
    delete gs_mutexDeleteThread;
}


static void ScheduleThreadForDeletion()
{
    wxMutexLocker lock( *gs_mutexDeleteThread );

    gs_nThreadsBeingDeleted++;

    wxLogTrace(TRACE_THREADS, wxT("%lu thread%s waiting to be deleted"),
               (unsigned long)gs_nThreadsBeingDeleted,
               gs_nThreadsBeingDeleted == 1 ? wxT("") : wxT("s"));
}

static void DeleteThread(wxThread *This)
{
    wxLogTrace(TRACE_THREADS, wxT("Thread %p auto deletes."), THR_ID(This));

    delete This;

                wxMutexLocker locker( *gs_mutexDeleteThread );

    wxCHECK_RET( gs_nThreadsBeingDeleted > 0,
                 wxT("no threads scheduled for deletion, yet we delete one?") );

    wxLogTrace(TRACE_THREADS, wxT("%lu threads remain scheduled for deletion."),
               (unsigned long)gs_nThreadsBeingDeleted - 1);

    if ( !--gs_nThreadsBeingDeleted )
    {
                gs_condAllDeleted->Signal();
    }
}

#ifndef __DARWIN__

void wxMutexGuiEnterImpl()
{
    gs_mutexGui->Lock();
}

void wxMutexGuiLeaveImpl()
{
    gs_mutexGui->Unlock();
}

#endif


#include "wx/thrimpl.cpp"

#endif 