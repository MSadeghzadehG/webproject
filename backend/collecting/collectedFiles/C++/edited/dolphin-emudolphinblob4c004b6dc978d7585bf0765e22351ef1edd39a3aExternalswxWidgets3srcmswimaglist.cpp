


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/msw/wrapcctl.h"     #include "wx/window.h"
    #include "wx/icon.h"
    #include "wx/dc.h"
    #include "wx/string.h"
    #include "wx/dcmemory.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/image.h"
    #include <stdio.h>
#endif

#include "wx/imaglist.h"
#include "wx/dc.h"
#include "wx/msw/dc.h"
#include "wx/msw/dib.h"
#include "wx/msw/private.h"


wxIMPLEMENT_DYNAMIC_CLASS(wxImageList, wxObject);

#define GetHImageList()     ((HIMAGELIST)m_hImageList)


static HBITMAP GetMaskForImage(const wxBitmap& bitmap, const wxBitmap& mask);



wxImageList::wxImageList()
{
    m_hImageList = 0;
    m_size = wxSize(0,0);
}

bool wxImageList::Create(int width, int height, bool mask, int initial)
{
    m_size = wxSize(width, height);
    UINT flags = 0;

                        flags |= ILC_COLOR32;

        if ( mask || wxApp::GetComCtl32Version() < 600 )
        flags |= ILC_MASK;

        m_hImageList = (WXHIMAGELIST) ImageList_Create(width, height, flags,
                                                   initial, 1);
    if ( !m_hImageList )
    {
        wxLogLastError(wxT("ImageList_Create()"));
    }

    return m_hImageList != 0;
}

wxImageList::~wxImageList()
{
    if ( m_hImageList )
    {
        ImageList_Destroy(GetHImageList());
        m_hImageList = 0;
    }
}


int wxImageList::GetImageCount() const
{
    wxASSERT_MSG( m_hImageList, wxT("invalid image list") );

    return ImageList_GetImageCount(GetHImageList());
}

bool wxImageList::GetSize(int WXUNUSED(index), int &width, int &height) const
{
    wxASSERT_MSG( m_hImageList, wxT("invalid image list") );

    return ImageList_GetIconSize(GetHImageList(), &width, &height) != 0;
}


int wxImageList::Add(const wxBitmap& bitmap, const wxBitmap& mask)
{
    HBITMAP hbmp;
    bool useMask;

#if wxUSE_WXDIB && wxUSE_IMAGE
                        AutoHBITMAP hbmpRelease;
    if ( bitmap.HasAlpha() )
    {
        wxImage img = bitmap.ConvertToImage();

                        if ( wxApp::GetComCtl32Version() < 600 )
        {
            img.ClearAlpha();
            useMask = true;
        }
        else
        {
            useMask = false;
        }

        hbmp = wxDIB(img, wxDIB::PixelFormat_NotPreMultiplied).Detach();
        hbmpRelease.Init(hbmp);
    }
    else
#endif     {
        hbmp = GetHbitmapOf(bitmap);
        useMask = true;
    }

            AutoHBITMAP hbmpMask;
    if ( useMask )
        hbmpMask.Init(GetMaskForImage(bitmap, mask));

    int index = ImageList_Add(GetHImageList(), hbmp, hbmpMask);
    if ( index == -1 )
    {
        wxLogError(_("Couldn't add an image to the image list."));
    }

    return index;
}

int wxImageList::Add(const wxBitmap& bitmap, const wxColour& maskColour)
{
    HBITMAP hbmp;

#if wxUSE_WXDIB && wxUSE_IMAGE
        AutoHBITMAP hbmpRelease;
    if ( bitmap.HasAlpha() )
    {
        wxImage img = bitmap.ConvertToImage();

        if ( wxApp::GetComCtl32Version() < 600 )
        {
            img.ClearAlpha();
        }

        hbmp = wxDIB(img, wxDIB::PixelFormat_NotPreMultiplied).Detach();
        hbmpRelease.Init(hbmp);
    }
    else
#endif         hbmp = GetHbitmapOf(bitmap);

    int index = ImageList_AddMasked(GetHImageList(),
                                    hbmp,
                                    wxColourToRGB(maskColour));
    if ( index == -1 )
    {
        wxLogError(_("Couldn't add an image to the image list."));
    }

    return index;
}

