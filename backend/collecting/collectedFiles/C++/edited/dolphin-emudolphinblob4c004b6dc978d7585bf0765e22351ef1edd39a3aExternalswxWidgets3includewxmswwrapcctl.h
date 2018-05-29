
#ifndef _WX_MSW_WRAPCCTL_H_
#define _WX_MSW_WRAPCCTL_H_

#include "wx/msw/wrapwin.h"

#include <commctrl.h>

#include "wx/msw/missing.h"

inline void wxSetCCUnicodeFormat(HWND hwnd)
{
    ::SendMessage(hwnd, CCM_SETUNICODEFORMAT, wxUSE_UNICODE, 0);
}

#if wxUSE_GUI
class wxFont;
extern wxFont wxGetCCDefaultFont();

struct wxHDITEM : public HDITEM
{
    wxHDITEM()
    {
        ::ZeroMemory(this, sizeof(*this));
    }
};

#endif 
#endif 