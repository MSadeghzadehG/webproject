


#include "wx/wxprec.h"

#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#if wxUSE_PROTOCOL_FTP

#ifndef WX_PRECOMP
    #include <stdlib.h>
    #include "wx/string.h"
    #include "wx/utils.h"
    #include "wx/log.h"
    #include "wx/intl.h"
    #include "wx/wxcrtvararg.h"
#endif 
#include "wx/sckaddr.h"
#include "wx/socket.h"
#include "wx/url.h"
#include "wx/sckstrm.h"
#include "wx/protocol/protocol.h"
#include "wx/protocol/ftp.h"

#include <memory.h>


static const size_t LEN_CODE = 3;


wxIMPLEMENT_DYNAMIC_CLASS(wxFTP, wxProtocol);
IMPLEMENT_PROTOCOL(wxFTP, wxT("ftp"), wxT("ftp"), true)



wxFTP::wxFTP()
{
    m_streaming = false;
    m_currentTransfermode = NONE;

    m_username = wxT("anonymous");
    m_password << wxGetUserId() << wxT('@') << wxGetFullHostName();

    m_bPassive = true;
    m_bEncounteredError = false;
}

wxFTP::~wxFTP()
{
    if ( m_streaming )
    {
                        (void)Abort();
    }

        Close();
}


bool wxFTP::Connect(const wxSockAddress& addr, bool WXUNUSED(wait))
{
    if ( !wxProtocol::Connect(addr) )
    {
        m_lastError = wxPROTO_NETERR;
        return false;
    }

    if ( !m_username )
    {
        m_lastError = wxPROTO_CONNERR;
        return false;
    }

        if ( !CheckResult('2') )
    {
        Close();
        return false;
    }

    wxString command;
    command.Printf(wxT("USER %s"), m_username.c_str());
    char rc = SendCommand(command);
    if ( rc == '2' )
    {
                m_lastError = wxPROTO_NOERR;
        return true;
    }

    if ( rc != '3' )
    {
        m_lastError = wxPROTO_CONNERR;
        Close();
        return false;
    }

    command.Printf(wxT("PASS %s"), m_password.c_str());
    if ( !CheckCommand(command, '2') )
    {
        m_lastError = wxPROTO_CONNERR;
        Close();
        return false;
    }

    m_lastError = wxPROTO_NOERR;
    return true;
}

bool wxFTP::Connect(const wxString& host, unsigned short port)
{
    wxIPV4address addr;
    addr.Hostname(host);

    if ( port )
        addr.Service(port);
    else if (!addr.Service(wxT("ftp")))
        addr.Service(21);

    return Connect(addr);
}

bool wxFTP::Close()
{
    if ( m_streaming )
    {
        m_lastError = wxPROTO_STREAMING;
        return false;
    }

    if ( IsConnected() )
    {
        if ( !CheckCommand(wxT("QUIT"), '2') )
        {
            m_lastError = wxPROTO_CONNERR;
            wxLogDebug(wxT("Failed to close connection gracefully."));
        }
    }

    return wxSocketClient::Close();
}


wxSocketBase *wxFTP::AcceptIfActive(wxSocketBase *sock)
{
    if ( m_bPassive )
        return sock;

        wxSocketServer *sockSrv = (wxSocketServer *)sock;
    if ( !sockSrv->WaitForAccept() )
    {
        m_lastError = wxPROTO_CONNERR;
        wxLogError(_("Timeout while waiting for FTP server to connect, try passive mode."));
        wxDELETE(sock);
    }
    else
    {
        m_lastError = wxPROTO_NOERR;
        sock = sockSrv->Accept(true);
        delete sockSrv;
    }

    return sock;
}

bool wxFTP::Abort()
{
    if ( !m_streaming )
        return true;

    m_streaming = false;
    if ( !CheckCommand(wxT("ABOR"), '4') )
        return false;

    return CheckResult('2');
}



char wxFTP::SendCommand(const wxString& command)
{
    if ( m_streaming )
    {
        m_lastError = wxPROTO_STREAMING;
        return 0;
    }

    wxString tmp_str = command + wxT("\r\n");
    const wxWX2MBbuf tmp_buf = tmp_str.mb_str();
    if ( Write(static_cast<const char *>(tmp_buf), strlen(tmp_buf)).Error())
    {
        m_lastError = wxPROTO_NETERR;
        return 0;
    }

        wxString cmd, password;
    if ( command.Upper().StartsWith(wxT("PASS "), &password) )
    {
        cmd << wxT("PASS ") << wxString(wxT('*'), password.length());
    }
    else
    {
        cmd = command;
    }

    LogRequest(cmd);

    m_lastError = wxPROTO_NOERR;
    return GetResult();
}


