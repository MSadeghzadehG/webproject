
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/iconbndl.h"

#ifndef WX_PRECOMP
    #include "wx/settings.h"
    #include "wx/log.h"
    #include "wx/intl.h"
    #include "wx/bitmap.h"
    #include "wx/image.h"
    #include "wx/stream.h"
#endif

#include "wx/wfstream.h"

#include "wx/arrimpl.cpp"
WX_DEFINE_OBJARRAY(wxIconArray)

wxIMPLEMENT_DYNAMIC_CLASS(wxIconBundle, wxGDIObject);

#define M_ICONBUNDLEDATA static_cast<wxIconBundleRefData*>(m_refData)


class WXDLLEXPORT wxIconBundleRefData : public wxGDIRefData
{
public:
    wxIconBundleRefData() { }

            wxIconBundleRefData(const wxIconBundleRefData& other)
        : wxGDIRefData(),
          m_icons(other.m_icons)
    {
    }

    
    virtual bool IsOk() const wxOVERRIDE { return !m_icons.empty(); }

    wxIconArray m_icons;
};


wxIconBundle::wxIconBundle()
{
}

#if wxUSE_STREAMS && wxUSE_IMAGE

#if wxUSE_FFILE || wxUSE_FILE
wxIconBundle::wxIconBundle(const wxString& file, wxBitmapType type)
            : wxGDIObject()
{
    AddIcon(file, type);
}
#endif 
wxIconBundle::wxIconBundle(wxInputStream& stream, wxBitmapType type)
            : wxGDIObject()
{
    AddIcon(stream, type);
}
#endif 
wxIconBundle::wxIconBundle(const wxIcon& icon)
            : wxGDIObject()
{
    AddIcon(icon);
}

wxGDIRefData *wxIconBundle::CreateGDIRefData() const
{
    return new wxIconBundleRefData;
}

wxGDIRefData *wxIconBundle::CloneGDIRefData(const wxGDIRefData *data) const
{
    return new wxIconBundleRefData(*static_cast<const wxIconBundleRefData *>(data));
}

void wxIconBundle::DeleteIcons()
{
    UnRef();
}

#if wxUSE_STREAMS && wxUSE_IMAGE

namespace
{

void DoAddIcon(wxIconBundle& bundle,
               wxInputStream& input,
               wxBitmapType type,
               const wxString& errorMessage)
{
    wxImage image;

    const wxFileOffset posOrig = input.TellI();

    const size_t count = wxImage::GetImageCount(input, type);
    for ( size_t i = 0; i < count; ++i )
    {
        if ( i )
        {
                                                input.SeekI(posOrig);
        }

        if ( !image.LoadFile(input, type, i) )
        {
            wxLogError(errorMessage, i);
            continue;
        }

        if ( type == wxBITMAP_TYPE_ANY )
        {
                                    type = image.GetType();
        }

        wxIcon tmp;
        tmp.CopyFromBitmap(wxBitmap(image));
        bundle.AddIcon(tmp);
    }
}

} 
#if wxUSE_FFILE || wxUSE_FILE

void wxIconBundle::AddIcon(const wxString& file, wxBitmapType type)
{
#ifdef __WXMAC__
        if ( type == wxBITMAP_TYPE_ICON_RESOURCE )
    {
        wxIcon tmp(file, type);
        if (tmp.IsOk())
        {
            AddIcon(tmp);
            return;
        }
    }
#endif 
#if wxUSE_FFILE
    wxFFileInputStream stream(file);
#elif wxUSE_FILE
    wxFileInputStream stream(file);
#endif
    DoAddIcon
    (
        *this,
        stream, type,
        wxString::Format(_("Failed to load image %%d from file '%s'."), file)
    );
}

#endif 
void wxIconBundle::AddIcon(wxInputStream& stream, wxBitmapType type)
{
    DoAddIcon(*this, stream, type, _("Failed to load image %d from stream."));
}

#endif 
wxIcon wxIconBundle::GetIcon(const wxSize& size, int flags) const
{
    wxASSERT( size == wxDefaultSize || (size.x >= 0 && size.y > 0) );

        wxCoord sysX = 0,
            sysY = 0;
    if ( flags & FALLBACK_SYSTEM )
    {
        sysX = wxSystemSettings::GetMetric(wxSYS_ICON_X);
        sysY = wxSystemSettings::GetMetric(wxSYS_ICON_Y);
    }

        wxCoord sizeX = size.x;
    wxCoord sizeY = size.y;
    if ( size == wxDefaultSize )
    {
        wxASSERT_MSG( flags == FALLBACK_SYSTEM,
                      wxS("Must have valid size if not using FALLBACK_SYSTEM") );

        sizeX = sysX;
        sizeY = sysY;
    }

            wxIcon iconBest;
    int bestDiff = 0;
    bool bestIsLarger = false;
    bool bestIsSystem = false;

    const size_t count = GetIconCount();

    const wxIconArray& iconArray = M_ICONBUNDLEDATA->m_icons;
    for ( size_t i = 0; i < count; i++ )
    {
        const wxIcon& icon = iconArray[i];
        if ( !icon.IsOk() )
            continue;
        wxCoord sx = icon.GetWidth(),
                sy = icon.GetHeight();

                if ( sx == sizeX && sy == sizeY )
        {
            iconBest = icon;
            break;
        }

        if ( flags & FALLBACK_SYSTEM )
        {
            if ( sx == sysX && sy == sysY )
            {
                iconBest = icon;
                bestIsSystem = true;
                continue;
            }
        }

        if ( !bestIsSystem && (flags & FALLBACK_NEAREST_LARGER) )
        {
            bool iconLarger = (sx >= sizeX) && (sy >= sizeY);
            int iconDiff = abs(sx - sizeX) + abs(sy - sizeY);

                                                            if ( !iconBest.IsOk() ||
                    (!bestIsLarger && iconLarger) ||
                        (iconLarger && (iconDiff < bestDiff)) )
            {
                iconBest = icon;
                bestIsLarger = iconLarger;
                bestDiff = iconDiff;
                continue;
            }
        }
    }

    return iconBest;
}

wxIcon wxIconBundle::GetIconOfExactSize(const wxSize& size) const
{
    return GetIcon(size, FALLBACK_NONE);
}

void wxIconBundle::AddIcon(const wxIcon& icon)
{
    wxCHECK_RET( icon.IsOk(), wxT("invalid icon") );

    AllocExclusive();

    wxIconArray& iconArray = M_ICONBUNDLEDATA->m_icons;

        const size_t count = iconArray.size();
    for ( size_t i = 0; i < count; ++i )
    {
        wxIcon& tmp = iconArray[i];
        if ( tmp.IsOk() &&
                tmp.GetWidth() == icon.GetWidth() &&
                tmp.GetHeight() == icon.GetHeight() )
        {
            tmp = icon;
            return;
        }
    }

        iconArray.Add(icon);
}

size_t wxIconBundle::GetIconCount() const
{
    return IsOk() ? M_ICONBUNDLEDATA->m_icons.size() : 0;
}

wxIcon wxIconBundle::GetIconByIndex(size_t n) const
{
    wxCHECK_MSG( n < GetIconCount(), wxNullIcon, wxT("invalid index") );

    return M_ICONBUNDLEDATA->m_icons[n];
}
