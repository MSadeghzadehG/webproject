


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_RICHTOOLTIP

#ifndef WX_PRECOMP
    #include "wx/dcmemory.h"
    #include "wx/icon.h"
    #include "wx/region.h"
    #include "wx/settings.h"
    #include "wx/sizer.h"
    #include "wx/statbmp.h"
    #include "wx/stattext.h"
    #include "wx/timer.h"
    #include "wx/utils.h"
#endif 
#include "wx/private/richtooltip.h"
#include "wx/generic/private/richtooltip.h"

#include "wx/artprov.h"
#include "wx/custombgwin.h"
#include "wx/display.h"
#include "wx/graphics.h"
#include "wx/popupwin.h"
#include "wx/textwrapper.h"

#ifdef __WXMSW__
    #include "wx/msw/uxtheme.h"

    static const int TTP_BALLOONTITLE = 4;

    static const int TMT_TEXTCOLOR = 3803;
    static const int TMT_GRADIENTCOLOR1 = 3810;
    static const int TMT_GRADIENTCOLOR2 = 3811;
#endif


class wxRichToolTipPopup :
    public wxCustomBackgroundWindow<wxPopupTransientWindow>
{
public:
    wxRichToolTipPopup(wxWindow* parent,
                       const wxString& title,
                       const wxString& message,
                       const wxIcon& icon,
                       wxTipKind tipKind,
                       const wxFont& titleFont_) :
        m_timer(this)
    {
        Create(parent, wxFRAME_SHAPED);


        wxBoxSizer* const sizerTitle = new wxBoxSizer(wxHORIZONTAL);
        if ( icon.IsOk() )
        {
            sizerTitle->Add(new wxStaticBitmap(this, wxID_ANY, icon),
                            wxSizerFlags().Centre().Border(wxRIGHT));
        }
        
        wxStaticText* const labelTitle = new wxStaticText(this, wxID_ANY, "");
        labelTitle->SetLabelText(title);

        wxFont titleFont(titleFont_);
        if ( !titleFont.IsOk() )
        {
                        titleFont = labelTitle->GetFont();

#ifdef __WXMSW__
                                    wxUxThemeEngine* const theme = GetTooltipTheme();
            if ( theme )
            {
                titleFont.MakeLarger();

                COLORREF c;
                if ( FAILED(theme->GetThemeColor
                                   (
                                        wxUxThemeHandle(parent, L"TOOLTIP"),
                                        TTP_BALLOONTITLE,
                                        0,
                                        TMT_TEXTCOLOR,
                                        &c
                                    )) )
                {
                                        c = 0x993300;
                }

                labelTitle->SetForegroundColour(wxRGBToColour(c));
            }
            else
#endif             {
                                                titleFont.MakeBold();
            }
        }

        labelTitle->SetFont(titleFont);
        sizerTitle->Add(labelTitle, wxSizerFlags().Centre());

        wxBoxSizer* const sizerTop = new wxBoxSizer(wxVERTICAL);
        sizerTop->Add(sizerTitle,
                        wxSizerFlags().DoubleBorder(wxLEFT|wxRIGHT|wxTOP));

                        sizerTop->AddSpacer(wxSizerFlags::GetDefaultBorder());

        wxTextSizerWrapper wrapper(this);
        wxSizer* sizerText = wrapper.CreateSizer(message, -1 );

#ifdef __WXMSW__
        if ( icon.IsOk() && GetTooltipTheme() )
        {
                                    wxBoxSizer* const sizerTextIndent = new wxBoxSizer(wxHORIZONTAL);
            sizerTextIndent->AddSpacer(icon.GetWidth());
            sizerTextIndent->Add(sizerText,
                                    wxSizerFlags().Border(wxLEFT).Centre());

            sizerText = sizerTextIndent;
        }
#endif         sizerTop->Add(sizerText,
                        wxSizerFlags().DoubleBorder(wxLEFT|wxRIGHT|wxBOTTOM)
                                      .Centre());

        SetSizer(sizerTop);

        const int offsetY = SetTipShapeAndSize(tipKind, GetBestSize());
        if ( offsetY > 0 )
        {
                                    sizerTop->PrependSpacer(offsetY);
        }

        Layout();
    }

    void SetBackgroundColours(wxColour colStart, wxColour colEnd)
    {
        if ( !colStart.IsOk() )
        {
            #ifdef __WXMSW__
            wxUxThemeEngine* const theme = GetTooltipTheme();
            if ( theme )
            {
                wxUxThemeHandle hTheme(GetParent(), L"TOOLTIP");

                COLORREF c1, c2;
                if ( FAILED(theme->GetThemeColor
                                   (
                                        hTheme,
                                        TTP_BALLOONTITLE,
                                        0,
                                        TMT_GRADIENTCOLOR1,
                                        &c1
                                    )) ||
                    FAILED(theme->GetThemeColor
                                  (
                                        hTheme,
                                        TTP_BALLOONTITLE,
                                        0,
                                        TMT_GRADIENTCOLOR2,
                                        &c2
                                  )) )
                {
                    c1 = 0xffffff;
                    c2 = 0xf0e5e4;
                }

                colStart = wxRGBToColour(c1);
                colEnd = wxRGBToColour(c2);
            }
            else
#endif             {
                colStart = wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK);
            }
        }

        if ( colEnd.IsOk() )
        {
                        const wxSize size = GetClientSize();
            wxBitmap bmp(size);
            {
                wxMemoryDC dc(bmp);
                dc.Clear();
                dc.GradientFillLinear(size, colStart, colEnd, wxDOWN);
            }

            SetBackgroundBitmap(bmp);
        }
        else         {
            SetBackgroundColour(colStart);
        }
    }

    void SetPosition(const wxRect* rect)
    {
        wxPoint pos;

        if ( !rect || rect->IsEmpty() )
            pos = GetTipPoint();
        else
            pos = GetParent()->ClientToScreen( wxPoint( rect->x + rect->width / 2, rect->y + rect->height / 2 ) );

                        pos -= m_anchorPos;

        Move(pos, wxSIZE_NO_ADJUSTMENTS);
    }

    void DoShow()
    {
        Popup();
    }

    void SetTimeoutAndShow(unsigned timeout, unsigned delay)
    {
        if ( !timeout && !delay )
        {
            DoShow();
            return;
        }

        Connect(wxEVT_TIMER, wxTimerEventHandler(wxRichToolTipPopup::OnTimer));

        m_timeout = timeout;         m_delayShow = delay != 0;

        if ( !m_delayShow )
            DoShow();

        m_timer.Start((delay ? delay : timeout), true );
    }

