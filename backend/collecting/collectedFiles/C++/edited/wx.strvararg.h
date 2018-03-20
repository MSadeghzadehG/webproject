
#ifndef _WX_STRVARARG_H_
#define _WX_STRVARARG_H_

#include "wx/platform.h"

#include "wx/cpp.h"
#include "wx/chartype.h"
#include "wx/strconv.h"
#include "wx/buffer.h"
#include "wx/unichar.h"

#if defined(HAVE_TYPE_TRAITS)
    #include <type_traits>
#elif defined(HAVE_TR1_TYPE_TRAITS)
    #ifdef __VISUALC__
        #include <type_traits>
    #else
        #include <tr1/type_traits>
    #endif
#endif

class WXDLLIMPEXP_FWD_BASE wxCStrData;
class WXDLLIMPEXP_FWD_BASE wxString;


#define WX_DEFINE_VARARG_FUNC(rettype, name, numfixed, fixed, impl, implUtf8) \
    _WX_VARARG_DEFINE_FUNC_N0(rettype, name, impl, implUtf8, numfixed, fixed) \
    WX_DEFINE_VARARG_FUNC_SANS_N0(rettype, name, numfixed, fixed, impl, implUtf8)

#define WX_DEFINE_VARARG_FUNC_SANS_N0(rettype, name,                          \
                                       numfixed, fixed, impl, implUtf8)       \
    _WX_VARARG_ITER(_WX_VARARG_MAX_ARGS,                                      \
                    _WX_VARARG_DEFINE_FUNC,                                   \
                    rettype, name, impl, implUtf8, numfixed, fixed)

#define WX_DEFINE_VARARG_FUNC_VOID(name, numfixed, fixed, impl, implUtf8)     \
    _WX_VARARG_DEFINE_FUNC_VOID_N0(name, impl, implUtf8, numfixed, fixed)     \
    _WX_VARARG_ITER(_WX_VARARG_MAX_ARGS,                                      \
                    _WX_VARARG_DEFINE_FUNC_VOID,                              \
                    void, name, impl, implUtf8, numfixed, fixed)

#define WX_DEFINE_VARARG_FUNC_NOP(name, numfixed, fixed)                      \
        _WX_VARARG_DEFINE_FUNC_NOP_N0(name, numfixed, fixed)                  \
        _WX_VARARG_ITER(_WX_VARARG_MAX_ARGS,                                  \
                        _WX_VARARG_DEFINE_FUNC_NOP,                           \
                        void, name, dummy, dummy, numfixed, fixed)

#define WX_DEFINE_VARARG_FUNC_CTOR(name, numfixed, fixed, impl, implUtf8)     \
    _WX_VARARG_DEFINE_FUNC_CTOR_N0(name, impl, implUtf8, numfixed, fixed)     \
    _WX_VARARG_ITER(_WX_VARARG_MAX_ARGS,                                      \
                    _WX_VARARG_DEFINE_FUNC_CTOR,                              \
                    void, name, impl, implUtf8, numfixed, fixed)



class WXDLLIMPEXP_BASE wxFormatString
{
public:
    wxFormatString(const char *str)
        : m_char(wxScopedCharBuffer::CreateNonOwned(str)), m_str(NULL), m_cstr(NULL) {}
    wxFormatString(const wchar_t *str)
        : m_wchar(wxScopedWCharBuffer::CreateNonOwned(str)), m_str(NULL), m_cstr(NULL) {}
    wxFormatString(const wxString& str)
        : m_str(&str), m_cstr(NULL) {}
    wxFormatString(const wxCStrData& str)
        : m_str(NULL), m_cstr(&str) {}
    wxFormatString(const wxScopedCharBuffer& str)
        : m_char(str), m_str(NULL), m_cstr(NULL)  {}
    wxFormatString(const wxScopedWCharBuffer& str)
        : m_wchar(str), m_str(NULL), m_cstr(NULL) {}

                    enum ArgumentType
    {
        Arg_Unused      = 0,                              
        Arg_Char        = 0x0001,            Arg_Pointer     = 0x0002,            Arg_String      = 0x0004 | Arg_Pointer, 
        Arg_Int         = 0x0008 | Arg_Char, #if SIZEOF_INT == SIZEOF_LONG
        Arg_LongInt     = Arg_Int,
#else
        Arg_LongInt     = 0x0010,
#endif
#if defined(SIZEOF_LONG_LONG) && SIZEOF_LONG_LONG == SIZEOF_LONG
        Arg_LongLongInt = Arg_LongInt,
#elif defined(wxLongLong_t)
        Arg_LongLongInt = 0x0020,
#endif

        Arg_Double      = 0x0040,
        Arg_LongDouble  = 0x0080,

#if defined(wxSIZE_T_IS_UINT)
        Arg_Size_t      = Arg_Int,
#elif defined(wxSIZE_T_IS_ULONG)
        Arg_Size_t      = Arg_LongInt,
#elif defined(SIZEOF_LONG_LONG) && SIZEOF_SIZE_T == SIZEOF_LONG_LONG
        Arg_Size_t      = Arg_LongLongInt,
#else
        Arg_Size_t      = 0x0100,
#endif

        Arg_IntPtr      = 0x0200,            Arg_ShortIntPtr = 0x0400,
        Arg_LongIntPtr  = 0x0800,

        Arg_Unknown     = 0x8000         };

                    ArgumentType GetArgumentType(unsigned n) const;

            wxString InputAsString() const;

#if !wxUSE_UNICODE_WCHAR
    operator const char*() const
        { return const_cast<wxFormatString*>(this)->AsChar(); }
private:
                    const char* InputAsChar();
    const char* AsChar();
    wxScopedCharBuffer m_convertedChar;
#endif 
#if wxUSE_UNICODE && !wxUSE_UTF8_LOCALE_ONLY
public:
    operator const wchar_t*() const
        { return const_cast<wxFormatString*>(this)->AsWChar(); }
private:
    const wchar_t* InputAsWChar();
    const wchar_t* AsWChar();
    wxScopedWCharBuffer m_convertedWChar;
#endif 
private:
    wxScopedCharBuffer  m_char;
    wxScopedWCharBuffer m_wchar;

