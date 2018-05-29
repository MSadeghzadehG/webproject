
#ifndef _WX_APP_H_BASE_
#define _WX_APP_H_BASE_


#include "wx/event.h"       #include "wx/eventfilter.h" #include "wx/build.h"
#include "wx/cmdargs.h"     #include "wx/init.h"        #include "wx/intl.h"        #include "wx/log.h"         
class WXDLLIMPEXP_FWD_BASE wxAppConsole;
class WXDLLIMPEXP_FWD_BASE wxAppTraits;
class WXDLLIMPEXP_FWD_BASE wxCmdLineParser;
class WXDLLIMPEXP_FWD_BASE wxEventLoopBase;
class WXDLLIMPEXP_FWD_BASE wxMessageOutput;

#if wxUSE_GUI
    struct WXDLLIMPEXP_FWD_CORE wxVideoMode;
    class WXDLLIMPEXP_FWD_CORE wxWindow;
#endif

#define wxDISABLE_DEBUG_SUPPORT() \
    wxDISABLE_ASSERTS_IN_RELEASE_BUILD(); \
    wxDISABLE_DEBUG_LOGGING_IN_RELEASE_BUILD()


typedef wxAppConsole* (*wxAppInitializerFunction)();


enum
{
    wxPRINT_WINDOWS = 1,
    wxPRINT_POSTSCRIPT = 2
};


extern WXDLLIMPEXP_DATA_BASE(wxList) wxPendingDelete;


class WXDLLIMPEXP_BASE wxAppConsoleBase : public wxEvtHandler,
                                          public wxEventFilter
{
public:
        wxAppConsoleBase();
    virtual ~wxAppConsoleBase();


        
                        virtual bool Initialize(int& argc, wxChar **argv);

        virtual bool CallOnInit() { return OnInit(); }

                    virtual bool OnInit();

            virtual int OnRun();

        virtual void OnLaunched();
    
                        virtual void OnEventLoopEnter(wxEventLoopBase* WXUNUSED(loop)) {}

            virtual int OnExit();

            virtual void OnEventLoopExit(wxEventLoopBase* WXUNUSED(loop)) {}

                virtual void CleanUp();

                            virtual void OnFatalException() { }

        virtual void Exit();


        
            
            wxString GetAppName() const;
    void SetAppName(const wxString& name) { m_appName = name; }

                                                            wxString GetAppDisplayName() const;

    void SetAppDisplayName(const wxString& name) { m_appDisplayName = name; }

            wxString GetClassName() const { return m_className; }
    void SetClassName(const wxString& name) { m_className = name; }

            const wxString& GetVendorName() const { return m_vendorName; }
    void SetVendorName(const wxString& name) { m_vendorName = name; }

                            const wxString& GetVendorDisplayName() const
    {
        return m_vendorDisplayName.empty() ? GetVendorName()
                                           : m_vendorDisplayName;
    }
    void SetVendorDisplayName(const wxString& name)
    {
        m_vendorDisplayName = name;
    }


        
                        
#if wxUSE_CMDLINE_PARSER
                virtual void OnInitCmdLine(wxCmdLineParser& parser);

                virtual bool OnCmdLineParsed(wxCmdLineParser& parser);

            virtual bool OnCmdLineHelp(wxCmdLineParser& parser);

            virtual bool OnCmdLineError(wxCmdLineParser& parser);
#endif 

        
                        wxAppTraits *GetTraits();

                            static wxAppTraits *GetTraitsIfExists();

                            static wxAppTraits& GetValidTraits();

                        wxEventLoopBase* GetMainLoop() const
        { return m_mainLoop; }

                                            virtual void SetCLocale();


        
            virtual int FilterEvent(wxEvent& event) wxOVERRIDE;

            static bool IsMainLoopRunning();

#if wxUSE_EXCEPTIONS
                            virtual void CallEventHandler(wxEvtHandler *handler,
                                  wxEventFunctor& functor,
                                  wxEvent& event) const;

                        virtual void HandleEvent(wxEvtHandler *handler,
                             wxEventFunction func,
                             wxEvent& event) const;

                    virtual void OnUnhandledException();

                            virtual bool OnExceptionInMainLoop();

                                                    virtual bool StoreCurrentException();

                            virtual void RethrowStoredException();
#endif 

        
                    
                    virtual void ProcessPendingEvents();

        bool HasPendingEvents() const;

        void SuspendProcessingOfPendingEvents();

            void ResumeProcessingOfPendingEvents();

            void RemovePendingEventHandler(wxEvtHandler* toRemove);

        void AppendPendingEventHandler(wxEvtHandler* toAppend);

            void DelayPendingEventHandler(wxEvtHandler* toDelay);

        void DeletePendingEvents();


        
                                
        void ScheduleForDestruction(wxObject *object);

        bool IsScheduledForDestruction(wxObject *object) const;


        
            virtual bool Pending();
    virtual bool Dispatch();

    virtual int MainLoop();
    virtual void ExitMainLoop();

    bool Yield(bool onlyIfNeeded = false);

    virtual void WakeUpIdle();

                            virtual bool ProcessIdle();

                    virtual bool UsesEventLoop() const;


        
                                virtual void OnAssertFailure(const wxChar *file,
                                 int line,
                                 const wxChar *func,
                                 const wxChar *cond,
                                 const wxChar *msg);

            virtual void OnAssert(const wxChar *file,
                          int line,
                          const wxChar *cond,
                          const wxChar *msg);

                static bool CheckBuildOptions(const char *optionsSignature,
                                  const char *componentName);

        
        static void SetInitializerFunction(wxAppInitializerFunction fn)
        { ms_appInitFn = fn; }
    static wxAppInitializerFunction GetInitializerFunction()
        { return ms_appInitFn; }

                    static wxAppConsole *GetInstance() { return ms_appInstance; }
    static void SetInstance(wxAppConsole *app) { ms_appInstance = app; }


        int argc;

                #if wxUSE_UNICODE
    wxCmdLineArgsArray argv;
#else
    char **argv;
#endif

protected:
                void DeletePendingObjects();

            virtual wxAppTraits *CreateTraits();

        static wxAppInitializerFunction ms_appInitFn;

        static wxAppConsole *ms_appInstance;

            wxEventLoopBase *CreateMainLoop();

        wxString m_vendorName,                     m_vendorDisplayName,              m_appName,                        m_appDisplayName,                 m_className;         
            wxAppTraits *m_traits;

            wxEventLoopBase *m_mainLoop;


    
            wxEvtHandlerArray m_handlersWithPendingEvents;

                    wxEvtHandlerArray m_handlersWithPendingDelayedEvents;

#if wxUSE_THREADS
        wxCriticalSection m_handlersWithPendingEventsLocker;
#endif

        bool m_bDoPendingEventProcessing;

    friend class WXDLLIMPEXP_FWD_BASE wxEvtHandler;

            wxDECLARE_NO_COPY_CLASS(wxAppConsoleBase);
};

