
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_PROTOCOL

#include "wx/protocol/protocol.h"
#include "wx/protocol/log.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/module.h"
#endif

#include "wx/url.h"
#include "wx/log.h"

#include <stdlib.h>


wxIMPLEMENT_CLASS(wxProtoInfo, wxObject);

wxProtoInfo::wxProtoInfo(const wxChar *name, const wxChar *serv,
                         const bool need_host1, wxClassInfo *info)
           : m_protoname(name),
             m_servname(serv)
{
    m_cinfo = info;
    m_needhost = need_host1;
#if wxUSE_URL
    next = wxURL::ms_protocols;
    wxURL::ms_protocols = this;
#else
    next = NULL;
#endif
}



#if wxUSE_SOCKETS
wxIMPLEMENT_ABSTRACT_CLASS(wxProtocol, wxSocketClient);
#else
wxIMPLEMENT_ABSTRACT_CLASS(wxProtocol, wxObject);
#endif

wxProtocol::wxProtocol()
#if wxUSE_SOCKETS
        : wxSocketClient((wxIsMainThread() && wxApp::IsMainLoopRunning()
                        ? wxSOCKET_NONE
                        : wxSOCKET_BLOCK) | wxSOCKET_WAITALL)
#endif
{
    m_lastError = wxPROTO_NOERR;
    m_log = NULL;
    SetDefaultTimeout(60);      }

void wxProtocol::SetDefaultTimeout(wxUint32 Value)
{
    m_uiDefaultTimeout = Value;
#if wxUSE_SOCKETS
    wxSocketBase::SetTimeout(Value); #endif
}

wxProtocol::~wxProtocol()
{
    delete m_log;
}

#if wxUSE_SOCKETS
bool wxProtocol::Reconnect()
{
    wxIPV4address addr;

    if (!GetPeer(addr))
    {
        Close();
        return false;
    }

    if (!Close())
        return false;

    if (!Connect(addr))
        return false;

    return true;
}



wxProtocolError wxProtocol::ReadLine(wxSocketBase *sock, wxString& result)
{
    static const int LINE_BUF = 4095;

    result.clear();

    wxCharBuffer buf(LINE_BUF);
    char *pBuf = buf.data();
    while ( sock->WaitForRead() )
    {
                sock->Peek(pBuf, LINE_BUF);

        size_t nRead = sock->LastCount();
        if ( !nRead && sock->Error() )
            return wxPROTO_NETERR;

                                pBuf[nRead] = '\0';
        const char *eol = strchr(pBuf, '\n');

                if ( eol )
        {
            if ( eol == pBuf )
            {
                                if ( result.empty() || result.Last() != wxT('\r') )
                {
                                        eol = NULL;
                }
                
                                nRead = 1;
            }
            else             {
                                nRead = eol - pBuf + 1;

                if ( eol[-1] != '\r' )
                {
                                        eol = NULL;
                }
            }
        }

        sock->Read(pBuf, nRead);
        if ( sock->LastCount() != nRead )
            return wxPROTO_NETERR;

        pBuf[nRead] = '\0';
        result += wxString::FromAscii(pBuf);

        if ( eol )
        {
                        result.RemoveLast(2);

            return wxPROTO_NOERR;
        }
    }

    return wxPROTO_NETERR;
}

wxProtocolError wxProtocol::ReadLine(wxString& result)
{
    return ReadLine(this, result);
}

#endif 

void wxProtocol::SetLog(wxProtocolLog *log)
{
    delete m_log;
    m_log = log;
}

void wxProtocol::LogRequest(const wxString& str)
{
    if ( m_log )
        m_log->LogRequest(str);
}

void wxProtocol::LogResponse(const wxString& str)
{
    if ( m_log )
        m_log->LogResponse(str);
}

void wxProtocolLog::DoLogString(const wxString& str)
{
    wxUnusedVar(str);     wxLogTrace(m_traceMask, "%s", str);
}

#endif 