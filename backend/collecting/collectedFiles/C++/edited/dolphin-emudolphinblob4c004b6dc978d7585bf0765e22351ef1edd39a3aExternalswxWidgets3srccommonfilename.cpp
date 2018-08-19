




#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #ifdef __WINDOWS__
        #include "wx/msw/wrapwin.h"     #endif
    #include "wx/dynarray.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/utils.h"
    #include "wx/crt.h"
#endif

#include "wx/filename.h"
#include "wx/private/filename.h"
#include "wx/tokenzr.h"
#include "wx/config.h"          #include "wx/dynlib.h"
#include "wx/dir.h"
#include "wx/longlong.h"

#if defined(__WIN32__) && defined(__MINGW32__)
    #include "wx/msw/gccpriv.h"
#endif

#ifdef __WINDOWS__
    #include "wx/msw/private.h"
    #include <shlobj.h>             #include "wx/msw/missing.h"
    #include "wx/msw/ole/oleutils.h"
    #include "wx/msw/private/comptr.h"
#endif

#if defined(__WXMAC__)
  #include  "wx/osx/private.h"  #endif

#ifdef __UNIX_LIKE__
#include <sys/types.h>
#include <utime.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifndef S_ISREG
    #define S_ISREG(mode) ((mode) & S_IFREG)
#endif
#ifndef S_ISDIR
    #define S_ISDIR(mode) ((mode) & S_IFDIR)
#endif

#if wxUSE_LONGLONG
extern const wxULongLong wxInvalidSize = (unsigned)-1;
#endif 
namespace
{


#if defined(__WIN32__)

class wxFileHandle
{
public:
    enum OpenMode
    {
        ReadAttr,
        WriteAttr
    };

    wxFileHandle(const wxString& filename, OpenMode mode, int flags = 0)
    {
                                        m_hFile = ::CreateFile
                    (
                     filename.t_str(),                                  mode == ReadAttr ? FILE_READ_ATTRIBUTES                                          : FILE_WRITE_ATTRIBUTES,
                     FILE_SHARE_READ |                                   FILE_SHARE_WRITE,                                   NULL,                                               OPEN_EXISTING,                                      flags,                                              NULL                                               );

        if ( m_hFile == INVALID_HANDLE_VALUE )
        {
            if ( mode == ReadAttr )
            {
                wxLogSysError(_("Failed to open '%s' for reading"),
                              filename.c_str());
            }
            else
            {
                wxLogSysError(_("Failed to open '%s' for writing"),
                              filename.c_str());
            }
        }
    }

    ~wxFileHandle()
    {
        if ( m_hFile != INVALID_HANDLE_VALUE )
        {
            if ( !::CloseHandle(m_hFile) )
            {
                wxLogSysError(_("Failed to close file handle"));
            }
        }
    }

        bool IsOk() const { return m_hFile != INVALID_HANDLE_VALUE; }

        operator HANDLE() const { return m_hFile; }

private:
    HANDLE m_hFile;
};

#endif 

#if wxUSE_DATETIME && defined(__WIN32__)

static wxInt64 EPOCH_OFFSET_IN_MSEC = wxLL(11644473600000);

static void ConvertFileTimeToWx(wxDateTime *dt, const FILETIME &ft)
{
    wxLongLong t(ft.dwHighDateTime, ft.dwLowDateTime);
    t /= 10000;     t -= EPOCH_OFFSET_IN_MSEC;

    *dt = wxDateTime(t);
}

static void ConvertWxToFileTime(FILETIME *ft, const wxDateTime& dt)
{
        wxLongLong t(dt.GetValue());
    t += EPOCH_OFFSET_IN_MSEC;
    t *= 10000;

    ft->dwHighDateTime = t.GetHi();
    ft->dwLowDateTime = t.GetLo();
}

#endif 
static wxString wxGetVolumeString(const wxString& volume, wxPathFormat format)
{
    wxString path;

    if ( !volume.empty() )
    {
        format = wxFileName::GetFormat(format);

                                        if ( format == wxPATH_DOS && volume.length() > 1 )
        {
                                    if ( wxFileName::IsMSWUniqueVolumeNamePath("\\\\?\\" + volume + "\\",
                                                       format) )
            {
                path << "\\\\?\\" << volume;
            }
            else
            {
                                path << wxFILE_SEP_PATH_DOS << wxFILE_SEP_PATH_DOS << volume;
            }
        }
        else if  ( format == wxPATH_DOS || format == wxPATH_VMS )
        {
            path << volume << wxFileName::GetVolumeSeparator(format);
        }
            }

    return path;
}

inline bool IsDOSPathSep(wxUniChar ch)
{
    return ch == wxFILE_SEP_PATH_DOS || ch == wxFILE_SEP_PATH_UNIX;
}

static bool IsUNCPath(const wxString& path, wxPathFormat format)
{
    return format == wxPATH_DOS &&
                path.length() >= 4 &&                     IsDOSPathSep(path[0u]) &&
                        IsDOSPathSep(path[1u]) &&
                            !IsDOSPathSep(path[2u]);
}


#if defined(__UNIX_LIKE__) || defined(__WXMAC__)
    #define wxHAVE_LSTAT
#endif

#ifdef wxHAVE_LSTAT

bool DoStatAny(wxStructStat& st, wxString path, bool dereference)
{
                    
    while ( wxEndsWithPathSeparator(path) )
    {
        const size_t posLast = path.length() - 1;
        if ( !posLast )
        {
                        break;
        }

        path.erase(posLast);
    }

    int ret = dereference ? wxStat(path, &st) : wxLstat(path, &st);
    return ret == 0;
}

inline
bool StatAny(wxStructStat& st, const wxString& path, int flags)
{
    return DoStatAny(st, path, !(flags & wxFILE_EXISTS_NO_FOLLOW));
}

inline
bool StatAny(wxStructStat& st, const wxFileName& fn)
{
    return DoStatAny(st, fn.GetFullPath(), fn.ShouldFollowLink());
}

#endif 

static const size_t wxMSWUniqueVolumePrefixLength = 49;

} 


void wxFileName::Assign( const wxFileName &filepath )
{
    m_volume = filepath.GetVolume();
    m_dirs = filepath.GetDirs();
    m_name = filepath.GetName();
    m_ext = filepath.GetExt();
    m_relative = filepath.m_relative;
    m_hasExt = filepath.m_hasExt;
    m_dontFollowLinks = filepath.m_dontFollowLinks;
}

void wxFileName::Assign(const wxString& volume,
                        const wxString& path,
                        const wxString& name,
                        const wxString& ext,
                        bool hasExt,
                        wxPathFormat format)
{
                                        if ( IsUNCPath(path, format) )
    {
                        wxString pathNonUNC(path, 1, wxString::npos);
        SetPath(pathNonUNC, format);
    }
    else     {
        SetPath(path, format);
    }

    m_volume = volume;
    m_ext = ext;
    m_name = name;

    m_hasExt = hasExt;
}

