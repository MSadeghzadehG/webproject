
#ifndef _WX_THREAD_H_
#define _WX_THREAD_H_


#include "wx/defs.h"

#if wxUSE_THREADS


enum wxMutexError
{
    wxMUTEX_NO_ERROR = 0,       wxMUTEX_INVALID,            wxMUTEX_DEAD_LOCK,          wxMUTEX_BUSY,               wxMUTEX_UNLOCKED,           wxMUTEX_TIMEOUT,            wxMUTEX_MISC_ERROR      };

enum wxCondError
{
    wxCOND_NO_ERROR = 0,
    wxCOND_INVALID,
    wxCOND_TIMEOUT,             wxCOND_MISC_ERROR
};

enum wxSemaError
{
    wxSEMA_NO_ERROR = 0,
    wxSEMA_INVALID,             wxSEMA_BUSY,                wxSEMA_TIMEOUT,             wxSEMA_OVERFLOW,            wxSEMA_MISC_ERROR
};

enum wxThreadError
{
    wxTHREAD_NO_ERROR = 0,          wxTHREAD_NO_RESOURCE,           wxTHREAD_RUNNING,               wxTHREAD_NOT_RUNNING,           wxTHREAD_KILLED,                wxTHREAD_MISC_ERROR         };

enum wxThreadKind
{
    wxTHREAD_DETACHED,
    wxTHREAD_JOINABLE
};

enum wxThreadWait
{
    wxTHREAD_WAIT_BLOCK,
    wxTHREAD_WAIT_YIELD,       
            #if WXWIN_COMPATIBILITY_2_8
    wxTHREAD_WAIT_DEFAULT = wxTHREAD_WAIT_YIELD
#else
    wxTHREAD_WAIT_DEFAULT = wxTHREAD_WAIT_BLOCK
#endif
};

enum
{
    WXTHREAD_MIN_PRIORITY      = wxPRIORITY_MIN,
    WXTHREAD_DEFAULT_PRIORITY  = wxPRIORITY_DEFAULT,
    WXTHREAD_MAX_PRIORITY      = wxPRIORITY_MAX
};

enum wxMutexType
{
        wxMUTEX_DEFAULT,

        wxMUTEX_RECURSIVE
};

class WXDLLIMPEXP_FWD_BASE wxThreadHelper;
class WXDLLIMPEXP_FWD_BASE wxConditionInternal;
class WXDLLIMPEXP_FWD_BASE wxMutexInternal;
class WXDLLIMPEXP_FWD_BASE wxSemaphoreInternal;
class WXDLLIMPEXP_FWD_BASE wxThreadInternal;


class WXDLLIMPEXP_BASE wxMutex
{
public:
        
        wxMutex(wxMutexType mutexType = wxMUTEX_DEFAULT);

        ~wxMutex();

        bool IsOk() const;

        
                        wxMutexError Lock();

            wxMutexError LockTimeout(unsigned long ms);

            wxMutexError TryLock();

        wxMutexError Unlock();

protected:
    wxMutexInternal *m_internal;

    friend class wxConditionInternal;

    wxDECLARE_NO_COPY_CLASS(wxMutex);
};

class WXDLLIMPEXP_BASE wxMutexLocker
{
public:
        wxMutexLocker(wxMutex& mutex)
        : m_isOk(false), m_mutex(mutex)
        { m_isOk = ( m_mutex.Lock() == wxMUTEX_NO_ERROR ); }

        bool IsOk() const
        { return m_isOk; }

        ~wxMutexLocker()
        { if ( IsOk() ) m_mutex.Unlock(); }

private:
        wxMutexLocker(const wxMutexLocker&);
    wxMutexLocker& operator=(const wxMutexLocker&);

    bool     m_isOk;
    wxMutex& m_mutex;
};


#if !defined(__WINDOWS__)
    #define wxCRITSECT_IS_MUTEX 1

    #define wxCRITSECT_INLINE WXEXPORT inline
#else     #define wxCRITSECT_IS_MUTEX 0

    #define wxCRITSECT_INLINE
#endif 
enum wxCriticalSectionType
{
        wxCRITSEC_DEFAULT,

        wxCRITSEC_NON_RECURSIVE
};

class WXDLLIMPEXP_BASE wxCriticalSection
{
public:
        wxCRITSECT_INLINE wxCriticalSection( wxCriticalSectionType critSecType = wxCRITSEC_DEFAULT );
    wxCRITSECT_INLINE ~wxCriticalSection();
        wxCRITSECT_INLINE void Enter();

        wxCRITSECT_INLINE bool TryEnter();

        wxCRITSECT_INLINE void Leave();

private:
#if wxCRITSECT_IS_MUTEX
    wxMutex m_mutex;
#elif defined(__WINDOWS__)
                            #ifdef __WIN64__
    typedef char wxCritSectBuffer[40];
#else     typedef char wxCritSectBuffer[24];
#endif
    union
    {
        unsigned long m_dummy1;
        void *m_dummy2;

