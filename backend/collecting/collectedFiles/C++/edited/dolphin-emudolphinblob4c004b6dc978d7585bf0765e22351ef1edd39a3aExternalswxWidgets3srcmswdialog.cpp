


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/dialog.h"
#include "wx/modalhook.h"

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcdlg.h"
    #include "wx/utils.h"
    #include "wx/frame.h"
    #include "wx/app.h"
    #include "wx/button.h"
    #include "wx/settings.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/toolbar.h"
#endif

#include "wx/msw/private.h"
#include "wx/evtloop.h"
#include "wx/scopedptr.h"



class wxDialogModalData
{
public:
    wxDialogModalData(wxDialog *dialog) : m_evtLoop(dialog) { }

    void RunLoop()
    {
        m_evtLoop.Run();
    }

    void ExitLoop()
    {
        m_evtLoop.Exit();
    }

private:
    wxModalEventLoop m_evtLoop;
};

wxDEFINE_TIED_SCOPED_PTR_TYPE(wxDialogModalData)



void wxDialog::Init()
{
    m_isShown = false;
    m_modalData = NULL;
    m_hGripper = 0;
}

bool wxDialog::Create(wxWindow *parent,
                      wxWindowID id,
                      const wxString& title,
                      const wxPoint& pos,
                      const wxSize& size,
                      long style,
                      const wxString& name)
{
    SetExtraStyle(GetExtraStyle() | wxTOPLEVEL_EX_DIALOG);

        style |= wxTAB_TRAVERSAL;

    if ( !wxTopLevelWindow::Create(parent, id, title, pos, size, style, name) )
        return false;

    if ( !m_hasFont )
        SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));

    if ( HasFlag(wxRESIZE_BORDER) )
    {
        CreateGripper();

        Connect(wxEVT_CREATE,
                wxWindowCreateEventHandler(wxDialog::OnWindowCreate));
    }

    return true;
}

wxDialog::~wxDialog()
{
        Show(false);

    DestroyGripper();
}


bool wxDialog::Show(bool show)
{
    if ( show == IsShown() )
        return false;

    if ( !show && m_modalData )
    {
                                                m_modalData->ExitLoop();
    }

    if ( show )
    {
        if (CanDoLayoutAdaptation())
            DoLayoutAdaptation();

                                InitDialog();
    }

    wxDialogBase::Show(show);

    if ( show )
    {
                                                        const wxSize size = GetClientSize();
        ::SendMessage(GetHwnd(), WM_SIZE,
                      SIZE_RESTORED, MAKELPARAM(size.x, size.y));
    }

    return true;
}

int wxDialog::ShowModal()
{
    WX_HOOK_MODAL_DIALOG();

    wxASSERT_MSG( !IsModal(), wxT("ShowModal() can't be called twice") );

    Show();

            if ( IsShown() )
    {
                wxDialogModalDataTiedPtr modalData(&m_modalData,
                                           new wxDialogModalData(this));
        modalData->RunLoop();
    }

    return GetReturnCode();
}

void wxDialog::EndModal(int retCode)
{
    wxASSERT_MSG( IsModal(), wxT("EndModal() called for non modal dialog") );

    SetReturnCode(retCode);

    Hide();
}


void wxDialog::SetWindowStyleFlag(long style)
{
    wxDialogBase::SetWindowStyleFlag(style);

    if ( HasFlag(wxRESIZE_BORDER) )
        CreateGripper();
    else
        DestroyGripper();
}

void wxDialog::CreateGripper()
{
    if ( !m_hGripper )
    {
                m_hGripper = (WXHWND)::CreateWindow
                               (
                                    wxT("SCROLLBAR"),
                                    wxT(""),
                                    WS_CHILD |
                                    WS_CLIPSIBLINGS |
                                    SBS_SIZEGRIP |
                                    SBS_SIZEBOX |
                                    SBS_SIZEBOXBOTTOMRIGHTALIGN,
                                    0, 0, 0, 0,
                                    GetHwnd(),
                                    0,
                                    wxGetInstance(),
                                    NULL
                               );
    }
}

void wxDialog::DestroyGripper()
{
    if ( m_hGripper )
    {
                                                wxASSERT_MSG( !IsShown() ||
                      ::GetWindow((HWND)m_hGripper, GW_HWNDNEXT) == 0,
            wxT("Bug in wxWidgets: gripper should be at the bottom of Z-order") );
        ::DestroyWindow((HWND) m_hGripper);
        m_hGripper = 0;
    }
}

void wxDialog::ShowGripper(bool show)
{
    wxASSERT_MSG( m_hGripper, wxT("shouldn't be called if we have no gripper") );

    if ( show )
        ResizeGripper();

    ::ShowWindow((HWND)m_hGripper, show ? SW_SHOW : SW_HIDE);
}

void wxDialog::ResizeGripper()
{
    wxASSERT_MSG( m_hGripper, wxT("shouldn't be called if we have no gripper") );

    HWND hwndGripper = (HWND)m_hGripper;

    const wxRect rectGripper = wxRectFromRECT(wxGetWindowRect(hwndGripper));
    const wxSize size = GetClientSize() - rectGripper.GetSize();

    ::SetWindowPos(hwndGripper, HWND_BOTTOM,
                   size.x, size.y,
                   rectGripper.width, rectGripper.height,
                   SWP_NOACTIVATE);
}

void wxDialog::OnWindowCreate(wxWindowCreateEvent& event)
{
    if ( m_hGripper && IsShown() &&
            event.GetWindow() && event.GetWindow()->GetParent() == this )
    {
                ::SetWindowPos((HWND)m_hGripper, HWND_BOTTOM, 0, 0, 0, 0,
                       SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
    }

    event.Skip();
}



WXLRESULT wxDialog::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
    WXLRESULT rc = 0;
    bool processed = false;

    switch ( message )
    {
        case WM_CLOSE:
                                    processed = !Close();
            break;

        case WM_SIZE:
            if ( m_hGripper )
            {
                switch ( wParam )
                {
                    case SIZE_MAXIMIZED:
                        ShowGripper(false);
                        break;

                    case SIZE_RESTORED:
                        ShowGripper(true);
                }
            }

                                                                                    rc = wxWindow::MSWWindowProc(message, wParam, lParam);
            processed = true;
            if ( HasFlag(wxFULL_REPAINT_ON_RESIZE) )
            {
                ::InvalidateRect(GetHwnd(), NULL, false );
            }
            break;
    }

    if ( !processed )
        rc = wxDialogBase::MSWWindowProc(message, wParam, lParam);

    return rc;
}
