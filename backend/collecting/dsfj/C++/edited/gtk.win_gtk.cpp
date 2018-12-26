
#include "wx/wxprec.h"

#include "wx/defs.h"

#include <gtk/gtk.h>
#include "wx/gtk/private/win_gtk.h"

#include "wx/gtk/private.h"
#include "wx/gtk/private/gtk2-compat.h"



struct wxPizzaChild
{
    GtkWidget* widget;
    int x, y, width, height;
};

static GtkWidgetClass* parent_class;

#ifdef __WXGTK3__
enum {
    PROP_0,
    PROP_HADJUSTMENT,
    PROP_VADJUSTMENT,
    PROP_HSCROLL_POLICY,
    PROP_VSCROLL_POLICY
};
#endif

extern "C" {

struct wxPizzaClass
{
    GtkFixedClass parent;
#ifndef __WXGTK3__
    void (*set_scroll_adjustments)(GtkWidget*, GtkAdjustment*, GtkAdjustment*);
#endif
};

static void pizza_size_allocate(GtkWidget* widget, GtkAllocation* alloc)
{
    wxPizza* pizza = WX_PIZZA(widget);
    GtkBorder border;
    pizza->get_border(border);
    int w = alloc->width - border.left - border.right;
    if (w < 0) w = 0;

    if (gtk_widget_get_realized(widget))
    {
        int h = alloc->height - border.top - border.bottom;
        if (h < 0) h = 0;
        const int x = alloc->x + border.left;
        const int y = alloc->y + border.top;

        GdkWindow* window = gtk_widget_get_window(widget);
        int old_x, old_y;
        gdk_window_get_position(window, &old_x, &old_y);

        if (x != old_x || y != old_y ||
            w != gdk_window_get_width(window) || h != gdk_window_get_height(window))
        {
            gdk_window_move_resize(window, x, y, w, h);

            if (border.left + border.right + border.top + border.bottom)
            {
                                                GtkAllocation old_alloc;
                gtk_widget_get_allocation(widget, &old_alloc);
                GdkWindow* parent = gtk_widget_get_parent_window(widget);
                gdk_window_invalidate_rect(parent, &old_alloc, false);
                gdk_window_invalidate_rect(parent, alloc, false);
            }
        }
    }

    gtk_widget_set_allocation(widget, alloc);

        for (const GList* p = pizza->m_children; p; p = p->next)
    {
        const wxPizzaChild* child = static_cast<wxPizzaChild*>(p->data);
        if (gtk_widget_get_visible(child->widget))
        {
            GtkAllocation child_alloc;
                                                child_alloc.x = child->x - pizza->m_scroll_x;
            child_alloc.y = child->y - pizza->m_scroll_y;
            child_alloc.width  = child->width;
            child_alloc.height = child->height;
            if (gtk_widget_get_direction(widget) == GTK_TEXT_DIR_RTL)
                child_alloc.x = w - child_alloc.x - child_alloc.width;

            #ifdef __WXGTK3__
            if (child_alloc.width && child_alloc.height)
#endif
            {
                gtk_widget_size_allocate(child->widget, &child_alloc);
            }
        }
    }
}

static void pizza_realize(GtkWidget* widget)
{
    parent_class->realize(widget);

    wxPizza* pizza = WX_PIZZA(widget);
    if (pizza->m_windowStyle & wxPizza::BORDER_STYLES)
    {
        GtkBorder border;
        pizza->get_border(border);
        GtkAllocation a;
        gtk_widget_get_allocation(widget, &a);
        int x = a.x + border.left;
        int y = a.y + border.top;
        int w = a.width - border.left - border.right;
        int h = a.height - border.top - border.bottom;
        if (w < 0) w = 0;
        if (h < 0) h = 0;
        gdk_window_move_resize(gtk_widget_get_window(widget), x, y, w, h);
    }
}

static void pizza_show(GtkWidget* widget)
{
    GtkWidget* parent = gtk_widget_get_parent(widget);
    if (parent && (WX_PIZZA(widget)->m_windowStyle & wxPizza::BORDER_STYLES))
    {
                GtkAllocation a;
        gtk_widget_get_allocation(widget, &a);
        gtk_widget_queue_draw_area(parent, a.x, a.y, a.width, a.height);
    }

    parent_class->show(widget);
}

static void pizza_hide(GtkWidget* widget)
{
    GtkWidget* parent = gtk_widget_get_parent(widget);
    if (parent && (WX_PIZZA(widget)->m_windowStyle & wxPizza::BORDER_STYLES))
    {
                GtkAllocation a;
        gtk_widget_get_allocation(widget, &a);
        gtk_widget_queue_draw_area(parent, a.x, a.y, a.width, a.height);
    }

    parent_class->hide(widget);
}

static void pizza_add(GtkContainer* container, GtkWidget* widget)
{
    WX_PIZZA(container)->put(widget, 0, 0, 1, 1);
}

static void pizza_remove(GtkContainer* container, GtkWidget* widget)
{
    GTK_CONTAINER_CLASS(parent_class)->remove(container, widget);

    wxPizza* pizza = WX_PIZZA(container);
    for (GList* p = pizza->m_children; p; p = p->next)
    {
        wxPizzaChild* child = static_cast<wxPizzaChild*>(p->data);
        if (child->widget == widget)
        {
            pizza->m_children = g_list_delete_link(pizza->m_children, p);
            delete child;
            break;
        }
    }
}

#ifdef __WXGTK3__
static void children_get_preferred_size(const GList* p)
{
    for (; p; p = p->next)
    {
        const wxPizzaChild* child = static_cast<wxPizzaChild*>(p->data);
        if (gtk_widget_get_visible(child->widget))
        {
            GtkRequisition req;
            gtk_widget_get_preferred_size(child->widget, &req, NULL);
        }
    }
}

static void pizza_get_preferred_width(GtkWidget* widget, int* minimum, int* natural)
{
    children_get_preferred_size(WX_PIZZA(widget)->m_children);
    *minimum = 0;
    gtk_widget_get_size_request(widget, natural, NULL);
    if (*natural < 0)
        *natural = 0;
}

static void pizza_get_preferred_height(GtkWidget* widget, int* minimum, int* natural)
{
    children_get_preferred_size(WX_PIZZA(widget)->m_children);
    *minimum = 0;
    gtk_widget_get_size_request(widget, NULL, natural);
    if (*natural < 0)
        *natural = 0;
}

static void pizza_get_property(GObject*, guint, GValue*, GParamSpec*)
{
}

static void pizza_set_property(GObject*, guint, const GValue*, GParamSpec*)
{
}
#else
static void pizza_set_scroll_adjustments(GtkWidget*, GtkAdjustment*, GtkAdjustment*)
{
}

#define g_marshal_value_peek_object(v)   g_value_get_object (v)
static void
g_cclosure_user_marshal_VOID__OBJECT_OBJECT (GClosure     *closure,
                                             GValue       * ,
                                             guint         n_param_values,
                                             const GValue *param_values,
                                             gpointer      ,
                                             gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__OBJECT_OBJECT) (gpointer     data1,
                                                    gpointer     arg_1,
                                                    gpointer     arg_2,
                                                    gpointer     data2);
  GMarshalFunc_VOID__OBJECT_OBJECT callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__OBJECT_OBJECT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_object (param_values + 1),
            g_marshal_value_peek_object (param_values + 2),
            data2);
}
#endif

