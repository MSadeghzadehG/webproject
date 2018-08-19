
#include "wx/wxprec.h"

#ifdef wxHAS_ANY_BUTTON

#ifndef WX_PRECOMP
    #include "wx/anybutton.h"
#endif

#include "wx/stockitem.h"

#include <gtk/gtk.h>
#include "wx/gtk/private/gtk2-compat.h"


extern "C"
{

static void
wxgtk_button_enter_callback(GtkWidget *WXUNUSED(widget), wxAnyButton *button)
{
    if ( button->GTKShouldIgnoreEvent() )
        return;

    button->GTKMouseEnters();
}

static void
wxgtk_button_leave_callback(GtkWidget *WXUNUSED(widget), wxAnyButton *button)
{
    if ( button->GTKShouldIgnoreEvent() )
        return;

    button->GTKMouseLeaves();
}

static void
wxgtk_button_press_callback(GtkWidget *WXUNUSED(widget), wxAnyButton *button)
{
    if ( button->GTKShouldIgnoreEvent() )
        return;

    button->GTKPressed();
}

static void
wxgtk_button_released_callback(GtkWidget *WXUNUSED(widget), wxAnyButton *button)
{
    if ( button->GTKShouldIgnoreEvent() )
        return;

    button->GTKReleased();
}

} 

bool wxAnyButton::Enable( bool enable )
{
    if (!base_type::Enable(enable))
        return false;

    gtk_widget_set_sensitive(gtk_bin_get_child(GTK_BIN(m_widget)), enable);

    if (enable)
        GTKFixSensitivity();

    GTKUpdateBitmap();

    return true;
}

GdkWindow *wxAnyButton::GTKGetWindow(wxArrayGdkWindows& WXUNUSED(windows)) const
{
    return gtk_button_get_event_window(GTK_BUTTON(m_widget));
}

wxVisualAttributes
wxAnyButton::GetClassDefaultAttributes(wxWindowVariant WXUNUSED(variant))
{
    return GetDefaultAttributesFromGTKWidget(gtk_button_new());
}


void wxAnyButton::GTKMouseEnters()
{
    m_isCurrent = true;

    GTKUpdateBitmap();
}

void wxAnyButton::GTKMouseLeaves()
{
    m_isCurrent = false;

    GTKUpdateBitmap();
}

void wxAnyButton::GTKPressed()
{
    m_isPressed = true;

    GTKUpdateBitmap();
}

void wxAnyButton::GTKReleased()
{
    m_isPressed = false;

    GTKUpdateBitmap();
}

void wxAnyButton::GTKOnFocus(wxFocusEvent& event)
{
    event.Skip();

    GTKUpdateBitmap();
}

wxAnyButton::State wxAnyButton::GTKGetCurrentBitmapState() const
{
    if ( !IsThisEnabled() )
    {
        if ( m_bitmaps[State_Disabled].IsOk() )
            return State_Disabled;
    }
    else
    {
        if ( m_isPressed && m_bitmaps[State_Pressed].IsOk() )
            return State_Pressed;

        if ( m_isCurrent && m_bitmaps[State_Current].IsOk() )
            return State_Current;

        if ( HasFocus() && m_bitmaps[State_Focused].IsOk() )
            return State_Focused;
    }

                const State normalState = GetNormalState();
    if ( m_bitmaps[normalState].IsOk() )
        return normalState;

                return State_Normal;
}

void wxAnyButton::GTKUpdateBitmap()
{
        if ( m_bitmaps[State_Normal].IsOk() )
    {
                        State state = GTKGetCurrentBitmapState();

        GTKDoShowBitmap(m_bitmaps[state]);
    }
}

void wxAnyButton::GTKDoShowBitmap(const wxBitmap& bitmap)
{
    wxASSERT_MSG( bitmap.IsOk(), "invalid bitmap" );

    GtkWidget *image;
    if ( DontShowLabel() )
    {
        image = gtk_bin_get_child(GTK_BIN(m_widget));
    }
    else     {
        image = gtk_button_get_image(GTK_BUTTON(m_widget));
    }

    wxCHECK_RET( image && GTK_IS_IMAGE(image), "must have image widget" );

    gtk_image_set_from_pixbuf(GTK_IMAGE(image), bitmap.GetPixbuf());
}

wxBitmap wxAnyButton::DoGetBitmap(State which) const
{
    return m_bitmaps[which];
}

