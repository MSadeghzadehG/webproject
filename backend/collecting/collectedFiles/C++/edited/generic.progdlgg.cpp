


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_PROGRESSDLG

#ifndef WX_PRECOMP
    #include "wx/utils.h"
    #include "wx/frame.h"
    #include "wx/button.h"
    #include "wx/stattext.h"
    #include "wx/sizer.h"
    #include "wx/event.h"
    #include "wx/gauge.h"
    #include "wx/intl.h"
    #include "wx/dcclient.h"
    #include "wx/timer.h"
    #include "wx/settings.h"
    #include "wx/app.h"
#endif

#include "wx/progdlg.h"
#include "wx/evtloop.h"


#define LAYOUT_MARGIN 8

static const int wxID_SKIP = 32000;  

wxBEGIN_EVENT_TABLE(wxGenericProgressDialog, wxDialog)
    EVT_BUTTON(wxID_CANCEL, wxGenericProgressDialog::OnCancel)
    EVT_BUTTON(wxID_SKIP, wxGenericProgressDialog::OnSkip)

    EVT_CLOSE(wxGenericProgressDialog::OnClose)
wxEND_EVENT_TABLE()


wxIMPLEMENT_CLASS(wxProgressDialog, wxDialog)


void wxGenericProgressDialog::Init()
{
        SetExtraStyle(GetExtraStyle() | wxWS_EX_TRANSIENT);

        
    m_pdStyle = 0;
    m_parentTop = NULL;

    m_gauge = NULL;
    m_msg = NULL;
    m_elapsed =
    m_estimated =
    m_remaining = NULL;

    m_state = Uncancelable;
    m_maximum = 0;

    m_timeStart = wxGetCurrentTime();
    m_timeStop = (unsigned long)-1;
    m_break = 0;

    m_skip = false;

    m_btnAbort =
    m_btnSkip = NULL;

    m_display_estimated =
    m_last_timeupdate =
    m_ctdelay = 0;

    m_delay = 3;

    m_winDisabler = NULL;
    m_tempEventLoop = NULL;

    SetWindowStyle(wxDEFAULT_DIALOG_STYLE);
}

wxGenericProgressDialog::wxGenericProgressDialog()
                       : wxDialog()
{
    Init();
}

wxGenericProgressDialog::wxGenericProgressDialog(const wxString& title,
                                                 const wxString& message,
                                                 int maximum,
                                                 wxWindow *parent,
                                                 int style)
                       : wxDialog()
{
    Init();

    Create( title, message, maximum, parent, style );
}

void wxGenericProgressDialog::SetTopParent(wxWindow* parent)
{
    m_parentTop = GetParentForModalDialog(parent, GetWindowStyle());
}

