


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_DIALUP_MANAGER

#include "wx/dialup.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/intl.h"
    #include "wx/event.h"
    #include "wx/app.h"
    #include "wx/timer.h"
    #include "wx/module.h"
#endif

#include "wx/generic/choicdgg.h"

#include "wx/msw/private.h"
#include "wx/msw/private/hiddenwin.h"
#include "wx/msw/private/event.h"
#include "wx/dynlib.h"

wxDEFINE_EVENT( wxEVT_DIALUP_CONNECTED, wxDialUpEvent );
wxDEFINE_EVENT( wxEVT_DIALUP_DISCONNECTED, wxDialUpEvent );

#if (!defined(__BORLANDC__) || (__BORLANDC__>=0x550)) && \
    (!defined(__GNUWIN32__) || wxCHECK_W32API_VERSION(0, 5)) && \
    !defined(__WINE__)

#include <ras.h>
#include <raserror.h>

#include <wininet.h>

#ifndef INTERNET_CONNECTION_LAN
#define INTERNET_CONNECTION_LAN 2
#endif
#ifndef INTERNET_CONNECTION_PROXY
#define INTERNET_CONNECTION_PROXY 4
#endif

static const wxChar *
    wxMSWDIALUP_WNDCLASSNAME = wxT("_wxDialUpManager_Internal_Class");
static const wxChar *gs_classForDialUpWindow = NULL;


#define wxWM_RAS_STATUS_CHANGED (WM_USER + 10010)
#define wxWM_RAS_DIALING_PROGRESS (WM_USER + 10011)



#ifndef UNICODE
    typedef DWORD (APIENTRY * RASDIAL)( LPRASDIALEXTENSIONS, LPCSTR, LPRASDIALPARAMSA, DWORD, LPVOID, LPHRASCONN );
    typedef DWORD (APIENTRY * RASENUMCONNECTIONS)( LPRASCONNA, LPDWORD, LPDWORD );
    typedef DWORD (APIENTRY * RASENUMENTRIES)( LPCSTR, LPCSTR, LPRASENTRYNAMEA, LPDWORD, LPDWORD );
    typedef DWORD (APIENTRY * RASGETCONNECTSTATUS)( HRASCONN, LPRASCONNSTATUSA );
    typedef DWORD (APIENTRY * RASGETERRORSTRING)( UINT, LPSTR, DWORD );
    typedef DWORD (APIENTRY * RASHANGUP)( HRASCONN );
    typedef DWORD (APIENTRY * RASGETPROJECTIONINFO)( HRASCONN, RASPROJECTION, LPVOID, LPDWORD );
    typedef DWORD (APIENTRY * RASCREATEPHONEBOOKENTRY)( HWND, LPCSTR );
    typedef DWORD (APIENTRY * RASEDITPHONEBOOKENTRY)( HWND, LPCSTR, LPCSTR );
    typedef DWORD (APIENTRY * RASSETENTRYDIALPARAMS)( LPCSTR, LPRASDIALPARAMSA, BOOL );
    typedef DWORD (APIENTRY * RASGETENTRYDIALPARAMS)( LPCSTR, LPRASDIALPARAMSA, LPBOOL );
    typedef DWORD (APIENTRY * RASENUMDEVICES)( LPRASDEVINFOA, LPDWORD, LPDWORD );
    typedef DWORD (APIENTRY * RASGETCOUNTRYINFO)( LPRASCTRYINFOA, LPDWORD );
    typedef DWORD (APIENTRY * RASGETENTRYPROPERTIES)( LPCSTR, LPCSTR, LPRASENTRYA, LPDWORD, LPBYTE, LPDWORD );
    typedef DWORD (APIENTRY * RASSETENTRYPROPERTIES)( LPCSTR, LPCSTR, LPRASENTRYA, DWORD, LPBYTE, DWORD );
    typedef DWORD (APIENTRY * RASRENAMEENTRY)( LPCSTR, LPCSTR, LPCSTR );
    typedef DWORD (APIENTRY * RASDELETEENTRY)( LPCSTR, LPCSTR );
    typedef DWORD (APIENTRY * RASVALIDATEENTRYNAME)( LPCSTR, LPCSTR );
    typedef DWORD (APIENTRY * RASCONNECTIONNOTIFICATION)( HRASCONN, HANDLE, DWORD );

    static const wxChar gs_funcSuffix = wxT('A');