                const wxString * const m_str;
    const wxCStrData * const m_cstr;

    wxDECLARE_NO_ASSIGN_CLASS(wxFormatString);
};

struct wxFormatStringArgument
{
    wxFormatStringArgument(const wxFormatString *s = NULL) : m_str(s) {}
    const wxFormatString *m_str;

        wxFormatStringArgument operator,(const wxFormatStringArgument& a) const
    {
        wxASSERT_MSG( m_str == NULL || a.m_str == NULL,
                      "can't have two format strings in vararg function" );
        return wxFormatStringArgument(m_str ? m_str : a.m_str);
    }

    operator const wxFormatString*() const { return m_str; }
};

template<typename T>
struct wxFormatStringArgumentFinder
{
    static wxFormatStringArgument find(T)
    {
                return wxFormatStringArgument();
    }
};

template<>
struct wxFormatStringArgumentFinder<const wxFormatString&>
{
    static wxFormatStringArgument find(const wxFormatString& arg)
        { return wxFormatStringArgument(&arg); }
};

template<>
struct wxFormatStringArgumentFinder<wxFormatString>
    : public wxFormatStringArgumentFinder<const wxFormatString&> {};

template<>
struct wxFormatStringArgumentFinder<wxString>
    : public wxFormatStringArgumentFinder<const wxString&> {};

template<>
struct wxFormatStringArgumentFinder<wxScopedCharBuffer>
    : public wxFormatStringArgumentFinder<const wxScopedCharBuffer&> {};

template<>
struct wxFormatStringArgumentFinder<wxScopedWCharBuffer>
    : public wxFormatStringArgumentFinder<const wxScopedWCharBuffer&> {};

template<>
struct wxFormatStringArgumentFinder<wxCharBuffer>
    : public wxFormatStringArgumentFinder<const wxCharBuffer&> {};

template<>
struct wxFormatStringArgumentFinder<wxWCharBuffer>
    : public wxFormatStringArgumentFinder<const wxWCharBuffer&> {};



#if wxDEBUG_LEVEL
                #define wxASSERT_ARG_TYPE(fmt, index, expected_mask)                    \
        wxSTATEMENT_MACRO_BEGIN                                             \
            if ( !fmt )                                                     \
                break;                                                      \
            const int argtype = fmt->GetArgumentType(index);                \
            wxASSERT_MSG( (argtype & (expected_mask)) == argtype,           \
                          "format specifier doesn't match argument type" ); \
        wxSTATEMENT_MACRO_END
#else
            #define wxASSERT_ARG_TYPE(fmt, index, expected_mask)                      \
        wxUnusedVar(fmt);                                                     \
        wxUnusedVar(index)
#endif 

#if defined(HAVE_TYPE_TRAITS) || defined(HAVE_TR1_TYPE_TRAITS)

template<bool IsEnum>
struct wxFormatStringSpecifierNonPodType {};

template<>
struct wxFormatStringSpecifierNonPodType<true>
{
    enum { value = wxFormatString::Arg_Int };
};

template<typename T>
struct wxFormatStringSpecifier
{
#ifdef HAVE_TYPE_TRAITS
    typedef std::is_enum<T> is_enum;
#elif defined HAVE_TR1_TYPE_TRAITS
    typedef std::tr1::is_enum<T> is_enum;
#endif
    enum { value = wxFormatStringSpecifierNonPodType<is_enum::value>::value };
};

#else 
template<typename T>
struct wxFormatStringSpecifier
{
                                #ifdef HAVE_PARTIAL_SPECIALIZATION
    enum { value = wxFormatString::Arg_Int };
#else
    enum { value = wxFormatString::Arg_Int | wxFormatString::Arg_Pointer };
#endif
};

#endif 

#ifdef HAVE_PARTIAL_SPECIALIZATION
template<typename T>
struct wxFormatStringSpecifier<T*>
{
    enum { value = wxFormatString::Arg_Pointer };
};

template<typename T>
struct wxFormatStringSpecifier<const T*>
{
    enum { value = wxFormatString::Arg_Pointer };
};
#endif 

#define wxFORMAT_STRING_SPECIFIER(T, arg)                                   \
    template<> struct wxFormatStringSpecifier<T>                            \
    {                                                                       \
        enum { value = arg };                                               \
    };

wxFORMAT_STRING_SPECIFIER(bool, wxFormatString::Arg_Int)
wxFORMAT_STRING_SPECIFIER(int, wxFormatString::Arg_Int)
wxFORMAT_STRING_SPECIFIER(unsigned int, wxFormatString::Arg_Int)
wxFORMAT_STRING_SPECIFIER(short int, wxFormatString::Arg_Int)
wxFORMAT_STRING_SPECIFIER(short unsigned int, wxFormatString::Arg_Int)
wxFORMAT_STRING_SPECIFIER(long int, wxFormatString::Arg_LongInt)
wxFORMAT_STRING_SPECIFIER(long unsigned int, wxFormatString::Arg_LongInt)
#ifdef wxLongLong_t
wxFORMAT_STRING_SPECIFIER(wxLongLong_t, wxFormatString::Arg_LongLongInt)
wxFORMAT_STRING_SPECIFIER(wxULongLong_t, wxFormatString::Arg_LongLongInt)
#endif
wxFORMAT_STRING_SPECIFIER(float, wxFormatString::Arg_Double)
wxFORMAT_STRING_SPECIFIER(double, wxFormatString::Arg_Double)
wxFORMAT_STRING_SPECIFIER(long double, wxFormatString::Arg_LongDouble)