bool wxGenericProgressDialog::Create( const wxString& title,
                                      const wxString& message,
                                      int maximum,
                                      wxWindow *parent,
                                      int style )
{
    SetTopParent(parent);

    m_parentTop = wxGetTopLevelParent(parent);
    m_pdStyle = style;

    wxWindow* const
        realParent = GetParentForModalDialog(parent, GetWindowStyle());

    if (!wxDialog::Create(realParent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, GetWindowStyle()))
        return false;

    SetMaximum(maximum);

                        if ( !wxEventLoopBase::GetActive() )
    {
        m_tempEventLoop = new wxEventLoop;
        wxEventLoop::SetActive(m_tempEventLoop);
    }

#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
                    if ( !HasPDFlag(wxPD_CAN_ABORT) )
    {
        EnableCloseButton(false);
    }
#endif 
    m_state = HasPDFlag(wxPD_CAN_ABORT) ? Continue : Uncancelable;

        wxSizer * const sizerTop = new wxBoxSizer(wxVERTICAL);

    m_msg = new wxStaticText(this, wxID_ANY, message);
    sizerTop->Add(m_msg, 0, wxLEFT | wxRIGHT | wxTOP, 2*LAYOUT_MARGIN);

    int gauge_style = wxGA_HORIZONTAL;
    if ( style & wxPD_SMOOTH )
        gauge_style |= wxGA_SMOOTH;
    gauge_style |= wxGA_PROGRESS;

#ifdef __WXMSW__
    maximum /= m_factor;
#endif

    m_gauge = new wxGauge
                  (
                    this,
                    wxID_ANY,
                    maximum,
                    wxDefaultPosition,
                                        wxSize(wxMin(wxGetClientDisplayRect().width/3, 300), -1),
                    gauge_style
                  );

    sizerTop->Add(m_gauge, 0, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 2*LAYOUT_MARGIN);
    m_gauge->SetValue(0);

        m_elapsed =
    m_estimated =
    m_remaining = NULL;

        size_t nTimeLabels = 0;

    wxSizer * const sizerLabels = new wxFlexGridSizer(2);

    if ( style & wxPD_ELAPSED_TIME )
    {
        nTimeLabels++;

        m_elapsed = CreateLabel(GetElapsedLabel(), sizerLabels);
    }

    if ( style & wxPD_ESTIMATED_TIME )
    {
        nTimeLabels++;

        m_estimated = CreateLabel(GetEstimatedLabel(), sizerLabels);
    }

    if ( style & wxPD_REMAINING_TIME )
    {
        nTimeLabels++;

        m_remaining = CreateLabel(GetRemainingLabel(), sizerLabels);
    }
    sizerTop->Add(sizerLabels, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, LAYOUT_MARGIN);

    wxStdDialogButtonSizer *buttonSizer = wxDialog::CreateStdDialogButtonSizer(0);

    const int borderFlags = wxALL;

    wxSizerFlags sizerFlags
        = wxSizerFlags().Border(borderFlags, LAYOUT_MARGIN);

    if ( HasPDFlag(wxPD_CAN_SKIP) )
    {
        m_btnSkip = new wxButton(this, wxID_SKIP, _("&Skip"));

        buttonSizer->SetNegativeButton(m_btnSkip);
    }

    if ( HasPDFlag(wxPD_CAN_ABORT) )
    {
        m_btnAbort = new wxButton(this, wxID_CANCEL);

        buttonSizer->SetCancelButton(m_btnAbort);
    }

    if ( !HasPDFlag(wxPD_CAN_SKIP | wxPD_CAN_ABORT) )
        buttonSizer->AddSpacer(LAYOUT_MARGIN);

    buttonSizer->Realize();

    sizerTop->Add(buttonSizer, sizerFlags.Expand());

    SetSizerAndFit(sizerTop);

    Centre(wxCENTER_FRAME | wxBOTH);

    DisableOtherWindows();

    Show();
    Enable();

                if ( m_elapsed )
    {
        SetTimeLabel(0, m_elapsed);
    }

    Update();
    return true;
}

void wxGenericProgressDialog::UpdateTimeEstimates(int value,
                                                  unsigned long &elapsedTime,
                                                  unsigned long &estimatedTime,
                                                  unsigned long &remainingTime)
{
    unsigned long elapsed = wxGetCurrentTime() - m_timeStart;
    if ( value != 0 && (m_last_timeupdate < elapsed || value == m_maximum) )
    {
        m_last_timeupdate = elapsed;
        unsigned long estimated = m_break +
                (unsigned long)(( (double) (elapsed-m_break) * m_maximum ) / ((double)value)) ;
        if (    estimated > m_display_estimated
                && m_ctdelay >= 0
            )
        {
            ++m_ctdelay;
        }
        else if (    estimated < m_display_estimated
                    && m_ctdelay <= 0
                )
        {
            --m_ctdelay;
        }
        else
        {
            m_ctdelay = 0;
        }
        if (    m_ctdelay >= m_delay                          || m_ctdelay <= (m_delay*-1)                     || value == m_maximum                            || elapsed > m_display_estimated                 || ( elapsed > 0 && elapsed < 4 )             )
        {
            m_display_estimated = estimated;
            m_ctdelay = 0;
        }
    }

    if ( value != 0 )
    {
        long display_remaining = m_display_estimated - elapsed;
        if ( display_remaining < 0 )
        {
            display_remaining = 0;
        }

        estimatedTime = m_display_estimated;
        remainingTime = display_remaining;
    }

    elapsedTime = elapsed;
}

wxString wxGenericProgressDialog::GetFormattedTime(unsigned long timeInSec)
{
    wxString timeAsHMS;

    if ( timeInSec == (unsigned long)-1 )
    {
        timeAsHMS = _("Unknown");
    }
    else
    {
        unsigned hours = timeInSec / 3600;
        unsigned minutes = (timeInSec % 3600) / 60;
        unsigned seconds = timeInSec % 60;
        timeAsHMS.Printf("%u:%02u:%02u", hours, minutes, seconds);
    }

    return timeAsHMS;
}

