

#include "wx/wxprec.h"

#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#if wxUSE_FILE

#if defined(__WINDOWS__) && !defined(__GNUWIN32__)

#define   WIN32_LEAN_AND_MEAN
#define   NOSERVICE
#define   NOIME
#define   NOATOM
#define   NOGDI
#define   NOGDICAPMASKS
#define   NOMETAFILE
#define   NOMINMAX
#define   NOMSG
#define   NOOPENFILE
#define   NORASTEROPS
#define   NOSCROLL
#define   NOSOUND
#define   NOSYSMETRICS
#define   NOTEXTMETRIC
#define   NOWH
#define   NOCOMM
#define   NOKANJI
#define   NOCRYPT
#define   NOMCX

#elif (defined(__UNIX__) || defined(__GNUWIN32__))
    #include  <unistd.h>
    #include  <time.h>
    #include  <sys/stat.h>
    #ifdef __GNUWIN32__
        #include "wx/msw/wrapwin.h"
    #endif
#elif (defined(__WXSTUBS__))
        #include <io.h>
#elif (defined(__WXMAC__))
#if __MSL__ < 0x6000
    int access( const char *path, int mode ) { return 0 ; }
#else
    int _access( const char *path, int mode ) { return 0 ; }
#endif
    char* mktemp( char * path ) { return path ;}
    #include <stat.h>
    #include <unistd.h>
#else
    #error  "Please specify the header with file functions declarations."
#endif  
#include  <stdio.h>       
#include <errno.h>

#ifndef W_OK
    enum
    {
        F_OK = 0,           X_OK = 1,           W_OK = 2,           R_OK = 4        };
#endif 
#ifndef WX_PRECOMP
    #include  "wx/string.h"
    #include  "wx/intl.h"
    #include  "wx/log.h"
    #include "wx/crt.h"
#endif 
#include  "wx/filename.h"
#include  "wx/file.h"
#include  "wx/filefn.h"

#if defined(__UNIX__) && !defined(O_BINARY)
    #define   O_BINARY    (0)
#endif  


bool wxFile::Exists(const wxString& name)
{
    return wxFileExists(name);
}

bool wxFile::Access(const wxString& name, OpenMode mode)
{
    int how;

    switch ( mode )
    {
        default:
            wxFAIL_MSG(wxT("bad wxFile::Access mode parameter."));
            wxFALLTHROUGH;

        case read:
            how = R_OK;
            break;

        case write:
            how = W_OK;
            break;

        case read_write:
            how = R_OK | W_OK;
            break;
    }

    return wxAccess(name, how) == 0;
}


wxFile::wxFile(const wxString& fileName, OpenMode mode)
{
    m_fd = fd_invalid;
    m_lasterror = 0;

    Open(fileName, mode);
}

bool wxFile::CheckForError(wxFileOffset rc) const
{
    if ( rc != -1 )
        return false;

    const_cast<wxFile *>(this)->m_lasterror = errno;

    return true;
}

bool wxFile::Create(const wxString& fileName, bool bOverwrite, int accessMode)
{
            int fildes = wxOpen( fileName,
                     O_BINARY | O_WRONLY | O_CREAT |
                     (bOverwrite ? O_TRUNC : O_EXCL),
                     accessMode );
    if ( CheckForError(fildes) )
    {
        wxLogSysError(_("can't create file '%s'"), fileName);
        return false;
    }

    Attach(fildes);
    return true;
}

bool wxFile::Open(const wxString& fileName, OpenMode mode, int accessMode)
{
    int flags = O_BINARY;

    switch ( mode )
    {
        case read:
            flags |= O_RDONLY;
            break;

        case write_append:
            if ( wxFile::Exists(fileName) )
            {
                flags |= O_WRONLY | O_APPEND;
                break;
            }
                                    wxFALLTHROUGH;

        case write:
            flags |= O_WRONLY | O_CREAT | O_TRUNC;
            break;

        case write_excl:
            flags |= O_WRONLY | O_CREAT | O_EXCL;
            break;

        case read_write:
            flags |= O_RDWR;
            break;
    }

#ifdef __WINDOWS__
                accessMode &= wxS_IRUSR | wxS_IWUSR;
#endif 
    int fildes = wxOpen( fileName, flags, accessMode);

    if ( CheckForError(fildes) )
    {
        wxLogSysError(_("can't open file '%s'"), fileName);
        return false;
    }

    Attach(fildes);
    return true;
}

