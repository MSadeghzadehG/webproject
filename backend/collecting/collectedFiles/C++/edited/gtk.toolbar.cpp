
#include "wx/wxprec.h"

#if wxUSE_TOOLBAR_NATIVE

#include "wx/toolbar.h"

#include <gtk/gtk.h>
#include "wx/gtk/private.h"
#include "wx/gtk/private/gtk2-compat.h"


extern bool       g_blockEventsOnDrag;


class wxToolBarTool : public wxToolBarToolBase
{
public:
    wxToolBarTool(wxToolBar *tbar,
                  int id,
                  const wxString& label,
                  const wxBitmap& bitmap1,
                  const wxBitmap& bitmap2,
                  wxItemKind kind,
                  wxObject *clientData,
                  const wxString& shortHelpString,
                  const wxString& longHelpString)
        : wxToolBarToolBase(tbar, id, label, bitmap1, bitmap2, kind,
                            clientData, shortHelpString, longHelpString)
    {
        m_item = NULL;
    }

    wxToolBarTool(wxToolBar *tbar, wxControl *control, const wxString& label)
        : wxToolBarToolBase(tbar, control, label)
    {
        m_item = NULL;
    }

    void SetImage();
    void CreateDropDown();
    void ShowDropdown(GtkToggleButton* button);
    virtual void SetLabel(const wxString& label) wxOVERRIDE;

    GtkToolItem* m_item;
};


wxIMPLEMENT_DYNAMIC_CLASS(wxToolBar, wxControl);



extern "C" {
static void item_clicked(GtkToolButton*, wxToolBarTool* tool)
{
    if (g_blockEventsOnDrag) return;

    tool->GetToolBar()->OnLeftClick(tool->GetId(), false);
}
}


extern "C" {
static void item_toggled(GtkToggleToolButton* button, wxToolBarTool* tool)
{
    if (g_blockEventsOnDrag) return;

    const bool active = gtk_toggle_tool_button_get_active(button) != 0;
    tool->Toggle(active);
    if (!active && tool->GetKind() == wxITEM_RADIO)
        return;

    if (!tool->GetToolBar()->OnLeftClick(tool->GetId(), active))
    {
                tool->Toggle();
    }
}
}


extern "C" {
static gboolean
button_press_event(GtkWidget*, GdkEventButton* event, wxToolBarTool* tool)
{
    if (event->button != 3)
        return FALSE;

    if (g_blockEventsOnDrag) return TRUE;

    tool->GetToolBar()->OnRightClick(
        tool->GetId(), int(event->x), int(event->y));

    return TRUE;
}
}


extern "C" {
static void child_detached(GtkWidget*, GtkToolbar* toolbar, void*)
{
            gtk_toolbar_set_show_arrow(toolbar, false);
}
}


extern "C" {
static void child_attached(GtkWidget*, GtkToolbar* toolbar, void*)
{
    gtk_toolbar_set_show_arrow(toolbar, true);
}
}


extern "C" {
static gboolean
enter_notify_event(GtkWidget*, GdkEventCrossing* event, wxToolBarTool* tool)
{
    if (g_blockEventsOnDrag) return TRUE;

    int id = -1;
    if (event->type == GDK_ENTER_NOTIFY)
        id = tool->GetId();
    tool->GetToolBar()->OnMouseEnter(id);

    return FALSE;
}
}


extern "C" {
static gboolean
#ifdef __WXGTK3__
image_draw(GtkWidget* widget, cairo_t* cr, wxToolBarTool* tool)
#else
image_expose_event(GtkWidget* widget, GdkEventExpose*, wxToolBarTool* tool)
#endif
{
    const wxBitmap& bitmap = tool->GetDisabledBitmap();
    if (tool->IsEnabled() || !bitmap.IsOk())
        return false;

        GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);
    int x = (alloc.width - bitmap.GetWidth()) / 2;
    int y = (alloc.height - bitmap.GetHeight()) / 2;
#ifdef __WXGTK3__
    bitmap.Draw(cr, x, y);
#else
    x += alloc.x;
    y += alloc.y;
    gdk_draw_pixbuf(
        gtk_widget_get_window(widget), gtk_widget_get_style(widget)->black_gc, bitmap.GetPixbuf(),
        0, 0, x, y,
        -1, -1, GDK_RGB_DITHER_NORMAL, 0, 0);
#endif
    return true;
}
}


