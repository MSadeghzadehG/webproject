


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/defs.h"

#if wxUSE_COLLPANE && wxUSE_BUTTON && wxUSE_STATLINE

#include "wx/collpane.h"

#ifndef WX_PRECOMP
    #include "wx/toplevel.h"
    #include "wx/sizer.h"
    #include "wx/panel.h"
#endif 
#include "wx/statline.h"
#include "wx/collheaderctrl.h"



const char wxCollapsiblePaneNameStr[] = "collapsiblePane";


wxDEFINE_EVENT( wxEVT_COLLAPSIBLEPANE_CHANGED, wxCollapsiblePaneEvent );
wxIMPLEMENT_DYNAMIC_CLASS(wxGenericCollapsiblePane, wxControl);
wxIMPLEMENT_DYNAMIC_CLASS(wxCollapsiblePaneEvent, wxCommandEvent);

wxBEGIN_EVENT_TABLE(wxGenericCollapsiblePane, wxControl)
    EVT_COLLAPSIBLEHEADER_CHANGED(wxID_ANY, wxGenericCollapsiblePane::OnButton)
    EVT_SIZE(wxGenericCollapsiblePane::OnSize)
wxEND_EVENT_TABLE()

void wxGenericCollapsiblePane::Init()
{
    m_pButton = NULL;
    m_pPane = NULL;
    m_pStaticLine = NULL;
    m_sz = NULL;
}

bool wxGenericCollapsiblePane::Create(wxWindow *parent,
                                      wxWindowID id,
                                      const wxString& label,
                                      const wxPoint& pos,
                                      const wxSize& size,
                                      long style,
                                      const wxValidator& val,
                                      const wxString& name)
{
    if ( !wxControl::Create(parent, id, pos, size, style, val, name) )
        return false;

        m_sz = new wxBoxSizer(wxVERTICAL);

            m_pButton = new wxCollapsibleHeaderCtrl(this, wxID_ANY, label, wxPoint(0, 0),
                             wxDefaultSize);

        m_sz->Add(m_pButton, 0, wxLEFT|wxTOP|wxBOTTOM, GetBorder());

#if !defined( __WXMAC__ ) || defined(__WXUNIVERSAL__)
    m_pStaticLine = new wxStaticLine(this, wxID_ANY);
    m_sz->Add(m_pStaticLine, 0, wxEXPAND, GetBorder());
    m_pStaticLine->Hide();
#endif

        #if defined(__WXGTK__)
    SetBackgroundColour(parent->GetBackgroundColour());
#endif

        m_pPane = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                          wxTAB_TRAVERSAL|wxNO_BORDER, wxT("wxCollapsiblePanePane") );

        m_pPane->Hide();

    return true;
}

wxGenericCollapsiblePane::~wxGenericCollapsiblePane()
{
    if (m_pButton)
        m_pButton->SetContainingSizer(NULL);

    if (m_pStaticLine)
        m_pStaticLine->SetContainingSizer(NULL);

        wxDELETE(m_sz);
}

wxSize wxGenericCollapsiblePane::DoGetBestSize() const
{
        wxSize sz = m_sz->GetMinSize();

        if ( IsExpanded() )
    {
        sz.SetWidth(wxMax( sz.GetWidth(), m_pPane->GetBestSize().x ));
        sz.SetHeight(sz.y + GetBorder() + m_pPane->GetBestSize().y);
    }

    return sz;
}

void wxGenericCollapsiblePane::OnStateChange(const wxSize& sz)
{
        SetSize(sz);

    if (this->HasFlag(wxCP_NO_TLW_RESIZE))
    {
                return;
    }


    wxTopLevelWindow *top =
        wxDynamicCast(wxGetTopLevelParent(this), wxTopLevelWindow);
    if ( !top )
        return;

    wxSizer *sizer = top->GetSizer();
    if ( !sizer )
        return;

    const wxSize newBestSize = sizer->ComputeFittingClientSize(top);
    top->SetMinClientSize(newBestSize);

        if ( !top->IsMaximized() )
        top->SetClientSize(newBestSize);
}

void wxGenericCollapsiblePane::Collapse(bool collapse)
{
        if ( IsCollapsed() == collapse )
        return;

    InvalidateBestSize();

        m_pPane->Show(!collapse);

            m_pButton->SetCollapsed(collapse);
    if ( m_pStaticLine )
        m_pStaticLine->Show(!collapse);


    OnStateChange(GetBestSize());
}

void wxGenericCollapsiblePane::SetLabel(const wxString &label)
{
    m_pButton->SetLabel(label);
    m_pButton->SetInitialSize();

    Layout();
}

wxString wxGenericCollapsiblePane::GetLabel() const
{
    return m_pButton->GetLabel();
}

bool wxGenericCollapsiblePane::Layout()
{
#ifdef __WXMAC__
    if (!m_pButton || !m_pPane || !m_sz)
        return false;     #else
    if (!m_pButton || !m_pStaticLine || !m_pPane || !m_sz)
        return false;     #endif

    wxSize oursz(GetSize());

        m_sz->SetDimension(0, 0, oursz.GetWidth(), m_sz->GetMinSize().GetHeight());
    m_sz->Layout();

    if ( IsExpanded() )
    {
                int yoffset = m_sz->GetSize().GetHeight() + GetBorder();
        m_pPane->SetSize(0, yoffset,
                        oursz.x, oursz.y - yoffset);

                m_pPane->Layout();
    }

    return true;
}

int wxGenericCollapsiblePane::GetBorder() const
{
#if defined( __WXMAC__ )
    return 6;
#elif defined(__WXMSW__)
    wxASSERT(m_pButton);
    return m_pButton->ConvertDialogToPixels(wxSize(2, 0)).x;
#else
    return 5;
#endif
}




void wxGenericCollapsiblePane::OnButton(wxCommandEvent& event)
{
    if ( event.GetEventObject() != m_pButton )
    {
        event.Skip();
        return;
    }

    Collapse(!IsCollapsed());

        wxCollapsiblePaneEvent ev(this, GetId(), IsCollapsed());
    GetEventHandler()->ProcessEvent(ev);
}

void wxGenericCollapsiblePane::OnSize(wxSizeEvent& WXUNUSED(event))
{
#if 0           wxClientDC dc(this);
    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(wxPoint(0,0), GetSize());
    dc.SetPen(*wxRED_PEN);
    dc.DrawRectangle(wxPoint(0,0), GetBestSize());
#endif

    Layout();
}

#endif 