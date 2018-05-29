
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_FILESYSTEM

#include "wx/fs_filter.h"

#ifndef WX_PRECOMP
#endif

#include "wx/scopedptr.h"

wxDEFINE_SCOPED_PTR_TYPE(wxFSFile)
wxDEFINE_SCOPED_PTR_TYPE(wxInputStream)


bool wxFilterFSHandler::CanOpen(const wxString& location)
{
    return wxFilterClassFactory::Find(GetProtocol(location)) != NULL;
}

wxFSFile* wxFilterFSHandler::OpenFile(
        wxFileSystem& fs,
        const wxString& location)
{
    wxString right = GetRightLocation(location);
    if (!right.empty())
        return NULL;

    wxString protocol = GetProtocol(location);
    const wxFilterClassFactory *factory = wxFilterClassFactory::Find(protocol);
    if (!factory)
        return NULL;

    wxString left = GetLeftLocation(location);
    wxFSFilePtr leftFile(fs.OpenFile(left));
    if (!leftFile.get())
        return NULL;

    wxInputStreamPtr leftStream(leftFile->DetachStream());
    if (!leftStream.get() || !leftStream->IsOk())
        return NULL;

    wxInputStreamPtr stream(factory->NewStream(leftStream.release()));

                                wxString mime = leftFile->GetMimeType();
    if (factory->CanHandle(mime, wxSTREAM_MIMETYPE))
        mime = GetMimeTypeFromExt(factory->PopExtension(left));

    return new wxFSFile(stream.release(),
                        left + wxT("#") + protocol + wxT(":") + right,
                        mime,
                        GetAnchor(location)
#if wxUSE_DATETIME
                        , leftFile->GetModificationTime()
#endif                        );
}

wxString wxFilterFSHandler::FindFirst(const wxString& WXUNUSED(spec), int WXUNUSED(flags))
{
    return wxEmptyString;
}

wxString wxFilterFSHandler::FindNext()
{
    return wxEmptyString;
}

#endif 