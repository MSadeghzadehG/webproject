

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_COMMANDLINKBUTTON

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/dcclient.h"
#endif

#include "wx/commandlinkbutton.h"
#include "wx/msw/private.h"
#include "wx/msw/private/button.h"
#include "wx/msw/private/dc.h"
#include "wx/private/window.h"

#ifndef BCM_SETNOTE
    #define BCM_SETNOTE 0x1609
#endif

#ifndef BS_COMMANDLINK
    #define BS_COMMANDLINK 0xE
#endif


namespace
{

inline bool HasNativeCommandLinkButton()
{
    return wxGetWinVersion() >= wxWinVersion_6;
}

} 

bool wxCommandLinkButton::Create(wxWindow *parent,
                                 wxWindowID id,
                                 const wxString& mainLabel,
                                 const wxString& note,
                                 const wxPoint& pos,
                                 const wxSize& size,
                                 long style,
                                 const wxValidator& validator,
                                 const wxString& name)
{
    if ( ! wxGenericCommandLinkButton::Create(parent,
                                              id,
                                              mainLabel,
                                              note,
                                              pos,
                                              size,
                                              style,
                                              validator,
                                              name) )
        return false;

    SetMainLabelAndNote(mainLabel, note);
    SetInitialSize();

    return true;
}

void
wxCommandLinkButton::SetMainLabelAndNote(const wxString& mainLabel,
                                         const wxString& note)
{
    if ( HasNativeCommandLinkButton() )
    {
        wxButton::SetLabel(mainLabel);
        ::SendMessage(m_hWnd, BCM_SETNOTE, 0, wxMSW_CONV_LPARAM(note));

                m_labelOrig = mainLabel;
        if ( !note.empty() )
            m_labelOrig << '\n' << note;
    }
    else
    {
        wxGenericCommandLinkButton::SetMainLabelAndNote(mainLabel, note);
    }
}

WXDWORD wxCommandLinkButton::MSWGetStyle(long style, WXDWORD *exstyle) const
{
    WXDWORD ret = wxButton::MSWGetStyle(style, exstyle);
    if ( HasNativeCommandLinkButton() )
        ret |= BS_COMMANDLINK;

    return ret;
}

bool wxCommandLinkButton::HasNativeBitmap() const
{
    return HasNativeCommandLinkButton();
}


namespace
{
    const int MAINLABEL_TOP_MARGIN = 16;     const int MAINLABEL_NOTE_LEFT_MARGIN = 23;
    const int NOTE_TOP_MARGIN = 21;
    const int NOTE_BOTTOM_MARGIN = 1;
    const int MAINLABEL_NOTE_MARGIN = NOTE_TOP_MARGIN - MAINLABEL_TOP_MARGIN;
};

wxSize wxCommandLinkButton::DoGetBestSize() const
{
    wxSize size;

                if ( ShowsLabel() || !m_imageData )
    {
        int flags = 0;
        if ( GetAuthNeeded() )
            flags |= wxMSWButton::Size_AuthNeeded;

        wxCommandLinkButton *thisButton =
            const_cast<wxCommandLinkButton *>(this);
        wxClientDC dc(thisButton);

        wxFont noteFont = dc.GetFont();

                dc.SetFont(noteFont.Scaled(4.0f/3.0f));
        size = dc.GetMultiLineTextExtent(GetLabelText(GetMainLabel()));

        dc.SetFont(noteFont);
        wxSize noteSize = dc.GetMultiLineTextExtent(GetLabelText(GetNote()));

        if ( noteSize.x > size.x )
            size.x = noteSize.x;
        size.y += noteSize.y;

        size = wxMSWButton::GetFittingSize(thisButton,
                                           size,
                                           flags);

                        int heightDef = GetNote().AfterFirst('\n').empty() ? 25 : 35;
        wxSize sizeDef = thisButton->ConvertDialogToPixels(wxSize(50,
                                                                  heightDef));

        if ( size.y < sizeDef.y )
            size.y = sizeDef.y;
    }

    if ( m_imageData )
    {
        AdjustForBitmapSize(size);
    }
    else
    {
                size.x += 16;
        if ( size.y < 16 )
            size.y = 16;
    }

    size.x += MAINLABEL_NOTE_LEFT_MARGIN;
    size.y += MAINLABEL_TOP_MARGIN + NOTE_BOTTOM_MARGIN;
    if ( !GetNote().empty() )
        size.y += MAINLABEL_NOTE_MARGIN;

    CacheBestSize(size);

    return size;
}

#endif 