#if wxWCHAR_T_IS_REAL_TYPE
wxFORMAT_STRING_SPECIFIER(wchar_t, wxFormatString::Arg_Char | wxFormatString::Arg_Int)
#endif

#if !wxUSE_UNICODE
wxFORMAT_STRING_SPECIFIER(char, wxFormatString::Arg_Char | wxFormatString::Arg_Int)
wxFORMAT_STRING_SPECIFIER(signed char, wxFormatString::Arg_Char | wxFormatString::Arg_Int)
wxFORMAT_STRING_SPECIFIER(unsigned char, wxFormatString::Arg_Char | wxFormatString::Arg_Int)
#endif

wxFORMAT_STRING_SPECIFIER(char*, wxFormatString::Arg_String)
wxFORMAT_STRING_SPECIFIER(unsigned char*, wxFormatString::Arg_String)
wxFORMAT_STRING_SPECIFIER(signed char*, wxFormatString::Arg_String)
wxFORMAT_STRING_SPECIFIER(const char*, wxFormatString::Arg_String)
wxFORMAT_STRING_SPECIFIER(const unsigned char*, wxFormatString::Arg_String)
wxFORMAT_STRING_SPECIFIER(const signed char*, wxFormatString::Arg_String)
wxFORMAT_STRING_SPECIFIER(wchar_t*, wxFormatString::Arg_String)
wxFORMAT_STRING_SPECIFIER(const wchar_t*, wxFormatString::Arg_String)

wxFORMAT_STRING_SPECIFIER(int*, wxFormatString::Arg_IntPtr | wxFormatString::Arg_Pointer)
wxFORMAT_STRING_SPECIFIER(short int*, wxFormatString::Arg_ShortIntPtr | wxFormatString::Arg_Pointer)
wxFORMAT_STRING_SPECIFIER(long int*, wxFormatString::Arg_LongIntPtr | wxFormatString::Arg_Pointer)

#undef wxFORMAT_STRING_SPECIFIER


template<typename T>
struct wxArgNormalizer
{
                    wxArgNormalizer(T value,
                    const wxFormatString *fmt, unsigned index)
        : m_value(value)
    {
        wxASSERT_ARG_TYPE( fmt, index, wxFormatStringSpecifier<T>::value );
    }

                T get() const { return m_value; }

    T m_value;
};

#if !wxUSE_UTF8_LOCALE_ONLY
template<typename T>
struct wxArgNormalizerWchar : public wxArgNormalizer<T>
{
    wxArgNormalizerWchar(T value,
                         const wxFormatString *fmt, unsigned index)
        : wxArgNormalizer<T>(value, fmt, index) {}
};
#endif 
#if wxUSE_UNICODE_UTF8
    template<typename T>
    struct wxArgNormalizerUtf8 : public wxArgNormalizer<T>
    {
        wxArgNormalizerUtf8(T value,
                            const wxFormatString *fmt, unsigned index)
            : wxArgNormalizer<T>(value, fmt, index) {}
    };

    #define wxArgNormalizerNative wxArgNormalizerUtf8
#else     #define wxArgNormalizerNative wxArgNormalizerWchar
#endif 




template<typename CharType>
struct wxArgNormalizerWithBuffer
{
    typedef wxScopedCharTypeBuffer<CharType> CharBuffer;

    wxArgNormalizerWithBuffer() {}
    wxArgNormalizerWithBuffer(const CharBuffer& buf,
                              const wxFormatString *fmt,
                              unsigned index)
        : m_value(buf)
    {
        wxASSERT_ARG_TYPE( fmt, index, wxFormatString::Arg_String );
    }

    const CharType *get() const { return m_value; }

    CharBuffer m_value;
};

template<>
struct WXDLLIMPEXP_BASE wxArgNormalizerNative<const wxString&>
{
    wxArgNormalizerNative(const wxString& s,
                          const wxFormatString *fmt,
                          unsigned index)
        : m_value(s)
    {
        wxASSERT_ARG_TYPE( fmt, index, wxFormatString::Arg_String );
    }

    const wxStringCharType *get() const;

    const wxString& m_value;
};

template<>
struct WXDLLIMPEXP_BASE wxArgNormalizerNative<const wxCStrData&>
{
    wxArgNormalizerNative(const wxCStrData& value,
                          const wxFormatString *fmt,
                          unsigned index)
        : m_value(value)
    {
        wxASSERT_ARG_TYPE( fmt, index, wxFormatString::Arg_String );
    }

    const wxStringCharType *get() const;

    const wxCStrData& m_value;
};

#if wxUSE_UNICODE_UTF8 && !wxUSE_UTF8_LOCALE_ONLY
template<>
struct WXDLLIMPEXP_BASE wxArgNormalizerWchar<const wxString&>
    : public wxArgNormalizerWithBuffer<wchar_t>
{
    wxArgNormalizerWchar(const wxString& s,
                         const wxFormatString *fmt, unsigned index);
};

template<>
struct WXDLLIMPEXP_BASE wxArgNormalizerWchar<const wxCStrData&>
    : public wxArgNormalizerWithBuffer<wchar_t>
{
    wxArgNormalizerWchar(const wxCStrData& s,
                         const wxFormatString *fmt, unsigned index);
};
#endif 

#if wxUSE_UNICODE_WCHAR

template<>
struct wxArgNormalizerWchar<const char*>
    : public wxArgNormalizerWithBuffer<wchar_t>
{
    wxArgNormalizerWchar(const char* s,
                         const wxFormatString *fmt, unsigned index)
        : wxArgNormalizerWithBuffer<wchar_t>(wxConvLibc.cMB2WC(s), fmt, index) {}
};

#elif wxUSE_UNICODE_UTF8

template<>
struct wxArgNormalizerUtf8<const wchar_t*>
    : public wxArgNormalizerWithBuffer<char>
{
    wxArgNormalizerUtf8(const wchar_t* s,
                        const wxFormatString *fmt, unsigned index)
        : wxArgNormalizerWithBuffer<char>(wxConvUTF8.cWC2MB(s), fmt, index) {}
};

