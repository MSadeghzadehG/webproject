


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/log.h"
    #include "wx/app.h"
    #include "wx/bitmap.h"
#endif 
#include "wx/msw/private.h"

#include "wx/msw/gdiimage.h"

#if wxUSE_WXDIB
#include "wx/msw/dib.h"
#endif

#if !defined(wxUSE_PNG_RESOURCE_HANDLER) && wxUSE_LIBPNG && wxUSE_IMAGE
    #define wxUSE_PNG_RESOURCE_HANDLER 1
#endif

#if wxUSE_PNG_RESOURCE_HANDLER
    #include "wx/image.h"
    #include "wx/utils.h"       #endif

#include "wx/file.h"

#include "wx/listimpl.cpp"
WX_DEFINE_LIST(wxGDIImageHandlerList)



class WXDLLEXPORT wxBMPFileHandler : public wxBitmapHandler
{
public:
    wxBMPFileHandler() : wxBitmapHandler(wxT("Windows bitmap file"), wxT("bmp"),
                                         wxBITMAP_TYPE_BMP)
    {
    }

    virtual bool LoadFile(wxBitmap *bitmap,
                          const wxString& name, wxBitmapType flags,
                          int desiredWidth, int desiredHeight);
    virtual bool SaveFile(const wxBitmap *bitmap,
                          const wxString& name, wxBitmapType type,
                          const wxPalette *palette = NULL) const;

private:
    wxDECLARE_DYNAMIC_CLASS(wxBMPFileHandler);
};

class WXDLLEXPORT wxBMPResourceHandler: public wxBitmapHandler
{
public:
    wxBMPResourceHandler() : wxBitmapHandler(wxT("Windows bitmap resource"),
                                             wxEmptyString,
                                             wxBITMAP_TYPE_BMP_RESOURCE)
    {
    }

    virtual bool LoadFile(wxBitmap *bitmap,
                          const wxString& name, wxBitmapType flags,
                          int desiredWidth, int desiredHeight);

private:
    wxDECLARE_DYNAMIC_CLASS(wxBMPResourceHandler);
};

class WXDLLEXPORT wxIconHandler : public wxGDIImageHandler
{
public:
    wxIconHandler(const wxString& name, const wxString& ext, wxBitmapType type)
        : wxGDIImageHandler(name, ext, type)
    {
    }

        virtual bool Create(wxGDIImage *WXUNUSED(image),
                        const void* WXUNUSED(data),
                        wxBitmapType WXUNUSED(flags),
                        int WXUNUSED(width),
                        int WXUNUSED(height),
                        int WXUNUSED(depth) = 1)
    {
        return false;
    }

    virtual bool Save(const wxGDIImage *WXUNUSED(image),
                      const wxString& WXUNUSED(name),
                      wxBitmapType WXUNUSED(type)) const
    {
        return false;
    }

    virtual bool Load(wxGDIImage *image,
                      const wxString& name,
                      wxBitmapType flags,
                      int desiredWidth, int desiredHeight)
    {
        wxIcon *icon = wxDynamicCast(image, wxIcon);
        wxCHECK_MSG( icon, false, wxT("wxIconHandler only works with icons") );

        return LoadIcon(icon, name, flags, desiredWidth, desiredHeight);
    }

protected:
    virtual bool LoadIcon(wxIcon *icon,
                          const wxString& name, wxBitmapType flags,
                          int desiredWidth = -1, int desiredHeight = -1) = 0;
};

class WXDLLEXPORT wxICOFileHandler : public wxIconHandler
{
public:
    wxICOFileHandler() : wxIconHandler(wxT("ICO icon file"),
                                       wxT("ico"),
                                       wxBITMAP_TYPE_ICO)
    {
    }

protected:
    virtual bool LoadIcon(wxIcon *icon,
                          const wxString& name, wxBitmapType flags,
                          int desiredWidth = -1, int desiredHeight = -1);

private:
    wxDECLARE_DYNAMIC_CLASS(wxICOFileHandler);
};

class WXDLLEXPORT wxICOResourceHandler: public wxIconHandler
{
public:
    wxICOResourceHandler() : wxIconHandler(wxT("ICO resource"),
                                           wxT("ico"),
                                           wxBITMAP_TYPE_ICO_RESOURCE)
    {
    }

protected:
    virtual bool LoadIcon(wxIcon *icon,
                          const wxString& name, wxBitmapType flags,
                          int desiredWidth = -1, int desiredHeight = -1);

private:
    wxDECLARE_DYNAMIC_CLASS(wxICOResourceHandler);
};

