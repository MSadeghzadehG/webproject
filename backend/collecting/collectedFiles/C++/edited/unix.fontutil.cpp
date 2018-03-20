


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/fontutil.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/font.h"     #include "wx/hash.h"
    #include "wx/utils.h"           #include "wx/module.h"
#endif 
#include "wx/encinfo.h"
#include "wx/fontmap.h"
#include "wx/tokenzr.h"
#include "wx/fontenum.h"

#if wxUSE_PANGO

#include <pango/pango.h>

PangoContext* wxGetPangoContext();

#ifdef __WXGTK20__
    #include "wx/gtk/private.h"

    #define wxPANGO_CONV wxGTK_CONV_SYS
    #define wxPANGO_CONV_BACK wxGTK_CONV_BACK_SYS
#else
    #include "wx/x11/private.h"
    #include "wx/gtk/private/string.h"

    #define wxPANGO_CONV(s) s.utf8_str()
    #define wxPANGO_CONV_BACK(s) wxString::FromUTF8Unchecked(s)
#endif


void wxNativeFontInfo::Init()
{
    description = NULL;
    m_underlined = false;
    m_strikethrough = false;
}

void wxNativeFontInfo::Init(const wxNativeFontInfo& info)
{
    if (info.description)
    {
        description = pango_font_description_copy(info.description);
        m_underlined = info.GetUnderlined();
        m_strikethrough = info.GetStrikethrough();
    }
    else
    {
        description = NULL;
        m_underlined = false;
        m_strikethrough = false;
    }
}

void wxNativeFontInfo::Free()
{
    if (description)
        pango_font_description_free(description);
}

int wxNativeFontInfo::GetPointSize() const
{
    return pango_font_description_get_size( description ) / PANGO_SCALE;
}

wxFontStyle wxNativeFontInfo::GetStyle() const
{
    wxFontStyle m_style = wxFONTSTYLE_NORMAL;

    switch (pango_font_description_get_style( description ))
    {
        case PANGO_STYLE_NORMAL:
            m_style = wxFONTSTYLE_NORMAL;
            break;
        case PANGO_STYLE_ITALIC:
            m_style = wxFONTSTYLE_ITALIC;
            break;
        case PANGO_STYLE_OBLIQUE:
            m_style = wxFONTSTYLE_SLANT;
            break;
    }

    return m_style;
}

wxFontWeight wxNativeFontInfo::GetWeight() const
{
                
    PangoWeight pango_weight = pango_font_description_get_weight( description );

                
    if (pango_weight >= 600)
        return wxFONTWEIGHT_BOLD;

    if (pango_weight < 350)
        return wxFONTWEIGHT_LIGHT;

    return wxFONTWEIGHT_NORMAL;
}

bool wxNativeFontInfo::GetUnderlined() const
{
    return m_underlined;
}

bool wxNativeFontInfo::GetStrikethrough() const
{
    return m_strikethrough;
}

wxString wxNativeFontInfo::GetFaceName() const
{
        return wxPANGO_CONV_BACK(pango_font_description_get_family(description));
}

wxFontFamily wxNativeFontInfo::GetFamily() const
{
    wxFontFamily ret = wxFONTFAMILY_UNKNOWN;

    const char *family_name = pango_font_description_get_family( description );

                if ( !family_name )
        return ret;
    wxGtkString family_text(g_ascii_strdown(family_name, strlen(family_name)));

            if (wxStrnicmp( family_text, "monospace", 9 ) == 0)
        ret = wxFONTFAMILY_TELETYPE;        else if (wxStrnicmp( family_text, "courier", 7 ) == 0)
        ret = wxFONTFAMILY_TELETYPE;    #if defined(__WXGTK20__) || defined(HAVE_PANGO_FONT_FAMILY_IS_MONOSPACE)
    else
    {
        PangoFontFamily **families;
        PangoFontFamily  *family = NULL;
        int n_families;
        PangoContext* context = wxGetPangoContext();
        pango_context_list_families(context, &families, &n_families);

        for (int i = 0; i < n_families; ++i)
        {
            if (g_ascii_strcasecmp(pango_font_family_get_name( families[i] ),
                                   pango_font_description_get_family( description )) == 0 )
            {
                family = families[i];
                break;
            }
        }

        g_free(families);
        g_object_unref(context);

                                
        if (family != NULL && pango_font_family_is_monospace( family ))
            ret = wxFONTFAMILY_TELETYPE;     }
#endif 
    if (ret == wxFONTFAMILY_UNKNOWN)
    {
        if (strstr( family_text, "sans" ) != NULL || strstr( family_text, "Sans" ) != NULL)
                        ret = wxFONTFAMILY_SWISS;               else if (strstr( family_text, "serif" ) != NULL || strstr( family_text, "Serif" ) != NULL)
            ret = wxFONTFAMILY_ROMAN;               else if (wxStrnicmp( family_text, "times", 5 ) == 0)
            ret = wxFONTFAMILY_ROMAN;               else if (wxStrnicmp( family_text, "old", 3 ) == 0)
            ret = wxFONTFAMILY_DECORATIVE;      }

    return ret;
}

