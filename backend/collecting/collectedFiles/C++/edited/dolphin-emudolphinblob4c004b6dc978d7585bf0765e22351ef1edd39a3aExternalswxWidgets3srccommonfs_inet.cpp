
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if !wxUSE_SOCKETS
    #undef wxUSE_FS_INET
    #define wxUSE_FS_INET 0
#endif

#if wxUSE_FILESYSTEM && wxUSE_FS_INET

#ifndef WX_PRECOMP
    #include "wx/module.h"
#endif

#include "wx/wfstream.h"
#include "wx/url.h"
#include "wx/filesys.h"
#include "wx/fs_inet.h"


class wxTemporaryFileInputStream : public wxFileInputStream
{
public:
    wxTemporaryFileInputStream(const wxString& filename) :
        wxFileInputStream(filename), m_filename(filename) {}

    virtual ~wxTemporaryFileInputStream()
    {
                        if (m_file_destroy)
        {
            delete m_file;
            m_file_destroy = false;
        }
        wxRemoveFile(m_filename);
    }

protected:
    wxString m_filename;
};



static wxString StripProtocolAnchor(const wxString& location)
{
    wxString myloc(location.BeforeLast(wxT('#')));
    if (myloc.empty()) myloc = location.AfterFirst(wxT(':'));
    else myloc = myloc.AfterFirst(wxT(':'));

        if (!myloc.Left(2).IsSameAs(wxT("    {
        if (myloc.GetChar(0) != wxT('/')) myloc = wxT("        else myloc = wxT("/") + myloc;
    }
    if (myloc.Mid(2).Find(wxT('/')) == wxNOT_FOUND) myloc << wxT('/');

    return myloc;
}


bool wxInternetFSHandler::CanOpen(const wxString& location)
{
#if wxUSE_URL
    wxString p = GetProtocol(location);
    if ((p == wxT("http")) || (p == wxT("ftp")))
    {
        wxURL url(p + wxT(":") + StripProtocolAnchor(location));
        return (url.GetError() == wxURL_NOERR);
    }
#endif
    return false;
}


wxFSFile* wxInternetFSHandler::OpenFile(wxFileSystem& WXUNUSED(fs),
                                        const wxString& location)
{
#if !wxUSE_URL
    return NULL;
#else
    wxString right =
        GetProtocol(location) + wxT(":") + StripProtocolAnchor(location);

    wxURL url(right);
    if (url.GetError() == wxURL_NOERR)
    {
        wxInputStream *s = url.GetInputStream();
        wxString content = url.GetProtocol().GetContentType();
        if (s)
        {
            wxString tmpfile =
                wxFileName::CreateTempFileName(wxT("wxhtml"));

            {                   wxFileOutputStream sout(tmpfile);
                s->Read(sout);
            }
            delete s;

            return new wxFSFile(new wxTemporaryFileInputStream(tmpfile),
                                right,
                                content,
                                GetAnchor(location)
#if wxUSE_DATETIME
                                , wxDateTime::Now()
#endif                         );
        }
    }

    return NULL; #endif
}


class wxFileSystemInternetModule : public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxFileSystemInternetModule);

    public:
        wxFileSystemInternetModule() :
           wxModule(),
           m_handler(NULL)
        {
        }

        virtual bool OnInit() wxOVERRIDE
        {
            m_handler = new wxInternetFSHandler;
            wxFileSystem::AddHandler(m_handler);
            return true;
        }

        virtual void OnExit() wxOVERRIDE
        {
            delete wxFileSystem::RemoveHandler(m_handler);
        }

    private:
        wxFileSystemHandler* m_handler;
};

wxIMPLEMENT_DYNAMIC_CLASS(wxFileSystemInternetModule, wxModule);

#endif 