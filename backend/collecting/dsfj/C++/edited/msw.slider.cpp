


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_SLIDER

#include "wx/slider.h"

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcctl.h"     #include "wx/brush.h"
#endif

#include "wx/msw/subwin.h"


namespace
{

enum
{
    SliderLabel_Min,
    SliderLabel_Max,
    SliderLabel_Value,
    SliderLabel_Last
};

const int HGAP = 5;
const int VGAP = 4;
const int BORDERPAD = 8;
const int THUMB = 24;
const int TICK = 8;

} 


void wxSlider::Init()
{
    m_labels = NULL;

    m_hBrushBg = NULL;

    m_pageSize = 1;
    m_lineSize = 1;
    m_rangeMax = 0;
    m_rangeMin = 0;
    m_tickFreq = 0;

    m_isDragging = false;
}

bool wxSlider::Create(wxWindow *parent,
                      wxWindowID id,
                      int value,
                      int minValue,
                      int maxValue,
                      const wxPoint& pos,
                      const wxSize& size,
                      long style,
                      const wxValidator& validator,
                      const wxString& name)
{
    wxCHECK_MSG( minValue < maxValue, false,
        wxT("Slider minimum must be strictly less than the maximum.") );

                    switch ( style & (wxSL_LEFT | wxSL_RIGHT | wxSL_TOP | wxSL_BOTTOM) )
    {
        case wxSL_LEFT:
        case wxSL_RIGHT:
            style |= wxSL_VERTICAL;
            break;

        case wxSL_TOP:
        case wxSL_BOTTOM:
            style |= wxSL_HORIZONTAL;
            break;

        case 0:
                        if ( !(style & (wxSL_HORIZONTAL | wxSL_VERTICAL)) )
            {
                                style |= wxSL_BOTTOM | wxSL_HORIZONTAL;
            }
    };

    wxASSERT_MSG( !(style & wxSL_VERTICAL) || !(style & wxSL_HORIZONTAL),
                    wxT("incompatible slider direction and orientation") );


        if ( !CreateControl(parent, id, pos, size, style, validator, name) )
        return false;

        m_rangeMin = minValue;
    m_rangeMax = maxValue;

                                if ( m_windowStyle & wxSL_LABELS )
    {
        m_labels = new wxSubwindows(SliderLabel_Last);

        HWND hwndParent = GetHwndOf(parent);
        for ( size_t n = 0; n < SliderLabel_Last; n++ )
        {
            wxWindowIDRef lblid = NewControlId();

            HWND wnd = ::CreateWindow
                         (
                            wxT("STATIC"),
                            NULL,
                            WS_CHILD | WS_VISIBLE | SS_CENTER,
                            0, 0, 0, 0,
                            hwndParent,
                            (HMENU)wxUIntToPtr(lblid.GetValue()),
                            wxGetInstance(),
                            NULL
                         );

            m_labels->Set(n, wnd, lblid);
        }
        m_labels->SetFont(GetFont());
    }

        if ( !MSWCreateControl(TRACKBAR_CLASS, wxEmptyString, pos, size) )
        return false;

        SetRange(minValue, maxValue);
    SetValue(value);
    SetPageSize( wxMax(1, (maxValue - minValue)/10) );

                if ( m_labels && size.x != wxDefaultCoord && size.y != wxDefaultCoord )
    {
        SetSize(size);
    }

    return true;
}

WXDWORD wxSlider::MSWGetStyle(long style, WXDWORD *exstyle) const
{
    WXDWORD msStyle = wxControl::MSWGetStyle(style, exstyle);

        msStyle |= style & wxSL_VERTICAL ? TBS_VERT : TBS_HORZ;

    if ( style & wxSL_BOTH )
    {
                msStyle |= TBS_BOTH;
    }
    else     {
        if ( style & wxSL_LEFT )
            msStyle |= TBS_LEFT;
        else if ( style & wxSL_RIGHT )
            msStyle |= TBS_RIGHT;
        else if ( style & wxSL_TOP )
            msStyle |= TBS_TOP;
        else if ( style & wxSL_BOTTOM )
            msStyle |= TBS_BOTTOM;
    }

    if ( style & wxSL_AUTOTICKS )
        msStyle |= TBS_AUTOTICKS;
    else
        msStyle |= TBS_NOTICKS;

    if ( style & wxSL_SELRANGE )
        msStyle |= TBS_ENABLESELRANGE;

    return msStyle;
}

wxSlider::~wxSlider()
{
    delete m_labels;
}


