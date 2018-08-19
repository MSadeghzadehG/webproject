


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_INTL

#ifndef WX_PRECOMP
    #include "wx/dynarray.h"
    #include "wx/string.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/utils.h"
    #include "wx/app.h"
    #include "wx/hashmap.h"
    #include "wx/module.h"
#endif 
#include <locale.h>

#include <ctype.h>
#include <stdlib.h>
#ifdef HAVE_LANGINFO_H
    #include <langinfo.h>
#endif

#ifdef __WIN32__
    #include "wx/dynlib.h"
    #include "wx/msw/private.h"
#endif

#include "wx/file.h"
#include "wx/filename.h"
#include "wx/tokenzr.h"
#include "wx/fontmap.h"
#include "wx/scopedptr.h"
#include "wx/apptrait.h"
#include "wx/stdpaths.h"
#include "wx/hashset.h"

#if defined(__WXOSX__)
    #include "wx/osx/core/cfref.h"
    #include <CoreFoundation/CFLocale.h>
    #include <CoreFoundation/CFDateFormatter.h>
    #include "wx/osx/core/cfstring.h"
#endif


#define TRACE_I18N wxS("i18n")



static wxLocale *wxSetLocale(wxLocale *pLocale);

namespace
{

inline wxString ExtractLang(const wxString& langFull)
{
    return langFull.BeforeFirst('_');
}

#ifdef __UNIX__

inline wxString ExtractNotLang(const wxString& langFull)
{
    size_t pos = langFull.find('_');
    if ( pos != wxString::npos )
        return langFull.substr(pos);
    else
        return wxString();
}

#endif 
} 

#ifdef __WINDOWS__

static wxString wxGetANSICodePageForLocale(LCID lcid)
{
    wxString cp;

    wxChar buffer[16];
    if ( ::GetLocaleInfo(lcid, LOCALE_IDEFAULTANSICODEPAGE,
                        buffer, WXSIZEOF(buffer)) > 0 )
    {
        if ( buffer[0] != wxT('0') || buffer[1] != wxT('\0') )
            cp = buffer;
            }

    return cp;
}

wxUint32 wxLanguageInfo::GetLCID() const
{
    return MAKELCID(MAKELANGID(WinLang, WinSublang), SORT_DEFAULT);
}

wxString wxLanguageInfo::GetLocaleName() const
{
    wxString locale;

    const LCID lcid = GetLCID();

    wxChar buffer[256];
    buffer[0] = wxT('\0');
    if ( !::GetLocaleInfo(lcid, LOCALE_SENGLANGUAGE, buffer, WXSIZEOF(buffer)) )
    {
        wxLogLastError(wxT("GetLocaleInfo(LOCALE_SENGLANGUAGE)"));
        return locale;
    }

    locale << buffer;
    if ( ::GetLocaleInfo(lcid, LOCALE_SENGCOUNTRY,
                        buffer, WXSIZEOF(buffer)) > 0 )
    {
        locale << wxT('_') << buffer;
    }

    const wxString cp = wxGetANSICodePageForLocale(lcid);
    if ( !cp.empty() )
    {
        locale << wxT('.') << cp;
    }

    return locale;
}

#endif 

#include "wx/arrimpl.cpp"
WX_DECLARE_USER_EXPORTED_OBJARRAY(wxLanguageInfo, wxLanguageInfoArray, WXDLLIMPEXP_BASE);
WX_DEFINE_OBJARRAY(wxLanguageInfoArray)

wxLanguageInfoArray *wxLocale::ms_languagesDB = NULL;

 void wxLocale::CreateLanguagesDB()
{
    if (ms_languagesDB == NULL)
    {
        ms_languagesDB = new wxLanguageInfoArray;
        InitLanguagesDB();
    }
}

 void wxLocale::DestroyLanguagesDB()
{
    wxDELETE(ms_languagesDB);
}


void wxLocale::DoCommonInit()
{
        m_pszOldLocale = wxSetlocale(LC_ALL, NULL);
    if ( m_pszOldLocale )
        m_pszOldLocale = wxStrdup(m_pszOldLocale);


    m_pOldLocale = wxSetLocale(this);

                                                wxTranslations *oldTrans = wxTranslations::Get();
    if ( !oldTrans ||
         (m_pOldLocale && oldTrans == &m_pOldLocale->m_translations) )
    {
        wxTranslations::SetNonOwned(&m_translations);
    }

    m_language = wxLANGUAGE_UNKNOWN;
    m_initialized = false;
}

bool wxLocale::Init(const wxString& name,
                    const wxString& shortName,
                    const wxString& locale,
                    bool            bLoadDefault
#if WXWIN_COMPATIBILITY_2_8
                   ,bool            WXUNUSED_UNLESS_DEBUG(bConvertEncoding)
#endif
                    )
{
#if WXWIN_COMPATIBILITY_2_8
    wxASSERT_MSG( bConvertEncoding,
                  wxS("wxLocale::Init with bConvertEncoding=false is no longer supported, add charset to your catalogs") );
#endif

    bool ret = DoInit(name, shortName, locale);

        wxTranslations *t = wxTranslations::Get();
    if ( t )
    {
        t->SetLanguage(shortName);

        if ( bLoadDefault )
            t->AddStdCatalog();
    }

    return ret;
}

