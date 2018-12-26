

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/colour.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/utils.h"
    #include "wx/gdicmn.h"
    #include "wx/wxcrtvararg.h"
#endif

#if wxUSE_VARIANT
IMPLEMENT_VARIANT_OBJECT_EXPORTED(wxColour,WXDLLEXPORT)
#endif



#if wxUSE_EXTENDED_RTTI

#include <string.h>

template<> void wxStringReadValue(const wxString &s, wxColour &data )
{
    if ( !data.Set(s) )
    {
        wxLogError(_("String To Colour : Incorrect colour specification : %s"),
                   s.c_str() );
        data = wxNullColour;
    }
}

template<> void wxStringWriteValue(wxString &s, const wxColour &data )
{
    s = data.GetAsString(wxC2S_HTML_SYNTAX);
}

wxTO_STRING_IMP( wxColour )
wxFROM_STRING_IMP( wxColour )

wxIMPLEMENT_DYNAMIC_CLASS_WITH_COPY_AND_STREAMERS_XTI( wxColour, wxObject,  \
                                                      "wx/colour.h",  &wxTO_STRING( wxColour ), &wxFROM_STRING( wxColour ))
wxBEGIN_PROPERTIES_TABLE(wxColour)
wxREADONLY_PROPERTY( Red, unsigned char, Red, wxEMPTY_PARAMETER_VALUE, \
                    0 , wxT("Helpstring"), wxT("group"))
wxREADONLY_PROPERTY( Green, unsigned char, Green, wxEMPTY_PARAMETER_VALUE, \
                    0 , wxT("Helpstring"), wxT("group"))
wxREADONLY_PROPERTY( Blue, unsigned char, Blue, wxEMPTY_PARAMETER_VALUE, \
                    0 , wxT("Helpstring"), wxT("group"))
wxEND_PROPERTIES_TABLE()

wxDIRECT_CONSTRUCTOR_3( wxColour, unsigned char, Red, \
                       unsigned char, Green, unsigned char, Blue )

wxEMPTY_HANDLERS_TABLE(wxColour)
#else

#if wxCOLOUR_IS_GDIOBJECT
wxIMPLEMENT_DYNAMIC_CLASS(wxColour, wxGDIObject);
#else
wxIMPLEMENT_DYNAMIC_CLASS(wxColour, wxObject);
#endif

#endif


bool wxColourBase::FromString(const wxString& str)
{
    if ( str.empty() )
        return false;       
    if ( wxStrnicmp(str, wxT("RGB"), 3) == 0 )
    {
                                int red, green, blue,
            alpha = wxALPHA_OPAQUE;
        if ( str.length() > 3 && (str[3] == wxT('a') || str[3] == wxT('A')) )
        {
                                                                                    
            const unsigned len = str.length();             wxCharBuffer alphaBuf(len);
            char * const alphaPtr = alphaBuf.data();

            for ( unsigned n = 0; n < len; n++ )
                alphaPtr[n] = '\0';

                                    wxString formatStr;
            formatStr << wxS("( %d , %d , %d , %") << len << 'c';

                                                                                    if ( sscanf(str.c_str() + 4,
                        formatStr.mb_str(),
                        &red, &green, &blue, alphaPtr) != 4 )
                return false;

                                    wxString alphaStr(alphaPtr, wxStrlen(alphaPtr));
            if ( alphaStr.empty() || alphaStr.Last() != ')' )
                return false;

            alphaStr.RemoveLast();
            alphaStr.Trim();

            double a;
            if ( !alphaStr.ToCDouble(&a) )
                return false;

            alpha = wxRound(a * 255);
        }
        else         {
            if ( wxSscanf(str.wx_str() + 3, wxT("( %d , %d , %d )"),
                                                &red, &green, &blue) != 3 )
                return false;
        }

        Set((unsigned char)wxClip(red, 0, 255),
            (unsigned char)wxClip(green, 0, 255),
            (unsigned char)wxClip(blue, 0, 255),
            (unsigned char)wxClip(alpha, 0, 255));
    }
    else if ( str[0] == wxT('#') && wxStrlen(str) == 7 )
    {
                unsigned long tmp;
        if (wxSscanf(str.wx_str() + 1, wxT("%lx"), &tmp) != 1)
            return false;

        Set((unsigned char)(tmp >> 16),
            (unsigned char)(tmp >> 8),
            (unsigned char)tmp);
    }
    else if (wxTheColourDatabase)     {
                                        wxColour clr = wxTheColourDatabase->Find(str);
        if (clr.IsOk())
            Set((unsigned char)clr.Red(),
                (unsigned char)clr.Green(),
                (unsigned char)clr.Blue());
    }

    if (IsOk())
        return true;

    wxLogDebug(wxT("wxColour::Set - couldn't set to colour string '%s'"), str);
    return false;
}