wxFontEncoding wxNativeFontInfo::GetEncoding() const
{
    return wxFONTENCODING_SYSTEM;
}

void wxNativeFontInfo::SetPointSize(int pointsize)
{
    pango_font_description_set_size( description, pointsize * PANGO_SCALE );
}

void wxNativeFontInfo::SetStyle(wxFontStyle style)
{
    switch (style)
    {
        case wxFONTSTYLE_ITALIC:
            pango_font_description_set_style( description, PANGO_STYLE_ITALIC );
            break;
        case wxFONTSTYLE_SLANT:
            pango_font_description_set_style( description, PANGO_STYLE_OBLIQUE );
            break;
        default:
            wxFAIL_MSG( "unknown font style" );
                    case wxFONTSTYLE_NORMAL:
            pango_font_description_set_style( description, PANGO_STYLE_NORMAL );
            break;
    }
}

void wxNativeFontInfo::SetWeight(wxFontWeight weight)
{
    switch (weight)
    {
        case wxFONTWEIGHT_BOLD:
            pango_font_description_set_weight(description, PANGO_WEIGHT_BOLD);
            break;
        case wxFONTWEIGHT_LIGHT:
            pango_font_description_set_weight(description, PANGO_WEIGHT_LIGHT);
            break;
        default:
            wxFAIL_MSG( "unknown font weight" );
                    case wxFONTWEIGHT_NORMAL:
            pango_font_description_set_weight(description, PANGO_WEIGHT_NORMAL);
    }
}

void wxNativeFontInfo::SetUnderlined(bool underlined)
{
            m_underlined = underlined;
}

void wxNativeFontInfo::SetStrikethrough(bool strikethrough)
{
            m_strikethrough = strikethrough;
}

bool wxNativeFontInfo::SetFaceName(const wxString& facename)
{
    pango_font_description_set_family(description, wxPANGO_CONV(facename));

                return true;
}

void wxNativeFontInfo::SetFamily(wxFontFamily family)
{
    wxArrayString facename;

        
    switch ( family )
    {
        case wxFONTFAMILY_SCRIPT:
                        facename.Add(wxS("URW Chancery L"));
            facename.Add(wxS("Comic Sans MS"));
            break;

        case wxFONTFAMILY_DECORATIVE:
                        facename.Add(wxS("Impact"));
            break;

        case wxFONTFAMILY_ROMAN:
                        facename.Add(wxS("Serif"));
            facename.Add(wxS("DejaVu Serif"));
            facename.Add(wxS("DejaVu LGC Serif"));
            facename.Add(wxS("Bitstream Vera Serif"));
            facename.Add(wxS("Liberation Serif"));
            facename.Add(wxS("FreeSerif"));
            facename.Add(wxS("Luxi Serif"));
            facename.Add(wxS("Times New Roman"));
            facename.Add(wxS("Century Schoolbook L"));
            facename.Add(wxS("URW Bookman L"));
            facename.Add(wxS("URW Palladio L"));
            facename.Add(wxS("Times"));
            break;

        case wxFONTFAMILY_TELETYPE:
        case wxFONTFAMILY_MODERN:
                        facename.Add(wxS("Monospace"));
            facename.Add(wxS("DejaVu Sans Mono"));
            facename.Add(wxS("DejaVu LGC Sans Mono"));
            facename.Add(wxS("Bitstream Vera Sans Mono"));
            facename.Add(wxS("Liberation Mono"));
            facename.Add(wxS("FreeMono"));
            facename.Add(wxS("Luxi Mono"));
            facename.Add(wxS("Courier New"));
            facename.Add(wxS("Lucida Sans Typewriter"));
            facename.Add(wxS("Nimbus Mono L"));
            facename.Add(wxS("Andale Mono"));
            facename.Add(wxS("Courier"));
            break;

        case wxFONTFAMILY_SWISS:
        case wxFONTFAMILY_DEFAULT:
        default:
                        facename.Add(wxS("Sans"));
            facename.Add(wxS("DejaVu Sans"));
            facename.Add(wxS("DejaVu LGC Sans"));
            facename.Add(wxS("Bitstream Vera Sans"));
            facename.Add(wxS("Liberation Sans"));
            facename.Add(wxS("FreeSans"));
            facename.Add(wxS("Luxi Sans"));
            facename.Add(wxS("Arial"));
            facename.Add(wxS("Lucida Sans"));
            facename.Add(wxS("Nimbus Sans L"));
            facename.Add(wxS("URW Gothic L"));
            break;
    }

    SetFaceName(facename);
}