bool wxSlider::MSWOnScroll(int WXUNUSED(orientation),
                           WXWORD wParam,
                           WXWORD WXUNUSED(pos),
                           WXHWND control)
{
    wxEventType scrollEvent;
    switch ( wParam )
    {
        case SB_TOP:
            scrollEvent = wxEVT_SCROLL_TOP;
            break;

        case SB_BOTTOM:
            scrollEvent = wxEVT_SCROLL_BOTTOM;
            break;

        case SB_LINEUP:
            scrollEvent = wxEVT_SCROLL_LINEUP;
            break;

        case SB_LINEDOWN:
            scrollEvent = wxEVT_SCROLL_LINEDOWN;
            break;

        case SB_PAGEUP:
            scrollEvent = wxEVT_SCROLL_PAGEUP;
            break;

        case SB_PAGEDOWN:
            scrollEvent = wxEVT_SCROLL_PAGEDOWN;
            break;

        case SB_THUMBTRACK:
            scrollEvent = wxEVT_SCROLL_THUMBTRACK;
            m_isDragging = true;
            break;

        case SB_THUMBPOSITION:
            if ( m_isDragging )
            {
                scrollEvent = wxEVT_SCROLL_THUMBRELEASE;
                m_isDragging = false;
            }
            else
            {
                                                                                                                scrollEvent = wxEVT_SCROLL_CHANGED;
            }
            break;

        case SB_ENDSCROLL:
            scrollEvent = wxEVT_SCROLL_CHANGED;
            break;

        default:
                        return false;
    }

    int newPos = ValueInvertOrNot((int) ::SendMessage((HWND) control, TBM_GETPOS, 0, 0));
    if ( (newPos < GetMin()) || (newPos > GetMax()) )
    {
                return true;
    }

    SetValue(newPos);

    wxScrollEvent event(scrollEvent, m_windowId);
    event.SetPosition(newPos);
    event.SetEventObject( this );
    HandleWindowEvent(event);

    wxCommandEvent cevent( wxEVT_SLIDER, GetId() );
    cevent.SetInt( newPos );
    cevent.SetEventObject( this );

    return HandleWindowEvent( cevent );
}

void wxSlider::Command (wxCommandEvent & event)
{
    SetValue (event.GetInt());
    ProcessCommand (event);
}


wxRect wxSlider::GetBoundingBox() const
{
        int x, y, w, h;
    wxSliderBase::DoGetPosition(&x, &y);
    wxSliderBase::DoGetSize(&w, &h);

    wxRect rect(x, y, w, h);
    if ( m_labels )
    {
        wxRect lrect = m_labels->GetBoundingBox();
        GetParent()->ScreenToClient(&lrect.x, &lrect.y);
        rect.Union(lrect);
    }

    return rect;
}

void wxSlider::DoGetSize(int *width, int *height) const
{
    wxRect rect = GetBoundingBox();

    if ( width )
        *width = rect.width;
    if ( height )
        *height = rect.height;
}

void wxSlider::DoGetPosition(int *x, int *y) const
{
    wxRect rect = GetBoundingBox();

    if ( x )
        *x = rect.x;
    if ( y )
        *y = rect.y;
}

int wxSlider::GetLabelsSize(int *widthMin, int *widthMax) const
{
    if ( widthMin && widthMax )
    {
        *widthMin = GetTextExtent(Format(m_rangeMin)).x;
        *widthMax = GetTextExtent(Format(m_rangeMax)).x;

        if ( HasFlag(wxSL_INVERSE) )
        {
            wxSwap(*widthMin, *widthMax);
        }
    }

    return HasFlag(wxSL_LABELS) ? GetCharHeight() : 0;
}

