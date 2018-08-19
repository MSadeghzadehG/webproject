
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_MSGDLG

#ifndef WX_PRECOMP
    #include "wx/msgdlg.h"
    #include "wx/app.h"
    #include "wx/intl.h"
    #include "wx/utils.h"
    #include "wx/msw/private.h"
    #include "wx/hashmap.h"
#endif

#include "wx/ptr_scpd.h"
#include "wx/dynlib.h"
#include "wx/msw/private/button.h"
#include "wx/msw/private/metrics.h"
#include "wx/msw/private/msgdlg.h"
#include "wx/modalhook.h"
#include "wx/fontutil.h"
#include "wx/textbuf.h"
#include "wx/display.h"

#ifndef TDF_SIZE_TO_CONTENT
    #define TDF_SIZE_TO_CONTENT 0x1000000
#endif

using namespace wxMSWMessageDialog;

wxIMPLEMENT_CLASS(wxMessageDialog, wxDialog);

WX_DECLARE_HASH_MAP(unsigned long, wxMessageDialog *,
                    wxIntegerHash, wxIntegerEqual,
                    wxMessageDialogMap);

const wxMessageDialog::ButtonAccessors wxMessageDialog::ms_buttons[] =
{
    { IDYES,    &wxMessageDialog::GetYesLabel    },
    { IDNO,     &wxMessageDialog::GetNoLabel     },
    { IDOK,     &wxMessageDialog::GetOKLabel     },
    { IDCANCEL, &wxMessageDialog::GetCancelLabel },
};

namespace
{

wxMessageDialogMap& HookMap()
{
    static wxMessageDialogMap s_Map;

    return s_Map;
}



void ScreenRectToClient(HWND hwnd, RECT& rc)
{
                    ::MapWindowPoints(HWND_DESKTOP, hwnd, reinterpret_cast<POINT *>(&rc), 2);
}

inline void SetWindowRect(HWND hwnd, const RECT& rc)
{
    ::MoveWindow(hwnd,
                 rc.left, rc.top,
                 rc.right - rc.left, rc.bottom - rc.top,
                 FALSE);
}

void MoveWindowToScreenRect(HWND hwnd, RECT rc)
{
    ScreenRectToClient(::GetParent(hwnd), rc);

    SetWindowRect(hwnd, rc);
}

} 

WXLRESULT wxCALLBACK
wxMessageDialog::HookFunction(int code, WXWPARAM wParam, WXLPARAM lParam)
{
        const DWORD tid = ::GetCurrentThreadId();
    wxMessageDialogMap::iterator node = HookMap().find(tid);
    wxCHECK_MSG( node != HookMap().end(), false,
                    wxT("bogus thread id in wxMessageDialog::Hook") );

    wxMessageDialog *  const wnd = node->second;

    const HHOOK hhook = (HHOOK)wnd->m_hook;
    const LRESULT rc = ::CallNextHookEx(hhook, code, wParam, lParam);

    if ( code == HCBT_ACTIVATE )
    {
                ::UnhookWindowsHookEx(hhook);
        wnd->m_hook = NULL;
        HookMap().erase(tid);

        wnd->SetHWND((HWND)wParam);

                        wnd->ReplaceStaticWithEdit();

                        if ( wnd->HasCustomLabels() )
            wnd->AdjustButtonLabels();

                if ( wnd->GetMessageDialogStyle() & wxCENTER )
            wnd->Center();         
                wnd->SetHWND(NULL);
    }

    return rc;
}