void wxFileName::SetPath( const wxString& pathOrig, wxPathFormat format )
{
    m_dirs.Clear();

    if ( pathOrig.empty() )
    {
                m_relative = true;

        return;
    }

    format = GetFormat( format );

        wxString volume,
             path;
    SplitVolume(pathOrig, &volume, &path, format);
    if ( !volume.empty() )
    {
        m_relative = false;

        SetVolume(volume);
    }

    
    if ( path.empty() )
    {
                return;
    }

    wxChar leadingChar = path[0u];

    switch (format)
    {
        case wxPATH_MAC:
            m_relative = leadingChar == wxT(':');

                                                                                                                                    if (m_relative)
                path.erase( 0, 1 );
            break;

        case wxPATH_VMS:
                        m_relative = false;
            break;

        default:
            wxFAIL_MSG( wxT("Unknown path format") );
            wxFALLTHROUGH;

        case wxPATH_UNIX:
            m_relative = leadingChar != wxT('/');
            break;

        case wxPATH_DOS:
            m_relative = !IsPathSeparator(leadingChar, format);
            break;

    }

            
    wxStringTokenizer tn( path, GetPathSeparators(format) );

    while ( tn.HasMoreTokens() )
    {
        wxString token = tn.GetNextToken();

                        if (token.empty())
        {
            if (format == wxPATH_MAC)
                m_dirs.Add( wxT("..") );
                    }
        else
        {
           m_dirs.Add( token );
        }
    }
}

void wxFileName::Assign(const wxString& fullpath,
                        wxPathFormat format)
{
    wxString volume, path, name, ext;
    bool hasExt;
    SplitPath(fullpath, &volume, &path, &name, &ext, &hasExt, format);

    Assign(volume, path, name, ext, hasExt, format);
}

void wxFileName::Assign(const wxString& fullpathOrig,
                        const wxString& fullname,
                        wxPathFormat format)
{
            wxString fullpath = fullpathOrig;
    if ( !fullpath.empty() && !wxEndsWithPathSeparator(fullpath) )
    {
        fullpath += GetPathSeparator(format);
    }

    wxString volume, path, name, ext;
    bool hasExt;

            wxString volDummy, pathDummy, nameDummy, extDummy;

    SplitPath(fullname, &volDummy, &pathDummy, &name, &ext, &hasExt, format);

    wxASSERT_MSG( volDummy.empty() && pathDummy.empty(),
                  wxT("the file name shouldn't contain the path") );

    SplitPath(fullpath, &volume, &path, &nameDummy, &extDummy, format);

#ifndef __VMS
      wxASSERT_MSG( nameDummy.empty() && extDummy.empty(),
                  wxT("the path shouldn't contain file name nor extension") );
#endif
    Assign(volume, path, name, ext, hasExt, format);
}

void wxFileName::Assign(const wxString& pathOrig,
                        const wxString& name,
                        const wxString& ext,
                        wxPathFormat format)
{
    wxString volume,
             path;
    SplitVolume(pathOrig, &volume, &path, format);

    Assign(volume, path, name, ext, format);
}

void wxFileName::AssignDir(const wxString& dir, wxPathFormat format)
{
    Assign(dir, wxEmptyString, format);
}

void wxFileName::Clear()
{
    m_dirs.clear();
    m_volume.clear();
    m_name.clear();
    m_ext.clear();

        m_relative = true;

        m_hasExt = false;

        m_dontFollowLinks = false;
}


wxFileName wxFileName::FileName(const wxString& file, wxPathFormat format)
{
    return wxFileName(file, format);
}


wxFileName wxFileName::DirName(const wxString& dir, wxPathFormat format)
{
    wxFileName fn;
    fn.AssignDir(dir, format);
    return fn;
}


namespace
{

#if defined(__WINDOWS__)

void RemoveTrailingSeparatorsFromPath(wxString& strPath)
{
                    while ( wxEndsWithPathSeparator( strPath ) )
    {
        size_t len = strPath.length();
        if ( len == 1 || (len == 3 && strPath[len - 2] == wxT(':')) ||
                (len == wxMSWUniqueVolumePrefixLength &&
                 wxFileName::IsMSWUniqueVolumeNamePath(strPath)))
        {
            break;
        }

        strPath.Truncate(len - 1);
    }
}

#endif 
bool
wxFileSystemObjectExists(const wxString& path, int flags)
{

            const bool acceptFile = (flags & wxFILE_EXISTS_REGULAR) != 0;
    const bool acceptDir  = (flags & wxFILE_EXISTS_DIR)  != 0;

    wxString strPath(path);

#if defined(__WINDOWS__)
    if ( acceptDir )
    {
                        RemoveTrailingSeparatorsFromPath(strPath);
    }

            DWORD ret = ::GetFileAttributes(strPath.t_str());

    if ( ret == INVALID_FILE_ATTRIBUTES )
        return false;

    if ( ret & FILE_ATTRIBUTE_DIRECTORY )
        return acceptDir;

            return acceptFile;
#else     wxStructStat st;
    if ( !StatAny(st, strPath, flags) )
        return false;

    if ( S_ISREG(st.st_mode) )
        return acceptFile;
    if ( S_ISDIR(st.st_mode) )
        return acceptDir;
    if ( S_ISLNK(st.st_mode) )
    {
                                return (flags & wxFILE_EXISTS_SYMLINK) == wxFILE_EXISTS_SYMLINK;
    }
    if ( S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode) )
        return (flags & wxFILE_EXISTS_DEVICE) != 0;
    if ( S_ISFIFO(st.st_mode) )
        return (flags & wxFILE_EXISTS_FIFO) != 0;
    if ( S_ISSOCK(st.st_mode) )
        return (flags & wxFILE_EXISTS_SOCKET) != 0;

    return flags & wxFILE_EXISTS_ANY;
#endif }

} 
bool wxFileName::FileExists() const
{
    int flags = wxFILE_EXISTS_REGULAR;
    if ( !ShouldFollowLink() )
        flags |= wxFILE_EXISTS_NO_FOLLOW;

    return wxFileSystemObjectExists(GetFullPath(), flags);
}


bool wxFileName::FileExists( const wxString &filePath )
{
    return wxFileSystemObjectExists(filePath, wxFILE_EXISTS_REGULAR);
}

bool wxFileName::DirExists() const
{
    int flags = wxFILE_EXISTS_DIR;
    if ( !ShouldFollowLink() )
        flags |= wxFILE_EXISTS_NO_FOLLOW;

    return Exists(GetPath(), flags);
}


bool wxFileName::DirExists( const wxString &dirPath )
{
    return wxFileSystemObjectExists(dirPath, wxFILE_EXISTS_DIR);
}

bool wxFileName::Exists(int flags) const
{
                    if ( !ShouldFollowLink() )
        flags |= wxFILE_EXISTS_NO_FOLLOW;

    return wxFileSystemObjectExists(GetFullPath(), flags);
}


bool wxFileName::Exists(const wxString& path, int flags)
{
    return wxFileSystemObjectExists(path, flags);
}


void wxFileName::AssignCwd(const wxString& volume)
{
    AssignDir(wxFileName::GetCwd(volume));
}


wxString wxFileName::GetCwd(const wxString& volume)
{
                    wxString cwdOld;
    if ( !volume.empty() )
    {
        cwdOld = wxGetCwd();
        SetCwd(volume + GetVolumeSeparator());
    }

    wxString cwd = ::wxGetCwd();

    if ( !volume.empty() )
    {
        SetCwd(cwdOld);
    }

    return cwd;
}

bool wxFileName::SetCwd() const
{
    return wxFileName::SetCwd( GetPath() );
}

bool wxFileName::SetCwd( const wxString &cwd )
{
    return ::wxSetWorkingDirectory( cwd );
}

