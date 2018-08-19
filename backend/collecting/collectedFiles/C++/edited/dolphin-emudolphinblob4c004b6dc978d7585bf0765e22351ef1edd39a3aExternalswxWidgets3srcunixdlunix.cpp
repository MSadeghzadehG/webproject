


#include  "wx/wxprec.h"

#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#if wxUSE_DYNLIB_CLASS

#include "wx/dynlib.h"
#include "wx/ffile.h"

#ifndef WX_PRECOMP
    #include "wx/intl.h"
    #include "wx/log.h"
#endif

#ifdef HAVE_DLOPEN
    #include <dlfcn.h>
#endif

#ifdef __DARWIN__
    #include <AvailabilityMacros.h>
#endif

#ifndef RTLD_LAZY
    #define RTLD_LAZY 0
#endif

#ifndef RTLD_NOW
    #define RTLD_NOW 0
#endif

#ifndef RTLD_GLOBAL
    #define RTLD_GLOBAL 0
#endif


#if defined(HAVE_DLOPEN) || defined(__DARWIN__)
    #define USE_POSIX_DL_FUNCS
#elif !defined(HAVE_SHL_LOAD)
    #error "Don't know how to load dynamic libraries on this platform!"
#endif



wxDllType wxDynamicLibrary::GetProgramHandle()
{
#ifdef USE_POSIX_DL_FUNCS
   return dlopen(0, RTLD_LAZY);
#else
   return PROG_HANDLE;
#endif
}


wxDllType wxDynamicLibrary::RawLoad(const wxString& libname, int flags)
{
    wxASSERT_MSG( !(flags & wxDL_NOW) || !(flags & wxDL_LAZY),
                  wxT("wxDL_LAZY and wxDL_NOW are mutually exclusive.") );

#ifdef USE_POSIX_DL_FUNCS
                int rtldFlags = flags & wxDL_LAZY ? RTLD_LAZY : RTLD_NOW;

    if ( flags & wxDL_GLOBAL )
        rtldFlags |= RTLD_GLOBAL;

    return dlopen(libname.fn_str(), rtldFlags);
#else     int shlFlags = 0;

    if ( flags & wxDL_LAZY )
    {
        shlFlags |= BIND_DEFERRED;
    }
    else if ( flags & wxDL_NOW )
    {
        shlFlags |= BIND_IMMEDIATE;
    }

    return shl_load(libname.fn_str(), shlFlags, 0);
#endif }


void wxDynamicLibrary::Unload(wxDllType handle)
{
#ifdef wxHAVE_DYNLIB_ERROR
    int rc =
#endif

#ifdef USE_POSIX_DL_FUNCS
    dlclose(handle);
#else     shl_unload(handle);
#endif 
#if defined(USE_POSIX_DL_FUNCS) && defined(wxHAVE_DYNLIB_ERROR)
    if ( rc != 0 )
        Error();
#endif
}


void *wxDynamicLibrary::RawGetSymbol(wxDllType handle, const wxString& name)
{
    void *symbol;

#ifdef USE_POSIX_DL_FUNCS
    symbol = dlsym(handle, name.fn_str());
#else             if ( shl_findsym(&handle, name.fn_str(), TYPE_UNDEFINED, &symbol) != 0 )
        symbol = 0;
#endif 
    return symbol;
}


#ifdef wxHAVE_DYNLIB_ERROR


void wxDynamicLibrary::Error()
{
    wxString err(dlerror());

    if ( err.empty() )
        err = _("Unknown dynamic library error");

    wxLogError(wxT("%s"), err);
}

#endif 

class wxDynamicLibraryDetailsCreator
{
public:
        static wxDynamicLibraryDetails *
    New(void *start, void *end, const wxString& path)
    {
        wxDynamicLibraryDetails *details = new wxDynamicLibraryDetails;
        details->m_path = path;
        details->m_name = path.AfterLast(wxT('/'));
        details->m_address = start;
        details->m_length = (char *)end - (char *)start;

                const size_t posExt = path.rfind(wxT(".so"));
        if ( posExt != wxString::npos )
        {
            if ( path.c_str()[posExt + 3] == wxT('.') )
            {
                                details->m_version.assign(path, posExt + 4, wxString::npos);
            }
            else
            {
                size_t posDash = path.find_last_of(wxT('-'), posExt);
                if ( posDash != wxString::npos )
                {
                                        posDash++;
                    details->m_version.assign(path, posDash, posExt - posDash);
                }
            }
        }

        return details;
    }
};


wxDynamicLibraryDetailsArray wxDynamicLibrary::ListLoaded()
{
    wxDynamicLibraryDetailsArray dlls;

#ifdef __LINUX__
        wxFFile file(wxT("/proc/self/maps"));
    if ( file.IsOpened() )
    {
                wxString pathCur;
        void *startCur = NULL,
             *endCur = NULL;

        char path[1024];
        char buf[1024];
        while ( fgets(buf, WXSIZEOF(buf), file.fp()) )
        {
                        void *start,
                 *end;
            switch ( sscanf(buf, "%p-%p %*4s %*p %*02x:%*02x %*d %1024s\n",
                            &start, &end, path) )
            {
                case 2:
                                        path[0] = '\0';
                    break;

                case 3:
                                        break;

                default:
                                        buf[strlen(buf) - 1] = '\0';
                    wxLogDebug(wxT("Failed to parse line \"%s\" in /proc/self/maps."),
                               buf);
                    continue;
            }

            wxASSERT_MSG( start >= endCur,
                          wxT("overlapping regions in /proc/self/maps?") );

            wxString pathNew = wxString::FromAscii(path);
            if ( pathCur.empty() )
            {
                                pathCur = pathNew;
                startCur = start;
                endCur = end;
            }
            else if ( pathCur == pathNew && endCur == end )
            {
                                endCur = end;
            }
            else             {
                dlls.Add(wxDynamicLibraryDetailsCreator::New(startCur,
                                                             endCur,
                                                             pathCur));
                pathCur.clear();
            }
        }
    }
#endif 
    return dlls;
}



void* wxDynamicLibrary::GetModuleFromAddress(const void* addr, wxString* path)
{
#ifdef HAVE_DLADDR
    Dl_info di = { 0 };

        if ( dladdr(const_cast<void*>(addr), &di) == 0 )
        return NULL;

    if ( path )
        *path = di.dli_fname;

    return di.dli_fbase;
#endif 
    return NULL;
}


#endif 
