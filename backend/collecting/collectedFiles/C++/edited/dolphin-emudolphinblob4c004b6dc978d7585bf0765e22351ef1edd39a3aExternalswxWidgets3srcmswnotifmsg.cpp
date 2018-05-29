


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_NOTIFICATION_MESSAGE && \
        wxUSE_TASKBARICON && wxUSE_TASKBARICON_BALLOONS

#include "wx/notifmsg.h"

#ifndef WX_PRECOMP
    #include "wx/toplevel.h"
    #include "wx/app.h"
    #include "wx/string.h"
    #include  "wx/app.h"
#endif 
#include "wx/private/notifmsg.h"
#include "wx/msw/rt/private/notifmsg.h"

#include "wx/taskbar.h"


class wxBalloonNotifMsgImpl : public wxNotificationMessageImpl
{
public:
                    wxBalloonNotifMsgImpl(wxNotificationMessageBase* notification) :
        wxNotificationMessageImpl(notification),
        m_flags(wxICON_INFORMATION),
        m_parent(NULL)
    {

    }

    virtual ~wxBalloonNotifMsgImpl();

    virtual bool Show(int timeout) wxOVERRIDE;

    virtual bool Close() wxOVERRIDE;

    virtual void SetTitle(const wxString& title) wxOVERRIDE
    {
        m_title = title;
    }

    virtual void SetMessage(const wxString& message) wxOVERRIDE
    {
        m_message = message;
    }

    virtual void SetParent(wxWindow *parent) wxOVERRIDE
    {
        m_parent = parent;
    }

    virtual void SetFlags(int flags) wxOVERRIDE
    {
        m_flags = flags;
    }

    virtual void SetIcon(const wxIcon& icon) wxOVERRIDE
    {
        m_icon = icon;
    }

    virtual bool AddAction(wxWindowID WXUNUSED(actionid), const wxString &WXUNUSED(label))
    {
                return false;
    }

        static wxTaskBarIcon *UseTaskBarIcon(wxTaskBarIcon *icon);

            static bool IsUsingOwnIcon()
    {
        return ms_refCountIcon != -1;
    }

                            static void ReleaseIcon()
    {
        wxASSERT_MSG( ms_refCountIcon != -1,
                      wxS("Must not be called when not using own icon") );

        if ( ms_refCountIcon > 0 && !--ms_refCountIcon )
        {
            delete ms_icon;
            ms_icon = NULL;
        }
    }

protected:
        void SetUpIcon(wxWindow *win);


                                                            static wxTaskBarIcon *ms_icon;
    static int ms_refCountIcon;
private:
    wxString m_title;
    wxString m_message;
    int m_flags;
    wxIcon m_icon;
    wxWindow* m_parent;

    void OnTimeout(wxTaskBarIconEvent& event);
    void OnClick(wxTaskBarIconEvent& event);

    void OnIconHidden();
};




wxTaskBarIcon *wxBalloonNotifMsgImpl::ms_icon = NULL;
int wxBalloonNotifMsgImpl::ms_refCountIcon = 0;


wxTaskBarIcon *wxBalloonNotifMsgImpl::UseTaskBarIcon(wxTaskBarIcon *icon)
{
    wxTaskBarIcon * const iconOld = ms_icon;
    ms_icon = icon;

        ms_refCountIcon = icon ? -1 : 0;

    return iconOld;
}

wxBalloonNotifMsgImpl::~wxBalloonNotifMsgImpl()
{
}

void wxBalloonNotifMsgImpl::OnIconHidden()
{
    SetActive(false);
    if ( ms_icon )
    {
        ms_icon->Unbind(wxEVT_TASKBAR_BALLOON_CLICK, &wxBalloonNotifMsgImpl::OnClick, this);
        ms_icon->Unbind(wxEVT_TASKBAR_BALLOON_TIMEOUT, &wxBalloonNotifMsgImpl::OnTimeout, this);
    }

    if ( IsUsingOwnIcon() )
        wxBalloonNotifMsgImpl::ReleaseIcon();
}

