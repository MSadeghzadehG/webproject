



#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_POPUPWIN

#ifndef WX_PRECOMP
#endif 
#include "wx/popupwin.h"
#include "wx/tooltip.h"

#include "wx/osx/private.h"


wxPopupWindow::~wxPopupWindow()
{
}

bool wxPopupWindow::Create(wxWindow *parent, int flags)
{
        Hide();

    return wxPopupWindowBase::Create(parent) &&
               wxNonOwnedWindow::Create(parent, wxID_ANY,
                                wxDefaultPosition, wxDefaultSize,
                                flags | wxPOPUP_WINDOW);

}


bool wxPopupWindow::Show(bool show)
{
    if ( !wxWindow::Show(show) )
        return false;

    if ( m_nowpeer && show)
        m_nowpeer->ShowWithoutActivating();
    else if ( m_nowpeer )
        m_nowpeer->Show(false);

    if ( show )
    {
                wxSizeEvent event(GetSize() , m_windowId);
        event.SetEventObject(this);
        HandleWindowEvent(event);
    }

    return true;
}


#endif 