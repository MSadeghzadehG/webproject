


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/event.h"
    #include "wx/app.h"
    #include "wx/utils.h"
#endif 
#include "wx/cmdline.h"
#include "wx/dynlib.h"

#include "wx/msw/private.h"
#include "wx/msw/seh.h"

#if wxUSE_ON_FATAL_EXCEPTION
    #include "wx/datetime.h"
    #include "wx/msw/crashrpt.h"
#endif 
#ifdef __BORLANDC__
            #define DllMain DllEntryPoint
#endif 
extern int wxEntryReal(int& argc, wxChar **argv);
extern int wxEntryCleanupReal(int& argc, wxChar **argv);


#if wxUSE_BASE


#if wxUSE_ON_FATAL_EXCEPTION

extern EXCEPTION_POINTERS *wxGlobalSEInformation = NULL;

static bool gs_handleExceptions = false;

static void wxFatalExit()
{
        ::ExitProcess(3);
}

unsigned long wxGlobalSEHandler(EXCEPTION_POINTERS *pExcPtrs)
{
    if ( gs_handleExceptions && wxTheApp )
    {
                wxGlobalSEInformation = pExcPtrs;

                wxSEH_TRY
        {
            wxTheApp->OnFatalException();
        }
        wxSEH_IGNORE      
        wxGlobalSEInformation = NULL;

                return EXCEPTION_EXECUTE_HANDLER;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

#ifdef __VISUALC__

void wxSETranslator(unsigned int WXUNUSED(code), EXCEPTION_POINTERS *ep)
{
    switch ( wxGlobalSEHandler(ep) )
    {
        default:
            wxFAIL_MSG( wxT("unexpected wxGlobalSEHandler() return value") );
            
        case EXCEPTION_EXECUTE_HANDLER:
                        #ifndef WXMAKINGDLL
            wxFatalExit();
#endif             break;

        case EXCEPTION_CONTINUE_SEARCH:
                                                            throw;
    }
}

#endif 
bool wxHandleFatalExceptions(bool doit)
{
        gs_handleExceptions = doit;

#if wxUSE_CRASHREPORT
    if ( doit )
    {
                wxChar fullname[MAX_PATH];
        if ( !::GetTempPath(WXSIZEOF(fullname), fullname) )
        {
            wxLogLastError(wxT("GetTempPath"));

                        wxStrcpy(fullname, wxT("c:\\"));
        }

                wxString name = wxString::Format
                        (
#if wxUSE_DATETIME
                            wxT("%s_%s_%lu.dmp"),
#else
                            wxT("%s_%lu.dmp"),
#endif
                            wxTheApp ? (const wxChar*)wxTheApp->GetAppDisplayName().c_str()
                                     : wxT("wxwindows"),
#if wxUSE_DATETIME
                            wxDateTime::Now().Format(wxT("%Y%m%dT%H%M%S")).c_str(),
#endif
                            ::GetCurrentProcessId()
                        );

        wxStrncat(fullname, name, WXSIZEOF(fullname) - wxStrlen(fullname) - 1);

        wxCrashReport::SetFileName(fullname);
    }
#endif 
    return true;
}

int wxEntry(int& argc, wxChar **argv)
{
    DisableAutomaticSETranslator();

    wxSEH_TRY
    {
        return wxEntryReal(argc, argv);
    }
    wxSEH_HANDLE(-1)
}

#else 
int wxEntry(int& argc, wxChar **argv)
{
    return wxEntryReal(argc, argv);
}

#endif 
#endif 
#if wxUSE_GUI


struct wxMSWCommandLineArguments
{
    wxMSWCommandLineArguments() { argc = 0; argv = NULL; }

    void Init(const wxArrayString& args)
    {
        argc = args.size();

                argv = new wxChar *[argc + 1];
        for ( int i = 0; i < argc; i++ )
        {
            argv[i] = wxStrdup(args[i].t_str());
        }

                argv[argc] = NULL;
    }

    ~wxMSWCommandLineArguments()
    {
        if ( !argc )
            return;

        for ( int i = 0; i < argc; i++ )
        {
            free(argv[i]);
        }

        wxDELETEA(argv);
        argc = 0;
    }

    int argc;
    wxChar **argv;
};

static wxMSWCommandLineArguments wxArgs;

static bool
wxMSWEntryCommon(HINSTANCE hInstance, int nCmdShow)
{
        wxSetInstance(hInstance);
#ifdef __WXMSW__
    wxApp::m_nCmdShow = nCmdShow;
#endif

            
        wxArrayString args;

    const wxChar *cmdLine = ::GetCommandLine();
    if ( cmdLine )
    {
        args = wxCmdLineParser::ConvertStringToArgs(cmdLine);
    }

    wxArgs.Init(args);

    return true;
}

WXDLLEXPORT bool wxEntryStart(HINSTANCE hInstance,
                              HINSTANCE WXUNUSED(hPrevInstance),
                              wxCmdLineArgType WXUNUSED(pCmdLine),
                              int nCmdShow)
{
    if ( !wxMSWEntryCommon(hInstance, nCmdShow) )
       return false;

    return wxEntryStart(wxArgs.argc, wxArgs.argv);
}

WXDLLEXPORT int wxEntry(HINSTANCE hInstance,
                        HINSTANCE WXUNUSED(hPrevInstance),
                        wxCmdLineArgType WXUNUSED(pCmdLine),
                        int nCmdShow)
{
    if ( !wxMSWEntryCommon(hInstance, nCmdShow) )
        return -1;

    return wxEntry(wxArgs.argc, wxArgs.argv);
}

#endif 

#if wxUSE_BASE

HINSTANCE wxhInstance = 0;

extern "C" HINSTANCE wxGetInstance()
{
    return wxhInstance;
}

void wxSetInstance(HINSTANCE hInst)
{
    wxhInstance = hInst;
}

#endif 