void wxNativeFontInfo::SetEncoding(wxFontEncoding WXUNUSED(encoding))
{
    wxFAIL_MSG( "not implemented: Pango encoding is always UTF8" );
}

bool wxNativeFontInfo::FromString(const wxString& s)
{
    wxString str(s);

                m_underlined = str.StartsWith(wxS("underlined "), &str);
    m_strikethrough = str.StartsWith(wxS("strikethrough "), &str);

    if (description)
        pango_font_description_free( description );

                                const size_t pos = str.find_last_of(wxS(" "));
    double size;
    if ( pos != wxString::npos && wxString(str, pos + 1).ToDouble(&size) )
    {
        wxString sizeStr;
        if ( size < 1 )
            sizeStr = wxS("1");
        else if ( size >= 1E6 )
            sizeStr = wxS("1E6");

        if ( !sizeStr.empty() )
        {
                        str = wxString(s, 0, pos) + sizeStr;
        }
    }

    description = pango_font_description_from_string(wxPANGO_CONV(str));

#if wxUSE_FONTENUM
        if (!wxFontEnumerator::IsValidFacename(GetFaceName()))
        SetFaceName(wxNORMAL_FONT->GetFaceName());
#endif 
    return true;
}

wxString wxNativeFontInfo::ToString() const
{
    wxGtkString str(pango_font_description_to_string( description ));
    wxString desc = wxPANGO_CONV_BACK(str);

                    if (m_strikethrough)
        desc.insert(0, wxS("strikethrough "));
    if (m_underlined)
        desc.insert(0, wxS("underlined "));

    return desc;
}

bool wxNativeFontInfo::FromUserString(const wxString& s)
{
    return FromString( s );
}

wxString wxNativeFontInfo::ToUserString() const
{
    return ToString();
}

#else 
#ifdef __X__
    #ifdef __VMS__
        #pragma message disable nosimpint
    #endif

    #include <X11/Xlib.h>

    #ifdef __VMS__
        #pragma message enable nosimpint
    #endif

#elif defined(__WXGTK__)
                    #include <time.h>

    #include <gdk/gdk.h>
#endif



static wxHashTable *g_fontHash = NULL;


#ifdef __X__
    wxNativeFont wxLoadFont(const wxString& fontSpec)
    {
        return XLoadQueryFont((Display *)wxGetDisplay(), fontSpec);
    }

    inline void wxFreeFont(wxNativeFont font)
    {
        XFreeFont((Display *)wxGetDisplay(), (XFontStruct *)font);
    }
#elif defined(__WXGTK__)
    wxNativeFont wxLoadFont(const wxString& fontSpec)
    {
                                                        return gdk_font_load( wxConvertWX2MB(fontSpec) );
    }

    inline void wxFreeFont(wxNativeFont font)
    {
        gdk_font_unref(font);
    }
#else
    #error "Unknown GUI toolkit"
#endif

static bool wxTestFontSpec(const wxString& fontspec);

static wxNativeFont wxLoadQueryFont(int pointSize,
                                    wxFontFamily family,
                                    wxFontStyle style,
                                    wxFontWeight weight,
                                    bool underlined,
                                    const wxString& facename,
                                    const wxString& xregistry,
                                    const wxString& xencoding,
                                    wxString* xFontName);



bool wxNativeEncodingInfo::FromString(const wxString& s)
{
        wxStringTokenizer tokenizer(s, wxT(";"));

    wxString encid = tokenizer.GetNextToken();
    long enc;
    if ( !encid.ToLong(&enc) )
        return false;
    encoding = (wxFontEncoding)enc;

    xregistry = tokenizer.GetNextToken();
    if ( !xregistry )
        return false;

    xencoding = tokenizer.GetNextToken();
    if ( !xencoding )
        return false;

        facename = tokenizer.GetNextToken();

    return true;
}

wxString wxNativeEncodingInfo::ToString() const
{
    wxString s;
    s << (long)encoding << wxT(';') << xregistry << wxT(';') << xencoding;
    if ( !facename.empty() )
    {
        s << wxT(';') << facename;
    }

    return s;
}


void wxNativeFontInfo::Init()
{
    m_isDefault = true;
}

bool wxNativeFontInfo::FromString(const wxString& s)
{
    wxStringTokenizer tokenizer(s, wxT(";"));

        wxString token = tokenizer.GetNextToken();
    if ( token != wxT('0') )
        return false;

    xFontName = tokenizer.GetNextToken();

        if ( tokenizer.HasMoreTokens() )
        return false;

    return FromXFontName(xFontName);
}

wxString wxNativeFontInfo::ToString() const
{
        return wxString::Format(wxT("%d;%s"), 0, GetXFontName().c_str());
}

bool wxNativeFontInfo::FromUserString(const wxString& s)
{
    return FromXFontName(s);
}

wxString wxNativeFontInfo::ToUserString() const
{
    return GetXFontName();
}

