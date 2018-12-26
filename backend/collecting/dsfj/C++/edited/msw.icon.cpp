


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/list.h"
    #include "wx/utils.h"
    #include "wx/app.h"
    #include "wx/icon.h"
    #include "wx/bitmap.h"
    #include "wx/log.h"
#endif

#include "wx/msw/private.h"


wxIMPLEMENT_DYNAMIC_CLASS(wxIcon, wxGDIObject);



void wxIconRefData::Free()
{
    if ( m_hIcon )
    {
        ::DestroyIcon((HICON) m_hIcon);

        m_hIcon = 0;
    }
}


wxIcon::wxIcon(const char bits[], int width, int height)
{
    wxBitmap bmp(bits, width, height);
    CopyFromBitmap(bmp);
}

wxIcon::wxIcon(const wxString& iconfile,
               wxBitmapType type,
               int desiredWidth,
               int desiredHeight)

{
    LoadFile(iconfile, type, desiredWidth, desiredHeight);
}

wxIcon::wxIcon(const wxIconLocation& loc)
{
        wxString fullname = loc.GetFileName();
    if ( loc.GetIndex() )
    {
        fullname << wxT(';') << loc.GetIndex();
    }
    
    LoadFile(fullname, wxBITMAP_TYPE_ICO);
}

wxIcon::~wxIcon()
{
}

wxObjectRefData *wxIcon::CloneRefData(const wxObjectRefData *dataOrig) const
{
    const wxIconRefData *
        data = static_cast<const wxIconRefData *>(dataOrig);
    if ( !data )
        return NULL;

                        return new wxIconRefData(*data);
}

void wxIcon::CopyFromBitmap(const wxBitmap& bmp)
{
    HICON hicon = wxBitmapToHICON(bmp);
    if ( !hicon )
    {
        wxLogLastError(wxT("CreateIconIndirect"));
    }
    else
    {
        SetHICON((WXHICON)hicon);
        SetSize(bmp.GetWidth(), bmp.GetHeight());
    }
}

void wxIcon::CreateIconFromXpm(const char* const* data)
{
    wxBitmap bmp(data);
    CopyFromBitmap(bmp);
}

bool wxIcon::LoadFile(const wxString& filename,
                      wxBitmapType type,
                      int desiredWidth, int desiredHeight)
{
    UnRef();

    wxGDIImageHandler *handler = FindHandler(type);

    if ( !handler )
    {
                        wxBitmap bmp;
        if ( !bmp.LoadFile(filename, type) )
            return false;

        CopyFromBitmap(bmp);
        return true;
    }

    return handler->Load(this, filename, type, desiredWidth, desiredHeight);
}

bool wxIcon::CreateFromHICON(WXHICON icon)
{
    SetHICON(icon);
    if ( !IsOk() )
        return false;

    SetSize(wxGetHiconSize(icon));

    return true;
}