protected:
    virtual void OnDismiss() wxOVERRIDE
    {
        Destroy();
    }

private:
#ifdef __WXMSW__
        static wxUxThemeEngine* GetTooltipTheme()
    {
                if ( wxGetWinVersion() <= wxWinVersion_XP )
            return NULL;

        return wxUxThemeEngine::GetIfActive();
    }
#endif 
            static int GetTipHeight()
    {
#ifdef __WXMSW__
        if ( GetTooltipTheme() )
            return 20;
#endif 
        return 15;
    }

        wxPoint GetTipPoint() const
    {
                                        const wxRect r = GetParent()->GetScreenRect();
        return wxPoint(r.x + r.width/2, r.y + r.height/2);
    }

                                wxTipKind GetBestTipKind() const
    {
        const wxPoint pos = GetTipPoint();

                        int dpy = wxDisplay::GetFromWindow(GetParent());
        if ( dpy == wxNOT_FOUND )
            dpy = 0; 
        const wxRect rectDpy = wxDisplay(dpy).GetClientArea();

#ifdef __WXMAC__
        return pos.y > rectDpy.height/2 ? wxTipKind_Bottom : wxTipKind_Top;
#else         return pos.y > rectDpy.height/2
                    ? pos.x > rectDpy.width/2
                        ? wxTipKind_BottomRight
                        : wxTipKind_BottomLeft
                    : pos.x > rectDpy.width/2
                        ? wxTipKind_TopRight
                        : wxTipKind_TopLeft;
#endif     }

            int SetTipShapeAndSize(wxTipKind tipKind, const wxSize& contentSize)
    {
#if wxUSE_GRAPHICS_CONTEXT
        wxSize size = contentSize;

                                const int tipSize = GetTipHeight();
        const int tipOffset = tipSize;

                int x = -1;

                int yBase = -1,
            yApex = -1;

                int dy = -1;

                                enum RectPoint
        {
            RectPoint_TopLeft,
            RectPoint_Top,
            RectPoint_TopRight,
            RectPoint_Right,
            RectPoint_BotRight,
            RectPoint_Bot,
            RectPoint_BotLeft,
            RectPoint_Left,
            RectPoint_Max
        };

                        RectPoint pointStart = RectPoint_Max;


                const double RADIUS = 5;

                wxGraphicsPath
            path = wxGraphicsRenderer::GetDefaultRenderer()->CreatePath();

        if ( tipKind == wxTipKind_Auto )
            tipKind = GetBestTipKind();

                        wxPoint2DDouble tipPoints[3];

        switch ( tipKind )
        {
            case wxTipKind_Auto:
                wxFAIL_MSG( "Impossible kind value" );
                break;

            case wxTipKind_TopLeft:
                x = tipOffset;
                yApex = 0;
                yBase = tipSize;
                dy = tipSize;

                tipPoints[0] = wxPoint2DDouble(x, yBase);
                tipPoints[1] = wxPoint2DDouble(x, yApex);
                tipPoints[2] = wxPoint2DDouble(x + tipSize, yBase);

                pointStart = RectPoint_TopRight;
                break;

            case wxTipKind_TopRight:
                x = size.x - tipOffset;
                yApex = 0;
                yBase = tipSize;
                dy = tipSize;

                tipPoints[0] = wxPoint2DDouble(x - tipSize, yBase);
                tipPoints[1] = wxPoint2DDouble(x, yApex);
                tipPoints[2] = wxPoint2DDouble(x, yBase);

                pointStart = RectPoint_TopRight;
                break;

            case wxTipKind_BottomLeft:
                x = tipOffset;
                yApex = size.y + tipSize;
                yBase = size.y;
                dy = 0;

                tipPoints[0] = wxPoint2DDouble(x + tipSize, yBase);
                tipPoints[1] = wxPoint2DDouble(x, yApex);
                tipPoints[2] = wxPoint2DDouble(x, yBase);

                pointStart = RectPoint_BotLeft;
                break;

            case wxTipKind_BottomRight:
                x = size.x - tipOffset;
                yApex = size.y + tipSize;
                yBase = size.y;
                dy = 0;

                tipPoints[0] = wxPoint2DDouble(x, yBase);
                tipPoints[1] = wxPoint2DDouble(x, yApex);
                tipPoints[2] = wxPoint2DDouble(x - tipSize, yBase);

                pointStart = RectPoint_BotLeft;
                break;

            case wxTipKind_Top:
                x = size.x/2;
                yApex = 0;
                yBase = tipSize;
                dy = tipSize;

                {
                                                            const double halfside = tipSize/1.73;

                    tipPoints[0] = wxPoint2DDouble(x - halfside, yBase);
                    tipPoints[1] = wxPoint2DDouble(x, yApex);
                    tipPoints[2] = wxPoint2DDouble(x + halfside, yBase);
                }

                pointStart = RectPoint_TopRight;
                break;

            case wxTipKind_Bottom:
                x = size.x/2;
                yApex = size.y + tipSize;
                yBase = size.y;
                dy = 0;

                {
                    const double halfside = tipSize/1.73;

                    tipPoints[0] = wxPoint2DDouble(x + halfside, yBase);
                    tipPoints[1] = wxPoint2DDouble(x, yApex);
                    tipPoints[2] = wxPoint2DDouble(x - halfside, yBase);
                }

                pointStart = RectPoint_BotLeft;
                break;

            case wxTipKind_None:
                x = size.x/2;
                dy = 0;

                path.AddRoundedRectangle(0, 0, size.x, size.y, RADIUS);
                break;
        }

        wxASSERT_MSG( dy != -1, wxS("Unknown tip kind?") );

        size.y += tipSize;
        SetSize(size);

        if ( tipKind != wxTipKind_None )
        {
            path.MoveToPoint(tipPoints[0]);
            path.AddLineToPoint(tipPoints[1]);
            path.AddLineToPoint(tipPoints[2]);

            const double xLeft = 0.;
            const double xMid = size.x/2.;
            const double xRight = size.x;

            const double yTop = dy;
            const double yMid = (dy + size.y)/2.;
            const double yBot = dy + contentSize.y;

            wxPoint2DDouble rectPoints[RectPoint_Max];
            rectPoints[RectPoint_TopLeft]  = wxPoint2DDouble(xLeft,  yTop);
            rectPoints[RectPoint_Top]      = wxPoint2DDouble(xMid,   yTop);
            rectPoints[RectPoint_TopRight] = wxPoint2DDouble(xRight, yTop);
            rectPoints[RectPoint_Right]    = wxPoint2DDouble(xRight, yMid);
            rectPoints[RectPoint_BotRight] = wxPoint2DDouble(xRight, yBot);
            rectPoints[RectPoint_Bot]      = wxPoint2DDouble(xMid,   yBot);
            rectPoints[RectPoint_BotLeft]  = wxPoint2DDouble(xLeft,  yBot);
            rectPoints[RectPoint_Left]     = wxPoint2DDouble(xLeft,  yMid);

                        unsigned n = pointStart;
            for ( unsigned corner = 0; corner < 3; corner++ )
            {
                const wxPoint2DDouble& pt1 = rectPoints[n];

                n = (n + 1) % RectPoint_Max;

                const wxPoint2DDouble& pt2 = rectPoints[n];

                path.AddArcToPoint(pt1.m_x, pt1.m_y, pt2.m_x, pt2.m_y, RADIUS);

                n = (n + 1) % RectPoint_Max;
            }

                        const wxPoint2DDouble& pt1 = rectPoints[n];
            const wxPoint2DDouble& pt2 = tipPoints[0];

            path.AddArcToPoint(pt1.m_x, pt1.m_y, pt2.m_x, pt2.m_y, RADIUS);

            path.CloseSubpath();
        }

        SetShape(path);
#else         wxUnusedVar(tipKind);

        int x = contentSize.x/2,
            yApex = 0,
            dy = 0;

        SetSize(contentSize);
#endif 
        m_anchorPos.x = x;
        m_anchorPos.y = yApex;

        return dy;
    }

        void OnTimer(wxTimerEvent& WXUNUSED(event))
    {
        if ( !m_delayShow )
        {
                                                DismissAndNotify();

            return;
        }

        m_delayShow = false;

        if ( m_timeout )
            m_timer.Start(m_timeout, true);

        DoShow();
    }


            wxPoint m_anchorPos;

        wxTimer m_timer;

        int m_timeout;

        bool m_delayShow;

    wxDECLARE_NO_COPY_CLASS(wxRichToolTipPopup);
};


