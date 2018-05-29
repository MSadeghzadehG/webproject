

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#if wxUSE_SVG

#ifndef WX_PRECOMP
    #include "wx/dcmemory.h"
    #include "wx/dcscreen.h"
    #include "wx/icon.h"
    #include "wx/image.h"
    #include "wx/math.h"
#endif

#include "wx/base64.h"
#include "wx/dcsvg.h"
#include "wx/wfstream.h"
#include "wx/filename.h"
#include "wx/mstream.h"

#include "wx/private/markupparser.h"


namespace
{

inline wxString NumStr(double f)
{
    return wxString::FromCDouble(f, 2);
}

wxString Col2SVG(wxColour c, float *opacity)
{
    if ( c.Alpha() != wxALPHA_OPAQUE )
    {
        *opacity = c.Alpha() / 255.0f;

                        c = wxColour(c.GetRGB());
    }
    else     {
        *opacity = 1.;
    }

    return c.GetAsString(wxC2S_HTML_SYNTAX);
}

wxString wxPenString(wxColour c, int style = wxPENSTYLE_SOLID)
{
    float opacity;
    wxString s = wxT("stroke:") + Col2SVG(c, &opacity)  + wxT("; ");

    switch ( style )
    {
        case wxPENSTYLE_SOLID:
            s += wxString::Format(wxT("stroke-opacity:%s; "), NumStr(opacity));
            break;
        case wxPENSTYLE_TRANSPARENT:
            s += wxT("stroke-opacity:0.0; ");
            break;
        default :
            wxASSERT_MSG(false, wxT("wxSVGFileDC::Requested Pen Style not available"));
    }

    return s;
}

wxString wxBrushString(wxColour c, int style = wxBRUSHSTYLE_SOLID)
{
    float opacity;
    wxString s = wxT("fill:") + Col2SVG(c, &opacity)  + wxT("; ");

    switch ( style )
    {
        case wxBRUSHSTYLE_SOLID:
            s += wxString::Format(wxT("fill-opacity:%s; "), NumStr(opacity));
            break;
        case wxBRUSHSTYLE_TRANSPARENT:
            s += wxT("fill-opacity:0.0; ");
            break;
        default :
            wxASSERT_MSG(false, wxT("wxSVGFileDC::Requested Brush Style not available"));
    }

    return s;
}

} 

bool
wxSVGBitmapEmbedHandler::ProcessBitmap(const wxBitmap& bmp,
                                       wxCoord x, wxCoord y,
                                       wxOutputStream& stream) const
{
    static int sub_images = 0;

    if ( wxImage::FindHandler(wxBITMAP_TYPE_PNG) == NULL )
        wxImage::AddHandler(new wxPNGHandler);

        wxMemoryOutputStream mem;
    bmp.ConvertToImage().SaveFile(mem, wxBITMAP_TYPE_PNG);
    wxString data = wxBase64Encode(mem.GetOutputStreamBuffer()->GetBufferStart(),
                                   mem.GetSize());

        wxString s;
    s += wxString::Format(" <image x=\"%d\" y=\"%d\" "
                          "width=\"%dpx\" height=\"%dpx\" "
                          "title=\"Image from wxSVG\"\n",
                          x, y, bmp.GetWidth(), bmp.GetHeight());
    s += wxString::Format(" id=\"image%d\" "
                          "xlink:href=\"data:image/png;base64,\n",
                          sub_images++);

        const unsigned WRAP = 76;
    for ( size_t i = 0; i < data.size(); i += WRAP )
    {
        if (i < data.size() - WRAP)
            s += data.Mid(i, WRAP) + "\n";
        else
            s += data.Mid(i, s.size() - i) + "\"\n/>";     }

        const wxCharBuffer buf = s.utf8_str();
    stream.Write(buf, strlen((const char *)buf));

    return stream.IsOk();
}


