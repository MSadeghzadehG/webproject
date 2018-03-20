
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/log.h"
#endif

#include "wx/filename.h"

#if wxUSE_HELP && wxUSE_MS_HTML_HELP \
    && wxUSE_WXHTML_HELP && !defined(__WXUNIVERSAL__)

#include "wx/msw/helpchm.h"
#include "wx/html/helpctrl.h"
#include "wx/msw/helpbest.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxBestHelpController, wxHelpControllerBase);

bool wxBestHelpController::Initialize( const wxString& filename )
{
        wxCHMHelpController* chm = new wxCHMHelpController(m_parentWindow);

    m_helpControllerType = wxUseChmHelp;
        wxLogNull dontWarnOnFailure;

    if( chm->Initialize( GetValidFilename( filename ) ) )
    {
        m_helpController = chm;
        m_parentWindow = NULL;
        return true;
    }

        delete chm;

        wxHtmlHelpController *
        html = new wxHtmlHelpController(m_style, m_parentWindow);

    m_helpControllerType = wxUseHtmlHelp;
    if( html->Initialize( GetValidFilename( filename ) ) )
    {
        m_helpController = html;
        m_parentWindow = NULL;
        return true;
    }

        delete html;

    return false;
}

wxString wxBestHelpController::GetValidFilename( const wxString& filename ) const
{
    wxFileName fn(filename);

    switch( m_helpControllerType )
    {
        case wxUseChmHelp:
            fn.SetExt("chm");
            if( fn.FileExists() )
                return fn.GetFullPath();

            return filename;

        case wxUseHtmlHelp:
            fn.SetExt("htb");
            if( fn.FileExists() )
                return fn.GetFullPath();

            fn.SetExt("zip");
            if( fn.FileExists() )
                return fn.GetFullPath();

            fn.SetExt("hhp");
            if( fn.FileExists() )
                return fn.GetFullPath();

            return filename;

        default:
                        wxFAIL_MSG( wxT("wxBestHelpController: Must call Initialize, first!") );
    }

    return wxEmptyString;
}

#endif
    