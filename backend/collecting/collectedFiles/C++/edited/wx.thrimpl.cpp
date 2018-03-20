


wxMutex::wxMutex(wxMutexType mutexType)
{
    m_internal = new wxMutexInternal(mutexType);

    if ( !m_internal->IsOk() )
    {
        delete m_internal;
        m_internal = NULL;
    }
}

wxMutex::~wxMutex()
{
    delete m_internal;
}

bool wxMutex::IsOk() const
{
    return m_internal != NULL;
}

wxMutexError wxMutex::Lock()
{
    wxCHECK_MSG( m_internal, wxMUTEX_INVALID,
                 wxT("wxMutex::Lock(): not initialized") );

    return m_internal->Lock();
}

wxMutexError wxMutex::LockTimeout(unsigned long ms)
{
    wxCHECK_MSG( m_internal, wxMUTEX_INVALID,
                 wxT("wxMutex::Lock(): not initialized") );

    return m_internal->Lock(ms);
}

wxMutexError wxMutex::TryLock()
{
    wxCHECK_MSG( m_internal, wxMUTEX_INVALID,
                 wxT("wxMutex::TryLock(): not initialized") );

    return m_internal->TryLock();
}

wxMutexError wxMutex::Unlock()
{
    wxCHECK_MSG( m_internal, wxMUTEX_INVALID,
                 wxT("wxMutex::Unlock(): not initialized") );

    return m_internal->Unlock();
}


#if defined(__WINDOWS__)

class wxConditionInternal
{
public:
    wxConditionInternal(wxMutex& mutex);

    bool IsOk() const { return m_mutex.IsOk() && m_semaphore.IsOk(); }

    wxCondError Wait();
    wxCondError WaitTimeout(unsigned long milliseconds);

    wxCondError Signal();
    wxCondError Broadcast();

private:
        LONG m_numWaiters;

        wxCriticalSection m_csWaiters;

    wxMutex& m_mutex;
    wxSemaphore m_semaphore;

    wxDECLARE_NO_COPY_CLASS(wxConditionInternal);
};

wxConditionInternal::wxConditionInternal(wxMutex& mutex)
                   : m_mutex(mutex)
{
            m_numWaiters = 0;
}

wxCondError wxConditionInternal::Wait()
{
        {
        wxCriticalSectionLocker lock(m_csWaiters);
        m_numWaiters++;
    }

    m_mutex.Unlock();

                    const wxSemaError err = m_semaphore.Wait();

    m_mutex.Lock();

    if ( err == wxSEMA_NO_ERROR )
    {
                return wxCOND_NO_ERROR;
    }

        {
        wxCriticalSectionLocker lock(m_csWaiters);
        m_numWaiters--;
    }

    return err == wxSEMA_TIMEOUT ? wxCOND_TIMEOUT : wxCOND_MISC_ERROR;
}

wxCondError wxConditionInternal::WaitTimeout(unsigned long milliseconds)
{
    {
        wxCriticalSectionLocker lock(m_csWaiters);
        m_numWaiters++;
    }

    m_mutex.Unlock();

    wxSemaError err = m_semaphore.WaitTimeout(milliseconds);

    m_mutex.Lock();

    if ( err == wxSEMA_NO_ERROR )
        return wxCOND_NO_ERROR;

    if ( err == wxSEMA_TIMEOUT )
    {
                                                                wxCriticalSectionLocker lock(m_csWaiters);

        err = m_semaphore.WaitTimeout(0);
        if ( err == wxSEMA_NO_ERROR )
            return wxCOND_NO_ERROR;

                        m_numWaiters--;

        return err == wxSEMA_TIMEOUT ? wxCOND_TIMEOUT : wxCOND_MISC_ERROR;
    }

        {
        wxCriticalSectionLocker lock(m_csWaiters);
        m_numWaiters--;
    }

    return wxCOND_MISC_ERROR;
}

