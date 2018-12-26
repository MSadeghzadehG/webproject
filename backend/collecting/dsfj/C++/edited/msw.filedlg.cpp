


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_FILEDLG

#include "wx/filedlg.h"

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcdlg.h"
    #include "wx/msw/missing.h"
    #include "wx/utils.h"
    #include "wx/msgdlg.h"
    #include "wx/filefn.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/app.h"
    #include "wx/math.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "wx/dynlib.h"
#include "wx/filename.h"
#include "wx/scopeguard.h"
#include "wx/tokenzr.h"
#include "wx/modalhook.h"


# define wxMAXPATH   65534

# define wxMAXFILE   1024

# define wxMAXEXT    5


static wxRect gs_rectDialog(0, 0, 428, 266);


wxIMPLEMENT_CLASS(wxFileDialog, wxFileDialogBase);


namespace
{

#if wxUSE_DYNLIB_CLASS

typedef BOOL (WINAPI *GetProcessUserModeExceptionPolicy_t)(LPDWORD);
typedef BOOL (WINAPI *SetProcessUserModeExceptionPolicy_t)(DWORD);

GetProcessUserModeExceptionPolicy_t gs_pfnGetProcessUserModeExceptionPolicy
    = (GetProcessUserModeExceptionPolicy_t) -1;

SetProcessUserModeExceptionPolicy_t gs_pfnSetProcessUserModeExceptionPolicy
    = (SetProcessUserModeExceptionPolicy_t) -1;

DWORD gs_oldExceptionPolicyFlags = 0;

bool gs_changedPolicy = false;

#endif 

void ChangeExceptionPolicy()
{
#if wxUSE_DYNLIB_CLASS
    gs_changedPolicy = false;

    wxLoadedDLL dllKernel32(wxT("kernel32.dll"));

    if ( gs_pfnGetProcessUserModeExceptionPolicy
        == (GetProcessUserModeExceptionPolicy_t) -1)
    {
        wxDL_INIT_FUNC(gs_pfn, GetProcessUserModeExceptionPolicy, dllKernel32);
        wxDL_INIT_FUNC(gs_pfn, SetProcessUserModeExceptionPolicy, dllKernel32);
    }

    if ( !gs_pfnGetProcessUserModeExceptionPolicy
        || !gs_pfnSetProcessUserModeExceptionPolicy
        || !gs_pfnGetProcessUserModeExceptionPolicy(&gs_oldExceptionPolicyFlags) )
    {
        return;
    }

    if ( gs_pfnSetProcessUserModeExceptionPolicy(gs_oldExceptionPolicyFlags
        | 0x1  ) )
    {
        gs_changedPolicy = true;
    }

#endif }

void RestoreExceptionPolicy()
{
#if wxUSE_DYNLIB_CLASS
    if (gs_changedPolicy)
    {
        gs_changedPolicy = false;
        (void) gs_pfnSetProcessUserModeExceptionPolicy(gs_oldExceptionPolicyFlags);
    }
#endif }

} 

UINT_PTR APIENTRY
wxFileDialogHookFunction(HWND      hDlg,
                         UINT      iMsg,
                         WPARAM    WXUNUSED(wParam),
                         LPARAM    lParam)
{
    switch ( iMsg )
    {
        case WM_INITDIALOG:
            {
                OPENFILENAME* ofn = reinterpret_cast<OPENFILENAME *>(lParam);
                reinterpret_cast<wxFileDialog *>(ofn->lCustData)
                    ->MSWOnInitDialogHook((WXHWND)hDlg);
            }
            break;

        case WM_NOTIFY:
            {
                NMHDR* const pNM = reinterpret_cast<NMHDR*>(lParam);
                if ( pNM->code > CDN_LAST && pNM->code <= CDN_FIRST )
                {
                    OFNOTIFY* const
                        pNotifyCode = reinterpret_cast<OFNOTIFY *>(lParam);
                    wxFileDialog* const
                        dialog = reinterpret_cast<wxFileDialog *>(
                                        pNotifyCode->lpOFN->lCustData
                                    );

                    switch ( pNotifyCode->hdr.code )
                    {
                        case CDN_INITDONE:
                            dialog->MSWOnInitDone((WXHWND)hDlg);
                            break;

                        case CDN_SELCHANGE:
                            dialog->MSWOnSelChange((WXHWND)hDlg);
                            break;
                    }
                }
            }
            break;

        case WM_DESTROY:
                                                                        wxCopyRECTToRect(wxGetWindowRect(::GetParent(hDlg)), gs_rectDialog);
            break;
    }

        return 0;
}


wxFileDialog::wxFileDialog(wxWindow *parent,
                           const wxString& message,
                           const wxString& defaultDir,
                           const wxString& defaultFileName,
                           const wxString& wildCard,
                           long style,
                           const wxPoint& pos,
                           const wxSize& sz,
                           const wxString& name)
            : wxFileDialogBase(parent, message, defaultDir, defaultFileName,
                               wildCard, style, pos, sz, name)

{
    
    m_bMovedWindow = false;
    m_centreDir = 0;

                gs_rectDialog.x =
    gs_rectDialog.y = 0;
}

void wxFileDialog::GetPaths(wxArrayString& paths) const
{
    paths.Empty();

    wxString dir(m_dir);
    if ( m_dir.empty() || m_dir.Last() != wxT('\\') )
        dir += wxT('\\');

    size_t count = m_fileNames.GetCount();
    for ( size_t n = 0; n < count; n++ )
    {
        if (wxFileName(m_fileNames[n]).IsAbsolute())
            paths.Add(m_fileNames[n]);
        else
            paths.Add(dir + m_fileNames[n]);
    }
}

void wxFileDialog::GetFilenames(wxArrayString& files) const
{
    files = m_fileNames;
}

void wxFileDialog::DoGetPosition(int *x, int *y) const
{
    if ( x )
        *x = gs_rectDialog.x;
    if ( y )
        *y = gs_rectDialog.y;
}

void wxFileDialog::DoGetSize(int *width, int *height) const
{
    if ( width )
        *width = gs_rectDialog.width;
    if ( height )
        *height = gs_rectDialog.height;
}

void wxFileDialog::DoMoveWindow(int x, int y, int WXUNUSED(w), int WXUNUSED(h))
{
    gs_rectDialog.x = x;
    gs_rectDialog.y = y;

            HWND hwnd = GetHwnd();
    if ( hwnd )
    {
                       ::SetWindowPos(hwnd, HWND_TOP, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }
    else     {
        m_bMovedWindow = true;

                        m_centreDir = 0;
    }
}

void wxFileDialog::DoCentre(int dir)
{
    m_centreDir = dir;
    m_bMovedWindow = true;

        }

void wxFileDialog::MSWOnInitDone(WXHWND hDlg)
{
            HWND hFileDlg = ::GetParent((HWND)hDlg);

        SetHWND((WXHWND)hFileDlg);

    if ( m_centreDir )
    {
                RECT rect;
        GetWindowRect(hFileDlg, &rect);
        gs_rectDialog = wxRectFromRECT(rect);

                        wxFileDialogBase::DoCentre(m_centreDir);
    }
    else     {
        SetPosition(gs_rectDialog.GetPosition());
    }

            MSWOnSelChange(hDlg);

        SetHWND(NULL);
}

void wxFileDialog::MSWOnSelChange(WXHWND hDlg)
{
    TCHAR buf[MAX_PATH];
    LRESULT len = SendMessage(::GetParent(hDlg), CDM_GETFILEPATH,
                              MAX_PATH, reinterpret_cast<LPARAM>(buf));

    if ( len > 0 )
        m_currentlySelectedFilename = buf;
    else
        m_currentlySelectedFilename.clear();

    if ( m_extraControl )
        m_extraControl->UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

static bool DoShowCommFileDialog(OPENFILENAME *of, long style, DWORD *err)
{
    if ( style & wxFD_SAVE ? GetSaveFileName(of) : GetOpenFileName(of) )
        return true;

    if ( err )
    {
        *err = CommDlgExtendedError();
    }

    return false;
}

#if defined(__WIN64__)
    typedef OPENFILENAME wxOPENFILENAME;

    static const DWORD gs_ofStructSize = sizeof(OPENFILENAME);
#else     #define wxTRY_SMALLER_OPENFILENAME

    struct wxOPENFILENAME : public OPENFILENAME
    {
                void *pVoid;
        DWORD dw1;
        DWORD dw2;
    };

                static const DWORD wxOPENFILENAME_V5_SIZE = 88;

        static const DWORD wxOPENFILENAME_V4_SIZE = 76;

        static DWORD gs_ofStructSize = wxOPENFILENAME_V5_SIZE;
#endif 
static bool ShowCommFileDialog(OPENFILENAME *of, long style)
{
    DWORD errCode;
    bool success = DoShowCommFileDialog(of, style, &errCode);

#ifdef wxTRY_SMALLER_OPENFILENAME
            if ( !success && errCode == CDERR_STRUCTSIZE &&
            of->lStructSize != wxOPENFILENAME_V4_SIZE )
    {
        of->lStructSize = wxOPENFILENAME_V4_SIZE;

        success = DoShowCommFileDialog(of, style, &errCode);

        if ( success || !errCode )
        {
                        gs_ofStructSize = of->lStructSize;
        }
    }
#endif 
    if ( !success &&
            errCode == FNERR_INVALIDFILENAME &&
                of->lpstrFile[0] )
    {
                        of->lpstrFile[0] = wxT('\0');
        success = DoShowCommFileDialog(of, style, &errCode);
    }

    if ( !success )
    {
                if ( errCode != 0 )
        {
            wxLogError(_("File dialog failed with error code %0lx."), errCode);
        }
        
        return false;
    }

    return true;
}

void wxFileDialog::MSWOnInitDialogHook(WXHWND hwnd)
{
   SetHWND(hwnd);

   CreateExtraControl();

   SetHWND(NULL);
}

int wxFileDialog::ShowModal()
{
    WX_HOOK_MODAL_DIALOG();

    wxWindow* const parent = GetParentForModalDialog(m_parent, GetWindowStyle());
    WXHWND hWndParent = parent ? GetHwndOf(parent) : NULL;

    static wxChar fileNameBuffer [ wxMAXPATH ];               wxChar        titleBuffer    [ wxMAXFILE+1+wxMAXEXT ];  
    *fileNameBuffer = wxT('\0');
    *titleBuffer    = wxT('\0');

    long msw_flags = OFN_HIDEREADONLY;

    if ( HasFdFlag(wxFD_NO_FOLLOW) )
        msw_flags |= OFN_NODEREFERENCELINKS;

    if ( HasFdFlag(wxFD_FILE_MUST_EXIST) )
        msw_flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (m_bMovedWindow || HasExtraControlCreator())     {
        ChangeExceptionPolicy();
        msw_flags |= OFN_EXPLORER|OFN_ENABLEHOOK;
        msw_flags |= OFN_ENABLESIZING;
    }

    wxON_BLOCK_EXIT0(RestoreExceptionPolicy);

    if ( HasFdFlag(wxFD_MULTIPLE) )
    {
                msw_flags |= OFN_EXPLORER | OFN_ALLOWMULTISELECT;
    }

                if ( !HasFdFlag(wxFD_CHANGE_DIR) )
    {
        msw_flags |= OFN_NOCHANGEDIR;
    }

    if ( HasFdFlag(wxFD_OVERWRITE_PROMPT) )
    {
        msw_flags |= OFN_OVERWRITEPROMPT;
    }

    wxOPENFILENAME of;
    wxZeroMemory(of);

    of.lStructSize       = gs_ofStructSize;
    of.hwndOwner         = hWndParent;
    of.lpstrTitle        = m_message.t_str();
    of.lpstrFileTitle    = titleBuffer;
    of.nMaxFileTitle     = wxMAXFILE + 1 + wxMAXEXT;

    GlobalPtr hgbl;
    if ( HasExtraControlCreator() )
    {
        msw_flags |= OFN_ENABLETEMPLATEHANDLE;

        hgbl.Init(256, GMEM_ZEROINIT);
        GlobalPtrLock hgblLock(hgbl);
        LPDLGTEMPLATE lpdt = static_cast<LPDLGTEMPLATE>(hgblLock.Get());

        
        lpdt->style = DS_CONTROL | WS_CHILD | WS_CLIPSIBLINGS;
        lpdt->cdit = 0;                 lpdt->x = 0;
        lpdt->y = 0;

                const wxSize extraSize = GetExtraControlSize();
        const LONG baseUnits = ::GetDialogBaseUnits();
        lpdt->cx = ::MulDiv(extraSize.x, 4, LOWORD(baseUnits));
        lpdt->cy = ::MulDiv(extraSize.y, 8, HIWORD(baseUnits));

                
        of.hInstance = (HINSTANCE)lpdt;
    }

            
    wxString  dir;
    size_t    i, len = m_dir.length();
    dir.reserve(len);
    for ( i = 0; i < len; i++ )
    {
        wxChar ch = m_dir[i];
        switch ( ch )
        {
            case wxT('/'):
                                ch = wxT('\\');

                
            case wxT('\\'):
                while ( i < len - 1 )
                {
                    wxChar chNext = m_dir[i + 1];
                    if ( chNext != wxT('\\') && chNext != wxT('/') )
                        break;

                                        if (i > 0)
                        i++;
                    else
                        break;
                }
                
            default:
                                dir += ch;
        }
    }

    of.lpstrInitialDir   = dir.c_str();

    of.Flags             = msw_flags;
    of.lpfnHook          = wxFileDialogHookFunction;
    of.lCustData         = (LPARAM)this;

    wxArrayString wildDescriptions, wildFilters;

    size_t items = wxParseCommonDialogsFilter(m_wildCard, wildDescriptions, wildFilters);

    wxASSERT_MSG( items > 0 , wxT("empty wildcard list") );

    wxString filterBuffer;

    for (i = 0; i < items ; i++)
    {
        filterBuffer += wildDescriptions[i];
        filterBuffer += wxT("|");
        filterBuffer += wildFilters[i];
        filterBuffer += wxT("|");
    }

        for (i = 0; i < filterBuffer.length(); i++ ) {
        if ( filterBuffer.GetChar(i) == wxT('|') ) {
            filterBuffer[i] = wxT('\0');
        }
    }

    of.lpstrFilter  = filterBuffer.t_str();
    of.nFilterIndex = m_filterIndex + 1;

    
    wxStrlcpy(fileNameBuffer, m_fileName.c_str(), WXSIZEOF(fileNameBuffer));

    of.lpstrFile = fileNameBuffer;      of.nMaxFile  = wxMAXPATH;

                    wxString defextBuffer;     if (HasFdFlag(wxFD_SAVE))
    {
        const wxChar* extension = filterBuffer.t_str();
        int maxFilter = (int)(of.nFilterIndex*2L) - 1;

        for( int j = 0; j < maxFilter; j++ )                       extension = extension + wxStrlen( extension ) + 1;

                defextBuffer = AppendExtension(wxT("a"), extension);
        if (defextBuffer.StartsWith(wxT("a.")))
        {
            defextBuffer = defextBuffer.Mid(2);             of.lpstrDefExt = defextBuffer.c_str();
        }
    }

        const wxString cwdOrig = wxGetCwd();

    
    if ( !ShowCommFileDialog(&of, m_windowStyle) )
        return wxID_CANCEL;

                        if ( msw_flags & OFN_NOCHANGEDIR )
    {
        wxSetWorkingDirectory(cwdOrig);
    }

    m_fileNames.Empty();

    if ( ( HasFdFlag(wxFD_MULTIPLE) ) &&
#if defined(OFN_EXPLORER)
         ( fileNameBuffer[of.nFileOffset-1] == wxT('\0') )
#else
         ( fileNameBuffer[of.nFileOffset-1] == wxT(' ') )
#endif        )
    {
#if defined(OFN_EXPLORER)
        m_dir = fileNameBuffer;
        i = of.nFileOffset;
        m_fileName = &fileNameBuffer[i];
        m_fileNames.Add(m_fileName);
        i += m_fileName.length() + 1;

        while (fileNameBuffer[i] != wxT('\0'))
        {
            m_fileNames.Add(&fileNameBuffer[i]);
            i += wxStrlen(&fileNameBuffer[i]) + 1;
        }
#else
        wxStringTokenizer toke(fileNameBuffer, wxT(" \t\r\n"));
        m_dir = toke.GetNextToken();
        m_fileName = toke.GetNextToken();
        m_fileNames.Add(m_fileName);

        while (toke.HasMoreTokens())
            m_fileNames.Add(toke.GetNextToken());
#endif 
        m_path = m_dir;
        if ( m_dir.Last() != wxT('\\') )
            m_path += wxT('\\');

        m_path += m_fileName;
        m_filterIndex = (int)of.nFilterIndex - 1;
    }
    else
    {
        
        m_filterIndex = (int)of.nFilterIndex - 1;

        if ( !of.nFileExtension ||
             (of.nFileExtension && fileNameBuffer[of.nFileExtension] == wxT('\0')) )
        {
                        const wxChar* extension = filterBuffer.t_str();
            int   maxFilter = (int)(of.nFilterIndex*2L) - 1;

            for( int j = 0; j < maxFilter; j++ )                           extension = extension + wxStrlen( extension ) + 1;

            m_fileName = AppendExtension(fileNameBuffer, extension);
            wxStrlcpy(fileNameBuffer, m_fileName.c_str(), WXSIZEOF(fileNameBuffer));
        }

        m_path = fileNameBuffer;
        m_fileName = wxFileNameFromPath(fileNameBuffer);
        m_fileNames.Add(m_fileName);
        m_dir = wxPathOnly(fileNameBuffer);
    }

    return wxID_OK;

}

#endif 