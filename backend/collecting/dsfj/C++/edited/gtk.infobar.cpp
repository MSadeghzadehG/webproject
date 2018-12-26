


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/infobar.h"

#if wxUSE_INFOBAR && defined(wxHAS_NATIVE_INFOBAR)

#ifndef WX_PRECOMP
#endif 
#include "wx/vector.h"
#include "wx/stockitem.h"

#include "wx/gtk/private.h"
#include "wx/gtk/private/messagetype.h"


class wxInfoBarGTKImpl
{
public:
    wxInfoBarGTKImpl()
    {
        m_label = NULL;
        m_close = NULL;
    }

        GtkWidget *m_label;

            GtkWidget *m_close;

        struct Button
    {
        Button(GtkWidget *button_, int id_)
            : button(button_),
              id(id_)
        {
        }

        GtkWidget *button;
        int id;
    };
    typedef wxVector<Button> Buttons;

    Buttons m_buttons;
};


namespace
{

inline bool UseNative()
{
#ifdef __WXGTK3__
    return true;
#else
        return gtk_check_version(2, 18, 0) == 0;
#endif
}

} 
extern "C"
{

static void wxgtk_infobar_response(GtkInfoBar * WXUNUSED(infobar),
                                   gint btnid,
                                   wxInfoBar *win)
{
    win->GTKResponse(btnid);
}

static void wxgtk_infobar_close(GtkInfoBar * WXUNUSED(infobar),
                                wxInfoBar *win)
{
    win->GTKResponse(wxID_CANCEL);
}

} 

bool wxInfoBar::Create(wxWindow *parent, wxWindowID winid)
{
    if ( !UseNative() )
        return wxInfoBarGeneric::Create(parent, winid);

    m_impl = new wxInfoBarGTKImpl;

        Hide();
    if ( !CreateBase(parent, winid) )
        return false;

        m_widget = gtk_info_bar_new();
    wxCHECK_MSG( m_widget, false, "failed to create GtkInfoBar" );
    g_object_ref(m_widget);

        m_impl->m_label = gtk_label_new("");
    gtk_widget_show(m_impl->m_label);

    GtkWidget * const
        contentArea = gtk_info_bar_get_content_area(GTK_INFO_BAR(m_widget));
    wxCHECK_MSG( contentArea, false, "failed to get GtkInfoBar content area" );
    gtk_container_add(GTK_CONTAINER(contentArea), m_impl->m_label);

        m_parent->DoAddChild(this);

    PostCreation(wxDefaultSize);

    GTKConnectWidget("response", G_CALLBACK(wxgtk_infobar_response));
    GTKConnectWidget("close", G_CALLBACK(wxgtk_infobar_close));

    return true;
}

wxInfoBar::~wxInfoBar()
{
    delete m_impl;
}

void wxInfoBar::ShowMessage(const wxString& msg, int flags)
{
    if ( !UseNative() )
    {
        wxInfoBarGeneric::ShowMessage(msg, flags);
        return;
    }

            if ( m_impl->m_buttons.empty() && !m_impl->m_close )
    {
        m_impl->m_close = GTKAddButton(wxID_CLOSE);
    }

    GtkMessageType type;
    if ( wxGTKImpl::ConvertMessageTypeFromWX(flags, &type) )
        gtk_info_bar_set_message_type(GTK_INFO_BAR(m_widget), type);
    gtk_label_set_text(GTK_LABEL(m_impl->m_label), wxGTK_CONV(msg));

    if ( !IsShown() )
        Show();

    UpdateParent();
}

void wxInfoBar::Dismiss()
{
    if ( !UseNative() )
    {
        wxInfoBarGeneric::Dismiss();
        return;
    }

    Hide();

    UpdateParent();
}

void wxInfoBar::GTKResponse(int btnid)
{
    wxCommandEvent event(wxEVT_BUTTON, btnid);
    event.SetEventObject(this);

    if ( !HandleWindowEvent(event) )
        Dismiss();
}

GtkWidget *wxInfoBar::GTKAddButton(wxWindowID btnid, const wxString& label)
{
            InvalidateBestSize();

    GtkWidget *button = gtk_info_bar_add_button
                        (
                            GTK_INFO_BAR(m_widget),
                            (label.empty()
                                ? GTKConvertMnemonics(wxGetStockGtkID(btnid))
                                : label).utf8_str(),
                            btnid
                        );

    wxASSERT_MSG( button, "unexpectedly failed to add button to info bar" );

    return button;
}

size_t wxInfoBar::GetButtonCount() const
{
    if ( !UseNative() )
        return wxInfoBarGeneric::GetButtonCount();

    return m_impl->m_buttons.size();
}

wxWindowID wxInfoBar::GetButtonId(size_t idx) const
{
    if ( !UseNative() )
        return wxInfoBarGeneric::GetButtonId(idx);

    wxCHECK_MSG( idx < m_impl->m_buttons.size(), wxID_NONE,
                 "Invalid infobar button position" );

    return m_impl->m_buttons[idx].id;
}

void wxInfoBar::AddButton(wxWindowID btnid, const wxString& label)
{
    if ( !UseNative() )
    {
        wxInfoBarGeneric::AddButton(btnid, label);
        return;
    }

            if ( m_impl->m_close )
    {
        gtk_widget_destroy(m_impl->m_close);
        m_impl->m_close = NULL;
    }

    GtkWidget * const button = GTKAddButton(btnid, label);
    if ( button )
        m_impl->m_buttons.push_back(wxInfoBarGTKImpl::Button(button, btnid));
}

bool wxInfoBar::HasButtonId(wxWindowID btnid) const
{
    if ( !UseNative() )
        return wxInfoBarGeneric::HasButtonId(btnid);

        const wxInfoBarGTKImpl::Buttons& buttons = m_impl->m_buttons;
    for ( wxInfoBarGTKImpl::Buttons::const_reverse_iterator i = buttons.rbegin();
          i != buttons.rend();
          ++i )
    {
        if ( i->id == btnid )
            return true;
    }

    return false;
}

void wxInfoBar::RemoveButton(wxWindowID btnid)
{
    if ( !UseNative() )
    {
        wxInfoBarGeneric::RemoveButton(btnid);
        return;
    }

        wxInfoBarGTKImpl::Buttons& buttons = m_impl->m_buttons;
    for ( wxInfoBarGTKImpl::Buttons::reverse_iterator i = buttons.rbegin();
          i != buttons.rend();
          ++i )
    {
        if (i->id == btnid)
        {
            gtk_widget_destroy(i->button);
            buttons.erase(i.base());

                        InvalidateBestSize();

            return;
        }
    }

    wxFAIL_MSG( wxString::Format("button with id %d not found", btnid) );
}

void wxInfoBar::DoApplyWidgetStyle(GtkRcStyle *style)
{
    wxInfoBarGeneric::DoApplyWidgetStyle(style);

    if ( UseNative() )
        GTKApplyStyle(m_impl->m_label, style);
}

#endif 