void wxFileName::AssignHomeDir()
{
    AssignDir(wxFileName::GetHomeDir());
}

wxString wxFileName::GetHomeDir()
{
    return ::wxGetHomeDir();
}



#if wxUSE_FILE || wxUSE_FFILE


#if !defined wx_fdopen && defined HAVE_FDOPEN
    #define wx_fdopen fdopen
#endif

#ifdef __WINDOWS__
    #define wxOPEN_EXCL 0
#else
    #define wxOPEN_EXCL O_EXCL
#endif


#ifdef wxOpenOSFHandle
#define WX_HAVE_DELETE_ON_CLOSE
static int wxOpenWithDeleteOnClose(const wxString& filename)
{
    DWORD access = GENERIC_READ | GENERIC_WRITE;

    DWORD disposition = OPEN_ALWAYS;

    DWORD attributes = FILE_ATTRIBUTE_TEMPORARY |
                       FILE_FLAG_DELETE_ON_CLOSE;

    HANDLE h = ::CreateFile(filename.t_str(), access, 0, NULL,
                            disposition, attributes, NULL);

    return wxOpenOSFHandle(h, wxO_BINARY);
}
#endif 

static int wxTempOpen(const wxString& path, bool *deleteOnClose)
{
#ifdef WX_HAVE_DELETE_ON_CLOSE
    if (*deleteOnClose)
        return wxOpenWithDeleteOnClose(path);
#endif

    *deleteOnClose = false;

    return wxOpen(path, wxO_BINARY | O_RDWR | O_CREAT | wxOPEN_EXCL, 0600);
}