extern "C" {
static void arrow_toggled(GtkToggleButton* button, wxToolBarTool* tool)
{
    if (gtk_toggle_button_get_active(button))
    {
        tool->ShowDropdown(button);
        gtk_toggle_button_set_active(button, false);
    }
}
}


extern "C" {
static gboolean
arrow_button_press_event(GtkToggleButton* button, GdkEventButton* event, wxToolBarTool* tool)
{
    if (event->button == 1)
    {
        g_signal_handlers_block_by_func(button, (void*)arrow_toggled, tool);
        gtk_toggle_button_set_active(button, true);
        tool->ShowDropdown(button);
        gtk_toggle_button_set_active(button, false);
        g_signal_handlers_unblock_by_func(button, (void*)arrow_toggled, tool);
        return true;
    }
    return false;
}
}

void wxToolBar::AddChildGTK(wxWindowGTK* child)
{
    GtkWidget* align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_widget_show(align);
    gtk_container_add(GTK_CONTAINER(align), child->m_widget);
    GtkToolItem* item = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(item), align);
        gtk_toolbar_insert(GTK_TOOLBAR(gtk_bin_get_child(GTK_BIN(m_widget))), item, -1);
}


void wxToolBarTool::SetImage()
{
    const wxBitmap& bitmap = GetNormalBitmap();
    wxCHECK_RET(bitmap.IsOk(), "invalid bitmap for wxToolBar icon");

    GtkWidget* image = gtk_tool_button_get_icon_widget(GTK_TOOL_BUTTON(m_item));
            gtk_image_set_from_pixbuf(GTK_IMAGE(image), bitmap.GetPixbuf());
}

void wxToolBarTool::CreateDropDown()
{
    gtk_tool_item_set_homogeneous(m_item, false);
    GtkOrientation orient = GTK_ORIENTATION_HORIZONTAL;
    GtkArrowType arrowType = GTK_ARROW_DOWN;
    if (GetToolBar()->HasFlag(wxTB_LEFT | wxTB_RIGHT))
    {
        orient = GTK_ORIENTATION_VERTICAL;
        arrowType = GTK_ARROW_RIGHT;
    }
    GtkWidget* box = gtk_box_new(orient, 0);
    GtkWidget* arrow = gtk_arrow_new(arrowType, GTK_SHADOW_NONE);
    GtkWidget* tool_button = gtk_bin_get_child(GTK_BIN(m_item));
    g_object_ref(tool_button);
    gtk_container_remove(GTK_CONTAINER(m_item), tool_button);
    gtk_container_add(GTK_CONTAINER(box), tool_button);
    g_object_unref(tool_button);
    GtkWidget* arrow_button = gtk_toggle_button_new();
    gtk_button_set_relief(GTK_BUTTON(arrow_button),
        gtk_tool_item_get_relief_style(GTK_TOOL_ITEM(m_item)));
    gtk_container_add(GTK_CONTAINER(arrow_button), arrow);
    gtk_container_add(GTK_CONTAINER(box), arrow_button);
    gtk_widget_show_all(box);
    gtk_container_add(GTK_CONTAINER(m_item), box);

    g_signal_connect(arrow_button, "toggled", G_CALLBACK(arrow_toggled), this);
    g_signal_connect(arrow_button, "button_press_event",
        G_CALLBACK(arrow_button_press_event), this);
}

