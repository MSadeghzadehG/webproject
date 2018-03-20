


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/apptrait.h"

#ifndef WX_PRECOMP
    #include "wx/utils.h"
#endif 
#include "wx/unix/private/execute.h"
#include "wx/evtloop.h"


int wxGUIAppTraits::WaitForChild(wxExecuteData& execData)
{
            wxBusyCursor bc;
    wxWindowDisabler wd(!(execData.flags & wxEXEC_NODISABLE));

                wxGUIEventLoop loop;
    return RunLoopUntilChildExit(execData, loop);
}