void wxMessageDialog::ReplaceStaticWithEdit()
{
        int nDisplay = wxDisplay::GetFromWindow(this);
    if ( nDisplay == wxNOT_FOUND )
        nDisplay = 0;
    const wxRect rectDisplay = wxDisplay(nDisplay).GetClientArea();

    if ( rectDisplay.Contains(GetRect()) )
    {
                return;
    }


                HWND hwndStatic = ::FindWindowEx(GetHwnd(), NULL, wxT("STATIC"), NULL);
    if ( ::GetWindowLong(hwndStatic, GWL_STYLE) & SS_ICON )
        hwndStatic = ::FindWindowEx(GetHwnd(), hwndStatic, wxT("STATIC"), NULL);

    if ( !hwndStatic )
    {
        wxLogDebug("Failed to find the static text control in message box.");
        return;
    }

        wxWindowBase::SetFont(GetMessageFont());

        RECT rc = wxGetWindowRect(hwndStatic);
    ScreenRectToClient(GetHwnd(), rc);

                const int hText = (7*rectDisplay.height)/8 -
                      (
                         2*::GetSystemMetrics(SM_CYFIXEDFRAME) +
                         ::GetSystemMetrics(SM_CYCAPTION) +
                         5*GetCharHeight()                       );
    const int dh = (rc.bottom - rc.top) - hText;     rc.bottom -= dh;

                                const int dw = ::GetSystemMetrics(SM_CXVSCROLL) +
                        4*::GetSystemMetrics(SM_CXEDGE);
    rc.right += dw;


                wxString text(wxGetWindowText(hwndStatic));
    for ( wxString::reverse_iterator i = text.rbegin(); i != text.rend(); ++i )
    {
        if ( *i != '\n' )
        {
                                    if ( i != text.rbegin() )
                text.erase(i.base() + 1, text.end());
            break;
        }
    }

        HWND hwndEdit = ::CreateWindow
                      (
                        wxT("EDIT"),
                        wxTextBuffer::Translate(text).t_str(),
                        WS_CHILD | WS_VSCROLL | WS_VISIBLE |
                        ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                        rc.left, rc.top,
                        rc.right - rc.left, rc.bottom - rc.top,
                        GetHwnd(),
                        NULL,
                        wxGetInstance(),
                        NULL
                      );

    if ( !hwndEdit )
    {
        wxLogDebug("Creation of replacement edit control failed in message box");
        return;
    }

        LRESULT hfont = ::SendMessage(hwndStatic, WM_GETFONT, 0, 0);
    ::SendMessage(hwndEdit, WM_SETFONT, hfont, 0);

        ::DestroyWindow(hwndStatic);


            RECT rcBox = wxGetWindowRect(GetHwnd());
    const int hMsgBox = rcBox.bottom - rcBox.top - dh;
    rcBox.top = (rectDisplay.height - hMsgBox)/2;
    rcBox.bottom = rcBox.top + hMsgBox + (rectDisplay.height - hMsgBox)%2;
    rcBox.left -= dw/2;
    rcBox.right += dw - dw/2;
    SetWindowRect(GetHwnd(), rcBox);

        for ( unsigned n = 0; n < WXSIZEOF(ms_buttons); n++ )
    {
        const HWND hwndBtn = ::GetDlgItem(GetHwnd(), ms_buttons[n].id);
        if ( !hwndBtn )
            continue;   
        rc = wxGetWindowRect(hwndBtn);
        rc.top -= dh;
        rc.bottom -= dh;
        rc.left += dw/2;
        rc.right += dw/2;
        MoveWindowToScreenRect(hwndBtn, rc);
    }
}

void wxMessageDialog::AdjustButtonLabels()
{
            
    
                    wxWindowBase::SetFont(GetMessageFont());

        int wBtnOld = 0,                    wBtnNew = 0;                RECT rcBtn;                     unsigned numButtons = 0;        unsigned n;
    for ( n = 0; n < WXSIZEOF(ms_buttons); n++ )
    {
        const HWND hwndBtn = ::GetDlgItem(GetHwnd(), ms_buttons[n].id);
        if ( !hwndBtn )
            continue;   
        numButtons++;

        const wxString label = (this->*ms_buttons[n].getter)();
        const wxSize sizeLabel = wxWindowBase::GetTextExtent(label);

                const RECT rc = wxGetWindowRect(hwndBtn);
        if ( !wBtnOld )
        {
                                    wBtnOld = rc.right - rc.left;

            rcBtn = rc;         }
        else
        {
            wxASSERT_MSG( wBtnOld == rc.right - rc.left,
                          "all buttons are supposed to be of same width" );
        }

        const int widthNeeded = wxMSWButton::GetFittingSize(this, sizeLabel).x;
        if ( widthNeeded > wBtnNew )
            wBtnNew = widthNeeded;

        ::SetWindowText(hwndBtn, label.t_str());
    }

    if ( wBtnNew <= wBtnOld )
    {
                return;
    }

        const int wBoxOld = wxGetClientRect(GetHwnd()).right;

    const int CHAR_WIDTH = GetCharWidth();
    const int MARGIN_OUTER = 2*CHAR_WIDTH;      const int MARGIN_INNER = CHAR_WIDTH;    
    RECT rcBox = wxGetWindowRect(GetHwnd());

    const int wAllButtons = numButtons*(wBtnNew + MARGIN_INNER) - MARGIN_INNER;
    int wBoxNew = 2*MARGIN_OUTER + wAllButtons;
    if ( wBoxNew > wBoxOld )
    {
        const int dw = wBoxNew - wBoxOld;
        rcBox.left -= dw/2;
        rcBox.right += dw - dw/2;

        SetWindowRect(GetHwnd(), rcBox);

                            }
    else     {
        wBoxNew = wBoxOld;
    }


    
            rcBtn.left = (rcBox.left + rcBox.right - wxGetClientRect(GetHwnd()).right +
                  wBoxNew - wAllButtons) / 2;
    rcBtn.right = rcBtn.left + wBtnNew;

    for ( n = 0; n < WXSIZEOF(ms_buttons); n++ )
    {
        const HWND hwndBtn = ::GetDlgItem(GetHwnd(), ms_buttons[n].id);
        if ( !hwndBtn )
            continue;

        MoveWindowToScreenRect(hwndBtn, rcBtn);

        rcBtn.left += wBtnNew + MARGIN_INNER;
        rcBtn.right += wBtnNew + MARGIN_INNER;
    }
}