template<>
struct wxArgNormalizerUtf8<const char*>
    : public wxArgNormalizerWithBuffer<char>
{
    wxArgNormalizerUtf8(const char* s,
                        const wxFormatString *fmt,
                        unsigned index)
    {
        wxASSERT_ARG_TYPE( fmt, index, wxFormatString::Arg_String );

        if ( wxLocaleIsUtf8 )
        {
            m_value = wxScopedCharBuffer::CreateNonOwned(s);
        }
        else
        {
                        wxScopedWCharBuffer buf(wxConvLibc.cMB2WC(s));

                        if ( buf )
                m_value = wxConvUTF8.cWC2MB(buf);
        }
    }
};

#if !wxUSE_UTF8_LOCALE_ONLY
template<>
struct wxArgNormalizerWchar<const char*>
    : public wxArgNormalizerWithBuffer<wchar_t>
{
    wxArgNormalizerWchar(const char* s,
                         const wxFormatString *fmt, unsigned index)
        : wxArgNormalizerWithBuffer<wchar_t>(wxConvLibc.cMB2WC(s), fmt, index) {}
};
#endif 
#else 
template<>
struct wxArgNormalizerWchar<const wchar_t*>
    : public wxArgNormalizerWithBuffer<char>
{
    wxArgNormalizerWchar(const wchar_t* s,
                         const wxFormatString *fmt, unsigned index)
        : wxArgNormalizerWithBuffer<char>(wxConvLibc.cWC2MB(s), fmt, index) {}
};

#endif 

#if wxUSE_UNICODE_UTF8
    #if wxUSE_UTF8_LOCALE_ONLY
        #define WX_ARG_NORMALIZER_FORWARD(T, BaseT)                         \
          _WX_ARG_NORMALIZER_FORWARD_IMPL(wxArgNormalizerUtf8, T, BaseT)
    #else         #define WX_ARG_NORMALIZER_FORWARD(T, BaseT)                         \
          _WX_ARG_NORMALIZER_FORWARD_IMPL(wxArgNormalizerWchar, T, BaseT);  \
          _WX_ARG_NORMALIZER_FORWARD_IMPL(wxArgNormalizerUtf8, T, BaseT)
    #endif
#else     #define WX_ARG_NORMALIZER_FORWARD(T, BaseT)                             \
        _WX_ARG_NORMALIZER_FORWARD_IMPL(wxArgNormalizerWchar, T, BaseT)
#endif 
#define _WX_ARG_NORMALIZER_FORWARD_IMPL(Normalizer, T, BaseT)               \
    template<>                                                              \
    struct Normalizer<T> : public Normalizer<BaseT>                         \
    {                                                                       \
        Normalizer(BaseT value,                                             \
                   const wxFormatString *fmt, unsigned index)               \
            : Normalizer<BaseT>(value, fmt, index) {}                       \
    }

WX_ARG_NORMALIZER_FORWARD(wxString, const wxString&);
WX_ARG_NORMALIZER_FORWARD(wxCStrData, const wxCStrData&);

WX_ARG_NORMALIZER_FORWARD(char*, const char*);
WX_ARG_NORMALIZER_FORWARD(wchar_t*, const wchar_t*);

WX_ARG_NORMALIZER_FORWARD(wxScopedCharBuffer, const char*);
WX_ARG_NORMALIZER_FORWARD(const wxScopedCharBuffer&, const char*);
WX_ARG_NORMALIZER_FORWARD(wxScopedWCharBuffer, const wchar_t*);
WX_ARG_NORMALIZER_FORWARD(const wxScopedWCharBuffer&, const wchar_t*);
WX_ARG_NORMALIZER_FORWARD(wxCharBuffer, const char*);
WX_ARG_NORMALIZER_FORWARD(const wxCharBuffer&, const char*);
WX_ARG_NORMALIZER_FORWARD(wxWCharBuffer, const wchar_t*);
WX_ARG_NORMALIZER_FORWARD(const wxWCharBuffer&, const wchar_t*);

#if wxUSE_STD_STRING

#include "wx/stringimpl.h"

#if !wxUSE_UTF8_LOCALE_ONLY
template<>
struct wxArgNormalizerWchar<const std::string&>
    : public wxArgNormalizerWchar<const char*>
{
    wxArgNormalizerWchar(const std::string& s,
                         const wxFormatString *fmt, unsigned index)
        : wxArgNormalizerWchar<const char*>(s.c_str(), fmt, index) {}
};

template<>
struct wxArgNormalizerWchar<const wxStdWideString&>
    : public wxArgNormalizerWchar<const wchar_t*>
{
    wxArgNormalizerWchar(const wxStdWideString& s,
                         const wxFormatString *fmt, unsigned index)
        : wxArgNormalizerWchar<const wchar_t*>(s.c_str(), fmt, index) {}
};
#endif 
#if wxUSE_UNICODE_UTF8
template<>
struct wxArgNormalizerUtf8<const std::string&>
    : public wxArgNormalizerUtf8<const char*>
{
    wxArgNormalizerUtf8(const std::string& s,
                        const wxFormatString *fmt, unsigned index)
        : wxArgNormalizerUtf8<const char*>(s.c_str(), fmt, index) {}
};

template<>
struct wxArgNormalizerUtf8<const wxStdWideString&>
    : public wxArgNormalizerUtf8<const wchar_t*>
{
    wxArgNormalizerUtf8(const wxStdWideString& s,
                        const wxFormatString *fmt, unsigned index)
        : wxArgNormalizerUtf8<const wchar_t*>(s.c_str(), fmt, index) {}
};
#endif 
WX_ARG_NORMALIZER_FORWARD(std::string, const std::string&);
WX_ARG_NORMALIZER_FORWARD(wxStdWideString, const wxStdWideString&);

