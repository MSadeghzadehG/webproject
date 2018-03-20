


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_SNGLINST_CHECKER

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/log.h"
    #include "wx/intl.h"
    #include "wx/utils.h"           #endif 
#include "wx/file.h"

#include "wx/snglinst.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>           #include <signal.h>             #include <errno.h>

#ifdef HAVE_FCNTL
    #include <fcntl.h>
#elif defined(HAVE_FLOCK)
    #include <sys/file.h>
#else
        #error "wxSingleInstanceChecker can't be compiled on this platform"
#endif 

enum LockOperation
{
    LOCK,
    UNLOCK
};

enum LockResult
{
    LOCK_ERROR = -1,
    LOCK_EXISTS,
    LOCK_CREATED
};


#ifdef HAVE_FCNTL

static int wxLockFile(int fd, LockOperation lock)
{
        struct flock fl;
    fl.l_type = lock == LOCK ? F_WRLCK : F_UNLCK;

        fl.l_start =
    fl.l_len =
    fl.l_whence = 0;

        fl.l_pid = getpid();

    return fcntl(fd, F_SETLK, &fl);
}

#else 
static int wxLockFile(int fd, LockOperation lock)
{
    return flock(fd, lock == LOCK ? LOCK_EX | LOCK_NB : LOCK_UN);
}

#endif 

class wxSingleInstanceCheckerImpl
{
public:
    wxSingleInstanceCheckerImpl()
    {
        m_fdLock = -1;
        m_pidLocker = 0;
    }

    bool Create(const wxString& name);

    pid_t GetLockerPID() const { return m_pidLocker; }

    ~wxSingleInstanceCheckerImpl() { Unlock(); }

private:
        LockResult CreateLockFile();

        void Unlock();

        int m_fdLock;

        pid_t m_pidLocker;

        wxString m_nameLock;
};


LockResult wxSingleInstanceCheckerImpl::CreateLockFile()
{
        m_fdLock = open(m_nameLock.fn_str(),
                    O_WRONLY | O_CREAT | O_EXCL,
                    S_IRUSR | S_IWUSR);

    if ( m_fdLock != -1 )
    {
                if ( wxLockFile(m_fdLock, LOCK) == 0 )
        {
                                    m_pidLocker = getpid();

                        char buf[256];             int len = sprintf(buf, "%d", (int)m_pidLocker) + 1;

            if ( write(m_fdLock, buf, len) != len )
            {
                wxLogSysError(_("Failed to write to lock file '%s'"),
                              m_nameLock.c_str());

                Unlock();

                return LOCK_ERROR;
            }

            fsync(m_fdLock);

                        if ( chmod(m_nameLock.fn_str(), S_IRUSR | S_IWUSR) != 0 )
            {
                wxLogSysError(_("Failed to set permissions on lock file '%s'"),
                              m_nameLock.c_str());

                Unlock();

                return LOCK_ERROR;
            }

            return LOCK_CREATED;
        }
        else         {
            close(m_fdLock);
            m_fdLock = -1;

            if ( errno != EACCES && errno != EAGAIN )
            {
                wxLogSysError(_("Failed to lock the lock file '%s'"),
                              m_nameLock.c_str());

                unlink(m_nameLock.fn_str());

                return LOCK_ERROR;
            }
                                                        }
    }

        return LOCK_EXISTS;
}

bool wxSingleInstanceCheckerImpl::Create(const wxString& name)
{
    m_nameLock = name;

    switch ( CreateLockFile() )
    {
        case LOCK_EXISTS:
                        break;

        case LOCK_CREATED:
                        return true;

        case LOCK_ERROR:
                        return false;
    }

                wxStructStat stats;
    if ( wxStat(name, &stats) != 0 )
    {
        wxLogSysError(_("Failed to inspect the lock file '%s'"), name.c_str());
        return false;
    }
    if ( stats.st_uid != getuid() )
    {
        wxLogError(_("Lock file '%s' has incorrect owner."), name.c_str());
        return false;
    }
    if ( stats.st_mode != (S_IFREG | S_IRUSR | S_IWUSR) )
    {
        wxLogError(_("Lock file '%s' has incorrect permissions."), name.c_str());
        return false;
    }

            wxFile file(name, wxFile::read);
    if ( !file.IsOpened() )
    {
                                                                                wxLogError(_("Failed to access lock file."));

        return false;
    }

    char buf[256];
    ssize_t count = file.Read(buf, WXSIZEOF(buf));
    if ( count == wxInvalidOffset )
    {
        wxLogError(_("Failed to read PID from lock file."));
    }
    else
    {
        if ( sscanf(buf, "%d", (int *)&m_pidLocker) == 1 )
        {
            if ( kill(m_pidLocker, 0) != 0 )
            {
                if ( unlink(name.fn_str()) != 0 )
                {
                    wxLogError(_("Failed to remove stale lock file '%s'."),
                               name.c_str());

                                    }
                else
                {
                                                                                                                                            wxLogInfo(_("Deleted stale lock file '%s'."),
                                 name.c_str());

                                        (void)CreateLockFile();
                }
            }
                    }
        else
        {
            wxLogWarning(_("Invalid lock file '%s'."), name.c_str());
        }
    }

                return m_pidLocker != 0;
}

void wxSingleInstanceCheckerImpl::Unlock()
{
    if ( m_fdLock != -1 )
    {
        if ( unlink(m_nameLock.fn_str()) != 0 )
        {
            wxLogSysError(_("Failed to remove lock file '%s'"),
                          m_nameLock.c_str());
        }

        if ( wxLockFile(m_fdLock, UNLOCK) != 0 )
        {
            wxLogSysError(_("Failed to unlock lock file '%s'"),
                          m_nameLock.c_str());
        }

        if ( close(m_fdLock) != 0 )
        {
            wxLogSysError(_("Failed to close lock file '%s'"),
                          m_nameLock.c_str());
        }
    }

    m_pidLocker = 0;
}


bool wxSingleInstanceChecker::Create(const wxString& name,
                                     const wxString& path)
{
    wxASSERT_MSG( !m_impl,
                  wxT("calling wxSingleInstanceChecker::Create() twice?") );

        wxASSERT_MSG( !name.empty(), wxT("lock file name can't be empty") );

    m_impl = new wxSingleInstanceCheckerImpl;

    wxString fullname = path;
    if ( fullname.empty() )
    {
        fullname = wxGetHomeDir();
    }

    if ( fullname.Last() != wxT('/') )
    {
        fullname += wxT('/');
    }

    fullname << name;

    return m_impl->Create(fullname);
}

bool wxSingleInstanceChecker::DoIsAnotherRunning() const
{
    wxCHECK_MSG( m_impl, false, wxT("must call Create() first") );

    const pid_t lockerPid = m_impl->GetLockerPID();

    if ( !lockerPid )
    {
                                return false;
    }

            return lockerPid != getpid();
}

wxSingleInstanceChecker::~wxSingleInstanceChecker()
{
    delete m_impl;
}

#endif 