


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_XLOCALE

#ifndef WX_PRECOMP
    #include "wx/module.h"
#endif

#include "wx/xlocale.h"

#include <errno.h>
#include <locale.h>


static wxXLocale *gs_cLocale = NULL;

wxXLocale wxNullXLocale;




class wxXLocaleModule : public wxModule
{
public:
    virtual bool OnInit() wxOVERRIDE { return true; }
    virtual void OnExit() wxOVERRIDE { wxDELETE(gs_cLocale); }

    wxDECLARE_DYNAMIC_CLASS(wxXLocaleModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxXLocaleModule, wxModule);




wxXLocale& wxXLocale::GetCLocale()
{
    if ( !gs_cLocale )
    {
                        static wxXLocaleCTag* const tag = NULL;
        gs_cLocale = new wxXLocale(tag);
    }

    return *gs_cLocale;
}

#ifdef wxHAS_XLOCALE_SUPPORT

#if wxUSE_INTL
wxXLocale::wxXLocale(wxLanguage lang)
{
    const wxLanguageInfo * const info = wxLocale::GetLanguageInfo(lang);
    if ( !info )
    {
        m_locale = NULL;
    }
    else
    {
        Init(info->GetLocaleName().c_str());
    }
}
#endif 
#if wxCHECK_VISUALC_VERSION(8)


void wxXLocale::Init(const char *loc)
{
    if (!loc || *loc == '\0')
        return;

    m_locale = _create_locale(LC_ALL, loc);
}

void wxXLocale::Free()
{
    if ( m_locale )
        _free_locale(m_locale);
}

#elif defined(HAVE_LOCALE_T)


void wxXLocale::Init(const char *loc)
{
    if (!loc || *loc == '\0')
        return;

    m_locale = newlocale(LC_ALL_MASK, loc, NULL);
    if (!m_locale)
    {
                        wxString buf(loc);
        wxString buf2;
        buf2 = buf + wxS(".UTF-8");
        m_locale = newlocale(LC_ALL_MASK, buf2.c_str(), NULL);
        if ( !m_locale )
        {
            buf2 = buf + wxS(".utf-8");
            m_locale = newlocale(LC_ALL_MASK, buf2.c_str(), NULL);
        }
        if ( !m_locale )
        {
            buf2 = buf + wxS(".UTF8");
            m_locale = newlocale(LC_ALL_MASK, buf2.c_str(), NULL);
        }
        if ( !m_locale )
        {
            buf2 = buf + wxS(".utf8");
            m_locale = newlocale(LC_ALL_MASK, buf2.c_str(), NULL);
        }
    }

            }

void wxXLocale::Free()
{
    if ( m_locale )
        freelocale(m_locale);
}

#else
    #error "Unknown xlocale support."
#endif

#endif 
#ifndef wxHAS_XLOCALE_SUPPORT



#define CTYPE_ALNUM 0x0001
#define CTYPE_ALPHA 0x0002
#define CTYPE_CNTRL 0x0004
#define CTYPE_DIGIT 0x0008
#define CTYPE_GRAPH 0x0010
#define CTYPE_LOWER 0x0020
#define CTYPE_PRINT 0x0040
#define CTYPE_PUNCT 0x0080
#define CTYPE_SPACE 0x0100
#define CTYPE_UPPER 0x0200
#define CTYPE_XDIGIT 0x0400

static const unsigned int gs_lookup[] =
{
    0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004,
    0x0004, 0x0104, 0x0104, 0x0104, 0x0104, 0x0104, 0x0004, 0x0004,
    0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004,
    0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004,
    0x0140, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0,
    0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0,
    0x0459, 0x0459, 0x0459, 0x0459, 0x0459, 0x0459, 0x0459, 0x0459,
    0x0459, 0x0459, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0,
    0x00D0, 0x0653, 0x0653, 0x0653, 0x0653, 0x0653, 0x0653, 0x0253,
    0x0253, 0x0253, 0x0253, 0x0253, 0x0253, 0x0253, 0x0253, 0x0253,
    0x0253, 0x0253, 0x0253, 0x0253, 0x0253, 0x0253, 0x0253, 0x0253,
    0x0253, 0x0253, 0x0253, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0,
    0x00D0, 0x0473, 0x0473, 0x0473, 0x0473, 0x0473, 0x0473, 0x0073,
    0x0073, 0x0073, 0x0073, 0x0073, 0x0073, 0x0073, 0x0073, 0x0073,
    0x0073, 0x0073, 0x0073, 0x0073, 0x0073, 0x0073, 0x0073, 0x0073,
    0x0073, 0x0073, 0x0073, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x0004
};


#define CTYPE_TEST(c, t) ( (c) <= 127 && (gs_lookup[(c)] & (t)) )


#define GEN_ISFUNC(name, test) \
int name(const wxUniChar& c, const wxXLocale& loc) \
{ \
    wxCHECK(loc.IsOk(), false); \
    return CTYPE_TEST(c.GetValue(), test); \
}

GEN_ISFUNC(wxIsalnum_l, CTYPE_ALNUM)
GEN_ISFUNC(wxIsalpha_l, CTYPE_ALPHA)
GEN_ISFUNC(wxIscntrl_l, CTYPE_CNTRL)
GEN_ISFUNC(wxIsdigit_l, CTYPE_DIGIT)
GEN_ISFUNC(wxIsgraph_l, CTYPE_GRAPH)
GEN_ISFUNC(wxIslower_l, CTYPE_LOWER)
GEN_ISFUNC(wxIsprint_l, CTYPE_PRINT)
GEN_ISFUNC(wxIspunct_l, CTYPE_PUNCT)
GEN_ISFUNC(wxIsspace_l, CTYPE_SPACE)
GEN_ISFUNC(wxIsupper_l, CTYPE_UPPER)
GEN_ISFUNC(wxIsxdigit_l, CTYPE_XDIGIT)

int wxTolower_l(const wxUniChar& c, const wxXLocale& loc)
{
    wxCHECK(loc.IsOk(), false);

    if(CTYPE_TEST(c.GetValue(), CTYPE_UPPER))
    {
        return c - 'A' + 'a';
    }

    return c;
}

int wxToupper_l(const wxUniChar& c, const wxXLocale& loc)
{
    wxCHECK(loc.IsOk(), false);

    if(CTYPE_TEST(c.GetValue(), CTYPE_LOWER))
    {
        return c - 'a' + 'A';
    }

    return c;
}





namespace
{

class CNumericLocaleSetter
{
public:
    CNumericLocaleSetter()
        : m_oldLocale(wxStrdupA(setlocale(LC_NUMERIC, NULL)))
    {
        if ( !wxSetlocale(LC_NUMERIC, "C") )
        {
                        wxFAIL_MSG( wxS("Couldn't set LC_NUMERIC to \"C\"") );
        }
    }