#if defined(__UNIX__) && !defined(__WINDOWS__)
    #include "wx/unix/app.h"
#else
        class wxAppConsole : public wxAppConsoleBase { };
#endif


#if wxUSE_GUI

class WXDLLIMPEXP_CORE wxAppBase : public wxAppConsole
{
public:
    wxAppBase();
    virtual ~wxAppBase();

        
                            virtual bool Initialize(int& argc, wxChar **argv) wxOVERRIDE;

                                    virtual bool OnInitGui();

                                                            virtual int OnRun() wxOVERRIDE;

            virtual int OnExit() wxOVERRIDE;

                            virtual void CleanUp() wxOVERRIDE;


        
            virtual bool SafeYield(wxWindow *win, bool onlyIfNeeded);
    virtual bool SafeYieldFor(wxWindow *win, long eventsToProcess);

                                            virtual bool ProcessIdle() wxOVERRIDE;

            virtual bool UsesEventLoop() const wxOVERRIDE { return true; }


        
            virtual bool IsActive() const { return m_isActive; }

            void SetTopWindow(wxWindow *win) { m_topWindow = win; }

                            virtual wxWindow *GetTopWindow() const;

                                            void SetExitOnFrameDelete(bool flag)
        { m_exitOnFrameDelete = flag ? Yes : No; }
    bool GetExitOnFrameDelete() const
        { return m_exitOnFrameDelete == Yes; }


        
                    virtual wxVideoMode GetDisplayMode() const;
                            virtual bool SetDisplayMode(const wxVideoMode& WXUNUSED(info)) { return true; }

            void SetUseBestVisual( bool flag, bool forceTrueColour = false )
        { m_useBestVisual = flag; m_forceTrueColour = forceTrueColour; }
    bool GetUseBestVisual() const { return m_useBestVisual; }

                                    virtual void SetPrintMode(int WXUNUSED(mode)) { }
    int GetPrintMode() const { return wxPRINT_POSTSCRIPT; }

            virtual wxLayoutDirection GetLayoutDirection() const;

        virtual bool SetNativeTheme(const wxString& WXUNUSED(theme)) { return false; }


        
#if wxUSE_CMDLINE_PARSER
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser) wxOVERRIDE;
    virtual void OnInitCmdLine(wxCmdLineParser& parser) wxOVERRIDE;
#endif

        
                virtual void SetActive(bool isActive, wxWindow *lastFocus);

protected:
        virtual wxAppTraits *CreateTraits() wxOVERRIDE;


        wxWindow *m_topWindow;

                    enum
    {
        Later = -1,
        No,
        Yes
    } m_exitOnFrameDelete;

            bool m_useBestVisual;
        bool m_forceTrueColour;

        bool m_isActive;

    wxDECLARE_NO_COPY_CLASS(wxAppBase);
};