#else     typedef DWORD (APIENTRY * RASDIAL)( LPRASDIALEXTENSIONS, LPCWSTR, LPRASDIALPARAMSW, DWORD, LPVOID, LPHRASCONN );
    typedef DWORD (APIENTRY * RASENUMCONNECTIONS)( LPRASCONNW, LPDWORD, LPDWORD );
    typedef DWORD (APIENTRY * RASENUMENTRIES)( LPCWSTR, LPCWSTR, LPRASENTRYNAMEW, LPDWORD, LPDWORD );
    typedef DWORD (APIENTRY * RASGETCONNECTSTATUS)( HRASCONN, LPRASCONNSTATUSW );
    typedef DWORD (APIENTRY * RASGETERRORSTRING)( UINT, LPWSTR, DWORD );
    typedef DWORD (APIENTRY * RASHANGUP)( HRASCONN );
    typedef DWORD (APIENTRY * RASGETPROJECTIONINFO)( HRASCONN, RASPROJECTION, LPVOID, LPDWORD );
    typedef DWORD (APIENTRY * RASCREATEPHONEBOOKENTRY)( HWND, LPCWSTR );
    typedef DWORD (APIENTRY * RASEDITPHONEBOOKENTRY)( HWND, LPCWSTR, LPCWSTR );
    typedef DWORD (APIENTRY * RASSETENTRYDIALPARAMS)( LPCWSTR, LPRASDIALPARAMSW, BOOL );
    typedef DWORD (APIENTRY * RASGETENTRYDIALPARAMS)( LPCWSTR, LPRASDIALPARAMSW, LPBOOL );
    typedef DWORD (APIENTRY * RASENUMDEVICES)( LPRASDEVINFOW, LPDWORD, LPDWORD );
    typedef DWORD (APIENTRY * RASGETCOUNTRYINFO)( LPRASCTRYINFOW, LPDWORD );
    typedef DWORD (APIENTRY * RASGETENTRYPROPERTIES)( LPCWSTR, LPCWSTR, LPRASENTRYW, LPDWORD, LPBYTE, LPDWORD );
    typedef DWORD (APIENTRY * RASSETENTRYPROPERTIES)( LPCWSTR, LPCWSTR, LPRASENTRYW, DWORD, LPBYTE, DWORD );
    typedef DWORD (APIENTRY * RASRENAMEENTRY)( LPCWSTR, LPCWSTR, LPCWSTR );
    typedef DWORD (APIENTRY * RASDELETEENTRY)( LPCWSTR, LPCWSTR );
    typedef DWORD (APIENTRY * RASVALIDATEENTRYNAME)( LPCWSTR, LPCWSTR );
    typedef DWORD (APIENTRY * RASCONNECTIONNOTIFICATION)( HRASCONN, HANDLE, DWORD );

    static const wxChar gs_funcSuffix = wxT('W');
#endif 
struct WXDLLEXPORT wxRasThreadData
{
    wxRasThreadData()
    {
        hWnd = 0;
        dialUpManager = NULL;
    }

    ~wxRasThreadData()
    {
        if ( hWnd )
            DestroyWindow(hWnd);
    }

    HWND    hWnd;           wxWinAPI::Event hEventRas,                      hEventQuit; 
    class WXDLLIMPEXP_FWD_CORE wxDialUpManagerMSW *dialUpManager;  };


class WXDLLEXPORT wxDialUpManagerMSW : public wxDialUpManager
{
public:
        wxDialUpManagerMSW();
    virtual ~wxDialUpManagerMSW();

        virtual bool IsOk() const;
    virtual size_t GetISPNames(wxArrayString& names) const;
    virtual bool Dial(const wxString& nameOfISP,
                      const wxString& username,
                      const wxString& password,
                      bool async);
    virtual bool IsDialing() const;
    virtual bool CancelDialing();
    virtual bool HangUp();
    virtual bool IsAlwaysOnline() const;
    virtual bool IsOnline() const;
    virtual void SetOnlineStatus(bool isOnline = true);
    virtual bool EnableAutoCheckOnlineStatus(size_t nSeconds);
    virtual void DisableAutoCheckOnlineStatus();
    virtual void SetWellKnownHost(const wxString& hostname, int port);
    virtual void SetConnectCommand(const wxString& commandDial,
                                   const wxString& commandHangup);

        void CheckRasStatus();

        void OnConnectStatusChange();
    void OnDialProgress(RASCONNSTATE rasconnstate, DWORD dwError);

        static HWND GetRasWindow() { return ms_hwndRas; }
    static void ResetRasWindow() { ms_hwndRas = NULL; }
    static wxDialUpManagerMSW *GetDialer() { return ms_dialer; }

private:
        static wxString GetErrorString(DWORD error);

        static HRASCONN FindActiveConnection();

        void NotifyApp(bool connected, bool fromOurselves = false) const;

        void CleanUpThreadData();

            int m_autoCheckLevel;

        class WXDLLEXPORT RasTimer : public wxTimer
    {
    public:
        RasTimer(wxDialUpManagerMSW *dialUpManager)
            { m_dialUpManager = dialUpManager; }

        virtual void Notify() { m_dialUpManager->CheckRasStatus(); }

