


#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/dc.h"
    #include "wx/window.h"
#endif 
#include "wx/private/textmeasure.h"


wxTextMeasureBase::wxTextMeasureBase(const wxDC *dc, const wxFont *theFont)
    : m_dc(dc),
      m_win(NULL),
      m_font(theFont)
{
    wxASSERT_MSG( dc, wxS("wxTextMeasure needs a valid wxDC") );

            m_useDCImpl = true;
}

wxTextMeasureBase::wxTextMeasureBase(const wxWindow *win, const wxFont *theFont)
    : m_dc(NULL),
      m_win(win),
      m_font(theFont)
{
    wxASSERT_MSG( win, wxS("wxTextMeasure needs a valid wxWindow") );

        m_useDCImpl = false;
}

wxFont wxTextMeasureBase::GetFont() const
{
    return m_font ? *m_font
                  : m_win ? m_win->GetFont()
                          : m_dc->GetFont();
}

void wxTextMeasureBase::CallGetTextExtent(const wxString& string,
                                          wxCoord *width,
                                          wxCoord *height,
                                          wxCoord *descent,
                                          wxCoord *externalLeading)
{
    if ( m_useDCImpl )
        m_dc->GetTextExtent(string, width, height, descent, externalLeading);
    else
        DoGetTextExtent(string, width, height, descent, externalLeading);
}

void wxTextMeasureBase::GetTextExtent(const wxString& string,
                                      wxCoord *width,
                                      wxCoord *height,
                                      wxCoord *descent,
                                      wxCoord *externalLeading)
{
            int unusedWidth, unusedHeight;
    if ( !width )
        width = &unusedWidth;
    if ( !height )
        height = &unusedHeight;

            if ( string.empty() && !descent && !externalLeading )
    {
        *width =
        *height = 0;

        return;
    }

    MeasuringGuard guard(*this);

    CallGetTextExtent(string, width, height, descent, externalLeading);
}

void wxTextMeasureBase::GetMultiLineTextExtent(const wxString& text,
                                               wxCoord *width,
                                               wxCoord *height,
                                               wxCoord *heightOneLine)
{
    MeasuringGuard guard(*this);

    wxCoord widthTextMax = 0, widthLine,
            heightTextTotal = 0, heightLineDefault = 0, heightLine = 0;

    wxString curLine;
    for ( wxString::const_iterator pc = text.begin(); ; ++pc )
    {
        if ( pc == text.end() || *pc == wxS('\n') )
        {
            if ( curLine.empty() )
            {
                                                
                                                if ( !heightLineDefault )
                    heightLineDefault = heightLine;

                if ( !heightLineDefault )
                {
                                        int dummy;
                    CallGetTextExtent(wxS("W"), &dummy, &heightLineDefault);
                }

                heightTextTotal += heightLineDefault;
            }
            else
            {
                CallGetTextExtent(curLine, &widthLine, &heightLine);
                if ( widthLine > widthTextMax )
                    widthTextMax = widthLine;
                heightTextTotal += heightLine;
            }

            if ( pc == text.end() )
            {
               break;
            }
            else             {
               curLine.clear();
            }
        }
        else
        {
            curLine += *pc;
        }
    }

    if ( width )
        *width = widthTextMax;
    if ( height )
        *height = heightTextTotal;
    if ( heightOneLine )
        *heightOneLine = heightLine;
}

wxSize wxTextMeasureBase::GetLargestStringExtent(size_t n,
                                                 const wxString* strings)
{
    MeasuringGuard guard(*this);

    wxCoord w, h, widthMax = 0, heightMax = 0;
    for ( size_t i = 0; i < n; ++i )
    {
        CallGetTextExtent(strings[i], &w, &h);

        if ( w > widthMax )
            widthMax = w;
        if ( h > heightMax )
            heightMax = h;
    }

    return wxSize(widthMax, heightMax);
}

bool wxTextMeasureBase::GetPartialTextExtents(const wxString& text,
                                              wxArrayInt& widths,
                                              double scaleX)
{
    widths.Empty();
    if ( text.empty() )
        return true;

    MeasuringGuard guard(*this);

    widths.Add(0, text.length());

    return DoGetPartialTextExtents(text, widths, scaleX);
}



#define FWC_SIZE 256

class FontWidthCache
{
public:
    FontWidthCache() : m_scaleX(1), m_widths(NULL) { }
    ~FontWidthCache() { delete []m_widths; }

    void Reset()
    {
        if ( !m_widths )
            m_widths = new int[FWC_SIZE];

        memset(m_widths, 0, sizeof(int)*FWC_SIZE);
    }

    wxFont m_font;
    double m_scaleX;
    int *m_widths;
};

static FontWidthCache s_fontWidthCache;

bool wxTextMeasureBase::DoGetPartialTextExtents(const wxString& text,
                                                wxArrayInt& widths,
                                                double scaleX)
{
    int totalWidth = 0;

        if ( !s_fontWidthCache.m_widths ||
         !wxIsSameDouble(s_fontWidthCache.m_scaleX, scaleX) ||
         (s_fontWidthCache.m_font != *m_font) )
    {
        s_fontWidthCache.Reset();
        s_fontWidthCache.m_font = *m_font;
        s_fontWidthCache.m_scaleX = scaleX;
    }

            int n = 0;
    for ( wxString::const_iterator it = text.begin();
          it != text.end();
          ++it )
    {
        const wxChar c = *it;
        unsigned int c_int = (unsigned int)c;

        int w;
        if ((c_int < FWC_SIZE) && (s_fontWidthCache.m_widths[c_int] != 0))
        {
            w = s_fontWidthCache.m_widths[c_int];
        }
        else
        {
            DoGetTextExtent(c, &w, NULL);
            if (c_int < FWC_SIZE)
                s_fontWidthCache.m_widths[c_int] = w;
        }

        totalWidth += w;
        widths[n++] = totalWidth;
    }

    return true;
}

