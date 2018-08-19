
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_BMPBUTTON

#include "wx/bmpbuttn.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/dcmemory.h"
    #include "wx/image.h"
#endif

#include "wx/msw/private.h"
#include "wx/msw/dc.h"          
#include "wx/msw/uxtheme.h"

#if wxUSE_UXTHEME
        #ifndef BP_PUSHBUTTON
        #define BP_PUSHBUTTON 1

        #define PBS_NORMAL    1
        #define PBS_HOT       2
        #define PBS_PRESSED   3
        #define PBS_DISABLED  4
        #define PBS_DEFAULTED 5

        #define TMT_CONTENTMARGINS 3602
    #endif
#endif 
#ifndef ODS_NOFOCUSRECT
    #define ODS_NOFOCUSRECT     0x0200
#endif


wxBEGIN_EVENT_TABLE(wxBitmapButton, wxBitmapButtonBase)
    EVT_SYS_COLOUR_CHANGED(wxBitmapButton::OnSysColourChanged)
wxEND_EVENT_TABLE()



bool wxBitmapButton::Create(wxWindow *parent,
                            wxWindowID id,
                            const wxBitmap& bitmap,
                            const wxPoint& pos,
                            const wxSize& size, long style,
                            const wxValidator& validator,
                            const wxString& name)
{
    if ( !wxBitmapButtonBase::Create(parent, id, pos, size, style,
                                     validator, name) )
        return false;

    if ( bitmap.IsOk() )
        SetBitmapLabel(bitmap);

    if ( !size.IsFullySpecified() )
    {
                        SetInitialSize(size);
    }

    return true;
}

#endif 