char wxFTP::GetResult()
{
            if ( m_bEncounteredError )
        return 0;

    wxString code;

            m_lastResult.clear();

                                bool badReply = false;
    bool firstLine = true;
    bool endOfReply = false;
    while ( !endOfReply && !badReply )
    {
        wxString line;
        m_lastError = ReadLine(this,line);
        if ( m_lastError )
        {
            m_bEncounteredError = true;
            return 0;
        }

        LogResponse(line);

        if ( !m_lastResult.empty() )
        {
                        m_lastResult += wxT('\n');
        }

        m_lastResult += line;

                        if ( line.Len() < LEN_CODE + 1 )
        {
            if ( firstLine )
            {
                badReply = true;
            }
        }
        else         {
                        wxChar chMarker = line.GetChar(LEN_CODE);

            if ( firstLine )
            {
                code = wxString(line, LEN_CODE);

                switch ( chMarker )
                {
                    case wxT(' '):
                        endOfReply = true;
                        break;

                    case wxT('-'):
                        firstLine = false;
                        break;

                    default:
                                                badReply = true;
                }
            }
            else             {
                if ( line.compare(0, LEN_CODE, code) == 0 )
                {
                    if ( chMarker == wxT(' ') )
                    {
                        endOfReply = true;
                    }
                }
            }
        }
    }

    if ( badReply )
    {
        wxLogDebug(wxT("Broken FTP server: '%s' is not a valid reply."),
                   m_lastResult.c_str());

        m_lastError = wxPROTO_PROTERR;

        return 0;
    }
    else
        m_lastError = wxPROTO_NOERR;

        return (char)code[0u];
}


bool wxFTP::SetTransferMode(TransferMode transferMode)
{
    if ( transferMode == m_currentTransfermode )
    {
                return true;
    }

    wxString mode;
    switch ( transferMode )
    {
        default:
            wxFAIL_MSG(wxT("unknown FTP transfer mode"));
            wxFALLTHROUGH;

        case BINARY:
            mode = wxT('I');
            break;

        case ASCII:
            mode = wxT('A');
            break;
    }

    if ( !DoSimpleCommand(wxT("TYPE"), mode) )
    {
        wxLogError(_("Failed to set FTP transfer mode to %s."),
                   (transferMode == ASCII ? _("ASCII") : _("binary")));

        return false;
    }

            m_currentTransfermode = transferMode;

    return true;
}

bool wxFTP::DoSimpleCommand(const wxChar *command, const wxString& arg)
{
    wxString fullcmd = command;
    if ( !arg.empty() )
    {
        fullcmd << wxT(' ') << arg;
    }

    if ( !CheckCommand(fullcmd, '2') )
    {
        wxLogDebug(wxT("FTP command '%s' failed."), fullcmd.c_str());
        m_lastError = wxPROTO_NETERR;

        return false;
    }

    m_lastError = wxPROTO_NOERR;
    return true;
}

bool wxFTP::ChDir(const wxString& dir)
{
            
    return DoSimpleCommand(wxT("CWD"), dir);
}

bool wxFTP::MkDir(const wxString& dir)
{
    return DoSimpleCommand(wxT("MKD"), dir);
}

bool wxFTP::RmDir(const wxString& dir)
{
    return DoSimpleCommand(wxT("RMD"), dir);
}

wxString wxFTP::Pwd()
{
    wxString path;

    if ( CheckCommand(wxT("PWD"), '2') )
    {
                wxString::const_iterator p = m_lastResult.begin() + LEN_CODE + 1;
        if ( *p != wxT('"') )
        {
            wxLogDebug(wxT("Missing starting quote in reply for PWD: %s"),
                       wxString(p, m_lastResult.end()));
        }
        else
        {
            for ( ++p; *p; ++p )
            {
                if ( *p == wxT('"') )
                {
                                        ++p;
                    if ( !*p || *p != wxT('"') )
                    {
                                                break;
                    }
                                                        }

                path += *p;
            }

            if ( !*p )
            {
                wxLogDebug(wxT("Missing ending quote in reply for PWD: %s"),
                           m_lastResult.c_str() + LEN_CODE + 1);
            }
        }
    }
    else
    {
        m_lastError = wxPROTO_PROTERR;
        wxLogDebug(wxT("FTP PWD command failed."));
    }

    return path;
}

bool wxFTP::Rename(const wxString& src, const wxString& dst)
{
    wxString str;

    str = wxT("RNFR ") + src;
    if ( !CheckCommand(str, '3') )
        return false;

    str = wxT("RNTO ") + dst;

    return CheckCommand(str, '2');
}

bool wxFTP::RmFile(const wxString& path)
{
    wxString str;
    str = wxT("DELE ") + path;

    return CheckCommand(str, '2');
}