    private:
        wxDialUpManagerMSW *m_dialUpManager;

        wxDECLARE_NO_COPY_CLASS(RasTimer);
    } m_timerStatusPolling;

        HANDLE m_hThread;

            wxRasThreadData *m_data;

        wxDynamicLibrary m_dllRas;

        static HWND ms_hwndRas;

        static HRASCONN ms_hRasConnection;

        static RASDIAL ms_pfnRasDial;
    static RASENUMCONNECTIONS ms_pfnRasEnumConnections;
    static RASENUMENTRIES ms_pfnRasEnumEntries;
    static RASGETCONNECTSTATUS ms_pfnRasGetConnectStatus;
    static RASGETERRORSTRING ms_pfnRasGetErrorString;
    static RASHANGUP ms_pfnRasHangUp;
    static RASGETPROJECTIONINFO ms_pfnRasGetProjectionInfo;
    static RASCREATEPHONEBOOKENTRY ms_pfnRasCreatePhonebookEntry;
    static RASEDITPHONEBOOKENTRY ms_pfnRasEditPhonebookEntry;
    static RASSETENTRYDIALPARAMS ms_pfnRasSetEntryDialParams;
    static RASGETENTRYDIALPARAMS ms_pfnRasGetEntryDialParams;
    static RASENUMDEVICES ms_pfnRasEnumDevices;
    static RASGETCOUNTRYINFO ms_pfnRasGetCountryInfo;
    static RASGETENTRYPROPERTIES ms_pfnRasGetEntryProperties;
    static RASSETENTRYPROPERTIES ms_pfnRasSetEntryProperties;
    static RASRENAMEENTRY ms_pfnRasRenameEntry;
    static RASDELETEENTRY ms_pfnRasDeleteEntry;
    static RASVALIDATEENTRYNAME ms_pfnRasValidateEntryName;

        static RASCONNECTIONNOTIFICATION ms_pfnRasConnectionNotification;

        static int ms_userSpecifiedOnlineStatus;

        static int ms_isConnected;

        static wxDialUpManagerMSW *ms_dialer;

    wxDECLARE_NO_COPY_CLASS(wxDialUpManagerMSW);
};

