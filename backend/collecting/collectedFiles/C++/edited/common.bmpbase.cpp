
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/bitmap.h"

#ifndef WX_PRECOMP
    #include "wx/colour.h"
    #include "wx/icon.h"
    #include "wx/image.h"
#endif 
#if wxUSE_IMAGE && wxUSE_LIBPNG && wxUSE_STREAMS
    #define wxHAS_PNG_LOAD

    #include "wx/mstream.h"
#endif


#if wxUSE_VARIANT
IMPLEMENT_VARIANT_OBJECT_EXPORTED_SHALLOWCMP(wxBitmap,WXDLLEXPORT)
IMPLEMENT_VARIANT_OBJECT_EXPORTED_SHALLOWCMP(wxIcon,WXDLLEXPORT)
#endif

#if wxUSE_EXTENDED_RTTI
#endif



#ifndef __WXOSX__


wxBitmap wxBitmapHelpers::NewFromPNGData(const void* data, size_t size)
{
    wxBitmap bitmap;

#ifdef wxHAS_PNG_LOAD
    wxMemoryInputStream is(data, size);
    wxImage image(is, wxBITMAP_TYPE_PNG);
    if ( image.IsOk() )
        bitmap = wxBitmap(image);
#endif 
    return bitmap;
}

#endif 

#if wxUSE_BITMAP_BASE

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/utils.h"
    #include "wx/palette.h"
    #include "wx/module.h"
#endif 

wxIMPLEMENT_ABSTRACT_CLASS(wxBitmapBase, wxGDIObject);
wxIMPLEMENT_ABSTRACT_CLASS(wxBitmapHandler, wxObject);

wxList wxBitmapBase::sm_handlers;

void wxBitmapBase::AddHandler(wxBitmapHandler *handler)
{
    sm_handlers.Append(handler);
}

void wxBitmapBase::InsertHandler(wxBitmapHandler *handler)
{
    sm_handlers.Insert(handler);
}

bool wxBitmapBase::RemoveHandler(const wxString& name)
{
    wxBitmapHandler *handler = FindHandler(name);
    if ( handler )
    {
        sm_handlers.DeleteObject(handler);
        return true;
    }
    else
        return false;
}

wxBitmapHandler *wxBitmapBase::FindHandler(const wxString& name)
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while ( node )
    {
        wxBitmapHandler *handler = (wxBitmapHandler *)node->GetData();
        if ( handler->GetName() == name )
            return handler;
        node = node->GetNext();
    }
    return NULL;
}

wxBitmapHandler *wxBitmapBase::FindHandler(const wxString& extension, wxBitmapType bitmapType)
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while ( node )
    {
        wxBitmapHandler *handler = (wxBitmapHandler *)node->GetData();
        if ( handler->GetExtension() == extension &&
                    (bitmapType == wxBITMAP_TYPE_ANY || handler->GetType() == bitmapType) )
            return handler;
        node = node->GetNext();
    }
    return NULL;
}

wxBitmapHandler *wxBitmapBase::FindHandler(wxBitmapType bitmapType)
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while ( node )
    {
        wxBitmapHandler *handler = (wxBitmapHandler *)node->GetData();
        if (handler->GetType() == bitmapType)
            return handler;
        node = node->GetNext();
    }
    return NULL;
}

void wxBitmapBase::CleanUpHandlers()
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while ( node )
    {
        wxBitmapHandler *handler = (wxBitmapHandler *)node->GetData();
        wxList::compatibility_iterator next = node->GetNext();
        delete handler;
        sm_handlers.Erase(node);
        node = next;
    }
}

class wxBitmapBaseModule: public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxBitmapBaseModule);
public:
    wxBitmapBaseModule() {}
    bool OnInit() wxOVERRIDE { wxBitmap::InitStandardHandlers(); return true; }
    void OnExit() wxOVERRIDE { wxBitmap::CleanUpHandlers(); }
};

wxIMPLEMENT_DYNAMIC_CLASS(wxBitmapBaseModule, wxModule);

#endif 

#if !(defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXX11__) || defined(__WXQT__))

wxBitmap::wxBitmap(const char* const* bits)
{
    wxCHECK2_MSG(bits != NULL, return, wxT("invalid bitmap data"));

#if wxUSE_IMAGE && wxUSE_XPM
    wxImage image(bits);
    wxCHECK2_MSG(image.IsOk(), return, wxT("invalid bitmap data"));

    *this = wxBitmap(image);
#else
    wxFAIL_MSG(wxT("creating bitmaps from XPMs not supported"));
#endif }
#endif 

bool wxMaskBase::Create(const wxBitmap& bitmap, const wxColour& colour)
{
    FreeData();

    return InitFromColour(bitmap, colour);
}

#if wxUSE_PALETTE

bool wxMaskBase::Create(const wxBitmap& bitmap, int paletteIndex)
{
    wxPalette *pal = bitmap.GetPalette();

    wxCHECK_MSG( pal, false,
                 wxT("Cannot create mask from palette index of a bitmap without palette") );

    unsigned char r,g,b;
    pal->GetRGB(paletteIndex, &r, &g, &b);

    return Create(bitmap, wxColour(r, g, b));
}

#endif 
bool wxMaskBase::Create(const wxBitmap& bitmap)
{
    FreeData();

    return InitFromMonoBitmap(bitmap);
}