bool wxLocale::DoInit(const wxString& name,
                      const wxString& shortName,
                      const wxString& locale)
{
    wxASSERT_MSG( !m_initialized,
                    wxS("you can't call wxLocale::Init more than once") );

    m_initialized = true;
    m_strLocale = name;
    m_strShort = shortName;
    m_language = wxLANGUAGE_UNKNOWN;

        wxString szLocale(locale);
    if ( szLocale.empty() )
    {
                szLocale = shortName;

        wxCHECK_MSG( !szLocale.empty(), false,
                    wxS("no locale to set in wxLocale::Init()") );
    }

    if ( !wxSetlocale(LC_ALL, szLocale) )
    {
        wxLogError(_("locale '%s' cannot be set."), szLocale);
    }

            if ( m_strShort.empty() ) {
                        if ( !szLocale.empty() )
        {
            m_strShort += (wxChar)wxTolower(szLocale[0]);
            if ( szLocale.length() > 1 )
                m_strShort += (wxChar)wxTolower(szLocale[1]);
        }
    }

    return true;
}


#if defined(__UNIX__) && wxUSE_UNICODE && !defined(__WXMAC__)
static const char *wxSetlocaleTryUTF8(int c, const wxString& lc)
{
    const char *l = NULL;

        
    if ( !lc.empty() )
    {
        wxString buf(lc);
        wxString buf2;
        buf2 = buf + wxS(".UTF-8");
        l = wxSetlocale(c, buf2);
        if ( !l )
        {
            buf2 = buf + wxS(".utf-8");
            l = wxSetlocale(c, buf2);
        }
        if ( !l )
        {
            buf2 = buf + wxS(".UTF8");
            l = wxSetlocale(c, buf2);
        }
        if ( !l )
        {
            buf2 = buf + wxS(".utf8");
            l = wxSetlocale(c, buf2);
        }
    }

        if ( !l )
        l = wxSetlocale(c, lc);

    return l;
}
#else
#define wxSetlocaleTryUTF8(c, lc)  wxSetlocale(c, lc)
#endif

