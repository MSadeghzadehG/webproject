


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_FINDREPLDLG

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcdlg.h"
    #include "wx/intl.h"
    #include "wx/log.h"
#endif

#include "wx/fdrepdlg.h"


UINT_PTR CALLBACK wxFindReplaceDialogHookProc(HWND hwnd,
                                              UINT uiMsg,
                                              WPARAM wParam,
                                              LPARAM lParam);


wxIMPLEMENT_DYNAMIC_CLASS(wxFindReplaceDialog, wxDialog);


class WXDLLEXPORT wxFindReplaceDialogImpl
{
public:
    wxFindReplaceDialogImpl(wxFindReplaceDialog *dialog, int flagsWX);
    ~wxFindReplaceDialogImpl();

    void InitFindWhat(const wxString& str);
    void InitReplaceWith(const wxString& str);

        FINDREPLACE *GetPtrFindReplace() { return &m_findReplace; }

        void SetClosedByUser() { m_wasClosedByUser = true; }
    bool WasClosedByUser() const { return m_wasClosedByUser; }

private:
        static bool FindMessageHandler(wxWindow *win,
                                   WXUINT nMsg,
                                   WPARAM wParam,
                                   LPARAM lParam);

        void InitString(const wxString& str, LPTSTR *ppStr, WORD *pLen);


        FINDREPLACE m_findReplace;

        bool m_wasClosedByUser;

        static UINT ms_msgFindDialog;

    wxDECLARE_NO_COPY_CLASS(wxFindReplaceDialogImpl);
};

UINT wxFindReplaceDialogImpl::ms_msgFindDialog = 0;



wxFindReplaceDialogImpl::wxFindReplaceDialogImpl(wxFindReplaceDialog *dialog,
                                                 int flagsWX)
{
        if ( !ms_msgFindDialog )
    {
        ms_msgFindDialog = ::RegisterWindowMessage(FINDMSGSTRING);

        if ( !ms_msgFindDialog )
        {
            wxLogLastError(wxT("RegisterWindowMessage(FINDMSGSTRING)"));
        }

        wxWindow::MSWRegisterMessageHandler
                  (
                    ms_msgFindDialog,
                    &wxFindReplaceDialogImpl::FindMessageHandler
                  );
    }

    m_wasClosedByUser = false;

    wxZeroMemory(m_findReplace);

    
        int flags = FR_ENABLEHOOK;

    int flagsDialog = dialog->GetWindowStyle();
    if ( flagsDialog & wxFR_NOMATCHCASE)
        flags |= FR_NOMATCHCASE;
    if ( flagsDialog & wxFR_NOWHOLEWORD)
        flags |= FR_NOWHOLEWORD;
    if ( flagsDialog & wxFR_NOUPDOWN)
        flags |= FR_NOUPDOWN;

        if ( flagsWX & wxFR_DOWN)
        flags |= FR_DOWN;
    if ( flagsWX & wxFR_MATCHCASE)
        flags |= FR_MATCHCASE;
    if ( flagsWX & wxFR_WHOLEWORD )
        flags |= FR_WHOLEWORD;

    m_findReplace.lStructSize = sizeof(FINDREPLACE);
    m_findReplace.hwndOwner = GetHwndOf(dialog->GetParent());
    m_findReplace.Flags = flags;

    m_findReplace.lCustData = (LPARAM)dialog;
    m_findReplace.lpfnHook = wxFindReplaceDialogHookProc;
}

void wxFindReplaceDialogImpl::InitString(const wxString& str,
                                         LPTSTR *ppStr, WORD *pLen)
{
    size_t len = str.length() + 1;
    if ( len < 80 )
    {
                len = 80;
    }

    *ppStr = new wxChar[len];
    wxStrcpy(*ppStr, str);
    *pLen = (WORD)len;
}

void wxFindReplaceDialogImpl::InitFindWhat(const wxString& str)
{
    InitString(str, &m_findReplace.lpstrFindWhat, &m_findReplace.wFindWhatLen);
}

void wxFindReplaceDialogImpl::InitReplaceWith(const wxString& str)
{
    InitString(str,
               &m_findReplace.lpstrReplaceWith,
               &m_findReplace.wReplaceWithLen);
}

wxFindReplaceDialogImpl::~wxFindReplaceDialogImpl()
{
    delete [] m_findReplace.lpstrFindWhat;
    delete [] m_findReplace.lpstrReplaceWith;
}