class wxDialUpManagerModule : public wxModule
{
public:
    bool OnInit() { return true; }
    void OnExit()
    {
        HWND hwnd = wxDialUpManagerMSW::GetRasWindow();
        if ( hwnd )
        {
            ::DestroyWindow(hwnd);
            wxDialUpManagerMSW::ResetRasWindow();
        }

        if ( gs_classForDialUpWindow )
        {
            ::UnregisterClass(wxMSWDIALUP_WNDCLASSNAME, wxGetInstance());
            gs_classForDialUpWindow = NULL;
        }
    }

private:
    wxDECLARE_DYNAMIC_CLASS(wxDialUpManagerModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxDialUpManagerModule, wxModule);


static LRESULT WINAPI wxRasStatusWindowProc(HWND hWnd, UINT message,
                                            WPARAM wParam, LPARAM lParam);

static DWORD wxRasMonitorThread(wxRasThreadData *data);

static void WINAPI wxRasDialFunc(UINT unMsg,
                                 RASCONNSTATE rasconnstate,
                                 DWORD dwError);



HRASCONN wxDialUpManagerMSW::ms_hRasConnection = 0;

HWND wxDialUpManagerMSW::ms_hwndRas = 0;

RASDIAL wxDialUpManagerMSW::ms_pfnRasDial = 0;
RASENUMCONNECTIONS wxDialUpManagerMSW::ms_pfnRasEnumConnections = 0;
RASENUMENTRIES wxDialUpManagerMSW::ms_pfnRasEnumEntries = 0;
RASGETCONNECTSTATUS wxDialUpManagerMSW::ms_pfnRasGetConnectStatus = 0;
RASGETERRORSTRING wxDialUpManagerMSW::ms_pfnRasGetErrorString = 0;
RASHANGUP wxDialUpManagerMSW::ms_pfnRasHangUp = 0;
RASGETPROJECTIONINFO wxDialUpManagerMSW::ms_pfnRasGetProjectionInfo = 0;
RASCREATEPHONEBOOKENTRY wxDialUpManagerMSW::ms_pfnRasCreatePhonebookEntry = 0;
RASEDITPHONEBOOKENTRY wxDialUpManagerMSW::ms_pfnRasEditPhonebookEntry = 0;
RASSETENTRYDIALPARAMS wxDialUpManagerMSW::ms_pfnRasSetEntryDialParams = 0;
RASGETENTRYDIALPARAMS wxDialUpManagerMSW::ms_pfnRasGetEntryDialParams = 0;
RASENUMDEVICES wxDialUpManagerMSW::ms_pfnRasEnumDevices = 0;
RASGETCOUNTRYINFO wxDialUpManagerMSW::ms_pfnRasGetCountryInfo = 0;
RASGETENTRYPROPERTIES wxDialUpManagerMSW::ms_pfnRasGetEntryProperties = 0;
RASSETENTRYPROPERTIES wxDialUpManagerMSW::ms_pfnRasSetEntryProperties = 0;
RASRENAMEENTRY wxDialUpManagerMSW::ms_pfnRasRenameEntry = 0;
RASDELETEENTRY wxDialUpManagerMSW::ms_pfnRasDeleteEntry = 0;
RASVALIDATEENTRYNAME wxDialUpManagerMSW::ms_pfnRasValidateEntryName = 0;
RASCONNECTIONNOTIFICATION wxDialUpManagerMSW::ms_pfnRasConnectionNotification = 0;

int wxDialUpManagerMSW::ms_userSpecifiedOnlineStatus = -1;
int wxDialUpManagerMSW::ms_isConnected = -1;
wxDialUpManagerMSW *wxDialUpManagerMSW::ms_dialer = NULL;


wxDialUpManager *wxDialUpManager::Create()
{
    return new wxDialUpManagerMSW;
}

#ifdef __VISUALC__
        #pragma warning(disable:4355)
#endif 
wxDialUpManagerMSW::wxDialUpManagerMSW()
                  : m_timerStatusPolling(this),
                    m_dllRas(wxT("RASAPI32"))
{
        m_autoCheckLevel = 0;
    m_hThread = 0;
    m_data = new wxRasThreadData;

    if ( !m_dllRas.IsLoaded() )
    {
        wxLogError(_("Dial up functions are unavailable because the remote access service (RAS) is not installed on this machine. Please install it."));
    }
    else if ( !ms_pfnRasDial )
    {
        
                        const char *funcName = NULL;

                #define RESOLVE_RAS_FUNCTION(type, name)                          \
            ms_pfn##name = (type)m_dllRas.GetSymbol( wxString(wxT(#name))  \
                                                     + gs_funcSuffix);    \
            if ( !ms_pfn##name )                                          \
            {                                                             \
                funcName = #name;                                         \
                goto exit;                                                \
            }

                        #define RESOLVE_OPTIONAL_RAS_FUNCTION(type, name)                 \
            ms_pfn##name = (type)m_dllRas.GetSymbol( wxString(wxT(#name))  \
                                                     + gs_funcSuffix);

        RESOLVE_RAS_FUNCTION(RASDIAL, RasDial);
        RESOLVE_RAS_FUNCTION(RASENUMCONNECTIONS, RasEnumConnections);
        RESOLVE_RAS_FUNCTION(RASENUMENTRIES, RasEnumEntries);
        RESOLVE_RAS_FUNCTION(RASGETCONNECTSTATUS, RasGetConnectStatus);
        RESOLVE_RAS_FUNCTION(RASGETERRORSTRING, RasGetErrorString);
        RESOLVE_RAS_FUNCTION(RASHANGUP, RasHangUp);
        RESOLVE_RAS_FUNCTION(RASGETENTRYDIALPARAMS, RasGetEntryDialParams);

                {
            wxLogNull noLog;

            RESOLVE_OPTIONAL_RAS_FUNCTION(RASGETPROJECTIONINFO, RasGetProjectionInfo);
            RESOLVE_OPTIONAL_RAS_FUNCTION(RASCREATEPHONEBOOKENTRY, RasCreatePhonebookEntry);
            RESOLVE_OPTIONAL_RAS_FUNCTION(RASEDITPHONEBOOKENTRY, RasEditPhonebookEntry);
            RESOLVE_OPTIONAL_RAS_FUNCTION(RASSETENTRYDIALPARAMS, RasSetEntryDialParams);
            RESOLVE_OPTIONAL_RAS_FUNCTION(RASGETENTRYPROPERTIES, RasGetEntryProperties);
            RESOLVE_OPTIONAL_RAS_FUNCTION(RASSETENTRYPROPERTIES, RasSetEntryProperties);
            RESOLVE_OPTIONAL_RAS_FUNCTION(RASRENAMEENTRY, RasRenameEntry);
            RESOLVE_OPTIONAL_RAS_FUNCTION(RASDELETEENTRY, RasDeleteEntry);
            RESOLVE_OPTIONAL_RAS_FUNCTION(RASVALIDATEENTRYNAME, RasValidateEntryName);
            RESOLVE_OPTIONAL_RAS_FUNCTION(RASGETCOUNTRYINFO, RasGetCountryInfo);
            RESOLVE_OPTIONAL_RAS_FUNCTION(RASENUMDEVICES, RasEnumDevices);
            RESOLVE_OPTIONAL_RAS_FUNCTION(RASCONNECTIONNOTIFICATION, RasConnectionNotification);
        }

                #undef RESOLVE_RAS_FUNCTION
        #undef RESOLVE_OPTIONAL_RAS_FUNCTION

exit:
        if ( funcName )
        {
            wxLogError(_("The version of remote access service (RAS) installed "
                          "on this machine is too old, please upgrade (the "
                          "following required function is missing: %s)."),
                       funcName);
            m_dllRas.Unload();
            return;
        }
    }

