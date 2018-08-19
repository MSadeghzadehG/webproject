


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #ifdef __WINDOWS__
        #include  "wx/msw/wrapwin.h"      #endif
    #include "wx/list.h"
    #include "wx/app.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/utils.h"
    #include "wx/wxcrtvararg.h"
#endif 
#include "wx/apptrait.h"
#include "wx/cmdline.h"
#include "wx/confbase.h"
#include "wx/evtloop.h"
#include "wx/filename.h"
#include "wx/msgout.h"
#include "wx/scopedptr.h"
#include "wx/sysopt.h"
#include "wx/tokenzr.h"
#include "wx/thread.h"
#include "wx/stdpaths.h"

#if wxUSE_EXCEPTIONS
            #if __cplusplus >= 201103L
                                                        #ifdef __GNUC__
                                                #if !defined(__GCC_ATOMIC_INT_LOCK_FREE) \
                    || __GCC_ATOMIC_INT_LOCK_FREE > 1
                #define HAS_EXCEPTION_PTR
            #endif
        #else
            #define HAS_EXCEPTION_PTR
        #endif
    #elif wxCHECK_VISUALC_VERSION(11)
                                                #define HAS_EXCEPTION_PTR
    #endif

    #ifdef HAS_EXCEPTION_PTR
        #include <exception>                #include <utility>              #endif

    #if wxUSE_STL
        #include <exception>
        #include <typeinfo>
    #endif
#endif 
#if !defined(__WINDOWS__)
  #include  <signal.h>      #endif  
#include <locale.h>

#if wxUSE_FONTMAP
    #include "wx/fontmap.h"
#endif 
#if wxDEBUG_LEVEL
    #if wxUSE_STACKWALKER
        #include "wx/stackwalk.h"
        #ifdef __WINDOWS__
            #include "wx/msw/debughlp.h"
        #endif
    #endif 
    #include "wx/recguard.h"
#endif 
#if wxABI_VERSION != wxMAJOR_VERSION * 10000 + wxMINOR_VERSION * 100 + 99
#error "wxABI_VERSION should not be defined when compiling the library"
#endif


#if wxDEBUG_LEVEL
        static bool DoShowAssertDialog(const wxString& msg);

            static
    void ShowAssertDialog(const wxString& file,
                          int line,
                          const wxString& func,
                          const wxString& cond,
                          const wxString& msg,
                          wxAppTraits *traits = NULL);
#endif 
#ifdef __WXDEBUG__
        static void LINKAGEMODE SetTraceMasks();
#endif 

wxAppConsole *wxAppConsoleBase::ms_appInstance = NULL;

wxAppInitializerFunction wxAppConsoleBase::ms_appInitFn = NULL;

wxSocketManager *wxAppTraitsBase::ms_manager = NULL;

WXDLLIMPEXP_DATA_BASE(wxList) wxPendingDelete;


wxDEFINE_TIED_SCOPED_PTR_TYPE(wxEventLoopBase)



wxAppConsoleBase::wxAppConsoleBase()
{
    m_traits = NULL;
    m_mainLoop = NULL;
    m_bDoPendingEventProcessing = true;

    ms_appInstance = static_cast<wxAppConsole *>(this);

#ifdef __WXDEBUG__
    SetTraceMasks();
#if wxUSE_UNICODE
                wxDELETE(m_traits);
#endif
#endif

    wxEvtHandler::AddFilter(this);
}

wxAppConsoleBase::~wxAppConsoleBase()
{
    wxEvtHandler::RemoveFilter(this);

            ms_appInstance = NULL;

    delete m_traits;
}


bool wxAppConsoleBase::Initialize(int& WXUNUSED(argc), wxChar **WXUNUSED(argv))
{
#if defined(__WINDOWS__)
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
#endif

    return true;
}