#if wxUSE_FFILE
static bool wxTempOpen(wxFFile *file, const wxString& path, bool *deleteOnClose)
{
#ifndef wx_fdopen
    *deleteOnClose = false;
    return file->Open(path, wxT("w+b"));
#else     int fd = wxTempOpen(path, deleteOnClose);
    if (fd == -1)
        return false;
    file->Attach(wx_fdopen(fd, "w+b"), path);
    return file->IsOpened();
#endif }
#endif 

#if !wxUSE_FILE
    #define WXFILEARGS(x, y) y
#elif !wxUSE_FFILE
    #define WXFILEARGS(x, y) x
#else
    #define WXFILEARGS(x, y) x, y
#endif


static wxString wxCreateTempImpl(
        const wxString& prefix,
        WXFILEARGS(wxFile *fileTemp, wxFFile *ffileTemp),
        bool *deleteOnClose = NULL)
{
#if wxUSE_FILE && wxUSE_FFILE
    wxASSERT(fileTemp == NULL || ffileTemp == NULL);
#endif
    wxString path, dir, name;
    bool wantDeleteOnClose = false;

    if (deleteOnClose)
    {
                wantDeleteOnClose = *deleteOnClose;
        *deleteOnClose = false;
    }
    else
    {
                deleteOnClose = &wantDeleteOnClose;
    }

        wxFileName::SplitPath(prefix, &dir, &name, NULL );

    if (dir.empty())
    {
        dir = wxFileName::GetTempDir();
    }

#if defined(__WINDOWS__)
    if (!::GetTempFileName(dir.t_str(), name.t_str(), 0,
                           wxStringBuffer(path, MAX_PATH + 1)))
    {
        wxLogLastError(wxT("GetTempFileName"));

        path.clear();
    }

#else     path = dir;

    if ( !wxEndsWithPathSeparator(dir) &&
            (name.empty() || !wxIsPathSeparator(name[0u])) )
    {
        path += wxFILE_SEP_PATH;
    }

    path += name;

#if defined(HAVE_MKSTEMP)
        path += wxT("XXXXXX");

        wxCharBuffer buf(path.fn_str());

        int fdTemp = mkstemp( (char*)(const char*) buf );
    if ( fdTemp == -1 )
    {
                        path.clear();
    }
    else     {
        path = wxConvFile.cMB2WX( (const char*) buf );

    #if wxUSE_FILE
                if ( fileTemp )
        {
            fileTemp->Attach(fdTemp);
        }
        else
    #endif

    #if wxUSE_FFILE
        if ( ffileTemp )
        {
        #ifdef wx_fdopen
            ffileTemp->Attach(wx_fdopen(fdTemp, "r+b"), path);
        #else
            ffileTemp->Open(path, wxT("r+b"));
            close(fdTemp);
        #endif
        }
        else
    #endif

        {
            close(fdTemp);
        }
    }
#else 
#ifdef HAVE_MKTEMP
        path += wxT("XXXXXX");

    wxCharBuffer buf = wxConvFile.cWX2MB( path );
    if ( !mktemp( (char*)(const char*) buf ) )
    {
        path.clear();
    }
    else
    {
        path = wxConvFile.cMB2WX( (const char*) buf );
    }
#else         path << (unsigned int)getpid();

    wxString pathTry;

    static const size_t numTries = 1000;
    for ( size_t n = 0; n < numTries; n++ )
    {
                pathTry = path + wxString::Format(wxT("%.03x"), (unsigned int) n);
        if ( !wxFileName::FileExists(pathTry) )
        {
            break;
        }

        pathTry.clear();
    }

    path = pathTry;
#endif 
#endif 
#endif 
    if ( path.empty() )
    {
        wxLogSysError(_("Failed to create a temporary file name"));
    }
    else
    {
        bool ok = true;

                    #if wxUSE_FILE
        if ( fileTemp && !fileTemp->IsOpened() )
        {
            *deleteOnClose = wantDeleteOnClose;
            int fd = wxTempOpen(path, deleteOnClose);
            if (fd != -1)
                fileTemp->Attach(fd);
            else
                ok = false;
        }
    #endif

    #if wxUSE_FFILE
        if ( ffileTemp && !ffileTemp->IsOpened() )
        {
            *deleteOnClose = wantDeleteOnClose;
            ok = wxTempOpen(ffileTemp, path, deleteOnClose);
        }
    #endif

        if ( !ok )
        {
                                                
            wxLogError(_("Failed to open temporary file."));

            path.clear();
        }
    }

    return path;
}


static bool wxCreateTempImpl(
        const wxString& prefix,
        WXFILEARGS(wxFile *fileTemp, wxFFile *ffileTemp),
        wxString *name)
{
    bool deleteOnClose = true;

    *name = wxCreateTempImpl(prefix,
                             WXFILEARGS(fileTemp, ffileTemp),
                             &deleteOnClose);

    bool ok = !name->empty();

    if (deleteOnClose)
        name->clear();
#ifdef __UNIX__
    else if (ok && wxRemoveFile(*name))
        name->clear();
#endif

    return ok;
}


static void wxAssignTempImpl(
        wxFileName *fn,
        const wxString& prefix,
        WXFILEARGS(wxFile *fileTemp, wxFFile *ffileTemp))
{
    wxString tempname;
    tempname = wxCreateTempImpl(prefix, WXFILEARGS(fileTemp, ffileTemp));

    if ( tempname.empty() )
    {
                fn->Clear();
    }
    else     {
        fn->Assign(tempname);
    }
}


void wxFileName::AssignTempFileName(const wxString& prefix)
{
    wxAssignTempImpl(this, prefix, WXFILEARGS(NULL, NULL));
}


wxString wxFileName::CreateTempFileName(const wxString& prefix)
{
    return wxCreateTempImpl(prefix, WXFILEARGS(NULL, NULL));
}

#endif 

#if wxUSE_FILE

wxString wxCreateTempFileName(const wxString& prefix,
                              wxFile *fileTemp,
                              bool *deleteOnClose)
{
    return wxCreateTempImpl(prefix, WXFILEARGS(fileTemp, NULL), deleteOnClose);
}

bool wxCreateTempFile(const wxString& prefix,
                      wxFile *fileTemp,
                      wxString *name)
{
    return wxCreateTempImpl(prefix, WXFILEARGS(fileTemp, NULL), name);
}

void wxFileName::AssignTempFileName(const wxString& prefix, wxFile *fileTemp)
{
    wxAssignTempImpl(this, prefix, WXFILEARGS(fileTemp, NULL));
}


wxString
wxFileName::CreateTempFileName(const wxString& prefix, wxFile *fileTemp)
{
    return wxCreateTempFileName(prefix, fileTemp);
}

#endif 

#if wxUSE_FFILE

wxString wxCreateTempFileName(const wxString& prefix,
                              wxFFile *fileTemp,
                              bool *deleteOnClose)
{
    return wxCreateTempImpl(prefix, WXFILEARGS(NULL, fileTemp), deleteOnClose);
}

bool wxCreateTempFile(const wxString& prefix,
                      wxFFile *fileTemp,
                      wxString *name)
{
    return wxCreateTempImpl(prefix, WXFILEARGS(NULL, fileTemp), name);

}

void wxFileName::AssignTempFileName(const wxString& prefix, wxFFile *fileTemp)
{
    wxAssignTempImpl(this, prefix, WXFILEARGS(NULL, fileTemp));
}


wxString
wxFileName::CreateTempFileName(const wxString& prefix, wxFFile *fileTemp)
{
    return wxCreateTempFileName(prefix, fileTemp);
}

#endif 


namespace
{

wxString CheckIfDirExists(const wxString& dir)
{
    return wxFileName::DirExists(dir) ? dir : wxString();
}

} 
wxString wxFileName::GetTempDir()
{
                wxString dir = CheckIfDirExists(wxGetenv("TMPDIR"));
    if ( dir.empty() )
    {
        dir = CheckIfDirExists(wxGetenv("TMP"));
        if ( dir.empty() )
            dir = CheckIfDirExists(wxGetenv("TEMP"));
    }

        if ( dir.empty() )
    {
#if defined(__WINDOWS__)
        if ( !::GetTempPath(MAX_PATH, wxStringBuffer(dir, MAX_PATH + 1)) )
        {
            wxLogLastError(wxT("GetTempPath"));
        }
#endif     }

    if ( !dir.empty() )
    {
                        const size_t lastNonSep = dir.find_last_not_of(GetPathSeparators());
        if ( lastNonSep == wxString::npos )
        {
                        dir = GetPathSeparator();
        }
        else
        {
            dir.erase(lastNonSep + 1);
        }
    }

        else
    {
#ifdef __UNIX_LIKE__
        dir = CheckIfDirExists("/tmp");
        if ( dir.empty() )
#endif             dir = ".";
    }

    return dir;
}

bool wxFileName::Mkdir( int perm, int flags ) const
{
    return wxFileName::Mkdir(GetPath(), perm, flags);
}

bool wxFileName::Mkdir( const wxString& dir, int perm, int flags )
{
    if ( flags & wxPATH_MKDIR_FULL )
    {
                wxFileName filename;
        filename.AssignDir(dir);

        wxString currPath;
        if ( filename.HasVolume())
        {
            currPath << wxGetVolumeString(filename.GetVolume(), wxPATH_NATIVE);
        }

        wxArrayString dirs = filename.GetDirs();
        size_t count = dirs.GetCount();
        for ( size_t i = 0; i < count; i++ )
        {
            if ( i > 0 || filename.IsAbsolute() )
                currPath += wxFILE_SEP_PATH;
            currPath += dirs[i];

            if (!DirExists(currPath))
            {
                if (!wxMkdir(currPath, perm))
                {
                                        return false;
                }
            }
        }

        return true;

    }

    return ::wxMkdir( dir, perm );
}

bool wxFileName::Rmdir(int flags) const
{
    return wxFileName::Rmdir( GetPath(), flags );
}

bool wxFileName::Rmdir(const wxString& dir, int flags)
{
#ifdef __WINDOWS__
    if ( flags & wxPATH_RMDIR_RECURSIVE )
    {
                        wxString path(dir);
        if ( path.Last() == wxFILE_SEP_PATH )
            path.RemoveLast();
        path += wxT('\0');

        SHFILEOPSTRUCT fileop;
        wxZeroMemory(fileop);
        fileop.wFunc = FO_DELETE;
        fileop.pFrom = path.t_str();
        fileop.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;
        fileop.fFlags |= FOF_NOERRORUI;

        int ret = SHFileOperation(&fileop);
        if ( ret != 0 )
        {
                                    wxLogApiError(wxT("SHFileOperation"), ret);
            return false;
        }

        return true;
    }
    else if ( flags & wxPATH_RMDIR_FULL )
#else     if ( flags != 0 )   #endif     {
#ifndef __WINDOWS__
        if ( flags & wxPATH_RMDIR_RECURSIVE )
        {
                                                            if ( wxFileName::Exists(dir, wxFILE_EXISTS_SYMLINK) )
            {
                return wxRemoveFile(dir);
            }
        }
#endif 
        wxString path(dir);
        if ( path.Last() != wxFILE_SEP_PATH )
            path += wxFILE_SEP_PATH;

        wxDir d(path);

        if ( !d.IsOpened() )
            return false;

        wxString filename;

                                bool cont = d.GetFirst(&filename, wxString(),
                               wxDIR_DIRS | wxDIR_HIDDEN | wxDIR_NO_FOLLOW);
        while ( cont )
        {
            wxFileName::Rmdir(path + filename, flags);
            cont = d.GetNext(&filename);
        }

#ifndef __WINDOWS__
        if ( flags & wxPATH_RMDIR_RECURSIVE )
        {
                                                cont = d.GetFirst(&filename, wxString(),
                              wxDIR_FILES | wxDIR_HIDDEN | wxDIR_NO_FOLLOW);
            while ( cont )
            {
                ::wxRemoveFile(path + filename);
                cont = d.GetNext(&filename);
            }
        }
#endif     }

    return ::wxRmdir(dir);
}


bool wxFileName::Normalize(int flags,
                           const wxString& cwd,
                           wxPathFormat format)
{
        if ( flags & wxPATH_NORM_ENV_VARS )
    {
        wxString pathOrig = GetFullPath(format);
        wxString path = wxExpandEnvVars(pathOrig);
        if ( path != pathOrig )
        {
            Assign(path);
        }
    }

        wxArrayString dirs = GetDirs();

        wxFileName curDir;

    format = GetFormat(format);

        if ( (flags & wxPATH_NORM_ABSOLUTE) && !IsAbsolute(format) )
    {
        if ( cwd.empty() )
        {
            curDir.AssignCwd(GetVolume());
        }
        else         {
            curDir.AssignDir(cwd);
        }
    }

        if ( (format == wxPATH_UNIX) && (flags & wxPATH_NORM_TILDE) && m_relative )
    {
        if ( !dirs.IsEmpty() )
        {
            wxString dir = dirs[0u];
            if ( !dir.empty() && dir[0u] == wxT('~') )
            {
                                curDir.AssignDir(wxGetUserHome(dir.c_str() + 1));
                dirs.RemoveAt(0u);
            }
        }
    }

        if ( curDir.IsOk() )
    {
                                if ( !HasVolume() && curDir.HasVolume() )
        {
            SetVolume(curDir.GetVolume());

            if ( !m_relative )
        {
                                curDir.Clear();
            }
        }

                wxArrayString dirsNew = curDir.GetDirs();
        WX_PREPEND_ARRAY(dirs, dirsNew);

                        if ( !curDir.m_relative )
        {
                                    m_relative = false;
        }
                    }

        m_dirs.Empty();
    size_t count = dirs.GetCount();
    for ( size_t n = 0; n < count; n++ )
    {
        wxString dir = dirs[n];

        if ( flags & wxPATH_NORM_DOTS )
        {
            if ( dir == wxT(".") )
            {
                                continue;
            }

            if ( dir == wxT("..") )
            {
                if ( m_dirs.empty() )
                {
                                                                                                                                                                if ( !m_relative )
                        continue;

                }
                else                 {
                    m_dirs.pop_back();
                    continue;
                }
            }
        }

        m_dirs.Add(dir);
    }

#if defined(__WIN32__) && wxUSE_OLE
    if ( (flags & wxPATH_NORM_SHORTCUT) )
    {
        wxString filename;
        if (GetShortcutTarget(GetFullPath(format), filename))
        {
            m_relative = false;
            Assign(filename);
        }
    }
#endif

#if defined(__WIN32__)
    if ( (flags & wxPATH_NORM_LONG) && (format == wxPATH_DOS) )
    {
        Assign(GetLongPath());
    }
#endif 
            if ( (flags & wxPATH_NORM_CASE) && !IsCaseSensitive(format) )
    {
        m_volume.MakeLower();
        m_name.MakeLower();
        m_ext.MakeLower();

                count = m_dirs.GetCount();
        for ( size_t i = 0; i < count; i++ )
        {
            m_dirs[i].MakeLower();
        }
    }

    return true;
}

bool wxFileName::ReplaceEnvVariable(const wxString& envname,
                                    const wxString& replacementFmtString,
                                    wxPathFormat format)
{
        wxString val;
    if (envname.empty() ||
        !wxGetEnv(envname, &val))
        return false;
    if (val.empty())
        return false;

    wxString stringForm = GetPath(wxPATH_GET_VOLUME, format);
        
    wxString replacement = wxString::Format(replacementFmtString, envname);
    stringForm.Replace(val, replacement);

        Assign(stringForm, GetFullName(), format);

    return true;
}

bool wxFileName::ReplaceHomeDir(wxPathFormat format)
{
    wxString homedir = wxGetHomeDir();
    if (homedir.empty())
        return false;

    wxString stringForm = GetPath(wxPATH_GET_VOLUME, format);
        
    stringForm.Replace(homedir, "~");

        Assign(stringForm, GetFullName(), format);

    return true;
}


#if defined(__WIN32__) && wxUSE_OLE

bool wxFileName::GetShortcutTarget(const wxString& shortcutPath,
                                   wxString& targetFilename,
                                   wxString* arguments) const
{
    wxString path, file, ext;
    wxFileName::SplitPath(shortcutPath, & path, & file, & ext);

    HRESULT hres;
    wxCOMPtr<IShellLink> psl;
    bool success = false;

        if (ext.CmpNoCase(wxT("lnk"))!=0)
        return false;

        wxOleInitializer oleInit;

        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (LPVOID*) &psl);

    if (SUCCEEDED(hres))
    {
        wxCOMPtr<IPersistFile> ppf;
        hres = psl->QueryInterface( IID_IPersistFile, (LPVOID *) &ppf);
        if (SUCCEEDED(hres))
        {
            WCHAR wsz[MAX_PATH];

            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, shortcutPath.mb_str(), -1, wsz,
                                MAX_PATH);

            hres = ppf->Load(wsz, 0);
            if (SUCCEEDED(hres))
            {
                wxChar buf[2048];
                #if defined(__MINGW32__) && !wxCHECK_W32API_VERSION(2, 2)
                psl->GetPath((CHAR*) buf, 2048, NULL, SLGP_UNCPRIORITY);
#else
                psl->GetPath(buf, 2048, NULL, SLGP_UNCPRIORITY);
#endif
                targetFilename = wxString(buf);
                success = (shortcutPath != targetFilename);

                psl->GetArguments(buf, 2048);
                wxString args(buf);
                if (!args.empty() && arguments)
                {
                    *arguments = args;
                }
            }
        }
    }
    return success;
}