        EnableAutoCheckOnlineStatus(0);
}

wxDialUpManagerMSW::~wxDialUpManagerMSW()
{
    CleanUpThreadData();
}


wxString wxDialUpManagerMSW::GetErrorString(DWORD error)
{
    wxChar buffer[512];     DWORD dwRet = ms_pfnRasGetErrorString(error, buffer, WXSIZEOF(buffer));
    switch ( dwRet )
    {
        case ERROR_INVALID_PARAMETER:
                        return wxString(wxSysErrorMsg(error));

        default:
            {
                wxLogSysError(dwRet,
                      _("Failed to retrieve text of RAS error message"));

                wxString msg;
                msg.Printf(_("unknown error (error code %08x)."), error);
                return msg;
            }

        case 0:
                        buffer[0] = (wxChar)wxTolower(buffer[0]);

            return wxString(buffer);
    }
}

HRASCONN wxDialUpManagerMSW::FindActiveConnection()
{
        DWORD cbBuf = sizeof(RASCONN);
    LPRASCONN lpRasConn = (LPRASCONN)malloc(cbBuf);
    if ( !lpRasConn )
    {
                return 0;
    }

    lpRasConn->dwSize = sizeof(RASCONN);

    DWORD nConnections = 0;
    DWORD dwRet = ERROR_BUFFER_TOO_SMALL;

    while ( dwRet == ERROR_BUFFER_TOO_SMALL )
    {
        dwRet = ms_pfnRasEnumConnections(lpRasConn, &cbBuf, &nConnections);

        if ( dwRet == ERROR_BUFFER_TOO_SMALL )
        {
            LPRASCONN lpRasConnOld = lpRasConn;
            lpRasConn = (LPRASCONN)realloc(lpRasConn, cbBuf);
            if ( !lpRasConn )
            {
                                free(lpRasConnOld);

                return 0;
            }
        }
        else if ( dwRet == 0 )
        {
                        break;
        }
        else
        {
                        wxLogError(_("Cannot find active dialup connection: %s"),
                       GetErrorString(dwRet).c_str());
            return 0;
        }
    }

    HRASCONN hrasconn;

    switch ( nConnections )
    {
        case 0:
                        hrasconn = 0;
            break;

        default:
                                                                        wxLogWarning(_("Several active dialup connections found, choosing one randomly."));
            
        case 1:
                        hrasconn = lpRasConn->hrasconn;
    }

    free(lpRasConn);

    return hrasconn;
}

void wxDialUpManagerMSW::CleanUpThreadData()
{
    if ( m_hThread )
    {
        if ( m_data->hEventQuit.Set() )
        {
            
                                    m_data = NULL;
        }

        CloseHandle(m_hThread);

        m_hThread = 0;
    }

    wxDELETE(m_data);
}


void wxDialUpManagerMSW::CheckRasStatus()
{
        int isConnected = FindActiveConnection() != 0;
    if ( isConnected != ms_isConnected )
    {
        if ( ms_isConnected != -1 )
        {
                        NotifyApp(isConnected != 0);
        }
        
        ms_isConnected = isConnected;
    }
}

void wxDialUpManagerMSW::NotifyApp(bool connected, bool fromOurselves) const
{
    wxDialUpEvent event(connected, fromOurselves);
    (void)wxTheApp->ProcessEvent(event);
}

void wxDialUpManagerMSW::OnConnectStatusChange()
{
            CheckRasStatus();
}

void wxDialUpManagerMSW::OnDialProgress(RASCONNSTATE rasconnstate,
                                        DWORD dwError)
{
    if ( !GetDialer() )
    {
                        return;
    }

        if ( dwError )
    {
        wxLogError(_("Failed to establish dialup connection: %s"),
                   GetErrorString(dwError).c_str());

                if ( ms_hRasConnection )
        {
            ms_pfnRasHangUp(ms_hRasConnection);
            ms_hRasConnection = 0;
        }

        ms_dialer = NULL;

        NotifyApp(false , true );
    }
    else if ( rasconnstate == RASCS_Connected )
    {
        ms_isConnected = true;
        ms_dialer = NULL;

        NotifyApp(true , true );
    }
}


bool wxDialUpManagerMSW::IsOk() const
{
    return m_dllRas.IsLoaded();
}

