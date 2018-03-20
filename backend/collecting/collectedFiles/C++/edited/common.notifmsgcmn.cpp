


#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#if wxUSE_NOTIFICATION_MESSAGE

#include "wx/notifmsg.h"

#include "wx/private/notifmsg.h"

#ifndef wxHAS_NATIVE_NOTIFICATION_MESSAGE
#include "wx/generic/private/notifmsg.h"
#endif

wxDEFINE_EVENT( wxEVT_NOTIFICATION_MESSAGE_CLICK, wxCommandEvent );
wxDEFINE_EVENT( wxEVT_NOTIFICATION_MESSAGE_DISMISSED, wxCommandEvent );
wxDEFINE_EVENT( wxEVT_NOTIFICATION_MESSAGE_ACTION, wxCommandEvent );


wxNotificationMessageBase::~wxNotificationMessageBase()
{
    m_impl->Detach();
}

bool wxNotificationMessageBase::Show(int timeout)
{
    return m_impl->Show(timeout);
}

bool wxNotificationMessageBase::Close()
{
    return m_impl->Close();
}

void wxNotificationMessageBase::SetTitle(const wxString& title)
{
    m_impl->SetTitle(title);
}

void wxNotificationMessageBase::SetMessage(const wxString& message)
{
    m_impl->SetMessage(message);
}

void wxNotificationMessageBase::SetParent(wxWindow *parent)
{
    m_impl->SetParent(parent);
}

void wxNotificationMessageBase::SetFlags(int flags)
{
    wxASSERT_MSG(flags == wxICON_INFORMATION ||
        flags == wxICON_WARNING || flags == wxICON_ERROR ||
        flags == 0,
        "Invalid icon flags specified");

    m_impl->SetFlags(flags);
}

void wxNotificationMessageBase::SetIcon(const wxIcon& icon)
{
    m_impl->SetIcon(icon);
}

bool wxNotificationMessageBase::AddAction(wxWindowID actionid, const wxString &label)
{
    return m_impl->AddAction(actionid, label);
}


#ifndef wxHAS_NATIVE_NOTIFICATION_MESSAGE

void wxNotificationMessage::Init()
{
    m_impl = new wxGenericNotificationMessageImpl(this);
}

#endif

#endif 