wxString wxAppConsoleBase::GetAppName() const
{
    wxString name = m_appName;
    if ( name.empty() )
    {
        if ( argv )
        {
                        wxFileName::SplitPath(argv[0], NULL, &name, NULL);
        }
#if wxUSE_STDPATHS
        else         {
            const wxString pathExe = wxStandardPaths::Get().GetExecutablePath();
            if ( !pathExe.empty() )
            {
                wxFileName::SplitPath(pathExe, NULL, &name, NULL);
            }
        }
#endif     }
    return name;
}

wxString wxAppConsoleBase::GetAppDisplayName() const
{
        if ( !m_appDisplayName.empty() )
        return m_appDisplayName;

            if ( !m_appName.empty() )
        return m_appName;

            return GetAppName().Capitalize();
}

wxEventLoopBase *wxAppConsoleBase::CreateMainLoop()
{
    return GetTraits()->CreateEventLoop();
}

void wxAppConsoleBase::CleanUp()
{
    wxDELETE(m_mainLoop);
}


bool wxAppConsoleBase::OnInit()
{
#if wxUSE_CMDLINE_PARSER
    wxCmdLineParser parser(argc, argv);

    OnInitCmdLine(parser);

    bool cont;
    switch ( parser.Parse(false ) )
    {
        case -1:
            cont = OnCmdLineHelp(parser);
            break;

        case 0:
            cont = OnCmdLineParsed(parser);
            break;

        default:
            cont = OnCmdLineError(parser);
            break;
    }

    if ( !cont )
        return false;
#endif 
    return true;
}

int wxAppConsoleBase::OnRun()
{
    return MainLoop();
}

void wxAppConsoleBase::OnLaunched()
{    
}

int wxAppConsoleBase::OnExit()
{
            DeletePendingObjects();

#if wxUSE_CONFIG
            delete wxConfigBase::Set(NULL);
#endif 
    return 0;
}

void wxAppConsoleBase::Exit()
{
    if (m_mainLoop != NULL)
        ExitMainLoop();
    else
        exit(-1);
}


wxAppTraits *wxAppConsoleBase::CreateTraits()
{
    return new wxConsoleAppTraits;
}

wxAppTraits *wxAppConsoleBase::GetTraits()
{
        if ( !m_traits )
    {
        m_traits = CreateTraits();

        wxASSERT_MSG( m_traits, wxT("wxApp::CreateTraits() failed?") );
    }

    return m_traits;
}


wxAppTraits *wxAppConsoleBase::GetTraitsIfExists()
{
    wxAppConsole * const app = GetInstance();
    return app ? app->GetTraits() : NULL;
}


wxAppTraits& wxAppConsoleBase::GetValidTraits()
{
    static wxConsoleAppTraits s_traitsConsole;
    wxAppTraits* const traits = (wxTheApp ? wxTheApp->GetTraits() : NULL);

    return *(traits ? traits : &s_traitsConsole);
}


int wxAppConsoleBase::MainLoop()
{
    wxEventLoopBaseTiedPtr mainLoop(&m_mainLoop, CreateMainLoop());

    if (wxTheApp)
        wxTheApp->OnLaunched();
    
    return m_mainLoop ? m_mainLoop->Run() : -1;
}

void wxAppConsoleBase::ExitMainLoop()
{
            if ( m_mainLoop && m_mainLoop->IsRunning() )
    {
        m_mainLoop->Exit(0);
    }
}

bool wxAppConsoleBase::Pending()
{
                wxEventLoopBase * const loop = wxEventLoopBase::GetActive();

    return loop && loop->Pending();
}

bool wxAppConsoleBase::Dispatch()
{
        wxEventLoopBase * const loop = wxEventLoopBase::GetActive();

    return loop && loop->Dispatch();
}

bool wxAppConsoleBase::Yield(bool onlyIfNeeded)
{
    wxEventLoopBase * const loop = wxEventLoopBase::GetActive();
    if ( loop )
       return loop->Yield(onlyIfNeeded);

    wxScopedPtr<wxEventLoopBase> tmpLoop(CreateMainLoop());
    return tmpLoop->Yield(onlyIfNeeded);
}

void wxAppConsoleBase::WakeUpIdle()
{
    wxEventLoopBase * const loop = wxEventLoopBase::GetActive();

    if ( loop )
        loop->WakeUp();
}

bool wxAppConsoleBase::ProcessIdle()
{
        wxIdleEvent event;
    event.SetEventObject(this);
    ProcessEvent(event);

#if wxUSE_LOG
            wxLog::FlushActive();
#endif

        DeletePendingObjects();

    return event.MoreRequested();
}

bool wxAppConsoleBase::UsesEventLoop() const
{
            return wxEventLoopBase::GetActive() != NULL;
}



bool wxAppConsoleBase::IsMainLoopRunning()
{
    const wxAppConsole * const app = GetInstance();

    return app && app->m_mainLoop != NULL;
}

int wxAppConsoleBase::FilterEvent(wxEvent& WXUNUSED(event))
{
        return Event_Skip;
}

void wxAppConsoleBase::DelayPendingEventHandler(wxEvtHandler* toDelay)
{
    wxENTER_CRIT_SECT(m_handlersWithPendingEventsLocker);

            m_handlersWithPendingEvents.Remove(toDelay);

    if (m_handlersWithPendingDelayedEvents.Index(toDelay) == wxNOT_FOUND)
        m_handlersWithPendingDelayedEvents.Add(toDelay);

    wxLEAVE_CRIT_SECT(m_handlersWithPendingEventsLocker);
}

void wxAppConsoleBase::RemovePendingEventHandler(wxEvtHandler* toRemove)
{
    wxENTER_CRIT_SECT(m_handlersWithPendingEventsLocker);

    if (m_handlersWithPendingEvents.Index(toRemove) != wxNOT_FOUND)
    {
        m_handlersWithPendingEvents.Remove(toRemove);

                wxASSERT_MSG( m_handlersWithPendingEvents.Index(toRemove) == wxNOT_FOUND,
                        "Handler occurs twice in the m_handlersWithPendingEvents list!" );
    }
    
    if (m_handlersWithPendingDelayedEvents.Index(toRemove) != wxNOT_FOUND)
    {
        m_handlersWithPendingDelayedEvents.Remove(toRemove);

                wxASSERT_MSG( m_handlersWithPendingDelayedEvents.Index(toRemove) == wxNOT_FOUND,
                        "Handler occurs twice in m_handlersWithPendingDelayedEvents list!" );
    }
    
    wxLEAVE_CRIT_SECT(m_handlersWithPendingEventsLocker);
}

void wxAppConsoleBase::AppendPendingEventHandler(wxEvtHandler* toAppend)
{
    wxENTER_CRIT_SECT(m_handlersWithPendingEventsLocker);

    if ( m_handlersWithPendingEvents.Index(toAppend) == wxNOT_FOUND )
        m_handlersWithPendingEvents.Add(toAppend);

    wxLEAVE_CRIT_SECT(m_handlersWithPendingEventsLocker);
}

bool wxAppConsoleBase::HasPendingEvents() const
{
    wxENTER_CRIT_SECT(const_cast<wxAppConsoleBase*>(this)->m_handlersWithPendingEventsLocker);

    bool has = !m_handlersWithPendingEvents.IsEmpty();

    wxLEAVE_CRIT_SECT(const_cast<wxAppConsoleBase*>(this)->m_handlersWithPendingEventsLocker);

    return has;
}

void wxAppConsoleBase::SuspendProcessingOfPendingEvents()
{
    m_bDoPendingEventProcessing = false;
}

void wxAppConsoleBase::ResumeProcessingOfPendingEvents()
{
    m_bDoPendingEventProcessing = true;
}

void wxAppConsoleBase::ProcessPendingEvents()
{
    if ( m_bDoPendingEventProcessing )
    {
        wxENTER_CRIT_SECT(m_handlersWithPendingEventsLocker);

        wxCHECK_RET( m_handlersWithPendingDelayedEvents.IsEmpty(),
                     "this helper list should be empty" );

                        while (!m_handlersWithPendingEvents.IsEmpty())
        {
                                    wxLEAVE_CRIT_SECT(m_handlersWithPendingEventsLocker);

                                                            m_handlersWithPendingEvents[0]->ProcessPendingEvents();

            wxENTER_CRIT_SECT(m_handlersWithPendingEventsLocker);
        }

                                                if (!m_handlersWithPendingDelayedEvents.IsEmpty())
        {
            WX_APPEND_ARRAY(m_handlersWithPendingEvents, m_handlersWithPendingDelayedEvents);
            m_handlersWithPendingDelayedEvents.Clear();
        }

        wxLEAVE_CRIT_SECT(m_handlersWithPendingEventsLocker);
    }
}

void wxAppConsoleBase::DeletePendingEvents()
{
    wxENTER_CRIT_SECT(m_handlersWithPendingEventsLocker);

    wxCHECK_RET( m_handlersWithPendingDelayedEvents.IsEmpty(),
                 "this helper list should be empty" );

    for (unsigned int i=0; i<m_handlersWithPendingEvents.GetCount(); i++)
        m_handlersWithPendingEvents[i]->DeletePendingEvents();

    m_handlersWithPendingEvents.Clear();

    wxLEAVE_CRIT_SECT(m_handlersWithPendingEventsLocker);
}


bool wxAppConsoleBase::IsScheduledForDestruction(wxObject *object) const
{
    return wxPendingDelete.Member(object);
}

void wxAppConsoleBase::ScheduleForDestruction(wxObject *object)
{
    if ( !UsesEventLoop() )
    {
                delete object;
        return;
    }
    
    if ( !wxPendingDelete.Member(object) )
        wxPendingDelete.Append(object);
}

void wxAppConsoleBase::DeletePendingObjects()
{
    wxList::compatibility_iterator node = wxPendingDelete.GetFirst();
    while (node)
    {
        wxObject *obj = node->GetData();

                                if ( wxPendingDelete.Member(obj) )
            wxPendingDelete.Erase(node);

        delete obj;

                        node = wxPendingDelete.GetFirst();
    }
}


#if wxUSE_EXCEPTIONS

void
wxAppConsoleBase::HandleEvent(wxEvtHandler *handler,
                              wxEventFunction func,
                              wxEvent& event) const
{
        (handler->*func)(event);
}

void wxAppConsoleBase::CallEventHandler(wxEvtHandler *handler,
                                        wxEventFunctor& functor,
                                        wxEvent& event) const
{
            wxEventFunction eventFunction = functor.GetEvtMethod();

    if ( eventFunction )
        HandleEvent(handler, eventFunction, event);
    else
        functor(handler, event);
}

void wxAppConsoleBase::OnUnhandledException()
{
#ifdef __WXDEBUG__
            wxString what;
    try
    {
        throw;
    }
#if wxUSE_STL
    catch ( std::exception& e )
    {
        what.Printf("std::exception of type \"%s\", what() = \"%s\"",
                    typeid(e).name(), e.what());
    }
#endif     catch ( ... )
    {
        what = "unknown exception";
    }

    wxMessageOutputBest().Printf(
        "*** Caught unhandled %s; terminating\n", what
    );
#endif }


bool wxAppConsoleBase::OnExceptionInMainLoop()
{
    throw;
}

#ifdef HAS_EXCEPTION_PTR
static std::exception_ptr gs_storedException;

bool wxAppConsoleBase::StoreCurrentException()
{
    if ( gs_storedException )
    {
                                        return false;
    }

    gs_storedException = std::current_exception();

    return true;
}

void wxAppConsoleBase::RethrowStoredException()
{
    if ( gs_storedException )
    {
        std::exception_ptr storedException;
        std::swap(storedException, gs_storedException);

        std::rethrow_exception(storedException);
    }
}

#else 
bool wxAppConsoleBase::StoreCurrentException()
{
    return false;
}

void wxAppConsoleBase::RethrowStoredException()
{
}

#endif 
#endif 

#if wxUSE_CMDLINE_PARSER

#define OPTION_VERBOSE "verbose"

void wxAppConsoleBase::OnInitCmdLine(wxCmdLineParser& parser)
{
        static const wxCmdLineEntryDesc cmdLineDesc[] =
    {
        {
            wxCMD_LINE_SWITCH,
            "h",
            "help",
            gettext_noop("show this help message"),
            wxCMD_LINE_VAL_NONE,
            wxCMD_LINE_OPTION_HELP
        },

#if wxUSE_LOG
        {
            wxCMD_LINE_SWITCH,
            NULL,
            OPTION_VERBOSE,
            gettext_noop("generate verbose log messages"),
            wxCMD_LINE_VAL_NONE,
            0x0
        },
#endif 
                wxCMD_LINE_DESC_END
    };

    parser.SetDesc(cmdLineDesc);
}

bool wxAppConsoleBase::OnCmdLineParsed(wxCmdLineParser& parser)
{
#if wxUSE_LOG
    if ( parser.Found(OPTION_VERBOSE) )
    {
        wxLog::SetVerbose(true);
    }
#else
    wxUnusedVar(parser);
#endif 
    return true;
}

bool wxAppConsoleBase::OnCmdLineHelp(wxCmdLineParser& parser)
{
    parser.Usage();

    return false;
}

bool wxAppConsoleBase::OnCmdLineError(wxCmdLineParser& parser)
{
    parser.Usage();

    return false;
}

#endif 


bool wxAppConsoleBase::CheckBuildOptions(const char *optionsSignature,
                                         const char *componentName)
{
#if 0     printf("checking build options object '%s' (ptr %p) in '%s'\n",
             optionsSignature, optionsSignature, componentName);
#endif

    if ( strcmp(optionsSignature, WX_BUILD_OPTIONS_SIGNATURE) != 0 )
    {
        wxString lib = wxString::FromAscii(WX_BUILD_OPTIONS_SIGNATURE);
        wxString prog = wxString::FromAscii(optionsSignature);
        wxString progName = wxString::FromAscii(componentName);
        wxString msg;

        msg.Printf(wxT("Mismatch between the program and library build versions detected.\nThe library used %s,\nand %s used %s."),
                   lib.c_str(), progName.c_str(), prog.c_str());

        wxLogFatalError(msg.c_str());

                return false;
    }

    return true;
}

void wxAppConsoleBase::OnAssertFailure(const wxChar *file,
                                       int line,
                                       const wxChar *func,
                                       const wxChar *cond,
                                       const wxChar *msg)
{
#if wxDEBUG_LEVEL
    ShowAssertDialog(file, line, func, cond, msg, GetTraits());
#else
                wxUnusedVar(file);
    wxUnusedVar(line);
    wxUnusedVar(func);
    wxUnusedVar(cond);
    wxUnusedVar(msg);
#endif }

void wxAppConsoleBase::OnAssert(const wxChar *file,
                                int line,
                                const wxChar *cond,
                                const wxChar *msg)
{
    OnAssertFailure(file, line, NULL, cond, msg);
}


void wxAppConsoleBase::SetCLocale()
{
                wxSetlocale(LC_ALL, "");
}



#if wxUSE_LOG

wxLog *wxConsoleAppTraitsBase::CreateLogTarget()
{
    return new wxLogStderr;
}

#endif 
wxMessageOutput *wxConsoleAppTraitsBase::CreateMessageOutput()
{
    return new wxMessageOutputStderr;
}

#if wxUSE_FONTMAP

wxFontMapper *wxConsoleAppTraitsBase::CreateFontMapper()
{
    return (wxFontMapper *)new wxFontMapperBase;
}

#endif 
wxRendererNative *wxConsoleAppTraitsBase::CreateRenderer()
{
        return NULL;
}

bool wxConsoleAppTraitsBase::ShowAssertDialog(const wxString& msg)
{
    return wxAppTraitsBase::ShowAssertDialog(msg);
}

bool wxConsoleAppTraitsBase::HasStderr()
{
        return true;
}


#if wxUSE_THREADS
void wxMutexGuiEnterImpl();
void wxMutexGuiLeaveImpl();

void wxAppTraitsBase::MutexGuiEnter()
{
    wxMutexGuiEnterImpl();
}

void wxAppTraitsBase::MutexGuiLeave()
{
    wxMutexGuiLeaveImpl();
}

void WXDLLIMPEXP_BASE wxMutexGuiEnter()
{
    wxAppTraits * const traits = wxAppConsoleBase::GetTraitsIfExists();
    if ( traits )
        traits->MutexGuiEnter();
}

void WXDLLIMPEXP_BASE wxMutexGuiLeave()
{
    wxAppTraits * const traits = wxAppConsoleBase::GetTraitsIfExists();
    if ( traits )
        traits->MutexGuiLeave();
}
#endif 
bool wxAppTraitsBase::ShowAssertDialog(const wxString& msgOriginal)
{
#if wxDEBUG_LEVEL
    wxString msg;

#if wxUSE_STACKWALKER
    const wxString stackTrace = GetAssertStackTrace();
    if ( !stackTrace.empty() )
    {
        msg << wxT("\n\nCall stack:\n") << stackTrace;

        wxMessageOutputDebug().Output(msg);
    }
#endif 
    return DoShowAssertDialog(msgOriginal + msg);
#else     wxUnusedVar(msgOriginal);

    return false;
#endif }

#if wxUSE_STACKWALKER
wxString wxAppTraitsBase::GetAssertStackTrace()
{
#if wxDEBUG_LEVEL

#if !defined(__WINDOWS__)
            wxFprintf(stderr, "Collecting stack trace information, please wait...");
    fflush(stderr);
#endif 

    class StackDump : public wxStackWalker
    {
    public:
        StackDump() { m_numFrames = 0; }

        const wxString& GetStackTrace() const { return m_stackTrace; }

    protected:
        virtual void OnStackFrame(const wxStackFrame& frame) wxOVERRIDE
        {
                                                if ( m_numFrames++ > 20 )
                return;

            m_stackTrace << wxString::Format(wxT("[%02u] "), m_numFrames);

            const wxString name = frame.GetName();
            if ( name.StartsWith("wxOnAssert") )
            {
                                                                                                m_stackTrace.clear();
                m_numFrames = 0;
                return;
            }

            if ( !name.empty() )
            {
                m_stackTrace << wxString::Format(wxT("%-40s"), name.c_str());
            }
            else
            {
                m_stackTrace << wxString::Format(wxT("%p"), frame.GetAddress());
            }

            if ( frame.HasSourceLocation() )
            {
                m_stackTrace << wxT('\t')
                             << frame.GetFileName()
                             << wxT(':')
                             << frame.GetLine();
            }

            m_stackTrace << wxT('\n');
        }

    private:
        wxString m_stackTrace;
        unsigned m_numFrames;
    };

    StackDump dump;
    dump.Walk();
    return dump.GetStackTrace();
#else             return wxString();
#endif }
#endif 


void wxExit()
{
    if ( wxTheApp )
    {
        wxTheApp->Exit();
    }
    else
    {
                exit(-1);
    }
}

void wxWakeUpIdle()
{
    if ( wxTheApp )
    {
        wxTheApp->WakeUpIdle();
    }
    }

bool wxAssertIsEqual(int x, int y)
{
    return x == y;
}

void wxAbort()
{
    abort();
}

#if wxDEBUG_LEVEL

#ifndef wxTrap

void wxTrap()
{
#if defined(__WINDOWS__)
    DebugBreak();
#elif defined(_MSL_USING_MW_C_HEADERS) && _MSL_USING_MW_C_HEADERS
    Debugger();
#elif defined(__UNIX__)
    raise(SIGTRAP);
#else
    #endif }

#endif 
static void
wxDefaultAssertHandler(const wxString& file,
                       int line,
                       const wxString& func,
                       const wxString& cond,
                       const wxString& msg)
{
        if ( wxSystemOptions::GetOptionInt("exit-on-assert") )
        wxAbort();

        static int s_bInAssert = 0;

    wxRecursionGuard guard(s_bInAssert);
    if ( guard.IsInside() )
    {
                wxTrap();

        return;
    }

    if ( !wxTheApp )
    {
                        ShowAssertDialog(file, line, func, cond, msg);
    }
    else
    {
                        wxTheApp->OnAssertFailure(file.c_str(), line, func.c_str(),
                                  cond.c_str(), msg.c_str());
    }
}

wxAssertHandler_t wxTheAssertHandler = wxDefaultAssertHandler;

void wxSetDefaultAssertHandler()
{
    wxTheAssertHandler = wxDefaultAssertHandler;
}

void wxOnAssert(const wxString& file,
                int line,
                const wxString& func,
                const wxString& cond,
                const wxString& msg)
{
    wxTheAssertHandler(file, line, func, cond, msg);
}

void wxOnAssert(const wxString& file,
                int line,
                const wxString& func,
                const wxString& cond)
{
    wxTheAssertHandler(file, line, func, cond, wxString());
}

void wxOnAssert(const wxChar *file,
                int line,
                const char *func,
                const wxChar *cond,
                const wxChar *msg)
{
            #if wxUSE_UNICODE
    if ( wxTheAssertHandler )
#endif         wxTheAssertHandler(file, line, func, cond, msg);
}

void wxOnAssert(const char *file,
                int line,
                const char *func,
                const char *cond,
                const wxString& msg)
{
    wxTheAssertHandler(file, line, func, cond, msg);
}

void wxOnAssert(const char *file,
                int line,
                const char *func,
                const char *cond,
                const wxCStrData& msg)
{
    wxTheAssertHandler(file, line, func, cond, msg);
}

#if wxUSE_UNICODE
void wxOnAssert(const char *file,
                int line,
                const char *func,
                const char *cond)
{
    wxTheAssertHandler(file, line, func, cond, wxString());
}

void wxOnAssert(const char *file,
                int line,
                const char *func,
                const char *cond,
                const char *msg)
{
    wxTheAssertHandler(file, line, func, cond, msg);
}

void wxOnAssert(const char *file,
                int line,
                const char *func,
                const char *cond,
                const wxChar *msg)
{
    wxTheAssertHandler(file, line, func, cond, msg);
}
#endif 
#endif 

#ifdef __WXDEBUG__

static void LINKAGEMODE SetTraceMasks()
{
#if wxUSE_LOG
    wxString mask;
    if ( wxGetEnv(wxT("WXTRACE"), &mask) )
    {
        wxStringTokenizer tkn(mask, wxT(",;:"));
        while ( tkn.HasMoreTokens() )
            wxLog::AddTraceMask(tkn.GetNextToken());
    }
#endif }

#endif 
#if wxDEBUG_LEVEL

bool wxTrapInAssert = false;

static
bool DoShowAssertDialog(const wxString& msg)
{
    #if defined(__WINDOWS__)
    wxString msgDlg(msg);

                msgDlg += wxT("\nDo you want to stop the program?\n")
              wxT("You can also choose [Cancel] to suppress ")
              wxT("further warnings.");

    switch ( ::MessageBox(NULL, msgDlg.t_str(), wxT("wxWidgets Debug Alert"),
                          MB_YESNOCANCEL | MB_ICONSTOP ) )
    {
        case IDYES:
                                                                                                wxTrapInAssert = true;
            break;

        case IDCANCEL:
                        return true;

            }
#else     wxUnusedVar(msg);
#endif 
        return false;
}

static
void ShowAssertDialog(const wxString& file,
                      int line,
                      const wxString& func,
                      const wxString& cond,
                      const wxString& msgUser,
                      wxAppTraits *traits)
{
        static bool s_bNoAsserts = false;

    wxString msg;
    msg.reserve(2048);

                msg.Printf(wxT("%s(%d): assert \"%s\" failed"), file, line, cond);

        if ( !func.empty() )
        msg << wxT(" in ") << func << wxT("()");

        if ( !msgUser.empty() )
    {
        msg << wxT(": ") << msgUser;
    }
    else     {
        msg << wxT('.');
    }

#if wxUSE_THREADS
    if ( !wxThread::IsMain() )
    {
        msg += wxString::Format(" [in thread %lx]", wxThread::GetCurrentId());
    }
#endif 
        wxMessageOutputDebug().Output(msg);

    if ( !s_bNoAsserts )
    {
        if ( traits )
        {
                        s_bNoAsserts = traits->ShowAssertDialog(msg);
        }
        else         {
                        s_bNoAsserts = DoShowAssertDialog(msg);
        }
    }
}

#endif 