


#include "wx/wxprec.h"

#if defined(__BORLANDC__)
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/window.h"
    #include "wx/bitmap.h"
    #include "wx/log.h"
    #include "wx/msgdlg.h"
    #include "wx/confbase.h"
    #include "wx/utils.h"
    #include "wx/wxcrtvararg.h"
#endif

#include "wx/apptrait.h"
#include "wx/cmdline.h"
#include "wx/msgout.h"
#include "wx/thread.h"
#include "wx/vidmode.h"
#include "wx/evtloop.h"

#if wxUSE_FONTMAP
    #include "wx/fontmap.h"
#endif 
#include "wx/build.h"
WX_CHECK_BUILD_OPTIONS("wxCore")



wxAppBase::wxAppBase()
{
    m_topWindow = NULL;

    m_useBestVisual = false;
    m_forceTrueColour = false;

    m_isActive = true;

                                                    m_exitOnFrameDelete = Later;
}

bool wxAppBase::Initialize(int& argcOrig, wxChar **argvOrig)
{
#ifdef __WXOSX__
                                    if ( argcOrig > 1 )
    {
        static const wxChar *ARG_PSN = wxT("-psn_");
        if ( wxStrncmp(argvOrig[1], ARG_PSN, wxStrlen(ARG_PSN)) == 0 )
        {
                        --argcOrig;
            memmove(argvOrig + 1, argvOrig + 2, argcOrig * sizeof(wxChar*));
        }
    }
#endif 
    if ( !wxAppConsole::Initialize(argcOrig, argvOrig) )
        return false;

    wxInitializeStockLists();

    wxBitmap::InitStandardHandlers();

        if ( !OnInitGui() )
        return false;

    return true;
}


wxAppBase::~wxAppBase()
{
    }

void wxAppBase::CleanUp()
{
                    DeletePendingObjects();

            while ( !wxTopLevelWindows.empty() )
    {
                        delete wxTopLevelWindows.GetFirst()->GetData();
    }

        wxBitmap::CleanUpHandlers();

    wxStockGDI::DeleteAll();

    wxDeleteStockLists();

    wxDELETE(wxTheColourDatabase);

    wxAppConsole::CleanUp();
}


wxWindow* wxAppBase::GetTopWindow() const
{
    wxWindow* window = m_topWindow;
    if (window == NULL && wxTopLevelWindows.GetCount() > 0)
        window = wxTopLevelWindows.GetFirst()->GetData();
    return window;
}

wxVideoMode wxAppBase::GetDisplayMode() const
{
    return wxVideoMode();
}

wxLayoutDirection wxAppBase::GetLayoutDirection() const
{
#if wxUSE_INTL
    const wxLocale *const locale = wxGetLocale();
    if ( locale )
    {
        const wxLanguageInfo *const
            info = wxLocale::GetLanguageInfo(locale->GetLanguage());

        if ( info )
            return info->LayoutDirection;
    }
#endif 
        return wxLayout_Default;
}

#if wxUSE_CMDLINE_PARSER


#ifdef __WXUNIVERSAL__
#define OPTION_THEME   "theme"
#endif
#if defined(__WXDFB__)
#define OPTION_MODE    "mode"
#endif

void wxAppBase::OnInitCmdLine(wxCmdLineParser& parser)
{
        wxAppConsole::OnInitCmdLine(parser);

        static const wxCmdLineEntryDesc cmdLineGUIDesc[] =
    {
#ifdef __WXUNIVERSAL__
        {
            wxCMD_LINE_OPTION,
            NULL,
            OPTION_THEME,
            gettext_noop("specify the theme to use"),
            wxCMD_LINE_VAL_STRING,
            0x0
        },
#endif 
#if defined(__WXDFB__)
                                {
            wxCMD_LINE_OPTION,
            NULL,
            OPTION_MODE,
            gettext_noop("specify display mode to use (e.g. 640x480-16)"),
            wxCMD_LINE_VAL_STRING,
            0x0
        },
#endif 
                wxCMD_LINE_DESC_END
    };

    parser.SetDesc(cmdLineGUIDesc);
}

bool wxAppBase::OnCmdLineParsed(wxCmdLineParser& parser)
{
#ifdef __WXUNIVERSAL__
    wxString themeName;
    if ( parser.Found(OPTION_THEME, &themeName) )
    {
        wxTheme *theme = wxTheme::Create(themeName);
        if ( !theme )
        {
            wxLogError(_("Unsupported theme '%s'."), themeName.c_str());
            return false;
        }

                delete wxTheme::Get();
        wxTheme::Set(theme);
    }
#endif 
#if defined(__WXDFB__)
    wxString modeDesc;
    if ( parser.Found(OPTION_MODE, &modeDesc) )
    {
        unsigned w, h, bpp;
        if ( wxSscanf(modeDesc.c_str(), wxT("%ux%u-%u"), &w, &h, &bpp) != 3 )
        {
            wxLogError(_("Invalid display mode specification '%s'."), modeDesc.c_str());
            return false;
        }

        if ( !SetDisplayMode(wxVideoMode(w, h, bpp)) )
            return false;
    }
#endif 
    return wxAppConsole::OnCmdLineParsed(parser);
}

