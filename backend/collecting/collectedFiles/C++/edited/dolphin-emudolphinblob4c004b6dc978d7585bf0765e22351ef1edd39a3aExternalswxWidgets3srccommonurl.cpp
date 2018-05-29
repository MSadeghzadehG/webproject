
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_URL

#include "wx/url.h"

#ifndef WX_PRECOMP
    #include "wx/list.h"
    #include "wx/string.h"
    #include "wx/utils.h"
    #include "wx/module.h"
#endif

#include <string.h>
#include <ctype.h>

wxIMPLEMENT_CLASS(wxURL, wxURI);

wxProtoInfo *wxURL::ms_protocols = NULL;

USE_PROTOCOL(wxFileProto)

#if wxUSE_PROTOCOL_HTTP
USE_PROTOCOL(wxHTTP)

    wxHTTP *wxURL::ms_proxyDefault = NULL;
    bool wxURL::ms_useDefaultProxy = false;
#endif

#if wxUSE_PROTOCOL_FTP
USE_PROTOCOL(wxFTP)
#endif



wxURL::wxURL(const wxString& url) : wxURI(url)
{
    Init(url);
    ParseURL();
}

wxURL::wxURL(const wxURI& uri) : wxURI(uri)
{
    Init(uri.BuildURI());
    ParseURL();
}

wxURL::wxURL(const wxURL& url) : wxURI(url)
{
    Init(url.GetURL());
    ParseURL();
}

void wxURL::Init(const wxString& url)
{
    m_protocol = NULL;
    m_error = wxURL_NOERR;
    m_url = url;
#if wxUSE_URL_NATIVE
    m_nativeImp = CreateNativeImpObject();
#endif

#if wxUSE_PROTOCOL_HTTP
    if ( ms_useDefaultProxy && !ms_proxyDefault )
    {
        SetDefaultProxy( wxGetenv(wxT("HTTP_PROXY")) );

        if ( !ms_proxyDefault )
        {
                        ms_useDefaultProxy = false;
        }
    }

    m_useProxy = ms_proxyDefault != NULL;
    m_proxy = ms_proxyDefault;
#endif 
}


wxURL& wxURL::operator = (const wxString& url)
{
    wxURI::operator = (url);
    Free();
    Init(url);
    ParseURL();

    return *this;
}

wxURL& wxURL::operator = (const wxURI& uri)
{
    if (&uri != this)
    {
        wxURI::operator = (uri);
        Free();
        Init(uri.BuildURI());
        ParseURL();
    }

    return *this;
}

wxURL& wxURL::operator = (const wxURL& url)
{
    if (&url != this)
    {
        wxURI::operator = (url);
        Free();
        Init(url.GetURL());
        ParseURL();
    }

    return *this;
}


bool wxURL::ParseURL()
{
        if (!m_protocol)
    {
                CleanData();

                if (!HasScheme())
        {
            m_error = wxURL_SNTXERR;
            return false;
        }

                if (!FetchProtocol())
        {
            m_error = wxURL_NOPROTO;
            return false;
        }

                if (m_protoinfo->m_needhost)
        {
                        if (!HasServer())
            {
                m_error = wxURL_SNTXERR;
                return false;
            }
        }
    }

#if wxUSE_PROTOCOL_HTTP
    if (m_useProxy)
    {
                m_url = m_scheme + wxT(":");
        if (m_protoinfo->m_needhost)
            m_url = m_url + wxT("
                if (m_protocol)
            m_protocol->Destroy();
        m_protocol = m_proxy;     }
#endif 
    m_error = wxURL_NOERR;
    return true;
}


void wxURL::CleanData()
{
#if wxUSE_PROTOCOL_HTTP
    if (!m_useProxy)
#endif     {
        if (m_protocol)
        {
                        m_protocol->Destroy();
            m_protocol = NULL;
        }
    }
}

void wxURL::Free()
{
    CleanData();
#if wxUSE_PROTOCOL_HTTP
    if (m_proxy && m_proxy != ms_proxyDefault)
        delete m_proxy;
#endif #if wxUSE_URL_NATIVE
    delete m_nativeImp;
#endif
}

wxURL::~wxURL()
{
    Free();
}


bool wxURL::FetchProtocol()
{
    wxProtoInfo *info = ms_protocols;

    while (info)
    {
        if (m_scheme == info->m_protoname)
        {
            if ( m_port.empty() )
                m_port = info->m_servname;
            m_protoinfo = info;
            m_protocol = (wxProtocol *)m_protoinfo->m_cinfo->CreateObject();
            return true;
        }
        info = info->next;
    }
    return false;
}