wxStaticText *
wxGenericProgressDialog::CreateLabel(const wxString& text, wxSizer *sizer)
{
    wxStaticText *label = new wxStaticText(this, wxID_ANY, text);
    wxStaticText *value = new wxStaticText(this, wxID_ANY, _("unknown"));

    #if defined(__WXMSW__) || defined(__WXMAC__) || defined(__WXGTK20__)
        sizer->Add(label, 1, wxALIGN_RIGHT | wxTOP | wxRIGHT, LAYOUT_MARGIN);
    sizer->Add(value, 1, wxALIGN_LEFT | wxTOP, LAYOUT_MARGIN);
#else
        sizer->Add(label);
    sizer->Add(value, 0, wxLEFT, LAYOUT_MARGIN);
#endif

    return value;
}


bool
wxGenericProgressDialog::Update(int value, const wxString& newmsg, bool *skip)
{
    if ( !DoBeforeUpdate(skip) )
        return false;

    wxCHECK_MSG( m_gauge, false, "dialog should be fully created" );

#ifdef __WXMSW__
    value /= m_factor;
#endif 
    wxASSERT_MSG( value <= m_maximum, wxT("invalid progress value") );

    m_gauge->SetValue(value);

    UpdateMessage(newmsg);

    if ( (m_elapsed || m_remaining || m_estimated) && (value != 0) )
    {
        unsigned long elapsed;
        unsigned long display_remaining;

        UpdateTimeEstimates( value,
                             elapsed,
                             m_display_estimated,
                             display_remaining );

        SetTimeLabel(elapsed, m_elapsed);
        SetTimeLabel(m_display_estimated, m_estimated);
        SetTimeLabel(display_remaining, m_remaining);
    }

    if ( value == m_maximum )
    {
        if ( m_state == Finished )
        {
                                                            return true;
        }

                        m_state = Finished;
        if( !HasPDFlag(wxPD_AUTO_HIDE) )
        {
            EnableClose();
            DisableSkip();
#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
            EnableCloseButton();
#endif 
            if ( newmsg.empty() )
            {
                                m_msg->SetLabel(_("Done."));
            }

                                                wxEventLoopBase::GetActive()->YieldFor(wxEVT_CATEGORY_UI);

                                                (void)ShowModal();
        }
        else         {
                                                ReenableOtherWindows();

            Hide();
        }
    }
    else     {
        DoAfterUpdate();
    }

        Update();

    return m_state != Canceled;
}

bool wxGenericProgressDialog::Pulse(const wxString& newmsg, bool *skip)
{
    if ( !DoBeforeUpdate(skip) )
        return false;

    wxCHECK_MSG( m_gauge, false, "dialog should be fully created" );

        m_gauge->Pulse();

    UpdateMessage(newmsg);

    if (m_elapsed || m_remaining || m_estimated)
    {
        unsigned long elapsed = wxGetCurrentTime() - m_timeStart;

        SetTimeLabel(elapsed, m_elapsed);
        SetTimeLabel((unsigned long)-1, m_estimated);
        SetTimeLabel((unsigned long)-1, m_remaining);
    }

    DoAfterUpdate();

    return m_state != Canceled;
}

bool wxGenericProgressDialog::DoBeforeUpdate(bool *skip)
{
                    wxEventLoopBase::GetActive()->YieldFor(wxEVT_CATEGORY_UI|wxEVT_CATEGORY_USER_INPUT);

    Update();

    if ( m_skip && skip && !*skip )
    {
        *skip = true;
        m_skip = false;
        EnableSkip();
    }

    return m_state != Canceled;
}

void wxGenericProgressDialog::DoAfterUpdate()
{
                wxEventLoopBase::GetActive()->YieldFor(wxEVT_CATEGORY_UI);
}

void wxGenericProgressDialog::Resume()
{
    m_state = Continue;
    m_ctdelay = m_delay;     m_break += wxGetCurrentTime()-m_timeStop;

    EnableAbort();
    EnableSkip();
    m_skip = false;
}

bool wxGenericProgressDialog::Show( bool show )
{
                if(!show)
        ReenableOtherWindows();

    return wxDialog::Show(show);
}