        wxCritSectBuffer m_buffer;
    };
#endif 
    wxDECLARE_NO_COPY_CLASS(wxCriticalSection);
};

#if wxCRITSECT_IS_MUTEX
        inline wxCriticalSection::wxCriticalSection( wxCriticalSectionType critSecType )
       : m_mutex( critSecType == wxCRITSEC_DEFAULT ? wxMUTEX_RECURSIVE : wxMUTEX_DEFAULT )  { }
    inline wxCriticalSection::~wxCriticalSection() { }

    inline void wxCriticalSection::Enter() { (void)m_mutex.Lock(); }
    inline bool wxCriticalSection::TryEnter() { return m_mutex.TryLock() == wxMUTEX_NO_ERROR; }
    inline void wxCriticalSection::Leave() { (void)m_mutex.Unlock(); }
#endif 
#undef wxCRITSECT_INLINE
#undef wxCRITSECT_IS_MUTEX

class WXDLLIMPEXP_BASE wxCriticalSectionLocker
{
public:
    wxCriticalSectionLocker(wxCriticalSection& cs)
        : m_critsect(cs)
    {
        m_critsect.Enter();
    }

    ~wxCriticalSectionLocker()
    {
        m_critsect.Leave();
    }

private:
    wxCriticalSection& m_critsect;

    wxDECLARE_NO_COPY_CLASS(wxCriticalSectionLocker);
};


class WXDLLIMPEXP_BASE wxCondition
{
public:
            wxCondition(wxMutex& mutex);

        ~wxCondition();

        bool IsOk() const;

                            wxCondError Wait();

        template<typename Functor>
    wxCondError Wait(const Functor& predicate)
    {
        while ( !predicate() )
        {
            wxCondError e = Wait();
            if ( e != wxCOND_NO_ERROR )
                return e;
        }
        return wxCOND_NO_ERROR;
    }

                                wxCondError WaitTimeout(unsigned long milliseconds);

                            wxCondError Signal();

                            wxCondError Broadcast();

private:
    wxConditionInternal *m_internal;

    wxDECLARE_NO_COPY_CLASS(wxCondition);
};


class WXDLLIMPEXP_BASE wxSemaphore
{
public:
            wxSemaphore( int initialcount = 0, int maxcount = 0 );

        ~wxSemaphore();

        bool IsOk() const;

                wxSemaError Wait();

            wxSemaError TryWait();

            wxSemaError WaitTimeout(unsigned long milliseconds);

        wxSemaError Post();

private:
    wxSemaphoreInternal *m_internal;

    wxDECLARE_NO_COPY_CLASS(wxSemaphore);
};




#ifdef __VMS
   typedef unsigned long long wxThreadIdType;
#else
   typedef unsigned long wxThreadIdType;
#endif

class WXDLLIMPEXP_BASE wxThread
{
public:
        typedef void *ExitCode;

                                                static wxThread *This();

                                            static bool IsMain()
    {
        return !ms_idMainThread || GetCurrentId() == ms_idMainThread;
    }

            static wxThreadIdType GetMainId() { return ms_idMainThread; }

            static void Yield();

                            static void Sleep(unsigned long milliseconds);

                                    static int GetCPUCount();

                            static wxThreadIdType GetCurrentId();

                                                            static bool SetConcurrency(size_t level);

            wxThread(wxThreadKind kind = wxTHREAD_DETACHED);

            
                    wxThreadError Create(unsigned int stackSize = 0);

                            wxThreadError Run();

                                                                                    wxThreadError Delete(ExitCode *rc = NULL,
                         wxThreadWait waitMode = wxTHREAD_WAIT_DEFAULT);

                                    ExitCode Wait(wxThreadWait waitMode = wxTHREAD_WAIT_DEFAULT);

                                                                    wxThreadError Kill();

                    wxThreadError Pause();

            wxThreadError Resume();

                                                void SetPriority(unsigned int prio);

            unsigned int GetPriority() const;

                bool IsAlive() const;
            bool IsRunning() const;
            bool IsPaused() const;

            bool IsDetached() const { return m_isDetached; }

            wxThreadIdType GetId() const;

#ifdef __WINDOWS__
        WXHANDLE MSWGetHandle() const;
#endif 
    wxThreadKind GetKind() const
        { return m_isDetached ? wxTHREAD_DETACHED : wxTHREAD_JOINABLE; }

                virtual bool TestDestroy();

            virtual ~wxThread();

protected:
        void Exit(ExitCode exitcode = 0);

            virtual void *Entry() = 0;

        void *CallEntry();

            
            virtual void OnDelete() {}

            virtual void OnKill() {}

private:
        wxThread(const wxThread&);
    wxThread& operator=(const wxThread&);

                virtual void OnExit() { }