wxFont wxMessageDialog::GetMessageFont()
{
    const NONCLIENTMETRICS& ncm = wxMSWImpl::GetNonClientMetrics();
    return wxNativeFontInfo(ncm.lfMessageFont);
}

int wxMessageDialog::ShowMessageBox()
{
    if ( wxTheApp && !wxTheApp->GetTopWindow() )
    {
                                        while ( wxTheApp->Pending() )
            wxTheApp->Dispatch();
    }

        m_parent = GetParentForModalDialog();
    HWND hWnd = m_parent ? GetHwndOf(m_parent) : NULL;

#if wxUSE_INTL
                            wxLocale * const loc = wxGetLocale();
    if ( loc && loc->GetLanguage() != wxLocale::GetSystemLanguage() )
    {
        if ( m_dialogStyle & wxYES_NO &&
                (GetCustomYesLabel().empty() && GetCustomNoLabel().empty()) )

        {
                                    SetYesNoLabels(_("&Yes"), _("&No"));
        }

                                                                                if ( GetCustomOKLabel().empty() && GetCustomCancelLabel().empty() )
            SetOKCancelLabels(_("OK"), _("Cancel"));
    }
#endif 
        unsigned int msStyle;
    const long wxStyle = GetMessageDialogStyle();
    if ( wxStyle & wxYES_NO )
    {
        if ( wxStyle & wxCANCEL )
            msStyle = MB_YESNOCANCEL;
        else
            msStyle = MB_YESNO;

        if ( wxStyle & wxNO_DEFAULT )
            msStyle |= MB_DEFBUTTON2;
        else if ( wxStyle & wxCANCEL_DEFAULT )
            msStyle |= MB_DEFBUTTON3;
    }
    else     {
        if ( wxStyle & wxCANCEL )
        {
            msStyle = MB_OKCANCEL;

            if ( wxStyle & wxCANCEL_DEFAULT )
                msStyle |= MB_DEFBUTTON2;
        }
        else         {
            msStyle = MB_OK;
        }
    }

    if ( wxStyle & wxHELP )
    {
        msStyle |= MB_HELP;
    }

        switch ( GetEffectiveIcon() )
    {
        case wxICON_ERROR:
            msStyle |= MB_ICONHAND;
            break;

        case wxICON_WARNING:
            msStyle |= MB_ICONEXCLAMATION;
            break;

        case wxICON_QUESTION:
            msStyle |= MB_ICONQUESTION;
            break;

        case wxICON_INFORMATION:
            msStyle |= MB_ICONINFORMATION;
            break;
    }

    if ( wxStyle & wxSTAY_ON_TOP )
        msStyle |= MB_TOPMOST;

    if ( wxApp::MSWGetDefaultLayout(m_parent) == wxLayout_RightToLeft )
        msStyle |= MB_RTLREADING | MB_RIGHT;

    if (hWnd)
        msStyle |= MB_APPLMODAL;
    else
        msStyle |= MB_TASKMODAL;

                const DWORD tid = ::GetCurrentThreadId();
    m_hook = ::SetWindowsHookEx(WH_CBT,
                                &wxMessageDialog::HookFunction, NULL, tid);
    HookMap()[tid] = this;

        const int msAns = MessageBox
                      (
                        hWnd,
                        GetFullMessage().t_str(),
                        m_caption.t_str(),
                        msStyle
                      );

    return MSWTranslateReturnCode(msAns);
}