wxString wxColourBase::GetAsString(long flags) const
{
    wxString colName;

    const bool isOpaque = Alpha() == wxALPHA_OPAQUE;

            if ( (flags & wxC2S_NAME) && isOpaque )
    {
        colName = wxTheColourDatabase->FindName(
                    static_cast<const wxColour &>(*this)).MakeLower();
    }

    if ( colName.empty() )
    {
        const int red = Red(),
                  blue = Blue(),
                  green = Green();

        if ( flags & wxC2S_CSS_SYNTAX )
        {
                        if ( isOpaque )
            {
                colName.Printf(wxT("rgb(%d, %d, %d)"), red, green, blue);
            }
            else             {
                colName.Printf(wxT("rgba(%d, %d, %d, %s)"),
                               red, green, blue,
                               wxString::FromCDouble(Alpha() / 255., 3));
            }
        }
        else if ( flags & wxC2S_HTML_SYNTAX )
        {
            wxASSERT_MSG( isOpaque, "alpha is lost in HTML syntax" );

                        colName.Printf(wxT("#%02X%02X%02X"), red, green, blue);
        }
    }

        wxASSERT_MSG(!colName.empty(),
                 wxT("Invalid wxColour -> wxString conversion flags"));

    return colName;
}

void wxColourBase::MakeMono(unsigned char* r, unsigned char* g, unsigned char* b,
                            bool on)
{
    *r = *g = *b = on ? 255 : 0;
}

void wxColourBase::MakeGrey(unsigned char* r, unsigned char* g, unsigned char* b
                            
                           )
{
    *r = *g = *b = (wxByte)(((*b)*117UL + (*g)*601UL + (*r)*306UL) >> 10);
}

void wxColourBase::MakeGrey(unsigned char* r, unsigned char* g, unsigned char* b,
                            double weight_r, double weight_g, double weight_b)
{
    double luma = (*r) * weight_r + (*g) * weight_g + (*b) * weight_b;
    *r = *g = *b = (wxByte)wxRound(luma);
}

void wxColourBase::MakeDisabled(unsigned char* r, unsigned char* g, unsigned char* b,
                                unsigned char brightness)
{
        *r = AlphaBlend(*r, brightness, 0.4);
    *g = AlphaBlend(*g, brightness, 0.4);
    *b = AlphaBlend(*b, brightness, 0.4);
}

wxColour& wxColourBase::MakeDisabled(unsigned char brightness)
{
    unsigned char r = Red(),
                  g = Green(),
                  b = Blue();
    MakeDisabled(&r, &g, &b, brightness);
    Set(r, g, b, Alpha());
    return static_cast<wxColour&>(*this);
}


unsigned char wxColourBase::AlphaBlend(unsigned char fg, unsigned char bg,
                                       double alpha)
{
    double result = bg + (alpha * (fg - bg));
    result = wxMax(result,   0.0);
    result = wxMin(result, 255.0);
    return (unsigned char)result;
}


void wxColourBase::ChangeLightness(unsigned char* r, unsigned char* g, unsigned char* b,
                                   int ialpha)
{
    if (ialpha == 100) return;

                ialpha = wxMax(ialpha,   0);
    ialpha = wxMin(ialpha, 200);
    double alpha = ((double)(ialpha - 100.0))/100.0;

    unsigned char bg;
    if (ialpha > 100)
    {
                bg = 255;
        alpha = 1.0 - alpha;      }
    else
    {
                bg = 0;
        alpha = 1.0 + alpha;      }

    *r = AlphaBlend(*r, bg, alpha);
    *g = AlphaBlend(*g, bg, alpha);
    *b = AlphaBlend(*b, bg, alpha);
}

wxColour wxColourBase::ChangeLightness(int ialpha) const
{
    wxByte r = Red();
    wxByte g = Green();
    wxByte b = Blue();
    ChangeLightness(&r, &g, &b, ialpha);
    return wxColour(r,g,b);
}

wxString wxToString(const wxColourBase& col)
{
    return col.IsOk() ? col.GetAsString(wxC2S_CSS_SYNTAX)
                      : wxString();
}

bool wxFromString(const wxString& str, wxColourBase *col)
{
    wxCHECK_MSG( col, false, wxT("NULL output parameter") );

    if ( str.empty() )
    {
        *col = wxNullColour;
        return true;
    }

    return col->Set(str);
}


