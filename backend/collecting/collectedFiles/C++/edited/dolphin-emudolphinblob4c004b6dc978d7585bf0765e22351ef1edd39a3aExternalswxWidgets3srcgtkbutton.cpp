
#include "wx/wxprec.h"

#if wxUSE_BUTTON

#ifndef WX_PRECOMP
    #include "wx/button.h"
#endif

#include "wx/stockitem.h"

#include <gtk/gtk.h>
#include "wx/gtk/private.h"
#include "wx/gtk/private/gtk2-compat.h"
#include "wx/gtk/private/list.h"


extern "C"
{

static void
wxgtk_button_clicked_callback(GtkWidget *WXUNUSED(widget), wxButton *button)
{
    if ( button->GTKShouldIgnoreEvent() )
        return;

    wxCommandEvent event(wxEVT_BUTTON, button->GetId());
    event.SetEventObject(button);
    button->HandleWindowEvent(event);
}


static void
wxgtk_button_style_set_callback(GtkWidget* widget, GtkStyle*, wxButton* win)
{
    
    wxWindow* parent = win->GetParent();
    if (parent && parent->m_wxwindow && gtk_widget_get_can_default(widget))
    {
        GtkBorder* border = NULL;
        gtk_widget_style_get(widget, "default_border", &border, NULL);
        if (border)
        {
            win->MoveWindow(
                win->m_x - border->left,
                win->m_y - border->top,
                win->m_width + border->left + border->right,
                win->m_height + border->top + border->bottom);
            gtk_border_free(border);
        }
    }
}

} 

bool wxButton::Create(wxWindow *parent,
                      wxWindowID id,
                      const wxString &label,
                      const wxPoint& pos,
                      const wxSize& size,
                      long style,
                      const wxValidator& validator,
                      const wxString& name)
{
    if (!PreCreation( parent, pos, size ) ||
        !CreateBase( parent, id, pos, size, style, validator, name ))
    {
        wxFAIL_MSG( wxT("wxButton creation failed") );
        return false;
    }

                const bool
        useLabel = !(style & wxBU_NOTEXT) && (!label.empty() || wxIsStockID(id));
    if ( useLabel )
    {
        m_widget = gtk_button_new_with_mnemonic("");
    }
    else     {
        m_widget = gtk_button_new();

        GtkWidget *image = gtk_image_new();
        gtk_widget_show(image);
        gtk_container_add(GTK_CONTAINER(m_widget), image);
    }

    g_object_ref(m_widget);

    float x_alignment = 0.5;
    if (HasFlag(wxBU_LEFT))
        x_alignment = 0.0;
    else if (HasFlag(wxBU_RIGHT))
        x_alignment = 1.0;

    float y_alignment = 0.5;
    if (HasFlag(wxBU_TOP))
        y_alignment = 0.0;
    else if (HasFlag(wxBU_BOTTOM))
        y_alignment = 1.0;

    gtk_button_set_alignment(GTK_BUTTON(m_widget), x_alignment, y_alignment);

    if ( useLabel )
        SetLabel(label);

    if (style & wxNO_BORDER)
       gtk_button_set_relief( GTK_BUTTON(m_widget), GTK_RELIEF_NONE );

    g_signal_connect_after (m_widget, "clicked",
                            G_CALLBACK (wxgtk_button_clicked_callback),
                            this);

    g_signal_connect_after (m_widget, "style_set",
                            G_CALLBACK (wxgtk_button_style_set_callback),
                            this);

    m_parent->DoAddChild( this );

    PostCreation(size);

    return true;
}


wxWindow *wxButton::SetDefault()
{
    wxWindow *oldDefault = wxButtonBase::SetDefault();

    gtk_widget_set_can_default(m_widget, TRUE);
    gtk_widget_grab_default( m_widget );

        wxgtk_button_style_set_callback( m_widget, NULL, this );

    return oldDefault;
}


wxSize wxButtonBase::GetDefaultSize()
{
    static wxSize size = wxDefaultSize;
    if (size == wxDefaultSize)
    {
                                                
        GtkWidget *wnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        GtkWidget *box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
        GtkWidget *btn = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
        gtk_container_add(GTK_CONTAINER(box), btn);
        gtk_container_add(GTK_CONTAINER(wnd), box);
        GtkRequisition req;
        gtk_widget_get_preferred_size(btn, NULL, &req);

        gint minwidth, minheight;
        gtk_widget_style_get(box,
                             "child-min-width", &minwidth,
                             "child-min-height", &minheight,
                             NULL);

        size.x = wxMax(minwidth, req.width);
        size.y = wxMax(minheight, req.height);

        gtk_widget_destroy(wnd);
    }
    return size;
}

