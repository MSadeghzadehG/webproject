


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_STATBMP

#include "wx/statbmp.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/window.h"
    #include "wx/icon.h"
    #include "wx/dcclient.h"
#endif

#include "wx/msw/private.h"
#include "wx/msw/dib.h"

#include "wx/sysopt.h"

#include <stdio.h>


wxBEGIN_EVENT_TABLE(wxStaticBitmap, wxStaticBitmapBase)
    EVT_SIZE(wxStaticBitmap::WXHandleSize)
wxEND_EVENT_TABLE()




static wxGDIImage* ConvertImage( const wxGDIImage& bitmap )
{
    bool isIcon = bitmap.IsKindOf( wxCLASSINFO(wxIcon) );

    if( !isIcon )
    {
        wxASSERT_MSG( wxDynamicCast(&bitmap, wxBitmap),
                      wxT("not an icon and not a bitmap?") );

        const wxBitmap& bmp = (const wxBitmap&)bitmap;
        wxMask *mask = bmp.GetMask();
        if ( mask && mask->GetMaskBitmap() )
        {
            wxIcon* icon = new wxIcon;
            icon->CopyFromBitmap(bmp);

            return icon;
        }

        return new wxBitmap( bmp );
    }

        return new wxIcon( (const wxIcon&)bitmap );
}

bool wxStaticBitmap::Create(wxWindow *parent,
                            wxWindowID id,
                            const wxGDIImage& bitmap,
                            const wxPoint& pos,
                            const wxSize& size,
                            long style,
                            const wxString& name)
{
    if ( !CreateControl(parent, id, pos, size, style, wxDefaultValidator, name) )
        return false;

                m_isIcon = bitmap.IsKindOf(wxCLASSINFO(wxIcon));

    wxGDIImage *image = ConvertImage( bitmap );
    m_isIcon = image->IsKindOf( wxCLASSINFO(wxIcon) );

        if ( !MSWCreateControl(wxT("STATIC"), wxEmptyString, pos, size) )
    {
                return false;
    }

        SetImageNoCopy(image);

        SetInitialSize(size);

                                    if ( wxTheApp->GetComCtl32Version() < 600 )
    {
        Connect(wxEVT_PAINT, wxPaintEventHandler(wxStaticBitmap::DoPaintManually));
    }

    return true;
}

WXDWORD wxStaticBitmap::MSWGetStyle(long style, WXDWORD *exstyle) const
{
    WXDWORD msStyle = wxControl::MSWGetStyle(style, exstyle);

        msStyle |= m_isIcon ? SS_ICON : SS_BITMAP;

                    msStyle |= SS_CENTERIMAGE | SS_NOTIFY;

    return msStyle;
}

bool wxStaticBitmap::ImageIsOk() const
{
    return m_image && m_image->IsOk();
}

wxIcon wxStaticBitmap::GetIcon() const
{
    wxCHECK_MSG( m_image, wxIcon(), wxT("no image in wxStaticBitmap") );

        wxCHECK_MSG( m_isIcon, wxIcon(), wxT("no icon in this wxStaticBitmap") );

    return *(wxIcon *)m_image;
}

wxBitmap wxStaticBitmap::GetBitmap() const
{
    if ( m_isIcon )
    {
                                        return wxBitmap(GetIcon());
    }
    else     {
        wxCHECK_MSG( m_image, wxBitmap(), wxT("no image in wxStaticBitmap") );

        return *(wxBitmap *)m_image;
    }
}

void wxStaticBitmap::Init()
{
    m_isIcon = true;
    m_image = NULL;
    m_currentHandle = 0;
    m_ownsCurrentHandle = false;
}

void wxStaticBitmap::DeleteCurrentHandleIfNeeded()
{
    if ( m_ownsCurrentHandle )
    {
        ::DeleteObject(m_currentHandle);
        m_ownsCurrentHandle = false;
    }
}

void wxStaticBitmap::Free()
{
    MSWReplaceImageHandle(0);

    DeleteCurrentHandleIfNeeded();

    wxDELETE(m_image);
}

wxSize wxStaticBitmap::DoGetBestClientSize() const
{
    wxSize size;
    if ( ImageIsOk() )
    {
        size = m_image->GetSize();
    }
    else     {
                size.x =
        size.y = 16;
    }

    return size;
}

void wxStaticBitmap::WXHandleSize(wxSizeEvent& event)
{
            Refresh();

    event.Skip();
}

void wxStaticBitmap::DoPaintManually(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);

    const wxSize size(GetSize());
    const wxBitmap bmp(GetBitmap());

                const wxWindow *win = UseBgCol() ? this : GetParent();
    dc.SetBrush(win->GetBackgroundColour());
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());

        dc.DrawBitmap(bmp,
                  (size.GetWidth() - bmp.GetWidth()) / 2,
                  (size.GetHeight() - bmp.GetHeight()) / 2,
                  true );
}

void wxStaticBitmap::SetImage( const wxGDIImage* image )
{
    wxGDIImage* convertedImage = ConvertImage( *image );
    SetImageNoCopy( convertedImage );
}

void wxStaticBitmap::MSWReplaceImageHandle(WXLPARAM handle)
{
    HGDIOBJ oldHandle = (HGDIOBJ)::SendMessage(GetHwnd(), STM_SETIMAGE,
                  m_isIcon ? IMAGE_ICON : IMAGE_BITMAP, (LPARAM)handle);
            if (oldHandle != 0 && oldHandle != (HGDIOBJ) m_currentHandle)
    {
                ::DeleteObject((HGDIOBJ) oldHandle);
    }
}

void wxStaticBitmap::SetImageNoCopy( wxGDIImage* image)
{
    Free();
    InvalidateBestSize();

    m_isIcon = image->IsKindOf( wxCLASSINFO(wxIcon) );
        m_image = image;

    int x, y;
    int w, h;
    GetPosition(&x, &y);
    GetSize(&w, &h);

                const HANDLE handleOrig = (HANDLE)m_image->GetHandle();
    HANDLE handle = handleOrig;

#if wxUSE_WXDIB
    if ( !m_isIcon )
    {
                                        const wxBitmap& bmp = static_cast<wxBitmap&>(*image);
        if ( bmp.HasAlpha() )
        {
                                    handle = wxDIB(bmp.ConvertToImage(),
                           wxDIB::PixelFormat_NotPreMultiplied).Detach();
        }
    }
#endif     LONG style = ::GetWindowLong( (HWND)GetHWND(), GWL_STYLE ) ;
    ::SetWindowLong( (HWND)GetHWND(), GWL_STYLE, ( style & ~( SS_BITMAP|SS_ICON ) ) |
                     ( m_isIcon ? SS_ICON : SS_BITMAP ) );

    MSWReplaceImageHandle((WXLPARAM)handle);

    DeleteCurrentHandleIfNeeded();

    m_currentHandle = (WXHANDLE)handle;
    m_ownsCurrentHandle = handle != handleOrig;

    if ( ImageIsOk() )
    {
        int width = image->GetWidth(),
            height = image->GetHeight();
        if ( width && height )
        {
            w = width;
            h = height;

            ::MoveWindow(GetHwnd(), x, y, width, height, FALSE);
        }
    }

    RECT rect;
    rect.left   = x;
    rect.top    = y;
    rect.right  = x + w;
    rect.bottom = y + h;
    ::InvalidateRect(GetHwndOf(GetParent()), &rect, TRUE);
}

#endif 