bool
wxSVGBitmapFileHandler::ProcessBitmap(const wxBitmap& bmp,
                                      wxCoord x, wxCoord y,
                                      wxOutputStream& stream) const
{
    static int sub_images = 0;

    if ( wxImage::FindHandler(wxBITMAP_TYPE_PNG) == NULL )
        wxImage::AddHandler(new wxPNGHandler);

        wxString sPNG;
    do
    {
        sPNG = wxString::Format("image%d.png", sub_images++);
    }
    while (wxFile::Exists(sPNG));

    if ( !bmp.SaveFile(sPNG, wxBITMAP_TYPE_PNG) )
        return false;

        sPNG = sPNG.AfterLast(wxFileName::GetPathSeparator());

        wxString s;
    s += wxString::Format(" <image x=\"%d\" y=\"%d\" "
                          "width=\"%dpx\" height=\"%dpx\" "
                          "title=\"Image from wxSVG\"\n",
                          x, y, bmp.GetWidth(), bmp.GetHeight());
    s += wxString::Format(" xlink:href=\"%s\">\n</image>\n", sPNG);

        const wxCharBuffer buf = s.utf8_str();
    stream.Write(buf, strlen((const char *)buf));

    return stream.IsOk();
}


void wxSVGFileDC::SetBitmapHandler(wxSVGBitmapHandler* handler)
{
    ((wxSVGFileDCImpl*)GetImpl())->SetBitmapHandler(handler);
}


wxIMPLEMENT_ABSTRACT_CLASS(wxSVGFileDCImpl, wxDC);

wxSVGFileDCImpl::wxSVGFileDCImpl( wxSVGFileDC *owner, const wxString &filename,
                    int width, int height, double dpi ) :
        wxDCImpl( owner )
    {
        Init( filename, width, height, dpi );
    }

