
#include "wx/wxprec.h"

#if wxUSE_GAUGE

#include "wx/gauge.h"

#include "wx/appprogress.h"
#include "wx/osx/private.h"

bool wxGauge::Create( wxWindow *parent,
    wxWindowID id,
    int range,
    const wxPoint& pos,
    const wxSize& s,
    long style,
    const wxValidator& validator,
    const wxString& name )
{    
    DontCreatePeer();
    
    if ( !wxGaugeBase::Create( parent, id, range, pos, s, style & 0xE0FFFFFF, validator, name ) )
        return false;

    wxSize size = s;

    SetPeer(wxWidgetImpl::CreateGauge( this, parent, id, GetValue() , 0, GetRange(), pos, size, style, GetExtraStyle() ));

    MacPostControlCreate( pos, size );

    return true;
}

void wxGauge::SetRange(int r)
{
            wxGaugeBase::SetRange( r ) ;
    if ( GetPeer() )
        GetPeer()->SetMaximum( GetRange() ) ;
}

void wxGauge::SetValue(int pos)
{
            wxGaugeBase::SetValue( pos ) ;

    if ( GetPeer() )
        GetPeer()->SetValue( GetValue() ) ;
}

int wxGauge::GetValue() const
{
    return m_gaugePos ;
}

void wxGauge::Pulse()
{
    GetPeer()->PulseGauge();

    if ( m_appProgressIndicator )
        m_appProgressIndicator->Pulse();
}

#endif 
