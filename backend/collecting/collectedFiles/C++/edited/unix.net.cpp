
#include "wx/setup.h"

#if wxUSE_DIALUP_MANAGER

#ifndef  WX_PRECOMP
#   include "wx/defs.h"
#endif 
#include "wx/string.h"
#include "wx/event.h"
#include "wx/net.h"
#include "wx/timer.h"
#include "wx/filename.h"
#include "wx/utils.h"
#include "wx/log.h"
#include "wx/file.h"

#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#define __STRICT_ANSI__
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>




class WXDLLEXPORT wxDialUpManagerImpl : public wxDialUpManager
{
public:
   wxDialUpManagerImpl()
      {
         m_IsOnline = -1;          m_timer = NULL;
         m_CanUseIfconfig = -1;          m_BeaconHost = WXDIALUP_MANAGER_DEFAULT_BEACONHOST;
         m_BeaconPort = 80;
      }

   
   virtual bool IsOk() const
      { return true; }

   
   virtual bool Dial(const wxString& nameOfISP,
                     const wxString& WXUNUSED(username),
                     const wxString& WXUNUSED(password));

      virtual bool HangUp();

            virtual bool IsOnline() const
      {
         if( (! m_timer)              || m_IsOnline == -1
            )
            CheckStatus();
         return m_IsOnline != 0;
      }

               virtual void SetOnlineStatus(bool isOnline = true)
      { m_IsOnline = isOnline; }

      
                        virtual bool EnableAutoCheckOnlineStatus(size_t nSeconds);

         virtual void DisableAutoCheckOnlineStatus();

            virtual void SetWellKnownHost(const wxString& hostname,
                                 int portno = 80);
   
   virtual void SetConnectCommand(const wxString &command, const wxString &hupcmd)
      { m_ConnectCommand = command; m_HangUpCommand = hupcmd; }

private:
      int m_IsOnline;

      int m_CanUseIfconfig;
      wxString m_IfconfigPath;

      wxString m_BeaconHost;
      int m_BeaconPort;

      wxString m_ConnectCommand;
      wxString m_HangUpCommand;
      wxString m_ISPname;
      class AutoCheckTimer *m_timer;

   friend class AutoCheckTimer;
      void CheckStatus(void) const;

      void CheckStatusInternal(void);
};


class AutoCheckTimer : public wxTimer
{
public:
   AutoCheckTimer(wxDialUpManagerImpl *dupman)
      {
         m_dupman = dupman;
         m_started = false;
      }

   virtual bool Start( int millisecs = -1 )
      { m_started = true; return wxTimer::Start(millisecs, false); }

   virtual void Notify()
      { wxLogTrace("Checking dial up network status."); m_dupman->CheckStatus(); }

   virtual void Stop()
      { if ( m_started ) wxTimer::Stop(); }
public:
   bool m_started;
   wxDialUpManagerImpl *m_dupman;
};

bool
wxDialUpManagerImpl::Dial(const wxString &isp,
                          const wxString & WXUNUSED(username),
                          const wxString & WXUNUSED(password))
{
   if(m_IsOnline == 1)
      return false;
   m_IsOnline = -1;
   m_ISPname = isp;
   wxString cmd;
   if(m_ConnectCommand.Find("%s"))
      cmd.Printf(m_ConnectCommand,m_ISPname.c_str());
   else
      cmd = m_ConnectCommand;
   return wxExecute(cmd,  TRUE) == 0;
}

bool
wxDialUpManagerImpl::HangUp(void)
{
   if(m_IsOnline == 0)
      return false;
   m_IsOnline = -1;
   wxString cmd;
   if(m_HangUpCommand.Find("%s"))
      cmd.Printf(m_HangUpCommand,m_ISPname.c_str());
   else
      cmd = m_HangUpCommand;
   return wxExecute(cmd,  TRUE) == 0;
}


bool
wxDialUpManagerImpl::EnableAutoCheckOnlineStatus(size_t nSeconds)
{
   wxASSERT(m_timer == NULL);
   m_timer = new AutoCheckTimer(this);
   bool rc = m_timer->Start(nSeconds*1000);
   if(! rc)
   {
      wxDELETE(m_timer);
   }
   return rc;
}

