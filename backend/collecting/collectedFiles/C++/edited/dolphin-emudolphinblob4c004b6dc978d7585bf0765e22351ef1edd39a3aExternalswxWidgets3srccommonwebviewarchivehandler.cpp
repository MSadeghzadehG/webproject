
#include "wx/wxprec.h"

#if wxUSE_WEBVIEW

#if defined(__BORLANDC__)
    #pragma hdrstop
#endif

#include "wx/webviewarchivehandler.h"
#include "wx/filesys.h"

static wxString EscapeFileNameCharsInURL(const char *in)
{
    wxString s;

    for ( const unsigned char *p = (const unsigned char*)in; *p; ++p )
    {
        const unsigned char c = *p;

        if ( c == '/' || c == '-' || c == '.' || c == '_' || c == '~' ||
             (c >= '0' && c <= '9') ||
             (c >= 'a' && c <= 'z') ||
             (c >= 'A' && c <= 'Z') )
        {
            s << c;
        }
        else
        {
            s << wxString::Format("%%%02x", c);
        }
    }

    return s;
}

wxWebViewArchiveHandler::wxWebViewArchiveHandler(const wxString& scheme) :
                         wxWebViewHandler(scheme)
{
    m_fileSystem = new wxFileSystem();
}

wxWebViewArchiveHandler::~wxWebViewArchiveHandler()
{
    wxDELETE(m_fileSystem);
}

wxFSFile* wxWebViewArchiveHandler::GetFile(const wxString &uri)
{
            wxString path = uri;
    size_t hashloc = uri.find('#');
    if(hashloc != wxString::npos)
    {
        path = uri.substr(0, hashloc);
    }

        size_t start = wxString::npos;
    for(size_t i = 0; i < path.length(); i++)
    {
        if(path[i] == ';' && path.substr(i, 10) == ";protocol=")
        {
            start = i;
            break;
        }
    }

        if(start == wxString::npos)
    {
        size_t doubleslash = path.find("                if(doubleslash == wxString::npos)
            return NULL;

        wxString fspath = "file:" + 
                          EscapeFileNameCharsInURL(path.substr(doubleslash + 2).c_str());
        return m_fileSystem->OpenFile(fspath);
    }
        else
    {
        size_t end = path.find('/', start);
                if(end == wxString::npos)
        {
            return NULL;
        }
        wxString mainpath = path.substr(0, start);
        wxString archivepath = path.substr(end);
        wxString protstring = path.substr(start, end - start);
        wxString protocol = protstring.substr(10);
                size_t doubleslash = path.find("                if(doubleslash == wxString::npos)
            return NULL;

        wxString fspath = "file:" + 
                          EscapeFileNameCharsInURL(mainpath.substr(doubleslash + 2).c_str())
                          + "#" + protocol +":" + archivepath;
        return m_fileSystem->OpenFile(fspath);
    }
}

#endif 