bool wxNativeFontInfo::HasElements() const
{
            return !fontElements[0].empty();
}

wxString wxNativeFontInfo::GetXFontComponent(wxXLFDField field) const
{
    wxCHECK_MSG( field < wxXLFD_MAX, wxEmptyString, wxT("invalid XLFD field") );

    if ( !HasElements() )
    {
        if ( !const_cast<wxNativeFontInfo *>(this)->FromXFontName(xFontName) )
            return wxEmptyString;
    }

    return fontElements[field];
}

bool wxNativeFontInfo::FromXFontName(const wxString& fontname)
{
        wxStringTokenizer tokenizer(fontname, wxT("-"));

        if ( !tokenizer.HasMoreTokens() )
        return false;

    (void)tokenizer.GetNextToken();

    for ( size_t n = 0; n < WXSIZEOF(fontElements); n++ )
    {
        if ( !tokenizer.HasMoreTokens() )
        {
                        return false;
        }

        wxString field = tokenizer.GetNextToken();
        if ( !field.empty() && field != wxT('*') )
        {
                        m_isDefault = false;
        }

        fontElements[n] = field;
    }

        if ( tokenizer.HasMoreTokens() )
        return false;

    return true;
}

wxString wxNativeFontInfo::GetXFontName() const
{
    if ( xFontName.empty() )
    {
        for ( size_t n = 0; n < WXSIZEOF(fontElements); n++ )
        {
                                    wxString elt = fontElements[n];
            if ( elt.empty() && n != wxXLFD_ADDSTYLE )
            {
                elt = wxT('*');
            }

            const_cast<wxNativeFontInfo *>(this)->xFontName << wxT('-') << elt;
        }
    }

    return xFontName;
}

void
wxNativeFontInfo::SetXFontComponent(wxXLFDField field, const wxString& value)
{
    wxCHECK_RET( field < wxXLFD_MAX, wxT("invalid XLFD field") );

            wxASSERT_MSG( !IsDefault(), wxT("can't modify an uninitialized XLFD") );

    if ( !HasElements() )
    {
        if ( !const_cast<wxNativeFontInfo *>(this)->FromXFontName(xFontName) )
        {
            wxFAIL_MSG( wxT("can't set font element for invalid XLFD") );

            return;
        }
    }

    fontElements[field] = value;

        xFontName.clear();
}

void wxNativeFontInfo::SetXFontName(const wxString& xFontName_)
{
        fontElements[0].clear();

    xFontName = xFontName_;

    m_isDefault = false;
}

int wxNativeFontInfo::GetPointSize() const
{
    const wxString s = GetXFontComponent(wxXLFD_POINTSIZE);

        long l;
    return s.ToLong(&l) ? l : -1;
}

wxFontStyle wxNativeFontInfo::GetStyle() const
{
    const wxString s = GetXFontComponent(wxXLFD_SLANT);

    if ( s.length() != 1 )
    {
                        return wxFONTSTYLE_NORMAL;
    }

    switch ( s[0].GetValue() )
    {
        default:
            
        case wxT('r'):
            return wxFONTSTYLE_NORMAL;

        case wxT('i'):
            return wxFONTSTYLE_ITALIC;

        case wxT('o'):
            return wxFONTSTYLE_SLANT;
    }
}

wxFontWeight wxNativeFontInfo::GetWeight() const
{
    const wxString s = GetXFontComponent(wxXLFD_WEIGHT).MakeLower();
    if ( s.find(wxT("bold")) != wxString::npos || s == wxT("black") )
        return wxFONTWEIGHT_BOLD;
    else if ( s == wxT("light") )
        return wxFONTWEIGHT_LIGHT;

    return wxFONTWEIGHT_NORMAL;
}

bool wxNativeFontInfo::GetUnderlined() const
{
        return false;
}

wxString wxNativeFontInfo::GetFaceName() const
{
        return GetXFontComponent(wxXLFD_FAMILY);
}

wxFontFamily wxNativeFontInfo::GetFamily() const
{
            wxFAIL_MSG(wxT("not implemented")); 
    return wxFONTFAMILY_DEFAULT;
}

wxFontEncoding wxNativeFontInfo::GetEncoding() const
{
        wxFAIL_MSG( wxT("not implemented") );

    return wxFONTENCODING_MAX;
}

void wxNativeFontInfo::SetPointSize(int pointsize)
{
    SetXFontComponent(wxXLFD_POINTSIZE, wxString::Format(wxT("%d"), pointsize));
}

void wxNativeFontInfo::SetStyle(wxFontStyle style)
{
    wxString s;
    switch ( style )
    {
        case wxFONTSTYLE_ITALIC:
            s = wxT('i');
            break;

        case wxFONTSTYLE_SLANT:
            s = wxT('o');
            break;

        case wxFONTSTYLE_NORMAL:
            s = wxT('r');

        default:
            wxFAIL_MSG( wxT("unknown wxFontStyle in wxNativeFontInfo::SetStyle") );
            return;
    }

    SetXFontComponent(wxXLFD_SLANT, s);
}