bool wxLocale::Init(int language, int flags)
{
#if WXWIN_COMPATIBILITY_2_8
    wxASSERT_MSG( !(flags & wxLOCALE_CONV_ENCODING),
                  wxS("wxLOCALE_CONV_ENCODING is no longer supported, add charset to your catalogs") );
#endif

    bool ret = true;

    int lang = language;
    if (lang == wxLANGUAGE_DEFAULT)
    {
                lang = GetSystemLanguage();
    }

        if (lang == wxLANGUAGE_UNKNOWN)
    {
        return false;
    }

    const wxLanguageInfo *info = GetLanguageInfo(lang);

        if (info == NULL)
    {
        wxLogError(wxS("Unknown language %i."), lang);
        return false;
    }

    wxString name = info->Description;
    wxString canonical = info->CanonicalName;
    wxString locale;

    #if defined(__UNIX__) && !defined(__WXMAC__)
    if (language != wxLANGUAGE_DEFAULT)
        locale = info->CanonicalName;

    const char *retloc = wxSetlocaleTryUTF8(LC_ALL, locale);

    const wxString langOnly = ExtractLang(locale);
    if ( !retloc )
    {
                retloc = wxSetlocaleTryUTF8(LC_ALL, langOnly);
    }

#if wxUSE_FONTMAP
                            if ( !retloc )
    {
        const wxChar **names =
            wxFontMapperBase::GetAllEncodingNames(wxFONTENCODING_UTF8);
        while ( *names )
        {
            retloc = wxSetlocale(LC_ALL, locale + wxS('.') + *names++);
            if ( retloc )
                break;
        }
    }
#endif 
    if ( !retloc )
    {
                        wxString localeAlt;
        if ( langOnly == wxS("he") )
            localeAlt = wxS("iw") + ExtractNotLang(locale);
        else if ( langOnly == wxS("id") )
            localeAlt = wxS("in") + ExtractNotLang(locale);
        else if ( langOnly == wxS("yi") )
            localeAlt = wxS("ji") + ExtractNotLang(locale);
        else if ( langOnly == wxS("nb") )
            localeAlt = wxS("no_NO");
        else if ( langOnly == wxS("nn") )
            localeAlt = wxS("no_NY");

        if ( !localeAlt.empty() )
        {
            retloc = wxSetlocaleTryUTF8(LC_ALL, localeAlt);
            if ( !retloc )
                retloc = wxSetlocaleTryUTF8(LC_ALL, ExtractLang(localeAlt));
        }
    }

    if ( !retloc )
        ret = false;

#ifdef __AIX__
                                char* p = const_cast<char*>(wxStrchr(retloc, ' '));
    if ( p )
        *p = '\0';
#endif 
#elif defined(__WIN32__)
    const char *retloc = "C";
    if ( language != wxLANGUAGE_DEFAULT )
    {
        if ( info->WinLang == 0 )
        {
            wxLogWarning(wxS("Locale '%s' not supported by OS."), name.c_str());
                    }
        else         {
            const wxUint32 lcid = info->GetLCID();

                        ::SetThreadLocale(lcid);

                                    if ( wxGetWinVersion() >= wxWinVersion_Vista )
            {
                wxLoadedDLL dllKernel32(wxS("kernel32.dll"));
                typedef LANGID(WINAPI *SetThreadUILanguage_t)(LANGID);
                SetThreadUILanguage_t pfnSetThreadUILanguage = NULL;
                wxDL_INIT_FUNC(pfn, SetThreadUILanguage, dllKernel32);
                if (pfnSetThreadUILanguage)
                    pfnSetThreadUILanguage(LANGIDFROMLCID(lcid));
            }

                        locale = info->GetLocaleName();
            if ( locale.empty() )
            {
                ret = false;
            }
            else             {
                retloc = wxSetlocale(LC_ALL, locale);
            }
        }
    }
    else     {
        retloc = wxSetlocale(LC_ALL, wxEmptyString);
    }

#if wxUSE_UNICODE && (defined(__VISUALC__) || defined(__MINGW32__))
                    if ( !retloc )
    {
        if ( wxGetANSICodePageForLocale(LOCALE_USER_DEFAULT).empty() )
        {
                                    retloc = "C";
        }
    }
#endif 
    if ( !retloc )
        ret = false;
#elif defined(__WXMAC__)
    if (lang == wxLANGUAGE_DEFAULT)
        locale = wxEmptyString;
    else
        locale = info->CanonicalName;

    const char *retloc = wxSetlocale(LC_ALL, locale);

    if ( !retloc )
    {
                retloc = wxSetlocale(LC_ALL, ExtractLang(locale));
    }
#else
    wxUnusedVar(flags);
    return false;
    #define WX_NO_LOCALE_SUPPORT
#endif

#ifndef WX_NO_LOCALE_SUPPORT
    if ( !ret )
    {
        wxLogWarning(_("Cannot set locale to language \"%s\"."), name.c_str());

                        free(const_cast<char *>(m_pszOldLocale));
        m_pszOldLocale = NULL;

                    }

    if ( !DoInit(name, canonical, retloc) )
    {
        ret = false;
    }

    if (IsOk())         m_language = lang;

        wxTranslations *t = wxTranslations::Get();
    if ( t )
    {
        t->SetLanguage(static_cast<wxLanguage>(language));

        if ( flags & wxLOCALE_LOAD_DEFAULT )
            t->AddStdCatalog();
    }

    return ret;
#endif }

namespace
{

#ifndef __WXOSX__
inline bool wxGetNonEmptyEnvVar(const wxString& name, wxString* value)
{
    return wxGetEnv(name, value) && !value->empty();
}
#endif

} 
 int wxLocale::GetSystemLanguage()
{
    CreateLanguagesDB();

        size_t i = 0,
        count = ms_languagesDB->GetCount();

#if defined(__UNIX__)
        wxString langFull;
#ifdef __WXOSX__
    wxCFRef<CFLocaleRef> userLocaleRef(CFLocaleCopyCurrent());

        
    wxCFStringRef str(wxCFRetain((CFStringRef)CFLocaleGetValue(userLocaleRef, kCFLocaleLanguageCode)));
    langFull = str.AsString()+"_";
    str.reset(wxCFRetain((CFStringRef)CFLocaleGetValue(userLocaleRef, kCFLocaleCountryCode)));
    langFull += str.AsString();
#else
    if (!wxGetNonEmptyEnvVar(wxS("LC_ALL"), &langFull) &&
        !wxGetNonEmptyEnvVar(wxS("LC_MESSAGES"), &langFull) &&
        !wxGetNonEmptyEnvVar(wxS("LANG"), &langFull))
    {
                return wxLANGUAGE_ENGLISH_US;
    }

    if ( langFull == wxS("C") || langFull == wxS("POSIX") )
    {
                return wxLANGUAGE_ENGLISH_US;
    }
#endif

                                                                
                        wxString modifier;
    size_t posModifier = langFull.find_first_of(wxS("@"));
    if ( posModifier != wxString::npos )
        modifier = langFull.Mid(posModifier);

    size_t posEndLang = langFull.find_first_of(wxS("@."));
    if ( posEndLang != wxString::npos )
    {
        langFull.Truncate(posEndLang);
    }

        const bool justLang = langFull.find('_') == wxString::npos;

            
        wxString langOrig = ExtractLang(langFull);

    wxString lang;
    if ( langOrig == wxS("iw"))
        lang = wxS("he");
    else if (langOrig == wxS("in"))
        lang = wxS("id");
    else if (langOrig == wxS("ji"))
        lang = wxS("yi");
    else if (langOrig == wxS("no_NO"))
        lang = wxS("nb_NO");
    else if (langOrig == wxS("no_NY"))
        lang = wxS("nn_NO");
    else if (langOrig == wxS("no"))
        lang = wxS("nb_NO");
    else
        lang = langOrig;

        if ( lang != langOrig )
    {
        langFull = lang + ExtractNotLang(langFull);
    }

            if ( !modifier.empty() )
    {
        wxString langFullWithModifier = langFull + modifier;
        for ( i = 0; i < count; i++ )
        {
            if ( ms_languagesDB->Item(i).CanonicalName == langFullWithModifier )
                break;
        }
    }

        if ( modifier.empty() || i == count )
    {
        for ( i = 0; i < count; i++ )
        {
            if ( ms_languagesDB->Item(i).CanonicalName == langFull )
                break;
        }
    }

        if ( i == count && !justLang )
    {
        for ( i = 0; i < count; i++ )
        {
            if ( ms_languagesDB->Item(i).CanonicalName == lang )
            {
                break;
            }
        }
    }

        if ( i == count && justLang )
    {
        for ( i = 0; i < count; i++ )
        {
            if ( ExtractLang(ms_languagesDB->Item(i).CanonicalName)
                    == langFull )
            {
                break;
            }
        }
    }


    if ( i == count )
    {
                                        for ( i = 0; i < count; i++ )
        {
            if (ms_languagesDB->Item(i).Description.CmpNoCase(langFull) == 0)
            {
                break;
            }
        }
    }
#elif defined(__WIN32__)
    LCID lcid = GetUserDefaultLCID();
    if ( lcid != 0 )
    {
        wxUint32 lang = PRIMARYLANGID(LANGIDFROMLCID(lcid));
        wxUint32 sublang = SUBLANGID(LANGIDFROMLCID(lcid));

        for ( i = 0; i < count; i++ )
        {
            if (ms_languagesDB->Item(i).WinLang == lang &&
                ms_languagesDB->Item(i).WinSublang == sublang)
            {
                break;
            }
        }
    }
    #endif 
    if ( i < count )
    {
                return ms_languagesDB->Item(i).Language;
    }

        return wxLANGUAGE_UNKNOWN;
}




wxString wxLocale::GetSystemEncodingName()
{
    wxString encname;

#if defined(__WIN32__)
        UINT codepage = ::GetACP();
    encname.Printf(wxS("windows-%u"), codepage);
#elif defined(__WXMAC__)
    encname = wxCFStringRef::AsString(
                CFStringGetNameOfEncoding(CFStringGetSystemEncoding())
              );
#elif defined(__UNIX_LIKE__)

#if defined(HAVE_LANGINFO_H) && defined(CODESET)
            char *oldLocale = strdup(setlocale(LC_CTYPE, NULL));
    setlocale(LC_CTYPE, "");
    const char *alang = nl_langinfo(CODESET);
    setlocale(LC_CTYPE, oldLocale);
    free(oldLocale);

    if ( alang )
    {
        encname = wxString::FromAscii( alang );
    }
    else #endif     {
                                char *lang = getenv( "LC_ALL");
        char *dot = lang ? strchr(lang, '.') : NULL;
        if (!dot)
        {
            lang = getenv( "LC_CTYPE" );
            if ( lang )
                dot = strchr(lang, '.' );
        }
        if (!dot)
        {
            lang = getenv( "LANG");
            if ( lang )
                dot = strchr(lang, '.');
        }

        if ( dot )
        {
            encname = wxString::FromAscii( dot+1 );
        }
    }
#endif 
    return encname;
}


