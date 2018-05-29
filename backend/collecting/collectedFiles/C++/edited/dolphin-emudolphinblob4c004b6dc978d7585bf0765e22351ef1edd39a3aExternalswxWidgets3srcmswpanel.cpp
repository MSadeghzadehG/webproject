


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/panel.h"
#endif 

bool wxPanel::HasTransparentBackground()
{
    for ( wxWindow *win = GetParent(); win; win = win->GetParent() )
    {
        if ( win->MSWHasInheritableBackground() )
            return true;

        if ( win->IsTopLevel() )
            break;
    }

    return false;
}