#endif 

template<>
struct wxArgNormalizer<const wxUniChar&> : public wxArgNormalizer<wchar_t>
{
    wxArgNormalizer(const wxUniChar& s,
                    const wxFormatString *fmt, unsigned index)
        : wxArgNormalizer<wchar_t>(wx_truncate_cast(wchar_t, s.GetValue()), fmt, index) {}
};


#if wxUSE_UNICODE
template<typename T>
struct wxArgNormalizerNarrowChar
{
    wxArgNormalizerNarrowChar(T value,
                              const wxFormatString *fmt, unsigned index)
    {
        wxASSERT_ARG_TYPE( fmt, index,
                           wxFormatString::Arg_Char | wxFormatString::Arg_Int );

                        if ( !fmt || fmt->GetArgumentType(index) == wxFormatString::Arg_Char )
            m_value = wx_truncate_cast(T, wxUniChar(value).GetValue());
        else
            m_value = value;
    }

    int get() const { return m_value; }

    T m_value;
};

template<>
struct wxArgNormalizer<char> : public wxArgNormalizerNarrowChar<char>
{
    wxArgNormalizer(char value,
                    const wxFormatString *fmt, unsigned index)
        : wxArgNormalizerNarrowChar<char>(value, fmt, index) {}
};

template<>
struct wxArgNormalizer<unsigned char>
    : public wxArgNormalizerNarrowChar<unsigned char>
{
    wxArgNormalizer(unsigned char value,
                    const wxFormatString *fmt, unsigned index)
        : wxArgNormalizerNarrowChar<unsigned char>(value, fmt, index) {}
};

template<>
struct wxArgNormalizer<signed char>
    : public wxArgNormalizerNarrowChar<signed char>
{
    wxArgNormalizer(signed char value,
                    const wxFormatString *fmt, unsigned index)
        : wxArgNormalizerNarrowChar<signed char>(value, fmt, index) {}
};

#endif 
WX_ARG_NORMALIZER_FORWARD(wxUniChar, const wxUniChar&);
WX_ARG_NORMALIZER_FORWARD(const wxUniCharRef&, const wxUniChar&);
WX_ARG_NORMALIZER_FORWARD(wxUniCharRef, const wxUniChar&);
WX_ARG_NORMALIZER_FORWARD(const wchar_t&, wchar_t);

WX_ARG_NORMALIZER_FORWARD(const char&, char);
WX_ARG_NORMALIZER_FORWARD(const unsigned char&, unsigned char);
WX_ARG_NORMALIZER_FORWARD(const signed char&, signed char);


#undef WX_ARG_NORMALIZER_FORWARD
#undef _WX_ARG_NORMALIZER_FORWARD_IMPL




struct WXDLLIMPEXP_BASE wxArgNormalizedString
{
    wxArgNormalizedString(const void* ptr) : m_ptr(ptr) {}

        bool IsValid() const { return m_ptr != NULL; }
    operator bool() const { return IsValid(); }

        wxString GetString() const;
    operator wxString() const;

private:
    const void *m_ptr;
};

#define WX_VA_ARG_STRING(ap) wxArgNormalizedString(va_arg(ap, const void*))


#define _WX_VARARG_MAX_ARGS        30
#define _WX_VARARG_MAX_FIXED_ARGS   4

#define _WX_VARARG_JOIN_1(m)                                 m(1)
#define _WX_VARARG_JOIN_2(m)       _WX_VARARG_JOIN_1(m),     m(2)
#define _WX_VARARG_JOIN_3(m)       _WX_VARARG_JOIN_2(m),     m(3)
#define _WX_VARARG_JOIN_4(m)       _WX_VARARG_JOIN_3(m),     m(4)
#define _WX_VARARG_JOIN_5(m)       _WX_VARARG_JOIN_4(m),     m(5)
#define _WX_VARARG_JOIN_6(m)       _WX_VARARG_JOIN_5(m),     m(6)
#define _WX_VARARG_JOIN_7(m)       _WX_VARARG_JOIN_6(m),     m(7)
#define _WX_VARARG_JOIN_8(m)       _WX_VARARG_JOIN_7(m),     m(8)
#define _WX_VARARG_JOIN_9(m)       _WX_VARARG_JOIN_8(m),     m(9)
#define _WX_VARARG_JOIN_10(m)      _WX_VARARG_JOIN_9(m),     m(10)
#define _WX_VARARG_JOIN_11(m)      _WX_VARARG_JOIN_10(m),    m(11)
#define _WX_VARARG_JOIN_12(m)      _WX_VARARG_JOIN_11(m),    m(12)
#define _WX_VARARG_JOIN_13(m)      _WX_VARARG_JOIN_12(m),    m(13)
#define _WX_VARARG_JOIN_14(m)      _WX_VARARG_JOIN_13(m),    m(14)
#define _WX_VARARG_JOIN_15(m)      _WX_VARARG_JOIN_14(m),    m(15)
#define _WX_VARARG_JOIN_16(m)      _WX_VARARG_JOIN_15(m),    m(16)
#define _WX_VARARG_JOIN_17(m)      _WX_VARARG_JOIN_16(m),    m(17)
#define _WX_VARARG_JOIN_18(m)      _WX_VARARG_JOIN_17(m),    m(18)
#define _WX_VARARG_JOIN_19(m)      _WX_VARARG_JOIN_18(m),    m(19)
#define _WX_VARARG_JOIN_20(m)      _WX_VARARG_JOIN_19(m),    m(20)
#define _WX_VARARG_JOIN_21(m)      _WX_VARARG_JOIN_20(m),    m(21)
#define _WX_VARARG_JOIN_22(m)      _WX_VARARG_JOIN_21(m),    m(22)
#define _WX_VARARG_JOIN_23(m)      _WX_VARARG_JOIN_22(m),    m(23)
#define _WX_VARARG_JOIN_24(m)      _WX_VARARG_JOIN_23(m),    m(24)
#define _WX_VARARG_JOIN_25(m)      _WX_VARARG_JOIN_24(m),    m(25)
#define _WX_VARARG_JOIN_26(m)      _WX_VARARG_JOIN_25(m),    m(26)
#define _WX_VARARG_JOIN_27(m)      _WX_VARARG_JOIN_26(m),    m(27)
#define _WX_VARARG_JOIN_28(m)      _WX_VARARG_JOIN_27(m),    m(28)
#define _WX_VARARG_JOIN_29(m)      _WX_VARARG_JOIN_28(m),    m(29)
#define _WX_VARARG_JOIN_30(m)      _WX_VARARG_JOIN_29(m),    m(30)