int wxMessageDialog::ShowModal()
{
    WX_HOOK_MODAL_DIALOG();

#ifdef wxHAS_MSW_TASKDIALOG
    if ( HasNativeTaskDialog() )
    {
        TaskDialogIndirect_t taskDialogIndirect = GetTaskDialogIndirectFunc();
        wxCHECK_MSG( taskDialogIndirect, wxID_CANCEL, wxS("no task dialog?") );

        WinStruct<TASKDIALOGCONFIG> tdc;
        wxMSWTaskDialogConfig wxTdc( *this );
        wxTdc.MSWCommonTaskDialogInit( tdc );

        int msAns;
        HRESULT hr = taskDialogIndirect( &tdc, &msAns, NULL, NULL );
        if ( FAILED(hr) )
        {
            wxLogApiError( "TaskDialogIndirect", hr );
            return wxID_CANCEL;
        }

                                        if ( (msAns == IDCANCEL)
            && !(GetMessageDialogStyle() & (wxYES_NO|wxCANCEL)) )
        {
            msAns = IDOK;
        }

        return MSWTranslateReturnCode( msAns );
    }
#endif 
    return ShowMessageBox();
}

long wxMessageDialog::GetEffectiveIcon() const
{
        if ( (m_dialogStyle & wxICON_AUTH_NEEDED) &&
        wxMSWMessageDialog::HasNativeTaskDialog() )
    {
        return wxICON_AUTH_NEEDED;
    }

    return wxMessageDialogBase::GetEffectiveIcon();
}

void wxMessageDialog::DoCentre(int dir)
{
#ifdef wxHAS_MSW_TASKDIALOG
                if ( HasNativeTaskDialog() )
        return;
#endif 
    wxMessageDialogBase::DoCentre(dir);
}


#ifdef wxHAS_MSW_TASKDIALOG

wxMSWTaskDialogConfig::wxMSWTaskDialogConfig(const wxMessageDialogBase& dlg)
                     : buttons(new TASKDIALOG_BUTTON[MAX_BUTTONS])
{
    parent = dlg.GetParentForModalDialog();
    caption = dlg.GetCaption();
    message = dlg.GetMessage();
    extendedMessage = dlg.GetExtendedMessage();

                        if ( extendedMessage.empty() )
    {
                                        const size_t posNL = message.find('\n');
        if ( posNL != wxString::npos &&
                posNL < message.length() - 1 &&
                    message[posNL + 1 ] == '\n' )
        {
            extendedMessage.assign(message, posNL + 2, wxString::npos);
            message.erase(posNL);
        }
    }

    iconId = dlg.GetEffectiveIcon();
    style = dlg.GetMessageDialogStyle();
    useCustomLabels = dlg.HasCustomLabels();
    btnYesLabel = dlg.GetYesLabel();
    btnNoLabel = dlg.GetNoLabel();
    btnOKLabel = dlg.GetOKLabel();
    btnCancelLabel = dlg.GetCancelLabel();
    btnHelpLabel = dlg.GetHelpLabel();
}

