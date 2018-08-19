


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#endif 
#if wxUSE_GAUGE

#include "wx/gauge.h"
#include "wx/appprogress.h"

const char wxGaugeNameStr[] = "gauge";


wxGaugeBase::~wxGaugeBase()
{
        delete m_appProgressIndicator;
}


wxDEFINE_FLAGS( wxGaugeStyle )
wxBEGIN_FLAGS( wxGaugeStyle )
wxFLAGS_MEMBER(wxBORDER_SIMPLE)
wxFLAGS_MEMBER(wxBORDER_SUNKEN)
wxFLAGS_MEMBER(wxBORDER_DOUBLE)
wxFLAGS_MEMBER(wxBORDER_RAISED)
wxFLAGS_MEMBER(wxBORDER_STATIC)
wxFLAGS_MEMBER(wxBORDER_NONE)

wxFLAGS_MEMBER(wxSIMPLE_BORDER)
wxFLAGS_MEMBER(wxSUNKEN_BORDER)
wxFLAGS_MEMBER(wxDOUBLE_BORDER)
wxFLAGS_MEMBER(wxRAISED_BORDER)
wxFLAGS_MEMBER(wxSTATIC_BORDER)
wxFLAGS_MEMBER(wxBORDER)

wxFLAGS_MEMBER(wxTAB_TRAVERSAL)
wxFLAGS_MEMBER(wxCLIP_CHILDREN)
wxFLAGS_MEMBER(wxTRANSPARENT_WINDOW)
wxFLAGS_MEMBER(wxWANTS_CHARS)
wxFLAGS_MEMBER(wxFULL_REPAINT_ON_RESIZE)
wxFLAGS_MEMBER(wxALWAYS_SHOW_SB )
wxFLAGS_MEMBER(wxVSCROLL)
wxFLAGS_MEMBER(wxHSCROLL)

wxFLAGS_MEMBER(wxGA_HORIZONTAL)
wxFLAGS_MEMBER(wxGA_VERTICAL)
wxFLAGS_MEMBER(wxGA_SMOOTH)
wxFLAGS_MEMBER(wxGA_PROGRESS)
wxEND_FLAGS( wxGaugeStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxGauge, wxControl, "wx/gauge.h");

wxBEGIN_PROPERTIES_TABLE(wxGauge)
wxPROPERTY( Value, int, SetValue, GetValue, 0, 0 , \
           wxT("Helpstring"), wxT("group"))
wxPROPERTY( Range, int, SetRange, GetRange, 0, 0 , \
           wxT("Helpstring"), wxT("group"))

wxPROPERTY_FLAGS( WindowStyle, wxGaugeStyle, long, SetWindowStyleFlag, \
                 GetWindowStyleFlag, wxEMPTY_PARAMETER_VALUE, 0 , \
                 wxT("Helpstring"), wxT("group")) wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxGauge)

wxCONSTRUCTOR_6( wxGauge, wxWindow*, Parent, wxWindowID, Id, int, Range, \
                wxPoint, Position, wxSize, Size, long, WindowStyle )


void wxGaugeBase::InitProgressIndicatorIfNeeded()
{
    m_appProgressIndicator = NULL;
    if ( HasFlag(wxGA_PROGRESS) )
    {
        wxWindow* topParent = wxGetTopLevelParent(this);
        if ( topParent != NULL )
        {
            m_appProgressIndicator =
                new wxAppProgressIndicator(topParent, GetRange());
        }
    }
}

bool wxGaugeBase::Create(wxWindow *parent,
                         wxWindowID id,
                         int range,
                         const wxPoint& pos,
                         const wxSize& size,
                         long style,
                         const wxValidator& validator,
                         const wxString& name)
{
    if ( !wxControl::Create(parent, id, pos, size, style, validator, name) )
        return false;

    SetName(name);

#if wxUSE_VALIDATORS
    SetValidator(validator);
#endif 
    SetRange(range);
    SetValue(0);

#if wxGAUGE_EMULATE_INDETERMINATE_MODE
    m_nDirection = wxRIGHT;
#endif

    InitProgressIndicatorIfNeeded();

    return true;
}


void wxGaugeBase::SetRange(int range)
{
    m_rangeMax = range;

    if ( m_appProgressIndicator )
        m_appProgressIndicator->SetRange(m_rangeMax);
}

int wxGaugeBase::GetRange() const
{
    return m_rangeMax;
}

void wxGaugeBase::SetValue(int pos)
{
    m_gaugePos = pos;

    if ( m_appProgressIndicator )
    {
        m_appProgressIndicator->SetValue(pos);
        if ( pos == 0 )
        {
            m_appProgressIndicator->Reset();
        }
    }
}

int wxGaugeBase::GetValue() const
{
    return m_gaugePos;
}


void wxGaugeBase::Pulse()
{
#if wxGAUGE_EMULATE_INDETERMINATE_MODE
        int curr = GetValue(), max = GetRange();

    if (m_nDirection == wxRIGHT)
    {
        if (curr < max)
            SetValue(curr + 1);
        else
        {
            SetValue(max - 1);
            m_nDirection = wxLEFT;
        }
    }
    else
    {
        if (curr > 0)
            SetValue(curr - 1);
        else
        {
            SetValue(1);
            m_nDirection = wxRIGHT;
        }
    }
#endif

    if ( m_appProgressIndicator )
        m_appProgressIndicator->Pulse();
}

#endif 