void wxToolBarTool::ShowDropdown(GtkToggleButton* button)
{
    wxToolBarBase* toolbar = GetToolBar();
    wxCommandEvent event(wxEVT_TOOL_DROPDOWN, GetId());
    if (!toolbar->HandleWindowEvent(event))
    {
        wxMenu* menu = GetDropdownMenu();
        if (menu)
        {
            GtkAllocation alloc;
            gtk_widget_get_allocation(GTK_WIDGET(button), &alloc);
            int x = alloc.x;
            int y = alloc.y;
            if (toolbar->HasFlag(wxTB_LEFT | wxTB_RIGHT))
                x += alloc.width;
            else
                y += alloc.height;
            toolbar->PopupMenu(menu, x, y);
        }
    }
}

void wxToolBarTool::SetLabel(const wxString& label)
{
    wxASSERT_MSG( IsButton(),
       wxS("Label can be set for button tool only") );

    if ( label == m_label )
        return;

    wxToolBarToolBase::SetLabel(label);
    if ( IsButton() )
    {
        if ( !label.empty() )
        {
            wxString newLabel = wxControl::RemoveMnemonics(label);
            gtk_tool_button_set_label(GTK_TOOL_BUTTON(m_item),
                                      wxGTK_CONV(newLabel));
                        gtk_tool_item_set_is_important(m_item, true);
        }
        else
        {
            gtk_tool_button_set_label(GTK_TOOL_BUTTON(m_item), NULL);
                        gtk_tool_item_set_is_important(m_item, false);
        }
    }

    }

wxToolBarToolBase *wxToolBar::CreateTool(int id,
                                         const wxString& text,
                                         const wxBitmap& bitmap1,
                                         const wxBitmap& bitmap2,
                                         wxItemKind kind,
                                         wxObject *clientData,
                                         const wxString& shortHelpString,
                                         const wxString& longHelpString)
{
    return new wxToolBarTool(this, id, text, bitmap1, bitmap2, kind,
                             clientData, shortHelpString, longHelpString);
}

wxToolBarToolBase *
wxToolBar::CreateTool(wxControl *control, const wxString& label)
{
    return new wxToolBarTool(this, control, label);
}


void wxToolBar::Init()
{
    m_toolbar = NULL;
    m_tooltips = NULL;
}

wxToolBar::~wxToolBar()
{
#ifndef __WXGTK3__
    if (m_tooltips)     {
        gtk_object_destroy(GTK_OBJECT(m_tooltips));
        g_object_unref(m_tooltips);
    }
#endif
}

bool wxToolBar::Create( wxWindow *parent,
                        wxWindowID id,
                        const wxPoint& pos,
                        const wxSize& size,
                        long style,
                        const wxString& name )
{
    if ( !PreCreation( parent, pos, size ) ||
         !CreateBase( parent, id, pos, size, style, wxDefaultValidator, name ))
    {
        wxFAIL_MSG( wxT("wxToolBar creation failed") );

        return false;
    }

    FixupStyle();

    m_toolbar = GTK_TOOLBAR( gtk_toolbar_new() );
#ifndef __WXGTK3__
    if (gtk_check_version(2, 12, 0))
    {
        m_tooltips = gtk_tooltips_new();
        g_object_ref(m_tooltips);
        gtk_object_sink(GTK_OBJECT(m_tooltips));
    }
#endif
    GtkSetStyle();

    if (style & wxTB_DOCKABLE)
    {
        m_widget = gtk_handle_box_new();

        g_signal_connect(m_widget, "child_detached",
            G_CALLBACK(child_detached), NULL);
        g_signal_connect(m_widget, "child_attached",
            G_CALLBACK(child_attached), NULL);

        if (style & wxTB_FLAT)
            gtk_handle_box_set_shadow_type( GTK_HANDLE_BOX(m_widget), GTK_SHADOW_NONE );
    }
    else
    {
        m_widget = gtk_event_box_new();
        ConnectWidget( m_widget );
    }
    g_object_ref(m_widget);
    gtk_container_add(GTK_CONTAINER(m_widget), GTK_WIDGET(m_toolbar));
    gtk_widget_show(GTK_WIDGET(m_toolbar));

    m_parent->DoAddChild( this );

    PostCreation(size);

    return true;
}

