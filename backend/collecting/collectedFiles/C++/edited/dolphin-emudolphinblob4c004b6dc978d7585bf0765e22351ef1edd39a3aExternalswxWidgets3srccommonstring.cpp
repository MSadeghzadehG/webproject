

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/wxcrtvararg.h"
    #include "wx/intl.h"
    #include "wx/log.h"
#endif

#include <ctype.h>

#include <errno.h>

#include <string.h>
#include <stdlib.h>

#include "wx/hashmap.h"
#include "wx/vector.h"
#include "wx/xlocale.h"

#ifdef __WINDOWS__
    #include "wx/msw/wrapwin.h"
#endif 
#if wxUSE_STD_IOSTREAM
    #include <sstream>
#endif

#ifndef HAVE_STD_STRING_COMPARE
#if wxUSE_UNICODE_UTF8
    #define wxStringMemcmp   memcmp
    #define wxStringStrlen   strlen
#else
    #define wxStringMemcmp   wxTmemcmp
    #define wxStringStrlen   wxStrlen
#endif
#endif

namespace wxPrivate
{

UntypedBufferData *GetUntypedNullData()
{
    static UntypedBufferData s_untypedNullData(NULL, 0);

    return &s_untypedNullData;
}

} 

const size_t wxString::npos = (size_t) -1;

#if wxUSE_STRING_POS_CACHE

#ifdef wxHAS_COMPILER_TLS

wxTLS_TYPE(wxString::Cache) wxString::ms_cache;

#else 
struct wxStrCacheInitializer
{
    wxStrCacheInitializer()
    {
                        wxString::GetCache();
    }
};



static wxStrCacheInitializer gs_stringCacheInit;

#endif 
#if wxDEBUG_LEVEL >= 2

struct wxStrCacheDumper
{
    static void ShowAll()
    {
        puts("*** wxString cache dump:");
        for ( unsigned n = 0; n < wxString::Cache::SIZE; n++ )
        {
            const wxString::Cache::Element&
                c = wxString::GetCacheBegin()[n];

            printf("\t%u%s\t%p: pos=(%lu, %lu), len=%ld\n",
                   n,
                   n == wxString::LastUsedCacheElement() ? " [*]" : "",
                   c.str,
                   (unsigned long)c.pos,
                   (unsigned long)c.impl,
                   (long)c.len);
        }
    }
};

void wxDumpStrCache() { wxStrCacheDumper::ShowAll(); }

#endif 
#ifdef wxPROFILE_STRING_CACHE

wxString::CacheStats wxString::ms_cacheStats;

struct wxStrCacheStatsDumper
{
    ~wxStrCacheStatsDumper()
    {
        const wxString::CacheStats& stats = wxString::ms_cacheStats;

        if ( stats.postot )
        {
            puts("*** wxString cache statistics:");
            printf("\tTotal non-trivial calls to PosToImpl(): %u\n",
                   stats.postot);
            printf("\tHits %u (of which %u not used) or %.2f%%\n",
                   stats.poshits,
                   stats.mishits,
                   100.*float(stats.poshits - stats.mishits)/stats.postot);
            printf("\tAverage position requested: %.2f\n",
                   float(stats.sumpos) / stats.postot);
            printf("\tAverage offset after cached hint: %.2f\n",
                   float(stats.sumofs) / stats.postot);
        }

        if ( stats.lentot )
        {
            printf("\tNumber of calls to length(): %u, hits=%.2f%%\n",
                   stats.lentot, 100.*float(stats.lenhits)/stats.lentot);
        }
    }
};

static wxStrCacheStatsDumper s_showCacheStats;

#endif 
#endif 

#if wxUSE_STD_IOSTREAM

#include <iostream>

wxSTD ostream& operator<<(wxSTD ostream& os, const wxCStrData& str)
{
#if wxUSE_UNICODE && !wxUSE_UNICODE_UTF8
    return os << wxConvWhateverWorks.cWX2MB(str);
#else
    return os << str.AsInternal();
#endif
}

wxSTD ostream& operator<<(wxSTD ostream& os, const wxString& str)
{
    return os << str.c_str();
}

wxSTD ostream& operator<<(wxSTD ostream& os, const wxScopedCharBuffer& str)
{
    return os << str.data();
}

#ifndef __BORLANDC__
wxSTD ostream& operator<<(wxSTD ostream& os, const wxScopedWCharBuffer& str)
{
    return os << str.data();
}
#endif

#if wxUSE_UNICODE && defined(HAVE_WOSTREAM)

wxSTD wostream& operator<<(wxSTD wostream& wos, const wxString& str)
{
    return wos << str.wc_str();
}

wxSTD wostream& operator<<(wxSTD wostream& wos, const wxCStrData& str)
{
    return wos << str.AsWChar();
}

wxSTD wostream& operator<<(wxSTD wostream& wos, const wxScopedWCharBuffer& str)
{
    return wos << str.data();
}

#endif  
#endif 

#if wxUSE_UNICODE_UTF8

void wxString::PosLenToImpl(size_t pos, size_t len,
                            size_t *implPos, size_t *implLen) const
{
    if ( pos == npos )
    {
        *implPos = npos;
    }
    else     {
        const const_iterator b = GetIterForNthChar(pos);
        *implPos = wxStringImpl::const_iterator(b.impl()) - m_impl.begin();
        if ( len == npos )
        {
            *implLen = npos;
        }
        else         {
                                    const const_iterator e(end());
            const_iterator i(b);
            while ( len && i <= e )
            {
                ++i;
                --len;
            }

            *implLen = i.impl() - b.impl();
        }
    }
}

#endif 

#if 0



template<typename T>
static inline void DeleteStringFromConversionCache(T& hash, const wxString *s)
{
    typename T::iterator i = hash.find(wxConstCast(s, wxString));
    if ( i != hash.end() )
    {
        free(i->second);
        hash.erase(i);
    }
}

#if wxUSE_UNICODE
WX_DECLARE_HASH_MAP(wxString*, char*, wxPointerHash, wxPointerEqual,
                    wxStringCharConversionCache);
static wxStringCharConversionCache gs_stringsCharCache;