wxCondError wxConditionInternal::Signal()
{
    wxCriticalSectionLocker lock(m_csWaiters);

    if ( m_numWaiters > 0 )
    {
                if ( m_semaphore.Post() != wxSEMA_NO_ERROR )
            return wxCOND_MISC_ERROR;

        m_numWaiters--;
    }

    return wxCOND_NO_ERROR;
}

wxCondError wxConditionInternal::Broadcast()
{
    wxCriticalSectionLocker lock(m_csWaiters);

    while ( m_numWaiters > 0 )
    {
        if ( m_semaphore.Post() != wxSEMA_NO_ERROR )
            return wxCOND_MISC_ERROR;

        m_numWaiters--;
    }

    return wxCOND_NO_ERROR;
}

#endif 

wxCondition::wxCondition(wxMutex& mutex)
{
    m_internal = new wxConditionInternal(mutex);

    if ( !m_internal->IsOk() )
    {
        delete m_internal;
        m_internal = NULL;
    }
}

wxCondition::~wxCondition()
{
    delete m_internal;
}

bool wxCondition::IsOk() const
{
    return m_internal != NULL;
}

wxCondError wxCondition::Wait()
{
    wxCHECK_MSG( m_internal, wxCOND_INVALID,
                 wxT("wxCondition::Wait(): not initialized") );

    return m_internal->Wait();
}

wxCondError wxCondition::WaitTimeout(unsigned long milliseconds)
{
    wxCHECK_MSG( m_internal, wxCOND_INVALID,
                 wxT("wxCondition::Wait(): not initialized") );

    return m_internal->WaitTimeout(milliseconds);
}

wxCondError wxCondition::Signal()
{
    wxCHECK_MSG( m_internal, wxCOND_INVALID,
                 wxT("wxCondition::Signal(): not initialized") );

    return m_internal->Signal();
}

wxCondError wxCondition::Broadcast()
{
    wxCHECK_MSG( m_internal, wxCOND_INVALID,
                 wxT("wxCondition::Broadcast(): not initialized") );

    return m_internal->Broadcast();
}


wxSemaphore::wxSemaphore(int initialcount, int maxcount)
{
    m_internal = new wxSemaphoreInternal( initialcount, maxcount );
    if ( !m_internal->IsOk() )
    {
        delete m_internal;
        m_internal = NULL;
    }
}

wxSemaphore::~wxSemaphore()
{
    delete m_internal;
}

bool wxSemaphore::IsOk() const
{
    return m_internal != NULL;
}

wxSemaError wxSemaphore::Wait()
{
    wxCHECK_MSG( m_internal, wxSEMA_INVALID,
                 wxT("wxSemaphore::Wait(): not initialized") );

    return m_internal->Wait();
}

wxSemaError wxSemaphore::TryWait()
{
    wxCHECK_MSG( m_internal, wxSEMA_INVALID,
                 wxT("wxSemaphore::TryWait(): not initialized") );

    return m_internal->TryWait();
}

wxSemaError wxSemaphore::WaitTimeout(unsigned long milliseconds)
{
    wxCHECK_MSG( m_internal, wxSEMA_INVALID,
                 wxT("wxSemaphore::WaitTimeout(): not initialized") );

    return m_internal->WaitTimeout(milliseconds);
}

wxSemaError wxSemaphore::Post()
{
    wxCHECK_MSG( m_internal, wxSEMA_INVALID,
                 wxT("wxSemaphore::Post(): not initialized") );

    return m_internal->Post();
}


#include "wx/utils.h"
#include "wx/private/threadinfo.h"
#include "wx/scopeguard.h"

void wxThread::Sleep(unsigned long milliseconds)
{
    wxMilliSleep(milliseconds);
}

void *wxThread::CallEntry()
{
    wxON_BLOCK_EXIT0(wxThreadSpecificInfo::ThreadCleanUp);
    return Entry();
}
