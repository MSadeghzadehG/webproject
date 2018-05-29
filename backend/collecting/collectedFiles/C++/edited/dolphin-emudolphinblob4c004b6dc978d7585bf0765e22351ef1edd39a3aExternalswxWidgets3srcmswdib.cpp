




#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_WXDIB

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/log.h"
    #include "wx/intl.h"
    #include "wx/bitmap.h"
    #include "wx/image.h"
#endif 
#include "wx/file.h"

#include <stdio.h>
#include <stdlib.h>

#include <memory.h>

#include "wx/msw/dib.h"


static inline WORD GetNumberOfColours(WORD bitsPerPixel)
{
            return (WORD)(bitsPerPixel <= 8 ? 1 << bitsPerPixel : 0);
}

static inline bool GetDIBSection(HBITMAP hbmp, DIBSECTION *ds)
{
                    return ::GetObject(hbmp, sizeof(DIBSECTION), ds) == sizeof(DIBSECTION) &&
                ds->dsBm.bmBits;
}



bool wxDIB::Create(int width, int height, int depth)
{
            wxASSERT_MSG( depth, wxT("invalid image depth in wxDIB::Create()") );
    if ( depth < 24 )
        depth = 24;

        BITMAPINFO info;
    wxZeroMemory(info);

    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;

                        info.bmiHeader.biHeight = height;

    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = (WORD)depth;
    info.bmiHeader.biSizeImage = GetLineSize(width, depth)*height;

    m_handle = ::CreateDIBSection
                 (
                    0,                                  &info,                              DIB_RGB_COLORS,                     &m_data,                            NULL,                               0                                );

    if ( !m_handle )
    {
        wxLogLastError(wxT("CreateDIBSection"));

        return false;
    }

    m_width = width;
    m_height = height;
    m_depth = depth;

    return true;
}

bool wxDIB::Create(HBITMAP hbmp)
{
    wxCHECK_MSG( hbmp, false, wxT("wxDIB::Create(): invalid bitmap") );

            DIBSECTION ds;
    if ( GetDIBSection(hbmp, &ds) )
    {
        m_handle = hbmp;

                m_ownsHandle = false;

                m_width = ds.dsBm.bmWidth;
        m_height = ds.dsBm.bmHeight;
        m_depth = ds.dsBm.bmBitsPixel;

        m_data = ds.dsBm.bmBits;
    }
    else     {
                BITMAP bm;
        if ( !::GetObject(hbmp, sizeof(bm), &bm) )
        {
            wxLogLastError(wxT("GetObject(bitmap)"));

            return false;
        }

        int d = bm.bmBitsPixel;
        if ( d <= 0 )
            d = wxDisplayDepth();

        if ( !Create(bm.bmWidth, bm.bmHeight, d) || !CopyFromDDB(hbmp) )
            return false;
    }

    return true;
}

bool wxDIB::CopyFromDDB(HBITMAP hbmp)
{
    DIBSECTION ds;
    if ( !GetDIBSection(m_handle, &ds) )
    {
                wxFAIL_MSG( wxT("GetObject(DIBSECTION) unexpectedly failed") );

        return false;
    }

    if ( !::GetDIBits
            (
                ScreenHDC(),                                hbmp,                                       0,                                          m_height,                                   ds.dsBm.bmBits,                             (BITMAPINFO *)&ds.dsBmih,                   DIB_RGB_COLORS                          ) )
    {
        wxLogLastError(wxT("GetDIBits()"));

        return false;
    }

    return true;
}


bool wxDIB::Load(const wxString& filename)
{
    m_handle = (HBITMAP)::LoadImage
                         (
                            wxGetInstance(),
                            filename.t_str(),
                            IMAGE_BITMAP,
                            0, 0,                             LR_CREATEDIBSECTION | LR_LOADFROMFILE
                         );

    if ( !m_handle )
    {
        wxLogLastError(wxT("Loading DIB from file"));

        return false;
    }

    return true;
}