void wxSVGFileDCImpl::Init (const wxString &filename, int Width, int Height, double dpi)
{
    m_width = Width;
    m_height = Height;

    m_dpi = dpi;

    m_OK = true;

    m_clipUniqueId = 0;
    m_clipNestingLevel = 0;

    m_mm_to_pix_x = dpi/25.4;
    m_mm_to_pix_y = dpi/25.4;

    m_backgroundBrush = *wxTRANSPARENT_BRUSH;
    m_textForegroundColour = *wxBLACK;
    m_textBackgroundColour = *wxWHITE;
    m_colour = wxColourDisplay();

    m_pen   = *wxBLACK_PEN;
    m_font  = *wxNORMAL_FONT;
    m_brush = *wxWHITE_BRUSH;

    m_graphics_changed = true;

    
    m_bmp_handler = NULL;
    m_outfile = new wxFileOutputStream(filename);
    m_OK = m_outfile->IsOk();
    if (m_OK)
    {
        m_filename = filename;
        m_sub_images = 0;
        wxString s;
        s = wxT("<?xml version=\"1.0\" standalone=\"no\"?>\n");
        write(s);
        s = wxT("<!DOCTYPE svg PUBLIC \"-        write(s);
        s = wxT("\"http:        write(s);
        s = wxT("<svg xmlns=\"http:        write(s);
        s.Printf( wxT("    width=\"%scm\" height=\"%scm\" viewBox=\"0 0 %d %d \">\n"), NumStr(float(Width)/dpi*2.54), NumStr(float(Height)/dpi*2.54), Width, Height );
        write(s);
        s = wxT("<title>SVG Picture created as ") + wxFileName(filename).GetFullName() + wxT(" </title>\n");
        write(s);
        s = wxString (wxT("<desc>Picture generated by wxSVG ")) + wxSVGVersion + wxT(" </desc>\n");
        write(s);
        s =  wxT("<g style=\"fill:black; stroke:black; stroke-width:1\">\n");
        write(s);
    }
}

wxSVGFileDCImpl::~wxSVGFileDCImpl()
{
    wxString s = wxT("</g> \n</svg> \n");
    write(s);
    delete m_outfile;
}

void wxSVGFileDCImpl::DoGetSizeMM( int *width, int *height ) const
{
    if (width)
        *width = wxRound( (double)m_width / m_mm_to_pix_x );

    if (height)
        *height = wxRound( (double)m_height / m_mm_to_pix_y );
}

wxSize wxSVGFileDCImpl::GetPPI() const
{
    return wxSize( wxRound(m_dpi), wxRound(m_dpi) );
}

void wxSVGFileDCImpl::DoDrawLine (wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2)
{
    NewGraphicsIfNeeded();
    wxString s;
    s.Printf ( wxT("<path d=\"M%d %d L%d %d\" /> \n"), x1,y1,x2,y2 );
    if (m_OK)
    {
        write(s);
    }
    CalcBoundingBox(x1, y1);
    CalcBoundingBox(x2, y2);
}

void wxSVGFileDCImpl::DoDrawLines(int n, const wxPoint points[], wxCoord xoffset , wxCoord yoffset )
{
    for ( int i = 1; i < n; i++ )
    {
        DoDrawLine ( points [i-1].x + xoffset, points [i-1].y + yoffset,
            points [ i ].x + xoffset, points [ i ].y + yoffset );
    }
}

void wxSVGFileDCImpl::DoDrawPoint (wxCoord x1, wxCoord y1)
{
    wxString s;
    NewGraphicsIfNeeded();
    s = wxT("<g style = \"stroke-linecap:round;\" > \n");
    write(s);
    DoDrawLine ( x1,y1,x1,y1 );
    s = wxT("</g>");
    write(s);
}

void wxSVGFileDCImpl::DoDrawCheckMark(wxCoord x1, wxCoord y1, wxCoord width, wxCoord height)
{
    wxDCImpl::DoDrawCheckMark (x1,y1,width,height);
}

void wxSVGFileDCImpl::DoDrawText(const wxString& text, wxCoord x1, wxCoord y1)
{
    DoDrawRotatedText(text, x1,y1,0.0);
}

void wxSVGFileDCImpl::DoDrawRotatedText(const wxString& sText, wxCoord x, wxCoord y, double angle)
{
        NewGraphicsIfNeeded();
    wxString s, sTmp;

        wxCoord w, h, desc;
    DoGetTextExtent(sText, &w, &h, &desc);

    double rad = wxDegToRad(angle);

        CalcBoundingBox(x, y);
    CalcBoundingBox((wxCoord)(x + w*cos(rad)), (wxCoord)(y - h*sin(rad)));

        CalcBoundingBox((wxCoord)(x + h*sin(rad)), (wxCoord)(y + h*cos(rad)));
    CalcBoundingBox((wxCoord)(x + h*sin(rad) + w*cos(rad)), (wxCoord)(y + h*cos(rad) - w*sin(rad)));

    if (m_backgroundMode == wxBRUSHSTYLE_SOLID)
    {
                
        sTmp.Printf ( wxT(" <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" "), x, y, w, h );
        s = sTmp + wxT("style=\"") + wxBrushString(m_textBackgroundColour);
        s += wxT("stroke-width:1; ") + wxPenString(m_textBackgroundColour);
        sTmp.Printf ( wxT("\" transform=\"rotate( %s %d %d )  \" />"), NumStr(-angle), x,y );
        s += sTmp + wxT("\n");
        write(s);
    }

        x = (wxCoord)(x + (h-desc)*sin(rad));
    y = (wxCoord)(y + (h-desc)*cos(rad));

        s.Printf (wxT(" <text x=\"%d\" y=\"%d\" "),x,y );

    sTmp = m_font.GetFaceName();
    if (sTmp.Len() > 0)  s += wxT("style=\"font-family:") + sTmp + wxT("; ");
    else s += wxT("style=\" ");

    wxString fontweight;
    switch ( m_font.GetWeight() )
    {
        case wxFONTWEIGHT_MAX:
            wxFAIL_MSG( wxS("invalid font weight value") );
            wxFALLTHROUGH;

        case wxFONTWEIGHT_NORMAL:
            fontweight = wxS("normal");
            break;

        case wxFONTWEIGHT_LIGHT:
            fontweight = wxS("lighter");
            break;

        case wxFONTWEIGHT_BOLD:
            fontweight = wxS("bold");
            break;
    }

    wxASSERT_MSG( !fontweight.empty(), wxS("unknown font weight value") );

    s += wxT("font-weight:") + fontweight + wxT("; ");

    wxString fontstyle;
    switch ( m_font.GetStyle() )
    {
        case wxFONTSTYLE_MAX:
            wxFAIL_MSG( wxS("invalid font style value") );
            wxFALLTHROUGH;

        case wxFONTSTYLE_NORMAL:
            fontstyle = wxS("normal");
            break;

        case wxFONTSTYLE_ITALIC:
            fontstyle = wxS("italic");
            break;

        case wxFONTSTYLE_SLANT:
            fontstyle = wxS("oblique");
            break;
    }

    wxASSERT_MSG( !fontstyle.empty(), wxS("unknown font style value") );

    s += wxT("font-style:") + fontstyle + wxT("; ");

    sTmp.Printf (wxT("font-size:%dpt; "), m_font.GetPointSize() );
    s += sTmp;
        s += wxBrushString(m_textForegroundColour) + wxPenString(m_textForegroundColour);
    sTmp.Printf ( wxT("stroke-width:0;\"  transform=\"rotate( %s %d %d )  \" >"),  NumStr(-angle), x,y );
    s += sTmp + wxMarkupParser::Quote(sText) + wxT("</text> ") + wxT("\n");
    if (m_OK)
    {
        write(s);
    }
}

void wxSVGFileDCImpl::DoDrawRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height)
{
    DoDrawRoundedRectangle(x, y, width, height, 0);
}

void wxSVGFileDCImpl::DoDrawRoundedRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height, double radius )

{
    NewGraphicsIfNeeded();
    wxString s;

    s.Printf ( wxT(" <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" rx=\"%s\" "),
            x, y, width, height, NumStr(radius) );

    s += wxT(" /> \n");
    write(s);

    CalcBoundingBox(x, y);
    CalcBoundingBox(x + width, y + height);
}

void wxSVGFileDCImpl::DoDrawPolygon(int n, const wxPoint points[],
                                    wxCoord xoffset, wxCoord yoffset,
                                    wxPolygonFillMode fillStyle)
{
    NewGraphicsIfNeeded();
    wxString s, sTmp;
    s = wxT("<polygon style=\"");
    if ( fillStyle == wxODDEVEN_RULE )
        s += wxT("fill-rule:evenodd; ");
    else
        s += wxT("fill-rule:nonzero; ");

    s += wxT("\" \npoints=\"");

    for (int i = 0; i < n;  i++)
    {
        sTmp.Printf ( wxT("%d,%d"), points [i].x+xoffset, points[i].y+yoffset );
        s += sTmp + wxT("\n");
        CalcBoundingBox ( points [i].x+xoffset, points[i].y+yoffset);
    }
    s += wxT("\" /> \n");
    write(s);
}

void wxSVGFileDCImpl::DoDrawEllipse (wxCoord x, wxCoord y, wxCoord width, wxCoord height)

{
    NewGraphicsIfNeeded();

    int rh = height /2;
    int rw = width  /2;

    wxString s;
    s.Printf ( wxT("<ellipse cx=\"%d\" cy=\"%d\" rx=\"%d\" ry=\"%d\" "), x+rw,y+rh, rw, rh );
    s += wxT(" /> \n");

    write(s);

    CalcBoundingBox(x, y);
    CalcBoundingBox(x + width, y + height);
}

void wxSVGFileDCImpl::DoDrawArc(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2, wxCoord xc, wxCoord yc)
{
    

    NewGraphicsIfNeeded();
    wxString s;

        double r1 = sqrt ( double( (x1-xc)*(x1-xc) ) + double( (y1-yc)*(y1-yc) ) );
    double r2 = sqrt ( double( (x2-xc)*(x2-xc) ) + double( (y2-yc)*(y2-yc) ) );

    wxASSERT_MSG( (fabs ( r2-r1 ) <= 3), wxT("wxSVGFileDC::DoDrawArc Error in getting radii of circle"));
    if ( fabs ( r2-r1 ) > 3 )        {
        s = wxT("<!--- wxSVGFileDC::DoDrawArc Error in getting radii of circle --> \n");
        write(s);
    }

    double theta1 = atan2((double)(yc-y1),(double)(x1-xc));
    if ( theta1 < 0 ) theta1 = theta1 + M_PI * 2;
    double theta2 = atan2((double)(yc-y2), (double)(x2-xc));
    if ( theta2 < 0 ) theta2 = theta2 + M_PI * 2;
    if ( theta2 < theta1 ) theta2 = theta2 + M_PI *2;

    int fArc;                      if ( fabs(theta2 - theta1) > M_PI ) fArc = 1; else fArc = 0;

    int fSweep = 0;             
    s.Printf ( wxT("<path d=\"M%d %d A%s %s 0.0 %d %d %d %d L%d %d z "),
        x1,y1, NumStr(r1), NumStr(r2), fArc, fSweep, x2, y2, xc, yc );

        s += wxT(" \" /> \n");


    if (m_OK)
    {
        write(s);
    }
}

void wxSVGFileDCImpl::DoDrawEllipticArc(wxCoord x,wxCoord y,wxCoord w,wxCoord h,double sa,double ea)
{
    

    
    NewGraphicsIfNeeded();

    wxString s;
        double rx = w / 2;
    double ry = h / 2;
        double xc = x + rx;
    double yc = y + ry;

    double xs, ys, xe, ye;
    xs = xc + rx * cos (wxDegToRad(sa));
    xe = xc + rx * cos (wxDegToRad(ea));
    ys = yc - ry * sin (wxDegToRad(sa));
    ye = yc - ry * sin (wxDegToRad(ea));

    
    double theta1 = atan2(ys-yc, xs-xc);
    double theta2 = atan2(ye-yc, xe-xc);

    int fArc;                      if ( (theta2 - theta1) > 0 ) fArc = 1; else fArc = 0;

    int fSweep;
    if ( fabs(theta2 - theta1) > M_PI) fSweep = 1; else fSweep = 0;

    s.Printf ( wxT("<path d=\"M%d %d A%d %d 0.0 %d %d  %d %d L %d %d z "),
        int(xs), int(ys), int(rx), int(ry),
        fArc, fSweep, int(xe), int(ye), int(xc), int(yc)  );

    s += wxT(" \" /> \n");

    if (m_OK)
    {
        write(s);
    }
}

void wxSVGFileDCImpl::DoSetClippingRegion( int x,  int y, int width, int height )
{
    wxString svg;

            svg << "</g>\n"
           "<defs>\n"
           "<clipPath id=\"clip" << m_clipNestingLevel << "\">\n"
           "<rect id=\"cliprect" << m_clipNestingLevel << "\" "
                "x=\"" << x << "\" "
                "y=\"" << y << "\" "
                "width=\"" << width << "\" "
                "height=\"" << height << "\" "
                "style=\"stroke: gray; fill: none;\"/>\n"
           "</clipPath>\n"
           "</defs>\n"
           "<g style=\"clip-path: url(#clip" << m_clipNestingLevel << ");\">\n";

    write(svg);

        DoStartNewGraphics();

    m_clipUniqueId++;
    m_clipNestingLevel++;
}

void wxSVGFileDCImpl::DestroyClippingRegion()
{
    wxString svg;

            svg << "</g>\n";

        for ( size_t i = 0; i < m_clipUniqueId; i++ )
    {
        svg << "</g>";
    }
    svg << "\n";

    write(svg);

                DoStartNewGraphics();

    m_clipUniqueId = 0;
}

void wxSVGFileDCImpl::DoGetTextExtent(const wxString& string, wxCoord *w, wxCoord *h, wxCoord *descent , wxCoord *externalLeading , const wxFont *font) const

{
    wxScreenDC sDC;

    sDC.SetFont (m_font);
    if ( font != NULL ) sDC.SetFont ( *font );
    sDC.GetTextExtent(string, w,  h, descent, externalLeading );
}

wxCoord wxSVGFileDCImpl::GetCharHeight() const
{
    wxScreenDC sDC;
    sDC.SetFont (m_font);

    return sDC.GetCharHeight();

}

wxCoord wxSVGFileDCImpl::GetCharWidth() const
{
    wxScreenDC sDC;
    sDC.SetFont (m_font);

    return sDC.GetCharWidth();
}



void wxSVGFileDCImpl::SetBackground( const wxBrush &brush )
{
    m_backgroundBrush = brush;
}


void wxSVGFileDCImpl::SetBackgroundMode( int mode )
{
    m_backgroundMode = mode;
}

void wxSVGFileDCImpl::SetBitmapHandler(wxSVGBitmapHandler* handler)
{
    delete m_bmp_handler;
    m_bmp_handler = handler;
}

void wxSVGFileDCImpl::SetBrush(const wxBrush& brush)
{
    m_brush = brush;

    m_graphics_changed = true;
}


void wxSVGFileDCImpl::SetPen(const wxPen& pen)
{
            m_pen = pen;

    m_graphics_changed = true;
}

void wxSVGFileDCImpl::NewGraphicsIfNeeded()
{
    if ( !m_graphics_changed )
        return;

    m_graphics_changed = false;

    write(wxS("</g>\n"));

    DoStartNewGraphics();
}

void wxSVGFileDCImpl::DoStartNewGraphics()
{
    wxString s, sBrush, sPenCap, sPenJoin, sPenStyle, sLast;

    sBrush = wxS("<g style=\"") + wxBrushString ( m_brush.GetColour(), m_brush.GetStyle() )
            + wxPenString(m_pen.GetColour(), m_pen.GetStyle());

    switch ( m_pen.GetCap() )
    {
        case  wxCAP_PROJECTING :
            sPenCap = wxT("stroke-linecap:square; ");
            break;
        case  wxCAP_BUTT :
            sPenCap = wxT("stroke-linecap:butt; ");
            break;
        case    wxCAP_ROUND :
        default :
            sPenCap = wxT("stroke-linecap:round; ");
    }

    switch ( m_pen.GetJoin() )
    {
        case  wxJOIN_BEVEL :
            sPenJoin = wxT("stroke-linejoin:bevel; ");
            break;
        case  wxJOIN_MITER :
            sPenJoin = wxT("stroke-linejoin:miter; ");
            break;
        case    wxJOIN_ROUND :
        default :
            sPenJoin = wxT("stroke-linejoin:round; ");
    }

    sLast.Printf( wxT("stroke-width:%d\" \n   transform=\"translate(%s %s) scale(%s %s)\">"),
                m_pen.GetWidth(), NumStr(m_logicalOriginX), NumStr(m_logicalOriginY), NumStr(m_scaleX), NumStr(m_scaleY)  );

    s = sBrush + sPenCap + sPenJoin + sPenStyle + sLast + wxT("\n");
    write(s);
}


void wxSVGFileDCImpl::SetFont(const wxFont& font)

{
    m_font = font;
}

bool wxSVGFileDCImpl::DoBlit(wxCoord xdest, wxCoord ydest, wxCoord width, wxCoord height,
                        wxDC* source, wxCoord xsrc, wxCoord ysrc,
                        wxRasterOperationMode logicalFunc , bool useMask ,
                        wxCoord , wxCoord )
{
    if (logicalFunc != wxCOPY)
    {
        wxASSERT_MSG(false, wxT("wxSVGFileDC::DoBlit Call requested nonCopy mode; this is not possible"));
        return false;
    }
    if (useMask != false)
    {
        wxASSERT_MSG(false, wxT("wxSVGFileDC::DoBlit Call requested false mask; this is not possible"));
        return false;
    }
    wxBitmap myBitmap (width, height);
    wxMemoryDC memDC;
    memDC.SelectObject( myBitmap );
    memDC.Blit(0, 0, width, height, source, xsrc, ysrc);
    memDC.SelectObject( wxNullBitmap );
    DoDrawBitmap(myBitmap, xdest, ydest);
    return false;
}

void wxSVGFileDCImpl::DoDrawIcon(const class wxIcon & myIcon, wxCoord x, wxCoord y)
{
    wxBitmap myBitmap (myIcon.GetWidth(), myIcon.GetHeight() );
    wxMemoryDC memDC;
    memDC.SelectObject( myBitmap );
    memDC.DrawIcon(myIcon,0,0);
    memDC.SelectObject( wxNullBitmap );
    DoDrawBitmap(myBitmap, x, y);
}

void wxSVGFileDCImpl::DoDrawBitmap(const class wxBitmap & bmp, wxCoord x, wxCoord y , bool  WXUNUSED(bTransparent)  )
{
    NewGraphicsIfNeeded();

        if ( !m_bmp_handler )
        m_bmp_handler = new wxSVGBitmapFileHandler();

    m_bmp_handler->ProcessBitmap(bmp, x, y, *m_outfile);
}

void wxSVGFileDCImpl::write(const wxString &s)
{
    const wxCharBuffer buf = s.utf8_str();
    m_outfile->Write(buf, strlen((const char *)buf));
    m_OK = m_outfile->IsOk();
}


#ifdef __BORLANDC__
#pragma warn .rch
#pragma warn .ccc
#endif

#endif 
