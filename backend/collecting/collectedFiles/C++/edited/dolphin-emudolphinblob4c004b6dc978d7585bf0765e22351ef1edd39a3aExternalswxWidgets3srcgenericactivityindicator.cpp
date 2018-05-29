


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_ACTIVITYINDICATOR && !defined(__WXGTK3__)

#include "wx/activityindicator.h"

#ifndef WX_PRECOMP
    #include "wx/dcclient.h"
    #include "wx/timer.h"
#endif 
#include "wx/graphics.h"
#include "wx/scopedptr.h"


namespace
{

static const int NUM_DOTS = 8;

static const int FRAME_DELAY = 150;

} 

class wxActivityIndicatorImpl
{
public:
    explicit wxActivityIndicatorImpl(wxWindow* win)
        : m_timer(this),
          m_win(win)
    {
        m_frame = 0;

        win->Bind(wxEVT_PAINT, &wxActivityIndicatorImpl::OnPaint, this);
    }

    void Start()
    {
                                if ( m_timer.IsRunning() )
            return;

        m_timer.Start(FRAME_DELAY);
    }

    void Stop()
    {
                        m_timer.Stop();
    }

    bool IsRunning() const
    {
        return m_timer.IsRunning();
    }

        void Advance()
    {
        if ( ++m_frame == NUM_DOTS )
            m_frame = 0;

        m_win->Refresh();
    }

private:
    class AdvanceTimer : public wxTimer
    {
    public:
        explicit AdvanceTimer(wxActivityIndicatorImpl* owner)
            : m_owner(owner)
        {
        }

        virtual void Notify() wxOVERRIDE
        {
            m_owner->Advance();
        }

    private:
        wxActivityIndicatorImpl* const m_owner;

        wxDECLARE_NO_COPY_CLASS(AdvanceTimer);
    };

    void OnPaint(wxPaintEvent& WXUNUSED(event))
    {
        wxPaintDC pdc(m_win);

        wxScopedPtr<wxGraphicsContext> const
            gc(wxGraphicsRenderer::GetDefaultRenderer()->CreateContext(pdc));

        const wxSize size = m_win->GetClientSize();

                gc->Translate(size.x/2., size.y/2.);

                                static const double RADIUS_FACTOR = 10;

        const double r = wxMin(size.x, size.y) / RADIUS_FACTOR;

                wxGraphicsPath path = gc->CreatePath();
        path.AddCircle(0, -(RADIUS_FACTOR / 2. - 1.)*r, r);

                        const double angle = wxDegToRad(360. / NUM_DOTS);

                        gc->Rotate(m_frame*angle);

        const bool isEnabled = m_win->IsThisEnabled();
        for ( int n = 0; n < NUM_DOTS; n++ )
        {
                                    const int opacityIndex = isEnabled ? n + 1 : 2;

                                                const int opacity = opacityIndex*(wxALPHA_OPAQUE + 1)/NUM_DOTS - 1;

            gc->SetBrush(wxBrush(wxColour(0, 0, 0, opacity)));

            gc->FillPath(path);
            gc->Rotate(angle);
        }
    }

    AdvanceTimer m_timer;
    wxWindow* const m_win;

    int m_frame;

    wxDECLARE_NO_COPY_CLASS(wxActivityIndicatorImpl);
};


#ifndef wxHAS_NATIVE_ACTIVITYINDICATOR
    wxIMPLEMENT_DYNAMIC_CLASS(wxActivityIndicator, wxControl);
#endif

bool
wxActivityIndicatorGeneric::Create(wxWindow* parent,
                                   wxWindowID winid,
                                   const wxPoint& pos,
                                   const wxSize& size,
                                   long style,
                                   const wxString& name)
{
            if ( !wxWindow::Create(parent, winid, pos, size, style, name) )
        return false;

    m_impl = new wxActivityIndicatorImpl(this);

    return true;
}

wxActivityIndicatorGeneric::~wxActivityIndicatorGeneric()
{
    delete m_impl;
}

void wxActivityIndicatorGeneric::Start()
{
    wxCHECK_RET( m_impl, wxS("Must be created first") );

    m_impl->Start();
}

void wxActivityIndicatorGeneric::Stop()
{
    wxCHECK_RET( m_impl, wxS("Must be created first") );

    m_impl->Stop();
}

bool wxActivityIndicatorGeneric::IsRunning() const
{
    return m_impl && m_impl->IsRunning();
}

wxSize wxActivityIndicatorGeneric::DoGetBestClientSize() const
{
    int size = 0;
    switch ( GetWindowVariant() )
    {
        case wxWINDOW_VARIANT_MAX:
            wxFAIL_MSG(wxS("Invalid window variant"));
            wxFALLTHROUGH;

        case wxWINDOW_VARIANT_NORMAL:
            size = 24;
            break;

        case wxWINDOW_VARIANT_SMALL:
            size = 16;
            break;

        case wxWINDOW_VARIANT_MINI:
            size = 12;
            break;

        case wxWINDOW_VARIANT_LARGE:
            size = 32;
            break;
    }

    wxASSERT_MSG( size, wxS("Unknown window variant") );

    return FromDIP(wxSize(size, size));
}

#endif 