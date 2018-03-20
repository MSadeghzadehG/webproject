


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/dcclient.h"
#include "wx/msw/dcclient.h"

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/hashmap.h"
    #include "wx/log.h"
    #include "wx/window.h"
#endif

#include "wx/msw/private.h"


class wxPaintDCInfo
{
public:
    wxPaintDCInfo(HDC hdc)
        : m_hdc(hdc)
    {
    }

        virtual ~wxPaintDCInfo() = 0;

    WXHDC GetHDC() const { return (WXHDC)m_hdc; }

protected:
    const HDC m_hdc;

    wxDECLARE_NO_COPY_CLASS(wxPaintDCInfo);
};

namespace
{

class wxPaintDCInfoOur : public wxPaintDCInfo
{
public:
    wxPaintDCInfoOur(wxWindow* win)
        : wxPaintDCInfo(::BeginPaint(GetHwndOf(win), GetPaintStructPtr(m_ps))),
          m_hwnd(GetHwndOf(win))
    {
    }

    virtual ~wxPaintDCInfoOur()
    {
        ::EndPaint(m_hwnd, &m_ps);
    }

private:
            static PAINTSTRUCT* GetPaintStructPtr(PAINTSTRUCT& ps)
    {
        wxZeroMemory(ps);
        return &ps;
    }

    const HWND m_hwnd;
    PAINTSTRUCT m_ps;

    wxDECLARE_NO_COPY_CLASS(wxPaintDCInfoOur);
};

class wxPaintDCInfoExternal : public wxPaintDCInfo
{
public:
    wxPaintDCInfoExternal(HDC hdc)
        : wxPaintDCInfo(hdc),
          m_state(::SaveDC(hdc))
    {
    }

    virtual ~wxPaintDCInfoExternal()
    {
        ::RestoreDC(m_hdc, m_state);
    }

private:
    const int m_state;

    wxDECLARE_NO_COPY_CLASS(wxPaintDCInfoExternal);
};

WX_DECLARE_HASH_MAP(wxWindow *, wxPaintDCInfo *,
                    wxPointerHash, wxPointerEqual,
                    PaintDCInfos);

PaintDCInfos gs_PaintDCInfos;

} 

#ifdef wxHAS_PAINT_DEBUG
                    int g_isPainting = 0;
#endif 


wxIMPLEMENT_ABSTRACT_CLASS(wxWindowDCImpl, wxMSWDCImpl);

wxWindowDCImpl::wxWindowDCImpl( wxDC *owner ) :
   wxMSWDCImpl( owner )
{
}

wxWindowDCImpl::wxWindowDCImpl( wxDC *owner, wxWindow *window ) :
   wxMSWDCImpl( owner )
{
    wxCHECK_RET( window, wxT("invalid window in wxWindowDCImpl") );

    m_window = window;
    m_hDC = (WXHDC) ::GetWindowDC(GetHwndOf(m_window));

            InitDC();
}

void wxWindowDCImpl::InitDC()
{
            ::SetBkMode(GetHdc(), TRANSPARENT);

    #if wxUSE_PALETTE
    InitializePalette();
#endif
}

void wxWindowDCImpl::DoGetSize(int *width, int *height) const
{
    wxCHECK_RET( m_window, wxT("wxWindowDCImpl without a window?") );

    m_window->GetSize(width, height);
}


wxIMPLEMENT_ABSTRACT_CLASS(wxClientDCImpl, wxWindowDCImpl);

wxClientDCImpl::wxClientDCImpl( wxDC *owner ) :
   wxWindowDCImpl( owner )
{
}

wxClientDCImpl::wxClientDCImpl( wxDC *owner, wxWindow *window ) :
   wxWindowDCImpl( owner )
{
    wxCHECK_RET( window, wxT("invalid window in wxClientDCImpl") );

    m_window = window;
    m_hDC = (WXHDC)::GetDC(GetHwndOf(window));

        
    InitDC();
}

