


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_REGEX

#include "wx/regex.h"

#ifndef WX_PRECOMP
    #include "wx/object.h"
    #include "wx/log.h"
    #include "wx/intl.h"
    #include "wx/crt.h"
#endif 
#if defined(__UNIX__)
#   include <sys/types.h>
#endif

#include <regex.h>

#ifdef __REG_NOFRONT
#   define WXREGEX_USING_BUILTIN
#   define WXREGEX_IF_NEED_LEN(x) ,x
#   if wxUSE_UNICODE
#       define WXREGEX_CHAR(x) (x).wc_str()
#   else
#       define WXREGEX_CHAR(x) (x).mb_str()
#   endif
#else
#   ifdef HAVE_RE_SEARCH
#       define WXREGEX_IF_NEED_LEN(x) ,x
#       define WXREGEX_USING_RE_SEARCH
#   else
#       define WXREGEX_IF_NEED_LEN(x)
#   endif
#   if wxUSE_UNICODE
#       define WXREGEX_CONVERT_TO_MB
#   endif
#   define WXREGEX_CHAR(x) (x).mb_str()
#   define wx_regfree regfree
#   define wx_regerror regerror
#endif


#ifndef WXREGEX_USING_RE_SEARCH

class wxRegExMatches
{
public:
    typedef regmatch_t *match_type;

    wxRegExMatches(size_t n)        { m_matches = new regmatch_t[n]; }
    ~wxRegExMatches()               { delete [] m_matches; }

                    size_t Start(size_t n) const
    {
        return wx_truncate_cast(size_t, m_matches[n].rm_so);
    }

    size_t End(size_t n) const
    {
        return wx_truncate_cast(size_t, m_matches[n].rm_eo);
    }

    regmatch_t *get() const         { return m_matches; }

private:
    regmatch_t *m_matches;
};

#else 
class wxRegExMatches
{
public:
    typedef re_registers *match_type;

    wxRegExMatches(size_t n)
    {
        m_matches.num_regs = n;
        m_matches.start = new regoff_t[n];
        m_matches.end = new regoff_t[n];
    }

    ~wxRegExMatches()
    {
        delete [] m_matches.start;
        delete [] m_matches.end;
    }

    size_t Start(size_t n) const    { return m_matches.start[n]; }
    size_t End(size_t n) const      { return m_matches.end[n]; }

    re_registers *get()             { return &m_matches; }

private:
    re_registers m_matches;
};

#endif 
#ifndef WXREGEX_CONVERT_TO_MB
typedef wxChar wxRegChar;
#else
typedef char wxRegChar;
#endif

class wxRegExImpl
{
public:
        wxRegExImpl();
    ~wxRegExImpl();

        bool IsValid() const { return m_isCompiled; }

        bool Compile(const wxString& expr, int flags = 0);
    bool Matches(const wxRegChar *str, int flags
                 WXREGEX_IF_NEED_LEN(size_t len)) const;
    bool GetMatch(size_t *start, size_t *len, size_t index = 0) const;
    size_t GetMatchCount() const;
    int Replace(wxString *pattern, const wxString& replacement,
                size_t maxMatches = 0) const;

private:
        wxString GetErrorMsg(int errorcode, bool badconv) const;

        void Init()
    {
        m_isCompiled = false;
        m_Matches = NULL;
        m_nMatches = 0;
    }

        void Free()
    {
        if ( IsValid() )
        {
            wx_regfree(&m_RegEx);
        }

        delete m_Matches;
    }

        void Reinit()
    {
        Free();
        Init();
    }

        regex_t         m_RegEx;

        wxRegExMatches *m_Matches;
    size_t          m_nMatches;

        bool            m_isCompiled;
};




wxRegExImpl::wxRegExImpl()
{
    Init();
}

wxRegExImpl::~wxRegExImpl()
{
    Free();
}