wxFontEncoding wxLocale::GetSystemEncoding()
{
#if defined(__WIN32__)
    UINT codepage = ::GetACP();

        if ( codepage >= 1250 && codepage <= 1257 )
    {
        return (wxFontEncoding)(wxFONTENCODING_CP1250 + codepage - 1250);
    }

    if ( codepage == 874 )
    {
        return wxFONTENCODING_CP874;
    }

    if ( codepage == 932 )
    {
        return wxFONTENCODING_CP932;
    }

    if ( codepage == 936 )
    {
        return wxFONTENCODING_CP936;
    }

    if ( codepage == 949 )
    {
        return wxFONTENCODING_CP949;
    }

    if ( codepage == 950 )
    {
        return wxFONTENCODING_CP950;
    }
#elif defined(__WXMAC__)
    CFStringEncoding encoding = 0 ;
    encoding = CFStringGetSystemEncoding() ;
    return wxMacGetFontEncFromSystemEnc( encoding ) ;
#elif defined(__UNIX_LIKE__) && wxUSE_FONTMAP
    const wxString encname = GetSystemEncodingName();
    if ( !encname.empty() )
    {
        wxFontEncoding enc = wxFontMapperBase::GetEncodingFromName(encname);

                        #if !wxUSE_UNICODE && \
        ((defined(__WXGTK__) && !defined(__WXGTK20__)) || defined(__WXMOTIF__))
        if ( enc == wxFONTENCODING_UTF8 )
        {
                        enc = wxFONTENCODING_ISO8859_1;
        }
#endif 
                                        if ( enc == wxFONTENCODING_DEFAULT )
        {
                        return wxFONTENCODING_ISO8859_1;
        }

        if ( enc != wxFONTENCODING_MAX )
        {
            return enc;
        }
            }
#endif 
    return wxFONTENCODING_SYSTEM;
}


void wxLocale::AddLanguage(const wxLanguageInfo& info)
{
    CreateLanguagesDB();
    ms_languagesDB->Add(info);
}


const wxLanguageInfo *wxLocale::GetLanguageInfo(int lang)
{
    CreateLanguagesDB();

            if ( lang == wxLANGUAGE_DEFAULT )
        lang = GetSystemLanguage();

    const size_t count = ms_languagesDB->GetCount();
    for ( size_t i = 0; i < count; i++ )
    {
        if ( ms_languagesDB->Item(i).Language == lang )
        {
                        wxLanguageInfo *ptr = &ms_languagesDB->Item(i);
            return ptr;
        }
    }

    return NULL;
}