wxSocketBase *wxFTP::GetPort()
{
    

    wxSocketBase *socket = m_bPassive ? GetPassivePort() : GetActivePort();
    if ( !socket )
    {
        m_bEncounteredError = true;
        return NULL;
    }

            socket->SetTimeout(m_uiDefaultTimeout);

    return socket;
}

wxString wxFTP::GetPortCmdArgument(const wxIPV4address& addrLocal,
                                   const wxIPV4address& addrNew)
{
            
    wxString addrIP = addrLocal.IPAddress();
    int portNew = addrNew.Service();

        addrIP.Replace(wxT("."), wxT(","));
    addrIP << wxT(',')
           << wxString::Format(wxT("%d"), portNew >> 8) << wxT(',')
           << wxString::Format(wxT("%d"), portNew & 0xff);

        return addrIP;
}

wxSocketBase *wxFTP::GetActivePort()
{
        wxIPV4address addrNew, addrLocal;
    GetLocal(addrLocal);
    addrNew.AnyAddress();
    addrNew.Service(0); 
    wxSocketServer *sockSrv = new wxSocketServer(addrNew);
    if (!sockSrv->IsOk())
    {
                m_lastError = wxPROTO_PROTERR;
        delete sockSrv;
        return NULL;
    }

        sockSrv->GetLocal(addrNew);

                wxString port = GetPortCmdArgument(addrLocal, addrNew);
    if ( !DoSimpleCommand(wxT("PORT"), port) )
    {
        m_lastError = wxPROTO_PROTERR;
        delete sockSrv;
        wxLogError(_("The FTP server doesn't support the PORT command."));
        return NULL;
    }

    m_lastError = wxPROTO_NOERR;
    sockSrv->Notify(false);     return sockSrv;
}

wxSocketBase *wxFTP::GetPassivePort()
{
    if ( !DoSimpleCommand(wxT("PASV")) )
    {
        m_lastError = wxPROTO_PROTERR;
        wxLogError(_("The FTP server doesn't support passive mode."));
        return NULL;
    }

    size_t addrStart = m_lastResult.find(wxT('('));
    size_t addrEnd = (addrStart == wxString::npos)
                     ? wxString::npos
                     : m_lastResult.find(wxT(')'), addrStart);

    if ( addrEnd == wxString::npos )
    {
        m_lastError = wxPROTO_PROTERR;
        return NULL;
    }

        int a[6];
    wxString straddr(m_lastResult, addrStart + 1, addrEnd - (addrStart + 1));
    wxSscanf(straddr, wxT("%d,%d,%d,%d,%d,%d"),
             &a[2],&a[3],&a[4],&a[5],&a[0],&a[1]);

    wxUint32 hostaddr = (wxUint16)a[2] << 24 |
                        (wxUint16)a[3] << 16 |
                        (wxUint16)a[4] << 8 |
                        a[5];
    wxUint16 port = (wxUint16)(a[0] << 8 | a[1]);

    wxIPV4address addr;
    addr.Hostname(hostaddr);
    addr.Service(port);

    wxSocketClient *client = new wxSocketClient();
    if ( !client->Connect(addr) )
    {
        m_lastError = wxPROTO_CONNERR;
        delete client;
        return NULL;
    }

    client->Notify(false);

    m_lastError = wxPROTO_NOERR;
    return client;
}



class wxInputFTPStream : public wxSocketInputStream
{
public:
    wxInputFTPStream(wxFTP *ftp, wxSocketBase *sock)
        : wxSocketInputStream(*sock)
    {
        m_ftp = ftp;
            }

    virtual ~wxInputFTPStream()
    {
        delete m_i_socket;   
                        
                char code = m_ftp->GetResult();
        if ('2' == code)
        {
                                     m_ftp->m_streaming = false;
            return;
        }
                if (0 == code)
        {
                                                m_ftp->Abort();
            m_ftp->Close();
            return;
        }
                                    }

    wxFTP *m_ftp;

    wxDECLARE_NO_COPY_CLASS(wxInputFTPStream);
};

class wxOutputFTPStream : public wxSocketOutputStream
{
public:
    wxOutputFTPStream(wxFTP *ftp_clt, wxSocketBase *sock)
        : wxSocketOutputStream(*sock), m_ftp(ftp_clt)
    {
    }

    virtual ~wxOutputFTPStream(void)
    {
        if ( IsOk() )
        {
                                    delete m_o_socket;

                        m_ftp->GetResult(); 
            m_ftp->m_streaming = false;
        }
        else
        {
                        m_ftp->Abort();

                        delete m_o_socket;
        }
    }

    wxFTP *m_ftp;

    wxDECLARE_NO_COPY_CLASS(wxOutputFTPStream);
};