void wxMSWTaskDialogConfig::MSWCommonTaskDialogInit(TASKDIALOGCONFIG &tdc)
{
                            tdc.dwFlags = TDF_EXPAND_FOOTER_AREA |
                  TDF_POSITION_RELATIVE_TO_WINDOW |
                  TDF_SIZE_TO_CONTENT;
    tdc.hInstance = wxGetInstance();
    tdc.pszWindowTitle = caption.t_str();

        tdc.hwndParent = parent ? GetHwndOf(parent) : NULL;

    if ( wxApp::MSWGetDefaultLayout(parent) == wxLayout_RightToLeft )
        tdc.dwFlags |= TDF_RTL_LAYOUT;

                                if ( !extendedMessage.empty() )
    {
        tdc.pszMainInstruction = message.t_str();
        tdc.pszContent = extendedMessage.t_str();
    }
    else
    {
        tdc.pszContent = message.t_str();
    }

        switch ( iconId )
    {
        case wxICON_ERROR:
            tdc.pszMainIcon = TD_ERROR_ICON;
            break;

        case wxICON_WARNING:
            tdc.pszMainIcon = TD_WARNING_ICON;
            break;

        case wxICON_INFORMATION:
            tdc.pszMainIcon = TD_INFORMATION_ICON;
            break;

        case wxICON_AUTH_NEEDED:
            tdc.pszMainIcon = TD_SHIELD_ICON;
            break;
    }

        tdc.pButtons = buttons.get();

    if ( style & wxYES_NO )
    {
        AddTaskDialogButton(tdc, IDYES, TDCBF_YES_BUTTON, btnYesLabel);
        AddTaskDialogButton(tdc, IDNO,  TDCBF_NO_BUTTON,  btnNoLabel);

        if (style & wxCANCEL)
            AddTaskDialogButton(tdc, IDCANCEL,
                                TDCBF_CANCEL_BUTTON, btnCancelLabel);

        if ( style & wxNO_DEFAULT )
            tdc.nDefaultButton = IDNO;
        else if ( style & wxCANCEL_DEFAULT )
            tdc.nDefaultButton = IDCANCEL;
    }
    else     {
        if ( style & wxCANCEL )
        {
            AddTaskDialogButton(tdc, IDOK, TDCBF_OK_BUTTON, btnOKLabel);
            AddTaskDialogButton(tdc, IDCANCEL,
                                TDCBF_CANCEL_BUTTON, btnCancelLabel);

            if ( style & wxCANCEL_DEFAULT )
                tdc.nDefaultButton = IDCANCEL;
        }
        else         {
                                                            if ( !useCustomLabels )
            {
                useCustomLabels = true;
                btnOKLabel = _("OK");
            }

            AddTaskDialogButton(tdc, IDCANCEL, TDCBF_CANCEL_BUTTON, btnOKLabel);
        }
    }

    if ( style & wxHELP )
    {
                        useCustomLabels = true;

        AddTaskDialogButton(tdc, IDHELP, 0 , btnHelpLabel);
    }
}

void wxMSWTaskDialogConfig::AddTaskDialogButton(TASKDIALOGCONFIG &tdc,
                                                int btnCustomId,
                                                int btnCommonId,
                                                const wxString& customLabel)
{
    if ( useCustomLabels )
    {
                TASKDIALOG_BUTTON &tdBtn = buttons[tdc.cButtons];

        tdBtn.nButtonID = btnCustomId;
        tdBtn.pszButtonText = customLabel.t_str();
        tdc.cButtons++;

                        wxASSERT_MSG( tdc.cButtons <= MAX_BUTTONS, wxT("Too many buttons") );
    }
    else
    {
        tdc.dwCommonButtons |= btnCommonId;
    }
}

wxCRIT_SECT_DECLARE(gs_csTaskDialogIndirect);

TaskDialogIndirect_t wxMSWMessageDialog::GetTaskDialogIndirectFunc()
{
                    static const TaskDialogIndirect_t
        INVALID_TASKDIALOG_FUNC = reinterpret_cast<TaskDialogIndirect_t>(-1);
    static TaskDialogIndirect_t s_TaskDialogIndirect = INVALID_TASKDIALOG_FUNC;

    wxCRIT_SECT_LOCKER(lock, gs_csTaskDialogIndirect);

    if ( s_TaskDialogIndirect == INVALID_TASKDIALOG_FUNC )
    {
        wxLoadedDLL dllComCtl32("comctl32.dll");
        wxDL_INIT_FUNC(s_, TaskDialogIndirect, dllComCtl32);
    }

    return s_TaskDialogIndirect;
}

#endif 
bool wxMSWMessageDialog::HasNativeTaskDialog()
{
#ifdef wxHAS_MSW_TASKDIALOG
    if ( wxGetWinVersion() >= wxWinVersion_6 )
    {
        if ( wxMSWMessageDialog::GetTaskDialogIndirectFunc() )
            return true;
    }
#endif 
    return false;
}

int wxMSWMessageDialog::MSWTranslateReturnCode(int msAns)
{
    int ans;
    switch (msAns)
    {
        default:
            wxFAIL_MSG(wxT("unexpected return code"));
            
        case IDCANCEL:
            ans = wxID_CANCEL;
            break;
        case IDOK:
            ans = wxID_OK;
            break;
        case IDYES:
            ans = wxID_YES;
            break;
        case IDNO:
            ans = wxID_NO;
            break;
        case IDHELP:
            ans = wxID_HELP;
            break;
    }

    return ans;
}

#endif 