wxString wxRegExImpl::GetErrorMsg(int errorcode, bool badconv) const
{
#ifdef WXREGEX_CONVERT_TO_MB
        if ( badconv )
    {
        return _("conversion to 8-bit encoding failed");
    }
#else
        (void)badconv;
#endif

    wxString szError;

        int len = wx_regerror(errorcode, &m_RegEx, NULL, 0);
    if ( len > 0 )
    {
        char* szcmbError = new char[++len];

        (void)wx_regerror(errorcode, &m_RegEx, szcmbError, len);

        szError = wxConvLibc.cMB2WX(szcmbError);
        delete [] szcmbError;
    }
    else     {
        szError = _("unknown error");
    }

    return szError;
}

bool wxRegExImpl::Compile(const wxString& expr, int flags)
{
    Reinit();

#ifdef WX_NO_REGEX_ADVANCED
#   define FLAVORS wxRE_BASIC
#else
#   define FLAVORS (wxRE_ADVANCED | wxRE_BASIC)
    wxASSERT_MSG( (flags & FLAVORS) != FLAVORS,
                  wxT("incompatible flags in wxRegEx::Compile") );
#endif
    wxASSERT_MSG( !(flags & ~(FLAVORS | wxRE_ICASE | wxRE_NOSUB | wxRE_NEWLINE)),
                  wxT("unrecognized flags in wxRegEx::Compile") );

        int flagsRE = 0;
    if ( !(flags & wxRE_BASIC) )
    {
#ifndef WX_NO_REGEX_ADVANCED
        if (flags & wxRE_ADVANCED)
            flagsRE |= REG_ADVANCED;
        else
#endif
            flagsRE |= REG_EXTENDED;
    }
    if ( flags & wxRE_ICASE )
        flagsRE |= REG_ICASE;
    if ( flags & wxRE_NOSUB )
        flagsRE |= REG_NOSUB;
    if ( flags & wxRE_NEWLINE )
        flagsRE |= REG_NEWLINE;

    #ifdef WXREGEX_USING_BUILTIN
    bool conv = true;
        int errorcode = wx_re_comp(&m_RegEx, expr.c_str(), expr.length(), flagsRE);
#else
            const wxWX2MBbuf conv = expr.mbc_str();
    int errorcode = conv ? regcomp(&m_RegEx, conv, flagsRE) : REG_BADPAT;
#endif

    if ( errorcode )
    {
        wxLogError(_("Invalid regular expression '%s': %s"),
                   expr.c_str(), GetErrorMsg(errorcode, !conv).c_str());

        m_isCompiled = false;
    }
    else     {
                if ( flags & wxRE_NOSUB )
        {
                        m_nMatches = 0;
        }
        else
        {
                        
                        m_nMatches = 1;

                        for ( const wxChar *cptr = expr.c_str(); *cptr; cptr++ )
            {
                if ( *cptr == wxT('\\') )
                {
                                        if ( *++cptr == wxT('(') && (flags & wxRE_BASIC) )
                    {
                        m_nMatches++;
                    }
                }
                else if ( *cptr == wxT('(') && !(flags & wxRE_BASIC) )
                {
                                                                                                                                            if ( cptr[1] != wxT('?') )
                        m_nMatches++;
                }
            }
        }

        m_isCompiled = true;
    }

    return IsValid();
}

#ifdef WXREGEX_USING_RE_SEARCH

static int ReSearch(const regex_t *preg,
                    const char *text,
                    size_t len,
                    re_registers *matches,
                    int eflags)
{
    regex_t *pattern = const_cast<regex_t*>(preg);

    pattern->not_bol = (eflags & REG_NOTBOL) != 0;
    pattern->not_eol = (eflags & REG_NOTEOL) != 0;
    pattern->regs_allocated = REGS_FIXED;

    int ret = re_search(pattern, text, len, 0, len, matches);
    return ret >= 0 ? 0 : REG_NOMATCH;
}