void wxNativeFontInfo::SetWeight(wxFontWeight weight)
{
    wxString s;
    switch ( weight )
    {
        case wxFONTWEIGHT_BOLD:
            s = wxT("bold");
            break;

        case wxFONTWEIGHT_LIGHT:
            s = wxT("light");
            break;

        case wxFONTWEIGHT_NORMAL:
            s = wxT("medium");
            break;

        default:
            wxFAIL_MSG( wxT("unknown wxFontWeight in wxNativeFontInfo::SetWeight") );
            return;
    }

    SetXFontComponent(wxXLFD_WEIGHT, s);
}

void wxNativeFontInfo::SetUnderlined(bool WXUNUSED(underlined))
{
    }

void wxNativeFontInfo::SetStrikethrough(bool WXUNUSED(strikethrough))
{
    }

bool wxNativeFontInfo::SetFaceName(const wxString& facename)
{
    SetXFontComponent(wxXLFD_FAMILY, facename);
    return true;
}

void wxNativeFontInfo::SetFamily(wxFontFamily WXUNUSED(family))
{
        wxFAIL_MSG( wxT("not implemented") );

    }

void wxNativeFontInfo::SetEncoding(wxFontEncoding encoding)
{
    wxNativeEncodingInfo info;
    if ( wxGetNativeFontEncoding(encoding, &info) )
    {
        SetXFontComponent(wxXLFD_ENCODING, info.xencoding);
        SetXFontComponent(wxXLFD_REGISTRY, info.xregistry);
    }
}


bool wxGetNativeFontEncoding(wxFontEncoding encoding,
                             wxNativeEncodingInfo *info)
{
    wxCHECK_MSG( info, false, wxT("bad pointer in wxGetNativeFontEncoding") );

    if ( encoding == wxFONTENCODING_DEFAULT )
    {
        encoding = wxFont::GetDefaultEncoding();
    }

    switch ( encoding )
    {
        case wxFONTENCODING_ISO8859_1:
        case wxFONTENCODING_ISO8859_2:
        case wxFONTENCODING_ISO8859_3:
        case wxFONTENCODING_ISO8859_4:
        case wxFONTENCODING_ISO8859_5:
        case wxFONTENCODING_ISO8859_6:
        case wxFONTENCODING_ISO8859_7:
        case wxFONTENCODING_ISO8859_8:
        case wxFONTENCODING_ISO8859_9:
        case wxFONTENCODING_ISO8859_10:
        case wxFONTENCODING_ISO8859_11:
        case wxFONTENCODING_ISO8859_12:
        case wxFONTENCODING_ISO8859_13:
        case wxFONTENCODING_ISO8859_14:
        case wxFONTENCODING_ISO8859_15:
            {
                int cp = encoding - wxFONTENCODING_ISO8859_1 + 1;
                info->xregistry = wxT("iso8859");
                info->xencoding.Printf(wxT("%d"), cp);
            }
            break;

        case wxFONTENCODING_UTF8:
            info->xregistry = wxT("iso10646");
            info->xencoding = wxT("*");
            break;

        case wxFONTENCODING_GB2312:
            info->xregistry = wxT("GB2312");               info->xencoding = wxT("*");
            break;

        case wxFONTENCODING_KOI8:
        case wxFONTENCODING_KOI8_U:
            info->xregistry = wxT("koi8");

                        info->xencoding = wxT("*");
            break;

        case wxFONTENCODING_CP1250:
        case wxFONTENCODING_CP1251:
        case wxFONTENCODING_CP1252:
        case wxFONTENCODING_CP1253:
        case wxFONTENCODING_CP1254:
        case wxFONTENCODING_CP1255:
        case wxFONTENCODING_CP1256:
        case wxFONTENCODING_CP1257:
            {
                int cp = encoding - wxFONTENCODING_CP1250 + 1250;
                info->xregistry = wxT("microsoft");
                info->xencoding.Printf(wxT("cp%d"), cp);
            }
            break;

        case wxFONTENCODING_EUC_JP:
        case wxFONTENCODING_SHIFT_JIS:
            info->xregistry = "jis*";
            info->xencoding = "*";
            break;

        case wxFONTENCODING_SYSTEM:
            info->xregistry =
            info->xencoding = wxT("*");
            break;

        default:
                        return false;
    }

    info->encoding = encoding;

    return true;
}

bool wxTestFontEncoding(const wxNativeEncodingInfo& info)
{
    wxString fontspec;
    fontspec.Printf(wxT("-*-%s-*-*-*-*-*-*-*-*-*-*-%s-%s"),
                    info.facename.empty() ? wxString("*") : info.facename,
                    info.xregistry,
                    info.xencoding);

    return wxTestFontSpec(fontspec);
}