size_t wxDialUpManagerMSW::GetISPNames(wxArrayString& names) const
{
        DWORD size = sizeof(RASENTRYNAME);
    RASENTRYNAME *rasEntries = (RASENTRYNAME *)malloc(size);
    rasEntries->dwSize = sizeof(RASENTRYNAME);

    DWORD nEntries;
    DWORD dwRet;
    do
    {
        dwRet = ms_pfnRasEnumEntries
                  (
                   NULL,                                   NULL,                                   rasEntries,                             &size,                                  &nEntries                              );

        if ( dwRet == ERROR_BUFFER_TOO_SMALL )
        {
                        void *n  = realloc(rasEntries, size);
            if (n == NULL)
            {
                free(rasEntries);
                return 0;
            }
            rasEntries = (RASENTRYNAME *)n;
        }
        else if ( dwRet != 0 )
        {
                        wxLogError(_("Failed to get ISP names: %s"),
                       GetErrorString(dwRet).c_str());

            free(rasEntries);

            return 0u;
        }
    }
    while ( dwRet != 0 );

        names.Empty();
    for ( size_t n = 0; n < (size_t)nEntries; n++ )
    {
        names.Add(rasEntries[n].szEntryName);
    }

    free(rasEntries);

        return names.GetCount();
}

bool wxDialUpManagerMSW::Dial(const wxString& nameOfISP,
                              const wxString& username,
                              const wxString& password,
                              bool async)
{
        wxCHECK_MSG( IsOk(), false, wxT("using uninitialized wxDialUpManager") );

    if ( ms_hRasConnection )
    {
        wxFAIL_MSG(wxT("there is already an active connection"));

        return true;
    }

        wxString entryName(nameOfISP);
    if ( !entryName )
    {
        wxArrayString names;
        size_t count = GetISPNames(names);
        switch ( count )
        {
            case 0:
                                wxLogError(_("Failed to connect: no ISP to dial."));

                return false;

            case 1:
                                entryName = names[0u];
                break;

            default:
                                {
                    wxString *strings = new wxString[count];
                    for ( size_t i = 0; i < count; i++ )
                    {
                        strings[i] = names[i];
                    }

                    entryName = wxGetSingleChoice
                                (
                                 _("Choose ISP to dial"),
                                 _("Please choose which ISP do you want to connect to"),
                                 count,
                                 strings
                                );

                    delete [] strings;

                    if ( !entryName )
                    {
                                                return false;
                    }
                }
        }
    }

    RASDIALPARAMS rasDialParams;
    rasDialParams.dwSize = sizeof(rasDialParams);
    wxStrlcpy(rasDialParams.szEntryName, entryName.c_str(), RAS_MaxEntryName);

        if ( !username || !password )
    {
        BOOL gotPassword;
        DWORD dwRet = ms_pfnRasGetEntryDialParams
                      (
                       NULL,                                   &rasDialParams,                         &gotPassword                           );

        if ( dwRet != 0 )
        {
            wxLogError(_("Failed to connect: missing username/password."));

            return false;
        }
    }
    else
    {
        wxStrlcpy(rasDialParams.szUserName, username.c_str(), UNLEN);
        wxStrlcpy(rasDialParams.szPassword, password.c_str(), PWLEN);
    }

        rasDialParams.szPhoneNumber[0] = '\0';
    rasDialParams.szCallbackNumber[0] = '\0';

    rasDialParams.szDomain[0] = '*';
    rasDialParams.szDomain[1] = '\0';

        #if 0
    wxString phoneBook;
    if ( wxGetOsVersion() == wxWINDOWS_NT )
    {
                UINT nLen = ::GetSystemDirectory(NULL, 0);
        nLen++;

        if ( !::GetSystemDirectory(phoneBook.GetWriteBuf(nLen), nLen) )
        {
            wxLogSysError(_("Cannot find the location of address book file"));
        }

        phoneBook.UngetWriteBuf();

                phoneBook << "\\ras\\rasphone.pbk";
    }
#endif 
        
    ms_dialer = this;

    DWORD dwRet = ms_pfnRasDial
                  (
                   NULL,                                       NULL,                                       &rasDialParams,
                   0,                                          async ? (void *)wxRasDialFunc                           : 0,                                  &ms_hRasConnection
                  );

    if ( dwRet != 0 )
    {
                if ( async )
        {
            wxLogError(_("Failed to initiate dialup connection: %s"),
                       GetErrorString(dwRet).c_str());
        }
        else
        {
            wxLogError(_("Failed to establish dialup connection: %s"),
                       GetErrorString(dwRet).c_str());
        }

                if ( ms_hRasConnection )
        {
            ms_pfnRasHangUp(ms_hRasConnection);
            ms_hRasConnection = 0;
        }

        ms_dialer = NULL;

        return false;
    }

        if ( !async )
    {
        ms_isConnected = true;
    }

    return true;
}

