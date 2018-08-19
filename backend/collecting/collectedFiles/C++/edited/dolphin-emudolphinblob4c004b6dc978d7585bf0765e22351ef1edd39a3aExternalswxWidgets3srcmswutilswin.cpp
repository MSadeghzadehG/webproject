
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/utils.h"
#endif 
#include "wx/private/launchbrowser.h"
#include "wx/msw/private.h"     #include "wx/msw/registry.h"
#include <shellapi.h> 

bool wxLaunchDefaultApplication(const wxString& document, int flags)
{
    wxUnusedVar(flags);

    WinStruct<SHELLEXECUTEINFO> sei;
    sei.lpFile = document.t_str();
    sei.nShow = SW_SHOWDEFAULT;

                sei.fMask = SEE_MASK_FLAG_NO_UI;

    if ( ::ShellExecuteEx(&sei) )
        return true;

    return false;
}



bool wxDoLaunchDefaultBrowser(const wxLaunchBrowserParams& params)
{
#if wxUSE_IPC
    if ( params.flags & wxBROWSER_NEW_WINDOW )
    {
                        wxRegKey key(wxRegKey::HKCR, params.scheme + wxT("\\shell\\open"));
        if ( !key.Exists() )
        {
                        key.SetName(wxRegKey::HKCR, wxT("http\\shell\\open"));
        }

        if ( key.Exists() )
        {
            wxRegKey keyDDE(key, wxT("DDEExec"));
            if ( keyDDE.Exists() )
            {
                                                                static const wxChar *TOPIC_OPEN_URL = wxT("WWW_OpenURL");
                wxString ddeCmd;
                wxRegKey keyTopic(keyDDE, wxT("topic"));
                bool ok = keyTopic.Exists() &&
                            keyTopic.QueryDefaultValue() == TOPIC_OPEN_URL;
                if ( ok )
                {
                    ddeCmd = keyDDE.QueryDefaultValue();
                    ok = !ddeCmd.empty();
                }

                if ( ok )
                {
                                                                                                                        ddeCmd.Replace(wxT("-1"), wxT("0"),
                                   false );

                                                                                                    ok = ddeCmd.Replace(wxT("%1"), params.url, false) == 1;
                }

                if ( ok )
                {
                                        wxLogNull noLog;

                    const wxString ddeServer = wxRegKey(keyDDE, wxT("application"));
                    if ( wxExecuteDDE(ddeServer, TOPIC_OPEN_URL, ddeCmd) )
                        return true;

                                                                                                                    }
            }
        }
    }
#endif 
    WinStruct<SHELLEXECUTEINFO> sei;
    sei.lpFile = params.GetPathOrURL().t_str();
    sei.lpVerb = wxT("open");
    sei.nShow = SW_SHOWNORMAL;
    sei.fMask = SEE_MASK_FLAG_NO_UI; 
    if ( ::ShellExecuteEx(&sei) )
        return true;

    return false;
}
