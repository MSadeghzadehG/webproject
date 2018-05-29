


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/window.h"
    #include "wx/dc.h"
#endif 
#include "wx/private/textmeasure.h"

#if wxUSE_GENERIC_TEXTMEASURE


void wxTextMeasure::DoGetTextExtent(const wxString& string,
                                    wxCoord *width,
                                    wxCoord *height,
                                    wxCoord *descent,
                                    wxCoord *externalLeading)
{
    if ( m_dc )
    {
        m_dc->GetTextExtent(string, width, height,
                            descent, externalLeading, m_font);
    }
    else if ( m_win )
    {
        m_win->GetTextExtent(string, width, height,
                             descent, externalLeading, m_font);
    }
    }

bool wxTextMeasure::DoGetPartialTextExtents(const wxString& text,
                                            wxArrayInt& widths,
                                            double scaleX)
{
    return wxTextMeasureBase::DoGetPartialTextExtents(text, widths, scaleX);
}

#endif 