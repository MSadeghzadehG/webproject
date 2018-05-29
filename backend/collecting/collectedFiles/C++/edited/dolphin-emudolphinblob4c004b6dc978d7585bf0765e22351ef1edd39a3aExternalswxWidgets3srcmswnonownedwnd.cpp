


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/dcclient.h"
    #include "wx/frame.h"           #include "wx/region.h"
    #include "wx/msw/private.h"
#endif 
#include "wx/nonownedwnd.h"

#if wxUSE_GRAPHICS_CONTEXT
    #include "wx/msw/wrapgdip.h"
    #include "wx/graphics.h"
#endif 
#include "wx/scopedptr.h"


bool wxNonOwnedWindow::DoClearShape()
{
    if (::SetWindowRgn(GetHwnd(), NULL, TRUE) == 0)
    {
        wxLogLastError(wxT("SetWindowRgn"));
        return false;
    }

    return true;
}

bool wxNonOwnedWindow::DoSetRegionShape(const wxRegion& region)
{
            DWORD noBytes = ::GetRegionData(GetHrgnOf(region), 0, NULL);
    RGNDATA *rgnData = (RGNDATA*) new char[noBytes];
    ::GetRegionData(GetHrgnOf(region), noBytes, rgnData);
    HRGN hrgn = ::ExtCreateRegion(NULL, noBytes, rgnData);
    delete[] (char*) rgnData;

            const wxPoint clientOrigin = GetClientAreaOrigin();
    ::OffsetRgn(hrgn, -clientOrigin.x, -clientOrigin.y);

        if (::SetWindowRgn(GetHwnd(), hrgn, TRUE) == 0)
    {
        wxLogLastError(wxT("SetWindowRgn"));
        return false;
    }
    return true;
}

#if wxUSE_GRAPHICS_CONTEXT

#include "wx/msw/wrapgdip.h"

class wxNonOwnedWindowShapeImpl : public wxEvtHandler
{
public:
    wxNonOwnedWindowShapeImpl(wxNonOwnedWindow* win, const wxGraphicsPath& path) :
        m_win(win),
        m_path(path)
    {
                        wxScopedPtr<wxGraphicsContext> context(wxGraphicsContext::Create(win));
        Region gr(static_cast<GraphicsPath*>(m_path.GetNativePath()));
        win->SetShape(
            wxRegion(
                gr.GetHRGN(static_cast<Graphics*>(context->GetNativeContext()))
            )
        );


                                m_win->Connect
               (
                wxEVT_PAINT,
                wxPaintEventHandler(wxNonOwnedWindowShapeImpl::OnPaint),
                NULL,
                this
               );
    }

    virtual ~wxNonOwnedWindowShapeImpl()
    {
        m_win->Disconnect
               (
                wxEVT_PAINT,
                wxPaintEventHandler(wxNonOwnedWindowShapeImpl::OnPaint),
                NULL,
                this
               );
    }

private:
    void OnPaint(wxPaintEvent& event)
    {
        event.Skip();

        wxPaintDC dc(m_win);
        wxScopedPtr<wxGraphicsContext> context(wxGraphicsContext::Create(dc));
        context->SetPen(wxPen(*wxLIGHT_GREY, 2));
        context->StrokePath(m_path);
    }

    wxNonOwnedWindow* const m_win;
    wxGraphicsPath m_path;

    wxDECLARE_NO_COPY_CLASS(wxNonOwnedWindowShapeImpl);
};

wxNonOwnedWindow::wxNonOwnedWindow()
{
    m_shapeImpl = NULL;
}

wxNonOwnedWindow::~wxNonOwnedWindow()
{
    delete m_shapeImpl;
}

bool wxNonOwnedWindow::DoSetPathShape(const wxGraphicsPath& path)
{
    delete m_shapeImpl;
    m_shapeImpl = new wxNonOwnedWindowShapeImpl(this, path);

    return true;
}

#else 
wxNonOwnedWindow::wxNonOwnedWindow()
{
}

wxNonOwnedWindow::~wxNonOwnedWindow()
{
}

#endif 