#define _WX_VARARG_ITER_1(m,a,b,c,d,e,f)                                    m(1,a,b,c,d,e,f)
#define _WX_VARARG_ITER_2(m,a,b,c,d,e,f)  _WX_VARARG_ITER_1(m,a,b,c,d,e,f)  m(2,a,b,c,d,e,f)
#define _WX_VARARG_ITER_3(m,a,b,c,d,e,f)  _WX_VARARG_ITER_2(m,a,b,c,d,e,f)  m(3,a,b,c,d,e,f)
#define _WX_VARARG_ITER_4(m,a,b,c,d,e,f)  _WX_VARARG_ITER_3(m,a,b,c,d,e,f)  m(4,a,b,c,d,e,f)
#define _WX_VARARG_ITER_5(m,a,b,c,d,e,f)  _WX_VARARG_ITER_4(m,a,b,c,d,e,f)  m(5,a,b,c,d,e,f)
#define _WX_VARARG_ITER_6(m,a,b,c,d,e,f)  _WX_VARARG_ITER_5(m,a,b,c,d,e,f)  m(6,a,b,c,d,e,f)
#define _WX_VARARG_ITER_7(m,a,b,c,d,e,f)  _WX_VARARG_ITER_6(m,a,b,c,d,e,f)  m(7,a,b,c,d,e,f)
#define _WX_VARARG_ITER_8(m,a,b,c,d,e,f)  _WX_VARARG_ITER_7(m,a,b,c,d,e,f)  m(8,a,b,c,d,e,f)
#define _WX_VARARG_ITER_9(m,a,b,c,d,e,f)  _WX_VARARG_ITER_8(m,a,b,c,d,e,f)  m(9,a,b,c,d,e,f)
#define _WX_VARARG_ITER_10(m,a,b,c,d,e,f) _WX_VARARG_ITER_9(m,a,b,c,d,e,f)  m(10,a,b,c,d,e,f)
#define _WX_VARARG_ITER_11(m,a,b,c,d,e,f) _WX_VARARG_ITER_10(m,a,b,c,d,e,f) m(11,a,b,c,d,e,f)
#define _WX_VARARG_ITER_12(m,a,b,c,d,e,f) _WX_VARARG_ITER_11(m,a,b,c,d,e,f) m(12,a,b,c,d,e,f)
#define _WX_VARARG_ITER_13(m,a,b,c,d,e,f) _WX_VARARG_ITER_12(m,a,b,c,d,e,f) m(13,a,b,c,d,e,f)
#define _WX_VARARG_ITER_14(m,a,b,c,d,e,f) _WX_VARARG_ITER_13(m,a,b,c,d,e,f) m(14,a,b,c,d,e,f)
#define _WX_VARARG_ITER_15(m,a,b,c,d,e,f) _WX_VARARG_ITER_14(m,a,b,c,d,e,f) m(15,a,b,c,d,e,f)
#define _WX_VARARG_ITER_16(m,a,b,c,d,e,f) _WX_VARARG_ITER_15(m,a,b,c,d,e,f) m(16,a,b,c,d,e,f)
#define _WX_VARARG_ITER_17(m,a,b,c,d,e,f) _WX_VARARG_ITER_16(m,a,b,c,d,e,f) m(17,a,b,c,d,e,f)
#define _WX_VARARG_ITER_18(m,a,b,c,d,e,f) _WX_VARARG_ITER_17(m,a,b,c,d,e,f) m(18,a,b,c,d,e,f)
#define _WX_VARARG_ITER_19(m,a,b,c,d,e,f) _WX_VARARG_ITER_18(m,a,b,c,d,e,f) m(19,a,b,c,d,e,f)
#define _WX_VARARG_ITER_20(m,a,b,c,d,e,f) _WX_VARARG_ITER_19(m,a,b,c,d,e,f) m(20,a,b,c,d,e,f)
#define _WX_VARARG_ITER_21(m,a,b,c,d,e,f) _WX_VARARG_ITER_20(m,a,b,c,d,e,f) m(21,a,b,c,d,e,f)
#define _WX_VARARG_ITER_22(m,a,b,c,d,e,f) _WX_VARARG_ITER_21(m,a,b,c,d,e,f) m(22,a,b,c,d,e,f)
#define _WX_VARARG_ITER_23(m,a,b,c,d,e,f) _WX_VARARG_ITER_22(m,a,b,c,d,e,f) m(23,a,b,c,d,e,f)
#define _WX_VARARG_ITER_24(m,a,b,c,d,e,f) _WX_VARARG_ITER_23(m,a,b,c,d,e,f) m(24,a,b,c,d,e,f)
#define _WX_VARARG_ITER_25(m,a,b,c,d,e,f) _WX_VARARG_ITER_24(m,a,b,c,d,e,f) m(25,a,b,c,d,e,f)
#define _WX_VARARG_ITER_26(m,a,b,c,d,e,f) _WX_VARARG_ITER_25(m,a,b,c,d,e,f) m(26,a,b,c,d,e,f)
#define _WX_VARARG_ITER_27(m,a,b,c,d,e,f) _WX_VARARG_ITER_26(m,a,b,c,d,e,f) m(27,a,b,c,d,e,f)
#define _WX_VARARG_ITER_28(m,a,b,c,d,e,f) _WX_VARARG_ITER_27(m,a,b,c,d,e,f) m(28,a,b,c,d,e,f)
#define _WX_VARARG_ITER_29(m,a,b,c,d,e,f) _WX_VARARG_ITER_28(m,a,b,c,d,e,f) m(29,a,b,c,d,e,f)
#define _WX_VARARG_ITER_30(m,a,b,c,d,e,f) _WX_VARARG_ITER_29(m,a,b,c,d,e,f) m(30,a,b,c,d,e,f)


