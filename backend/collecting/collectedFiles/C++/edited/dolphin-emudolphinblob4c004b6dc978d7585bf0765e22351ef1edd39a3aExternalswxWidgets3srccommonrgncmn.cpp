

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/region.h"

#ifndef WX_PRECOMP
    #include "wx/dcmemory.h"
    #include "wx/bitmap.h"
    #include "wx/image.h"
#endif 


bool wxRegionBase::IsEqual(const wxRegion& region) const
{
    if ( m_refData == region.GetRefData() )
    {
                return true;
    }

    if ( !m_refData || !region.GetRefData() )
    {
                return false;
    }

    return DoIsEqual(region);
}


wxBitmap wxRegionBase::ConvertToBitmap() const
{
    wxRect box = GetBox();
    wxBitmap bmp(box.GetRight() + 1, box.GetBottom() + 1);
    wxMemoryDC dc;
    dc.SelectObject(bmp);
    dc.SetBackground(*wxBLACK_BRUSH);
    dc.Clear();
    dc.SetDeviceClippingRegion(*static_cast<const wxRegion *>(this));
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    dc.SelectObject(wxNullBitmap);
    return bmp;
}

#if wxUSE_IMAGE

static bool DoRegionUnion(wxRegionBase& region,
                          const wxImage& image,
                          unsigned char loR,
                          unsigned char loG,
                          unsigned char loB,
                          int tolerance)
{
    unsigned char hiR, hiG, hiB;

    hiR = (unsigned char)wxMin(0xFF, loR + tolerance);
    hiG = (unsigned char)wxMin(0xFF, loG + tolerance);
    hiB = (unsigned char)wxMin(0xFF, loB + tolerance);

            int width = image.GetWidth();
    int height = image.GetHeight();
    for (int y=0; y < height; y++)
    {
        wxRect rect;
        rect.y = y;
        rect.height = 1;

        for (int x=0; x < width; x++)
        {
                        int x0 = x;
            while ( x < width)
            {
                unsigned char R = image.GetRed(x,y);
                unsigned char G = image.GetGreen(x,y);
                unsigned char B = image.GetBlue(x,y);
                if (( R >= loR && R <= hiR) &&
                    ( G >= loG && G <= hiG) &&
                    ( B >= loB && B <= hiB))                      break;
                x++;
            }

                        if (x > x0) {
                rect.x = x0;
                rect.width = x - x0;
                region.Union(rect);
            }
        }
    }

    return true;
}


bool wxRegionBase::Union(const wxBitmap& bmp)
{
    if (bmp.GetMask())
    {
        wxImage image = bmp.ConvertToImage();
        wxASSERT_MSG( image.HasMask(), wxT("wxBitmap::ConvertToImage doesn't preserve mask?") );
        return DoRegionUnion(*this, image,
                             image.GetMaskRed(),
                             image.GetMaskGreen(),
                             image.GetMaskBlue(),
                             0);
    }
    else
    {
        return Union(0, 0, bmp.GetWidth(), bmp.GetHeight());
    }
}

bool wxRegionBase::Union(const wxBitmap& bmp,
                         const wxColour& transColour,
                         int   tolerance)
{
    wxImage image = bmp.ConvertToImage();
    return DoRegionUnion(*this, image,
                         transColour.Red(),
                         transColour.Green(),
                         transColour.Blue(),
                         tolerance);
}

#endif 
#ifdef wxHAS_REGION_COMBINE

bool wxRegionWithCombine::DoUnionWithRect(const wxRect& rect)
{
    return Combine(rect, wxRGN_OR);
}

bool wxRegionWithCombine::DoUnionWithRegion(const wxRegion& region)
{
    return DoCombine(region, wxRGN_OR);
}

bool wxRegionWithCombine::DoIntersect(const wxRegion& region)
{
    return DoCombine(region, wxRGN_AND);
}

bool wxRegionWithCombine::DoSubtract(const wxRegion& region)
{
    return DoCombine(region, wxRGN_DIFF);
}

bool wxRegionWithCombine::DoXor(const wxRegion& region)
{
    return DoCombine(region, wxRGN_XOR);
}

#endif 