void
wxDialUpManagerImpl::DisableAutoCheckOnlineStatus()
{
   wxASSERT(m_timer != NULL);
   m_timer->Stop();
   wxDELETE(m_timer);
}


void
wxDialUpManagerImpl::SetWellKnownHost(const wxString& hostname, int portno)
{
      wxString port = hostname.After(':');
   if(port.empty())
   {
      m_BeaconHost = hostname;
      m_BeaconPort = portno;
   }
   else
   {
      m_BeaconHost = hostname.Before(':');
      m_BeaconPort = atoi(port);
   }
}


void
wxDialUpManagerImpl::CheckStatus(void) const
{
      
   int oldIsOnline = m_IsOnline;
   (  (wxDialUpManagerImpl *)this)->CheckStatusInternal();

      if(m_IsOnline != oldIsOnline)
   {
      if(m_IsOnline)
         ;       else
         ;    }
}



void
wxDialUpManagerImpl::CheckStatusInternal(void)
{
   m_IsOnline = -1;

         if(m_CanUseIfconfig == -1)    {
      if(wxFileExists("/sbin/ifconfig"))
         m_IfconfigPath = "/sbin/ifconfig";
      else if(wxFileExists("/usr/sbin/ifconfig"))
         m_IfconfigPath = "/usr/sbin/ifconfig";
   }

   wxLogNull ln;       if(m_CanUseIfconfig != 0)    {
      wxASSERT( !m_IfconfigPath.empty() );

      wxString tmpfile = wxFileName::CreateTempFileName("_wxdialuptest");
      wxString cmd = "/bin/sh -c \'";
      cmd << m_IfconfigPath << " >" << tmpfile <<  '\'';
      
#if 0
            int
         new_stdout = dup(STDOUT_FILENO),
         new_stderr = dup(STDERR_FILENO);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);

      int
                  output_fd = open(tmpfile, O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR),
                  null_fd = open("/dev/null", O_CREAT, S_IRUSR|S_IWUSR);
            wxASSERT(output_fd == STDOUT_FILENO);
      wxASSERT(null_fd == STDERR_FILENO);
      int rc = wxExecute(m_IfconfigPath,TRUE ,NULL ,wxEXECUTE_DONT_CLOSE_FDS);
      close(null_fd); close(output_fd);
            int test;
      test = dup(new_stdout); close(new_stdout); wxASSERT(test == STDOUT_FILENO);
      test = dup(new_stderr); close(new_stderr); wxASSERT(test == STDERR_FILENO);
      if(rc == 0)
#endif
      if(wxExecute(cmd,TRUE ) == 0)
      {
         m_CanUseIfconfig = 1;
         wxFile file;
         if( file.Open(tmpfile) )
         {
            char *output = new char [file.Length()+1];
            output[file.Length()] = '\0';
            if(file.Read(output,file.Length()) == file.Length())
            {
               if(strstr(output,"ppp")                     || strstr(output,"sl")                   || strstr(output,"pl")                   )
                  m_IsOnline = 1;
               else
                  m_IsOnline = 0;
            }
            file.Close();
            delete [] output;
         }
               }
      else          m_CanUseIfconfig = 0;       (void) wxRemoveFile(tmpfile);
      if(m_IsOnline != -1)          return;
   }

         struct hostent     *hp;
   struct sockaddr_in  serv_addr;
   int sockfd;

   m_IsOnline = 0;    if((hp = gethostbyname(m_BeaconHost)) == NULL)
      return; 
   serv_addr.sin_family = hp->h_addrtype;
   memcpy(&serv_addr.sin_addr,hp->h_addr, hp->h_length);
   serv_addr.sin_port = htons(m_BeaconPort);
   if( ( sockfd = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0)
   {
            return;
   }
   if( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
   {
            return;
   }
      close(sockfd);
}



wxDialUpManager *
wxDialUpManager::wxDialUpManager::Create(void)
{
   return new wxDialUpManagerImpl;
}

#endif 