#if wxUSE_PNG_RESOURCE_HANDLER

class WXDLLEXPORT wxPNGResourceHandler : public wxBitmapHandler
{
public:
    wxPNGResourceHandler() : wxBitmapHandler(wxS("Windows PNG resource"),
                                             wxString(),
                                             wxBITMAP_TYPE_PNG_RESOURCE)
    {
    }

    virtual bool LoadFile(wxBitmap *bitmap,
                          const wxString& name, wxBitmapType flags,
                          int desiredWidth, int desiredHeight);

private:
    wxDECLARE_DYNAMIC_CLASS(wxPNGResourceHandler);
};

#endif 

wxIMPLEMENT_DYNAMIC_CLASS(wxBMPFileHandler, wxBitmapHandler);
wxIMPLEMENT_DYNAMIC_CLASS(wxBMPResourceHandler, wxBitmapHandler);
wxIMPLEMENT_DYNAMIC_CLASS(wxICOFileHandler, wxObject);
wxIMPLEMENT_DYNAMIC_CLASS(wxICOResourceHandler, wxObject);
#if wxUSE_PNG_RESOURCE_HANDLER
wxIMPLEMENT_DYNAMIC_CLASS(wxPNGResourceHandler, wxBitmapHandler);
#endif 



wxGDIImageHandlerList wxGDIImage::ms_handlers;


bool wxGDIImage::FreeResource(bool WXUNUSED(force))
{
    if ( !IsNull() )
    {
        GetGDIImageData()->Free();
        GetGDIImageData()->m_handle = 0;
    }

    return true;
}

WXHANDLE wxGDIImage::GetResourceHandle() const
{
    return GetHandle();
}


void wxGDIImage::AddHandler(wxGDIImageHandler *handler)
{
    ms_handlers.Append(handler);
}

void wxGDIImage::InsertHandler(wxGDIImageHandler *handler)
{
    ms_handlers.Insert(handler);
}

bool wxGDIImage::RemoveHandler(const wxString& name)
{
    wxGDIImageHandler *handler = FindHandler(name);
    if ( handler )
    {
        ms_handlers.DeleteObject(handler);
        return true;
    }
    else
        return false;
}

wxGDIImageHandler *wxGDIImage::FindHandler(const wxString& name)
{
    wxGDIImageHandlerList::compatibility_iterator node = ms_handlers.GetFirst();
    while ( node )
    {
        wxGDIImageHandler *handler = node->GetData();
        if ( handler->GetName() == name )
            return handler;
        node = node->GetNext();
    }

    return NULL;
}

wxGDIImageHandler *wxGDIImage::FindHandler(const wxString& extension,
                                           long type)
{
    wxGDIImageHandlerList::compatibility_iterator node = ms_handlers.GetFirst();
    while ( node )
    {
        wxGDIImageHandler *handler = node->GetData();
        if ( (handler->GetExtension() == extension) &&
             (type == -1 || handler->GetType() == type) )
        {
            return handler;
        }

        node = node->GetNext();
    }
    return NULL;
}

wxGDIImageHandler *wxGDIImage::FindHandler(long type)
{
    wxGDIImageHandlerList::compatibility_iterator node = ms_handlers.GetFirst();
    while ( node )
    {
        wxGDIImageHandler *handler = node->GetData();
        if ( handler->GetType() == type )
            return handler;

        node = node->GetNext();
    }

    return NULL;
}

void wxGDIImage::CleanUpHandlers()
{
    wxGDIImageHandlerList::compatibility_iterator node = ms_handlers.GetFirst();
    while ( node )
    {
        wxGDIImageHandler *handler = node->GetData();
        wxGDIImageHandlerList::compatibility_iterator next = node->GetNext();
        delete handler;
        ms_handlers.Erase( node );
        node = next;
    }
}