void wxAnyButton::DoSetBitmap(const wxBitmap& bitmap, State which)
{
    switch ( which )
    {
        case State_Normal:
            if ( DontShowLabel() )
            {
                                                                InvalidateBestSize();
            }
                                    else
            {
                GtkWidget *image = gtk_button_get_image(GTK_BUTTON(m_widget));
                if ( image && !bitmap.IsOk() )
                {
                    gtk_container_remove(GTK_CONTAINER(m_widget), image);
                }
                else if ( !image && bitmap.IsOk() )
                {
                    image = gtk_image_new();
                    gtk_button_set_image(GTK_BUTTON(m_widget), image);

                                                                                GTKApplyWidgetStyle();
                }
                else                 {
                                        break;
                }

                InvalidateBestSize();
            }
            break;

        case State_Pressed:
            if ( bitmap.IsOk() )
            {
                if ( !m_bitmaps[which].IsOk() )
                {
                                                            g_signal_connect
                    (
                        m_widget,
                        "pressed",
                        G_CALLBACK(wxgtk_button_press_callback),
                        this
                    );

                    g_signal_connect
                    (
                        m_widget,
                        "released",
                        G_CALLBACK(wxgtk_button_released_callback),
                        this
                    );
                }
            }
            else             {
                if ( m_bitmaps[which].IsOk() )
                {
                                                            g_signal_handlers_disconnect_by_func
                    (
                        m_widget,
                        (gpointer)wxgtk_button_press_callback,
                        this
                    );

                    g_signal_handlers_disconnect_by_func
                    (
                        m_widget,
                        (gpointer)wxgtk_button_released_callback,
                        this
                    );

                                        if ( m_isPressed )
                    {
                        m_isPressed = false;
                        GTKUpdateBitmap();
                    }
                }
            }
            break;

        case State_Current:
                                                            if ( bitmap.IsOk() )
            {
                if ( !m_bitmaps[which].IsOk() )
                {
                    g_signal_connect
                    (
                        m_widget,
                        "enter",
                        G_CALLBACK(wxgtk_button_enter_callback),
                        this
                    );

                    g_signal_connect
                    (
                        m_widget,
                        "leave",
                        G_CALLBACK(wxgtk_button_leave_callback),
                        this
                    );
                }
            }
            else             {
                if ( m_bitmaps[which].IsOk() )
                {
                    g_signal_handlers_disconnect_by_func
                    (
                        m_widget,
                        (gpointer)wxgtk_button_enter_callback,
                        this
                    );

                    g_signal_handlers_disconnect_by_func
                    (
                        m_widget,
                        (gpointer)wxgtk_button_leave_callback,
                        this
                    );

                    if ( m_isCurrent )
                    {
                        m_isCurrent = false;
                        GTKUpdateBitmap();
                    }
                }
            }
            break;

        case State_Focused:
            if ( bitmap.IsOk() )
            {
                Connect(wxEVT_SET_FOCUS,
                        wxFocusEventHandler(wxAnyButton::GTKOnFocus));
                Connect(wxEVT_KILL_FOCUS,
                        wxFocusEventHandler(wxAnyButton::GTKOnFocus));
            }
            else             {
                Disconnect(wxEVT_SET_FOCUS,
                           wxFocusEventHandler(wxAnyButton::GTKOnFocus));
                Disconnect(wxEVT_KILL_FOCUS,
                           wxFocusEventHandler(wxAnyButton::GTKOnFocus));
            }
            break;

        default:
                        ;
    }

    m_bitmaps[which] = bitmap;

                if ( bitmap.IsOk() && which == GTKGetCurrentBitmapState() )
    {
        GTKDoShowBitmap(bitmap);
    }
}

void wxAnyButton::DoSetBitmapPosition(wxDirection dir)
{
#ifdef __WXGTK210__
    if ( !gtk_check_version(2,10,0) )
    {
        GtkPositionType gtkpos;
        switch ( dir )
        {
            default:
                wxFAIL_MSG( "invalid position" );
                
            case wxLEFT:
                gtkpos = GTK_POS_LEFT;
                break;

            case wxRIGHT:
                gtkpos = GTK_POS_RIGHT;
                break;

            case wxTOP:
                gtkpos = GTK_POS_TOP;
                break;

            case wxBOTTOM:
                gtkpos = GTK_POS_BOTTOM;
                break;
        }

        gtk_button_set_image_position(GTK_BUTTON(m_widget), gtkpos);

                        GTKApplyWidgetStyle();

        InvalidateBestSize();
    }
#endif }

#endif 