#endif 

bool wxAppBase::OnInitGui()
{
#ifdef __WXUNIVERSAL__
    if ( !wxTheme::Get() && !wxTheme::CreateDefault() )
        return false;
#endif 
    return true;
}

int wxAppBase::OnRun()
{
            if ( m_exitOnFrameDelete == Later )
    {
        m_exitOnFrameDelete = Yes;
    }
    
    return wxAppConsole::OnRun();
}

int wxAppBase::OnExit()
{
#ifdef __WXUNIVERSAL__
    delete wxTheme::Set(NULL);
#endif 
    return wxAppConsole::OnExit();
}

wxAppTraits *wxAppBase::CreateTraits()
{
    return new wxGUIAppTraits;
}


void wxAppBase::SetActive(bool active, wxWindow * WXUNUSED(lastFocus))
{
    if ( active == m_isActive )
        return;

    m_isActive = active;

    wxActivateEvent event(wxEVT_ACTIVATE_APP, active);
    event.SetEventObject(this);

    (void)ProcessEvent(event);
}

bool wxAppBase::SafeYield(wxWindow *win, bool onlyIfNeeded)
{
    wxWindowDisabler wd(win);

    wxEventLoopBase * const loop = wxEventLoopBase::GetActive();

    return loop && loop->Yield(onlyIfNeeded);
}

bool wxAppBase::SafeYieldFor(wxWindow *win, long eventsToProcess)
{
    wxWindowDisabler wd(win);

    wxEventLoopBase * const loop = wxEventLoopBase::GetActive();

    return loop && loop->YieldFor(eventsToProcess);
}



bool wxAppBase::ProcessIdle()
{
            bool needMore = wxAppConsoleBase::ProcessIdle();
    wxIdleEvent event;
    wxWindowList::compatibility_iterator node = wxTopLevelWindows.GetFirst();
    while (node)
    {
        wxWindow* win = node->GetData();

                        if ( !wxPendingDelete.Member(win) && win->SendIdleEvents(event) )
            needMore = true;
        node = node->GetNext();
    }

    wxUpdateUIEvent::ResetUpdateTime();

    return needMore;
}


#if wxUSE_LOG

wxLog *wxGUIAppTraitsBase::CreateLogTarget()
{
#if wxUSE_LOGGUI
#ifndef __WXOSX_IPHONE__
    return new wxLogGui;
#else
    return new wxLogStderr;
#endif
#else
        return new wxLogStderr;
#endif
}

#endif 
wxMessageOutput *wxGUIAppTraitsBase::CreateMessageOutput()
{
                        #ifdef __UNIX__
    return new wxMessageOutputStderr;
#else         #ifdef __WXMOTIF__
        return new wxMessageOutputLog;
    #elif wxUSE_MSGDLG
        return new wxMessageOutputBest(wxMSGOUT_PREFER_STDERR);
    #else
        return new wxMessageOutputStderr;
    #endif
#endif }

#if wxUSE_FONTMAP

wxFontMapper *wxGUIAppTraitsBase::CreateFontMapper()
{
    return new wxFontMapper;
}

#endif 
wxRendererNative *wxGUIAppTraitsBase::CreateRenderer()
{
        return NULL;
}

bool wxGUIAppTraitsBase::ShowAssertDialog(const wxString& msg)
{
#if wxDEBUG_LEVEL
                                #if !defined(__WXMSW__) && !defined(__WXDFB__) && wxUSE_MSGDLG

            if ( wxIsMainThread() )
    {
        wxString msgDlg = msg;

#if wxUSE_STACKWALKER
        const wxString stackTrace = GetAssertStackTrace();
        if ( !stackTrace.empty() )
            msgDlg << wxT("\n\nCall stack:\n") << stackTrace;
#endif 
                        msgDlg += wxT("\nDo you want to stop the program?\n")
                  wxT("You can also choose [Cancel] to suppress ")
                  wxT("further warnings.");

        switch ( wxMessageBox(msgDlg, wxT("wxWidgets Debug Alert"),
                              wxYES_NO | wxCANCEL | wxICON_STOP ) )
        {
            case wxYES:
                wxTrap();
                break;

            case wxCANCEL:
                                return true;

                    }

        return false;
    }
#endif #endif 
    return wxAppTraitsBase::ShowAssertDialog(msg);
}

bool wxGUIAppTraitsBase::HasStderr()
{
        #ifdef __UNIX__
    return true;
#else
    return false;
#endif
}

