


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#endif 
#include "wx/nativewin.h"

#include <gtk/gtk.h>
#include "wx/gtk/private/gtk2-compat.h"

#ifdef GDK_WINDOWING_X11
    #include <gdk/gdkx.h>
#endif



bool
wxNativeWindow::Create(wxWindow* parent,
                       wxWindowID winid,
                       wxNativeWindowHandle widget)
{
    wxCHECK_MSG( widget, false, wxS("Invalid null GtkWidget") );

            if ( !CreateBase(parent, winid) )
        return false;

        m_widget = widget;
    g_object_ref(m_widget);

    parent->DoAddChild(this);

    PostCreation();

            GtkRequisition req;
    gtk_widget_get_preferred_size(widget, NULL, &req);
    SetInitialSize(wxSize(req.width, req.height));

    return true;
}

void wxNativeWindow::DoDisown()
{
    g_object_unref(m_widget);
}


#ifdef GDK_WINDOWING_X11

extern "C" GdkFilterReturn
wxNativeContainerWindowFilter(GdkXEvent *gdkxevent,
                              GdkEvent *event,
                              gpointer data)
{
    XEvent * const xevent = static_cast<XEvent *>(gdkxevent);
    if ( xevent->type == DestroyNotify )
    {
                gdk_window_remove_filter(event->any.window,
                                 wxNativeContainerWindowFilter, data);

                static_cast<wxNativeContainerWindow *>(data)->OnNativeDestroyed();
    }

    return GDK_FILTER_CONTINUE;
}

#endif 
bool wxNativeContainerWindow::Create(wxNativeContainerWindowHandle win)
{
    wxCHECK( win, false );

    if ( !wxTopLevelWindow::Create(NULL, wxID_ANY, "") )
        return false;

        gtk_widget_realize(m_widget);
    gdk_window_reparent(gtk_widget_get_window(m_widget), win, 0, 0);

#ifdef GDK_WINDOWING_X11
                        gdk_window_add_filter(gtk_widget_get_window(m_widget), wxNativeContainerWindowFilter, this);
#endif 
                    Show();

    return true;
}

bool wxNativeContainerWindow::Create(wxNativeContainerWindowId anid)
{
    bool rc;
#ifdef __WXGTK3__
#ifdef GDK_WINDOWING_X11
    GdkWindow * const win = gdk_x11_window_foreign_new_for_display(gdk_display_get_default(), anid);
#else
    GdkWindow * const win = NULL;
#endif
#else
    GdkWindow * const win = gdk_window_foreign_new(anid);
#endif
    if ( win )
    {
        rc = Create(win);
        g_object_unref(win);
    }
    else     {
        rc = false;
    }

    return rc;
}

void wxNativeContainerWindow::OnNativeDestroyed()
{
                            GTKDisconnect(m_widget);
    m_widget = NULL;

                    delete this;
}

wxNativeContainerWindow::~wxNativeContainerWindow()
{
                }
