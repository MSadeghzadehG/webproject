


#include  "wx/wxprec.h"

#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#if wxUSE_DYNLIB_CLASS

#include "wx/msw/private.h"
#include "wx/msw/debughlp.h"
#include "wx/filename.h"

#ifdef __VISUALC__
    #pragma comment(lib, "version")
#endif


class WXDLLIMPEXP_BASE wxDynamicLibraryDetailsCreator
{
public:
        struct EnumModulesProcParams
    {
        wxDynamicLibraryDetailsArray *dlls;
    };

    static BOOL CALLBACK
    EnumModulesProc(const wxChar* name, DWORD64 base, ULONG size, PVOID data);
};


static wxString GetFileVersion(const wxString& filename)
{
    wxString ver;
    wxChar *pc = const_cast<wxChar *>((const wxChar*) filename.t_str());

    DWORD dummy;
    DWORD sizeVerInfo = ::GetFileVersionInfoSize(pc, &dummy);
    if ( sizeVerInfo )
    {
        wxCharBuffer buf(sizeVerInfo);
        if ( ::GetFileVersionInfo(pc, 0, sizeVerInfo, buf.data()) )
        {
            void *pVer;
            UINT sizeInfo;
            if ( ::VerQueryValue(buf.data(),
                                    const_cast<wxChar *>(wxT("\\")),
                                    &pVer,
                                    &sizeInfo) )
            {
                VS_FIXEDFILEINFO *info = (VS_FIXEDFILEINFO *)pVer;
                ver.Printf(wxT("%d.%d.%d.%d"),
                            HIWORD(info->dwFileVersionMS),
                            LOWORD(info->dwFileVersionMS),
                            HIWORD(info->dwFileVersionLS),
                            LOWORD(info->dwFileVersionLS));
            }
        }
    }

    return ver;
}



BOOL CALLBACK
wxDynamicLibraryDetailsCreator::EnumModulesProc(const wxChar* name,
                                                DWORD64 base,
                                                ULONG size,
                                                void *data)
{
    EnumModulesProcParams *params = (EnumModulesProcParams *)data;

    wxDynamicLibraryDetails *details = new wxDynamicLibraryDetails;

        details->m_name = name;
    details->m_address = wxUIntToPtr(base);
    details->m_length = size;

        const HMODULE hmod = wxDynamicLibrary::MSWGetModuleHandle
                         (
                            details->m_name,
                            details->m_address
                         );
    if ( hmod )
    {
        wxString fullname = wxGetFullModuleName(hmod);
        if ( !fullname.empty() )
        {
            details->m_path = fullname;
            details->m_version = GetFileVersion(fullname);
        }
    }

    params->dlls->Add(details);

        return TRUE;
}



wxDllType wxDynamicLibrary::GetProgramHandle()
{
    return (wxDllType)::GetModuleHandle(NULL);
}


#ifndef MAX_PATH
    #define MAX_PATH 260        #endif


wxDllType
wxDynamicLibrary::RawLoad(const wxString& libname, int flags)
{
    if (flags & wxDL_GET_LOADED)
        return ::GetModuleHandle(libname.t_str());

    return ::LoadLibrary(libname.t_str());
}


void wxDynamicLibrary::Unload(wxDllType handle)
{
    ::FreeLibrary(handle);
}


void *wxDynamicLibrary::RawGetSymbol(wxDllType handle, const wxString& name)
{
    return (void *)::GetProcAddress(handle,
                                            name.ToAscii()
                                   );
}



wxDynamicLibraryDetailsArray wxDynamicLibrary::ListLoaded()
{
    wxDynamicLibraryDetailsArray dlls;

#if wxUSE_DBGHELP
    if ( wxDbgHelpDLL::Init() )
    {
        wxDynamicLibraryDetailsCreator::EnumModulesProcParams params;
        params.dlls = &dlls;

        if ( !wxDbgHelpDLL::CallEnumerateLoadedModules
                            (
                                ::GetCurrentProcess(),
                                wxDynamicLibraryDetailsCreator::EnumModulesProc,
                                &params
                            ) )
        {
            wxLogLastError(wxT("EnumerateLoadedModules"));
        }
    }
#endif 
    return dlls;
}


namespace
{

HMODULE CallGetModuleHandleEx(const void* addr)
{
    typedef BOOL (WINAPI *GetModuleHandleEx_t)(DWORD, LPCTSTR, HMODULE *);
    static const GetModuleHandleEx_t INVALID_FUNC_PTR = (GetModuleHandleEx_t)-1;

    static GetModuleHandleEx_t s_pfnGetModuleHandleEx = INVALID_FUNC_PTR;
    if ( s_pfnGetModuleHandleEx == INVALID_FUNC_PTR )
    {
        wxDynamicLibrary dll(wxT("kernel32.dll"), wxDL_VERBATIM);

        wxDL_INIT_FUNC_AW(s_pfn, GetModuleHandleEx, dll);

            }

    if ( !s_pfnGetModuleHandleEx )
        return NULL;

            HMODULE hmod;
    if ( !s_pfnGetModuleHandleEx(6, (LPCTSTR)addr, &hmod) )
        return NULL;

    return hmod;
}

} 

void* wxDynamicLibrary::GetModuleFromAddress(const void* addr, wxString* path)
{
    HMODULE hmod = CallGetModuleHandleEx(addr);
    if ( !hmod )
    {
        wxLogLastError(wxT("GetModuleHandleEx"));
        return NULL;
    }

    if ( path )
    {
        TCHAR libname[MAX_PATH];
        if ( !::GetModuleFileName(hmod, libname, MAX_PATH) )
        {
                                                            wxLogLastError(wxT("GetModuleFromAddress"));
            return NULL;
        }

        libname[MAX_PATH-1] = wxT('\0');

        *path = libname;
    }

            return reinterpret_cast<void *>(hmod);
}


WXHMODULE wxDynamicLibrary::MSWGetModuleHandle(const wxString& name, void *addr)
{
                    HMODULE hmod = CallGetModuleHandleEx(addr);

    return hmod ? hmod : ::GetModuleHandle(name.t_str());
}

#endif 