#endif 


bool wxFileName::IsAbsolute(wxPathFormat format) const
{
        if ( format == wxPATH_UNIX )
    {
        if ( !m_dirs.IsEmpty() )
        {
            wxString dir = m_dirs[0u];

            if (!dir.empty() && dir[0u] == wxT('~'))
                return true;
        }
    }

            if ( m_relative )
        return false;

    if ( !GetVolumeSeparator(format).empty() )
    {
                        if ( GetVolume().empty() )
            return false;
    }

    return true;
}

bool wxFileName::MakeRelativeTo(const wxString& pathBase, wxPathFormat format)
{
    wxFileName fnBase = wxFileName::DirName(pathBase, format);

        wxString cwd = wxGetCwd();

            const int normFlags = wxPATH_NORM_ALL &
                            ~(wxPATH_NORM_CASE | wxPATH_NORM_SHORTCUT);
    Normalize(normFlags, cwd, format);
    fnBase.Normalize(normFlags, cwd, format);

    bool withCase = IsCaseSensitive(format);

        if ( !GetVolume().IsSameAs(fnBase.GetVolume(), withCase) )
    {
                return false;
    }

        m_volume.clear();

        while ( !m_dirs.IsEmpty() && !fnBase.m_dirs.IsEmpty() &&
                m_dirs[0u].IsSameAs(fnBase.m_dirs[0u], withCase) )
    {
        m_dirs.RemoveAt(0);
        fnBase.m_dirs.RemoveAt(0);
    }

        size_t count = fnBase.m_dirs.GetCount();
    for ( size_t i = 0; i < count; i++ )
    {
        m_dirs.Insert(wxT(".."), 0u);
    }

    switch ( GetFormat(format) )
    {
        case wxPATH_NATIVE:
        case wxPATH_MAX:
            wxFAIL_MSG( wxS("unreachable") );
            wxFALLTHROUGH;

        case wxPATH_UNIX:
        case wxPATH_DOS:
                                                if ( m_dirs.IsEmpty() && IsDir() )
            {
                m_dirs.Add(wxT('.'));
            }
            break;

        case wxPATH_MAC:
        case wxPATH_VMS:
            break;
    }

    m_relative = true;

        return true;
}


bool wxFileName::SameAs(const wxFileName& filepath, wxPathFormat format) const
{
    wxFileName fn1 = *this,
               fn2 = filepath;

        wxString cwd = wxGetCwd();
    fn1.Normalize(wxPATH_NORM_ALL | wxPATH_NORM_CASE, cwd, format);
    fn2.Normalize(wxPATH_NORM_ALL | wxPATH_NORM_CASE, cwd, format);

    if ( fn1.GetFullPath() == fn2.GetFullPath() )
        return true;

#ifdef wxHAVE_LSTAT
    wxStructStat st1, st2;
    if ( StatAny(st1, fn1) && StatAny(st2, fn2) )
    {
        if ( st1.st_ino == st2.st_ino && st1.st_dev == st2.st_dev )
            return true;
    }
    #endif 
    return false;
}