#define _WX_VARARG_FIXED_EXPAND_1(t1) \
         t1 f1
#define _WX_VARARG_FIXED_EXPAND_2(t1,t2) \
         t1 f1, t2 f2
#define _WX_VARARG_FIXED_EXPAND_3(t1,t2,t3) \
         t1 f1, t2 f2, t3 f3
#define _WX_VARARG_FIXED_EXPAND_4(t1,t2,t3,t4) \
         t1 f1, t2 f2, t3 f3, t4 f4

#define _WX_VARARG_FIXED_UNUSED_EXPAND_1(t1) \
         t1 WXUNUSED(f1)
#define _WX_VARARG_FIXED_UNUSED_EXPAND_2(t1,t2) \
         t1 WXUNUSED(f1), t2 WXUNUSED(f2)
#define _WX_VARARG_FIXED_UNUSED_EXPAND_3(t1,t2,t3) \
         t1 WXUNUSED(f1), t2 WXUNUSED(f2), t3 WXUNUSED(f3)
#define _WX_VARARG_FIXED_UNUSED_EXPAND_4(t1,t2,t3,t4) \
         t1 WXUNUSED(f1), t2 WXUNUSED(f2), t3 WXUNUSED(f3), t4 WXUNUSED(f4)

#define _WX_VARARG_FIXED_TYPEDEFS_1(t1) \
             typedef t1 TF1
#define _WX_VARARG_FIXED_TYPEDEFS_2(t1,t2) \
             _WX_VARARG_FIXED_TYPEDEFS_1(t1); typedef t2 TF2
#define _WX_VARARG_FIXED_TYPEDEFS_3(t1,t2,t3) \
             _WX_VARARG_FIXED_TYPEDEFS_2(t1,t2); typedef t3 TF3
#define _WX_VARARG_FIXED_TYPEDEFS_4(t1,t2,t3,t4) \
             _WX_VARARG_FIXED_TYPEDEFS_3(t1,t2,t3); typedef t4 TF4

#define _WX_VARARG_FIXED_EXPAND(N, args) \
                _WX_VARARG_FIXED_EXPAND_IMPL(N, args)
#define _WX_VARARG_FIXED_EXPAND_IMPL(N, args) \
                _WX_VARARG_FIXED_EXPAND_##N args

#define _WX_VARARG_FIXED_UNUSED_EXPAND(N, args) \
                _WX_VARARG_FIXED_UNUSED_EXPAND_IMPL(N, args)
#define _WX_VARARG_FIXED_UNUSED_EXPAND_IMPL(N, args) \
                _WX_VARARG_FIXED_UNUSED_EXPAND_##N args

#define _WX_VARARG_FIXED_TYPEDEFS(N, args) \
                _WX_VARARG_FIXED_TYPEDEFS_IMPL(N, args)
#define _WX_VARARG_FIXED_TYPEDEFS_IMPL(N, args) \
                _WX_VARARG_FIXED_TYPEDEFS_##N args


#define _WX_VARARG_JOIN(N, m)             _WX_VARARG_JOIN_IMPL(N, m)
#define _WX_VARARG_JOIN_IMPL(N, m)        _WX_VARARG_JOIN_##N(m)


#define _WX_VARARG_ITER(N,m,a,b,c,d,e,f) \
        _WX_VARARG_ITER_IMPL(N,m,a,b,c,d,e,f)
#define _WX_VARARG_ITER_IMPL(N,m,a,b,c,d,e,f) \
        _WX_VARARG_ITER_##N(m,a,b,c,d,e,f)

#define _WX_VARARG_ARG(i)               T##i a##i

#define _WX_VARARG_ARG_UNUSED(i)        T##i WXUNUSED(a##i)

#define _WX_VARARG_TEMPL(i)             typename T##i

#define _WX_VARARG_PASS_WCHAR(i) \
    wxArgNormalizerWchar<T##i>(a##i, fmt, i).get()
#define _WX_VARARG_PASS_UTF8(i) \
    wxArgNormalizerUtf8<T##i>(a##i, fmt, i).get()


#define _WX_VARARG_PASS_FIXED(i)        f##i