bool wxDialUpManagerMSW::IsDialing() const
{
    return GetDialer() != NULL;
}

bool wxDialUpManagerMSW::CancelDialing()
{
    if ( !GetDialer() )
    {
                return false;
    }

    wxASSERT_MSG( ms_hRasConnection, wxT("dialing but no connection?") );

    ms_dialer = NULL;

    return HangUp();
}

bool wxDialUpManagerMSW::HangUp()
{
    wxCHECK_MSG( IsOk(), false, wxT("using uninitialized wxDialUpManager") );

            HRASCONN hRasConn;
    if ( ms_hRasConnection )
    {
        hRasConn = ms_hRasConnection;

        ms_hRasConnection = 0;
    }
    else
    {
        hRasConn = FindActiveConnection();
    }

    if ( !hRasConn )
    {
        wxLogError(_("Cannot hang up - no active dialup connection."));

        return false;
    }

            const DWORD dwRet = ms_pfnRasHangUp(hRasConn);
    if ( dwRet != 0 && dwRet != ERROR_NO_CONNECTION )
    {
        wxLogError(_("Failed to terminate the dialup connection: %s"),
                   GetErrorString(dwRet).c_str());
    }

    ms_isConnected = false;

    return true;
}

bool wxDialUpManagerMSW::IsAlwaysOnline() const
{
        bool isAlwaysOnline = false;

    
            
    wxDynamicLibrary hDll(wxT("WININET"));
    if ( hDll.IsLoaded() )
    {
        typedef BOOL (WINAPI *INTERNETGETCONNECTEDSTATE)(LPDWORD, DWORD);
        INTERNETGETCONNECTEDSTATE pfnInternetGetConnectedState;

        #define RESOLVE_FUNCTION(type, name) \
            pfn##name = (type)hDll.GetSymbol(wxT(#name))

        RESOLVE_FUNCTION(INTERNETGETCONNECTEDSTATE, InternetGetConnectedState);

        if ( pfnInternetGetConnectedState )
        {
            DWORD flags = 0;
            if ( pfnInternetGetConnectedState(&flags, 0 ) )
            {
                                isAlwaysOnline = (flags & (INTERNET_CONNECTION_LAN |
                                           INTERNET_CONNECTION_PROXY)) != 0;
            }
                    }
    }

    return isAlwaysOnline;
}

bool wxDialUpManagerMSW::IsOnline() const
{
    wxCHECK_MSG( IsOk(), false, wxT("using uninitialized wxDialUpManager") );

    if ( IsAlwaysOnline() )
    {
                return true;
    }

    if ( ms_userSpecifiedOnlineStatus != -1 )
    {
                return ms_userSpecifiedOnlineStatus != 0;
    }
    else
    {
                return FindActiveConnection() != 0;
    }
}

void wxDialUpManagerMSW::SetOnlineStatus(bool isOnline)
{
    wxCHECK_RET( IsOk(), wxT("using uninitialized wxDialUpManager") );

    ms_userSpecifiedOnlineStatus = isOnline;
}

bool wxDialUpManagerMSW::EnableAutoCheckOnlineStatus(size_t nSeconds)
{
    wxCHECK_MSG( IsOk(), false, wxT("using uninitialized wxDialUpManager") );

    if ( m_autoCheckLevel++ )
    {
                return true;
    }

    bool ok = ms_pfnRasConnectionNotification != 0;

    if ( ok )
    {
                
                if ( m_hThread != 0 )
        {
            if ( ::ResumeThread(m_hThread) != (DWORD)-1 )
                return true;

                        wxLogLastError(wxT("ResumeThread(RasThread)"));

            ok = false;
        }
    }

        
    if ( ok )
    {
                if ( !m_data->hEventRas.Create
                            (
                             wxWinAPI::Event::AutomaticReset,
                             wxWinAPI::Event::Nonsignaled
                            ) )
        {
            wxLogLastError(wxT("CreateEvent(RasStatus)"));

            ok = false;
        }
    }

    if ( ok )
    {
                                        if ( !m_data->hEventQuit.Create
                             (
                                wxWinAPI::Event::ManualReset,
                                wxWinAPI::Event::Nonsignaled
                             ) )
        {
            wxLogLastError(wxT("CreateEvent(RasThreadQuit)"));

            CleanUpThreadData();

            ok = false;
        }
    }

    if ( ok && !ms_hwndRas )
    {
                        ms_hwndRas = wxCreateHiddenWindow
                     (
                        &gs_classForDialUpWindow,
                        wxMSWDIALUP_WNDCLASSNAME,
                        wxRasStatusWindowProc
                     );
        if ( !ms_hwndRas )
        {
            wxLogLastError(wxT("CreateWindow(RasHiddenWindow)"));

            CleanUpThreadData();

            ok = false;
        }
    }

    m_data->hWnd = ms_hwndRas;

    if ( ok )
    {
                m_data->dialUpManager = this;

        DWORD tid;
        m_hThread = CreateThread
                    (
                     NULL,
                     0,
                     (LPTHREAD_START_ROUTINE)wxRasMonitorThread,
                     (void *)m_data,
                     0,
                     &tid
                    );

        if ( !m_hThread )
        {
            wxLogLastError(wxT("CreateThread(RasStatusThread)"));

            CleanUpThreadData();

            ok = false;
        }
    }

    if ( ok )
    {
                DWORD dwRet = ms_pfnRasConnectionNotification
                      (
                        (HRASCONN)INVALID_HANDLE_VALUE,
                        m_data->hEventRas,
                        3 
                      );

        if ( dwRet != 0 )
        {
            wxLogDebug(wxT("RasConnectionNotification() failed: %s"),
                       GetErrorString(dwRet).c_str());

            CleanUpThreadData();
        }
        else
        {
            return true;
        }
    }

            m_timerStatusPolling.Stop();
    if ( nSeconds == 0 )
    {
                nSeconds = 60;
    }
    m_timerStatusPolling.Start(nSeconds * 1000);

    return true;
}

