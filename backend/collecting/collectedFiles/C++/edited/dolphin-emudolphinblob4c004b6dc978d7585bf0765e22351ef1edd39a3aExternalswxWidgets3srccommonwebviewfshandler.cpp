
#include "wx/wxprec.h"

#if wxUSE_WEBVIEW

#if defined(__BORLANDC__)
    #pragma hdrstop
#endif

#include "wx/webviewfshandler.h"
#include "wx/filesys.h"

wxWebViewFSHandler::wxWebViewFSHandler(const wxString& scheme) :
                         wxWebViewHandler(scheme)
{
    m_fileSystem = new wxFileSystem();
}

wxWebViewFSHandler::~wxWebViewFSHandler()
{
    wxDELETE(m_fileSystem);
}

wxFSFile* wxWebViewFSHandler::GetFile(const wxString &uri)
{
    return m_fileSystem->OpenFile(uri);
}

#endif 