void wxSlider::DoMoveWindow(int x, int y, int width, int height)
{
            if ( !m_labels )
    {
        wxSliderBase::DoMoveWindow(x, y, width, height);
        return;
    }

    int minLabelWidth,
        maxLabelWidth;
    const int labelHeight = GetLabelsSize(&minLabelWidth, &maxLabelWidth);
    const int longestLabelWidth = wxMax(minLabelWidth, maxLabelWidth);
    if ( !HasFlag(wxSL_MIN_MAX_LABELS) )
    {
        minLabelWidth =
        maxLabelWidth = 0;
    }

    int tickOffset = 0;
    if ( HasFlag(wxSL_TICKS))
       tickOffset = TICK;
    if ( HasFlag(wxSL_BOTH))
       tickOffset *= 2;

                if ( HasFlag(wxSL_VERTICAL) )
    {
        int labelOffset =  0;
        int holdTopX;
        int holdBottomX;
        int xLabel = (wxMax((THUMB + (BORDERPAD * 2)), longestLabelWidth) / 2) -
            (longestLabelWidth / 2) + x;
        if ( HasFlag(wxSL_LEFT) )
        {
            holdTopX = xLabel;
            holdBottomX = xLabel - (abs(maxLabelWidth - minLabelWidth) / 2);
        }
        else         {
            holdTopX = xLabel + longestLabelWidth + (abs(maxLabelWidth - minLabelWidth) / 2);
            holdBottomX = xLabel + longestLabelWidth;

            labelOffset = longestLabelWidth + HGAP;
        }

        int labelHeightUsed = 0 ;

        if ( HasFlag(wxSL_MIN_MAX_LABELS) )
        {
            if ( HasFlag(wxSL_INVERSE) )
            {
                wxSwap(holdTopX, holdBottomX);
            }

            DoMoveSibling((HWND)(*m_labels)[SliderLabel_Min],
                holdTopX,
                y,
                minLabelWidth, labelHeight);
            DoMoveSibling((HWND)(*m_labels)[SliderLabel_Max],
                holdBottomX,
                y + height - labelHeight,
                maxLabelWidth, labelHeight);

            labelHeightUsed = labelHeight ;
        }

        if ( HasFlag(wxSL_VALUE_LABEL) )
        {
            DoMoveSibling((HWND)(*m_labels)[SliderLabel_Value],
                x + ( HasFlag(wxSL_LEFT) ? THUMB + tickOffset + HGAP : 0 ),
                y + (height - labelHeight)/2,
                longestLabelWidth, labelHeight);
        }

                wxSliderBase::DoMoveWindow(
            x + labelOffset,
            y + labelHeightUsed,
            THUMB + tickOffset + HGAP,
            height - (labelHeightUsed * 2));
    }
    else     {
        int yLabelMinMax =
            (y + ((THUMB + tickOffset) / 2)) - (labelHeight / 2);
        int xLabelValue =
            x + minLabelWidth +
            ((width  - (minLabelWidth + maxLabelWidth)) / 2) -
            (longestLabelWidth / 2);

        int ySlider = y;
        int minLabelWidthUsed = 0 ;
        int maxLabelWidthUsed = 0 ;

        if ( HasFlag(wxSL_VALUE_LABEL) )
        {
            DoMoveSibling((HWND)(*m_labels)[SliderLabel_Value],
                xLabelValue,
                y + (HasFlag(wxSL_BOTTOM) ? 0 : THUMB + tickOffset),
                longestLabelWidth, labelHeight);

            if ( HasFlag(wxSL_BOTTOM) )
            {
                ySlider += labelHeight;
                yLabelMinMax += labelHeight;
            }
        }

        if ( HasFlag(wxSL_MIN_MAX_LABELS) )
        {
            DoMoveSibling((HWND)(*m_labels)[SliderLabel_Min],
                x,
                yLabelMinMax,
                minLabelWidth, labelHeight);
            DoMoveSibling((HWND)(*m_labels)[SliderLabel_Max],
                x + width - maxLabelWidth,
                yLabelMinMax,
                maxLabelWidth, labelHeight);

            minLabelWidthUsed = minLabelWidth + VGAP ;
            maxLabelWidthUsed = maxLabelWidth + VGAP ;
        }

                wxSliderBase::DoMoveWindow(
            x + minLabelWidthUsed,
            ySlider,
            width - (minLabelWidthUsed + maxLabelWidthUsed),
            THUMB + tickOffset);
    }
}

wxSize wxSlider::DoGetBestSize() const
{
        static const int length = 100;

    int *width;
    wxSize size;
    if ( HasFlag(wxSL_VERTICAL) )
    {
        size.x = THUMB;
        size.y = length;
        width = &size.x;

        if ( m_labels )
        {
            int widthMin,
                widthMax;
            int hLabel = GetLabelsSize(&widthMin, &widthMax);

                        if ( HasFlag(wxSL_MIN_MAX_LABELS) )
                size.x += HGAP + wxMax(widthMin, widthMax);

                        size.y += hLabel;
        }
    }
    else     {
        size.x = length;
        size.y = THUMB;
        width = &size.y;

        if ( m_labels )
        {
            int labelSize = GetLabelsSize();

                        
                                    if ( HasFlag(wxSL_VALUE_LABEL) )
                size.y += labelSize;
        }
    }

        if ( HasFlag(wxSL_TICKS) )
    {
        *width += TICK;
                if ( HasFlag(wxSL_BOTH) )
            *width += TICK;
    }
    return size;
}

WXHBRUSH wxSlider::DoMSWControlColor(WXHDC pDC, wxColour colBg, WXHWND hWnd)
{
    const WXHBRUSH hBrush = wxSliderBase::DoMSWControlColor(pDC, colBg, hWnd);

                if ( hWnd == GetHwnd() && hBrush != m_hBrushBg )
    {
        m_hBrushBg = hBrush;

                        ::PostMessage(hWnd, WM_ENABLE, ::IsWindowEnabled(hWnd), 0);
    }

    return hBrush;
}