void wxClientDCImpl::InitDC()
{
    wxWindowDCImpl::InitDC();

                        #if defined(__WXUNIVERSAL__)
    wxPoint ptOrigin = m_window->GetClientAreaOrigin();
    if ( ptOrigin.x || ptOrigin.y )
    {
                SetDeviceOrigin(ptOrigin.x, ptOrigin.y);
    }

        wxSize size = m_window->GetClientSize();
    DoSetClippingRegion(0, 0, size.x, size.y);
#endif }

wxClientDCImpl::~wxClientDCImpl()
{
}

void wxClientDCImpl::DoGetSize(int *width, int *height) const
{
    wxCHECK_RET( m_window, wxT("wxClientDCImpl without a window?") );

    m_window->GetClientSize(width, height);
}


wxIMPLEMENT_ABSTRACT_CLASS(wxPaintDCImpl, wxClientDCImpl);

wxPaintDCImpl::wxPaintDCImpl( wxDC *owner ) :
   wxClientDCImpl( owner )
{
}

wxPaintDCImpl::wxPaintDCImpl( wxDC *owner, wxWindow *window ) :
   wxClientDCImpl( owner )
{
    wxCHECK_RET( window, wxT("NULL canvas in wxPaintDCImpl ctor") );

#ifdef wxHAS_PAINT_DEBUG
    if ( g_isPainting <= 0 )
    {
        wxFAIL_MSG( wxT("wxPaintDCImpl may be created only in EVT_PAINT handler!") );

        return;
    }
#endif 
        extern bool wxDidCreatePaintDC;

    wxDidCreatePaintDC = true;


    m_window = window;

        m_hDC = FindDCInCache(m_window);
    if ( !m_hDC )
    {
                wxPaintDCInfoOur* const info = new wxPaintDCInfoOur(m_window);
        gs_PaintDCInfos[m_window] = info;
        m_hDC = info->GetHDC();
    }

        if (!GetHDC())
        return;

        InitDC();

            m_clipping = true;
}

wxPaintDCImpl::~wxPaintDCImpl()
{
    if ( m_hDC )
    {
        SelectOldObjects(m_hDC);
        m_hDC = 0;
    }
}



wxPaintDCInfo *wxPaintDCImpl::FindInCache(wxWindow *win)
{
    PaintDCInfos::const_iterator it = gs_PaintDCInfos.find( win );

    return it != gs_PaintDCInfos.end() ? it->second : NULL;
}


WXHDC wxPaintDCImpl::FindDCInCache(wxWindow* win)
{
    wxPaintDCInfo* const info = FindInCache(win);

    return info ? info->GetHDC() : 0;
}


void wxPaintDCImpl::EndPaint(wxWindow *win)
{
    wxPaintDCInfo *info = FindInCache(win);
    if ( info )
    {
        gs_PaintDCInfos.erase(win);
        delete info;
    }
}

wxPaintDCInfo::~wxPaintDCInfo()
{
}



class wxPaintDCExImpl: public wxPaintDCImpl
{
public:
    wxPaintDCExImpl( wxDC *owner, wxWindow *window, WXHDC dc );
    ~wxPaintDCExImpl();
};


wxIMPLEMENT_ABSTRACT_CLASS(wxPaintDCEx, wxPaintDC);

wxPaintDCEx::wxPaintDCEx(wxWindow *window, WXHDC dc)
           : wxPaintDC(new wxPaintDCExImpl(this, window, dc))
{
}

wxPaintDCExImpl::wxPaintDCExImpl(wxDC *owner, wxWindow *window, WXHDC dc)
               : wxPaintDCImpl( owner )
{
    wxCHECK_RET( dc, wxT("wxPaintDCEx requires an existing device context") );

    m_window = window;
    m_hDC = dc;
}

wxPaintDCExImpl::~wxPaintDCExImpl()
{
        m_hDC = 0;
}
