


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_POPUPWIN

#ifndef WX_PRECOMP
#endif 
#include "wx/popupwin.h"

#include "wx/msw/private.h"     

bool wxPopupWindow::Create(wxWindow *parent, int flags)
{
        Hide();

    return wxPopupWindowBase::Create(parent) &&
               wxWindow::Create(parent, wxID_ANY,
                                wxDefaultPosition, wxDefaultSize,
                                flags | wxPOPUP_WINDOW);
}

WXDWORD wxPopupWindow::MSWGetStyle(long flags, WXDWORD *exstyle) const
{
        WXDWORD style = wxWindow::MSWGetStyle(flags & wxBORDER_MASK, exstyle);

    if ( exstyle )
    {
                *exstyle |= WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
    }

    return style;
}

WXHWND wxPopupWindow::MSWGetParent() const
{
                                return (WXHWND)::GetDesktopWindow();
}

void wxPopupWindow::SetFocus()
{
                    }

bool wxPopupWindow::Show(bool show)
{
    if ( !wxWindowMSW::Show(show) )
        return false;

    if ( show )
    {
                if (!::SetWindowPos(GetHwnd(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE))
        {
            wxLogLastError(wxT("SetWindowPos"));
        }

                ::SetForegroundWindow(GetHwnd());
    }

    return true;
}

#endif 