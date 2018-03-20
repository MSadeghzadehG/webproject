


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/cursor.h"

#ifndef WX_PRECOMP
    #include "wx/utils.h"
    #include "wx/app.h"
    #include "wx/bitmap.h"
    #include "wx/icon.h"
    #include "wx/settings.h"
    #include "wx/intl.h"
    #include "wx/image.h"
    #include "wx/module.h"
#endif

#include "wx/msw/private.h"
#include "wx/msw/missing.h" 

class WXDLLEXPORT wxCursorRefData : public wxGDIImageRefData
{
public:
                wxCursorRefData(HCURSOR hcursor = 0, bool takeOwnership = false);

    virtual ~wxCursorRefData() { Free(); }

    virtual void Free();


            static wxCoord GetStandardWidth();
    static wxCoord GetStandardHeight();

private:
    bool m_destroyCursor;

        static wxSize ms_sizeStd;
};


wxIMPLEMENT_DYNAMIC_CLASS(wxCursor, wxGDIObject);


static wxCursor *gs_globalCursor = NULL;


class wxCursorModule : public wxModule
{
public:
    virtual bool OnInit()
    {
        gs_globalCursor = new wxCursor;

        return true;
    }

    virtual void OnExit()
    {
        wxDELETE(gs_globalCursor);
    }
};



wxSize wxCursorRefData::ms_sizeStd;

wxCoord wxCursorRefData::GetStandardWidth()
{
    if ( !ms_sizeStd.x )
        ms_sizeStd.x = wxSystemSettings::GetMetric(wxSYS_CURSOR_X);

    return ms_sizeStd.x;
}

wxCoord wxCursorRefData::GetStandardHeight()
{
    if ( !ms_sizeStd.y )
        ms_sizeStd.y = wxSystemSettings::GetMetric(wxSYS_CURSOR_Y);

    return ms_sizeStd.y;
}

wxCursorRefData::wxCursorRefData(HCURSOR hcursor, bool destroy)
{
    m_hCursor = (WXHCURSOR)hcursor;

    if ( m_hCursor )
    {
        m_width = GetStandardWidth();
        m_height = GetStandardHeight();
    }

    m_destroyCursor = destroy;
}

void wxCursorRefData::Free()
{
    if ( m_hCursor )
    {
        if ( m_destroyCursor )
            ::DestroyCursor((HCURSOR)m_hCursor);

        m_hCursor = 0;
    }
}


wxCursor::wxCursor()
{
}

#if wxUSE_IMAGE
wxCursor::wxCursor(const wxImage& image)
{
            const int w = wxCursorRefData::GetStandardWidth();
    const int h = wxCursorRefData::GetStandardHeight();

    int hotSpotX = image.GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_X);
    int hotSpotY = image.GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_Y);
    int image_w = image.GetWidth();
    int image_h = image.GetHeight();

    wxASSERT_MSG( hotSpotX >= 0 && hotSpotX < image_w &&
                  hotSpotY >= 0 && hotSpotY < image_h,
                  wxT("invalid cursor hot spot coordinates") );

    wxImage imageSized(image); 
        if ((w > image_w) && (h > image_h))
    {
        wxPoint offset((w - image_w)/2, (h - image_h)/2);
        hotSpotX = hotSpotX + offset.x;
        hotSpotY = hotSpotY + offset.y;

        imageSized = image.Size(wxSize(w, h), offset);
    }
    else if ((w != image_w) || (h != image_h))
    {
        hotSpotX = int(hotSpotX * double(w) / double(image_w));
        hotSpotY = int(hotSpotY * double(h) / double(image_h));

        imageSized = image.Scale(w, h);
    }

    HCURSOR hcursor = wxBitmapToHCURSOR( wxBitmap(imageSized),
                                         hotSpotX, hotSpotY );

    if ( !hcursor )
    {
        wxLogWarning(_("Failed to create cursor."));
        return;
    }

    m_refData = new wxCursorRefData(hcursor, true );
}
#endif 
wxCursor::wxCursor(const wxString& filename,
                   wxBitmapType kind,
                   int hotSpotX,
                   int hotSpotY)
{
    HCURSOR hcursor;
    switch ( kind )
    {
        case wxBITMAP_TYPE_CUR_RESOURCE:
            hcursor = ::LoadCursor(wxGetInstance(), filename.t_str());
            break;

        case wxBITMAP_TYPE_ANI:
        case wxBITMAP_TYPE_CUR:
            hcursor = ::LoadCursorFromFile(filename.t_str());
            break;

        case wxBITMAP_TYPE_ICO:
            hcursor = wxBitmapToHCURSOR
                      (
                       wxIcon(filename, wxBITMAP_TYPE_ICO),
                       hotSpotX,
                       hotSpotY
                      );
            break;

        case wxBITMAP_TYPE_BMP:
            hcursor = wxBitmapToHCURSOR
                      (
                       wxBitmap(filename, wxBITMAP_TYPE_BMP),
                       hotSpotX,
                       hotSpotY
                      );
            break;

        default:
            wxLogError( wxT("unknown cursor resource type '%d'"), kind );

            hcursor = NULL;
    }

    if ( hcursor )
    {
        m_refData = new wxCursorRefData(hcursor, true );
    }
}