wxNativeFont wxLoadQueryNearestFont(int pointSize,
                                    wxFontFamily family,
                                    wxFontStyle style,
                                    wxFontWeight weight,
                                    bool underlined,
                                    const wxString &facename,
                                    wxFontEncoding encoding,
                                    wxString* xFontName)
{
    if ( encoding == wxFONTENCODING_DEFAULT )
    {
        encoding = wxFont::GetDefaultEncoding();
    }

                wxNativeEncodingInfo info;
    if ( encoding == wxFONTENCODING_SYSTEM )
    {
                wxGetNativeFontEncoding(wxFONTENCODING_SYSTEM, &info);
    }
    else
    {
        if ( !wxGetNativeFontEncoding(encoding, &info) ||
             !wxTestFontEncoding(info) )
        {
#if wxUSE_FONTMAP
            if ( !wxFontMapper::Get()->GetAltForEncoding(encoding, &info) )
#endif             {
                                                                                                wxGetNativeFontEncoding(wxFONTENCODING_SYSTEM, &info);
            }
        }
    }

        wxNativeFont font = 0;

        if( xFontName && !xFontName->empty() )
    {
                                wxStringTokenizer tokenizer(*xFontName, wxT("-"), wxTOKEN_RET_DELIMS);
        wxString newFontName;

        for(int i = 0; i < 8; i++)
          newFontName += tokenizer.NextToken();

        (void) tokenizer.NextToken();

        newFontName += wxString::Format(wxT("%d-"), pointSize);

        while(tokenizer.HasMoreTokens())
          newFontName += tokenizer.GetNextToken();

        font = wxLoadFont(newFontName);

        if(font)
          *xFontName = newFontName;
    }

    if ( !font )
    {
                int max_size = pointSize + 20 * (1 + (pointSize/180));
        int min_size = pointSize - 20 * (1 + (pointSize/180));

        int i, round; 
                        wxFontWeight testweight = weight;
        wxFontStyle teststyle = style;

        for ( round = 0; round < 3; round++ )
        {
                        if ( round == 1 )
            {
                if ( testweight != wxFONTWEIGHT_NORMAL )
                {
                    testweight = wxFONTWEIGHT_NORMAL;
                }
                else
                {
                    ++round;                 }
            }

                        if ( round == 2 )
            {
                if ( teststyle != wxFONTSTYLE_NORMAL )
                {
                    teststyle = wxFONTSTYLE_NORMAL;
                }
                else
                {
                    break;
                }
            }
                        for ( i = pointSize; !font && i >= 10 && i >= min_size; i -= 10 )
            {
                font = wxLoadQueryFont(i, family, teststyle, testweight, underlined,
                                   facename, info.xregistry, info.xencoding,
                                   xFontName);
            }

                        for ( i = pointSize + 10; !font && i <= max_size; i += 10 )
            {
                font = wxLoadQueryFont(i, family, teststyle, testweight, underlined,
                                   facename, info.xregistry, info.xencoding,
                                   xFontName);
            }
        }

                if ( !font && family != wxFONTFAMILY_DEFAULT )
        {
            font = wxLoadQueryFont(pointSize, wxFONTFAMILY_DEFAULT, style, weight,
                                   underlined, facename,
                                   info.xregistry, info.xencoding,
                                   xFontName );
        }

                        if ( !font )
        {
            font = wxLoadQueryFont(120, wxFONTFAMILY_DEFAULT,
                                   wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
                                   underlined, facename,
                                   info.xregistry, info.xencoding,
                                   xFontName);

                        if ( !font )
            {
                font = wxLoadQueryFont(120, wxFONTFAMILY_DEFAULT,
                                       wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
                                       underlined, wxEmptyString,
                                       info.xregistry, info.xencoding,
                                       xFontName);

                                                                                if ( !font )
                {
                    font = wxLoadQueryFont(-1, wxFONTFAMILY_DEFAULT,
                                           wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
                                           false, wxEmptyString,
                                           info.xregistry, info.xencoding,
                                           xFontName);

                                                                                if ( !font )
                    {
                        wxFAIL_MSG( wxT("this encoding should be available!") );

                        font = wxLoadQueryFont(-1, wxFONTFAMILY_DEFAULT,
                                               wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
                                               false, wxEmptyString,
                                               wxT("*"), wxT("*"),
                                               xFontName);
                    }
                }
            }
        }
    }

    return font;
}


static bool wxTestFontSpec(const wxString& fontspec)
{
            if ( fontspec == wxT("-*-*-*-*-*-*-*-*-*-*-*-*-*-*") )
    {
        return true;
    }

    wxNativeFont test = (wxNativeFont) g_fontHash->Get( fontspec );
    if (test)
    {
        return true;
    }

    test = wxLoadFont(fontspec);
    g_fontHash->Put( fontspec, (wxObject*) test );

    if ( test )
    {
        wxFreeFont(test);

        return true;
    }
    else
    {
        return false;
    }
}

