


#include "wx/wxprec.h"

#if wxUSE_FONTENUM

#include "wx/fontenum.h"

#ifndef WX_PRECOMP
    #include "wx/dynarray.h"
    #include "wx/string.h"
    #include "wx/app.h"
    #include "wx/utils.h"
#endif

#include "wx/regex.h"
#include "wx/fontmap.h"
#include "wx/fontutil.h"
#include "wx/encinfo.h"


#if wxUSE_PANGO

#include <pango/pango.h>

PangoContext* wxGetPangoContext();

extern "C"
{
static int wxCMPFUNC_CONV
wxCompareFamilies (const void *a, const void *b)
{
  const char *a_name = pango_font_family_get_name (*(PangoFontFamily **)a);
  const char *b_name = pango_font_family_get_name (*(PangoFontFamily **)b);

  return g_utf8_collate (a_name, b_name);
}
}

bool wxFontEnumerator::EnumerateFacenames(wxFontEncoding encoding,
                                          bool fixedWidthOnly)
{
    if ( encoding != wxFONTENCODING_SYSTEM && encoding != wxFONTENCODING_UTF8 )
    {
                        return false;
    }

    PangoFontFamily **families = NULL;
    gint n_families = 0;
    PangoContext* context = wxGetPangoContext();
    pango_context_list_families(context, &families, &n_families);
    qsort (families, n_families, sizeof (PangoFontFamily *), wxCompareFamilies);

    for ( int i = 0; i < n_families; i++ )
    {
#if defined(__WXGTK20__) || defined(HAVE_PANGO_FONT_FAMILY_IS_MONOSPACE)
        if ( !fixedWidthOnly ||
            pango_font_family_is_monospace(families[i]) )
#endif
        {
            const gchar *name = pango_font_family_get_name(families[i]);
            OnFacename(wxString(name, wxConvUTF8));
        }
    }
    g_free(families);
    g_object_unref(context);

    return true;
}

bool wxFontEnumerator::EnumerateEncodings(const wxString& facename)
{
    return EnumerateEncodingsUTF8(facename);
}


#else 
#ifdef __VMS__                #pragma message disable nosimpint
#endif
#include <X11/Xlib.h>
#ifdef __VMS__
#pragma message enable nosimpint
#endif


static char **CreateFontList(wxChar spacing, wxFontEncoding encoding,
                             int *nFonts);

static bool ProcessFamiliesFromFontList(wxFontEnumerator *This,
                                        char **fonts,
                                        int nFonts);





#if !wxUSE_NANOX
static char **CreateFontList(wxChar spacing,
                             wxFontEncoding encoding,
                             int *nFonts)
{
    wxNativeEncodingInfo info;
    wxGetNativeFontEncoding(encoding, &info);

#if wxUSE_FONTMAP
    if ( !wxTestFontEncoding(info) )
    {
                (void)wxFontMapper::Get()->GetAltForEncoding(encoding, &info);
    }
#endif 
    wxString pattern;
    pattern.Printf(wxT("-*-*-*-*-*-*-*-*-*-*-%c-*-%s-%s"),
                   spacing,
                   info.xregistry.c_str(),
                   info.xencoding.c_str());

        return XListFonts((Display *)wxGetDisplay(), pattern.mb_str(), 32767, nFonts);
}

static bool ProcessFamiliesFromFontList(wxFontEnumerator *This,
                                        char **fonts,
                                        int nFonts)
{
#if wxUSE_REGEX
    wxRegEx re(wxT("^(-[^-]*){14}$"), wxRE_NOSUB);
#endif 
        wxSortedArrayString families;
    for ( int n = 0; n < nFonts; n++ )
    {
        char *font = fonts[n];
#if wxUSE_REGEX
        if ( !re.Matches(font) )
#else         if ( !wxString(font).Matches(wxT("-*-*-*-*-*-*-*-*-*-*-*-*-*-*")) )
#endif         {
                        continue;
        }

                char *dash = strchr(font + 1, '-');
        char *family = dash + 1;
        dash = strchr(family, '-');
        *dash = '\0';         wxString fam(family);

        if ( families.Index(fam) == wxNOT_FOUND )
        {
            if ( !This->OnFacename(fam) )
            {
                                return false;
            }

            families.Add(fam);
        }
            }

    return true;
}
#endif
  

bool wxFontEnumerator::EnumerateFacenames(wxFontEncoding encoding,
                                          bool fixedWidthOnly)
{
#if wxUSE_NANOX
    return false;
#else
    int nFonts;
    char **fonts;

    if ( fixedWidthOnly )
    {
        bool cont = true;
        fonts = CreateFontList(wxT('m'), encoding, &nFonts);
        if ( fonts )
        {
            cont = ProcessFamiliesFromFontList(this, fonts, nFonts);

            XFreeFontNames(fonts);
        }

        if ( !cont )
        {
            return true;
        }

        fonts = CreateFontList(wxT('c'), encoding, &nFonts);
        if ( !fonts )
        {
            return true;
        }
    }
    else
    {
        fonts = CreateFontList(wxT('*'), encoding, &nFonts);

        if ( !fonts )
        {
                                    wxASSERT_MSG(encoding != wxFONTENCODING_SYSTEM,
                         wxT("No fonts at all on this system?"));

            return false;
        }
    }

    (void)ProcessFamiliesFromFontList(this, fonts, nFonts);

    XFreeFontNames(fonts);
    return true;
#endif
    }

bool wxFontEnumerator::EnumerateEncodings(const wxString& family)
{
#if wxUSE_NANOX
    return false;
#else
    wxString pattern;
    pattern.Printf(wxT("-*-%s-*-*-*-*-*-*-*-*-*-*-*-*"),
                   family.empty() ? wxT("*") : family.c_str());

        int nFonts;
    char **fonts = XListFonts((Display *)wxGetDisplay(), pattern.mb_str(),
                              32767, &nFonts);

    if ( !fonts )
    {
                return false;
    }

        wxSortedArrayString encodings;
    for ( int n = 0; n < nFonts; n++ )
    {
        char *font = fonts[n];
        if ( !wxString(font).Matches(wxT("-*-*-*-*-*-*-*-*-*-*-*-*-*-*")) )
        {
                        continue;
        }

                char *dash = strchr(font + 1, '-');
        char *familyFont = dash + 1;
        dash = strchr(familyFont, '-');
        *dash = '\0'; 
        if ( !family.empty() && (family != familyFont) )
        {
                        continue;
        }

                char *p = dash + 1;         dash = strrchr(p, '-');

        wxString registry(dash + 1);
        *dash = '\0';

        dash = strrchr(p, '-');
        wxString encoding(dash + 1);

        encoding << wxT('-') << registry;
        if ( encodings.Index(encoding) == wxNOT_FOUND )
        {
            if ( !OnFontEncoding(familyFont, encoding) )
            {
                break;
            }

            encodings.Add(encoding);
        }
            }

    XFreeFontNames(fonts);

    return true;
#endif
    }

#endif 
#endif 