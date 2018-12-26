
#include "wx/wxprec.h"

#if wxUSE_TOGGLEBTN

#include "wx/tglbtn.h"

#ifndef WX_PRECOMP
    #include "wx/button.h"
#endif

#include <gtk/gtk.h>
#include "wx/gtk/private.h"
#include "wx/gtk/private/eventsdisabler.h"
#include "wx/gtk/private/list.h"

extern bool      g_blockEventsOnDrag;

extern "C" {
static void gtk_togglebutton_clicked_callback(GtkWidget *WXUNUSED(widget), wxToggleButton *cb)
{
    if (g_blockEventsOnDrag)
        return;

        wxCommandEvent event(wxEVT_TOGGLEBUTTON, cb->GetId());
    event.SetInt(cb->GetValue());
    event.SetEventObject(cb);
    cb->HandleWindowEvent(event);
}
}

wxDEFINE_EVENT( wxEVT_TOGGLEBUTTON, wxCommandEvent );


wxIMPLEMENT_DYNAMIC_CLASS(wxBitmapToggleButton, wxToggleButton);

bool wxBitmapToggleButton::Create(wxWindow *parent, wxWindowID id,
                            const wxBitmap &bitmap, const wxPoint &pos,
                            const wxSize &size, long style,
                            const wxValidator& validator,
                            const wxString &name)
{
    if ( !wxToggleButton::Create(parent, id, wxEmptyString, pos, size, style | wxBU_NOTEXT | wxBU_EXACTFIT,
                                 validator, name) )
        return false;

    if ( bitmap.IsOk() )
    {
        SetBitmapLabel(bitmap);

                        SetInitialSize(size);
    }

    return true;
}



wxIMPLEMENT_DYNAMIC_CLASS(wxToggleButton, wxControl);

bool wxToggleButton::Create(wxWindow *parent, wxWindowID id,
                            const wxString &label, const wxPoint &pos,
                            const wxSize &size, long style,
                            const wxValidator& validator,
                            const wxString &name)
{
    if (!PreCreation(parent, pos, size) ||
        !CreateBase(parent, id, pos, size, style, validator, name ))
    {
        wxFAIL_MSG(wxT("wxToggleButton creation failed"));
        return false;
    }

                const bool
        useLabel = !(style & wxBU_NOTEXT) && !label.empty();
    if ( useLabel )
    {
        m_widget = gtk_toggle_button_new_with_mnemonic("");
    }
    else     {
        m_widget = gtk_toggle_button_new();

        GtkWidget *image = gtk_image_new();
        gtk_widget_show(image);
        gtk_container_add(GTK_CONTAINER(m_widget), image);
    }

    g_object_ref(m_widget);

    if ( useLabel )
        SetLabel(label);

    g_signal_connect (m_widget, "clicked",
                      G_CALLBACK (gtk_togglebutton_clicked_callback),
                      this);

    m_parent->DoAddChild(this);

    PostCreation(size);

    return true;
}

void wxToggleButton::GTKDisableEvents()
{
    g_signal_handlers_block_by_func(m_widget,
                                (gpointer) gtk_togglebutton_clicked_callback, this);
}

void wxToggleButton::GTKEnableEvents()
{
    g_signal_handlers_unblock_by_func(m_widget,
                                (gpointer) gtk_togglebutton_clicked_callback, this);
}

void wxToggleButton::SetValue(bool state)
{
    wxCHECK_RET(m_widget != NULL, wxT("invalid toggle button"));

    if (state == GetValue())
        return;

    wxGtkEventsDisabler<wxToggleButton> noEvents(this);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_widget), state);
}

bool wxToggleButton::GetValue() const
{
    wxCHECK_MSG(m_widget != NULL, false, wxT("invalid toggle button"));

    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_widget)) != 0;
}

void wxToggleButton::SetLabel(const wxString& label)
{
    wxCHECK_RET(m_widget != NULL, wxT("invalid toggle button"));

    wxAnyButton::SetLabel(label);

    if ( HasFlag(wxBU_NOTEXT) )
    {
                                return;
    }

    const wxString labelGTK = GTKConvertMnemonics(label);

    gtk_button_set_label(GTK_BUTTON(m_widget), wxGTK_CONV(labelGTK));

    GTKApplyWidgetStyle( false );
}

#if wxUSE_MARKUP
bool wxToggleButton::DoSetLabelMarkup(const wxString& markup)
{
    wxCHECK_MSG( m_widget != NULL, false, "invalid toggle button" );

    const wxString stripped = RemoveMarkup(markup);
    if ( stripped.empty() && !markup.empty() )
        return false;

    wxControl::SetLabel(stripped);

    if ( !HasFlag(wxBU_NOTEXT) )
    {
        GtkLabel * const label = GTKGetLabel();
        wxCHECK_MSG( label, false, "no label in this toggle button?" );

        GTKSetLabelWithMarkupForLabel(label, markup);
    }

    return true;
}
#endif 
GtkLabel *wxToggleButton::GTKGetLabel() const
{
    GtkWidget* child = gtk_bin_get_child(GTK_BIN(m_widget));
    return GTK_LABEL(child);
}

void wxToggleButton::DoApplyWidgetStyle(GtkRcStyle *style)
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

wxSize wxToggleButton::DoGetBestSize() const
{
    wxSize ret(wxAnyButton::DoGetBestSize());

    if (!HasFlag(wxBU_EXACTFIT))
    {
        if (ret.x < 80) ret.x = 80;
    }

    CacheBestSize(ret);
    return ret;
}

wxVisualAttributes
wxToggleButton::GetClassDefaultAttributes(wxWindowVariant WXUNUSED(variant))
{
    return GetDefaultAttributesFromGTKWidget(gtk_toggle_button_new());
}

#endif 