int wxGenericProgressDialog::GetValue() const
{
    wxCHECK_MSG( m_gauge, -1, "dialog should be fully created" );

    return m_gauge->GetValue();
}

int wxGenericProgressDialog::GetRange() const
{
    return m_maximum;
}

wxString wxGenericProgressDialog::GetMessage() const
{
    return m_msg->GetLabel();
}

void wxGenericProgressDialog::SetRange(int maximum)
{
    wxCHECK_RET( m_gauge, "dialog should be fully created" );

    wxCHECK_RET( maximum > 0, "Invalid range" );

    m_gauge->SetRange(maximum);

    SetMaximum(maximum);
}

void wxGenericProgressDialog::SetMaximum(int maximum)
{
    m_maximum = maximum;

#if defined(__WXMSW__)
            m_factor = m_maximum / 65536 + 1;
#endif }


bool wxGenericProgressDialog::WasCancelled() const
{
    return HasPDFlag(wxPD_CAN_ABORT) && m_state == Canceled;
}

bool wxGenericProgressDialog::WasSkipped() const
{
    return HasPDFlag(wxPD_CAN_SKIP) && m_skip;
}

void wxGenericProgressDialog::SetTimeLabel(unsigned long val,
                                           wxStaticText *label)
{
    if ( label )
    {
        wxString s;

        if (val != (unsigned long)-1)
        {
            s = GetFormattedTime(val);
        }
        else
        {
            s = _("Unknown");
        }

        if ( s != label->GetLabel() )
            label->SetLabel(s);
    }
}


void wxGenericProgressDialog::OnCancel(wxCommandEvent& event)
{
    if ( m_state == Finished )
    {
                        event.Skip();
    }
    else
    {
                        m_state = Canceled;

                        DisableAbort();
        DisableSkip();

                m_timeStop = wxGetCurrentTime();
    }
}

void wxGenericProgressDialog::OnSkip(wxCommandEvent& WXUNUSED(event))
{
    DisableSkip();
    m_skip = true;
}

void wxGenericProgressDialog::OnClose(wxCloseEvent& event)
{
    if ( m_state == Uncancelable )
    {
                event.Veto();
    }
    else if ( m_state == Finished )
    {
                event.Skip();
    }
    else
    {
                m_state = Canceled;
        DisableAbort();
        DisableSkip();

        m_timeStop = wxGetCurrentTime();
    }
}


wxGenericProgressDialog::~wxGenericProgressDialog()
{
        ReenableOtherWindows();

    if ( m_tempEventLoop )
    {
        wxEventLoopBase::SetActive(NULL);
        delete m_tempEventLoop;
    }
}

void wxGenericProgressDialog::DisableOtherWindows()
{
    if ( HasPDFlag(wxPD_APP_MODAL) )
    {
        m_winDisabler = new wxWindowDisabler(this);
    }
    else
    {
        if ( m_parentTop )
            m_parentTop->Disable();
        m_winDisabler = NULL;
    }
}

void wxGenericProgressDialog::ReenableOtherWindows()
{
    if ( HasPDFlag(wxPD_APP_MODAL) )
    {
        wxDELETE(m_winDisabler);
    }
    else
    {
        if ( m_parentTop )
            m_parentTop->Enable();
    }
}


void wxGenericProgressDialog::EnableSkip(bool enable)
{
    if ( HasPDFlag(wxPD_CAN_SKIP) )
    {
        if(m_btnSkip)
            m_btnSkip->Enable(enable);
    }
}

void wxGenericProgressDialog::EnableAbort(bool enable)
{
    if( HasPDFlag(wxPD_CAN_ABORT) )
    {
        if(m_btnAbort)
            m_btnAbort->Enable(enable);
    }
}

void wxGenericProgressDialog::EnableClose()
{
    if(HasPDFlag(wxPD_CAN_ABORT))
    {
        if(m_btnAbort)
        {
            m_btnAbort->Enable();
            m_btnAbort->SetLabel(_("Close"));
        }
    }
}

void wxGenericProgressDialog::UpdateMessage(const wxString &newmsg)
{
    if ( !newmsg.empty() && newmsg != m_msg->GetLabel() )
    {
        m_msg->SetLabel(newmsg);

                                wxEventLoopBase::GetActive()->YieldFor(wxEVT_CATEGORY_UI);
    }
}

#endif 