    friend class wxThreadInternal;
    friend class wxThreadModule;


        static wxThreadIdType ms_idMainThread;

        wxThreadInternal *m_internal;

        wxCriticalSection m_critsect;

        bool m_isDetached;
};


class WXDLLIMPEXP_BASE wxThreadHelperThread : public wxThread
{
public:
            wxThreadHelperThread(wxThreadHelper& owner, wxThreadKind kind)
        : wxThread(kind), m_owner(owner)
        { }

protected:
        virtual void *Entry() wxOVERRIDE;

private:
        wxThreadHelper& m_owner;

        wxThreadHelperThread(const wxThreadHelperThread&);
    wxThreadHelperThread& operator=(const wxThreadHelperThread&);
};


class WXDLLIMPEXP_BASE wxThreadHelper
{
private:
    void KillThread()
    {
                                        wxCriticalSectionLocker locker(m_critSection);

        if ( m_thread )
        {
            m_thread->Kill();

            if ( m_kind == wxTHREAD_JOINABLE )
              delete m_thread;

            m_thread = NULL;
        }
    }

public:
        wxThreadHelper(wxThreadKind kind = wxTHREAD_JOINABLE)
        : m_thread(NULL), m_kind(kind) { }

        virtual ~wxThreadHelper() { KillThread(); }

#if WXWIN_COMPATIBILITY_2_8
    wxDEPRECATED( wxThreadError Create(unsigned int stackSize = 0) );
#endif

            wxThreadError CreateThread(wxThreadKind kind = wxTHREAD_JOINABLE,
                               unsigned int stackSize = 0)
    {
        KillThread();

        m_kind = kind;
        m_thread = new wxThreadHelperThread(*this, m_kind);

        return m_thread->Create(stackSize);
    }

            virtual void *Entry() = 0;

        wxThread *GetThread() const
    {
        wxCriticalSectionLocker locker((wxCriticalSection&)m_critSection);

        wxThread* thread = m_thread;

        return thread;
    }

protected:
    wxThread *m_thread;
    wxThreadKind m_kind;
    wxCriticalSection m_critSection; 
    friend class wxThreadHelperThread;
};

#if WXWIN_COMPATIBILITY_2_8
inline wxThreadError wxThreadHelper::Create(unsigned int stackSize)
{ return CreateThread(m_kind, stackSize); }
#endif

inline void *wxThreadHelperThread::Entry()
{
    void * const result = m_owner.Entry();

    wxCriticalSectionLocker locker(m_owner.m_critSection);

                    if ( m_owner.m_kind == wxTHREAD_DETACHED )
        m_owner.m_thread = NULL;

    return result;
}


void WXDLLIMPEXP_BASE wxMutexGuiEnter();
void WXDLLIMPEXP_BASE wxMutexGuiLeave();

#define wxENTER_CRIT_SECT(cs)   (cs).Enter()
#define wxLEAVE_CRIT_SECT(cs)   (cs).Leave()
#define wxCRIT_SECT_DECLARE(cs) static wxCriticalSection cs
#define wxCRIT_SECT_DECLARE_MEMBER(cs) wxCriticalSection cs
#define wxCRIT_SECT_LOCKER(name, cs)  wxCriticalSectionLocker name(cs)

inline bool wxIsMainThread() { return wxThread::IsMain(); }

#else 
inline void wxMutexGuiEnter() { }
inline void wxMutexGuiLeave() { }

#define wxENTER_CRIT_SECT(cs)            do {} while (0)
#define wxLEAVE_CRIT_SECT(cs)            do {} while (0)
#define wxCRIT_SECT_DECLARE(cs)          struct wxDummyCS##cs
#define wxCRIT_SECT_DECLARE_MEMBER(cs)   struct wxDummyCSMember##cs { }
#define wxCRIT_SECT_LOCKER(name, cs)     struct wxDummyCSLocker##name

inline bool wxIsMainThread() { return true; }

#endif 
#define wxCRITICAL_SECTION(name) \
    wxCRIT_SECT_DECLARE(s_cs##name);  \
    wxCRIT_SECT_LOCKER(cs##name##Locker, s_cs##name)

class WXDLLIMPEXP_BASE wxMutexGuiLocker
{
public:
    wxMutexGuiLocker() { wxMutexGuiEnter(); }
   ~wxMutexGuiLocker() { wxMutexGuiLeave(); }
};


#if wxUSE_THREADS

#if defined(__WINDOWS__) || defined(__DARWIN__)
                extern void WXDLLIMPEXP_BASE wxMutexGuiLeaveOrEnter();

        extern bool WXDLLIMPEXP_BASE wxGuiOwnedByMainThread();

        extern void WXDLLIMPEXP_BASE wxWakeUpMainThread();

#ifndef __DARWIN__
            extern bool WXDLLIMPEXP_BASE wxIsWaitingForThread();
#endif
#endif 
#endif 
#endif 