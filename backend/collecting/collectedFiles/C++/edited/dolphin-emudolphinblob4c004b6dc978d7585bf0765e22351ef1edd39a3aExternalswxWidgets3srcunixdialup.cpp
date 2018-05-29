
#include "wx/wxprec.h"

#if wxUSE_DIALUP_MANAGER

#include "wx/dialup.h"

#ifndef  WX_PRECOMP
    #include "wx/string.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/event.h"
    #include "wx/app.h"
    #include "wx/utils.h"
    #include "wx/timer.h"
#endif 
#include "wx/filename.h"
#include "wx/ffile.h"
#include "wx/process.h"
#include "wx/crt.h"

#include <stdlib.h>

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

wxDEFINE_EVENT( wxEVT_DIALUP_CONNECTED, wxDialUpEvent );
wxDEFINE_EVENT( wxEVT_DIALUP_DISCONNECTED, wxDialUpEvent );




class WXDLLEXPORT wxDialUpManagerImpl : public wxDialUpManager
{
public:
   wxDialUpManagerImpl();
   virtual ~wxDialUpManagerImpl();

   
   virtual bool IsOk() const wxOVERRIDE
      { return true; }

   
   virtual bool Dial(const wxString& nameOfISP,
                     const wxString& WXUNUSED(username),
                     const wxString& WXUNUSED(password),
                     bool async) wxOVERRIDE;

      virtual bool HangUp() wxOVERRIDE;

            virtual bool IsOnline() const wxOVERRIDE
      {
         CheckStatus();
         return m_IsOnline == Net_Connected;
      }

      virtual bool IsAlwaysOnline() const wxOVERRIDE;

      virtual bool IsDialing() const wxOVERRIDE
      { return m_DialProcess != NULL; }

         virtual bool CancelDialing() wxOVERRIDE;

   size_t GetISPNames(class wxArrayString &) const wxOVERRIDE
      { return 0; }

               virtual void SetOnlineStatus(bool isOnline = true) wxOVERRIDE
      { m_IsOnline = isOnline ? Net_Connected : Net_No; }

      
                        virtual bool EnableAutoCheckOnlineStatus(size_t nSeconds) wxOVERRIDE;

         virtual void DisableAutoCheckOnlineStatus() wxOVERRIDE;

            virtual void SetWellKnownHost(const wxString& hostname,
                                 int portno = 80) wxOVERRIDE;
   
   virtual void SetConnectCommand(const wxString &command, const wxString &hupcmd) wxOVERRIDE
      { m_ConnectCommand = command; m_HangUpCommand = hupcmd; }


      enum NetConnection
   {
       Net_Unknown = -1,           Net_No,                     Net_Connected           };

      enum NetDeviceType
   {
       NetDevice_None    = 0x0000,         NetDevice_Unknown = 0x0001,         NetDevice_Modem   = 0x0002,         NetDevice_LAN     = 0x0004      };

private:
      NetConnection m_IsOnline;

      NetConnection m_connCard;

      int m_CanUseIfconfig;

      wxString m_IfconfigPath;

      int m_CanUsePing;
      wxString m_PingPath;

      wxString m_BeaconHost;
      int m_BeaconPort;

      wxString m_ConnectCommand;
      wxString m_HangUpCommand;
      wxString m_ISPname;
      class AutoCheckTimer *m_timer;
   friend class AutoCheckTimer;

      class wxDialProcess *m_DialProcess;
      int m_DialPId;
   friend class wxDialProcess;

      void CheckStatus(bool fromAsync = false) const;

      void CheckStatusInternal();

         int CheckProcNet();

         int CheckIfconfig();

      NetConnection CheckConnectAndPing();

      NetConnection CheckPing();

      NetConnection CheckConnect();
};


class AutoCheckTimer : public wxTimer
{
public:
   AutoCheckTimer(wxDialUpManagerImpl *dupman)
   {
       m_dupman = dupman;
   }

   virtual void Notify() wxOVERRIDE
   {
       wxLogTrace(wxT("dialup"), wxT("Checking dial up network status."));

       m_dupman->CheckStatus();
   }

public:
   wxDialUpManagerImpl *m_dupman;
};

class wxDialProcess : public wxProcess
{
public:
   wxDialProcess(wxDialUpManagerImpl *dupman)
      {
         m_DupMan = dupman;
      }
   void Disconnect() { m_DupMan = NULL; }
   virtual void OnTerminate(int WXUNUSED(pid), int WXUNUSED(status)) wxOVERRIDE
      {
         if(m_DupMan)
         {
            m_DupMan->m_DialProcess = NULL;
            m_DupMan->CheckStatus(true);
         }
      }
private:
      wxDialUpManagerImpl *m_DupMan;
};