#endif 
bool wxRegExImpl::Matches(const wxRegChar *str,
                          int flags
                          WXREGEX_IF_NEED_LEN(size_t len)) const
{
    wxCHECK_MSG( IsValid(), false, wxT("must successfully Compile() first") );

        wxASSERT_MSG( !(flags & ~(wxRE_NOTBOL | wxRE_NOTEOL)),
                  wxT("unrecognized flags in wxRegEx::Matches") );

    int flagsRE = 0;
    if ( flags & wxRE_NOTBOL )
        flagsRE |= REG_NOTBOL;
    if ( flags & wxRE_NOTEOL )
        flagsRE |= REG_NOTEOL;

        wxRegExImpl *self = wxConstCast(this, wxRegExImpl);
    if ( !m_Matches && m_nMatches )
    {
        self->m_Matches = new wxRegExMatches(m_nMatches);
    }

    wxRegExMatches::match_type matches = m_Matches ? m_Matches->get() : NULL;

    #if defined WXREGEX_USING_BUILTIN
    int rc = wx_re_exec(&self->m_RegEx, str, len, NULL, m_nMatches, matches, flagsRE);
#elif defined WXREGEX_USING_RE_SEARCH
    int rc = str ? ReSearch(&self->m_RegEx, str, len, matches, flagsRE) : REG_BADPAT;
#else
    int rc = str ? regexec(&self->m_RegEx, str, m_nMatches, matches, flagsRE) : REG_BADPAT;
#endif

    switch ( rc )
    {
        case 0:
                        return true;

        default:
                        wxLogError(_("Failed to find match for regular expression: %s"),
                       GetErrorMsg(rc, !str).c_str());
            wxFALLTHROUGH;

        case REG_NOMATCH:
                        return false;
    }
}

bool wxRegExImpl::GetMatch(size_t *start, size_t *len, size_t index) const
{
    wxCHECK_MSG( IsValid(), false, wxT("must successfully Compile() first") );
    wxCHECK_MSG( m_nMatches, false, wxT("can't use with wxRE_NOSUB") );
    wxCHECK_MSG( m_Matches, false, wxT("must call Matches() first") );
    wxCHECK_MSG( index < m_nMatches, false, wxT("invalid match index") );

    if ( start )
        *start = m_Matches->Start(index);
    if ( len )
        *len = m_Matches->End(index) - m_Matches->Start(index);

    return true;
}

size_t wxRegExImpl::GetMatchCount() const
{
    wxCHECK_MSG( IsValid(), 0, wxT("must successfully Compile() first") );
    wxCHECK_MSG( m_nMatches, 0, wxT("can't use with wxRE_NOSUB") );

    return m_nMatches;
}