    ~CNumericLocaleSetter()
    {
        wxSetlocale(LC_NUMERIC, m_oldLocale);
        free(m_oldLocale);
    }

private:
    char * const m_oldLocale;

    wxDECLARE_NO_COPY_CLASS(CNumericLocaleSetter);
};

} 
double wxStrtod_l(const wchar_t* str, wchar_t **endptr, const wxXLocale& loc)
{
    wxCHECK( loc.IsOk(), 0. );

    CNumericLocaleSetter locSetter;

    return wxStrtod(str, endptr);
}

double wxStrtod_l(const char* str, char **endptr, const wxXLocale& loc)
{
    wxCHECK( loc.IsOk(), 0. );

    CNumericLocaleSetter locSetter;

    return wxStrtod(str, endptr);
}

long wxStrtol_l(const wchar_t* str, wchar_t **endptr, int base, const wxXLocale& loc)
{
    wxCHECK( loc.IsOk(), 0 );

    CNumericLocaleSetter locSetter;

    return wxStrtol(str, endptr, base);
}

long wxStrtol_l(const char* str, char **endptr, int base, const wxXLocale& loc)
{
    wxCHECK( loc.IsOk(), 0 );

    CNumericLocaleSetter locSetter;

    return wxStrtol(str, endptr, base);
}

unsigned long wxStrtoul_l(const wchar_t* str, wchar_t **endptr, int base, const wxXLocale& loc)
{
    wxCHECK( loc.IsOk(), 0 );

    CNumericLocaleSetter locSetter;

    return wxStrtoul(str, endptr, base);
}

unsigned long wxStrtoul_l(const char* str, char **endptr, int base, const wxXLocale& loc)
{
    wxCHECK( loc.IsOk(), 0 );

    CNumericLocaleSetter locSetter;

    return wxStrtoul(str, endptr, base);
}

#endif 
#endif 