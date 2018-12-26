


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/nonownedwnd.h"
    #include "wx/dcclient.h"
    #include "wx/dcmemory.h"
    #include "wx/region.h"
#endif 
#include "wx/graphics.h"

#include <gtk/gtk.h>
#include "wx/gtk/private/gtk2-compat.h"


class wxNonOwnedWindowShapeImpl : public wxEvtHandler
{
public:
    wxNonOwnedWindowShapeImpl(wxWindow* win) : m_win(win)
    {
    }

    virtual ~wxNonOwnedWindowShapeImpl() { }

    bool SetShape()
    {
        if ( m_win->m_wxwindow )
            SetShapeIfNonNull(gtk_widget_get_window(m_win->m_wxwindow));

        return SetShapeIfNonNull(gtk_widget_get_window(m_win->m_widget));
    }

            virtual bool CanBeDeleted() const = 0;

protected:
    wxWindow* const m_win;

private:
        bool SetShapeIfNonNull(GdkWindow* window)
    {
        return window && DoSetShape(window);
    }

            virtual bool DoSetShape(GdkWindow* window) = 0;

    wxDECLARE_NO_COPY_CLASS(wxNonOwnedWindowShapeImpl);
};

class wxNonOwnedWindowShapeImplNone : public wxNonOwnedWindowShapeImpl
{
public:
    wxNonOwnedWindowShapeImplNone(wxWindow* win) :
        wxNonOwnedWindowShapeImpl(win)
    {
    }

    virtual bool CanBeDeleted() const wxOVERRIDE { return true; }

private:
    virtual bool DoSetShape(GdkWindow* window) wxOVERRIDE
    {
        gdk_window_shape_combine_region(window, NULL, 0, 0);

        return true;
    }
};

class wxNonOwnedWindowShapeImplRegion : public wxNonOwnedWindowShapeImpl
{
public:
    wxNonOwnedWindowShapeImplRegion(wxWindow* win, const wxRegion& region) :
        wxNonOwnedWindowShapeImpl(win),
        m_region(region)
    {
    }

    virtual bool CanBeDeleted() const wxOVERRIDE { return true; }

private:
    virtual bool DoSetShape(GdkWindow* window) wxOVERRIDE
    {
        gdk_window_shape_combine_region(window, m_region.GetRegion(), 0, 0);

        return true;
    }

    wxRegion m_region;
};

#if wxUSE_GRAPHICS_CONTEXT

class wxNonOwnedWindowShapeImplPath : public wxNonOwnedWindowShapeImpl
{
public:
    wxNonOwnedWindowShapeImplPath(wxWindow* win, const wxGraphicsPath& path) :
        wxNonOwnedWindowShapeImpl(win),
        m_path(path),
        m_mask(CreateShapeBitmap(path), *wxBLACK)
    {

        m_win->Connect
               (
                wxEVT_PAINT,
                wxPaintEventHandler(wxNonOwnedWindowShapeImplPath::OnPaint),
                NULL,
                this
               );
    }

    virtual ~wxNonOwnedWindowShapeImplPath()
    {
        m_win->Disconnect
               (
                wxEVT_PAINT,
                wxPaintEventHandler(wxNonOwnedWindowShapeImplPath::OnPaint),
                NULL,
                this
               );
    }

            virtual bool CanBeDeleted() const wxOVERRIDE { return false; }

private:
    wxBitmap CreateShapeBitmap(const wxGraphicsPath& path)
    {
                                                wxBitmap bmp(m_win->GetSize());

        wxMemoryDC dc(bmp);

        dc.SetBackground(*wxBLACK);
        dc.Clear();

#ifdef __WXGTK3__
        wxGraphicsContext* context = dc.GetGraphicsContext();
#else
        wxScopedPtr<wxGraphicsContext> context(wxGraphicsContext::Create(dc));
#endif
        context->SetBrush(*wxWHITE);
        context->FillPath(path);

        return bmp;
    }

    virtual bool DoSetShape(GdkWindow *window) wxOVERRIDE
    {
        if (!m_mask)
            return false;

#ifdef __WXGTK3__
        cairo_region_t* region = gdk_cairo_region_create_from_surface(m_mask);
        gdk_window_shape_combine_region(window, region, 0, 0);
        cairo_region_destroy(region);
#else
        gdk_window_shape_combine_mask(window, m_mask, 0, 0);
#endif

        return true;
    }

        void OnPaint(wxPaintEvent& event)
    {
        event.Skip();

        wxPaintDC dc(m_win);
#ifdef __WXGTK3__
        wxGraphicsContext* context = dc.GetGraphicsContext();
#else
        wxScopedPtr<wxGraphicsContext> context(wxGraphicsContext::Create(dc));
#endif
        context->SetPen(wxPen(*wxLIGHT_GREY, 2));
        context->StrokePath(m_path);
    }

    wxGraphicsPath m_path;
    wxMask m_mask;
};

#endif 

wxNonOwnedWindow::~wxNonOwnedWindow()
{
    delete m_shapeImpl;
}

void wxNonOwnedWindow::GTKHandleRealized()
{
    wxNonOwnedWindowBase::GTKHandleRealized();

    if ( m_shapeImpl )
    {
        m_shapeImpl->SetShape();

                                if ( m_shapeImpl->CanBeDeleted() )
        {
            delete m_shapeImpl;
            m_shapeImpl = NULL;
        }
    }
}

bool wxNonOwnedWindow::DoClearShape()
{
    if ( !m_shapeImpl )
    {
                return true;
    }

    if ( gtk_widget_get_realized(m_widget) )
    {
                wxNonOwnedWindowShapeImplNone data(this);
        data.SetShape();
    }
        
    delete m_shapeImpl;
    m_shapeImpl = NULL;

    return true;
}

bool wxNonOwnedWindow::DoSetRegionShape(const wxRegion& region)
{
        delete m_shapeImpl;
    m_shapeImpl = NULL;

    if ( gtk_widget_get_realized(m_widget) )
    {
                        wxNonOwnedWindowShapeImplRegion data(this, region);
        return data.SetShape();
    }
    else     {
        m_shapeImpl = new wxNonOwnedWindowShapeImplRegion(this, region);

                        return true;
    }
}

#if wxUSE_GRAPHICS_CONTEXT

bool wxNonOwnedWindow::DoSetPathShape(const wxGraphicsPath& path)
{
            
    delete m_shapeImpl;
    m_shapeImpl = new wxNonOwnedWindowShapeImplPath(this, path);

    if ( gtk_widget_get_realized(m_widget) )
    {
        return m_shapeImpl->SetShape();
    }
    
    return true;
}

#endif 