bool wxFileName::IsCaseSensitive( wxPathFormat format )
{
        return GetFormat(format) == wxPATH_UNIX;
}


wxString wxFileName::GetForbiddenChars(wxPathFormat format)
{
        wxString strForbiddenChars = wxT("*?");

            wxCOMPILE_TIME_ASSERT(wxPATH_MAX == 5, wxPathFormatChanged);
    switch ( GetFormat(format) )
    {
        default :
            wxFAIL_MSG( wxT("Unknown path format") );
            wxFALLTHROUGH;

        case wxPATH_UNIX:
            break;

        case wxPATH_MAC:
                                    strForbiddenChars.clear();
            break;

        case wxPATH_DOS:
            strForbiddenChars += wxT("\\/:\"<>|");
            break;

        case wxPATH_VMS:
            break;
    }

    return strForbiddenChars;
}


wxString wxFileName::GetVolumeSeparator(wxPathFormat format)
{
    wxString sepVol;

    if ( (GetFormat(format) == wxPATH_DOS) ||
         (GetFormat(format) == wxPATH_VMS) )
    {
        sepVol = wxFILE_SEP_DSK;
    }
    
    return sepVol;
}


wxString wxFileName::GetPathSeparators(wxPathFormat format)
{
    wxString seps;
    switch ( GetFormat(format) )
    {
        case wxPATH_DOS:
                                    seps << wxFILE_SEP_PATH_DOS << wxFILE_SEP_PATH_UNIX;
            break;

        default:
            wxFAIL_MSG( wxT("Unknown wxPATH_XXX style") );
            wxFALLTHROUGH;

        case wxPATH_UNIX:
            seps = wxFILE_SEP_PATH_UNIX;
            break;

        case wxPATH_MAC:
            seps = wxFILE_SEP_PATH_MAC;
            break;

        case wxPATH_VMS:
            seps = wxFILE_SEP_PATH_VMS;
            break;
    }

    return seps;
}


wxString wxFileName::GetPathTerminators(wxPathFormat format)
{
    format = GetFormat(format);

            return format == wxPATH_VMS ? wxString(wxT(']')) : GetPathSeparators(format);
}


bool wxFileName::IsPathSeparator(wxChar ch, wxPathFormat format)
{
            return ch != wxT('\0') && GetPathSeparators(format).Find(ch) != wxNOT_FOUND;
}


bool
wxFileName::IsMSWUniqueVolumeNamePath(const wxString& path, wxPathFormat format)
{
            return format == wxPATH_DOS &&
            path.length() >= wxMSWUniqueVolumePrefixLength &&
             path.StartsWith(wxS("\\\\?\\Volume{")) &&
              path[wxMSWUniqueVolumePrefixLength - 1] == wxFILE_SEP_PATH_DOS;
}


 bool wxFileName::IsValidDirComponent(const wxString& dir)
{
    if ( dir.empty() )
    {
        wxFAIL_MSG( wxT("empty directory passed to wxFileName::InsertDir()") );

        return false;
    }

    const size_t len = dir.length();
    for ( size_t n = 0; n < len; n++ )
    {
        if ( dir[n] == GetVolumeSeparator() || IsPathSeparator(dir[n]) )
        {
            wxFAIL_MSG( wxT("invalid directory component in wxFileName") );

            return false;
        }
    }

    return true;
}

bool wxFileName::AppendDir( const wxString& dir )
{
    if (!IsValidDirComponent(dir))
        return false;
    m_dirs.Add(dir);
    return true;
}

void wxFileName::PrependDir( const wxString& dir )
{
    InsertDir(0, dir);
}

bool wxFileName::InsertDir(size_t before, const wxString& dir)
{
    if (!IsValidDirComponent(dir))
        return false;
    m_dirs.Insert(dir, before);
    return true;
}

void wxFileName::RemoveDir(size_t pos)
{
    m_dirs.RemoveAt(pos);
}


void wxFileName::SetFullName(const wxString& fullname)
{
    SplitPath(fullname, NULL , NULL ,
                        &m_name, &m_ext, &m_hasExt);
}

wxString wxFileName::GetFullName() const
{
    wxString fullname = m_name;
    if ( m_hasExt )
    {
        fullname << wxFILE_SEP_EXT << m_ext;
    }

    return fullname;
}

wxString wxFileName::GetPath( int flags, wxPathFormat format ) const
{
    format = GetFormat( format );

    wxString fullpath;

        if ( flags & wxPATH_GET_VOLUME )
    {
        fullpath += wxGetVolumeString(GetVolume(), format);
    }

        switch ( format )
    {
        case wxPATH_MAC:
            if ( m_relative )
                fullpath += wxFILE_SEP_PATH_MAC;
            break;

        case wxPATH_DOS:
            if ( !m_relative )
                fullpath += wxFILE_SEP_PATH_DOS;
            break;

        default:
            wxFAIL_MSG( wxT("Unknown path format") );
            wxFALLTHROUGH;

        case wxPATH_UNIX:
            if ( !m_relative )
            {
                fullpath += wxFILE_SEP_PATH_UNIX;
            }
            break;

        case wxPATH_VMS:
                                                            flags &= ~wxPATH_GET_SEPARATOR;
    }

    if ( m_dirs.empty() )
    {
                return fullpath;
    }

        if ( format == wxPATH_VMS )
    {
        fullpath += wxT('[');
    }

    const size_t dirCount = m_dirs.GetCount();
    for ( size_t i = 0; i < dirCount; i++ )
    {
        switch (format)
        {
            case wxPATH_MAC:
                if ( m_dirs[i] == wxT(".") )
                {
                                                            continue;
                }

                                if ( !m_dirs[i].IsSameAs(wxT("..")) )
                     fullpath += m_dirs[i];
                break;

            default:
                wxFAIL_MSG( wxT("Unexpected path format") );
                wxFALLTHROUGH;

            case wxPATH_DOS:
            case wxPATH_UNIX:
                fullpath += m_dirs[i];
                break;

            case wxPATH_VMS:
                
                                if ( !m_dirs[i].IsSameAs(wxT("..")) )
                    fullpath += m_dirs[i];
                break;
        }

        if ( (flags & wxPATH_GET_SEPARATOR) || (i != dirCount - 1) )
            fullpath += GetPathSeparator(format);
    }

    if ( format == wxPATH_VMS )
    {
        fullpath += wxT(']');
    }

    return fullpath;
}

wxString wxFileName::GetFullPath( wxPathFormat format ) const
{
        wxString fullpath = GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR,
                                format);

        fullpath += GetFullName();

    return fullpath;
}

wxString wxFileName::GetShortPath() const
{
    wxString path(GetFullPath());

#if defined(__WINDOWS__) && defined(__WIN32__)
    DWORD sz = ::GetShortPathName(path.t_str(), NULL, 0);
    if ( sz != 0 )
    {
        wxString pathOut;
        if ( ::GetShortPathName
               (
                path.t_str(),
                wxStringBuffer(pathOut, sz),
                sz
               ) != 0 )
        {
            return pathOut;
        }
    }
#endif 
    return path;
}