static void class_init(void* g_class, void*)
{
    GtkWidgetClass* widget_class = (GtkWidgetClass*)g_class;
    widget_class->size_allocate = pizza_size_allocate;
    widget_class->realize = pizza_realize;
    widget_class->show = pizza_show;
    widget_class->hide = pizza_hide;
    GtkContainerClass* container_class = (GtkContainerClass*)g_class;
    container_class->add = pizza_add;
    container_class->remove = pizza_remove;

#ifdef __WXGTK3__
    widget_class->get_preferred_width = pizza_get_preferred_width;
    widget_class->get_preferred_height = pizza_get_preferred_height;
    GObjectClass *gobject_class = G_OBJECT_CLASS(g_class);
    gobject_class->set_property = pizza_set_property;
    gobject_class->get_property = pizza_get_property;
    g_object_class_override_property(gobject_class, PROP_HADJUSTMENT, "hadjustment");
    g_object_class_override_property(gobject_class, PROP_VADJUSTMENT, "vadjustment");
    g_object_class_override_property(gobject_class, PROP_HSCROLL_POLICY, "hscroll-policy");
    g_object_class_override_property(gobject_class, PROP_VSCROLL_POLICY, "vscroll-policy");
#else
    wxPizzaClass* klass = static_cast<wxPizzaClass*>(g_class);
        klass->set_scroll_adjustments = pizza_set_scroll_adjustments;
    widget_class->set_scroll_adjustments_signal =
        g_signal_new(
            "set_scroll_adjustments",
            G_TYPE_FROM_CLASS(g_class),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET(wxPizzaClass, set_scroll_adjustments),
            NULL, NULL,
            g_cclosure_user_marshal_VOID__OBJECT_OBJECT,
            G_TYPE_NONE, 2, GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT);
#endif
    parent_class = GTK_WIDGET_CLASS(g_type_class_peek_parent(g_class));
}

} 
GType wxPizza::type()
{
    static GType type;
    if (type == 0)
    {
        const GTypeInfo info = {
            sizeof(wxPizzaClass),
            NULL, NULL,
            class_init,
            NULL, NULL,
            sizeof(wxPizza), 0,
            NULL, NULL
        };
        type = g_type_register_static(
            GTK_TYPE_FIXED, "wxPizza", &info, GTypeFlags(0));
#ifdef __WXGTK3__
        const GInterfaceInfo interface_info = { NULL, NULL, NULL };
        g_type_add_interface_static(type, GTK_TYPE_SCROLLABLE, &interface_info);
#endif
    }
    return type;
}

