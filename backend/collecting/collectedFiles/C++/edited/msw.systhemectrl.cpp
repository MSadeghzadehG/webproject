
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/systhemectrl.h"

#include "wx/msw/private.h"
#include "wx/msw/uxtheme.h"

#ifdef wxHAS_SYSTEM_THEMED_CONTROL

void wxSystemThemedControlBase::DoEnableSystemTheme(bool enable, wxWindow* window)
{
    if ( wxGetWinVersion() >= wxWinVersion_Vista )
    {
        if ( wxUxThemeEngine *te = wxUxThemeEngine::GetIfActive() )
        {
            const wchar_t* const sysThemeId = enable ? L"EXPLORER" : NULL;
            te->SetWindowTheme(GetHwndOf(window), sysThemeId, NULL);
        }
    }
}

#endif 