static wxNativeFont wxLoadQueryFont(int pointSize,
                                    wxFontFamily family,
                                    wxFontStyle style,
                                    wxFontWeight weight,
                                    bool WXUNUSED(underlined),
                                    const wxString& facename,
                                    const wxString& xregistry,
                                    const wxString& xencoding,
                                    wxString* xFontName)
{
    wxString xfamily("*");
    switch (family)
    {
        case wxFONTFAMILY_DECORATIVE: xfamily = wxT("lucida"); break;
        case wxFONTFAMILY_ROMAN:      xfamily = wxT("times");  break;
        case wxFONTFAMILY_MODERN:     xfamily = wxT("courier"); break;
        case wxFONTFAMILY_SWISS:      xfamily = wxT("helvetica"); break;
        case wxFONTFAMILY_TELETYPE:   xfamily = wxT("lucidatypewriter"); break;
        case wxFONTFAMILY_SCRIPT:     xfamily = wxT("utopia"); break;
    }
#if wxUSE_NANOX
    int xweight;
    switch (weight)
    {
         case wxFONTWEIGHT_BOLD:
             {
                 xweight = MWLF_WEIGHT_BOLD;
                 break;
             }
        case wxFONTWEIGHT_LIGHT:
             {
                 xweight = MWLF_WEIGHT_LIGHT;
                 break;
             }
         case wxFONTWEIGHT_NORMAL:
             {
                 xweight = MWLF_WEIGHT_NORMAL;
                 break;
             }

     default:
             {
                 xweight = MWLF_WEIGHT_DEFAULT;
                 break;
             }
    }
    GR_SCREEN_INFO screenInfo;
    GrGetScreenInfo(& screenInfo);

    int yPixelsPerCM = screenInfo.ydpcm;

                    
    int pixelHeight = (int) ( (((float)pointSize) / 720.0) * 2.541 * (float) yPixelsPerCM) ;

            
    GR_LOGFONT logFont;
    logFont.lfHeight = pixelHeight;
    logFont.lfWidth = 0;
    logFont.lfEscapement = 0;
    logFont.lfOrientation = 0;
    logFont.lfWeight = xweight;
    logFont.lfItalic = (style == wxFONTSTYLE_ITALIC ? 0 : 1) ;
    logFont.lfUnderline = 0;
    logFont.lfStrikeOut = 0;
    logFont.lfCharSet = MWLF_CHARSET_DEFAULT;     logFont.lfOutPrecision = MWLF_TYPE_DEFAULT;
    logFont.lfClipPrecision = 0;     logFont.lfRoman = (family == wxROMAN ? 1 : 0) ;
    logFont.lfSerif = (family == wxSWISS ? 0 : 1) ;
    logFont.lfSansSerif = !logFont.lfSerif ;
    logFont.lfModern = (family == wxMODERN ? 1 : 0) ;
    logFont.lfProportional = (family == wxTELETYPE ? 0 : 1) ;
    logFont.lfOblique = 0;
    logFont.lfSmallCaps = 0;
    logFont.lfPitch = 0;     strcpy(logFont.lfFaceName, facename.c_str());

    XFontStruct* fontInfo = (XFontStruct*) malloc(sizeof(XFontStruct));
    fontInfo->fid = GrCreateFont((GR_CHAR*) facename.c_str(), pixelHeight, & logFont);
    GrGetFontInfo(fontInfo->fid, & fontInfo->info);
    return (wxNativeFont) fontInfo;

#else
    wxString fontSpec;
    if (!facename.empty())
    {
        fontSpec.Printf(wxT("-*-%s-*-*-normal-*-*-*-*-*-*-*-*-*"),
                        facename.c_str());

        if ( wxTestFontSpec(fontSpec) )
        {
            xfamily = facename;
        }
            }

    wxString xstyle;
    switch (style)
    {
        case wxFONTSTYLE_SLANT:
            fontSpec.Printf(wxT("-*-%s-*-o-*-*-*-*-*-*-*-*-*-*"),
                    xfamily.c_str());
            if ( wxTestFontSpec(fontSpec) )
            {
                xstyle = wxT("o");
                break;
            }
            
        case wxFONTSTYLE_ITALIC:
            fontSpec.Printf(wxT("-*-%s-*-i-*-*-*-*-*-*-*-*-*-*"),
                    xfamily.c_str());
            if ( wxTestFontSpec(fontSpec) )
            {
                xstyle = wxT("i");
            }
            else if ( style == wxFONTSTYLE_ITALIC )             {
                                fontSpec.Printf(wxT("-*-%s-*-o-*-*-*-*-*-*-*-*-*-*"),
                        xfamily.c_str());
                if ( wxTestFontSpec(fontSpec) )
                {
                    xstyle = wxT("o");
                }
                else
                {
                                        xstyle = wxT("*");
                }
            }
            break;

        default:
            wxFAIL_MSG(wxT("unknown font style"));
            
        case wxFONTSTYLE_NORMAL:
            xstyle = wxT("r");
            break;
    }

    wxString xweight;
    switch (weight)
    {
         case wxFONTWEIGHT_BOLD:
             {
                  fontSpec.Printf(wxT("-*-%s-bold-*-*-*-*-*-*-*-*-*-*-*"),
                         xfamily.c_str());
                  if ( wxTestFontSpec(fontSpec) )
                  {
                       xweight = wxT("bold");
                       break;
                  }
                  fontSpec.Printf(wxT("-*-%s-heavy-*-*-*-*-*-*-*-*-*-*-*"),
                         xfamily.c_str());
                  if ( wxTestFontSpec(fontSpec) )
                  {
                       xweight = wxT("heavy");
                       break;
                  }
                  fontSpec.Printf(wxT("-*-%s-extrabold-*-*-*-*-*-*-*-*-*-*-*"),
                         xfamily.c_str());
                  if ( wxTestFontSpec(fontSpec) )
                  {
                      xweight = wxT("extrabold");
                      break;
                  }
                  fontSpec.Printf(wxT("-*-%s-demibold-*-*-*-*-*-*-*-*-*-*-*"),
                         xfamily.c_str());
                  if ( wxTestFontSpec(fontSpec) )
                  {
                      xweight = wxT("demibold");
                      break;
                  }
                  fontSpec.Printf(wxT("-*-%s-black-*-*-*-*-*-*-*-*-*-*-*"),
                         xfamily.c_str());
                  if ( wxTestFontSpec(fontSpec) )
                  {
                      xweight = wxT("black");
                      break;
                  }
                  fontSpec.Printf(wxT("-*-%s-ultrablack-*-*-*-*-*-*-*-*-*-*-*"),
                         xfamily.c_str());
                  if ( wxTestFontSpec(fontSpec) )
                  {
                      xweight = wxT("ultrablack");
                      break;
                  }
              }
              break;
        case wxFONTWEIGHT_LIGHT:
             {
                  fontSpec.Printf(wxT("-*-%s-light-*-*-*-*-*-*-*-*-*-*-*"),
                         xfamily.c_str());
                  if ( wxTestFontSpec(fontSpec) )
                  {
                       xweight = wxT("light");
                       break;
                  }
                  fontSpec.Printf(wxT("-*-%s-thin-*-*-*-*-*-*-*-*-*-*-*"),
                         xfamily.c_str());
                  if ( wxTestFontSpec(fontSpec) )
                  {
                       xweight = wxT("thin");
                       break;
                  }
             }
             break;
         case wxFONTWEIGHT_NORMAL:
             {
                  fontSpec.Printf(wxT("-*-%s-medium-*-*-*-*-*-*-*-*-*-*-*"),
                         xfamily.c_str());
                  if ( wxTestFontSpec(fontSpec) )
                  {
                       xweight = wxT("medium");
                       break;
                  }
                  fontSpec.Printf(wxT("-*-%s-normal-*-*-*-*-*-*-*-*-*-*-*"),
                         xfamily.c_str());
                  if ( wxTestFontSpec(fontSpec) )
                  {
                       xweight = wxT("normal");
                       break;
                  }
                  fontSpec.Printf(wxT("-*-%s-regular-*-*-*-*-*-*-*-*-*-*-*"),
                         xfamily.c_str());
                  if ( wxTestFontSpec(fontSpec) )
                  {
                      xweight = wxT("regular");
                      break;
                  }
                  xweight = wxT("*");
              }
              break;
        default:           xweight = wxT("*"); break;
    }

        wxString sizeSpec;
    if ( pointSize == -1 )
    {
        sizeSpec = wxT('*');
    }
    else
    {
        sizeSpec.Printf(wxT("%d"), pointSize);
    }

        fontSpec.Printf(wxT("-*-%s-%s-%s-normal-*-*-%s-*-*-*-*-%s-%s"),
                    xfamily.c_str(), xweight.c_str(), xstyle.c_str(),
                    sizeSpec.c_str(), xregistry.c_str(), xencoding.c_str());

    if( xFontName )
        *xFontName = fontSpec;

    return wxLoadFont(fontSpec);
#endif
    }


class wxFontModule : public wxModule
{
public:
    bool OnInit();
    void OnExit();

private:
    wxDECLARE_DYNAMIC_CLASS(wxFontModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxFontModule, wxModule);

bool wxFontModule::OnInit()
{
    g_fontHash = new wxHashTable( wxKEY_STRING );

    return true;
}

void wxFontModule::OnExit()
{
    wxDELETE(g_fontHash);
}

#endif 