wxPoint wxCursor::GetHotSpot() const
{
    if ( !GetGDIImageData() )
        return wxDefaultPosition;

    AutoIconInfo ii;
    if ( !ii.GetFrom((HICON)GetGDIImageData()->m_hCursor) )
        return wxDefaultPosition;

    return wxPoint(ii.xHotspot, ii.yHotspot);
}

namespace
{

void ReverseBitmap(HBITMAP bitmap, int width, int height)
{
    MemoryHDC hdc;
    SelectInHDC selBitmap(hdc, bitmap);
    ::StretchBlt(hdc, width - 1, 0, -width, height,
                 hdc, 0, 0, width, height, SRCCOPY);
}

HCURSOR CreateReverseCursor(HCURSOR cursor)
{
    AutoIconInfo info;
    if ( !info.GetFrom(cursor) )
        return NULL;

    BITMAP bmp;
    if ( !::GetObject(info.hbmMask, sizeof(bmp), &bmp) )
        return NULL;

    ReverseBitmap(info.hbmMask, bmp.bmWidth, bmp.bmHeight);
    if ( info.hbmColor )
        ReverseBitmap(info.hbmColor, bmp.bmWidth, bmp.bmHeight);
    info.xHotspot = (DWORD)bmp.bmWidth - 1 - info.xHotspot;

    return ::CreateIconIndirect(&info);
}

} 
void wxCursor::InitFromStock(wxStockCursor idCursor)
{
        static const struct StdCursor
    {
                bool isStd;

                LPCTSTR name;
    } stdCursors[] =
    {
        {  true, NULL                        },         {  true, IDC_ARROW                   },         { false, wxT("WXCURSOR_RIGHT_ARROW")  },         { false, wxT("WXCURSOR_BULLSEYE")     },         {  true, IDC_ARROW                   },         {  true, IDC_CROSS                   },         {  true, IDC_HAND                    },         {  true, IDC_IBEAM                   },         {  true, IDC_ARROW                   },         { false, wxT("WXCURSOR_MAGNIFIER")    },         {  true, IDC_ARROW                   },         {  true, IDC_NO                      },         { false, wxT("WXCURSOR_PBRUSH")       },         { false, wxT("WXCURSOR_PENCIL")       },         { false, wxT("WXCURSOR_PLEFT")        },         { false, wxT("WXCURSOR_PRIGHT")       },         {  true, IDC_HELP                    },         {  true, IDC_ARROW                   },         {  true, IDC_SIZENESW                },         {  true, IDC_SIZENS                  },         {  true, IDC_SIZENWSE                },         {  true, IDC_SIZEWE                  },         {  true, IDC_SIZEALL                 },         { false, wxT("WXCURSOR_PBRUSH")       },         {  true, IDC_WAIT                    },         {  true, IDC_WAIT                    },         { false, wxT("WXCURSOR_BLANK")        },         {  true, IDC_APPSTARTING             }, 
            };

    wxCOMPILE_TIME_ASSERT( WXSIZEOF(stdCursors) == wxCURSOR_MAX,
                           CursorsIdArrayMismatch );

    wxCHECK_RET( idCursor > 0 && (size_t)idCursor < WXSIZEOF(stdCursors),
                 wxT("invalid cursor id in wxCursor() ctor") );

    const StdCursor& stdCursor = stdCursors[idCursor];
    bool deleteLater = !stdCursor.isStd;

    HCURSOR hcursor = ::LoadCursor(stdCursor.isStd ? NULL : wxGetInstance(),
                                   stdCursor.name);

        if ( !hcursor && idCursor == wxCURSOR_HAND)
    {
        hcursor = ::LoadCursor(wxGetInstance(), wxT("WXCURSOR_HAND"));
        deleteLater = true;
    }

    if ( !hcursor && idCursor == wxCURSOR_RIGHT_ARROW)
    {
        hcursor = ::LoadCursor(NULL, IDC_ARROW);
        if ( hcursor )
        {
            hcursor = CreateReverseCursor(hcursor);
            deleteLater = true;
        }
    }

    if ( !hcursor )
    {
        if ( !stdCursor.isStd )
        {
                                    wxFAIL_MSG(wxT("Loading a cursor defined by wxWidgets failed, ")
                       wxT("did you include include/wx/msw/wx.rc file from ")
                       wxT("your resource file?"));
        }

        wxLogLastError(wxT("LoadCursor"));
    }
    else
    {
        m_refData = new wxCursorRefData(hcursor, deleteLater);
    }
}

wxCursor::~wxCursor()
{
}


wxGDIImageRefData *wxCursor::CreateData() const
{
    return new wxCursorRefData;
}


const wxCursor *wxGetGlobalCursor()
{
    return gs_globalCursor;
}

void wxSetCursor(const wxCursor& cursor)
{
    if ( cursor.IsOk() )
    {
        ::SetCursor(GetHcursorOf(cursor));

        if ( gs_globalCursor )
            *gs_globalCursor = cursor;
    }
}