void wxButton::SetLabel( const wxString &lbl )
{
    wxCHECK_RET( m_widget != NULL, wxT("invalid button") );

    wxString label(lbl);

    if (label.empty() && wxIsStockID(m_windowId))
        label = wxGetStockLabel(m_windowId);

    wxAnyButton::SetLabel(label);

        if ( HasFlag(wxBU_NOTEXT) )
        return;

    if (wxIsStockID(m_windowId) && wxIsStockLabel(m_windowId, label))
    {
        const char *stock = wxGetStockGtkID(m_windowId);
        if (stock)
        {
            gtk_button_set_label(GTK_BUTTON(m_widget), stock);
            gtk_button_set_use_stock(GTK_BUTTON(m_widget), TRUE);
            return;
        }
    }

                gtk_button_set_use_underline(GTK_BUTTON(m_widget), TRUE);
    const wxString labelGTK = GTKConvertMnemonics(label);
    gtk_button_set_label(GTK_BUTTON(m_widget), wxGTK_CONV(labelGTK));
    gtk_button_set_use_stock(GTK_BUTTON(m_widget), FALSE);

    GTKApplyWidgetStyle( false );
}

#if wxUSE_MARKUP
bool wxButton::DoSetLabelMarkup(const wxString& markup)
{
    wxCHECK_MSG( m_widget != NULL, false, "invalid button" );

    const wxString stripped = RemoveMarkup(markup);
    if ( stripped.empty() && !markup.empty() )
        return false;

    wxControl::SetLabel(stripped);

    GtkLabel * const label = GTKGetLabel();
    wxCHECK_MSG( label, false, "no label in this button?" );

    GTKSetLabelWithMarkupForLabel(label, markup);

    return true;
}

GtkLabel *wxButton::GTKGetLabel() const
{
    GtkWidget* child = gtk_bin_get_child(GTK_BIN(m_widget));
    if ( GTK_IS_ALIGNMENT(child) )
    {
        GtkWidget* box = gtk_bin_get_child(GTK_BIN(child));
        GtkLabel* label = NULL;
        wxGtkList list(gtk_container_get_children(GTK_CONTAINER(box)));
        for (GList* item = list; item; item = item->next)
        {
            if (GTK_IS_LABEL(item->data))
                label = GTK_LABEL(item->data);
        }

        return label;
    }

    return GTK_LABEL(child);
}
#endif 
void wxButton::DoApplyWidgetStyle(GtkRcStyle *style)
{
    GTKApplyStyle(m_widget, style);
    GtkWidget* child = gtk_bin_get_child(GTK_BIN(m_widget));
    GTKApplyStyle(child, style);

            if ( GTK_IS_ALIGNMENT(child) )
    {
        GtkWidget* box = gtk_bin_get_child(GTK_BIN(child));
        if ( GTK_IS_BOX(box) )
        {
            wxGtkList list(gtk_container_get_children(GTK_CONTAINER(box)));
            for (GList* item = list; item; item = item->next)
            {
                GTKApplyStyle(GTK_WIDGET(item->data), style);
            }
        }
    }
}

wxSize wxButton::DoGetBestSize() const
{
                    const bool isDefault = gtk_widget_has_default(m_widget) != 0;
    if ( isDefault )
    {
                gtk_widget_set_can_default(m_widget, FALSE);
    }

    wxSize ret( wxAnyButton::DoGetBestSize() );

    if ( isDefault )
    {
                gtk_widget_set_can_default(m_widget, TRUE);
    }

    if (!HasFlag(wxBU_EXACTFIT))
    {
        wxSize defaultSize = GetDefaultSize();
        if (ret.x < defaultSize.x)
            ret.x = defaultSize.x;
        if (ret.y < defaultSize.y)
            ret.y = defaultSize.y;
    }

    CacheBestSize(ret);
    return ret;
}

wxVisualAttributes
wxButton::GetClassDefaultAttributes(wxWindowVariant WXUNUSED(variant))
{
    return GetDefaultAttributesFromGTKWidget(gtk_button_new());
}

#endif 