void
wxRichToolTipGenericImpl::SetBackgroundColour(const wxColour& col,
                                              const wxColour& colEnd)
{
    m_colStart = col;
    m_colEnd = colEnd;
}

void wxRichToolTipGenericImpl::SetCustomIcon(const wxIcon& icon)
{
    m_icon = icon;
}

void wxRichToolTipGenericImpl::SetStandardIcon(int icon)
{
    switch ( icon & wxICON_MASK )
    {
        case wxICON_WARNING:
        case wxICON_ERROR:
        case wxICON_INFORMATION:
                                                m_icon = wxArtProvider::GetIcon
                     (
                        wxArtProvider::GetMessageBoxIconId(icon),
                        wxART_LIST
                     );
            break;

        case wxICON_QUESTION:
            wxFAIL_MSG("Question icon doesn't make sense for a tooltip");
            break;

        case wxICON_NONE:
            m_icon = wxNullIcon;
            break;
    }
}

void wxRichToolTipGenericImpl::SetTimeout(unsigned millisecondsTimeout,
                                          unsigned millisecondsDelay)
{
    m_delay = millisecondsDelay;
    m_timeout = millisecondsTimeout;
}

void wxRichToolTipGenericImpl::SetTipKind(wxTipKind tipKind)
{
    m_tipKind = tipKind;
}

void wxRichToolTipGenericImpl::SetTitleFont(const wxFont& font)
{
    m_titleFont = font;
}

void wxRichToolTipGenericImpl::ShowFor(wxWindow* win, const wxRect* rect)
{
        win->SetFocus();

    wxRichToolTipPopup* const popup = new wxRichToolTipPopup
                                          (
                                            win,
                                            m_title,
                                            m_message,
                                            m_icon,
                                            m_tipKind,
                                            m_titleFont
                                          );

    popup->SetBackgroundColours(m_colStart, m_colEnd);

    popup->SetPosition(rect);
        popup->SetTimeoutAndShow( m_timeout, m_delay );
}

#ifndef __WXMSW__


wxRichToolTipImpl*
wxRichToolTipImpl::Create(const wxString& title, const wxString& message)
{
    return new wxRichToolTipGenericImpl(title, message);
}

#endif 
#endif 