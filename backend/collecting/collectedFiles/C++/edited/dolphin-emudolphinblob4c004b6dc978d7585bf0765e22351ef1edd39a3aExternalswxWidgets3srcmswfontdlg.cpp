


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_FONTDLG

#include "wx/fontdlg.h"
#include "wx/modalhook.h"

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcdlg.h"
    #include "wx/utils.h"
    #include "wx/dialog.h"
    #include "wx/log.h"
    #include "wx/math.h"
#endif

#include <stdlib.h>
#include <string.h>


wxIMPLEMENT_DYNAMIC_CLASS(wxFontDialog, wxDialog);



int wxFontDialog::ShowModal()
{
    WX_HOOK_MODAL_DIALOG();

    wxWindow* const parent = GetParentForModalDialog(m_parent, GetWindowStyle());
    WXHWND hWndParent = parent ? GetHwndOf(parent) : NULL;
        DWORD flags = CF_SCREENFONTS  ;

    LOGFONT logFont;

    CHOOSEFONT chooseFontStruct;
    wxZeroMemory(chooseFontStruct);

    chooseFontStruct.lStructSize = sizeof(CHOOSEFONT);
    chooseFontStruct.hwndOwner = hWndParent;
    chooseFontStruct.lpLogFont = &logFont;

    if ( m_fontData.m_initialFont.IsOk() )
    {
        flags |= CF_INITTOLOGFONTSTRUCT;
        wxFillLogFont(&logFont, &m_fontData.m_initialFont);
    }

    if ( m_fontData.m_fontColour.IsOk() )
    {
        chooseFontStruct.rgbColors = wxColourToRGB(m_fontData.m_fontColour);
    }

        if ( !m_fontData.GetAllowSymbols() )
    {
      flags |= CF_SELECTSCRIPT;
      logFont.lfCharSet = ANSI_CHARSET;
    }

    if ( m_fontData.GetEnableEffects() )
      flags |= CF_EFFECTS;
    if ( m_fontData.GetShowHelp() )
      flags |= CF_SHOWHELP;

    if ( m_fontData.m_minSize != 0 || m_fontData.m_maxSize != 0 )
    {
        chooseFontStruct.nSizeMin = m_fontData.m_minSize;
        chooseFontStruct.nSizeMax = m_fontData.m_maxSize;
        flags |= CF_LIMITSIZE;
    }

    chooseFontStruct.Flags = flags;

    if ( ChooseFont(&chooseFontStruct) != 0 )
    {
        wxRGBToColour(m_fontData.m_fontColour, chooseFontStruct.rgbColors);
        m_fontData.m_chosenFont = wxCreateFontFromLogFont(&logFont);
        m_fontData.EncodingInfo().facename = logFont.lfFaceName;
        m_fontData.EncodingInfo().charset = logFont.lfCharSet;

        return wxID_OK;
    }
    else
    {
        DWORD dwErr = CommDlgExtendedError();
        if ( dwErr != 0 )
        {
            wxLogError(_("Common dialog failed with error code %0lx."), dwErr);
        }
        
        return wxID_CANCEL;
    }
}

#endif 