void wxBalloonNotifMsgImpl::OnTimeout(wxTaskBarIconEvent& WXUNUSED(event))
{
    wxCommandEvent evt(wxEVT_NOTIFICATION_MESSAGE_DISMISSED);
    ProcessNotificationEvent(evt);

    OnIconHidden();
}

void wxBalloonNotifMsgImpl::OnClick(wxTaskBarIconEvent& WXUNUSED(event))
{
    wxCommandEvent evt(wxEVT_NOTIFICATION_MESSAGE_CLICK);
    ProcessNotificationEvent(evt);

    OnIconHidden();
}

void wxBalloonNotifMsgImpl::SetUpIcon(wxWindow *win)
{
    if ( ms_icon )
    {
                if ( ms_refCountIcon != -1 )
            ms_refCountIcon++;
    }
    else     {
        wxASSERT_MSG( ms_refCountIcon == 0,
                      wxS("Shouldn't reference not existent icon") );

        ms_icon = new wxTaskBarIcon;
        ms_refCountIcon = 1;

                wxIcon icon;
        if ( win )
            win = wxGetTopLevelParent(win);
        if ( !win )
            win = wxTheApp->GetTopWindow();
        if ( win )
        {
            const wxTopLevelWindow * const
                tlw = wxDynamicCast(win, wxTopLevelWindow);
            if ( tlw )
                icon = tlw->GetIcon();
        }

        if ( !icon.IsOk() )
        {
                        icon = wxIcon(wxT("wxICON_AAA"));
        }

        ms_icon->SetIcon(icon);
    }
}

bool
wxBalloonNotifMsgImpl::Show(int timeout)
{
        wxTaskBarIconEvent event(wxEVT_TASKBAR_BALLOON_TIMEOUT, ms_icon);
    OnTimeout(event);

    SetUpIcon(m_parent);

    if ( !ms_icon->IsIconInstalled() )
    {
                                                                                delete ms_icon;
        ms_icon = NULL;
        ms_refCountIcon = 0;

        return false;
    }

        if ( timeout == wxNotificationMessage::Timeout_Auto )
    {
                timeout = 1;
    }
    else if ( timeout == wxNotificationMessage::Timeout_Never )
    {
                        timeout = 30;
    }

    timeout *= 1000; 
    bool res = ms_icon->ShowBalloon(m_title, m_message, timeout, m_flags, m_icon);
    if ( res )
    {
        ms_icon->Bind(wxEVT_TASKBAR_BALLOON_CLICK, &wxBalloonNotifMsgImpl::OnClick, this);
        ms_icon->Bind(wxEVT_TASKBAR_BALLOON_TIMEOUT, &wxBalloonNotifMsgImpl::OnTimeout, this);
        SetActive(true);
    }

    return res;
}

bool wxBalloonNotifMsgImpl::Close()
{
    wxCommandEvent evt(wxEVT_NOTIFICATION_MESSAGE_DISMISSED);
    ProcessNotificationEvent(evt);

    OnIconHidden();

    if ( !IsUsingOwnIcon() && ms_icon )
    {
                ms_icon->ShowBalloon("", "");
    }

    return true;
}



wxTaskBarIcon *wxNotificationMessage::UseTaskBarIcon(wxTaskBarIcon *icon)
{
    return wxBalloonNotifMsgImpl::UseTaskBarIcon(icon);
}

bool wxNotificationMessage::MSWUseToasts(
    const wxString& shortcutPath,
    const wxString& appId)
{
    return wxToastNotificationHelper::UseToasts(shortcutPath, appId);
}

void wxNotificationMessage::Init()
{
    if ( wxToastNotificationHelper::IsEnabled() )
        m_impl = wxToastNotificationHelper::CreateInstance(this);
    else
        m_impl = new wxBalloonNotifMsgImpl(this);
}

#endif 