int wxImageList::Add(const wxIcon& icon)
{
                    if ( wxApp::GetComCtl32Version() < 600 )
    {
        wxBitmap bmp(icon);
        if ( bmp.HasAlpha() )
        {
            return Add(bmp);
        }
    }

    int index = ImageList_AddIcon(GetHImageList(), GetHiconOf(icon));
    if ( index == -1 )
    {
        wxLogError(_("Couldn't add an image to the image list."));
    }

    return index;
}

bool wxImageList::Replace(int index,
                          const wxBitmap& bitmap,
                          const wxBitmap& mask)
{
    HBITMAP hbmp;
    bool useMask;

#if wxUSE_WXDIB && wxUSE_IMAGE
        AutoHBITMAP hbmpRelease;
    if ( bitmap.HasAlpha() )
    {
        wxImage img = bitmap.ConvertToImage();

        if ( wxApp::GetComCtl32Version() < 600 )
        {
            img.ClearAlpha();
            useMask = true;
        }
        else
        {
            useMask = false;
        }

        hbmp = wxDIB(img, wxDIB::PixelFormat_NotPreMultiplied).Detach();
        hbmpRelease.Init(hbmp);
    }
    else
#endif     {
        hbmp = GetHbitmapOf(bitmap);
        useMask = true;
    }

    AutoHBITMAP hbmpMask;
    if ( useMask )
        hbmpMask.Init(GetMaskForImage(bitmap, mask));

    if ( !ImageList_Replace(GetHImageList(), index, hbmp, hbmpMask) )
    {
        wxLogLastError(wxT("ImageList_Replace()"));
        return false;
    }

    return true;
}

bool wxImageList::Replace(int i, const wxIcon& icon)
{
                    if ( wxApp::GetComCtl32Version() < 600 )
    {
        wxBitmap bmp(icon);
        if ( bmp.HasAlpha() )
        {
            return Replace(i, bmp);
        }
    }

    bool ok = ImageList_ReplaceIcon(GetHImageList(), i, GetHiconOf(icon)) != -1;
    if ( !ok )
    {
        wxLogLastError(wxT("ImageList_ReplaceIcon()"));
    }

    return ok;
}

bool wxImageList::Remove(int index)
{
    bool ok = ImageList_Remove(GetHImageList(), index) != 0;
    if ( !ok )
    {
        wxLogLastError(wxT("ImageList_Remove()"));
    }

    return ok;
}

bool wxImageList::RemoveAll()
{
        return Remove(-1);
}

bool wxImageList::Draw(int index,
                       wxDC& dc,
                       int x, int y,
                       int flags,
                       bool solidBackground)
{
    wxDCImpl *impl = dc.GetImpl();
    wxMSWDCImpl *msw_impl = wxDynamicCast( impl, wxMSWDCImpl );
    if (!msw_impl)
       return false;

    HDC hDC = GetHdcOf(*msw_impl);
    wxCHECK_MSG( hDC, false, wxT("invalid wxDC in wxImageList::Draw") );

    COLORREF clr = CLR_NONE;        if ( solidBackground )
    {
        const wxBrush& brush = dc.GetBackground();
        if ( brush.IsOk() )
        {
            clr = wxColourToRGB(brush.GetColour());
        }
    }

    ImageList_SetBkColor(GetHImageList(), clr);

    UINT style = 0;
    if ( flags & wxIMAGELIST_DRAW_NORMAL )
        style |= ILD_NORMAL;
    if ( flags & wxIMAGELIST_DRAW_TRANSPARENT )
        style |= ILD_TRANSPARENT;
    if ( flags & wxIMAGELIST_DRAW_SELECTED )
        style |= ILD_SELECTED;
    if ( flags & wxIMAGELIST_DRAW_FOCUSED )
        style |= ILD_FOCUS;

    bool ok = ImageList_Draw(GetHImageList(), index, hDC, x, y, style) != 0;
    if ( !ok )
    {
        wxLogLastError(wxT("ImageList_Draw()"));
    }

    return ok;
}

