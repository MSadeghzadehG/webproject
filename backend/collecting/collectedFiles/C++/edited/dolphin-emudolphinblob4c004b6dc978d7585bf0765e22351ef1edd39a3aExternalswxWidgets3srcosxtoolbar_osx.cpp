
#include "wx/wxprec.h"

#if wxUSE_TOOLBAR

#include "wx/toolbar.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/app.h"
#include "wx/osx/private.h"
#include "wx/geometry.h"
#include "wx/sysopt.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxToolBar, wxControl);

bool wxToolBar::Destroy()
{
#if wxOSX_USE_NATIVE_TOOLBAR
    MacUninstallNativeToolbar();
#endif
    return wxToolBarBase::Destroy();
}

#endif 