bool wxFile::Close()
{
    if ( IsOpened() ) {
        if ( CheckForError(wxClose(m_fd)) )
        {
            wxLogSysError(_("can't close file descriptor %d"), m_fd);
            m_fd = fd_invalid;
            return false;
        }
        else
            m_fd = fd_invalid;
    }

    return true;
}


bool wxFile::ReadAll(wxString *str, const wxMBConv& conv)
{
    wxCHECK_MSG( str, false, wxS("Output string must be non-NULL") );

    ssize_t length = Length();
    wxCHECK_MSG( (wxFileOffset)length == Length(), false, wxT("huge file not supported") );

    wxCharBuffer buf(length);
    char* p = buf.data();
    for ( ;; )
    {
        static const ssize_t READSIZE = 4096;

        ssize_t nread = Read(p, length > READSIZE ? READSIZE : length);
        if ( nread == wxInvalidOffset )
            return false;

        p += nread;
        if ( length <= nread )
            break;

        length -= nread;
    }

    *p = 0;

    wxString strTmp(buf, conv);
    str->swap(strTmp);

    return true;
}

ssize_t wxFile::Read(void *pBuf, size_t nCount)
{
    if ( !nCount )
        return 0;

    wxCHECK( (pBuf != NULL) && IsOpened(), 0 );

    ssize_t iRc = wxRead(m_fd, pBuf, nCount);

    if ( CheckForError(iRc) )
    {
        wxLogSysError(_("can't read from file descriptor %d"), m_fd);
        return wxInvalidOffset;
    }

    return iRc;
}

size_t wxFile::Write(const void *pBuf, size_t nCount)
{
    if ( !nCount )
        return 0;

    wxCHECK( (pBuf != NULL) && IsOpened(), 0 );

    ssize_t iRc = wxWrite(m_fd, pBuf, nCount);

    if ( CheckForError(iRc) )
    {
        wxLogSysError(_("can't write to file descriptor %d"), m_fd);
        iRc = 0;
    }

    return iRc;
}

bool wxFile::Write(const wxString& s, const wxMBConv& conv)
{
            if ( s.empty() )
        return true;

    const wxWX2MBbuf buf = s.mb_str(conv);

#if wxUSE_UNICODE
    const size_t size = buf.length();

    if ( !size )
    {
                                return false;
    }
#else
    const size_t size = s.length();
#endif

    return Write(buf, size) == size;
}

bool wxFile::Flush()
{
#ifdef HAVE_FSYNC
            if ( IsOpened() && GetKind() == wxFILE_KIND_DISK )
    {
        if ( CheckForError(wxFsync(m_fd)) )
        {
            wxLogSysError(_("can't flush file descriptor %d"), m_fd);
            return false;
        }
    }
#endif 
    return true;
}


wxFileOffset wxFile::Seek(wxFileOffset ofs, wxSeekMode mode)
{
    wxASSERT_MSG( IsOpened(), wxT("can't seek on closed file") );
    wxCHECK_MSG( ofs != wxInvalidOffset || mode != wxFromStart,
                 wxInvalidOffset,
                 wxT("invalid absolute file offset") );

    int origin;
    switch ( mode ) {
        default:
            wxFAIL_MSG(wxT("unknown seek origin"));
            wxFALLTHROUGH;
        case wxFromStart:
            origin = SEEK_SET;
            break;

        case wxFromCurrent:
            origin = SEEK_CUR;
            break;

        case wxFromEnd:
            origin = SEEK_END;
            break;
    }

    wxFileOffset iRc = wxSeek(m_fd, ofs, origin);
    if ( CheckForError(iRc) )
    {
        wxLogSysError(_("can't seek on file descriptor %d"), m_fd);
    }

    return iRc;
}

