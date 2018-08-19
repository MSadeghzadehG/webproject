
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/icon.h"


wxIMPLEMENT_DYNAMIC_CLASS(wxIcon, wxBitmap);

wxIcon::wxIcon(const char* const* bits) :
    wxBitmap( bits )
{
}

#ifdef wxNEEDS_CHARPP
wxIcon::wxIcon(char **bits) :
    wxBitmap( bits )
{
}
#endif

wxIcon::wxIcon() :  wxBitmap()
{
}

void wxIcon::CopyFromBitmap(const wxBitmap& bmp)
{
    wxIcon *icon = (wxIcon*)(&bmp);
    *this = *icon;
}