wxDialUpManagerImpl::wxDialUpManagerImpl()
{
   m_IsOnline =
   m_connCard = Net_Unknown;
   m_DialProcess = NULL;
   m_timer = NULL;
   m_CanUseIfconfig = -1;    m_CanUsePing = -1;    m_BeaconHost = WXDIALUP_MANAGER_DEFAULT_BEACONHOST;
   m_BeaconPort = 80;

#ifdef __SGI__
   m_ConnectCommand = wxT("/usr/etc/ppp");
#elif defined(__LINUX__)
      m_ConnectCommand = wxT("pon");
   m_HangUpCommand = wxT("poff");
#endif

   wxChar * dial = wxGetenv(wxT("WXDIALUP_DIALCMD"));
   wxChar * hup = wxGetenv(wxT("WXDIALUP_HUPCMD"));
   SetConnectCommand(dial ? wxString(dial) : m_ConnectCommand,
                     hup ? wxString(hup) : m_HangUpCommand);
}

wxDialUpManagerImpl::~wxDialUpManagerImpl()
{
   if(m_timer) delete m_timer;
   if(m_DialProcess)
   {
      m_DialProcess->Disconnect();
      m_DialProcess->Detach();
   }
}

bool
wxDialUpManagerImpl::Dial(const wxString &isp,
                          const wxString & WXUNUSED(username),
                          const wxString & WXUNUSED(password),
                          bool async)
{
    if(m_IsOnline == Net_Connected)
        return false;
    m_ISPname = isp;
    wxString cmd;
    if(m_ConnectCommand.Find(wxT("%s")))
        cmd.Printf(m_ConnectCommand,m_ISPname.c_str());
    else
        cmd = m_ConnectCommand;

    if ( async )
    {
        m_DialProcess = new wxDialProcess(this);
        m_DialPId = (int)wxExecute(cmd, false, m_DialProcess);
        if(m_DialPId == 0)
        {
            wxDELETE(m_DialProcess);
            return false;
        }
        else
            return true;
    }
    else
        return wxExecute(cmd,  true) == 0;
}

bool wxDialUpManagerImpl::HangUp()
{
    if(m_IsOnline == Net_No)
        return false;
    if(IsDialing())
    {
        wxLogError(_("Already dialling ISP."));
        return false;
    }
    wxString cmd;
    if(m_HangUpCommand.Find(wxT("%s")))
        cmd.Printf(m_HangUpCommand,m_ISPname.c_str(), m_DialProcess);
    else
        cmd = m_HangUpCommand;
    return wxExecute(cmd,  true) == 0;
}


bool wxDialUpManagerImpl::CancelDialing()
{
   if(! IsDialing())
      return false;
   return kill(m_DialPId, SIGTERM) > 0;
}

bool wxDialUpManagerImpl::EnableAutoCheckOnlineStatus(size_t nSeconds)
{
   DisableAutoCheckOnlineStatus();
   m_timer = new AutoCheckTimer(this);
   bool rc = m_timer->Start(nSeconds*1000);
   if(! rc)
   {
      wxDELETE(m_timer);
   }
   return rc;
}

void wxDialUpManagerImpl::DisableAutoCheckOnlineStatus()
{
   if(m_timer != NULL)
   {
      m_timer->Stop();
      wxDELETE(m_timer);
   }
}


void wxDialUpManagerImpl::SetWellKnownHost(const wxString& hostname, int portno)
{
   if( hostname.empty() )
   {
      m_BeaconHost = WXDIALUP_MANAGER_DEFAULT_BEACONHOST;
      m_BeaconPort = 80;
      return;
   }

      wxString port = hostname.After(wxT(':'));
   if( !port.empty() )
   {
      m_BeaconHost = hostname.Before(wxT(':'));
      m_BeaconPort = wxAtoi(port);
   }
   else
   {
      m_BeaconHost = hostname;
      m_BeaconPort = portno;
   }
}


void wxDialUpManagerImpl::CheckStatus(bool fromAsync) const
{
        
    NetConnection oldIsOnline = m_IsOnline;
    (  (wxDialUpManagerImpl *)this)->CheckStatusInternal();

            if(m_IsOnline != oldIsOnline
            && m_IsOnline != Net_Unknown
            && oldIsOnline != Net_Unknown )
    {
        wxDialUpEvent event(m_IsOnline == Net_Connected, ! fromAsync);
        (void)wxTheApp->ProcessEvent(event);
    }
}