const char* wxCStrData::AsChar() const
{
        DeleteStringFromConversionCache(gs_stringsCharCache, m_str);

        const char *s = gs_stringsCharCache[wxConstCast(m_str, wxString)] =
        m_str->mb_str().release();

    return s + m_offset;
}
#endif 
#if !wxUSE_UNICODE_WCHAR
WX_DECLARE_HASH_MAP(wxString*, wchar_t*, wxPointerHash, wxPointerEqual,
                    wxStringWCharConversionCache);
static wxStringWCharConversionCache gs_stringsWCharCache;

const wchar_t* wxCStrData::AsWChar() const
{
        DeleteStringFromConversionCache(gs_stringsWCharCache, m_str);

        const wchar_t *s = gs_stringsWCharCache[wxConstCast(m_str, wxString)] =
        m_str->wc_str().release();

    return s + m_offset;
}
#endif 
wxString::~wxString()
{
#if wxUSE_UNICODE
        DeleteStringFromConversionCache(gs_stringsCharCache, this);
#endif
#if !wxUSE_UNICODE_WCHAR
    DeleteStringFromConversionCache(gs_stringsWCharCache, this);
#endif
}
#endif



#if wxUSE_UNICODE_WCHAR

wxString::SubstrBufFromMB wxString::ConvertStr(const char *psz, size_t nLength,
                                               const wxMBConv& conv)
{
        if ( !psz || nLength == 0 )
        return SubstrBufFromMB(wxWCharBuffer(L""), 0);

    if ( nLength == npos )
        nLength = wxNO_LEN;

    size_t wcLen;
    wxScopedWCharBuffer wcBuf(conv.cMB2WC(psz, nLength, &wcLen));
    if ( !wcLen )
        return SubstrBufFromMB(wxWCharBuffer(L""), 0);
    else
        return SubstrBufFromMB(wcBuf, wcLen);
}
#endif 
#if wxUSE_UNICODE_UTF8

wxString::SubstrBufFromMB wxString::ConvertStr(const char *psz, size_t nLength,
                                               const wxMBConv& conv)
{
        if ( !psz || nLength == 0 )
        return SubstrBufFromMB(wxCharBuffer(""), 0);

            if ( conv.IsUTF8() )
    {
                        if ( wxStringOperations::IsValidUtf8String(psz, nLength) )
        {
                        if ( nLength == npos )
                nLength = psz ? strlen(psz) : 0;
            return SubstrBufFromMB(wxScopedCharBuffer::CreateNonOwned(psz, nLength),
                                   nLength);
        }
            }

    if ( nLength == npos )
        nLength = wxNO_LEN;

        size_t wcLen;
    wxScopedWCharBuffer wcBuf(conv.cMB2WC(psz, nLength, &wcLen));
    if ( !wcLen )
        return SubstrBufFromMB(wxCharBuffer(""), 0);

        SubstrBufFromMB buf(ConvertStr(wcBuf, wcLen, wxMBConvStrictUTF8()));
        wxASSERT_MSG( buf.data, wxT("conversion to UTF-8 failed") );

    return buf;
}
#endif 
#if wxUSE_UNICODE_UTF8 || !wxUSE_UNICODE

wxString::SubstrBufFromWC wxString::ConvertStr(const wchar_t *pwz, size_t nLength,
                                               const wxMBConv& conv)
{
        if ( !pwz || nLength == 0 )
        return SubstrBufFromWC(wxCharBuffer(""), 0);

    if ( nLength == npos )
        nLength = wxNO_LEN;

    size_t mbLen;
    wxScopedCharBuffer mbBuf(conv.cWC2MB(pwz, nLength, &mbLen));
    if ( !mbLen )
        return SubstrBufFromWC(wxCharBuffer(""), 0);
    else
        return SubstrBufFromWC(mbBuf, mbLen);
}
#endif 
#if !wxUSE_UNICODE_WCHAR

const wchar_t *wxString::AsWChar(const wxMBConv& conv) const
{
    const char * const strMB = m_impl.c_str();
    const size_t lenMB = m_impl.length();

        const size_t lenWC = conv.ToWChar(NULL, 0, strMB, lenMB);
    if ( lenWC == wxCONV_FAILED )
        return NULL;

                    
                    if ( !m_convertedToWChar.m_str || lenWC != m_convertedToWChar.m_len )
    {
        if ( !const_cast<wxString *>(this)->m_convertedToWChar.Extend(lenWC) )
            return NULL;
    }

        m_convertedToWChar.m_str[lenWC] = L'\0';
    if ( conv.ToWChar(m_convertedToWChar.m_str, lenWC,
                      strMB, lenMB) == wxCONV_FAILED )
        return NULL;

    return m_convertedToWChar.m_str;
}

#endif 

#if wxUSE_UNICODE

const char *wxString::AsChar(const wxMBConv& conv) const
{
#if wxUSE_UNICODE_UTF8
    if ( conv.IsUTF8() )
        return m_impl.c_str();

    const wchar_t * const strWC = AsWChar(wxMBConvStrictUTF8());
    const size_t lenWC = m_convertedToWChar.m_len;
#else     const wchar_t * const strWC = m_impl.c_str();
    const size_t lenWC = m_impl.length();
#endif 
    const size_t lenMB = conv.FromWChar(NULL, 0, strWC, lenWC);
    if ( lenMB == wxCONV_FAILED )
        return NULL;

    if ( !m_convertedToChar.m_str || lenMB != m_convertedToChar.m_len )
    {
        if ( !const_cast<wxString *>(this)->m_convertedToChar.Extend(lenMB) )
            return NULL;
    }

    m_convertedToChar.m_str[lenMB] = '\0';
    if ( conv.FromWChar(m_convertedToChar.m_str, lenMB,
                        strWC, lenWC) == wxCONV_FAILED )
        return NULL;

    return m_convertedToChar.m_str;
}

#endif 
bool wxString::Shrink()
{
  wxString tmp(begin(), end());
  swap(tmp);
  return tmp.length() == length();
}

#if WXWIN_COMPATIBILITY_2_8 && !wxUSE_STL_BASED_WXSTRING && !wxUSE_UNICODE_UTF8
wxStringCharType *wxString::GetWriteBuf(size_t nLen)
{
    return DoGetWriteBuf(nLen);
}