#define _WX_VARARG_FIND_FMT(i) \
            (wxFormatStringArgumentFinder<TF##i>::find(f##i))

#define _WX_VARARG_FORMAT_STRING(numfixed, fixed)                             \
    _WX_VARARG_FIXED_TYPEDEFS(numfixed, fixed);                               \
    const wxFormatString *fmt =                                               \
            (_WX_VARARG_JOIN(numfixed, _WX_VARARG_FIND_FMT))

#if wxUSE_UNICODE_UTF8
    #define _WX_VARARG_DO_CALL_UTF8(return_kw, impl, implUtf8, N, numfixed)   \
        return_kw implUtf8(_WX_VARARG_JOIN(numfixed, _WX_VARARG_PASS_FIXED),  \
                        _WX_VARARG_JOIN(N, _WX_VARARG_PASS_UTF8))
    #define _WX_VARARG_DO_CALL0_UTF8(return_kw, impl, implUtf8, numfixed)     \
        return_kw implUtf8(_WX_VARARG_JOIN(numfixed, _WX_VARARG_PASS_FIXED))
#endif 
#define _WX_VARARG_DO_CALL_WCHAR(return_kw, impl, implUtf8, N, numfixed)      \
    return_kw impl(_WX_VARARG_JOIN(numfixed, _WX_VARARG_PASS_FIXED),          \
                    _WX_VARARG_JOIN(N, _WX_VARARG_PASS_WCHAR))
#define _WX_VARARG_DO_CALL0_WCHAR(return_kw, impl, implUtf8, numfixed)        \
    return_kw impl(_WX_VARARG_JOIN(numfixed, _WX_VARARG_PASS_FIXED))

#if wxUSE_UNICODE_UTF8
    #if wxUSE_UTF8_LOCALE_ONLY
        #define _WX_VARARG_DO_CALL _WX_VARARG_DO_CALL_UTF8
        #define _WX_VARARG_DO_CALL0 _WX_VARARG_DO_CALL0_UTF8
    #else         #define _WX_VARARG_DO_CALL(return_kw, impl, implUtf8, N, numfixed)    \
            if ( wxLocaleIsUtf8 )                                             \
              _WX_VARARG_DO_CALL_UTF8(return_kw, impl, implUtf8, N, numfixed);\
            else                                                              \
              _WX_VARARG_DO_CALL_WCHAR(return_kw, impl, implUtf8, N, numfixed)

        #define _WX_VARARG_DO_CALL0(return_kw, impl, implUtf8, numfixed)      \
            if ( wxLocaleIsUtf8 )                                             \
              _WX_VARARG_DO_CALL0_UTF8(return_kw, impl, implUtf8, numfixed);  \
            else                                                              \
              _WX_VARARG_DO_CALL0_WCHAR(return_kw, impl, implUtf8, numfixed)
    #endif #else     #define _WX_VARARG_DO_CALL _WX_VARARG_DO_CALL_WCHAR
    #define _WX_VARARG_DO_CALL0 _WX_VARARG_DO_CALL0_WCHAR
#endif 

#define _WX_VARARG_DEFINE_FUNC(N, rettype, name,                              \
                               impl, implUtf8, numfixed, fixed)               \
    template<_WX_VARARG_JOIN(N, _WX_VARARG_TEMPL)>                            \
    rettype name(_WX_VARARG_FIXED_EXPAND(numfixed, fixed),                    \
                 _WX_VARARG_JOIN(N, _WX_VARARG_ARG))                          \
    {                                                                         \
        _WX_VARARG_FORMAT_STRING(numfixed, fixed);                            \
        _WX_VARARG_DO_CALL(return, impl, implUtf8, N, numfixed);              \
    }

#define _WX_VARARG_DEFINE_FUNC_N0(rettype, name,                              \
                                  impl, implUtf8, numfixed, fixed)            \
    inline rettype name(_WX_VARARG_FIXED_EXPAND(numfixed, fixed))             \
    {                                                                         \
        _WX_VARARG_DO_CALL0(return, impl, implUtf8, numfixed);                \
    }

#define _WX_VARARG_DEFINE_FUNC_VOID(N, rettype, name,                         \
                                    impl, implUtf8, numfixed, fixed)          \
    template<_WX_VARARG_JOIN(N, _WX_VARARG_TEMPL)>                            \
    void name(_WX_VARARG_FIXED_EXPAND(numfixed, fixed),                       \
                 _WX_VARARG_JOIN(N, _WX_VARARG_ARG))                          \
    {                                                                         \
        _WX_VARARG_FORMAT_STRING(numfixed, fixed);                            \
        _WX_VARARG_DO_CALL(wxEMPTY_PARAMETER_VALUE,                           \
                           impl, implUtf8, N, numfixed);                      \
    }

#define _WX_VARARG_DEFINE_FUNC_VOID_N0(name, impl, implUtf8, numfixed, fixed) \
    inline void name(_WX_VARARG_FIXED_EXPAND(numfixed, fixed))                \
    {                                                                         \
        _WX_VARARG_DO_CALL0(wxEMPTY_PARAMETER_VALUE,                          \
                            impl, implUtf8, numfixed);                        \
    }

#define _WX_VARARG_DEFINE_FUNC_CTOR(N, rettype, name,                         \
                                    impl, implUtf8, numfixed, fixed)          \
    template<_WX_VARARG_JOIN(N, _WX_VARARG_TEMPL)>                            \
    name(_WX_VARARG_FIXED_EXPAND(numfixed, fixed),                            \
                _WX_VARARG_JOIN(N, _WX_VARARG_ARG))                           \
    {                                                                         \
        _WX_VARARG_FORMAT_STRING(numfixed, fixed);                            \
        _WX_VARARG_DO_CALL(wxEMPTY_PARAMETER_VALUE,                           \
                           impl, implUtf8, N, numfixed);                      \
    }

#define _WX_VARARG_DEFINE_FUNC_CTOR_N0(name, impl, implUtf8, numfixed, fixed) \
    inline name(_WX_VARARG_FIXED_EXPAND(numfixed, fixed))                     \
    {                                                                         \
        _WX_VARARG_DO_CALL0(wxEMPTY_PARAMETER_VALUE,                          \
                            impl, implUtf8, numfixed);                        \
    }

#define _WX_VARARG_DEFINE_FUNC_NOP(N, rettype, name,                          \
                                   impl, implUtf8, numfixed, fixed)           \
    template<_WX_VARARG_JOIN(N, _WX_VARARG_TEMPL)>                            \
    void name(_WX_VARARG_FIXED_UNUSED_EXPAND(numfixed, fixed),                \
                 _WX_VARARG_JOIN(N, _WX_VARARG_ARG_UNUSED))                   \
    {}

#define _WX_VARARG_DEFINE_FUNC_NOP_N0(name, numfixed, fixed)                  \
    inline void name(_WX_VARARG_FIXED_UNUSED_EXPAND(numfixed, fixed))         \
    {}

#endif 