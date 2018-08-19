
#include "wx/wxprec.h"

#include "wx/colordlg.h"
#include "wx/fontdlg.h"
#include "wx/modalhook.h"


#if !USE_NATIVE_FONT_DIALOG_FOR_MACOSX && wxUSE_COLOURDLG

wxIMPLEMENT_DYNAMIC_CLASS(wxColourDialog, wxDialog);

#include "wx/osx/private.h"



wxColourDialog::wxColourDialog()
{
    m_dialogParent = NULL;
}

wxColourDialog::wxColourDialog(wxWindow *parent, wxColourData *data)
{
    Create(parent, data);
}

bool wxColourDialog::Create(wxWindow *parent, wxColourData *data)
{
    m_dialogParent = parent;

    if (data)
        m_colourData = *data;
    return true;
}

int wxColourDialog::ShowModal()
{
    WX_HOOK_MODAL_DIALOG();

    RGBColor currentColor ;

    m_colourData.m_dataColour.GetRGBColor( &currentColor );
    NColorPickerInfo info;
    OSStatus err ;
    memset(&info, 0, sizeof(info)) ;
        info.placeWhere = kCenterOnMainScreen ;
    info.flags = kColorPickerDialogIsMoveable | kColorPickerDialogIsModal ;
    info.theColor.color.rgb.red =  currentColor.red ;
    info.theColor.color.rgb.green =  currentColor.green ;
    info.theColor.color.rgb.blue =  currentColor.blue ;
    wxDialog::OSXBeginModalDialog();
    err = NPickColor(&info);
    wxDialog::OSXEndModalDialog();
    if ((err == noErr) && info.newColorChosen)
    {
        currentColor.red = info.theColor.color.rgb.red ;
        currentColor.green = info.theColor.color.rgb.green ;
        currentColor.blue = info.theColor.color.rgb.blue ;
        m_colourData.m_dataColour = currentColor;

        return wxID_OK;
    }
    return wxID_CANCEL;
}

#endif
