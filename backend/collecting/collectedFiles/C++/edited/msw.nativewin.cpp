


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#endif 
#include "wx/nativewin.h"
#include "wx/msw/private.h"



bool
wxNativeWindow::Create(wxWindow* parent,
                       wxWindowID winid,
                       wxNativeWindowHandle hwnd)
{
    wxCHECK_MSG( hwnd, false, wxS("Invalid null HWND") );
    wxCHECK_MSG( parent, false, wxS("Must have a valid parent") );
    wxASSERT_MSG( ::GetParent(hwnd) == GetHwndOf(parent),
                  wxS("The native window has incorrect parent") );

    const wxRect r = wxRectFromRECT(wxGetWindowRect(hwnd));

            if ( !CreateBase(parent, winid,
                     r.GetPosition(), r.GetSize(),
                     0, wxDefaultValidator, wxS("nativewindow")) )
        return false;

    parent->AddChild(this);

    SubclassWin(hwnd);

    if ( winid == wxID_ANY )
    {
                                SetId(GetId());
    }
    else     {
                        wxASSERT_MSG( ::GetWindowLong(hwnd, GWL_ID) == winid,
                      wxS("Mismatch between wx and native IDs") );
    }

    InheritAttributes();

    return true;
}

void wxNativeWindow::DoDisown()
{
    }

wxNativeWindow::~wxNativeWindow()
{
            if ( m_ownedByUser )
        UnsubclassWin();
}


bool wxNativeContainerWindow::Create(wxNativeContainerWindowHandle hwnd)
{
    if ( !::IsWindow(hwnd) )
    {
                                                return false;
    }

        SubclassWin(hwnd);

        AdoptAttributesFromHWND();

    return true;
}

bool wxNativeContainerWindow::IsShown() const
{
    return (IsWindowVisible(static_cast<HWND>(m_hWnd)) != 0);
}

void wxNativeContainerWindow::OnNativeDestroyed()
{
            delete this;
}

WXLRESULT wxNativeContainerWindow::MSWWindowProc(WXUINT nMsg,
                                                 WXWPARAM wParam,
                                                 WXLPARAM lParam)
{
    switch ( nMsg )
    {
        case WM_CLOSE:
                                                            return MSWDefWindowProc(nMsg, wParam, lParam);

        case WM_DESTROY:
                                                MSWDefWindowProc(nMsg, wParam, lParam);

            OnNativeDestroyed();

            return 0;
    }

    return wxTopLevelWindow::MSWWindowProc(nMsg, wParam, lParam);
}

wxNativeContainerWindow::~wxNativeContainerWindow()
{
            DissociateHandle();
}