void wxString::UngetWriteBuf()
{
    DoUngetWriteBuf();
}

void wxString::UngetWriteBuf(size_t nLen)
{
    DoUngetWriteBuf(nLen);
}
#endif 






wxString operator+(const wxString& str1, const wxString& str2)
{
#if !wxUSE_STL_BASED_WXSTRING
    wxASSERT( str1.IsValid() );
    wxASSERT( str2.IsValid() );
#endif

    wxString s = str1;
    s += str2;

    return s;
}

wxString operator+(const wxString& str, wxUniChar ch)
{
#if !wxUSE_STL_BASED_WXSTRING
    wxASSERT( str.IsValid() );
#endif

    wxString s = str;
    s += ch;

    return s;
}

wxString operator+(wxUniChar ch, const wxString& str)
{
#if !wxUSE_STL_BASED_WXSTRING
    wxASSERT( str.IsValid() );
#endif

    wxString s = ch;
    s += str;

    return s;
}

wxString operator+(const wxString& str, const char *psz)
{
#if !wxUSE_STL_BASED_WXSTRING
    wxASSERT( str.IsValid() );
#endif

    wxString s;
    if ( !s.Alloc(strlen(psz) + str.length()) ) {
        wxFAIL_MSG( wxT("out of memory in wxString::operator+") );
    }
    s += str;
    s += psz;

    return s;
}

wxString operator+(const wxString& str, const wchar_t *pwz)
{
#if !wxUSE_STL_BASED_WXSTRING
    wxASSERT( str.IsValid() );
#endif

    wxString s;
    if ( !s.Alloc(wxWcslen(pwz) + str.length()) ) {
        wxFAIL_MSG( wxT("out of memory in wxString::operator+") );
    }
    s += str;
    s += pwz;

    return s;
}

wxString operator+(const char *psz, const wxString& str)
{
#if !wxUSE_STL_BASED_WXSTRING
    wxASSERT( str.IsValid() );
#endif

    wxString s;
    if ( !s.Alloc(strlen(psz) + str.length()) ) {
        wxFAIL_MSG( wxT("out of memory in wxString::operator+") );
    }
    s = psz;
    s += str;

    return s;
}

wxString operator+(const wchar_t *pwz, const wxString& str)
{
#if !wxUSE_STL_BASED_WXSTRING
    wxASSERT( str.IsValid() );
#endif

    wxString s;
    if ( !s.Alloc(wxWcslen(pwz) + str.length()) ) {
        wxFAIL_MSG( wxT("out of memory in wxString::operator+") );
    }
    s = pwz;
    s += str;

    return s;
}


bool wxString::IsSameAs(wxUniChar c, bool compareWithCase) const
{
    return (length() == 1) && (compareWithCase ? GetChar(0u) == c
                               : wxToupper(GetChar(0u)) == wxToupper(c));
}

#ifdef HAVE_STD_STRING_COMPARE


int wxString::compare(const wxString& str) const
{
    return m_impl.compare(str.m_impl);
}

int wxString::compare(size_t nStart, size_t nLen,
                      const wxString& str) const
{
    size_t pos, len;
    PosLenToImpl(nStart, nLen, &pos, &len);
    return m_impl.compare(pos, len, str.m_impl);
}

int wxString::compare(size_t nStart, size_t nLen,
                      const wxString& str,
                      size_t nStart2, size_t nLen2) const
{
    size_t pos, len;
    PosLenToImpl(nStart, nLen, &pos, &len);

    size_t pos2, len2;
    str.PosLenToImpl(nStart2, nLen2, &pos2, &len2);

    return m_impl.compare(pos, len, str.m_impl, pos2, len2);
}

int wxString::compare(const char* sz) const
{
    return m_impl.compare(ImplStr(sz));
}

int wxString::compare(const wchar_t* sz) const
{
    return m_impl.compare(ImplStr(sz));
}

int wxString::compare(size_t nStart, size_t nLen,
                      const char* sz, size_t nCount) const
{
    size_t pos, len;
    PosLenToImpl(nStart, nLen, &pos, &len);

    SubstrBufFromMB str(ImplStr(sz, nCount));

    return m_impl.compare(pos, len, str.data, str.len);
}

int wxString::compare(size_t nStart, size_t nLen,
                      const wchar_t* sz, size_t nCount) const
{
    size_t pos, len;
    PosLenToImpl(nStart, nLen, &pos, &len);

    SubstrBufFromWC str(ImplStr(sz, nCount));

    return m_impl.compare(pos, len, str.data, str.len);
}

#else 
static inline int wxDoCmp(const wxStringCharType* s1, size_t l1,
                          const wxStringCharType* s2, size_t l2)
{
    if( l1 == l2 )
        return wxStringMemcmp(s1, s2, l1);
    else if( l1 < l2 )
    {
        int ret = wxStringMemcmp(s1, s2, l1);
        return ret == 0 ? -1 : ret;
    }
    else
    {
        int ret = wxStringMemcmp(s1, s2, l2);
        return ret == 0 ? +1 : ret;
    }
}

int wxString::compare(const wxString& str) const
{
    return ::wxDoCmp(m_impl.data(), m_impl.length(),
                     str.m_impl.data(), str.m_impl.length());
}

int wxString::compare(size_t nStart, size_t nLen,
                      const wxString& str) const
{
    wxASSERT(nStart <= length());
    size_type strLen = length() - nStart;
    nLen = strLen < nLen ? strLen : nLen;

    size_t pos, len;
    PosLenToImpl(nStart, nLen, &pos, &len);

    return ::wxDoCmp(m_impl.data() + pos,  len,
                     str.m_impl.data(), str.m_impl.length());
}

int wxString::compare(size_t nStart, size_t nLen,
                      const wxString& str,
                      size_t nStart2, size_t nLen2) const
{
    wxASSERT(nStart <= length());
    wxASSERT(nStart2 <= str.length());
    size_type strLen  =     length() - nStart,
              strLen2 = str.length() - nStart2;
    nLen  = strLen  < nLen  ? strLen  : nLen;
    nLen2 = strLen2 < nLen2 ? strLen2 : nLen2;

    size_t pos, len;
    PosLenToImpl(nStart, nLen, &pos, &len);
    size_t pos2, len2;
    str.PosLenToImpl(nStart2, nLen2, &pos2, &len2);

    return ::wxDoCmp(m_impl.data() + pos, len,
                     str.m_impl.data() + pos2, len2);
}