int wxSlider::GetValue() const
{
    return ValueInvertOrNot(::SendMessage(GetHwnd(), TBM_GETPOS, 0, 0));
}

void wxSlider::SetValue(int value)
{
    ::SendMessage(GetHwnd(), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)ValueInvertOrNot(value));

    if ( m_labels )
    {
        ::SetWindowText((*m_labels)[SliderLabel_Value], Format(value).t_str());
    }
}

void wxSlider::SetRange(int minValue, int maxValue)
{
                const int valueOld = HasFlag(wxSL_INVERSE) ? GetValue() : 0;

    m_rangeMin = minValue;
    m_rangeMax = maxValue;

    ::SendMessage(GetHwnd(), TBM_SETRANGEMIN, TRUE, m_rangeMin);
    ::SendMessage(GetHwnd(), TBM_SETRANGEMAX, TRUE, m_rangeMax);

    if ( m_labels )
    {
        Move(wxDefaultPosition); 
                        if ( HasFlag(wxSL_VALUE_LABEL) )
        {
            SetValue(GetValue());
        }
        ::SetWindowText((*m_labels)[SliderLabel_Min],
                        Format(ValueInvertOrNot(m_rangeMin)).t_str());
        ::SetWindowText((*m_labels)[SliderLabel_Max],
                        Format(ValueInvertOrNot(m_rangeMax)).t_str());
    }

                if ( HasFlag(wxSL_INVERSE) )
    {
        ::SendMessage(GetHwnd(), TBM_SETPOS, TRUE, ValueInvertOrNot(valueOld));
    }
}

void wxSlider::DoSetTickFreq(int n)
{
    m_tickFreq = n;
    ::SendMessage( GetHwnd(), TBM_SETTICFREQ, (WPARAM) n, (LPARAM) 0 );
}

void wxSlider::SetPageSize(int pageSize)
{
    ::SendMessage( GetHwnd(), TBM_SETPAGESIZE, (WPARAM) 0, (LPARAM) pageSize );
    m_pageSize = pageSize;
}

int wxSlider::GetPageSize() const
{
    return m_pageSize;
}

void wxSlider::ClearSel()
{
    ::SendMessage(GetHwnd(), TBM_CLEARSEL, (WPARAM) TRUE, (LPARAM) 0);
}

void wxSlider::ClearTicks()
{
    ::SendMessage(GetHwnd(), TBM_CLEARTICS, (WPARAM) TRUE, (LPARAM) 0);
}

void wxSlider::SetLineSize(int lineSize)
{
    m_lineSize = lineSize;
    ::SendMessage(GetHwnd(), TBM_SETLINESIZE, (WPARAM) 0, (LPARAM) lineSize);
}

int wxSlider::GetLineSize() const
{
    return (int)::SendMessage(GetHwnd(), TBM_GETLINESIZE, 0, 0);
}

int wxSlider::GetSelEnd() const
{
    return (int)::SendMessage(GetHwnd(), TBM_GETSELEND, 0, 0);
}

int wxSlider::GetSelStart() const
{
    return (int)::SendMessage(GetHwnd(), TBM_GETSELSTART, 0, 0);
}

void wxSlider::SetSelection(int minPos, int maxPos)
{
    ::SendMessage(GetHwnd(), TBM_SETSEL,
                  (WPARAM) TRUE ,
                  (LPARAM) MAKELONG( minPos, maxPos) );
}

void wxSlider::SetThumbLength(int len)
{
    ::SendMessage(GetHwnd(), TBM_SETTHUMBLENGTH, (WPARAM) len, (LPARAM) 0);
}

int wxSlider::GetThumbLength() const
{
    return (int)::SendMessage( GetHwnd(), TBM_GETTHUMBLENGTH, 0, 0);
}

void wxSlider::SetTick(int tickPos)
{
    ::SendMessage( GetHwnd(), TBM_SETTIC, (WPARAM) 0, (LPARAM) tickPos );
}


WXHWND wxSlider::GetStaticMin() const
{
    return m_labels ? (WXHWND)(*m_labels)[SliderLabel_Min] : NULL;
}

WXHWND wxSlider::GetStaticMax() const
{
    return m_labels ? (WXHWND)(*m_labels)[SliderLabel_Max] : NULL;
}

WXHWND wxSlider::GetEditValue() const
{
    return m_labels ? (WXHWND)(*m_labels)[SliderLabel_Value] : NULL;
}

WX_FORWARD_STD_METHODS_TO_SUBWINDOWS(wxSlider, wxSliderBase, m_labels)

#endif 