wxString wxFileName::GetLongPath() const
{
    wxString pathOut,
             path = GetFullPath();

#if defined(__WIN32__)

    DWORD dwSize = ::GetLongPathName(path.t_str(), NULL, 0);
    if ( dwSize > 0 )
    {
        if ( ::GetLongPathName
                (
                path.t_str(),
                wxStringBuffer(pathOut, dwSize),
                dwSize
                ) != 0 )
        {
            return pathOut;
        }
    }

        
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;

    if ( HasVolume() )
        pathOut = GetVolume() +
                  GetVolumeSeparator(wxPATH_DOS) +
                  GetPathSeparator(wxPATH_DOS);
    else
        pathOut.clear();

    wxArrayString dirs = GetDirs();
    dirs.Add(GetFullName());

    wxString tmpPath;

    size_t count = dirs.GetCount();
    for ( size_t i = 0; i < count; i++ )
    {
        const wxString& dir = dirs[i];

                                tmpPath = pathOut + dir;

                                                if ( tmpPath.empty() || dir == '.' || dir == ".." ||
                tmpPath.Last() == GetVolumeSeparator(wxPATH_DOS) )
        {
            tmpPath += wxFILE_SEP_PATH;
            pathOut = tmpPath;
            continue;
        }

        hFind = ::FindFirstFile(tmpPath.t_str(), &findFileData);
        if (hFind == INVALID_HANDLE_VALUE)
        {
                                    for ( i += 1; i < count; i++ )
                tmpPath += wxFILE_SEP_PATH + dirs[i];

            return tmpPath;
        }

        pathOut += findFileData.cFileName;
        if ( (i < (count-1)) )
            pathOut += wxFILE_SEP_PATH;

        ::FindClose(hFind);
    }
#else     pathOut = path;
#endif 
    return pathOut;
}

wxPathFormat wxFileName::GetFormat( wxPathFormat format )
{
    if (format == wxPATH_NATIVE)
    {
#if defined(__WINDOWS__)
        format = wxPATH_DOS;
#elif defined(__VMS)
        format = wxPATH_VMS;
#else
        format = wxPATH_UNIX;
#endif
    }
    return format;
}

#ifdef wxHAS_FILESYSTEM_VOLUMES


wxString wxFileName::GetVolumeString(char drive, int flags)
{
    wxASSERT_MSG( !(flags & ~wxPATH_GET_SEPARATOR), "invalid flag specified" );

    wxString vol(drive);
    vol += wxFILE_SEP_DSK;
    if ( flags & wxPATH_GET_SEPARATOR )
        vol += wxFILE_SEP_PATH;

    return vol;
}

#endif 


void
wxFileName::SplitVolume(const wxString& fullpathWithVolume,
                        wxString *pstrVolume,
                        wxString *pstrPath,
                        wxPathFormat format)
{
    format = GetFormat(format);

    wxString fullpath = fullpathWithVolume;

    if ( IsMSWUniqueVolumeNamePath(fullpath, format) )
    {
                        
                        fullpath[wxMSWUniqueVolumePrefixLength - 1] = wxFILE_SEP_DSK;

                fullpath.insert(wxMSWUniqueVolumePrefixLength, 1, wxFILE_SEP_PATH_DOS);

                fullpath.erase(0, 4);
    }
    else if ( IsUNCPath(fullpath, format) )
    {
        
        fullpath.erase(0, 2);

        size_t posFirstSlash =
            fullpath.find_first_of(GetPathTerminators(format));
        if ( posFirstSlash != wxString::npos )
        {
            fullpath[posFirstSlash] = wxFILE_SEP_DSK;

                        fullpath.insert(posFirstSlash + 1, 1, wxFILE_SEP_PATH_DOS);
        }
    }

        if ( format == wxPATH_DOS || format == wxPATH_VMS )
    {
        wxString sepVol = GetVolumeSeparator(format);

                                size_t posFirstColon = fullpath.find_first_of(sepVol);
        if ( posFirstColon && posFirstColon != wxString::npos )
        {
            if ( pstrVolume )
            {
                *pstrVolume = fullpath.Left(posFirstColon);
            }

                        fullpath.erase(0, posFirstColon + sepVol.length());
        }
    }

    if ( pstrPath )
        *pstrPath = fullpath;
}


void wxFileName::SplitPath(const wxString& fullpathWithVolume,
                           wxString *pstrVolume,
                           wxString *pstrPath,
                           wxString *pstrName,
                           wxString *pstrExt,
                           bool *hasExt,
                           wxPathFormat format)
{
    format = GetFormat(format);

    wxString fullpath;
    SplitVolume(fullpathWithVolume, pstrVolume, &fullpath, format);

        size_t posLastDot = fullpath.find_last_of(wxFILE_SEP_EXT);
    size_t posLastSlash = fullpath.find_last_of(GetPathTerminators(format));

        if ( (posLastDot != wxString::npos) &&
         (posLastDot == 0 ||
            IsPathSeparator(fullpath[posLastDot - 1]) ||
            (format == wxPATH_VMS && fullpath[posLastDot - 1] == wxT(']'))) )
    {
                                posLastDot = wxString::npos;
    }

        if ( (posLastDot != wxString::npos) &&
         (posLastSlash != wxString::npos) &&
         (posLastDot < posLastSlash) )
    {
                posLastDot = wxString::npos;
    }

        if ( pstrPath )
    {
        if ( posLastSlash == wxString::npos )
        {
                        pstrPath->Empty();
        }
        else
        {
                                                size_t len = posLastSlash;

                                    if ( !len && format != wxPATH_MAC)
                len++;

            *pstrPath = fullpath.Left(len);

                        if ( format == wxPATH_VMS )
            {
                if ( (*pstrPath)[0u] == wxT('[') )
                    pstrPath->erase(0, 1);
            }
        }
    }

    if ( pstrName )
    {
                        size_t nStart = posLastSlash == wxString::npos ? 0 : posLastSlash + 1;
        size_t count;
        if ( posLastDot == wxString::npos )
        {
                        count = wxString::npos;
        }
        else if ( posLastSlash == wxString::npos )
        {
            count = posLastDot;
        }
        else         {
            count = posLastDot - posLastSlash - 1;
        }

        *pstrName = fullpath.Mid(nStart, count);
    }

                    if ( posLastDot == wxString::npos )
    {
                if ( pstrExt )
            pstrExt->clear();
        if ( hasExt )
            *hasExt = false;
    }
    else
    {
                if ( pstrExt )
            *pstrExt = fullpath.Mid(posLastDot + 1);
        if ( hasExt )
            *hasExt = true;
    }
}


void wxFileName::SplitPath(const wxString& fullpath,
                           wxString *path,
                           wxString *name,
                           wxString *ext,
                           wxPathFormat format)
{
    wxString volume;
    SplitPath(fullpath, &volume, path, name, ext, format);

    if ( path )
    {
        path->Prepend(wxGetVolumeString(volume, format));
    }
}


wxString wxFileName::StripExtension(const wxString& fullpath)
{
    wxFileName fn(fullpath);
    fn.SetExt("");
    return fn.GetFullPath();
}


bool wxFileName::SetPermissions(int permissions)
{
        if ( m_dontFollowLinks &&
            Exists(GetFullPath(), wxFILE_EXISTS_SYMLINK|wxFILE_EXISTS_NO_FOLLOW) )
    {
                                return false;
    }

#ifdef __WINDOWS__
    int accMode = 0;

    if ( permissions & (wxS_IRUSR|wxS_IRGRP|wxS_IROTH) )
        accMode = _S_IREAD;

    if ( permissions & (wxS_IWUSR|wxS_IWGRP|wxS_IWOTH) )
        accMode |= _S_IWRITE;

    permissions = accMode;
#endif 
    return wxChmod(GetFullPath(), permissions) == 0;
}