GdkWindow *wxToolBar::GTKGetWindow(wxArrayGdkWindows& WXUNUSED(windows)) const
{
    return gtk_widget_get_window(GTK_WIDGET(m_toolbar));
}

void wxToolBar::GtkSetStyle()
{
    GtkOrientation orient = GTK_ORIENTATION_HORIZONTAL;
    if (HasFlag(wxTB_LEFT | wxTB_RIGHT))
        orient = GTK_ORIENTATION_VERTICAL;

    GtkToolbarStyle style = GTK_TOOLBAR_ICONS;
    if (HasFlag(wxTB_NOICONS))
        style = GTK_TOOLBAR_TEXT;
    else if (HasFlag(wxTB_TEXT))
    {
        style = GTK_TOOLBAR_BOTH;
        if (HasFlag(wxTB_HORZ_LAYOUT))
            style = GTK_TOOLBAR_BOTH_HORIZ;
    }

#ifdef __WXGTK3__
    gtk_orientable_set_orientation(GTK_ORIENTABLE(m_toolbar), orient);
#else
    gtk_toolbar_set_orientation(m_toolbar, orient);
#endif
    gtk_toolbar_set_style(m_toolbar, style);
}

void wxToolBar::SetWindowStyleFlag( long style )
{
    wxToolBarBase::SetWindowStyleFlag(style);

    if ( m_toolbar )
        GtkSetStyle();
}

bool wxToolBar::Realize()
{
    if ( !wxToolBarBase::Realize() )
        return false;

                        for ( wxToolBarToolsList::const_iterator i = m_tools.begin();
          i != m_tools.end();
          ++i )
    {
                                if ( !(*i)->IsEnabled() )
            DoEnableTool(*i, false);
        if ( (*i)->IsToggled() )
            DoToggleTool(*i, true);
    }

    return true;
}