int wxString::compare(const char* sz) const
{
    SubstrBufFromMB str(ImplStr(sz, npos));
    if ( str.len == npos )
        str.len = wxStringStrlen(str.data);
    return ::wxDoCmp(m_impl.data(), m_impl.length(), str.data, str.len);
}

int wxString::compare(const wchar_t* sz) const
{
    SubstrBufFromWC str(ImplStr(sz, npos));
    if ( str.len == npos )
        str.len = wxStringStrlen(str.data);
    return ::wxDoCmp(m_impl.data(), m_impl.length(), str.data, str.len);
}

int wxString::compare(size_t nStart, size_t nLen,
                      const char* sz, size_t nCount) const
{
    wxASSERT(nStart <= length());
    size_type strLen = length() - nStart;
    nLen = strLen < nLen ? strLen : nLen;

    size_t pos, len;
    PosLenToImpl(nStart, nLen, &pos, &len);

    SubstrBufFromMB str(ImplStr(sz, nCount));
    if ( str.len == npos )
        str.len = wxStringStrlen(str.data);

    return ::wxDoCmp(m_impl.data() + pos, len, str.data, str.len);
}

int wxString::compare(size_t nStart, size_t nLen,
                      const wchar_t* sz, size_t nCount) const
{
    wxASSERT(nStart <= length());
    size_type strLen = length() - nStart;
    nLen = strLen < nLen ? strLen : nLen;

    size_t pos, len;
    PosLenToImpl(nStart, nLen, &pos, &len);

    SubstrBufFromWC str(ImplStr(sz, nCount));
    if ( str.len == npos )
        str.len = wxStringStrlen(str.data);

    return ::wxDoCmp(m_impl.data() + pos, len, str.data, str.len);
}

#endif 


#if !wxUSE_STL_BASED_WXSTRING || wxUSE_UNICODE_UTF8


size_t wxString::find_first_of(const wxChar* sz, size_t nStart) const
{
    return find_first_of(sz, nStart, wxStrlen(sz));
}

size_t wxString::find_first_not_of(const wxChar* sz, size_t nStart) const
{
    return find_first_not_of(sz, nStart, wxStrlen(sz));
}

size_t wxString::find_first_of(const wxChar* sz, size_t nStart, size_t n) const
{
    wxASSERT_MSG( nStart <= length(),  wxT("invalid index") );

    size_t idx = nStart;
    for ( const_iterator i = begin() + nStart; i != end(); ++idx, ++i )
    {
        if ( wxTmemchr(sz, *i, n) )
            return idx;
    }

    return npos;
}

size_t wxString::find_first_not_of(const wxChar* sz, size_t nStart, size_t n) const
{
    wxASSERT_MSG( nStart <= length(),  wxT("invalid index") );

    size_t idx = nStart;
    for ( const_iterator i = begin() + nStart; i != end(); ++idx, ++i )
    {
        if ( !wxTmemchr(sz, *i, n) )
            return idx;
    }

    return npos;
}


size_t wxString::find_last_of(const wxChar* sz, size_t nStart) const
{
    return find_last_of(sz, nStart, wxStrlen(sz));
}

size_t wxString::find_last_not_of(const wxChar* sz, size_t nStart) const
{
    return find_last_not_of(sz, nStart, wxStrlen(sz));
}

size_t wxString::find_last_of(const wxChar* sz, size_t nStart, size_t n) const
{
    size_t len = length();

    if ( nStart == npos )
    {
        nStart = len - 1;
    }
    else
    {
        wxASSERT_MSG( nStart <= len, wxT("invalid index") );
    }

    size_t idx = nStart;
    for ( const_reverse_iterator i = rbegin() + (len - nStart - 1);
          i != rend(); --idx, ++i )
    {
        if ( wxTmemchr(sz, *i, n) )
            return idx;
    }

    return npos;
}

size_t wxString::find_last_not_of(const wxChar* sz, size_t nStart, size_t n) const
{
    size_t len = length();

    if ( nStart == npos )
    {
        nStart = len - 1;
    }
    else
    {
        wxASSERT_MSG( nStart <= len, wxT("invalid index") );
    }

    size_t idx = nStart;
    for ( const_reverse_iterator i = rbegin() + (len - nStart - 1);
          i != rend(); --idx, ++i )
    {
        if ( !wxTmemchr(sz, *i, n) )
            return idx;
    }

    return npos;
}

size_t wxString::find_first_not_of(wxUniChar ch, size_t nStart) const
{
    wxASSERT_MSG( nStart <= length(),  wxT("invalid index") );

    size_t idx = nStart;
    for ( const_iterator i = begin() + nStart; i != end(); ++idx, ++i )
    {
        if ( *i != ch )
            return idx;
    }

    return npos;
}

size_t wxString::find_last_not_of(wxUniChar ch, size_t nStart) const
{
    size_t len = length();

    if ( nStart == npos )
    {
        nStart = len - 1;
    }
    else
    {
        wxASSERT_MSG( nStart <= len, wxT("invalid index") );
    }

    size_t idx = nStart;
    for ( const_reverse_iterator i = rbegin() + (len - nStart - 1);
          i != rend(); --idx, ++i )
    {
        if ( *i != ch )
            return idx;
    }

    return npos;
}

#if wxUSE_UNICODE
    #define wxOtherCharType char
    #define STRCONV         (const wxChar*)wxConvLibc.cMB2WC
#else
    #define wxOtherCharType wchar_t
    #define STRCONV         (const wxChar*)wxConvLibc.cWC2MB
#endif

size_t wxString::find_first_of(const wxOtherCharType* sz, size_t nStart) const
    { return find_first_of(STRCONV(sz), nStart); }

size_t wxString::find_first_of(const wxOtherCharType* sz, size_t nStart,
                               size_t n) const
    { return find_first_of(STRCONV(sz, n, NULL), nStart, n); }
size_t wxString::find_last_of(const wxOtherCharType* sz, size_t nStart) const
    { return find_last_of(STRCONV(sz), nStart); }