#if wxUSE_DATETIME

bool wxFileName::SetTimes(const wxDateTime *dtAccess,
                          const wxDateTime *dtMod,
                          const wxDateTime *dtCreate) const
{
#if defined(__WIN32__)
    FILETIME ftAccess, ftCreate, ftWrite;

    if ( dtCreate )
        ConvertWxToFileTime(&ftCreate, *dtCreate);
    if ( dtAccess )
        ConvertWxToFileTime(&ftAccess, *dtAccess);
    if ( dtMod )
        ConvertWxToFileTime(&ftWrite, *dtMod);

    wxString path;
    int flags;
    if ( IsDir() )
    {
        path = GetPath();
        flags = FILE_FLAG_BACKUP_SEMANTICS;
    }
    else     {
        path = GetFullPath();
        flags = 0;
    }

    wxFileHandle fh(path, wxFileHandle::WriteAttr, flags);
    if ( fh.IsOk() )
    {
        if ( ::SetFileTime(fh,
                           dtCreate ? &ftCreate : NULL,
                           dtAccess ? &ftAccess : NULL,
                           dtMod ? &ftWrite : NULL) )
        {
            return true;
        }
    }
#elif defined(__UNIX_LIKE__)
    wxUnusedVar(dtCreate);

    if ( !dtAccess && !dtMod )
    {
                return true;
    }

            utimbuf utm;
    utm.actime = dtAccess ? dtAccess->GetTicks() : dtMod->GetTicks();
    utm.modtime = dtMod ? dtMod->GetTicks() : dtAccess->GetTicks();
    if ( utime(GetFullPath().fn_str(), &utm) == 0 )
    {
        return true;
    }
#else     wxUnusedVar(dtAccess);
    wxUnusedVar(dtMod);
    wxUnusedVar(dtCreate);
#endif 
    wxLogSysError(_("Failed to modify file times for '%s'"),
                  GetFullPath().c_str());

    return false;
}

bool wxFileName::Touch() const
{
#if defined(__UNIX_LIKE__)
        if ( utime(GetFullPath().fn_str(), NULL) == 0 )
    {
        return true;
    }

    wxLogSysError(_("Failed to touch the file '%s'"), GetFullPath().c_str());

    return false;
#else     wxDateTime dtNow = wxDateTime::Now();

    return SetTimes(&dtNow, &dtNow, NULL );
#endif }

bool wxFileName::GetTimes(wxDateTime *dtAccess,
                          wxDateTime *dtMod,
                          wxDateTime *dtCreate) const
{
#if defined(__WIN32__)
                    bool ok;
    FILETIME ftAccess, ftCreate, ftWrite;
    if ( IsDir() )
    {
                extern bool wxGetDirectoryTimes(const wxString& dirname,
                                        FILETIME *, FILETIME *, FILETIME *);

                        ok = wxGetDirectoryTimes(GetPath(wxPATH_GET_VOLUME),
                                 &ftAccess, &ftCreate, &ftWrite);
    }
    else     {
        wxFileHandle fh(GetFullPath(), wxFileHandle::ReadAttr);
        if ( fh.IsOk() )
        {
            ok = ::GetFileTime(fh,
                               dtCreate ? &ftCreate : NULL,
                               dtAccess ? &ftAccess : NULL,
                               dtMod ? &ftWrite : NULL) != 0;
        }
        else
        {
            ok = false;
        }
    }

    if ( ok )
    {
        if ( dtCreate )
            ConvertFileTimeToWx(dtCreate, ftCreate);
        if ( dtAccess )
            ConvertFileTimeToWx(dtAccess, ftAccess);
        if ( dtMod )
            ConvertFileTimeToWx(dtMod, ftWrite);

        return true;
    }
#elif defined(wxHAVE_LSTAT)
        wxStructStat stBuf;
    if ( StatAny(stBuf, *this) )
    {
                        if ( dtAccess )
            dtAccess->Set(static_cast<time_t>(stBuf.st_atime));
        if ( dtMod )
            dtMod->Set(static_cast<time_t>(stBuf.st_mtime));
        if ( dtCreate )
            dtCreate->Set(static_cast<time_t>(stBuf.st_ctime));

        return true;
    }
#else     wxUnusedVar(dtAccess);
    wxUnusedVar(dtMod);
    wxUnusedVar(dtCreate);
#endif 
    wxLogSysError(_("Failed to retrieve file times for '%s'"),
                  GetFullPath().c_str());

    return false;
}

#endif 


#if wxUSE_LONGLONG


wxULongLong wxFileName::GetSize(const wxString &filename)
{
    if (!wxFileExists(filename))
        return wxInvalidSize;

#if defined(__WIN32__)
    wxFileHandle f(filename, wxFileHandle::ReadAttr);
    if (!f.IsOk())
        return wxInvalidSize;

    DWORD lpFileSizeHigh;
    DWORD ret = GetFileSize(f, &lpFileSizeHigh);
    if ( ret == INVALID_FILE_SIZE && ::GetLastError() != NO_ERROR )
        return wxInvalidSize;

    return wxULongLong(lpFileSizeHigh, ret);
#else     wxStructStat st;
    if (wxStat( filename, &st) != 0)
        return wxInvalidSize;
    return wxULongLong(st.st_size);
#endif
}


wxString wxFileName::GetHumanReadableSize(const wxULongLong &bs,
                                          const wxString &nullsize,
                                          int precision,
                                          wxSizeConvention conv)
{
        if ( bs == 0 || bs == wxInvalidSize )
        return nullsize;

            double multiplier = 1024.;
    wxString biInfix;

    switch ( conv )
    {
        case wxSIZE_CONV_TRADITIONAL:
                                    break;

        case wxSIZE_CONV_IEC:
            biInfix = "i";
            break;

        case wxSIZE_CONV_SI:
            multiplier = 1000;
            break;
    }

    const double kiloByteSize = multiplier;
    const double megaByteSize = multiplier * kiloByteSize;
    const double gigaByteSize = multiplier * megaByteSize;
    const double teraByteSize = multiplier * gigaByteSize;

    const double bytesize = bs.ToDouble();

    wxString result;
    if ( bytesize < kiloByteSize )
        result.Printf("%s B", bs.ToString());
    else if ( bytesize < megaByteSize )
        result.Printf("%.*f K%sB", precision, bytesize/kiloByteSize, biInfix);
    else if (bytesize < gigaByteSize)
        result.Printf("%.*f M%sB", precision, bytesize/megaByteSize, biInfix);
    else if (bytesize < teraByteSize)
        result.Printf("%.*f G%sB", precision, bytesize/gigaByteSize, biInfix);
    else
        result.Printf("%.*f T%sB", precision, bytesize/teraByteSize, biInfix);

    return result;
}

wxULongLong wxFileName::GetSize() const
{
    return GetSize(GetFullPath());
}

wxString wxFileName::GetHumanReadableSize(const wxString& failmsg,
                                          int precision,
                                          wxSizeConvention conv) const
{
    return GetHumanReadableSize(GetSize(), failmsg, precision, conv);
}

#endif 