bool wxToolBar::DoInsertTool(size_t pos, wxToolBarToolBase *toolBase)
{
    wxToolBarTool* tool = static_cast<wxToolBarTool*>(toolBase);

    GSList* radioGroup;
    GtkWidget* bin_child;
    switch ( tool->GetStyle() )
    {
        case wxTOOL_STYLE_BUTTON:
            switch (tool->GetKind())
            {
                case wxITEM_CHECK:
                    tool->m_item = gtk_toggle_tool_button_new();
                    g_signal_connect(tool->m_item, "toggled",
                        G_CALLBACK(item_toggled), tool);
                    break;
                case wxITEM_RADIO:
                    radioGroup = GetRadioGroup(pos);
                    if (!radioGroup)
                    {
                                                                                                tool->Toggle(true);
                    }
                    tool->m_item = gtk_radio_tool_button_new(radioGroup);
                    g_signal_connect(tool->m_item, "toggled",
                        G_CALLBACK(item_toggled), tool);
                    break;
                default:
                    wxFAIL_MSG("unknown toolbar child type");
                                    case wxITEM_DROPDOWN:
                case wxITEM_NORMAL:
                    tool->m_item = gtk_tool_button_new(NULL, "");
                    g_signal_connect(tool->m_item, "clicked",
                        G_CALLBACK(item_clicked), tool);
                    break;
            }
            if (!HasFlag(wxTB_NOICONS))
            {
                GtkWidget* image = gtk_image_new();
                gtk_tool_button_set_icon_widget(
                    GTK_TOOL_BUTTON(tool->m_item), image);
                tool->SetImage();
                gtk_widget_show(image);
#ifdef __WXGTK3__
                g_signal_connect(image, "draw",
                    G_CALLBACK(image_draw), tool);
#else
                g_signal_connect(image, "expose_event",
                    G_CALLBACK(image_expose_event), tool);
#endif
            }
            if (!tool->GetLabel().empty())
            {
                wxString const
                    label = wxControl::RemoveMnemonics(tool->GetLabel());

                gtk_tool_button_set_label(
                    GTK_TOOL_BUTTON(tool->m_item), wxGTK_CONV(label));
                                gtk_tool_item_set_is_important(tool->m_item, true);
            }
            if (!HasFlag(wxTB_NO_TOOLTIPS) && !tool->GetShortHelp().empty())
            {
#if GTK_CHECK_VERSION(2, 12, 0)
                if (GTK_CHECK_VERSION(3,0,0) || gtk_check_version(2,12,0) == NULL)
                {
                    gtk_tool_item_set_tooltip_text(tool->m_item,
                        wxGTK_CONV(tool->GetShortHelp()));
                }
                else
#endif
                {
#ifndef __WXGTK3__
                    gtk_tool_item_set_tooltip(tool->m_item,
                        m_tooltips, wxGTK_CONV(tool->GetShortHelp()), "");
#endif
                }
            }
            bin_child = gtk_bin_get_child(GTK_BIN(tool->m_item));
            g_signal_connect(bin_child, "button_press_event",
                G_CALLBACK(button_press_event), tool);
            g_signal_connect(bin_child, "enter_notify_event",
                G_CALLBACK(enter_notify_event), tool);
            g_signal_connect(bin_child, "leave_notify_event",
                G_CALLBACK(enter_notify_event), tool);

            if (tool->GetKind() == wxITEM_DROPDOWN)
                tool->CreateDropDown();
            gtk_toolbar_insert(m_toolbar, tool->m_item, int(pos));
            break;

        case wxTOOL_STYLE_SEPARATOR:
            tool->m_item = gtk_separator_tool_item_new();
            if ( tool->IsStretchable() )
            {
                gtk_separator_tool_item_set_draw
                (
                    GTK_SEPARATOR_TOOL_ITEM(tool->m_item),
                    FALSE
                );
                gtk_tool_item_set_expand(tool->m_item, TRUE);
            }
            gtk_toolbar_insert(m_toolbar, tool->m_item, int(pos));
            break;

        case wxTOOL_STYLE_CONTROL:
            wxWindow* control = tool->GetControl();
            if (gtk_widget_get_parent(control->m_widget) == NULL)
                AddChildGTK(control);
            tool->m_item = GTK_TOOL_ITEM(gtk_widget_get_parent(gtk_widget_get_parent(control->m_widget)));
            if (gtk_toolbar_get_item_index(m_toolbar, tool->m_item) != int(pos))
            {
                g_object_ref(tool->m_item);
                gtk_container_remove(
                    GTK_CONTAINER(m_toolbar), GTK_WIDGET(tool->m_item));
                gtk_toolbar_insert(m_toolbar, tool->m_item, int(pos));
                g_object_unref(tool->m_item);
            }
            break;
    }
    gtk_widget_show(GTK_WIDGET(tool->m_item));

    InvalidateBestSize();

    return true;
}

bool wxToolBar::DoDeleteTool(size_t , wxToolBarToolBase* toolBase)
{
    wxToolBarTool* tool = static_cast<wxToolBarTool*>(toolBase);

    if (tool->GetStyle() == wxTOOL_STYLE_CONTROL)
    {
                                        GtkWidget* widget = tool->GetControl()->m_widget;
        gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(widget)), widget);
    }
    gtk_widget_destroy(GTK_WIDGET(tool->m_item));
    tool->m_item = NULL;

    InvalidateBestSize();
    return true;
}

