


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/msw/wrapcctl.h"
#endif 
#if wxUSE_DATEPICKCTRL || wxUSE_CALENDARCTRL

#include "wx/msw/private/datecontrols.h"

#include "wx/dynlib.h"


bool wxMSWDateControls::CheckInitialization()
{
                    
            static int s_initResult = -1;     if ( s_initResult == -1 )
    {
                        s_initResult = false;

#if wxUSE_DYNLIB_CLASS
        INITCOMMONCONTROLSEX icex;
        icex.dwSize = sizeof(icex);
        icex.dwICC = ICC_DATE_CLASSES;

                        wxLoadedDLL dllComCtl32(wxT("comctl32.dll"));
        if ( dllComCtl32.IsLoaded() )
        {
            wxLogNull noLog;

            typedef BOOL (WINAPI *ICCEx_t)(INITCOMMONCONTROLSEX *);
            wxDYNLIB_FUNCTION( ICCEx_t, InitCommonControlsEx, dllComCtl32 );

            if ( pfnInitCommonControlsEx )
            {
                s_initResult = (*pfnInitCommonControlsEx)(&icex);
            }
        }
#endif     }

    return s_initResult != 0;
}

#endif 