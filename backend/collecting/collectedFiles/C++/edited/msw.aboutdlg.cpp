


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_ABOUTDLG

#ifndef WX_PRECOMP
    #include "wx/msgdlg.h"
#endif 
#include "wx/aboutdlg.h"
#include "wx/generic/aboutdlgg.h"


void wxAboutBox(const wxAboutDialogInfo& info, wxWindow* parent)
{
                if ( info.IsSimple() )
    {
                const wxString name = info.GetName();
        wxString msg;
        msg << name;
        if ( info.HasVersion() )
        {
            msg << wxT('\n');
            msg << info.GetLongVersion();
        }

        msg << wxT("\n\n");

        if ( info.HasCopyright() )
            msg << info.GetCopyrightToDisplay() << wxT('\n');

                msg << info.GetDescriptionAndCredits();

        wxMessageBox(msg, wxString::Format(_("About %s"), name), wxOK | wxCENTRE, parent);
    }
    else     {
                wxGenericAboutBox(info, parent);
    }
}

#endif 