wxString wxLocale::GetLanguageName(int lang)
{
    if ( lang == wxLANGUAGE_DEFAULT || lang == wxLANGUAGE_UNKNOWN )
        return wxEmptyString;

    const wxLanguageInfo *info = GetLanguageInfo(lang);
    if ( !info )
        return wxEmptyString;
    else
        return info->Description;
}


wxString wxLocale::GetLanguageCanonicalName(int lang)
{
    if ( lang == wxLANGUAGE_DEFAULT || lang == wxLANGUAGE_UNKNOWN )
        return wxEmptyString;

    const wxLanguageInfo *info = GetLanguageInfo(lang);
    if ( !info )
        return wxEmptyString;
    else
        return info->CanonicalName;
}


const wxLanguageInfo *wxLocale::FindLanguageInfo(const wxString& locale)
{
    CreateLanguagesDB();

    const wxLanguageInfo *infoRet = NULL;

    const size_t count = ms_languagesDB->GetCount();
    for ( size_t i = 0; i < count; i++ )
    {
        const wxLanguageInfo *info = &ms_languagesDB->Item(i);

        if ( wxStricmp(locale, info->CanonicalName) == 0 ||
                wxStricmp(locale, info->Description) == 0 )
        {
                        infoRet = info;
            break;
        }

        if ( wxStricmp(locale, info->CanonicalName.BeforeFirst(wxS('_'))) == 0 )
        {
                                                                                    if ( !infoRet )
                infoRet = info;
        }
    }

    return infoRet;
}

wxString wxLocale::GetSysName() const
{
    return wxSetlocale(LC_ALL, NULL);
}

wxLocale::~wxLocale()
{
                if ( wxTranslations::Get() == &m_translations )
    {
        if ( m_pOldLocale )
            wxTranslations::SetNonOwned(&m_pOldLocale->m_translations);
        else
            wxTranslations::Set(NULL);
    }

        wxSetLocale(m_pOldLocale);

    if ( m_pszOldLocale )
    {
        wxSetlocale(LC_ALL, m_pszOldLocale);
        free(const_cast<char *>(m_pszOldLocale));
    }
}



bool wxLocale::IsAvailable(int lang)
{
    const wxLanguageInfo *info = wxLocale::GetLanguageInfo(lang);
    if ( !info )
    {
                        wxASSERT_MSG( lang == wxLANGUAGE_DEFAULT,
                      wxS("No info for a valid language?") );
        return false;
    }

#if defined(__WIN32__)
    if ( !info->WinLang )
        return false;

    if ( !::IsValidLocale(info->GetLCID(), LCID_INSTALLED) )
        return false;

#elif defined(__UNIX__)

        char * const oldLocale = wxStrdupA(setlocale(LC_ALL, NULL));

            const bool
        available = wxSetlocaleTryUTF8(LC_ALL, info->CanonicalName) ||
                    wxSetlocaleTryUTF8(LC_ALL, ExtractLang(info->CanonicalName));

        wxSetlocale(LC_ALL, oldLocale);

    free(oldLocale);

    if ( !available )
        return false;
#endif

    return true;
}


bool wxLocale::AddCatalog(const wxString& domain)
{
    wxTranslations *t = wxTranslations::Get();
    if ( !t )
        return false;
    return t->AddCatalog(domain);
}

bool wxLocale::AddCatalog(const wxString& domain, wxLanguage msgIdLanguage)
{
    wxTranslations *t = wxTranslations::Get();
    if ( !t )
        return false;
    return t->AddCatalog(domain, msgIdLanguage);
}

bool wxLocale::AddCatalog(const wxString& szDomain,
                        wxLanguage      msgIdLanguage,
                        const wxString& msgIdCharset)
{
    wxTranslations *t = wxTranslations::Get();
    if ( !t )
        return false;
#if wxUSE_UNICODE
    wxUnusedVar(msgIdCharset);
    return t->AddCatalog(szDomain, msgIdLanguage);
#else
    return t->AddCatalog(szDomain, msgIdLanguage, msgIdCharset);
#endif
}

bool wxLocale::IsLoaded(const wxString& domain) const
{
    wxTranslations *t = wxTranslations::Get();
    if ( !t )
        return false;
    return t->IsLoaded(domain);
}

wxString wxLocale::GetHeaderValue(const wxString& header,
                                  const wxString& domain) const
{
    wxTranslations *t = wxTranslations::Get();
    if ( !t )
        return wxEmptyString;
    return t->GetHeaderValue(header, domain);
}


#if defined(__WINDOWS__) || defined(__WXOSX__)

namespace
{

bool IsAtTwoSingleQuotes(const wxString& fmt, wxString::const_iterator p)
{
    if ( p != fmt.end() && *p == '\'')
    {
        ++p;
        if ( p != fmt.end() && *p == '\'')
        {
            return true;
        }
    }

    return false;
}

} 