wxFileOffset wxFile::Tell() const
{
    wxASSERT( IsOpened() );

    wxFileOffset iRc = wxTell(m_fd);
    if ( CheckForError(iRc) )
    {
        wxLogSysError(_("can't get seek position on file descriptor %d"), m_fd);
    }

    return iRc;
}

wxFileOffset wxFile::Length() const
{
    wxASSERT( IsOpened() );

                #ifdef __LINUX__
    struct stat st;
    if ( fstat(m_fd, &st) == 0 )
    {
                        return st.st_blocks ? st.st_size : 0;
    }
    #endif 
    wxFileOffset iRc = Tell();
    if ( iRc != wxInvalidOffset ) {
        wxFileOffset iLen = const_cast<wxFile *>(this)->SeekEnd();
        if ( iLen != wxInvalidOffset ) {
                        if ( ((wxFile *)this)->Seek(iRc) == wxInvalidOffset ) {
                                iLen = wxInvalidOffset;
            }
        }

        iRc = iLen;
    }

    if ( iRc == wxInvalidOffset )
    {
                wxLogSysError(_("can't find length of file on file descriptor %d"), m_fd);
    }

    return iRc;
}

bool wxFile::Eof() const
{
    wxASSERT( IsOpened() );

    wxFileOffset iRc;

#if defined(__UNIX__) || defined(__GNUWIN32__)
        wxFileOffset ofsCur = Tell(),
    ofsMax = Length();
    if ( ofsCur == wxInvalidOffset || ofsMax == wxInvalidOffset )
        iRc = wxInvalidOffset;
    else
        iRc = ofsCur == ofsMax;
#else      iRc = wxEof(m_fd);
#endif 
    if ( iRc == 0 )
        return false;

    if ( iRc == wxInvalidOffset )
    {
        wxLogSysError(_("can't determine if the end of file is reached on descriptor %d"), m_fd);
    }

    return true;
}



wxTempFile::wxTempFile(const wxString& strName)
{
    Open(strName);
}

bool wxTempFile::Open(const wxString& strName)
{
                                wxFileName fn(strName);
    if ( !fn.IsAbsolute() )
    {
        fn.Normalize(wxPATH_NORM_ABSOLUTE);
    }

    m_strName = fn.GetFullPath();

    m_strTemp = wxFileName::CreateTempFileName(m_strName, &m_file);

    if ( m_strTemp.empty() )
    {
                return false;
    }

#ifdef __UNIX__
        mode_t mode;

    wxStructStat st;
    if ( stat( (const char*) m_strName.fn_str(), &st) == 0 )
    {
        mode = st.st_mode;
    }
    else
    {
                        mode_t mask = umask(0777);
        mode = 0666 & ~mask;
        umask(mask);
    }

    if ( chmod( (const char*) m_strTemp.fn_str(), mode) == -1 )
    {
        wxLogSysError(_("Failed to set temporary file permissions"));
    }
#endif 
    return true;
}


wxTempFile::~wxTempFile()
{
    if ( IsOpened() )
        Discard();
}

bool wxTempFile::Commit()
{
    m_file.Close();

    if ( wxFile::Exists(m_strName) && wxRemove(m_strName) != 0 ) {
        wxLogSysError(_("can't remove file '%s'"), m_strName.c_str());
        return false;
    }

    if ( !wxRenameFile(m_strTemp, m_strName)  ) {
        wxLogSysError(_("can't commit changes to file '%s'"), m_strName.c_str());
        return false;
    }

    return true;
}

void wxTempFile::Discard()
{
    m_file.Close();
    if ( wxRemove(m_strTemp) != 0 )
    {
        wxLogSysError(_("can't remove temporary file '%s'"), m_strTemp.c_str());
    }
}

#endif 
