


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_MARKUP

#ifndef WX_PRECOMP
    #include "wx/gdicmn.h"
    #include "wx/control.h"
    #include "wx/dc.h"
#endif 
#include "wx/generic/private/markuptext.h"

#include "wx/private/markupparserattr.h"

namespace
{


class wxMarkupParserMeasureOutput : public wxMarkupParserAttrOutput
{
public:
                wxMarkupParserMeasureOutput(wxDC& dc, int *visibleHeight)
        : wxMarkupParserAttrOutput(dc.GetFont(), wxColour(), wxColour()),
          m_dc(dc),
          m_visibleHeight(visibleHeight)
    {
        if ( visibleHeight )
            *visibleHeight = 0;
    }

    const wxSize& GetSize() const { return m_size; }


    virtual void OnText(const wxString& text_) wxOVERRIDE
    {
        const wxString text(wxControl::RemoveMnemonics(text_));

                const wxSize size = m_dc.GetTextExtent(text);

        m_size.x += size.x;
        if ( size.y > m_size.y )
            m_size.y = size.y;

        if ( m_visibleHeight )
        {
            wxFontMetrics tm = m_dc.GetFontMetrics();
            int visibleHeight = tm.ascent - tm.internalLeading;
            if ( *m_visibleHeight < visibleHeight )
                *m_visibleHeight = visibleHeight;
        }
    }

    virtual void OnAttrStart(const Attr& attr) wxOVERRIDE
    {
        m_dc.SetFont(attr.font);
    }

    virtual void OnAttrEnd(const Attr& WXUNUSED(attr)) wxOVERRIDE
    {
        m_dc.SetFont(GetFont());
    }

private:
    wxDC& m_dc;

        wxSize m_size;
    int * const m_visibleHeight;    
    wxDECLARE_NO_COPY_CLASS(wxMarkupParserMeasureOutput);
};


class wxMarkupParserRenderOutput : public wxMarkupParserAttrOutput
{
public:
                wxMarkupParserRenderOutput(wxDC& dc,
                               const wxRect& rect,
                               int flags)
        : wxMarkupParserAttrOutput(dc.GetFont(),
                                   dc.GetTextForeground(),
                                   wxColour()),
          m_dc(dc),
          m_rect(rect),
          m_flags(flags)
    {
        m_pos = m_rect.x;

                                                                        m_origTextBackground = dc.GetTextBackground();
    }

    virtual void OnText(const wxString& text_) wxOVERRIDE
    {
        wxString text;
        int indexAccel = wxControl::FindAccelIndex(text_, &text);
        if ( !(m_flags & wxMarkupText::Render_ShowAccels) )
            indexAccel = wxNOT_FOUND;

                                        wxRect rect(m_rect);
        rect.x = m_pos;

        int descent;
        m_dc.GetTextExtent(text, &rect.width, &rect.height, &descent);
        rect.height -= descent;
        rect.y += m_rect.height - rect.height;

        wxRect bounds;
        m_dc.DrawLabel(text, wxBitmap(),
                       rect, wxALIGN_LEFT | wxALIGN_TOP,
                       indexAccel,
                       &bounds);

                m_pos += bounds.width;
    }

    virtual void OnAttrStart(const Attr& attr) wxOVERRIDE
    {
        m_dc.SetFont(attr.font);
        if ( attr.foreground.IsOk() )
            m_dc.SetTextForeground(attr.foreground);

        if ( attr.background.IsOk() )
        {
                                    m_dc.SetBackgroundMode(wxSOLID);
            m_dc.SetTextBackground(attr.background);
        }
    }

    virtual void OnAttrEnd(const Attr& attr) wxOVERRIDE
    {
                m_dc.SetFont(GetFont());

                if ( attr.foreground.IsOk() )
            m_dc.SetTextForeground(GetAttr().foreground);

        if ( attr.background.IsOk() )
        {
            wxColour background = GetAttr().background;
            if ( !background.IsOk() )
            {
                                                                                m_dc.SetBackgroundMode(wxTRANSPARENT);
                background = m_origTextBackground;
            }

            m_dc.SetTextBackground(background);
        }
    }

private:
    wxDC& m_dc;
    const wxRect m_rect;
    const int m_flags;

    wxColour m_origTextBackground;

                int m_pos;

    wxDECLARE_NO_COPY_CLASS(wxMarkupParserRenderOutput);
};

} 

wxSize wxMarkupText::Measure(wxDC& dc, int *visibleHeight) const
{
    wxMarkupParserMeasureOutput out(dc, visibleHeight);
    wxMarkupParser parser(out);
    if ( !parser.Parse(m_markup) )
    {
        wxFAIL_MSG( "Invalid markup" );
        return wxDefaultSize;
    }

    return out.GetSize();
}

void wxMarkupText::Render(wxDC& dc, const wxRect& rect, int flags)
{
                int visibleHeight;
    wxRect rectText(rect.GetPosition(), Measure(dc, &visibleHeight));
    rectText.height = visibleHeight;

    wxMarkupParserRenderOutput out(dc, rectText.CentreIn(rect), flags);
    wxMarkupParser parser(out);
    parser.Parse(m_markup);
}

#endif 