WXDLLIMPEXP_BASE
wxString wxTranslateFromUnicodeFormat(const wxString& fmt)
{
    wxString fmtWX;
    fmtWX.reserve(fmt.length());

    char chLast = '\0';
    size_t lastCount = 0;

    const char* formatchars =
        "dghHmMsSy"
#ifdef __WINDOWS__
        "t"
#else
        "EawD"
#endif
        ;
    for ( wxString::const_iterator p = fmt.begin(); ; ++p )
    {
        if ( p != fmt.end() )
        {
            if ( *p == chLast )
            {
                lastCount++;
                continue;
            }

            const wxUniChar ch = (*p).GetValue();
            if ( ch.IsAscii() && strchr(formatchars, ch) )
            {
                                chLast = ch;
                lastCount = 1;
                continue;
            }
        }

                if ( lastCount )
        {
            switch ( chLast )
            {
                case 'd':
                    switch ( lastCount )
                    {
                        case 1:                         case 2:                                                                                     fmtWX += "%d";
                            break;
#ifdef __WINDOWS__
                        case 3:                             fmtWX += "%a";
                            break;

                        case 4:                             fmtWX += "%A";
                            break;
#endif
                        default:
                            wxFAIL_MSG( "too many 'd's" );
                    }
                    break;
#ifndef __WINDOWS__
                case 'D':
                    switch ( lastCount )
                    {
                        case 1:                         case 2:                         case 3:                             fmtWX += "%j";
                            break;

                        default:
                            wxFAIL_MSG( "wrong number of 'D's" );
                    }
                    break;
               case 'w':
                    switch ( lastCount )
                    {
                        case 1:                         case 2:                             fmtWX += "%W";
                            break;

                        default:
                            wxFAIL_MSG( "wrong number of 'w's" );
                    }
                    break;
                case 'E':
                   switch ( lastCount )
                    {
                        case 1:                         case 2:                         case 3:                             fmtWX += "%a";
                            break;
                        case 4:                             fmtWX += "%A";
                            break;
                        case 5:                         case 6:                                                         fmtWX += "%a";
                            break;

                        default:
                            wxFAIL_MSG( "wrong number of 'E's" );
                    }
                    break;
#endif
                case 'M':
                    switch ( lastCount )
                    {
                        case 1:                         case 2:                                                         fmtWX += "%m";
                            break;

                        case 3:
                            fmtWX += "%b";
                            break;

                        case 4:
                            fmtWX += "%B";
                            break;

                        case 5:
                                                        fmtWX += "%b";
                            break;

                        default:
                            wxFAIL_MSG( "too many 'M's" );
                    }
                    break;

                case 'y':
                    switch ( lastCount )
                    {
                        case 1:                         case 2:                             fmtWX += "%y";
                            break;

                        case 4:                             fmtWX += "%Y";
                            break;

                        default:
                            wxFAIL_MSG( "wrong number of 'y's" );
                    }
                    break;

                case 'H':
                    switch ( lastCount )
                    {
                        case 1:                         case 2:                             fmtWX += "%H";
                            break;

                        default:
                            wxFAIL_MSG( "wrong number of 'H's" );
                    }
                    break;

               case 'h':
                    switch ( lastCount )
                    {
                        case 1:                         case 2:                             fmtWX += "%I";
                            break;

                        default:
                            wxFAIL_MSG( "wrong number of 'h's" );
                    }
                    break;

               case 'm':
                    switch ( lastCount )
                    {
                        case 1:                         case 2:                             fmtWX += "%M";
                            break;

                        default:
                            wxFAIL_MSG( "wrong number of 'm's" );
                    }
                    break;

               case 's':
                    switch ( lastCount )
                    {
                        case 1:                         case 2:                             fmtWX += "%S";
                            break;

                        default:
                            wxFAIL_MSG( "wrong number of 's's" );
                    }
                    break;

                case 'g':
                                                            wxASSERT_MSG( lastCount <= 2, "too many 'g's" );

                    break;
#ifndef __WINDOWS__
                case 'a':
                    fmtWX += "%p";
                    break;
#endif
#ifdef __WINDOWS__
                case 't':
                    switch ( lastCount )
                    {
                        case 1:                         case 2:                             fmtWX += "%p";
                            break;

                        default:
                            wxFAIL_MSG( "too many 't's" );
                    }
                    break;
#endif
                default:
                    wxFAIL_MSG( "unreachable" );
            }

            chLast = '\0';
            lastCount = 0;
        }

        if ( p == fmt.end() )
            break;

        

        if ( IsAtTwoSingleQuotes(fmt, p) )
        {
            fmtWX += '\'';
            ++p;             continue;
        }

        bool isEndQuote = false;
        if ( *p == '\'' )
        {
            ++p;
            while ( p != fmt.end() )
            {
                if ( IsAtTwoSingleQuotes(fmt, p) )
                {
                    fmtWX += '\'';
                    p += 2;
                    continue;
                }

                if ( *p == '\'' )
                {
                    isEndQuote = true;
                    break;
                }

                fmtWX += *p;
                ++p;
            }
        }

        if ( p == fmt.end() )
            break;

        if ( !isEndQuote )
        {
                        if ( *p == wxT('%') )
            {
                                fmtWX += wxT('%');
            }

            fmtWX += *p;
        }
    }

    return fmtWX;
}


#endif 
#if defined(__WINDOWS__)

