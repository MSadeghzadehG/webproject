


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/settings.h"

#ifndef WX_PRECOMP
    #include "wx/utils.h"
#endif 

wxSystemScreenType wxSystemSettings::ms_screen = wxSYS_SCREEN_NONE;


wxSystemScreenType wxSystemSettings::GetScreenType()
{
    if (ms_screen == wxSYS_SCREEN_NONE)
    {
                int x = GetMetric( wxSYS_SCREEN_X );

        ms_screen = wxSYS_SCREEN_DESKTOP;

        if (x < 800)
            ms_screen = wxSYS_SCREEN_SMALL;

        if (x < 640)
            ms_screen = wxSYS_SCREEN_PDA;

        if (x < 200)
            ms_screen = wxSYS_SCREEN_TINY;

                if (x < 10)
            ms_screen = wxSYS_SCREEN_DESKTOP;
    }

    return ms_screen;
}

void wxSystemSettings::SetScreenType( wxSystemScreenType screen )
{
    ms_screen = screen;
}
