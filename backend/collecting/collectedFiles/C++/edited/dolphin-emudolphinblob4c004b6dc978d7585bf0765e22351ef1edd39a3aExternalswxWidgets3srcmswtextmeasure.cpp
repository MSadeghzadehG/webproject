


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/msw/private.h"

#ifndef WX_PRECOMP
    #include "wx/window.h"
    #include "wx/font.h"
#endif 
#include "wx/private/textmeasure.h"

#include "wx/msw/dc.h"


void wxTextMeasure::Init()
{
    m_hdc = NULL;
    m_hfontOld = NULL;

    if ( m_dc )
    {
        wxClassInfo* const ci = m_dc->GetImpl()->GetClassInfo();

        if ( ci->IsKindOf(wxCLASSINFO(wxMSWDCImpl)))
        {
            m_useDCImpl = false;
        }
    }
}

void wxTextMeasure::BeginMeasuring()
{
    if ( m_dc )
    {
        m_hdc = m_dc->GetHDC();

                        wxASSERT_MSG( m_hdc, wxS("Must not be used with non-native wxDCs") );
    }
    else if ( m_win )
    {
        m_hdc = ::GetDC(GetHwndOf(m_win));
    }

                    if ( m_font || m_win )
        m_hfontOld = (HFONT)::SelectObject(m_hdc, GetHfontOf(GetFont()));
}

void wxTextMeasure::EndMeasuring()
{
    if ( m_hfontOld )
    {
        ::SelectObject(m_hdc, m_hfontOld);
        m_hfontOld = NULL;
    }

    if ( m_win )
        ::ReleaseDC(GetHwndOf(m_win), m_hdc);
    
    m_hdc = NULL;
}

void wxTextMeasure::DoGetTextExtent(const wxString& string,
                                       wxCoord *width,
                                       wxCoord *height,
                                       wxCoord *descent,
                                       wxCoord *externalLeading)
{
    SIZE sizeRect;
    const size_t len = string.length();
    if ( !::GetTextExtentPoint32(m_hdc, string.t_str(), len, &sizeRect) )
    {
        wxLogLastError(wxT("GetTextExtentPoint32()"));
    }

                if ( len > 0 )
    {
        ABC widthABC;
        const wxChar chFirst = *string.begin();
        if ( ::GetCharABCWidths(m_hdc, chFirst, chFirst, &widthABC) )
        {
            if ( widthABC.abcA < 0 )
                sizeRect.cx -= widthABC.abcA;

            if ( len > 1 )
            {
                const wxChar chLast = *string.rbegin();
                ::GetCharABCWidths(m_hdc, chLast, chLast, &widthABC);
            }
            
            if ( widthABC.abcC < 0 )
                sizeRect.cx -= widthABC.abcC;
        }
            }

    *width = sizeRect.cx;
    *height = sizeRect.cy;

    if ( descent || externalLeading )
    {
        TEXTMETRIC tm;
        ::GetTextMetrics(m_hdc, &tm);
        if ( descent )
            *descent = tm.tmDescent;
        if ( externalLeading )
            *externalLeading = tm.tmExternalLeading;
    }
}

bool wxTextMeasure::DoGetPartialTextExtents(const wxString& text,
                                            wxArrayInt& widths,
                                            double scaleX)
{
    if ( !m_hdc )
        return wxTextMeasureBase::DoGetPartialTextExtents(text, widths, scaleX);

    int fit = 0;
    SIZE sz = {0,0};
    if ( !::GetTextExtentExPoint(m_hdc,
                                 text.t_str(),                                  text.length(),
                                 INT_MAX,                                       &fit,                                                                                         &widths[0],                                    &sz) )
    {
        wxLogLastError(wxT("GetTextExtentExPoint"));

        return false;
    }

    return true;
}