void wxDialUpManagerImpl::CheckStatusInternal()
{
    m_IsOnline = Net_Unknown;

            int netDeviceType = CheckProcNet();
    if ( netDeviceType == NetDevice_Unknown )
    {
                netDeviceType = CheckIfconfig();
    }

    switch ( netDeviceType )
    {
        case NetDevice_None:
                        m_IsOnline = Net_No;
            break;

        case NetDevice_LAN:
                                                            if ( m_connCard == Net_Unknown )
            {
                m_connCard = CheckConnectAndPing();
            }
            m_IsOnline = m_connCard;
            break;

        case NetDevice_Unknown:
                        m_IsOnline = CheckConnectAndPing();
            break;

        case NetDevice_LAN + NetDevice_Modem:
        case NetDevice_Modem:
                        m_IsOnline = Net_Connected;
            break;

        default:
            wxFAIL_MSG(wxT("Unexpected netDeviceType"));
    }
}

bool wxDialUpManagerImpl::IsAlwaysOnline() const
{
    wxDialUpManagerImpl *self = wxConstCast(this, wxDialUpManagerImpl);

    int netDeviceType = self->CheckProcNet();
    if ( netDeviceType == NetDevice_Unknown )
    {
                netDeviceType = self->CheckIfconfig();
    }

    if ( netDeviceType == NetDevice_Unknown )
    {
                self->HangUp();
        return IsOnline();
    }
    else
    {
                return (netDeviceType & NetDevice_LAN) != 0;
    }
}

wxDialUpManagerImpl::NetConnection wxDialUpManagerImpl::CheckConnectAndPing()
{
    NetConnection conn;

        conn = CheckConnect();
    if ( conn == Net_Unknown )
    {
                conn = CheckPing();
    }

    return conn;
}