size_t wxString::find_last_of(const wxOtherCharType* sz, size_t nStart,
                              size_t n) const
    { return find_last_of(STRCONV(sz, n, NULL), nStart, n); }
size_t wxString::find_first_not_of(const wxOtherCharType* sz, size_t nStart) const
    { return find_first_not_of(STRCONV(sz), nStart); }
size_t wxString::find_first_not_of(const wxOtherCharType* sz, size_t nStart,
                                   size_t n) const
    { return find_first_not_of(STRCONV(sz, n, NULL), nStart, n); }
size_t wxString::find_last_not_of(const wxOtherCharType* sz, size_t nStart) const
    { return find_last_not_of(STRCONV(sz), nStart); }
size_t wxString::find_last_not_of(const wxOtherCharType* sz, size_t nStart,
                                  size_t n) const
    { return find_last_not_of(STRCONV(sz, n, NULL), nStart, n); }

#undef wxOtherCharType
#undef STRCONV

#endif 

int wxString::CmpNoCase(const wxString& s) const
{
#if !wxUSE_UNICODE_UTF8
            
    typedef const wxStringImpl::value_type *pchar_type;
    const pchar_type thisBegin = m_impl.c_str();
    const pchar_type thatBegin = s.m_impl.c_str();

    const pchar_type thisEnd = thisBegin + m_impl.length();
    const pchar_type thatEnd = thatBegin + s.m_impl.length();

    pchar_type thisCur = thisBegin;
    pchar_type thatCur = thatBegin;

    int rc;
    for ( ;; )
    {
                        rc = wxStricmp(thisCur, thatCur);
        if ( rc )
            break;

        const size_t lenChunk = wxStrlen(thisCur);
        thisCur += lenChunk;
        thatCur += lenChunk;

                for ( ; !*thisCur; thisCur++, thatCur++ )
        {
                        if ( thisCur == thisEnd )
            {
                                return thatCur == thatEnd ? 0 : -1;
            }

            if ( thatCur == thatEnd )
            {
                                                return 1;
            }

            if ( *thatCur )
            {
                                return -1;
            }
        }
    }

    return rc;
#else         
        const_iterator i1 = begin();
    const_iterator end1 = end();
    const_iterator i2 = s.begin();
    const_iterator end2 = s.end();

    for ( ; i1 != end1 && i2 != end2; ++i1, ++i2 )
    {
        wxUniChar lower1 = (wxChar)wxTolower(*i1);
        wxUniChar lower2 = (wxChar)wxTolower(*i2);
        if ( lower1 != lower2 )
            return lower1 < lower2 ? -1 : 1;
    }

    size_t len1 = length();
    size_t len2 = s.length();

    if ( len1 < len2 )
        return -1;
    else if ( len1 > len2 )
        return 1;
    return 0;
#endif }


#if wxUSE_UNICODE

wxString wxString::FromAscii(const char *ascii, size_t len)
{
    if (!ascii || len == 0)
       return wxEmptyString;

    wxString res;

    {
        wxStringInternalBuffer buf(res, len);
        wxStringCharType *dest = buf;

        for ( ; len > 0; --len )
        {
            unsigned char c = (unsigned char)*ascii++;
            wxASSERT_MSG( c < 0x80,
                          wxT("Non-ASCII value passed to FromAscii().") );

            *dest++ = static_cast<wxStringCharType>(c);
        }
    }

    return res;
}

wxString wxString::FromAscii(const char *ascii)
{
    return FromAscii(ascii, wxStrlen(ascii));
}

wxString wxString::FromAscii(char ascii)
{
    
    unsigned char c = (unsigned char)ascii;

    wxASSERT_MSG( c < 0x80, wxT("Non-ASCII value passed to FromAscii().") );

        return wxString(wxUniChar((wchar_t)c));
}

const wxScopedCharBuffer wxString::ToAscii(char replaceWith) const
{
        wxCharBuffer buffer(length());
    char *dest = buffer.data();

    for ( const_iterator i = begin(); i != end(); ++i )
    {
        wxUniChar c(*i);
                *dest++ = c.IsAscii() ? (char)c : replaceWith;

                        if ( !c )
            break;
    }

    return buffer;
}

#endif 
wxString wxString::Mid(size_t nFirst, size_t nCount) const
{
    size_t nLen = length();

        if ( nCount == npos )
    {
        nCount = nLen - nFirst;
    }

        if ( nFirst > nLen )
    {
                return wxEmptyString;
    }

    if ( nCount > nLen - nFirst )
    {
        nCount = nLen - nFirst;
    }

    wxString dest(*this, nFirst, nCount);
    if ( dest.length() != nCount )
    {
        wxFAIL_MSG( wxT("out of memory in wxString::Mid") );
    }

    return dest;
}

bool wxString::StartsWith(const wxString& prefix, wxString *rest) const
{
    if ( compare(0, prefix.length(), prefix) != 0 )
        return false;

    if ( rest )
    {
                rest->assign(*this, prefix.length(), npos);
    }

    return true;
}


bool wxString::EndsWith(const wxString& suffix, wxString *rest) const
{
    int start = length() - suffix.length();

    if ( start < 0 || compare(start, npos, suffix) != 0 )
        return false;

    if ( rest )
    {
                rest->assign(*this, 0, start);
    }

    return true;
}


wxString wxString::Right(size_t nCount) const
{
  if ( nCount > length() )
    nCount = length();

  wxString dest(*this, length() - nCount, nCount);
  if ( dest.length() != nCount ) {
    wxFAIL_MSG( wxT("out of memory in wxString::Right") );
  }
  return dest;
}

wxString wxString::AfterLast(wxUniChar ch) const
{
  wxString str;
  int iPos = Find(ch, true);
  if ( iPos == wxNOT_FOUND )
    str = *this;
  else
    str.assign(*this, iPos + 1, npos);

  return str;
}

wxString wxString::Left(size_t nCount) const
{
  if ( nCount > length() )
    nCount = length();

  wxString dest(*this, 0, nCount);
  if ( dest.length() != nCount ) {
    wxFAIL_MSG( wxT("out of memory in wxString::Left") );
  }
  return dest;
}

wxString wxString::BeforeFirst(wxUniChar ch, wxString *rest) const
{
  int iPos = Find(ch);
  if ( iPos == wxNOT_FOUND )
  {
    iPos = length();
    if ( rest )
      rest->clear();
  }
  else
  {
    if ( rest )
      rest->assign(*this, iPos + 1, npos);
  }

  return wxString(*this, 0, iPos);
}

wxString wxString::BeforeLast(wxUniChar ch, wxString *rest) const
{
  wxString str;
  int iPos = Find(ch, true);
  if ( iPos != wxNOT_FOUND )
  {
    if ( iPos != 0 )
      str.assign(*this, 0, iPos);

    if ( rest )
      rest->assign(*this, iPos + 1, npos);
  }
  else
  {
    if ( rest )
      *rest = *this;
  }

  return str;
}

wxString wxString::AfterFirst(wxUniChar ch) const
{
  wxString str;
  int iPos = Find(ch);
  if ( iPos != wxNOT_FOUND )
      str.assign(*this, iPos + 1, npos);

  return str;
}

size_t wxString::Replace(const wxString& strOld,
                         const wxString& strNew, bool bReplaceAll)
{
        wxCHECK_MSG( !strOld.empty(), 0,
                 wxT("wxString::Replace(): invalid parameter") );

    wxSTRING_INVALIDATE_CACHE();

    size_t uiCount = 0;   
                            if ( strOld.m_impl.length() == 1 && strNew.m_impl.length() == 1 )
    {
        const wxStringCharType chOld = strOld.m_impl[0],
                               chNew = strNew.m_impl[0];

                for ( size_t pos = 0; ; )
        {
            pos = m_impl.find(chOld, pos);
            if ( pos == npos )
                break;

            m_impl[pos++] = chNew;

            uiCount++;

            if ( !bReplaceAll )
                break;
        }
    }
    else if ( !bReplaceAll)
    {
        size_t pos = m_impl.find(strOld.m_impl, 0);
        if ( pos != npos )
        {
            m_impl.replace(pos, strOld.m_impl.length(), strNew.m_impl);
            uiCount = 1;
        }
    }
    else     {
        const size_t uiOldLen = strOld.m_impl.length();
        const size_t uiNewLen = strNew.m_impl.length();

                        wxVector<size_t> replacePositions;

        size_t pos;
        for ( pos = m_impl.find(strOld.m_impl, 0);
              pos != npos;
              pos = m_impl.find(strOld.m_impl, pos + uiOldLen))
        {
            replacePositions.push_back(pos);
            ++uiCount;
        }

        if ( !uiCount )
            return 0;

                wxString tmp;
        tmp.m_impl.reserve(m_impl.length() + uiCount*(uiNewLen - uiOldLen));

                size_t replNum = 0;
        for ( pos = 0; replNum < uiCount; replNum++ )
        {
            const size_t nextReplPos = replacePositions[replNum];

            if ( pos != nextReplPos )
            {
                tmp.m_impl.append(m_impl, pos, nextReplPos - pos);
            }

            tmp.m_impl.append(strNew.m_impl);
            pos = nextReplPos + uiOldLen;
        }

        if ( pos != m_impl.length() )
        {
                        tmp.m_impl.append(m_impl, pos, m_impl.length() - pos);
        }

        swap(tmp);
    }

    return uiCount;
}

bool wxString::IsAscii() const
{
    for ( const_iterator i = begin(); i != end(); ++i )
    {
        if ( !(*i).IsAscii() )
            return false;
    }

    return true;
}

bool wxString::IsWord() const
{
    for ( const_iterator i = begin(); i != end(); ++i )
    {
        if ( !wxIsalpha(*i) )
            return false;
    }

    return true;
}

bool wxString::IsNumber() const
{
    if ( empty() )
        return true;

    const_iterator i = begin();

    if ( *i == wxT('-') || *i == wxT('+') )
        ++i;

    for ( ; i != end(); ++i )
    {
        if ( !wxIsdigit(*i) )
            return false;
    }

    return true;
}

wxString wxString::Strip(stripType w) const
{
    wxString s = *this;
    if ( w & leading ) s.Trim(false);
    if ( w & trailing ) s.Trim(true);
    return s;
}


wxString& wxString::MakeUpper()
{
  for ( iterator it = begin(), en = end(); it != en; ++it )
    *it = (wxChar)wxToupper(*it);

  return *this;
}

wxString& wxString::MakeLower()
{
  for ( iterator it = begin(), en = end(); it != en; ++it )
    *it = (wxChar)wxTolower(*it);

  return *this;
}

wxString& wxString::MakeCapitalized()
{
    const iterator en = end();
    iterator it = begin();
    if ( it != en )
    {
        *it = (wxChar)wxToupper(*it);
        for ( ++it; it != en; ++it )
            *it = (wxChar)wxTolower(*it);
    }

    return *this;
}


inline int wxSafeIsspace(wxChar ch) { return (ch < 127) && wxIsspace(ch); }

wxString& wxString::Trim(bool bFromRight)
{
        if ( !empty() &&
         (
          (bFromRight && wxSafeIsspace(GetChar(length() - 1))) ||
          (!bFromRight && wxSafeIsspace(GetChar(0u)))
         )
       )
    {
        if ( bFromRight )
        {
                        reverse_iterator psz = rbegin();
            while ( (psz != rend()) && wxSafeIsspace(*psz) )
                ++psz;

                        erase(psz.base(), end());
        }
        else
        {
                        iterator psz = begin();
            while ( (psz != end()) && wxSafeIsspace(*psz) )
                ++psz;

                        erase(begin(), psz);
        }
    }

    return *this;
}

wxString& wxString::Pad(size_t nCount, wxUniChar chPad, bool bFromRight)
{
    wxString s(chPad, nCount);

    if ( bFromRight )
        *this += s;
    else
    {
        s += *this;
        swap(s);
    }

    return *this;
}

wxString& wxString::Truncate(size_t uiLen)
{
    if ( uiLen < length() )
    {
        erase(begin() + uiLen, end());
    }
    
    return *this;
}


int wxString::Find(wxUniChar ch, bool bFromEnd) const
{
    size_type idx = bFromEnd ? find_last_of(ch) : find_first_of(ch);

    return (idx == npos) ? wxNOT_FOUND : (int)idx;
}



#define WX_STRING_TO_X_TYPE_START                                           \
    wxCHECK_MSG( pVal, false, wxT("NULL output pointer") );                  \
    errno = 0;                                                              \
    const wxStringCharType *start = wx_str();                               \
    wxStringCharType *end;

#define WX_STRING_TO_X_TYPE_END                                             \
    if ( end == start || errno == ERANGE )                                  \
        return false;                                                       \
    *pVal = val;                                                            \
    return !*end;

bool wxString::ToLong(long *pVal, int base) const
{
    wxASSERT_MSG( !base || (base > 1 && base <= 36), wxT("invalid base") );

    WX_STRING_TO_X_TYPE_START
    long val = wxStrtol(start, &end, base);
    WX_STRING_TO_X_TYPE_END
}

bool wxString::ToULong(unsigned long *pVal, int base) const
{
    wxASSERT_MSG( !base || (base > 1 && base <= 36), wxT("invalid base") );

    WX_STRING_TO_X_TYPE_START
    unsigned long val = wxStrtoul(start, &end, base);
    WX_STRING_TO_X_TYPE_END
}

bool wxString::ToLongLong(wxLongLong_t *pVal, int base) const
{
    wxASSERT_MSG( !base || (base > 1 && base <= 36), wxT("invalid base") );

    WX_STRING_TO_X_TYPE_START
    wxLongLong_t val = wxStrtoll(start, &end, base);
    WX_STRING_TO_X_TYPE_END
}

bool wxString::ToULongLong(wxULongLong_t *pVal, int base) const
{
    wxASSERT_MSG( !base || (base > 1 && base <= 36), wxT("invalid base") );

    WX_STRING_TO_X_TYPE_START
    wxULongLong_t val = wxStrtoull(start, &end, base);
    WX_STRING_TO_X_TYPE_END
}

bool wxString::ToDouble(double *pVal) const
{
    WX_STRING_TO_X_TYPE_START
    double val = wxStrtod(start, &end);
    WX_STRING_TO_X_TYPE_END
}

#if wxUSE_XLOCALE

bool wxString::ToCLong(long *pVal, int base) const
{
    wxASSERT_MSG( !base || (base > 1 && base <= 36), wxT("invalid base") );

    WX_STRING_TO_X_TYPE_START
#if (wxUSE_UNICODE_UTF8 || !wxUSE_UNICODE) && defined(wxHAS_XLOCALE_SUPPORT)
    long val = wxStrtol_lA(start, &end, base, wxCLocale);
#else
    long val = wxStrtol_l(start, &end, base, wxCLocale);
#endif
    WX_STRING_TO_X_TYPE_END
}

bool wxString::ToCULong(unsigned long *pVal, int base) const
{
    wxASSERT_MSG( !base || (base > 1 && base <= 36), wxT("invalid base") );

    WX_STRING_TO_X_TYPE_START
#if (wxUSE_UNICODE_UTF8 || !wxUSE_UNICODE) && defined(wxHAS_XLOCALE_SUPPORT)
    unsigned long val = wxStrtoul_lA(start, &end, base, wxCLocale);
#else
    unsigned long val = wxStrtoul_l(start, &end, base, wxCLocale);
#endif
    WX_STRING_TO_X_TYPE_END
}

bool wxString::ToCDouble(double *pVal) const
{
    WX_STRING_TO_X_TYPE_START
#if (wxUSE_UNICODE_UTF8 || !wxUSE_UNICODE) && defined(wxHAS_XLOCALE_SUPPORT)
    double val = wxStrtod_lA(start, &end, wxCLocale);
#else
    double val = wxStrtod_l(start, &end, wxCLocale);
#endif
    WX_STRING_TO_X_TYPE_END
}

#else 

bool wxString::ToCLong(long *pVal, int base) const
{
    return ToLong(pVal, base);
}

bool wxString::ToCULong(unsigned long *pVal, int base) const
{
    return ToULong(pVal, base);
}

bool wxString::ToCDouble(double *pVal) const
{
    
        #if wxUSE_INTL
    wxString sep = wxLocale::GetInfo(wxLOCALE_DECIMAL_POINT,
                                     wxLOCALE_CAT_NUMBER);
    if ( sep == "." )
    {
                return ToDouble(pVal);
    }
#else             if ( ToDouble(pVal) )
    {
                return true;
    }

        wxString sep(",");
#endif     wxString cstr(*this);
    cstr.Replace(".", sep);

    return cstr.ToDouble(pVal);
}

#endif  


wxString wxString::FromDouble(double val, int precision)
{
    wxCHECK_MSG( precision >= -1, wxString(), "Invalid negative precision" );

    wxString format;
    if ( precision == -1 )
    {
        format = "%g";
    }
    else     {
        format.Printf("%%.%df", precision);
    }

    return wxString::Format(format, val);
}


wxString wxString::FromCDouble(double val, int precision)
{
    wxCHECK_MSG( precision >= -1, wxString(), "Invalid negative precision" );

                                                
    wxString s = FromDouble(val, precision);
#if wxUSE_INTL
    wxString sep = wxLocale::GetInfo(wxLOCALE_DECIMAL_POINT,
                                     wxLOCALE_CAT_NUMBER);
#else                 wxString sep(",");
#endif 
    s.Replace(sep, ".");
    return s;
}


#if !wxUSE_UTF8_LOCALE_ONLY

#ifdef wxNEEDS_WXSTRING_PRINTF_MIXIN
wxString wxStringPrintfMixinBase::DoFormatWchar(const wxChar *format, ...)
#else
wxString wxString::DoFormatWchar(const wxChar *format, ...)
#endif
{
    va_list argptr;
    va_start(argptr, format);

    wxString s;
    s.PrintfV(format, argptr);

    va_end(argptr);

    return s;
}
#endif 
#if wxUSE_UNICODE_UTF8

wxString wxString::DoFormatUtf8(const char *format, ...)
{
    va_list argptr;
    va_start(argptr, format);

    wxString s;
    s.PrintfV(format, argptr);

    va_end(argptr);

    return s;
}
#endif 

wxString wxString::FormatV(const wxString& format, va_list argptr)
{
    wxString s;
    s.PrintfV(format, argptr);
    return s;
}

#if !wxUSE_UTF8_LOCALE_ONLY
#ifdef wxNEEDS_WXSTRING_PRINTF_MIXIN
int wxStringPrintfMixinBase::DoPrintfWchar(const wxChar *format, ...)
#else
int wxString::DoPrintfWchar(const wxChar *format, ...)
#endif
{
    va_list argptr;
    va_start(argptr, format);

#ifdef wxNEEDS_WXSTRING_PRINTF_MIXIN
                wxString *str = static_cast<wxString*>(this);
#else
    wxString *str = this;
#endif

    int iLen = str->PrintfV(format, argptr);

    va_end(argptr);

    return iLen;
}
#endif 
#if wxUSE_UNICODE_UTF8
int wxString::DoPrintfUtf8(const char *format, ...)
{
    va_list argptr;
    va_start(argptr, format);

    int iLen = PrintfV(format, argptr);

    va_end(argptr);

    return iLen;
}
#endif 

#if wxUSE_UNICODE_UTF8
template<typename BufferType>
#else
#endif
static int DoStringPrintfV(wxString& str,
                           const wxString& format, va_list argptr)
{
    int size = 1024;

    for ( ;; )
    {
#if wxUSE_UNICODE_UTF8
        BufferType tmp(str, size + 1);
        typename BufferType::CharType *buf = tmp;
#else
        wxStringBuffer tmp(str, size + 1);
        wxChar *buf = tmp;
#endif

        if ( !buf )
        {
                        return -1;
        }

                        va_list argptrcopy;
        wxVaCopy(argptrcopy, argptr);

                errno = 0;
        int len = wxVsnprintf(buf, size, format, argptrcopy);
        va_end(argptrcopy);

                                                buf[size] = wxT('\0');

                                if ( len < 0 )
        {
                                                #if wxUSE_WXVSNPRINTF
                                                buf[0] = '\0';
            return -1;
#else                                                 if( (errno == EILSEQ) || (errno == EINVAL) )
                                        return -1;
            else
                                        size *= 2;
#endif         }
        else if ( len >= size )
        {
#if wxUSE_WXVSNPRINTF
                                                size *= 2;      #else
                                                                                                size = len + 1;
#endif
        }
        else         {
            break;
        }
    }

        str.Shrink();

    return str.length();
}

int wxString::PrintfV(const wxString& format, va_list argptr)
{
#if wxUSE_UNICODE_UTF8
    #if wxUSE_STL_BASED_WXSTRING
        typedef wxStringTypeBuffer<char> Utf8Buffer;
    #else
        typedef wxStringInternalBuffer Utf8Buffer;
    #endif
#endif

#if wxUSE_UTF8_LOCALE_ONLY
    return DoStringPrintfV<Utf8Buffer>(*this, format, argptr);
#else
    #if wxUSE_UNICODE_UTF8
    if ( wxLocaleIsUtf8 )
        return DoStringPrintfV<Utf8Buffer>(*this, format, argptr);
    else
                return DoStringPrintfV<wxStringBuffer>(*this, format, argptr);
    #else
        return DoStringPrintfV(*this, format, argptr);
    #endif #endif
}


bool wxString::Matches(const wxString& mask) const
{
                #if 0         wxString pattern;
    pattern.reserve(wxStrlen(pszMask));

    pattern += wxT('^');
    while ( *pszMask )
    {
        switch ( *pszMask )
        {
            case wxT('?'):
                pattern += wxT('.');
                break;

            case wxT('*'):
                pattern += wxT(".*");
                break;

            case wxT('^'):
            case wxT('.'):
            case wxT('$'):
            case wxT('('):
            case wxT(')'):
            case wxT('|'):
            case wxT('+'):
            case wxT('\\'):
                                                                pattern += wxT('\\');
                wxFALLTHROUGH;

            default:
                pattern += *pszMask;
        }

        pszMask++;
    }
    pattern += wxT('$');

        return wxRegEx(pattern, wxRE_NOSUB | wxRE_EXTENDED).Matches(c_str());
#else   
  #if wxUSE_UNICODE_UTF8
  const wxScopedWCharBuffer maskBuf = mask.wc_str();
  const wxScopedWCharBuffer txtBuf = wc_str();
  const wxChar *pszMask = maskBuf.data();
  const wxChar *pszTxt = txtBuf.data();
#else
  const wxChar *pszMask = mask.wx_str();
    const wxChar *pszTxt = wx_str();
#endif

    const wxChar *pszLastStarInText = NULL;
  const wxChar *pszLastStarInMask = NULL;

match:
  for ( ; *pszMask != wxT('\0'); pszMask++, pszTxt++ ) {
    switch ( *pszMask ) {
      case wxT('?'):
        if ( *pszTxt == wxT('\0') )
          return false;

        
        break;

      case wxT('*'):
        {
                    pszLastStarInText = pszTxt;
          pszLastStarInMask = pszMask;

                              while ( *pszMask == wxT('*') || *pszMask == wxT('?') )
            pszMask++;

                    if ( *pszMask == wxT('\0') )
            return true;

                    size_t uiLenMask;
          const wxChar *pEndMask = wxStrpbrk(pszMask, wxT("*?"));

          if ( pEndMask != NULL ) {
                        uiLenMask = pEndMask - pszMask;
          }
          else {
                        uiLenMask = wxStrlen(pszMask);
          }

          wxString strToMatch(pszMask, uiLenMask);
          const wxChar* pMatch = wxStrstr(pszTxt, strToMatch);
          if ( pMatch == NULL )
            return false;

                    pszTxt = pMatch + uiLenMask - 1;
          pszMask += uiLenMask - 1;
        }
        break;

      default:
        if ( *pszMask != *pszTxt )
          return false;
        break;
    }
  }

    if ( *pszTxt == wxT('\0') )
    return true;

    if ( pszLastStarInText ) {
    pszTxt = pszLastStarInText + 1;
    pszMask = pszLastStarInMask;

    pszLastStarInText = NULL;

    
    goto match;
  }

  return false;
#endif }

int wxString::Freq(wxUniChar ch) const
{
    int count = 0;
    for ( const_iterator i = begin(); i != end(); ++i )
    {
        if ( *i == ch )
            count ++;
    }
    return count;
}

