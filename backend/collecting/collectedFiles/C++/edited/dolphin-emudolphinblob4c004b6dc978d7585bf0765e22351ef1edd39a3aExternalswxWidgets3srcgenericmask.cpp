


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/bitmap.h"
    #include "wx/image.h"
#endif 
#if wxUSE_GENERIC_MASK


wxIMPLEMENT_DYNAMIC_CLASS(wxMask, wxObject);

void wxMask::FreeData()
{
    m_bitmap = wxNullBitmap;
}

bool wxMask::InitFromColour(const wxBitmap& bitmap, const wxColour& colour)
{
#if wxUSE_IMAGE
    const wxColour clr(bitmap.QuantizeColour(colour));

    wxImage imgSrc(bitmap.ConvertToImage());
    imgSrc.SetMask(false);
    wxImage image(imgSrc.ConvertToMono(clr.Red(), clr.Green(), clr.Blue()));
    if ( !image.IsOk() )
        return false;

    m_bitmap = wxBitmap(image, 1);

    return m_bitmap.IsOk();
#else     wxUnusedVar(bitmap);
    wxUnusedVar(colour);

    return false;
#endif }

bool wxMask::InitFromMonoBitmap(const wxBitmap& bitmap)
{
    wxCHECK_MSG( bitmap.IsOk(), false, wxT("Invalid bitmap") );
    wxCHECK_MSG( bitmap.GetDepth() == 1, false, wxT("Cannot create mask from colour bitmap") );

    m_bitmap = bitmap;

    return true;
}

#endif 