wxDialUpManagerImpl::NetConnection wxDialUpManagerImpl::CheckConnect()
{
         struct hostent     *hp;
   struct sockaddr_in  serv_addr;

   if((hp = gethostbyname(m_BeaconHost.mb_str())) == NULL)
      return Net_No; 
   serv_addr.sin_family = hp->h_addrtype;
   memcpy(&serv_addr.sin_addr,hp->h_addr, hp->h_length);
   serv_addr.sin_port = htons(m_BeaconPort);

   int sockfd;
   if( ( sockfd = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0)
   {
      return Net_Unknown;     }

   if( connect(sockfd, (struct sockaddr *) &serv_addr,
               sizeof(serv_addr)) >= 0)
   {
      close(sockfd);
      return Net_Connected;    }
   else    {
#ifdef ENETUNREACH
       if(errno == ENETUNREACH)
          return Net_No;        else
#endif
          return Net_Unknown;    }
}


int
wxDialUpManagerImpl::CheckProcNet()
{
        int netDevice = NetDevice_Unknown;

#ifdef __LINUX__
    if (wxFileExists(wxT("/proc/net/route")))
    {
                        FILE *f = fopen("/proc/net/route", "rt");
        if (f != NULL)
        {
                        netDevice = NetDevice_None;

            char output[256];

            while (fgets(output, 256, f) != NULL)
            {
                                if ( strstr(output, "eth")
                        || strstr(output, "wlan")
                        || strstr(output, "ath") )
                {
                    netDevice |= NetDevice_LAN;
                }
                else if (strstr(output,"ppp")                           || strstr(output,"sl")                          || strstr(output,"pl"))                 {
                    netDevice |= NetDevice_Modem;
                }
            }

            fclose(f);
        }
    }
#endif 
    return netDevice;
}


int
wxDialUpManagerImpl::CheckIfconfig()
{
#ifdef __VMS
    m_CanUseIfconfig = 0;
    return -1;
#else
        int netDevice = NetDevice_Unknown;

        if ( m_CanUseIfconfig == -1 )     {
        static const wxChar *const ifconfigLocations[] =
        {
            wxT("/sbin"),                     wxT("/usr/sbin"),                 wxT("/usr/etc"),                  wxT("/etc"),                  };

        for ( size_t n = 0; n < WXSIZEOF(ifconfigLocations); n++ )
        {
            wxString path(ifconfigLocations[n]);
            path << wxT("/ifconfig");

            if ( wxFileExists(path) )
            {
                m_IfconfigPath = path;
                break;
            }
        }
    }

    if ( m_CanUseIfconfig != 0 )     {
        wxLogNull ln; 
        wxASSERT_MSG( !m_IfconfigPath.empty(),
                      wxT("can't use ifconfig if it wasn't found") );

        wxString tmpfile = wxFileName::CreateTempFileName( wxT("_wxdialuptest") );
        wxString cmd = wxT("/bin/sh -c \'");
        cmd << m_IfconfigPath;
#if defined(__AIX__) || \
    defined(__NETBSD__) || \
    defined(__OSF__) || \
    defined(__SOLARIS__) || defined (__SUNOS__)
                cmd << wxT(" -a");
#elif defined(__LINUX__) || defined(__SGI__) || defined(__OPENBSD__)
        #elif defined(__FREEBSD__) || defined(__DARWIN__) || defined(__QNX__)
                cmd << wxT(" -l");
#elif defined(__HPUX__)
                cmd << wxT(" ppp0");
#else
        #if defined(__GNUG__)
            #warning "No ifconfig information for this OS."
        #else
            #pragma warning "No ifconfig information for this OS."
        #endif

        m_CanUseIfconfig = 0;
        return -1;
#endif
       cmd << wxT(" >") << tmpfile <<  wxT('\'');
        
        if ( wxExecute(cmd,true ) == 0 )
        {
            m_CanUseIfconfig = 1;
            wxFFile file;
            if( file.Open(tmpfile) )
            {
                wxString output;
                if ( file.ReadAll(&output) )
                {
                    
                    bool hasModem = false,
                         hasLAN = false;

#if defined(__SOLARIS__) || defined (__SUNOS__)
                                        hasModem = strstr(output.fn_str(),"ipdptp") != NULL;
                    hasLAN = strstr(output.fn_str(), "hme") != NULL;
#elif defined(__LINUX__) || defined (__FREEBSD__) || defined (__QNX__) || \
      defined(__OPENBSD__)
                    hasModem = strstr(output.fn_str(),"ppp")                            || strstr(output.fn_str(),"sl")                          || strstr(output.fn_str(),"pl");                     hasLAN = strstr(output.fn_str(), "eth") != NULL;
#elif defined(__SGI__)                      hasModem = strstr(output.fn_str(), "ppp") != NULL; #elif defined(__HPUX__)
                                        hasModem = true;
#endif

                    netDevice = NetDevice_None;
                    if ( hasModem )
                        netDevice |= NetDevice_Modem;
                    if ( hasLAN )
                        netDevice |= NetDevice_LAN;
                }
                            }
                    }
        else         {
            m_CanUseIfconfig = 0;         }

        (void) wxRemoveFile(tmpfile);
    }

    return netDevice;
#endif
}

wxDialUpManagerImpl::NetConnection wxDialUpManagerImpl::CheckPing()
{
            if(m_CanUsePing == -1)     {
#ifdef __VMS
        if (wxFileExists( wxT("SYS$SYSTEM:TCPIP$PING.EXE") ))
            m_PingPath = wxT("$SYS$SYSTEM:TCPIP$PING");
#elif defined(__AIX__)
        m_PingPath = wxT("/etc/ping");
#elif defined(__SGI__)
        m_PingPath = wxT("/usr/etc/ping");
#else
        if (wxFileExists( wxT("/bin/ping") ))
            m_PingPath = wxT("/bin/ping");
        else if (wxFileExists( wxT("/usr/sbin/ping") ))
            m_PingPath = wxT("/usr/sbin/ping");
#endif
        if (!m_PingPath)
        {
            m_CanUsePing = 0;
        }
    }

    if(! m_CanUsePing)
    {
              return Net_Unknown;
    }

    wxLogNull ln;     wxASSERT(m_PingPath.length());
    wxString cmd;
    cmd << m_PingPath << wxT(' ');
#if defined(__SOLARIS__) || defined (__SUNOS__)
    #elif defined(__AIX__) || \
      defined (__BSD__) || \
      defined(__LINUX__) || \
      defined(__OSF__) || \
      defined(__SGI__) || \
      defined(__VMS) || \
      defined(__QNX__)
    cmd << wxT("-c 1 "); #elif defined(__HPUX__)
    cmd << wxT("64 1 "); #else
    #if defined(__GNUG__)
        #warning "No Ping information for this OS."
    #else
        #pragma warning "No Ping information for this OS."
    #endif

    m_CanUsePing = 0;
    return Net_Unknown;
#endif
    cmd << m_BeaconHost;
    if(wxExecute(cmd, true ) == 0)
        return Net_Connected;
    else
        return Net_No;
}


wxDialUpManager *wxDialUpManager::Create()
{
   return new wxDialUpManagerImpl;
}

#endif 