bool wxDIB::Save(const wxString& filename)
{
    wxCHECK_MSG( m_handle, false, wxT("wxDIB::Save(): invalid object") );

#if wxUSE_FILE
    wxFile file(filename, wxFile::write);
    bool ok = file.IsOpened();
    if ( ok )
    {
        DIBSECTION ds;
        if ( !GetDIBSection(m_handle, &ds) )
        {
            wxLogLastError(wxT("GetObject(hDIB)"));
        }
        else
        {
            BITMAPFILEHEADER bmpHdr;
            wxZeroMemory(bmpHdr);

            const size_t sizeHdr = ds.dsBmih.biSize;
            const size_t sizeImage = ds.dsBmih.biSizeImage;

            bmpHdr.bfType = 0x4d42;                bmpHdr.bfOffBits = sizeof(BITMAPFILEHEADER) + ds.dsBmih.biSize;
            bmpHdr.bfSize = bmpHdr.bfOffBits + sizeImage;

                                    ok = file.Write(&bmpHdr, sizeof(bmpHdr)) == sizeof(bmpHdr) &&
                    file.Write(&ds.dsBmih, sizeHdr) == sizeHdr &&
                        file.Write(ds.dsBm.bmBits, sizeImage) == sizeImage;
        }
    }
#else     bool ok = false;
#endif 
    if ( !ok )
    {
        wxLogError(_("Failed to save the bitmap image to file \"%s\"."),
                   filename.c_str());
    }

    return ok;
}


void wxDIB::DoGetObject() const
{
            if ( m_handle && !m_data )
    {
                                                DIBSECTION ds;
        if ( !GetDIBSection(m_handle, &ds) )
        {
            wxLogLastError(wxT("GetObject(hDIB)"));
            return;
        }

        wxDIB *self = wxConstCast(this, wxDIB);

        self->m_width = ds.dsBm.bmWidth;
        self->m_height = ds.dsBm.bmHeight;
        self->m_depth = ds.dsBm.bmBitsPixel;
        self->m_data = ds.dsBm.bmBits;
    }
}


HBITMAP wxDIB::CreateDDB(HDC hdc) const
{
    wxCHECK_MSG( m_handle, 0, wxT("wxDIB::CreateDDB(): invalid object") );

    DIBSECTION ds;
    if ( !GetDIBSection(m_handle, &ds) )
    {
        wxLogLastError(wxT("GetObject(hDIB)"));

        return 0;
    }

        DWORD biClrUsed = ds.dsBmih.biClrUsed;
    if ( !biClrUsed )
    {
                biClrUsed = GetNumberOfColours(ds.dsBmih.biBitCount);
    }

    if ( !biClrUsed )
    {
        return ConvertToBitmap((BITMAPINFO *)&ds.dsBmih, hdc, ds.dsBm.bmBits);
    }
    else
    {
                wxCharBuffer bmi(sizeof(BITMAPINFO) + (biClrUsed - 1)*sizeof(RGBQUAD));
        BITMAPINFO *pBmi = (BITMAPINFO *)bmi.data();
        MemoryHDC hDC;
                SelectInHDC sDC(hDC, m_handle);
        ::GetDIBColorTable(hDC, 0, biClrUsed, pBmi->bmiColors);
        memcpy(&pBmi->bmiHeader, &ds.dsBmih, ds.dsBmih.biSize);

        return ConvertToBitmap(pBmi, hdc, ds.dsBm.bmBits);
    }
}


HBITMAP wxDIB::ConvertToBitmap(const BITMAPINFO *pbmi, HDC hdc, void *bits)
{
    wxCHECK_MSG( pbmi, 0, wxT("invalid DIB in ConvertToBitmap") );

            const BITMAPINFOHEADER *pbmih = &pbmi->bmiHeader;

                if ( !bits )
    {
                                                        int numColors;
        switch ( pbmih->biCompression )
        {
            case BI_BITFIELDS:
                numColors = 3;
                break;

            case BI_RGB:
                                                numColors = pbmih->biClrUsed;
                if ( !numColors )
                {
                    numColors = GetNumberOfColours(pbmih->biBitCount);
                }
                break;

            default:
                                numColors = 0;
        }

        bits = (char *)pbmih + sizeof(*pbmih) + numColors*sizeof(RGBQUAD);
    }

    HBITMAP hbmp = ::CreateDIBitmap
                     (
                        hdc ? hdc                                       : (HDC) ScreenHDC(),                          pbmih,                                      CBM_INIT,                                   bits,                                       pbmi,                                       DIB_RGB_COLORS                           );

    if ( !hbmp )
    {
        wxLogLastError(wxT("CreateDIBitmap"));
    }

    return hbmp;
}