int wxRegExImpl::Replace(wxString *text,
                         const wxString& replacement,
                         size_t maxMatches) const
{
    wxCHECK_MSG( text, wxNOT_FOUND, wxT("NULL text in wxRegEx::Replace") );
    wxCHECK_MSG( IsValid(), wxNOT_FOUND, wxT("must successfully Compile() first") );

    #ifndef WXREGEX_CONVERT_TO_MB
    const wxChar *textstr = text->c_str();
    size_t textlen = text->length();
#else
    const wxWX2MBbuf textstr = WXREGEX_CHAR(*text);
    if (!textstr)
    {
        wxLogError(_("Failed to find match for regular expression: %s"),
                   GetErrorMsg(0, true).c_str());
        return 0;
    }
    size_t textlen = strlen(textstr);
    text->clear();
#endif

        wxString textNew;

        wxString result;
    result.reserve(5 * textlen / 4);

            bool mayHaveBackrefs =
        replacement.find_first_of(wxT("\\&")) != wxString::npos;

    if ( !mayHaveBackrefs )
    {
        textNew = replacement;
    }

        size_t matchStart = 0;

            size_t countRepl = 0;

            while ( (!maxMatches || countRepl < maxMatches) &&
             Matches(
#ifndef WXREGEX_CONVERT_TO_MB
                    textstr + matchStart,
#else
                    textstr.data() + matchStart,
#endif
                    countRepl ? wxRE_NOTBOL : 0
                    WXREGEX_IF_NEED_LEN(textlen - matchStart)) )
    {
                        if ( mayHaveBackrefs )
        {
            mayHaveBackrefs = false;
            textNew.clear();
            textNew.reserve(replacement.length());

            for ( const wxChar *p = replacement.c_str(); *p; p++ )
            {
                size_t index = (size_t)-1;

                if ( *p == wxT('\\') )
                {
                    if ( wxIsdigit(*++p) )
                    {
                                                wxChar *end;
                        index = (size_t)wxStrtoul(p, &end, 10);
                        p = end - 1;                     }
                                    }
                else if ( *p == wxT('&') )
                {
                                        index = 0;
                }

                                if ( index != (size_t)-1 )
                {
                                        size_t start, len;
                    if ( !GetMatch(&start, &len, index) )
                    {
                        wxFAIL_MSG( wxT("invalid back reference") );

                                            }
                    else
                    {
                        textNew += wxString(
#ifndef WXREGEX_CONVERT_TO_MB
                                textstr
#else
                                textstr.data()
#endif
                                + matchStart + start,
                                *wxConvCurrent, len);

                        mayHaveBackrefs = true;
                    }
                }
                else                 {
                    textNew += *p;
                }
            }
        }

        size_t start, len;
        if ( !GetMatch(&start, &len) )
        {
                        wxFAIL_MSG( wxT("internal logic error in wxRegEx::Replace") );

            return wxNOT_FOUND;
        }

                        if (result.capacity() < result.length() + start + textNew.length())
            result.reserve(2 * result.length());

#ifndef WXREGEX_CONVERT_TO_MB
        result.append(*text, matchStart, start);
#else
        result.append(wxString(textstr.data() + matchStart, *wxConvCurrent, start));
#endif
        matchStart += start;
        result.append(textNew);

        countRepl++;

        matchStart += len;
    }

#ifndef WXREGEX_CONVERT_TO_MB
    result.append(*text, matchStart, wxString::npos);
#else
    result.append(wxString(textstr.data() + matchStart, *wxConvCurrent));
#endif
    *text = result;

    return countRepl;
}


void wxRegEx::Init()
{
    m_impl = NULL;
}

wxRegEx::~wxRegEx()
{
    delete m_impl;
}

bool wxRegEx::Compile(const wxString& expr, int flags)
{
    if ( !m_impl )
    {
        m_impl = new wxRegExImpl;
    }

    if ( !m_impl->Compile(expr, flags) )
    {
                wxDELETE(m_impl);

        return false;
    }

    return true;
}

bool wxRegEx::Matches(const wxString& str, int flags) const
{
    wxCHECK_MSG( IsValid(), false, wxT("must successfully Compile() first") );

    return m_impl->Matches(WXREGEX_CHAR(str), flags
                            WXREGEX_IF_NEED_LEN(str.length()));
}

bool wxRegEx::GetMatch(size_t *start, size_t *len, size_t index) const
{
    wxCHECK_MSG( IsValid(), false, wxT("must successfully Compile() first") );

    return m_impl->GetMatch(start, len, index);
}

wxString wxRegEx::GetMatch(const wxString& text, size_t index) const
{
    size_t start, len;
    if ( !GetMatch(&start, &len, index) )
        return wxEmptyString;

    return text.Mid(start, len);
}

size_t wxRegEx::GetMatchCount() const
{
    wxCHECK_MSG( IsValid(), 0, wxT("must successfully Compile() first") );

    return m_impl->GetMatchCount();
}

int wxRegEx::Replace(wxString *pattern,
                     const wxString& replacement,
                     size_t maxMatches) const
{
    wxCHECK_MSG( IsValid(), wxNOT_FOUND, wxT("must successfully Compile() first") );

    return m_impl->Replace(pattern, replacement, maxMatches);
}

#endif 