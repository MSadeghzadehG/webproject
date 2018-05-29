
#include "wx/wxprec.h"

#include "wx/button.h"

#ifndef WX_PRECOMP
    #include "wx/panel.h"
    #include "wx/toplevel.h"
    #include "wx/dcclient.h"
    #include "wx/stattext.h"
#endif

#include "wx/stockitem.h"

#include "wx/osx/private.h"

namespace
{

bool IsHelpButtonWithStandardLabel(wxWindowID id, const wxString& label)
{
    if ( id != wxID_HELP )
        return false;

    if ( label.empty() )
        return true;

    const wxString labelText = wxStaticText::GetLabelText(label);
    return labelText == "Help" || labelText == _("Help");
}

} 
bool wxButton::Create(wxWindow *parent,
    wxWindowID id,
    const wxString& labelOrig,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxValidator& validator,
    const wxString& name)
{
                            if ( style & wxBU_NOTEXT && !ShouldCreatePeer() )
    {
        return wxControl::Create(parent, id, pos, size, style,
                                 validator, name);
    }

    DontCreatePeer();

    m_marginX =
    m_marginY = 0;

    wxString label;

    if ( !(style & wxBU_NOTEXT) )
    {
                        if ( !IsHelpButtonWithStandardLabel(id, labelOrig) )
        {
            label = labelOrig.empty() && wxIsStockID(id) ? wxGetStockLabel(id)
                                                         : labelOrig;
        }
    }


    if ( !wxButtonBase::Create(parent, id, pos, size, style, validator, name) )
        return false;

    m_labelOrig =
    m_label = label ;

    SetPeer(wxWidgetImpl::CreateButton( this, parent, id, label, pos, size, style, GetExtraStyle() ));

    MacPostControlCreate( pos, size );

    return true;
}

void wxButton::SetLabel(const wxString& label)
{
    if ( IsHelpButtonWithStandardLabel(GetId(), label) )
    {
                return;
    }

    wxAnyButton::SetLabel(label);
#if wxOSX_USE_COCOA
    OSXUpdateAfterLabelChange(label);
#endif
}

wxWindow *wxButton::SetDefault()
{
    wxWindow *btnOldDefault = wxButtonBase::SetDefault();

    if ( btnOldDefault )
    {
        btnOldDefault->GetPeer()->SetDefaultButton( false );
    }

    GetPeer()->SetDefaultButton( true );

    return btnOldDefault;
}

void wxButton::Command (wxCommandEvent & WXUNUSED(event))
{
    GetPeer()->PerformClick() ;
    }

bool wxButton::OSXHandleClicked( double WXUNUSED(timestampsec) )
{
    wxCommandEvent event(wxEVT_BUTTON, m_windowId);
    event.SetEventObject(this);
    ProcessCommand(event);
    return true;
}


wxSize wxButtonBase::GetDefaultSize()
{
    return wxAnyButton::GetDefaultSize();
}
