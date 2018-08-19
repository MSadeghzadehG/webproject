
#include "wx/wxprec.h"

#include "wx/gtk/dcscreen.h"

#include <gtk/gtk.h>


wxIMPLEMENT_ABSTRACT_CLASS(wxScreenDCImpl, wxWindowDCImpl);

wxScreenDCImpl::wxScreenDCImpl( wxScreenDC *owner )
  : wxWindowDCImpl( owner )
{
    Init();
}

void wxScreenDCImpl::Init()
{
    m_ok = false;
    m_cmap = gdk_colormap_get_system();
    m_gdkwindow = gdk_get_default_root_window();

    m_context = gdk_pango_context_get();
            pango_context_set_language( m_context, gtk_get_default_language() );
    m_layout = pango_layout_new( m_context );

    m_isScreenDC = true;

    SetUpDC();

    gdk_gc_set_subwindow( m_penGC, GDK_INCLUDE_INFERIORS );
    gdk_gc_set_subwindow( m_brushGC, GDK_INCLUDE_INFERIORS );
    gdk_gc_set_subwindow( m_textGC, GDK_INCLUDE_INFERIORS );
    gdk_gc_set_subwindow( m_bgGC, GDK_INCLUDE_INFERIORS );
}

wxScreenDCImpl::~wxScreenDCImpl()
{
    g_object_unref(m_context);

    gdk_gc_set_subwindow( m_penGC, GDK_CLIP_BY_CHILDREN );
    gdk_gc_set_subwindow( m_brushGC, GDK_CLIP_BY_CHILDREN );
    gdk_gc_set_subwindow( m_textGC, GDK_CLIP_BY_CHILDREN );
    gdk_gc_set_subwindow( m_bgGC, GDK_CLIP_BY_CHILDREN );
}

void wxScreenDCImpl::DoGetSize(int *width, int *height) const
{
    wxDisplaySize(width, height);
}