size_t wxDIB::ConvertFromBitmap(BITMAPINFO *pbi, HBITMAP hbmp)
{
    wxASSERT_MSG( hbmp, wxT("invalid bmp can't be converted to DIB") );

        BITMAP bm;
    if ( !::GetObject(hbmp, sizeof(bm), &bm) )
    {
        wxLogLastError(wxT("GetObject(bitmap)"));

        return 0;
    }

            BITMAPINFO bi2;

    const bool wantSizeOnly = pbi == NULL;
    if ( wantSizeOnly )
        pbi = &bi2;

        const int h = bm.bmHeight;

        BITMAPINFOHEADER& bi = pbi->bmiHeader;
    wxZeroMemory(bi);
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bm.bmWidth;
    bi.biHeight = h;
    bi.biPlanes = 1;
    bi.biBitCount = bm.bmBitsPixel;

        DWORD dwLen = bi.biSize + GetNumberOfColours(bm.bmBitsPixel) * sizeof(RGBQUAD);

        if ( !::GetDIBits
            (
                ScreenHDC(),                                        hbmp,                                               0,                                                  h,                                                  wantSizeOnly ? NULL                                              : (char *)pbi + dwLen,                 pbi,                                                DIB_RGB_COLORS                                  ) )
    {
        wxLogLastError(wxT("GetDIBits()"));

        return 0;
    }

        return dwLen + bi.biSizeImage;
}


HGLOBAL wxDIB::ConvertFromBitmap(HBITMAP hbmp)
{
        const size_t size = ConvertFromBitmap(NULL, hbmp);
    if ( !size )
    {
                return NULL;
    }

    HGLOBAL hDIB = ::GlobalAlloc(GMEM_MOVEABLE, size);
    if ( !hDIB )
    {
                        wxLogError(_("Failed to allocate %luKb of memory for bitmap data."),
                   (unsigned long)(size / 1024));

        return NULL;
    }

    if ( !ConvertFromBitmap((BITMAPINFO *)(void *)GlobalPtrLock(hDIB), hbmp) )
    {
                        wxFAIL_MSG( wxT("wxDIB::ConvertFromBitmap() unexpectedly failed") );

        return NULL;
    }

    return hDIB;
}


#if defined(__WXMSW__) && wxUSE_PALETTE

wxPalette *wxDIB::CreatePalette() const
{
    wxCHECK_MSG( m_handle, NULL, wxT("wxDIB::CreatePalette(): invalid object") );

    DIBSECTION ds;
    if ( !GetDIBSection(m_handle, &ds) )
    {
        wxLogLastError(wxT("GetObject(hDIB)"));

        return 0;
    }

        DWORD biClrUsed = ds.dsBmih.biClrUsed;
    if ( !biClrUsed )
    {
                biClrUsed = GetNumberOfColours(ds.dsBmih.biBitCount);
    }

    if ( !biClrUsed )
    {
                                        return NULL;
    }

    MemoryHDC hDC;

            LOGPALETTE *pPalette = (LOGPALETTE *)
        malloc(sizeof(LOGPALETTE) + (biClrUsed - 1)*sizeof(PALETTEENTRY));
    wxCHECK_MSG( pPalette, NULL, wxT("out of memory") );

        pPalette->palVersion = 0x300;      pPalette->palNumEntries = (WORD)biClrUsed;

        wxCharBuffer rgb(sizeof(RGBQUAD) * biClrUsed);
    RGBQUAD *pRGB = (RGBQUAD*)rgb.data();
    SelectInHDC selectHandle(hDC, m_handle);
    ::GetDIBColorTable(hDC, 0, biClrUsed, pRGB);
    for ( DWORD i = 0; i < biClrUsed; i++, pRGB++ )
    {
        pPalette->palPalEntry[i].peRed = pRGB->rgbRed;
        pPalette->palPalEntry[i].peGreen = pRGB->rgbGreen;
        pPalette->palPalEntry[i].peBlue = pRGB->rgbBlue;
        pPalette->palPalEntry[i].peFlags = 0;
    }

    HPALETTE hPalette = ::CreatePalette(pPalette);

    free(pPalette);

    if ( !hPalette )
    {
        wxLogLastError(wxT("CreatePalette"));

        return NULL;
    }

    wxPalette *palette = new wxPalette;
    palette->SetHPALETTE((WXHPALETTE)hPalette);

    return palette;
}

#endif 

#if wxUSE_IMAGE