void wxDialUpManagerMSW::DisableAutoCheckOnlineStatus()
{
    wxCHECK_RET( IsOk(), wxT("using uninitialized wxDialUpManager") );

    if ( --m_autoCheckLevel != 0 )
    {
                return;
    }

    if ( m_hThread )
    {
                if ( SuspendThread(m_hThread) == (DWORD)-1 )
        {
            wxLogLastError(wxT("SuspendThread(RasThread)"));
        }
    }
    else
    {
                m_timerStatusPolling.Stop();
    }
}


void wxDialUpManagerMSW::SetWellKnownHost(const wxString& WXUNUSED(hostname),
                                          int WXUNUSED(port))
{
    wxCHECK_RET( IsOk(), wxT("using uninitialized wxDialUpManager") );

    }

void wxDialUpManagerMSW::SetConnectCommand(const wxString& WXUNUSED(dial),
                                           const wxString& WXUNUSED(hangup))
{
    wxCHECK_RET( IsOk(), wxT("using uninitialized wxDialUpManager") );

    }


static DWORD wxRasMonitorThread(wxRasThreadData *data)
{
    HANDLE handles[2];
    handles[0] = data->hEventRas;
    handles[1] = data->hEventQuit;

    bool cont = true;
    while ( cont )
    {
        DWORD dwRet = ::WaitForMultipleObjects(2, handles, FALSE, INFINITE);

        switch ( dwRet )
        {
            case WAIT_OBJECT_0:
                                SendMessage(data->hWnd, wxWM_RAS_STATUS_CHANGED,
                            0, (LPARAM)data);
                break;

            case WAIT_OBJECT_0 + 1:
                cont = false;
                break;

            default:
                wxFAIL_MSG( wxT("unexpected return of WaitForMultipleObjects()") );
                
            case WAIT_FAILED:
                                                DWORD err = GetLastError();
                wxMessageOutputDebug dbg;
                dbg.Printf
                (
                    wxT("WaitForMultipleObjects(RasMonitor) failed: 0x%08lx (%s)"),
                    err,
                    wxSysErrorMsg(err)
                );

                                                return (DWORD)-1;
        }
    }

            delete data;

    return 0;
}

static LRESULT APIENTRY wxRasStatusWindowProc(HWND hWnd, UINT message,
                                              WPARAM wParam, LPARAM lParam)
{
    switch ( message )
    {
        case wxWM_RAS_STATUS_CHANGED:
            {
                wxRasThreadData *data = (wxRasThreadData *)lParam;
                data->dialUpManager->OnConnectStatusChange();
            }
            break;

        case wxWM_RAS_DIALING_PROGRESS:
            {
                wxDialUpManagerMSW *dialMan = wxDialUpManagerMSW::GetDialer();

                dialMan->OnDialProgress((RASCONNSTATE)wParam, lParam);
            }
            break;

        default:
            return ::DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

static void WINAPI wxRasDialFunc(UINT WXUNUSED(unMsg),
                                 RASCONNSTATE rasconnstate,
                                 DWORD dwError)
{
    wxDialUpManagerMSW *dialUpManager = wxDialUpManagerMSW::GetDialer();

    wxCHECK_RET( dialUpManager, wxT("who started to dial then?") );

    SendMessage(wxDialUpManagerMSW::GetRasWindow(), wxWM_RAS_DIALING_PROGRESS,
                rasconnstate, dwError);
}

#endif 
#endif 