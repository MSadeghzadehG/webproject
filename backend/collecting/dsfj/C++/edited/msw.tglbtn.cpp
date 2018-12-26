


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_TOGGLEBTN

#include "wx/tglbtn.h"

#ifndef WX_PRECOMP
    #include "wx/button.h"
    #include "wx/brush.h"
    #include "wx/dcscreen.h"
    #include "wx/settings.h"

    #include "wx/log.h"
#endif 
#include "wx/msw/private.h"
#include "wx/msw/private/button.h"


wxDEFINE_EVENT( wxEVT_TOGGLEBUTTON, wxCommandEvent );



wxIMPLEMENT_DYNAMIC_CLASS(wxBitmapToggleButton, wxToggleButton);

bool wxBitmapToggleButton::Create( wxWindow *parent, wxWindowID id,
                const wxBitmap& label,const wxPoint& pos, const wxSize& size, long style,
                const wxValidator& validator, const wxString& name )
{
    if (!wxToggleButton::Create( parent, id, wxEmptyString, pos, size, style, validator, name ))
        return false;

    SetBitmap(label);

    if (size.x == -1 || size.y == -1)
    {
        wxSize new_size = GetBestSize();
        if (size.x != -1)
            new_size.x = size.x;
        if (size.y != -1)
            new_size.y = size.y;
        SetSize( new_size );
    }

    return true;
}



wxIMPLEMENT_DYNAMIC_CLASS(wxToggleButton, wxControl);

void wxToggleButton::Init()
{
    m_state = false;
}

bool wxToggleButton::Create(wxWindow *parent,
                            wxWindowID id,
                            const wxString& label,
                            const wxPoint& pos,
                            const wxSize& size, long style,
                            const wxValidator& validator,
                            const wxString& name)
{
    Init();

    if ( !CreateControl(parent, id, pos, size, style, validator, name) )
        return false;

                            WXDWORD exstyle;
    WXDWORD msStyle = MSWGetStyle(style, &exstyle);
    msStyle |= wxMSWButton::GetMultilineStyle(label);

    return MSWCreateControl(wxT("BUTTON"), msStyle, pos, size, label, exstyle);
}

WXDWORD wxToggleButton::MSWGetStyle(long style, WXDWORD *exstyle) const
{
    WXDWORD msStyle = wxControl::MSWGetStyle(style, exstyle);

    msStyle |= BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP;

    if ( style & wxBU_LEFT )
      msStyle |= BS_LEFT;
    if ( style & wxBU_RIGHT )
      msStyle |= BS_RIGHT;
    if ( style & wxBU_TOP )
      msStyle |= BS_TOP;
    if ( style & wxBU_BOTTOM )
      msStyle |= BS_BOTTOM;

    return msStyle;
}

bool wxToggleButton::MSWIsPushed() const
{
    return GetValue();
}

void wxToggleButton::SetValue(bool val)
{
    m_state = val;
    if ( IsOwnerDrawn() )
    {
        Refresh();
    }
    else
    {
        ::SendMessage(GetHwnd(), BM_SETCHECK, val ? BST_CHECKED : BST_UNCHECKED, 0);
    }
}

bool wxToggleButton::GetValue() const
{
    if ( IsOwnerDrawn() )
    {
        return m_state;
    }
    else
    {
        return ::SendMessage(GetHwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED;
    }
}

void wxToggleButton::Command(wxCommandEvent& event)
{
    SetValue(event.GetInt() != 0);
    ProcessCommand(event);
}

bool wxToggleButton::MSWCommand(WXUINT param, WXWORD WXUNUSED(id))
{
    if ( param != BN_CLICKED && param != BN_DBLCLK )
        return false;

        
            m_state = !m_state;

    wxCommandEvent event(wxEVT_TOGGLEBUTTON, m_windowId);
    event.SetInt(GetValue());
    event.SetEventObject(this);
    ProcessCommand(event);
    return true;
}

#endif 