namespace
{

LCTYPE GetLCTYPEFormatFromLocalInfo(wxLocaleInfo index)
{
    switch ( index )
    {
        case wxLOCALE_SHORT_DATE_FMT:
            return LOCALE_SSHORTDATE;

        case wxLOCALE_LONG_DATE_FMT:
            return LOCALE_SLONGDATE;

        case wxLOCALE_TIME_FMT:
            return LOCALE_STIMEFORMAT;

        default:
            wxFAIL_MSG( "no matching LCTYPE" );
    }

    return 0;
}

wxString
GetInfoFromLCID(LCID lcid,
                wxLocaleInfo index,
                wxLocaleCategory cat = wxLOCALE_CAT_DEFAULT)
{
    wxString str;

    wxChar buf[256];
    buf[0] = wxT('\0');

    switch ( index )
    {
        case wxLOCALE_THOUSANDS_SEP:
            if ( ::GetLocaleInfo(lcid, LOCALE_STHOUSAND, buf, WXSIZEOF(buf)) )
                str = buf;
            break;

        case wxLOCALE_DECIMAL_POINT:
            if ( ::GetLocaleInfo(lcid,
                                 cat == wxLOCALE_CAT_MONEY
                                     ? LOCALE_SMONDECIMALSEP
                                     : LOCALE_SDECIMAL,
                                 buf,
                                 WXSIZEOF(buf)) )
            {
                str = buf;

                                                                                                                                wxASSERT_MSG
                (
                    wxString::Format("%.3f", 1.23).find(str) != wxString::npos,
                    "Decimal separator mismatch -- did you use setlocale()?"
                    "If so, use wxLocale to change the locale instead."
                );
            }
            break;

        case wxLOCALE_SHORT_DATE_FMT:
        case wxLOCALE_LONG_DATE_FMT:
        case wxLOCALE_TIME_FMT:
            if ( ::GetLocaleInfo(lcid, GetLCTYPEFormatFromLocalInfo(index),
                                 buf, WXSIZEOF(buf)) )
            {
                return wxTranslateFromUnicodeFormat(buf);
            }
            break;

        case wxLOCALE_DATE_TIME_FMT:
                                                                                    {
                const wxString
                    datefmt = GetInfoFromLCID(lcid, wxLOCALE_SHORT_DATE_FMT);
                if ( datefmt.empty() )
                    break;

                const wxString
                    timefmt = GetInfoFromLCID(lcid, wxLOCALE_TIME_FMT);
                if ( timefmt.empty() )
                    break;

                str << datefmt << ' ' << timefmt;
            }
            break;

        default:
            wxFAIL_MSG( "unknown wxLocaleInfo" );
    }

    return str;
}

} 

wxString wxLocale::GetInfo(wxLocaleInfo index, wxLocaleCategory cat)
{
    const wxLanguageInfo * const
        info = wxGetLocale() ? GetLanguageInfo(wxGetLocale()->GetLanguage())
                             : NULL;
    if ( !info )
    {
                                wxASSERT_MSG( strcmp(setlocale(LC_ALL, NULL), "C") == 0,
                      wxS("You probably called setlocale() directly instead ")
                      wxS("of using wxLocale and now there is a ")
                      wxS("mismatch between C/C++ and Windows locale.\n")
                      wxS("Things are going to break, please only change ")
                      wxS("locale by creating wxLocale objects to avoid this!") );


                                                switch ( index )
        {
            case wxLOCALE_THOUSANDS_SEP:
                return wxString();

            case wxLOCALE_DECIMAL_POINT:
                return ".";

            case wxLOCALE_SHORT_DATE_FMT:
                return "%m/%d/%y";

            case wxLOCALE_LONG_DATE_FMT:
                return "%A, %B %d, %Y";

            case wxLOCALE_TIME_FMT:
                return "%H:%M:%S";

            case wxLOCALE_DATE_TIME_FMT:
                return "%m/%d/%y %H:%M:%S";

            default:
                wxFAIL_MSG( "unknown wxLocaleInfo" );
        }
    }

    return GetInfoFromLCID(info->GetLCID(), index, cat);
}


wxString wxLocale::GetOSInfo(wxLocaleInfo index, wxLocaleCategory cat)
{
    return GetInfoFromLCID(::GetThreadLocale(), index, cat);
}

#elif defined(__WXOSX__)


wxString wxLocale::GetInfo(wxLocaleInfo index, wxLocaleCategory WXUNUSED(cat))
{
    CFLocaleRef userLocaleRefRaw;
    if ( wxGetLocale() )
    {
        userLocaleRefRaw = CFLocaleCreate
                        (
                                kCFAllocatorDefault,
                                wxCFStringRef(wxGetLocale()->GetCanonicalName())
                        );
    }
    else     {
        userLocaleRefRaw = CFLocaleCopyCurrent();
    }

    wxCFRef<CFLocaleRef> userLocaleRef(userLocaleRefRaw);

    CFStringRef cfstr = 0;
    switch ( index )
    {
        case wxLOCALE_THOUSANDS_SEP:
            cfstr = (CFStringRef) CFLocaleGetValue(userLocaleRef, kCFLocaleGroupingSeparator);
            break;

        case wxLOCALE_DECIMAL_POINT:
            cfstr = (CFStringRef) CFLocaleGetValue(userLocaleRef, kCFLocaleDecimalSeparator);
            break;

        case wxLOCALE_SHORT_DATE_FMT:
        case wxLOCALE_LONG_DATE_FMT:
        case wxLOCALE_DATE_TIME_FMT:
        case wxLOCALE_TIME_FMT:
            {
                CFDateFormatterStyle dateStyle = kCFDateFormatterNoStyle;
                CFDateFormatterStyle timeStyle = kCFDateFormatterNoStyle;
                switch (index )
                {
                    case wxLOCALE_SHORT_DATE_FMT:
                        dateStyle = kCFDateFormatterShortStyle;
                        break;
                    case wxLOCALE_LONG_DATE_FMT:
                        dateStyle = kCFDateFormatterFullStyle;
                        break;
                    case wxLOCALE_DATE_TIME_FMT:
                        dateStyle = kCFDateFormatterFullStyle;
                        timeStyle = kCFDateFormatterMediumStyle;
                        break;
                    case wxLOCALE_TIME_FMT:
                        timeStyle = kCFDateFormatterMediumStyle;
                        break;
                    default:
                        wxFAIL_MSG( "unexpected time locale" );
                        return wxString();
                }
                wxCFRef<CFDateFormatterRef> dateFormatter( CFDateFormatterCreate
                    (NULL, userLocaleRef, dateStyle, timeStyle));
                wxCFStringRef cfs = wxCFRetain( CFDateFormatterGetFormat(dateFormatter ));
                wxString format = wxTranslateFromUnicodeFormat(cfs.AsString());
                                format.Replace("%y","%Y");
                return format;
            }
            break;

        default:
            wxFAIL_MSG( "Unknown locale info" );
            return wxString();
    }

    wxCFStringRef str(wxCFRetain(cfstr));
    return str.AsString();
}

