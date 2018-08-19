


#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/window.h"
    #include "wx/log.h"
#endif 
#include "wx/private/textmeasure.h"

#include "wx/fontutil.h"
#include "wx/gtk/private.h"
#include "wx/gtk/dc.h"

#ifndef __WXGTK3__
    #include "wx/gtk/dcclient.h"
#endif


void wxTextMeasure::Init()
{
    m_context = NULL;
    m_layout = NULL;

#ifndef __WXGTK3__
    m_wdc = NULL;

    if ( m_dc )
    {
        wxClassInfo* const ci = m_dc->GetImpl()->GetClassInfo();

                        if ( ci->IsKindOf(wxCLASSINFO(wxWindowDCImpl)))
        {
            m_useDCImpl = false;
        }
    }
#endif }

void wxTextMeasure::BeginMeasuring()
{
    if ( m_dc )
    {
#ifndef __WXGTK3__
        m_wdc = wxDynamicCast(m_dc->GetImpl(), wxWindowDCImpl);
        if ( m_wdc )
        {
            m_context = m_wdc->m_context;
            m_layout = m_wdc->m_layout;
        }
#endif     }
    else if ( m_win )
    {
        m_context = gtk_widget_get_pango_context( m_win->GetHandle() );
        if ( m_context )
            m_layout = pango_layout_new(m_context);
    }

        if ( m_layout )
    {
        pango_layout_set_font_description(m_layout,
                                          GetFont().GetNativeFontInfo()->description);
    }
}

void wxTextMeasure::EndMeasuring()
{
    if ( !m_layout )
        return;

#ifndef __WXGTK3__
    if ( m_wdc )
    {
                pango_layout_set_font_description( m_wdc->m_layout, m_wdc->m_fontdesc );
    }
    else
#endif     {
        g_object_unref (m_layout);
    }
}

void wxTextMeasure::DoGetTextExtent(const wxString& string,
                                    wxCoord *width,
                                    wxCoord *height,
                                    wxCoord *descent,
                                    wxCoord *externalLeading)
{
    if ( !m_context )
    {
        if ( width )
            *width = 0;

        if ( height )
            *height = 0;
        return;
    }

        const wxCharBuffer dataUTF8 = wxGTK_CONV_FONT(string, GetFont());
    if ( !dataUTF8 && !string.empty() )
    {
                wxLogLastError(wxT("GetTextExtent"));
        return;
    }
    pango_layout_set_text(m_layout, dataUTF8, -1);

    if ( m_dc )
    {
                pango_layout_get_pixel_size(m_layout, width, height);
    }
    else     {
                PangoRectangle rect;
        pango_layout_get_extents(m_layout, NULL, &rect);
        *width = PANGO_PIXELS(rect.width);
        *height = PANGO_PIXELS(rect.height);
    }

    if (descent)
    {
        PangoLayoutIter *iter = pango_layout_get_iter(m_layout);
        int baseline = pango_layout_iter_get_baseline(iter);
        pango_layout_iter_free(iter);
        *descent = *height - PANGO_PIXELS(baseline);
    }

    if (externalLeading)
    {
                *externalLeading = 0;
    }
}

bool wxTextMeasure::DoGetPartialTextExtents(const wxString& text,
                                            wxArrayInt& widths,
                                            double scaleX)
{
    if ( !m_layout )
        return wxTextMeasureBase::DoGetPartialTextExtents(text, widths, scaleX);

        const wxCharBuffer dataUTF8 = wxGTK_CONV_FONT(text, GetFont());
    if ( !dataUTF8 )
    {
                wxLogLastError(wxT("GetPartialTextExtents"));
        return false;
    }

    pango_layout_set_text(m_layout, dataUTF8, -1);

        
        PangoLayoutIter *iter = pango_layout_get_iter(m_layout);
    PangoRectangle pos;
    pango_layout_iter_get_cluster_extents(iter, NULL, &pos);
    size_t i = 0;
    while (pango_layout_iter_next_cluster(iter))
    {
        pango_layout_iter_get_cluster_extents(iter, NULL, &pos);
        int position = PANGO_PIXELS(pos.x);
        widths[i++] = position;
    }

    const size_t len = text.length();
    while (i < len)
        widths[i++] = PANGO_PIXELS(pos.x + pos.width);
    pango_layout_iter_free(iter);

    return true;
}
