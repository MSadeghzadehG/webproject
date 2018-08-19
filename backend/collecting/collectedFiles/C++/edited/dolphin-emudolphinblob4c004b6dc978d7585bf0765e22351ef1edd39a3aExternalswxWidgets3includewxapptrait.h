
#ifndef _WX_APPTRAIT_H_
#define _WX_APPTRAIT_H_

#include "wx/string.h"
#include "wx/platinfo.h"

class WXDLLIMPEXP_FWD_BASE wxArrayString;
class WXDLLIMPEXP_FWD_BASE wxConfigBase;
class WXDLLIMPEXP_FWD_BASE wxEventLoopBase;
#if wxUSE_FONTMAP
    class WXDLLIMPEXP_FWD_CORE wxFontMapper;
#endif class WXDLLIMPEXP_FWD_BASE wxLog;
class WXDLLIMPEXP_FWD_BASE wxMessageOutput;
class WXDLLIMPEXP_FWD_BASE wxObject;
class WXDLLIMPEXP_FWD_CORE wxRendererNative;
class WXDLLIMPEXP_FWD_BASE wxStandardPaths;
class WXDLLIMPEXP_FWD_BASE wxString;
class WXDLLIMPEXP_FWD_BASE wxTimer;
class WXDLLIMPEXP_FWD_BASE wxTimerImpl;

class wxSocketManager;



class WXDLLIMPEXP_BASE wxAppTraitsBase
{
public:
        virtual ~wxAppTraitsBase() { }

        
#if wxUSE_CONFIG
                virtual wxConfigBase *CreateConfig();
#endif 
#if wxUSE_LOG
        virtual wxLog *CreateLogTarget() = 0;
#endif 
        virtual wxMessageOutput *CreateMessageOutput() = 0;

#if wxUSE_FONTMAP
        virtual wxFontMapper *CreateFontMapper() = 0;
#endif 
                        virtual wxRendererNative *CreateRenderer() = 0;

        virtual wxStandardPaths& GetStandardPaths();


        
                                virtual bool ShowAssertDialog(const wxString& msg) = 0;

        virtual bool HasStderr() = 0;

#if wxUSE_SOCKETS
                                static void SetDefaultSocketManager(wxSocketManager *manager)
    {
        ms_manager = manager;
    }

            virtual wxSocketManager *GetSocketManager() { return ms_manager; }
#endif

        virtual wxEventLoopBase *CreateEventLoop() = 0;

#if wxUSE_TIMER
        virtual wxTimerImpl *CreateTimerImpl(wxTimer *timer) = 0;
#endif

#if wxUSE_THREADS
    virtual void MutexGuiEnter();
    virtual void MutexGuiLeave();
#endif

        
                    virtual wxPortId GetToolkitVersion(int *majVer = NULL,
                                       int *minVer = NULL,
                                       int *microVer = NULL) const = 0;

        virtual bool IsUsingUniversalWidgets() const = 0;

            virtual wxString GetDesktopEnvironment() const = 0;

                    virtual wxString GetStandardCmdLineOptions(wxArrayString& names,
                                               wxArrayString& desc) const
    {
        wxUnusedVar(names);
        wxUnusedVar(desc);

        return wxEmptyString;
    }


protected:
#if wxUSE_STACKWALKER
        virtual wxString GetAssertStackTrace();
#endif

private:
    static wxSocketManager *ms_manager;
};


#if defined(__WIN32__)
    #include "wx/msw/apptbase.h"
#elif defined(__UNIX__)
    #include "wx/unix/apptbase.h"
#else         class WXDLLIMPEXP_BASE wxAppTraits : public wxAppTraitsBase
    {
    };
#endif 


class WXDLLIMPEXP_BASE wxConsoleAppTraitsBase : public wxAppTraits
{
public:
#if !wxUSE_CONSOLE_EVENTLOOP
    virtual wxEventLoopBase *CreateEventLoop() { return NULL; }
#endif 
#if wxUSE_LOG
    virtual wxLog *CreateLogTarget() wxOVERRIDE;
#endif     virtual wxMessageOutput *CreateMessageOutput() wxOVERRIDE;
#if wxUSE_FONTMAP
    virtual wxFontMapper *CreateFontMapper() wxOVERRIDE;
#endif     virtual wxRendererNative *CreateRenderer() wxOVERRIDE;

    virtual bool ShowAssertDialog(const wxString& msg) wxOVERRIDE;
    virtual bool HasStderr() wxOVERRIDE;

        wxPortId GetToolkitVersion(int *verMaj = NULL,
                               int *verMin = NULL,
                               int *verMicro = NULL) const wxOVERRIDE
    {
                                if (verMaj) *verMaj = 0;
        if (verMin) *verMin = 0;
        if (verMicro) *verMicro = 0;
        return wxPORT_BASE;
    }

    virtual bool IsUsingUniversalWidgets() const wxOVERRIDE { return false; }
    virtual wxString GetDesktopEnvironment() const wxOVERRIDE { return wxEmptyString; }
};


#if wxUSE_GUI

class WXDLLIMPEXP_CORE wxGUIAppTraitsBase : public wxAppTraits
{
public:
#if wxUSE_LOG
    virtual wxLog *CreateLogTarget() wxOVERRIDE;
#endif     virtual wxMessageOutput *CreateMessageOutput() wxOVERRIDE;
#if wxUSE_FONTMAP
    virtual wxFontMapper *CreateFontMapper() wxOVERRIDE;
#endif     virtual wxRendererNative *CreateRenderer() wxOVERRIDE;

    virtual bool ShowAssertDialog(const wxString& msg) wxOVERRIDE;
    virtual bool HasStderr() wxOVERRIDE;

    virtual bool IsUsingUniversalWidgets() const wxOVERRIDE
    {
    #ifdef __WXUNIVERSAL__
        return true;
    #else
        return false;
    #endif
    }

    virtual wxString GetDesktopEnvironment() const wxOVERRIDE { return wxEmptyString; }
};

#endif 

#if defined(__WIN32__)
    #include "wx/msw/apptrait.h"
#elif defined(__UNIX__)
    #include "wx/unix/apptrait.h"
#else
    #if wxUSE_GUI
        class wxGUIAppTraits : public wxGUIAppTraitsBase
        {
        };
    #endif     class wxConsoleAppTraits: public wxConsoleAppTraitsBase
    {
    };
#endif 
#endif 