wxInputStream *wxFTP::GetInputStream(const wxString& path)
{
    if ( ( m_currentTransfermode == NONE ) && !SetTransferMode(BINARY) )
    {
        m_lastError = wxPROTO_CONNERR;
        return NULL;
    }

    wxSocketBase *sock = GetPort();

    if ( !sock )
    {
        m_lastError = wxPROTO_NETERR;
        return NULL;
    }

    wxString tmp_str = wxT("RETR ") + wxURI::Unescape(path);
    if ( !CheckCommand(tmp_str, '1') )
    {
        delete sock;

        return NULL;
    }

    sock = AcceptIfActive(sock);
    if ( !sock )
    {
        m_lastError = wxPROTO_CONNERR;
        return NULL;
    }

    m_streaming = true;

    wxInputFTPStream *in_stream = new wxInputFTPStream(this, sock);

    m_lastError = wxPROTO_NOERR;
    return in_stream;
}

wxOutputStream *wxFTP::GetOutputStream(const wxString& path)
{
    if ( ( m_currentTransfermode == NONE ) && !SetTransferMode(BINARY) )
    {
        m_lastError = wxPROTO_CONNERR;
        return NULL;
    }

    wxSocketBase *sock = GetPort();

    wxString tmp_str = wxT("STOR ") + path;
    if ( !CheckCommand(tmp_str, '1') )
    {
        delete sock;

        return NULL;
    }

    sock = AcceptIfActive(sock);

    m_streaming = true;

    m_lastError = wxPROTO_NOERR;
    return new wxOutputFTPStream(this, sock);
}


bool wxFTP::GetList(wxArrayString& files,
                    const wxString& wildcard,
                    bool details)
{
    wxSocketBase *sock = GetPort();
    if (!sock) {
        m_lastError = wxPROTO_NETERR;
        return false;
    }

                        wxString line(details ? wxT("LIST") : wxT("NLST"));
    if ( !wildcard.empty() )
    {
        line << wxT(' ') << wildcard;
    }

    if ( !CheckCommand(line, '1') )
    {
        m_lastError = wxPROTO_PROTERR;
        wxLogDebug(wxT("FTP 'LIST' command returned unexpected result from server"));
        delete sock;
        return false;
    }

    sock = AcceptIfActive(sock);
    if ( !sock ) {
        m_lastError = wxPROTO_CONNERR;
        return false;
    }

    files.Empty();
    while (ReadLine(sock, line) == wxPROTO_NOERR )
    {
        files.Add(line);
    }

    delete sock;

        m_lastError = wxPROTO_NOERR;
    return CheckResult('2');
}

bool wxFTP::FileExists(const wxString& fileName)
{
            
    bool retval = false;
    wxArrayString fileList;

    if ( GetList(fileList, fileName, false) )
    {
                                                                        retval = !fileList.IsEmpty();
    }

    return retval;
}


int wxFTP::GetFileSize(const wxString& fileName)
{
            
    int filesize = -1;

        if ( FileExists(fileName) )
    {
        wxString command;

                                                TransferMode oldTransfermode = m_currentTransfermode;
        SetTransferMode(BINARY);
        command << wxT("SIZE ") << fileName;

        bool ok = CheckCommand(command, '2');

        if ( ok )
        {
                                                int statuscode;
            if ( wxSscanf(GetLastResult().c_str(), wxT("%i %i"),
                          &statuscode, &filesize) == 2 )
            {
                                ok = true;
            }
            else
            {
                                                ok = false;
            }
        }

                        if ( oldTransfermode != NONE )
        {
            SetTransferMode(oldTransfermode);
        }

                        if ( !ok )
        {
                                                            wxArrayString fileList;
            if ( GetList(fileList, fileName, true) )
            {
                if ( !fileList.IsEmpty() )
                {
                                                                                                                        const size_t numFiles = fileList.size();
                    size_t i;
                    for ( i = 0; i < fileList.GetCount(); i++ )
                    {
                        if ( fileList[i].Upper().Contains(fileName.Upper()) )
                            break;
                    }

                    if ( i != numFiles )
                    {
                                                                                                                                                                                                                                                
                                                                                                if ( fileList[i].Mid(0, 1) == wxT("-") )
                        {

                            if ( wxSscanf(fileList[i].c_str(),
                                          wxT("%*s %*s %*s %*s %i %*s %*s %*s %*s"),
                                          &filesize) != 9 )
                            {
                                                                wxLogDebug(wxT("Invalid LIST response"));
                            }
                        }
                        else                         {
                            if ( wxSscanf(fileList[i].c_str(),
                                          wxT("%*s %*s %i %*s"),
                                          &filesize) != 4 )
                            {
                                                                wxLogDebug(wxT("Invalid or unknown LIST response"));
                            }
                        }
                    }
                }
            }
        }
    }

        return filesize;
}

#endif 