void wxGDIImage::InitStandardHandlers()
{
    AddHandler(new wxBMPResourceHandler);
    AddHandler(new wxBMPFileHandler);
    AddHandler(new wxICOFileHandler);
    AddHandler(new wxICOResourceHandler);
#if wxUSE_PNG_RESOURCE_HANDLER
    AddHandler(new wxPNGResourceHandler);
#endif }


bool wxBMPResourceHandler::LoadFile(wxBitmap *bitmap,
                                    const wxString& name, wxBitmapType WXUNUSED(flags),
                                    int WXUNUSED(desiredWidth),
                                    int WXUNUSED(desiredHeight))
{
        bitmap->SetHBITMAP((WXHBITMAP)::LoadBitmap(wxGetInstance(), name.t_str()));

    if ( !bitmap->IsOk() )
    {
                wxLogError(wxT("Can't load bitmap '%s' from resources! Check .rc file."),
                   name.c_str());

        return false;
    }

    BITMAP bm;
    if ( !::GetObject(GetHbitmapOf(*bitmap), sizeof(BITMAP), (LPSTR) &bm) )
    {
        wxLogLastError(wxT("GetObject(HBITMAP)"));
    }

    bitmap->SetWidth(bm.bmWidth);
    bitmap->SetHeight(bm.bmHeight);
    bitmap->SetDepth(bm.bmBitsPixel);

        bitmap->SetMask(new wxMask(*bitmap, *wxLIGHT_GREY));

    return true;
}

bool wxBMPFileHandler::LoadFile(wxBitmap *bitmap,
                                const wxString& name, wxBitmapType WXUNUSED(flags),
                                int WXUNUSED(desiredWidth),
                                int WXUNUSED(desiredHeight))
{
    wxCHECK_MSG( bitmap, false, wxT("NULL bitmap in LoadFile") );

#if wxUSE_WXDIB
        wxDIB dib(name);
    if ( dib.IsOk() )
        return bitmap->CopyFromDIB(dib);
#endif 
            #if wxUSE_IMAGE
    wxImage img(name, wxBITMAP_TYPE_BMP);
    if ( img.IsOk() )
    {
        *bitmap = wxBitmap(img);
        return true;
    }
#endif 
    return false;
}

bool wxBMPFileHandler::SaveFile(const wxBitmap *bitmap,
                                const wxString& name,
                                wxBitmapType WXUNUSED(type),
                                const wxPalette * WXUNUSED(pal)) const
{
#if wxUSE_WXDIB
    wxCHECK_MSG( bitmap, false, wxT("NULL bitmap in SaveFile") );

    wxDIB dib(*bitmap);

    return dib.Save(name);
#else
    return false;
#endif
}


bool wxICOFileHandler::LoadIcon(wxIcon *icon,
                                const wxString& name,
                                wxBitmapType WXUNUSED(flags),
                                int desiredWidth, int desiredHeight)
{
    icon->UnRef();

    HICON hicon = NULL;

                        int iconIndex = 0;
    wxString nameReal(name);
    wxString strIconIndex = name.AfterLast(wxT(';'));
    if (strIconIndex != name)
    {
        iconIndex = wxAtoi(strIconIndex);
        nameReal = name.BeforeLast(wxT(';'));
    }

#if 0
                            if ( desiredWidth == -1 &&
         desiredHeight == -1)
    {
                if ( ::ExtractIconEx(nameReal, iconIndex, &hicon, NULL, 1) == 1)
        {
        }
                else if ( ::ExtractIconEx(nameReal, iconIndex, NULL, &hicon, 1) == 1)
        {
        }
    }
    else
#endif
            if ( desiredWidth == ::GetSystemMetrics(SM_CXICON) &&
         desiredHeight == ::GetSystemMetrics(SM_CYICON) )
    {
                if ( !::ExtractIconEx(nameReal.t_str(), iconIndex, &hicon, NULL, 1) )
        {
                                    wxLogTrace(wxT("iconload"),
                       wxT("No large icons found in the file '%s'."),
                       name.c_str());
        }
    }
    else if ( desiredWidth == ::GetSystemMetrics(SM_CXSMICON) &&
              desiredHeight == ::GetSystemMetrics(SM_CYSMICON) )
    {
                if ( !::ExtractIconEx(nameReal.t_str(), iconIndex, NULL, &hicon, 1) )
        {
            wxLogTrace(wxT("iconload"),
                       wxT("No small icons found in the file '%s'."),
                       name.c_str());
        }
    }
    
    if ( !hicon )
    {
                hicon = ::ExtractIcon(wxGetInstance(), nameReal.t_str(), iconIndex);
    }

    if ( !hicon )
    {
        wxLogSysError(wxT("Failed to load icon from the file '%s'"),
                      name.c_str());

        return false;
    }

    if ( !icon->CreateFromHICON(hicon) )
        return false;

    if ( (desiredWidth != -1 && desiredWidth != icon->GetWidth()) ||
         (desiredHeight != -1 && desiredHeight != icon->GetHeight()) )
    {
        wxLogTrace(wxT("iconload"),
                   wxT("Returning false from wxICOFileHandler::Load because of the size mismatch: actual (%d, %d), requested (%d, %d)"),
                   icon->GetWidth(), icon->GetHeight(),
                   desiredWidth, desiredHeight);

        icon->UnRef();

        return false;
    }

    return true;
}

bool wxICOResourceHandler::LoadIcon(wxIcon *icon,
                                    const wxString& name,
                                    wxBitmapType WXUNUSED(flags),
                                    int desiredWidth, int desiredHeight)
{
    HICON hicon;

        bool hasSize = desiredWidth != -1 || desiredHeight != -1;

    wxASSERT_MSG( !hasSize || (desiredWidth != -1 && desiredHeight != -1),
                  wxT("width and height should be either both -1 or not") );

            
            if ( hasSize )
    {
        hicon = (HICON)::LoadImage(wxGetInstance(), name.t_str(), IMAGE_ICON,
                                    desiredWidth, desiredHeight,
                                    LR_DEFAULTCOLOR);
    }
    else
    {
        hicon = ::LoadIcon(wxGetInstance(), name.t_str());
    }

        if ( !hicon && !hasSize )
    {
        static const struct
        {
            const wxChar *name;
            LPTSTR id;
        } stdIcons[] =
        {
            { wxT("wxICON_QUESTION"),   IDI_QUESTION    },
            { wxT("wxICON_WARNING"),    IDI_EXCLAMATION },
            { wxT("wxICON_ERROR"),      IDI_HAND        },
            { wxT("wxICON_INFORMATION"),       IDI_ASTERISK    },
        };

        for ( size_t nIcon = 0; !hicon && nIcon < WXSIZEOF(stdIcons); nIcon++ )
        {
            if ( name == stdIcons[nIcon].name )
            {
                hicon = ::LoadIcon((HINSTANCE)NULL, stdIcons[nIcon].id);
                break;
            }
        }
    }

    return icon->CreateFromHICON((WXHICON)hicon);
}

#if wxUSE_PNG_RESOURCE_HANDLER


bool wxPNGResourceHandler::LoadFile(wxBitmap *bitmap,
                                    const wxString& name,
                                    wxBitmapType WXUNUSED(flags),
                                    int WXUNUSED(desiredWidth),
                                    int WXUNUSED(desiredHeight))
{
    const void* pngData = NULL;
    size_t pngSize = 0;

                    if ( !wxLoadUserResource(&pngData, &pngSize,
                             name,
                             RT_RCDATA,
                             wxGetInstance()) )
    {
                        wxLogError(wxS("Bitmap in PNG format \"%s\" not found, check ")
                   wxS("that the resource file contains \"RCDATA\" ")
                   wxS("resource with this name."),
                   name);

        return false;
    }

    *bitmap = wxBitmap::NewFromPNGData(pngData, pngSize);
    if ( !bitmap->IsOk() )
    {
        wxLogError(wxS("Couldn't load resource bitmap \"%s\" as a PNG. "),
                   wxS("Have you registered PNG image handler?"),
                   name);

        return false;
    }

    return true;
}

#endif 

wxSize wxGetHiconSize(HICON hicon)
{
    wxSize size;

    if ( hicon )
    {
        AutoIconInfo info;
        if ( info.GetFrom(hicon) )
        {
            HBITMAP hbmp = info.hbmMask;
            if ( hbmp )
            {
                BITMAP bm;
                if ( ::GetObject(hbmp, sizeof(BITMAP), (LPSTR) &bm) )
                {
                    size = wxSize(bm.bmWidth, bm.bmHeight);
                }
            }
        }
    }

    if ( !size.x )
    {
                size.x = ::GetSystemMetrics(SM_CXICON);
        size.y = ::GetSystemMetrics(SM_CYICON);
    }

    return size;
}
