


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/filehistory.h"

#if wxUSE_FILE_HISTORY

#include "wx/filename.h"

#include <glib.h>
#include <gtk/gtk.h>
#include "wx/gtk/private/string.h"
#include "wx/gtk/private.h"


void wxFileHistory::AddFileToHistory(const wxString& file)
{
    wxFileHistoryBase::AddFileToHistory(file);

#ifdef __WXGTK210__
    const wxString fullPath = wxFileName(file).GetFullPath();
#ifndef __WXGTK3__
    if ( !gtk_check_version(2,10,0) )
#endif
    {
        wxGtkString uri(g_filename_to_uri(wxGTK_CONV_FN(fullPath), NULL, NULL));

        if ( uri )
            gtk_recent_manager_add_item(gtk_recent_manager_get_default(), uri);
    }
#endif
}

#endif 