#else 
namespace
{

wxString GetDateFormatFromLangInfo(wxLocaleInfo index)
{
#ifdef HAVE_LANGINFO_H
            static const nl_item items[] =
    {
        D_FMT, D_T_FMT, D_T_FMT, T_FMT,
    };

    const int nlidx = index - wxLOCALE_SHORT_DATE_FMT;
    if ( nlidx < 0 || nlidx >= (int)WXSIZEOF(items) )
    {
        wxFAIL_MSG( "logic error in GetInfo() code" );
        return wxString();
    }

    const wxString fmt(nl_langinfo(items[nlidx]));

                if ( fmt.empty() || index != wxLOCALE_LONG_DATE_FMT )
        return fmt;

                    static const char *timeFmtSpecs = "HIklMpPrRsSTXzZ";
    static const char *timeSep = " :./-";

    wxString fmtDateOnly;
    const wxString::const_iterator end = fmt.end();
    wxString::const_iterator lastSep = end;
    for ( wxString::const_iterator p = fmt.begin(); p != end; ++p )
    {
        if ( strchr(timeSep, *p) )
        {
            if ( lastSep == end )
                lastSep = p;

                                    continue;
        }

        if ( *p == '%' &&
                (p + 1 != end) && strchr(timeFmtSpecs, p[1]) )
        {
                        ++p;
            lastSep = end;
            continue;
        }

        if ( lastSep != end )
        {
            fmtDateOnly += wxString(lastSep, p);
            lastSep = end;
        }

        fmtDateOnly += *p;
    }

    return fmtDateOnly;
#else     wxUnusedVar(index);

                        return wxString();
#endif }

} 

wxString wxLocale::GetInfo(wxLocaleInfo index, wxLocaleCategory cat)
{
#if !(defined(__WXQT__) && defined(__ANDROID__))
    lconv * const lc = localeconv();
    if ( !lc )
        return wxString();

    switch ( index )
    {
        case wxLOCALE_THOUSANDS_SEP:
            if ( cat == wxLOCALE_CAT_NUMBER )
                return lc->thousands_sep;
            else if ( cat == wxLOCALE_CAT_MONEY )
                return lc->mon_thousands_sep;

            wxFAIL_MSG( "invalid wxLocaleCategory" );
            break;


        case wxLOCALE_DECIMAL_POINT:
            if ( cat == wxLOCALE_CAT_NUMBER )
                return lc->decimal_point;
            else if ( cat == wxLOCALE_CAT_MONEY )
                return lc->mon_decimal_point;

            wxFAIL_MSG( "invalid wxLocaleCategory" );
            break;

        case wxLOCALE_SHORT_DATE_FMT:
        case wxLOCALE_LONG_DATE_FMT:
        case wxLOCALE_DATE_TIME_FMT:
        case wxLOCALE_TIME_FMT:
            if ( cat != wxLOCALE_CAT_DATE && cat != wxLOCALE_CAT_DEFAULT )
            {
                wxFAIL_MSG( "invalid wxLocaleCategory" );
                break;
            }

            return GetDateFormatFromLangInfo(index);


        default:
            wxFAIL_MSG( "unknown wxLocaleInfo value" );
    }
#endif
    return wxString();
}

#endif 
#ifndef __WINDOWS__


wxString wxLocale::GetOSInfo(wxLocaleInfo index, wxLocaleCategory cat)
{
    return GetInfo(index, cat);
}

#endif 


static wxLocale *g_pLocale = NULL;

wxLocale *wxGetLocale()
{
    return g_pLocale;
}

wxLocale *wxSetLocale(wxLocale *pLocale)
{
    wxLocale *pOld = g_pLocale;
    g_pLocale = pLocale;
    return pOld;
}




class wxLocaleModule: public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxLocaleModule);
    public:
        wxLocaleModule() {}

        bool OnInit() wxOVERRIDE
        {
            return true;
        }

        void OnExit() wxOVERRIDE
        {
            wxLocale::DestroyLanguagesDB();
        }
};

wxIMPLEMENT_DYNAMIC_CLASS(wxLocaleModule, wxModule);

#endif 