bool wxDIB::Create(const wxImage& image, PixelFormat pf)
{
    wxCHECK_MSG( image.IsOk(), false, wxT("invalid wxImage in wxDIB ctor") );

    const int h = image.GetHeight();
    const int w = image.GetWidth();

            const bool hasAlpha = image.HasAlpha();
    const int bpp = hasAlpha ? 32 : 24;

    if ( !Create(w, h, bpp) )
        return false;

            const int srcBytesPerLine = w * 3;
    const int dstBytesPerLine = GetLineSize(w, bpp);
    const unsigned char *src = image.GetData() + ((h - 1) * srcBytesPerLine);
    const unsigned char *alpha = hasAlpha ? image.GetAlpha() + (h - 1)*w
                                          : NULL;
    unsigned char *dstLineStart = (unsigned char *)m_data;
    for ( int y = 0; y < h; y++ )
    {
                        unsigned char *dst = dstLineStart;
        if ( alpha )
        {
            int x;

            switch ( pf )
            {
                case PixelFormat_PreMultiplied:
                                                            for ( x = 0; x < w; x++ )
                    {
                        const unsigned char a = *alpha++;
                        *dst++ = (unsigned char)((src[2] * a + 127) / 255);
                        *dst++ = (unsigned char)((src[1] * a + 127) / 255);
                        *dst++ = (unsigned char)((src[0] * a + 127) / 255);
                        *dst++ = a;
                        src += 3;
                    }
                    break;

                case PixelFormat_NotPreMultiplied:
                                        for ( x = 0; x < w; x++ )
                    {
                        *dst++ = src[2];
                        *dst++ = src[1];
                        *dst++ = src[0];

                        *dst++ = *alpha++;
                        src += 3;
                    }
                    break;
            }

        }
        else         {
            for ( int x = 0; x < w; x++ )
            {
                *dst++ = src[2];
                *dst++ = src[1];
                *dst++ = src[0];
                src += 3;
            }
        }

                src -= 2*srcBytesPerLine;
        if ( alpha )
            alpha -= 2*w;

                dstLineStart += dstBytesPerLine;
    }

    return true;
}

wxImage wxDIB::ConvertToImage(ConversionFlags flags) const
{
    wxCHECK_MSG( IsOk(), wxNullImage,
                    wxT("can't convert invalid DIB to wxImage") );

        const int w = GetWidth();
    const int h = GetHeight();
    wxImage image(w, h, false );
    if ( !image.IsOk() )
    {
        wxFAIL_MSG( wxT("could not allocate data for image") );
        return wxNullImage;
    }

    const int bpp = GetDepth();

                bool hasAlpha = false;
    bool hasOpaque = false;
    bool hasTransparent = false;

    if ( bpp == 32 )
    {
                                image.SetAlpha();
    }

            const int dstBytesPerLine = w * 3;
    const int srcBytesPerLine = GetLineSize(w, bpp);
    unsigned char *dst = image.GetData() + ((h - 1) * dstBytesPerLine);
    unsigned char *alpha = image.HasAlpha() ? image.GetAlpha() + (h - 1)*w
                                            : NULL;
    const unsigned char *srcLineStart = (unsigned char *)GetData();
    for ( int y = 0; y < h; y++ )
    {
                const unsigned char *src = srcLineStart;
        for ( int x = 0; x < w; x++ )
        {
            dst[2] = *src++;
            dst[1] = *src++;
            dst[0] = *src++;

            if ( bpp == 32 )
            {
                                                const unsigned char a = *src;
                *alpha++ = a;

                                switch ( a )
                {
                    case 0:
                        hasTransparent = true;
                        break;

                    default:
                                                                        hasAlpha = true;
                        break;

                    case 255:
                        hasOpaque = true;
                        break;
                }

                if ( a > 0 )
                {
                    dst[0] = (dst[0] * 255) / a;
                    dst[1] = (dst[1] * 255) / a;
                    dst[2] = (dst[2] * 255) / a;
                }

                src++;
            }

            dst += 3;
        }

                dst -= 2*dstBytesPerLine;
        if ( alpha )
            alpha -= 2*w;

                srcLineStart += srcBytesPerLine;
    }

    if ( hasOpaque && hasTransparent )
        hasAlpha = true;

    if ( !hasAlpha && image.HasAlpha() && flags == Convert_AlphaAuto )
        image.ClearAlpha();

    return image;
}

#endif 
#endif 