wxBitmap wxImageList::GetBitmap(int index) const
{
    int bmp_width = 0, bmp_height = 0;
    GetSize(index, bmp_width, bmp_height);

    wxBitmap bitmap(bmp_width, bmp_height);

#if wxUSE_WXDIB && wxUSE_IMAGE
    wxMemoryDC dc;
    dc.SelectObject(bitmap);

    IMAGEINFO ii;
    ImageList_GetImageInfo(GetHImageList(), index, &ii);
    if ( ii.hbmMask )
    {
                ((wxImageList*)this)->Draw(index, dc, 0, 0, wxIMAGELIST_DRAW_TRANSPARENT);
        dc.SelectObject(wxNullBitmap);

                wxImage image = bitmap.ConvertToImage();
        unsigned char r = 0, g = 0, b = 0;
        image.FindFirstUnusedColour(&r, &g, &b);

                image.Create(bmp_width, bmp_height);
        image.Replace(0, 0, 0, r, g, b);
        bitmap = wxBitmap(image);

                dc.SelectObject(bitmap);
        ((wxImageList*)this)->Draw(index, dc, 0, 0, wxIMAGELIST_DRAW_TRANSPARENT);
        dc.SelectObject(wxNullBitmap);

                image = bitmap.ConvertToImage();
        image.SetMaskColour(r, g, b);
        bitmap = wxBitmap(image);
    }
    else     {
                ((wxImageList*)this)->Draw(index, dc, 0, 0, wxIMAGELIST_DRAW_NORMAL);
        dc.SelectObject(wxNullBitmap);

                                                                bitmap.MSWUpdateAlpha();
    }
#endif
    return bitmap;
}

wxIcon wxImageList::GetIcon(int index) const
{
    HICON hIcon = ImageList_ExtractIcon(0, GetHImageList(), index);
    if (hIcon)
    {
        wxIcon icon;
        icon.SetHICON((WXHICON)hIcon);

        int iconW, iconH;
        GetSize(index, iconW, iconH);
        icon.SetSize(iconW, iconH);

        return icon;
    }
    else
        return wxNullIcon;
}


static HBITMAP GetMaskForImage(const wxBitmap& bitmap, const wxBitmap& mask)
{
#if wxUSE_IMAGE
    wxBitmap bitmapWithMask;
#endif 
    HBITMAP hbmpMask;
    wxMask *pMask;
    bool deleteMask = false;

    if ( mask.IsOk() )
    {
        hbmpMask = GetHbitmapOf(mask);
        pMask = NULL;
    }
    else
    {
        pMask = bitmap.GetMask();

#if wxUSE_IMAGE
                                if ( !pMask )
        {
            wxImage img(bitmap.ConvertToImage());
            if ( img.HasAlpha() )
            {
                img.ConvertAlphaToMask();
                bitmapWithMask = wxBitmap(img);
                pMask = bitmapWithMask.GetMask();
            }
        }
#endif 
        if ( !pMask )
        {
                                                wxCOLORMAP *cmap = wxGetStdColourMap();
            wxColour col;
            wxRGBToColour(col, cmap[wxSTD_COL_BTNFACE].from);

            pMask = new wxMask(bitmap, col);

            deleteMask = true;
        }

        hbmpMask = (HBITMAP)pMask->GetMaskBitmap();
    }

        HBITMAP hbmpMaskInv = wxInvertMask(hbmpMask);

    if ( deleteMask )
    {
        delete pMask;
    }

    return hbmpMaskInv;
}