GSList* wxToolBar::GetRadioGroup(size_t pos)
{
    GSList* radioGroup = NULL;
    GtkToolItem* item = NULL;
    if (pos > 0)
    {
        item = gtk_toolbar_get_nth_item(m_toolbar, int(pos) - 1);
        if (!GTK_IS_RADIO_TOOL_BUTTON(item))
            item = NULL;
    }
    if (item == NULL && pos < m_tools.size())
    {
        item = gtk_toolbar_get_nth_item(m_toolbar, int(pos));
        if (!GTK_IS_RADIO_TOOL_BUTTON(item))
            item = NULL;
    }
    if (item)
        radioGroup = gtk_radio_tool_button_get_group((GtkRadioToolButton*)item);
    return radioGroup;
}


void wxToolBar::DoEnableTool(wxToolBarToolBase *toolBase, bool enable)
{
    wxToolBarTool* tool = static_cast<wxToolBarTool*>(toolBase);

    if (tool->m_item)
        gtk_widget_set_sensitive(GTK_WIDGET(tool->m_item), enable);
}

void wxToolBar::DoToggleTool( wxToolBarToolBase *toolBase, bool toggle )
{
    wxToolBarTool* tool = static_cast<wxToolBarTool*>(toolBase);

    if (tool->m_item)
    {
        g_signal_handlers_block_by_func(tool->m_item, (void*)item_toggled, tool);

        gtk_toggle_tool_button_set_active(
            GTK_TOGGLE_TOOL_BUTTON(tool->m_item), toggle);

        g_signal_handlers_unblock_by_func(tool->m_item, (void*)item_toggled, tool);
    }
}

void wxToolBar::DoSetToggle(wxToolBarToolBase * WXUNUSED(tool),
                            bool WXUNUSED(toggle))
{
        wxFAIL_MSG( wxT("not implemented") );
}


wxSize wxToolBar::DoGetBestSize() const
{
                        gtk_toolbar_set_show_arrow(m_toolbar, false);
    const wxSize size = wxToolBarBase::DoGetBestSize();
    gtk_toolbar_set_show_arrow(m_toolbar, true);
    return size;
}

wxToolBarToolBase *wxToolBar::FindToolForPosition(wxCoord WXUNUSED(x),
                                                  wxCoord WXUNUSED(y)) const
{
        wxFAIL_MSG( wxT("wxToolBar::FindToolForPosition() not implemented") );

    return NULL;
}

void wxToolBar::SetToolShortHelp( int id, const wxString& helpString )
{
    wxToolBarTool* tool = static_cast<wxToolBarTool*>(FindById(id));

    if ( tool )
    {
        (void)tool->SetShortHelp(helpString);
        if (tool->m_item)
        {
#if GTK_CHECK_VERSION(2, 12, 0)
            if (GTK_CHECK_VERSION(3,0,0) || gtk_check_version(2,12,0) == NULL)
            {
                gtk_tool_item_set_tooltip_text(tool->m_item,
                    wxGTK_CONV(helpString));
            }
            else
#endif
            {
#ifndef __WXGTK3__
                gtk_tool_item_set_tooltip(tool->m_item,
                    m_tooltips, wxGTK_CONV(helpString), "");
#endif
            }
        }
    }
}

void wxToolBar::SetToolNormalBitmap( int id, const wxBitmap& bitmap )
{
    wxToolBarTool* tool = static_cast<wxToolBarTool*>(FindById(id));
    if ( tool )
    {
        wxCHECK_RET( tool->IsButton(), wxT("Can only set bitmap on button tools."));

        tool->SetNormalBitmap(bitmap);
        tool->SetImage();
    }
}

void wxToolBar::SetToolDisabledBitmap( int id, const wxBitmap& bitmap )
{
    wxToolBarTool* tool = static_cast<wxToolBarTool*>(FindById(id));
    if ( tool )
    {
        wxCHECK_RET( tool->IsButton(), wxT("Can only set bitmap on button tools."));

        tool->SetDisabledBitmap(bitmap);
    }
}


wxVisualAttributes
wxToolBar::GetClassDefaultAttributes(wxWindowVariant WXUNUSED(variant))
{
    return GetDefaultAttributesFromGTKWidget(gtk_toolbar_new());
}

#endif 