GtkWidget* wxPizza::New(long windowStyle)
{
    GtkWidget* widget = GTK_WIDGET(g_object_new(type(), NULL));
    wxPizza* pizza = WX_PIZZA(widget);
    pizza->m_children = NULL;
    pizza->m_scroll_x = 0;
    pizza->m_scroll_y = 0;
    pizza->m_windowStyle = windowStyle;
#ifdef __WXGTK3__
    gtk_widget_set_has_window(widget, true);
#else
    gtk_fixed_set_has_window(GTK_FIXED(widget), true);
#endif
    gtk_widget_add_events(widget,
        GDK_EXPOSURE_MASK |
        GDK_SCROLL_MASK |
        GDK_POINTER_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK |
        GDK_BUTTON_MOTION_MASK |
        GDK_BUTTON1_MOTION_MASK |
        GDK_BUTTON2_MOTION_MASK |
        GDK_BUTTON3_MOTION_MASK |
        GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK |
        GDK_KEY_PRESS_MASK |
        GDK_KEY_RELEASE_MASK |
        GDK_ENTER_NOTIFY_MASK |
        GDK_LEAVE_NOTIFY_MASK |
        GDK_FOCUS_CHANGE_MASK);
    return widget;
}

void wxPizza::move(GtkWidget* widget, int x, int y, int width, int height)
{
    for (const GList* p = m_children; p; p = p->next)
    {
        wxPizzaChild* child = static_cast<wxPizzaChild*>(p->data);
        if (child->widget == widget)
        {
            child->x = x;
            child->y = y;
            child->width = width;
            child->height = height;
                                    break;
        }
    }
}

void wxPizza::put(GtkWidget* widget, int x, int y, int width, int height)
{
            if (!gtk_widget_is_toplevel(GTK_WIDGET(widget)))
        gtk_fixed_put(GTK_FIXED(this), widget, 0, 0);

    wxPizzaChild* child = new wxPizzaChild;
    child->widget = widget;
    child->x = x;
    child->y = y;
    child->width = width;
    child->height = height;
    m_children = g_list_append(m_children, child);
}

struct AdjustData {
    GdkWindow* window;
    int dx, dy;
};

extern "C" {
static void scroll_adjust(GtkWidget* widget, void* data)
{
    const AdjustData* p = static_cast<AdjustData*>(data);
    GtkAllocation a;
    gtk_widget_get_allocation(widget, &a);
    a.x += p->dx;
    a.y += p->dy;
    gtk_widget_set_allocation(widget, &a);

    if (gtk_widget_get_window(widget) == p->window)
    {
                                gtk_widget_queue_resize_no_redraw(widget);
        if (GTK_IS_CONTAINER(widget))
            gtk_container_forall(GTK_CONTAINER(widget), scroll_adjust, data);
    }
}
}

void wxPizza::scroll(int dx, int dy)
{
    GtkWidget* widget = GTK_WIDGET(this);
    if (gtk_widget_get_direction(widget) == GTK_TEXT_DIR_RTL)
        dx = -dx;
    m_scroll_x -= dx;
    m_scroll_y -= dy;
    GdkWindow* window = gtk_widget_get_window(widget);
    if (window)
    {
        gdk_window_scroll(window, dx, dy);
                        AdjustData data = { window, dx, dy };
        gtk_container_forall(GTK_CONTAINER(widget), scroll_adjust, &data);
    }
}

void wxPizza::get_border(GtkBorder& border)
{
#ifndef __WXUNIVERSAL__
    if (m_windowStyle & wxBORDER_SIMPLE)
        border.left = border.right = border.top = border.bottom = 1;
    else if (m_windowStyle & (wxBORDER_RAISED | wxBORDER_SUNKEN | wxBORDER_THEME))
    {
#ifdef __WXGTK3__
        GtkStyleContext* sc;
        if (m_windowStyle & (wxHSCROLL | wxVSCROLL))
            sc = gtk_widget_get_style_context(wxGTKPrivate::GetTreeWidget());
        else
            sc = gtk_widget_get_style_context(wxGTKPrivate::GetEntryWidget());

        gtk_style_context_set_state(sc, GTK_STATE_FLAG_NORMAL);
        gtk_style_context_get_border(sc, GTK_STATE_FLAG_NORMAL, &border);
#else         GtkStyle* style;
        if (m_windowStyle & (wxHSCROLL | wxVSCROLL))
            style = gtk_widget_get_style(wxGTKPrivate::GetTreeWidget());
        else
            style = gtk_widget_get_style(wxGTKPrivate::GetEntryWidget());

        border.left = border.right = style->xthickness;
        border.top = border.bottom = style->ythickness;
#endif     }
    else
#endif     {
        border.left = border.right = border.top = border.bottom = 0;
    }
}
