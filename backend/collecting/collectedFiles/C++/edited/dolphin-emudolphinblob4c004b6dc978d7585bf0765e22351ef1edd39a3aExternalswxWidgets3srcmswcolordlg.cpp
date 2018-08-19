


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_COLOURDLG

#include "wx/colordlg.h"
#include "wx/modalhook.h"

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcdlg.h"
    #include <stdio.h>
    #include "wx/colour.h"
    #include "wx/gdicmn.h"
    #include "wx/utils.h"
    #include "wx/math.h"
#endif

#include "wx/msw/private.h"

#include <stdlib.h>
#include <string.h>


static wxRect gs_rectDialog(0, 0, 222, 324);


wxIMPLEMENT_DYNAMIC_CLASS(wxColourDialog, wxDialog);



UINT_PTR CALLBACK
wxColourDialogHookProc(HWND hwnd,
                       UINT uiMsg,
                       WPARAM WXUNUSED(wParam),
                       LPARAM lParam)
{
    if ( uiMsg == WM_INITDIALOG )
    {
        CHOOSECOLOR *pCC = (CHOOSECOLOR *)lParam;
        wxColourDialog * const
            dialog = reinterpret_cast<wxColourDialog *>(pCC->lCustData);

        const wxString title = dialog->GetTitle();
        if ( !title.empty() )
            ::SetWindowText(hwnd, title.t_str());

        dialog->MSWOnInitDone((WXHWND)hwnd);
    }

    return 0;
}


void wxColourDialog::Init()
{
    m_movedWindow = false;
    m_centreDir = 0;

                gs_rectDialog.x =
    gs_rectDialog.y = 0;
}

bool wxColourDialog::Create(wxWindow *parent, wxColourData *data)
{
    m_parent = parent;
    if (data)
        m_colourData = *data;

    return true;
}

int wxColourDialog::ShowModal()
{
    WX_HOOK_MODAL_DIALOG();

    wxWindow* const parent = GetParentForModalDialog(m_parent, GetWindowStyle());
    WXHWND hWndParent = parent ? GetHwndOf(parent) : NULL;

        CHOOSECOLOR chooseColorStruct;
    memset(&chooseColorStruct, 0, sizeof(CHOOSECOLOR));

    size_t i;

        COLORREF custColours[16];
    for ( i = 0; i < WXSIZEOF(custColours); i++ )
    {
        if ( m_colourData.GetCustomColour(i).IsOk() )
            custColours[i] = wxColourToRGB(m_colourData.GetCustomColour(i));
        else
            custColours[i] = RGB(255,255,255);
    }

    chooseColorStruct.lStructSize = sizeof(CHOOSECOLOR);
    chooseColorStruct.hwndOwner = hWndParent;
    chooseColorStruct.rgbResult = wxColourToRGB(m_colourData.GetColour());
    chooseColorStruct.lpCustColors = custColours;

    chooseColorStruct.Flags = CC_RGBINIT | CC_ENABLEHOOK;
    chooseColorStruct.lCustData = (LPARAM)this;
    chooseColorStruct.lpfnHook = wxColourDialogHookProc;

    if ( m_colourData.GetChooseFull() )
        chooseColorStruct.Flags |= CC_FULLOPEN;

        if ( !::ChooseColor(&chooseColorStruct) )
    {
                        const DWORD err = CommDlgExtendedError();
        if ( err )
        {
            wxLogError(_("Colour selection dialog failed with error %0lx."), err);
        }

        return wxID_CANCEL;
    }


        for ( i = 0; i < WXSIZEOF(custColours); i++ )
    {
      wxRGBToColour(m_colourData.m_custColours[i], custColours[i]);
    }

    wxRGBToColour(m_colourData.GetColour(), chooseColorStruct.rgbResult);

                
    return wxID_OK;
}


void wxColourDialog::SetTitle(const wxString& title)
{
    m_title = title;
}

wxString wxColourDialog::GetTitle() const
{
    return m_title;
}


void wxColourDialog::DoGetPosition(int *x, int *y) const
{
    if ( x )
        *x = gs_rectDialog.x;
    if ( y )
        *y = gs_rectDialog.y;
}

void wxColourDialog::DoCentre(int dir)
{
    m_centreDir = dir;

        }

void wxColourDialog::DoMoveWindow(int x, int y, int WXUNUSED(w), int WXUNUSED(h))
{
    gs_rectDialog.x = x;
    gs_rectDialog.y = y;

            HWND hwnd = GetHwnd();
    if ( hwnd )
    {
                       ::SetWindowPos(hwnd, HWND_TOP, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }
    else     {
        m_movedWindow = true;

                        m_centreDir = 0;
    }
}

void wxColourDialog::DoGetSize(int *width, int *height) const
{
    if ( width )
        *width = gs_rectDialog.width;
    if ( height )
        *height = gs_rectDialog.height;
}

void wxColourDialog::DoGetClientSize(int *width, int *height) const
{
    if ( width )
        *width = gs_rectDialog.width;
    if ( height )
        *height = gs_rectDialog.height;
}

void wxColourDialog::MSWOnInitDone(WXHWND hDlg)
{
        SetHWND(hDlg);

    if ( m_centreDir )
    {
                RECT rect;
        ::GetWindowRect((HWND)hDlg, &rect);
        gs_rectDialog = wxRectFromRECT(rect);

                        wxDialog::DoCentre(m_centreDir);
    }
    else if ( m_movedWindow )     {
        SetPosition(GetPosition());
    }

        SetHWND(NULL);
}

#endif 