#if defined(__WXMSW__)
    #include "wx/msw/app.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/app.h"
#elif defined(__WXDFB__)
    #include "wx/dfb/app.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/app.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/app.h"
#elif defined(__WXX11__)
    #include "wx/x11/app.h"
#elif defined(__WXMAC__)
    #include "wx/osx/app.h"
#elif defined(__WXQT__)
    #include "wx/qt/app.h"
#endif

#else 
#define wxApp wxAppConsole

#endif 

#define wxTheApp static_cast<wxApp*>(wxApp::GetInstance())



WXDLLIMPEXP_BASE void wxExit();

#ifndef wx_YIELD_DECLARED
#define wx_YIELD_DECLARED

WXDLLIMPEXP_CORE bool wxYield();

#endif 
WXDLLIMPEXP_BASE void wxWakeUpIdle();



class WXDLLIMPEXP_BASE wxAppInitializer
{
public:
    wxAppInitializer(wxAppInitializerFunction fn)
        { wxApp::SetInitializerFunction(fn); }
};


#if wxUSE_UNICODE && defined(__VISUALC__)
    #define wxIMPLEMENT_WXWIN_MAIN_CONSOLE                                    \
        int wmain(int argc, wchar_t **argv)                                   \
        {                                                                     \
            wxDISABLE_DEBUG_SUPPORT();                                        \
                                                                              \
            return wxEntry(argc, argv);                                       \
        }
#else     #define wxIMPLEMENT_WXWIN_MAIN_CONSOLE                                    \
        int main(int argc, char **argv)                                       \
        {                                                                     \
            wxDISABLE_DEBUG_SUPPORT();                                        \
                                                                              \
            return wxEntry(argc, argv);                                       \
        }
#endif

#ifndef wxIMPLEMENT_WXWIN_MAIN
    #define wxIMPLEMENT_WXWIN_MAIN          wxIMPLEMENT_WXWIN_MAIN_CONSOLE
#endif 
#ifdef __WXUNIVERSAL__
    #include "wx/univ/theme.h"

    #ifdef wxUNIV_DEFAULT_THEME
        #define wxIMPLEMENT_WX_THEME_SUPPORT \
            WX_USE_THEME(wxUNIV_DEFAULT_THEME);
    #else
        #define wxIMPLEMENT_WX_THEME_SUPPORT
    #endif
#else
    #define wxIMPLEMENT_WX_THEME_SUPPORT
#endif

#define wxIMPLEMENT_APP_NO_MAIN(appname)                                    \
    appname& wxGetApp() { return *static_cast<appname*>(wxApp::GetInstance()); }    \
    wxAppConsole *wxCreateApp()                                             \
    {                                                                       \
        wxAppConsole::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE,         \
                                        "your program");                    \
        return new appname;                                                 \
    }                                                                       \
    wxAppInitializer                                                        \
        wxTheAppInitializer((wxAppInitializerFunction) wxCreateApp)

#define wxIMPLEMENT_APP_NO_THEMES(appname)  \
    wxIMPLEMENT_WXWIN_MAIN                  \
    wxIMPLEMENT_APP_NO_MAIN(appname)

#define wxIMPLEMENT_APP(appname)            \
    wxIMPLEMENT_WX_THEME_SUPPORT            \
    wxIMPLEMENT_APP_NO_THEMES(appname)

#define wxIMPLEMENT_APP_CONSOLE(appname)    \
    wxIMPLEMENT_WXWIN_MAIN_CONSOLE          \
    wxIMPLEMENT_APP_NO_MAIN(appname)

#define wxDECLARE_APP(appname)              \
    extern appname& wxGetApp()


extern wxAppConsole *wxCreateApp();
extern wxAppInitializer wxTheAppInitializer;



#define IMPLEMENT_WXWIN_MAIN_CONSOLE            wxIMPLEMENT_WXWIN_MAIN_CONSOLE
#define IMPLEMENT_WXWIN_MAIN                    wxIMPLEMENT_WXWIN_MAIN
#define IMPLEMENT_WX_THEME_SUPPORT              wxIMPLEMENT_WX_THEME_SUPPORT
#define IMPLEMENT_APP_NO_MAIN(app)              wxIMPLEMENT_APP_NO_MAIN(app);
#define IMPLEMENT_APP_NO_THEMES(app)            wxIMPLEMENT_APP_NO_THEMES(app);
#define IMPLEMENT_APP(app)                      wxIMPLEMENT_APP(app);
#define IMPLEMENT_APP_CONSOLE(app)              wxIMPLEMENT_APP_CONSOLE(app);
#define DECLARE_APP(app)                        wxDECLARE_APP(app);

#endif 