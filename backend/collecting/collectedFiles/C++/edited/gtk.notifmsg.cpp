


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_NOTIFICATION_MESSAGE && wxUSE_LIBNOTIFY

#include "wx/notifmsg.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/icon.h"
#endif 
#include <libnotify/notify.h>

#include "wx/module.h"
#include "wx/private/notifmsg.h"
#include <wx/stockitem.h>

#include "wx/gtk/private/error.h"


class wxLibnotifyModule : public wxModule
{
public:
    virtual bool OnInit() wxOVERRIDE
    {
                return true;
    }

    virtual void OnExit() wxOVERRIDE
    {
        if ( notify_is_initted() )
            notify_uninit();
    }

        static bool Initialize()
    {
        if ( !notify_is_initted() )
        {
            if ( !notify_init(wxTheApp->GetAppName().utf8_str()) )
                return false;
        }

        return true;
    }

private:
    wxDECLARE_DYNAMIC_CLASS(wxLibnotifyModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxLibnotifyModule, wxModule);


class wxLibNotifyMsgImpl;

void wxLibNotifyMsgImplActionCallback(NotifyNotification *notification,
                                                         char *action,
                                                         gpointer user_data);

extern "C" {
static gboolean closed_notification(NotifyNotification *notification,
    const char* WXUNUSED(data), void* user_data);
}

class wxLibNotifyMsgImpl : public wxNotificationMessageImpl
{
public:
    wxLibNotifyMsgImpl(wxNotificationMessageBase* notification) :
        wxNotificationMessageImpl(notification),
        m_notification(NULL),
        m_flags(wxICON_INFORMATION)
    {
        if ( !wxLibnotifyModule::Initialize() )
            wxLogError(_("Could not initalize libnotify."));

    }

    virtual ~wxLibNotifyMsgImpl()
    {
        if ( m_notification )
            g_object_unref(m_notification);
    }

    bool CreateOrUpdateNotification()
    {
        if ( !wxLibnotifyModule::Initialize() )
            return false;

                        const char* icon;
        switch ( m_flags )
        {
            case wxICON_INFORMATION:
                icon = "dialog-information";
                break;

            case wxICON_WARNING:
                icon = "dialog-warning";
                break;

            case wxICON_ERROR:
                icon = "dialog-error";
                break;

            default:
                wxFAIL_MSG( "Unknown notification message flags." );
                return false;
        }

                        if ( !m_notification )
        {
            m_notification = notify_notification_new
                             (
                                m_title.utf8_str(),
                                m_message.utf8_str(),
                                icon
#if !wxUSE_LIBNOTIFY_0_7
                                                                                                                                                                , 0
#endif                              );
            if ( !m_notification )
            {
                wxLogDebug("Failed to creation notification.");

                return false;
            }


            g_signal_connect(m_notification, "closed", G_CALLBACK(closed_notification), this);

        }
        else
        {
            if ( !notify_notification_update
                  (
                    m_notification,
                    m_title.utf8_str(),
                    m_message.utf8_str(),
                    icon
                  ) )
            {
                wxLogDebug(wxS("notify_notification_update() unexpectedly failed."));
            }
        }

                        if ( m_icon.IsOk() )
        {
#ifdef __WXGTK3__
            notify_notification_set_image_from_pixbuf(
                m_notification,
                m_icon.GetPixbufNoMask()
            );
#endif
        }

        return true;
    }

    virtual bool Show(int timeout) wxOVERRIDE
    {
        if ( !CreateOrUpdateNotification() )
            return false;

                notify_notification_set_timeout
        (
            m_notification,
            timeout == wxNotificationMessage::Timeout_Auto ? NOTIFY_EXPIRES_DEFAULT
                                    : timeout == wxNotificationMessage::Timeout_Never ? NOTIFY_EXPIRES_NEVER
                                                               : 1000*timeout
        );

        NotifyUrgency urgency;
        switch ( m_flags )
        {
            case wxICON_INFORMATION:
                urgency = NOTIFY_URGENCY_LOW;
                break;

            case wxICON_WARNING:
                urgency = NOTIFY_URGENCY_NORMAL;
                break;

            case wxICON_ERROR:
                urgency = NOTIFY_URGENCY_CRITICAL;
                break;

            default:
                wxFAIL_MSG( "Unknown notification message flags." );
                return false;
        }
        notify_notification_set_urgency(m_notification, urgency);


                wxGtkError error;
        if ( !notify_notification_show(m_notification, error.Out()) )
        {
            wxLogDebug("Failed to shown notification: %s", error.GetMessage());

            return false;
        }

        return true;
    }

    virtual bool Close() wxOVERRIDE
    {
        wxCHECK_MSG( m_notification, false,
                     wxS("Can't close not shown notification.") );

        wxGtkError error;
        if ( !notify_notification_close(m_notification, error.Out()) )
        {
            wxLogDebug("Failed to hide notification: %s", error.GetMessage());

            return false;
        }

        return true;
    }

    virtual void SetTitle(const wxString& title) wxOVERRIDE
    {
        m_title = title;
    }

    virtual void SetMessage(const wxString& message) wxOVERRIDE
    {
        m_message = message;
    }

    virtual void SetParent(wxWindow *WXUNUSED(parent)) wxOVERRIDE
    {
    }

    virtual void SetFlags(int flags) wxOVERRIDE
    {
        m_flags = flags;
    }

    virtual void SetIcon(const wxIcon& icon) wxOVERRIDE
    {
        m_icon = icon;
        CreateOrUpdateNotification();
    }

    virtual bool AddAction(wxWindowID actionid, const wxString &label) wxOVERRIDE
    {
        if ( !CreateOrUpdateNotification() )
            return false;

        wxString labelStr = label;
        if ( labelStr.empty() )
            labelStr = wxGetStockLabel(actionid, wxSTOCK_NOFLAGS);

        notify_notification_add_action
            (
                m_notification,
                wxString::Format("%d", actionid).utf8_str(),
                labelStr.utf8_str(),
                &wxLibNotifyMsgImplActionCallback,
                this,
                NULL
            );

        return true;
    }

    void NotifyClose(int closeReason)
    {
                
        switch (closeReason)
        {
            case 1:             case 2:             {
                wxCommandEvent evt(wxEVT_NOTIFICATION_MESSAGE_DISMISSED);
                ProcessNotificationEvent(evt);
                break;
            }
        }

    }

    void NotifyAction(wxWindowID actionid)
    {
        wxCommandEvent evt(wxEVT_NOTIFICATION_MESSAGE_ACTION, actionid);
        ProcessNotificationEvent(evt);
    }

private:
    NotifyNotification* m_notification;
    wxString m_title;
    wxString m_message;
    wxIcon m_icon;
    int m_flags;
};

void wxLibNotifyMsgImplActionCallback(NotifyNotification *WXUNUSED(notification),
                                                         char *action,
                                                         gpointer user_data)
{
    wxLibNotifyMsgImpl* impl = (wxLibNotifyMsgImpl*) user_data;

    impl->NotifyAction(wxAtoi(action));
}

extern "C" {
static gboolean closed_notification(NotifyNotification *notification,
    const char* WXUNUSED(data), void* user_data)
{
    wxLibNotifyMsgImpl* impl = (wxLibNotifyMsgImpl*) user_data;
    gint closeReason = notify_notification_get_closed_reason(notification);
    impl->NotifyClose(closeReason);
    return true;
}
}


void wxNotificationMessage::Init()
{
    m_impl = new wxLibNotifyMsgImpl(this);
}

#endif 