wxInputStream *wxURL::GetInputStream()
{
    if (!m_protocol)
    {
        m_error = wxURL_NOPROTO;
        return NULL;
    }

    m_error = wxURL_NOERR;
    if (HasUserInfo())
    {
        size_t dwPasswordPos = m_userinfo.find(':');

        if (dwPasswordPos == wxString::npos)
            m_protocol->SetUser(Unescape(m_userinfo));
        else
        {
            m_protocol->SetUser(Unescape(m_userinfo(0, dwPasswordPos)));
            m_protocol->SetPassword(Unescape(m_userinfo(dwPasswordPos+1, m_userinfo.length() + 1)));
        }
    }

#if wxUSE_URL_NATIVE
            if (m_nativeImp)
    {
        wxInputStream *rc;
        rc = m_nativeImp->GetInputStream(this);
        if (rc != 0)
            return rc;
    }
    #endif 
#if wxUSE_SOCKETS
    wxIPV4address addr;

        if (
#if wxUSE_PROTOCOL_HTTP
         !m_useProxy &&
#endif          m_protoinfo->m_needhost )
    {
        if (!addr.Hostname(m_server))
        {
            m_error = wxURL_NOHOST;
            return NULL;
        }

        addr.Service(m_port);

        if (!m_protocol->Connect(addr))
        {
            m_error = wxURL_CONNERR;
            return NULL;
        }
    }
#endif 
    wxString fullPath;

#if wxUSE_PROTOCOL_HTTP
        if (m_useProxy)
        fullPath += m_url;
#endif 
    if(m_path.empty())
        fullPath += wxT("/");
    else
        fullPath += m_path;

    if (HasQuery())
        fullPath += wxT("?") + m_query;

    if (HasFragment())
        fullPath += wxT("#") + m_fragment;

    wxInputStream *the_i_stream = m_protocol->GetInputStream(fullPath);

    if (!the_i_stream)
    {
        m_error = wxURL_PROTOERR;
        return NULL;
    }

    return the_i_stream;
}

#if wxUSE_PROTOCOL_HTTP
void wxURL::SetDefaultProxy(const wxString& url_proxy)
{
    if ( !url_proxy )
    {
        if ( ms_proxyDefault )
        {
            ms_proxyDefault->Close();
            wxDELETE(ms_proxyDefault);
        }
    }
    else
    {
        wxString tmp_str = url_proxy;
        int pos = tmp_str.Find(wxT(':'));
        if (pos == wxNOT_FOUND)
            return;

        wxString hostname = tmp_str(0, pos),
        port = tmp_str(pos+1, tmp_str.length()-pos);
        wxIPV4address addr;

        if (!addr.Hostname(hostname))
            return;
        if (!addr.Service(port))
            return;

        if (ms_proxyDefault)
                        ms_proxyDefault->Close();
        else
            ms_proxyDefault = new wxHTTP();
        ms_proxyDefault->Connect(addr);
    }
}

void wxURL::SetProxy(const wxString& url_proxy)
{
    if ( !url_proxy )
    {
        if ( m_proxy && m_proxy != ms_proxyDefault )
        {
            m_proxy->Close();
            delete m_proxy;
        }

        m_useProxy = false;
    }
    else
    {
        wxString tmp_str;
        wxString hostname, port;
        int pos;
        wxIPV4address addr;

        tmp_str = url_proxy;
        pos = tmp_str.Find(wxT(':'));
                if (pos == wxNOT_FOUND)
            return;

        hostname = tmp_str(0, pos);
        port = tmp_str(pos+1, tmp_str.length()-pos);

        addr.Hostname(hostname);
        addr.Service(port);

                if (m_proxy && m_proxy != ms_proxyDefault)
            delete m_proxy;
        m_proxy = new wxHTTP();
        m_proxy->Connect(addr);

        CleanData();
                m_useProxy = true;
        ParseURL();
    }
}
#endif 

#if wxUSE_SOCKETS

class wxURLModule : public wxModule
{
public:
    wxURLModule();

    virtual bool OnInit() wxOVERRIDE;
    virtual void OnExit() wxOVERRIDE;

private:
    wxDECLARE_DYNAMIC_CLASS(wxURLModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxURLModule, wxModule);

wxURLModule::wxURLModule()
{
            AddDependency(wxClassInfo::FindClass(wxT("wxSocketModule")));
}

bool wxURLModule::OnInit()
{
#if wxUSE_PROTOCOL_HTTP
                
    if ( wxGetenv(wxT("HTTP_PROXY")) )
    {
        wxURL::ms_useDefaultProxy = true;
    }
#endif     return true;
}

void wxURLModule::OnExit()
{
#if wxUSE_PROTOCOL_HTTP
    wxDELETE(wxURL::ms_proxyDefault);
#endif }

#endif 

#endif 