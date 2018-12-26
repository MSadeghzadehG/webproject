


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_DIRDLG

#include "wx/dirdlg.h"

#ifndef WX_PRECOMP
#endif 

wxString wxDirSelector(const wxString& message,
                       const wxString& defaultPath,
                       long style,
                       const wxPoint& pos,
                       wxWindow *parent)
{
    wxString path;

    wxDirDialog dirDialog(parent, message, defaultPath, style, pos);
    if ( dirDialog.ShowModal() == wxID_OK )
    {
        path = dirDialog.GetPath();
    }

    return path;
}

#endif 