bool
wxFindReplaceDialogImpl::FindMessageHandler(wxWindow * WXUNUSED(win),
                                            WXUINT WXUNUSED_UNLESS_DEBUG(nMsg),
                                            WPARAM WXUNUSED(wParam),
                                            LPARAM lParam)
{
    wxASSERT_MSG( nMsg == ms_msgFindDialog, wxT("unexpected message received") );

    FINDREPLACE *pFR = (FINDREPLACE *)lParam;

    wxFindReplaceDialog *dialog = (wxFindReplaceDialog *)pFR->lCustData;

        wxEventType evtType;

    bool replace = false;
    if ( pFR->Flags & FR_DIALOGTERM )
    {
                                dialog->GetImpl()->SetClosedByUser();

        evtType = wxEVT_FIND_CLOSE;
    }
    else if ( pFR->Flags & FR_FINDNEXT )
    {
        evtType = wxEVT_FIND_NEXT;
    }
    else if ( pFR->Flags & FR_REPLACE )
    {
        evtType = wxEVT_FIND_REPLACE;

        replace = true;
    }
    else if ( pFR->Flags & FR_REPLACEALL )
    {
        evtType = wxEVT_FIND_REPLACE_ALL;

        replace = true;
    }
    else
    {
        wxFAIL_MSG( wxT("unknown find dialog event") );

        return 0;
    }

    wxUint32 flags = 0;
    if ( pFR->Flags & FR_DOWN )
        flags |= wxFR_DOWN;
    if ( pFR->Flags & FR_WHOLEWORD )
        flags |= wxFR_WHOLEWORD;
    if ( pFR->Flags & FR_MATCHCASE )
        flags |= wxFR_MATCHCASE;

    wxFindDialogEvent event(evtType, dialog->GetId());
    event.SetEventObject(dialog);
    event.SetFlags(flags);
    event.SetFindString(pFR->lpstrFindWhat);
    if ( replace )
    {
        event.SetReplaceString(pFR->lpstrReplaceWith);
    }

    dialog->Send(event);

    return true;
}


UINT_PTR CALLBACK
wxFindReplaceDialogHookProc(HWND hwnd,
                            UINT uiMsg,
                            WPARAM WXUNUSED(wParam),
                            LPARAM lParam)
{
    if ( uiMsg == WM_INITDIALOG )
    {
        FINDREPLACE *pFR = (FINDREPLACE *)lParam;
        wxFindReplaceDialog *dialog = (wxFindReplaceDialog *)pFR->lCustData;

        ::SetWindowText(hwnd, dialog->GetTitle().t_str());

                return TRUE;
    }

    return 0;
}



void wxFindReplaceDialog::Init()
{
    m_impl = NULL;
    m_FindReplaceData = NULL;

        m_isShown = false;
}

wxFindReplaceDialog::wxFindReplaceDialog(wxWindow *parent,
                                         wxFindReplaceData *data,
                                         const wxString &title,
                                         int flags)
                   : wxFindReplaceDialogBase(parent, data, title, flags)
{
    Init();

    (void)Create(parent, data, title, flags);
}

wxFindReplaceDialog::~wxFindReplaceDialog()
{
    if ( m_impl )
    {
                                if ( !m_impl->WasClosedByUser() )
        {
                        if ( !::DestroyWindow(GetHwnd()) )
            {
                wxLogLastError(wxT("DestroyWindow(find dialog)"));
            }
        }

                delete m_impl;
    }

        m_isShown = false;

        m_hWnd = (WXHWND)NULL;
}

bool wxFindReplaceDialog::Create(wxWindow *parent,
                                 wxFindReplaceData *data,
                                 const wxString &title,
                                 int flags)
{
    m_windowStyle = flags;
    m_FindReplaceData = data;
    m_parent = parent;

    SetTitle(title);

        return parent != NULL;
}


bool wxFindReplaceDialog::Show(bool show)
{
    if ( !wxWindowBase::Show(show) )
    {
                return false;
    }

        if ( m_hWnd )
    {
                (void)::ShowWindow(GetHwnd(), show ? SW_SHOW : SW_HIDE);

        return true;
    }

    if ( !show )
    {
                return true;
    }

    wxCHECK_MSG( m_FindReplaceData, false, wxT("call Create() first!") );

    wxASSERT_MSG( !m_impl, wxT("why don't we have the window then?") );

    m_impl = new wxFindReplaceDialogImpl(this, m_FindReplaceData->GetFlags());

    m_impl->InitFindWhat(m_FindReplaceData->GetFindString());

    bool replace = HasFlag(wxFR_REPLACEDIALOG);
    if ( replace )
    {
        m_impl->InitReplaceWith(m_FindReplaceData->GetReplaceString());
    }

        FINDREPLACE *pFR = m_impl->GetPtrFindReplace();
    HWND hwnd;
    if ( replace )
        hwnd = ::ReplaceText(pFR);
    else
        hwnd = ::FindText(pFR);

    if ( !hwnd )
    {
        wxLogError(_("Failed to create the standard find/replace dialog (error code %d)"),
                   ::CommDlgExtendedError());

        wxDELETE(m_impl);

        return false;
    }

    if ( !::ShowWindow(hwnd, SW_SHOW) )
    {
        wxLogLastError(wxT("ShowWindow(find dialog)"));
    }

    m_hWnd = (WXHWND)hwnd;

    return true;
}



void wxFindReplaceDialog::SetTitle( const wxString& title)
{
    m_title = title;
}

wxString wxFindReplaceDialog::GetTitle() const
{
    return m_title;
}


void wxFindReplaceDialog::DoSetSize(int WXUNUSED(x), int WXUNUSED(y),
                                    int WXUNUSED(width), int WXUNUSED(height),
                                    int WXUNUSED(sizeFlags))
{
        return;
}

void wxFindReplaceDialog::DoGetSize(int *width, int *height) const
{
        if ( width )
        *width = 225;
    if ( height )
        *height = 324;
}

void wxFindReplaceDialog::DoGetClientSize(int *width, int *height) const
{
        if ( width )
        *width = 219;
    if ( height )
        *height = 299;
}

#endif 