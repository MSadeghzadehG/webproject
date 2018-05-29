


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/strvararg.h"
#include "wx/string.h"
#include "wx/crt.h"
#include "wx/private/wxprintf.h"



const wxStringCharType *wxArgNormalizerNative<const wxString&>::get() const
{
    return m_value.wx_str();
}

const wxStringCharType *wxArgNormalizerNative<const wxCStrData&>::get() const
{
    return m_value.AsInternal();
}

#if wxUSE_UNICODE_UTF8 && !wxUSE_UTF8_LOCALE_ONLY
wxArgNormalizerWchar<const wxString&>::wxArgNormalizerWchar(
                            const wxString& s,
                            const wxFormatString *fmt, unsigned index)
    : wxArgNormalizerWithBuffer<wchar_t>(s.wc_str(), fmt, index)
{
}

wxArgNormalizerWchar<const wxCStrData&>::wxArgNormalizerWchar(
                            const wxCStrData& s,
                            const wxFormatString *fmt, unsigned index)
    : wxArgNormalizerWithBuffer<wchar_t>(s.AsWCharBuf(), fmt, index)
{
}
#endif 

wxString wxArgNormalizedString::GetString() const
{
    if ( !IsValid() )
        return wxEmptyString;

#if wxUSE_UTF8_LOCALE_ONLY
    return wxString(reinterpret_cast<const char*>(m_ptr));
#else
    #if wxUSE_UNICODE_UTF8
        if ( wxLocaleIsUtf8 )
            return wxString(reinterpret_cast<const char*>(m_ptr));
        else
    #endif
        return wxString(reinterpret_cast<const wxChar*>(m_ptr));
#endif }

wxArgNormalizedString::operator wxString() const
{
    return GetString();
}




template<typename T>
class wxFormatConverterBase
{
public:
    typedef T CharType;

    wxFormatConverterBase()
    {
        m_fmtOrig = NULL;
        m_fmtLast = NULL;
        m_nCopied = 0;
    }

    wxScopedCharTypeBuffer<CharType> Convert(const CharType *format)
    {
                m_fmtOrig = format;

        while ( *format )
        {
            if ( CopyFmtChar(*format++) == wxT('%') )
            {
#if wxUSE_PRINTF_POS_PARAMS
                if ( *format >= '0' && *format <= '9' )
                {
                    SkipDigits(&format);
                    if ( *format == '$' )
                    {
                                                CopyFmtChar(*format++);
                    }
                                    }
#endif 
                                while ( IsFlagChar(*format) )
                    CopyFmtChar(*format++);

                                if ( *format == wxT('*') )
                    CopyFmtChar(*format++);
                else
                    SkipDigits(&format);

                                if ( *format == wxT('.') )
                {
                    CopyFmtChar(*format++);
                    if ( *format == wxT('*') )
                        CopyFmtChar(*format++);
                    else
                        SkipDigits(&format);
                }

                                SizeModifier size;

                switch ( *format )
                {
#ifdef __VISUALC__
                    case 'z':
                                                                                                ChangeFmtChar('I');
                        format++;
                        size = Size_Default;
                        break;
#endif 
                    case 'h':
                        size = Size_Short;
                        format++;
                        break;

                    case 'l':
                                                if ( format[1] != 'l' )
                        {
                            size = Size_Long;
                            format++;
                            break;
                        }
                        wxFALLTHROUGH;

                    default:
                        size = Size_Default;
                }

                CharType outConv = *format;
                SizeModifier outSize = size;

                                switch ( *format )
                {
                    case wxT('S'):
                    case wxT('s'):
                                                                                                HandleString(*format, size, outConv, outSize);
                        break;

                    case wxT('C'):
                    case wxT('c'):
                        HandleChar(*format, size, outConv, outSize);
                        break;

                    default:
                                                break;
                }

                if ( outConv == *format && outSize == size )                 {
                    if ( size != Size_Default )
                        CopyFmtChar(*(format - 1));
                    CopyFmtChar(*format);
                }
                else                 {
                    switch ( outSize )
                    {
                        case Size_Long:
                            InsertFmtChar(wxT('l'));
                            break;

                        case Size_Short:
                            InsertFmtChar(wxT('h'));
                            break;

                        case Size_Default:
                                                        break;
                    }
                    InsertFmtChar(outConv);
                }

                format++;
            }
        }

                                if ( m_fmtOrig )
        {
            return wxScopedCharTypeBuffer<CharType>::CreateNonOwned(m_fmtOrig);
        }
        else
        {
                                                m_fmt.shrink(m_fmtLast - m_fmt.data());
            return m_fmt;
        }
    }

    virtual ~wxFormatConverterBase() {}

protected:
    enum SizeModifier
    {
        Size_Default,
        Size_Short,
        Size_Long
    };

                virtual void HandleString(CharType conv, SizeModifier size,
                              CharType& outConv, SizeModifier& outSize) = 0;

        virtual void HandleChar(CharType conv, SizeModifier size,
                            CharType& outConv, SizeModifier& outSize) = 0;

private:
                    CharType CopyFmtChar(CharType ch)
    {
        if ( !m_fmtOrig )
        {
                        *(m_fmtLast++) = ch;
        }
        else
        {
                                    m_nCopied++;
        }

        return ch;
    }

        void InsertFmtChar(CharType ch)
    {
        if ( m_fmtOrig )
        {
                        CopyAllBefore();
        }

        *(m_fmtLast++) = ch;
    }

        void ChangeFmtChar(CharType ch)
    {
        if ( m_fmtOrig )
        {
                        CopyAllBefore();
        }

        *m_fmtLast = ch;
    }

    void CopyAllBefore()
    {
        wxASSERT_MSG( m_fmtOrig && m_fmt.data() == NULL, "logic error" );

                                                size_t fmtLen = wxStrlen(m_fmtOrig);
                m_fmt.extend(fmtLen * 3 / 2);

        if ( m_nCopied > 0 )
            wxStrncpy(m_fmt.data(), m_fmtOrig, m_nCopied);
        m_fmtLast = m_fmt.data() + m_nCopied;

                        m_fmtOrig = NULL;
    }

    static bool IsFlagChar(CharType ch)
    {
        return ch == wxT('-') || ch == wxT('+') ||
               ch == wxT('0') || ch == wxT(' ') || ch == wxT('#');
    }

    void SkipDigits(const CharType **ptpc)
    {
        while ( **ptpc >= wxT('0') && **ptpc <= wxT('9') )
            CopyFmtChar(*(*ptpc)++);
    }

        wxCharTypeBuffer<CharType> m_fmt;
    CharType *m_fmtLast;

        const CharType *m_fmtOrig;

            size_t m_nCopied;
};

#if defined(__WINDOWS__) && \
    !defined(__CYGWIN__) && \
    (!defined(__MINGW32__) || (__USE_MINGW_ANSI_STDIO +0 == 0))

class wxPrintfFormatConverterWchar : public wxFormatConverterBase<wchar_t>
{
    virtual void HandleString(CharType WXUNUSED(conv),
                              SizeModifier WXUNUSED(size),
                              CharType& outConv, SizeModifier& outSize)
    {
        outConv = 's';
        outSize = Size_Default;
    }

    virtual void HandleChar(CharType WXUNUSED(conv),
                            SizeModifier WXUNUSED(size),
                            CharType& outConv, SizeModifier& outSize)
    {
        outConv = 'c';
        outSize = Size_Default;
    }
};

#else 

#if !wxUSE_UTF8_LOCALE_ONLY
class wxPrintfFormatConverterWchar : public wxFormatConverterBase<wchar_t>
{
    virtual void HandleString(CharType WXUNUSED(conv),
                              SizeModifier WXUNUSED(size),
                              CharType& outConv, SizeModifier& outSize) wxOVERRIDE
    {
        outConv = 's';
        outSize = Size_Long;
    }

    virtual void HandleChar(CharType WXUNUSED(conv),
                            SizeModifier WXUNUSED(size),
                            CharType& outConv, SizeModifier& outSize) wxOVERRIDE
    {
        outConv = 'c';
        outSize = Size_Long;
    }
};
#endif 
#endif 
#if wxUSE_UNICODE_UTF8
class wxPrintfFormatConverterUtf8 : public wxFormatConverterBase<char>
{
    virtual void HandleString(CharType WXUNUSED(conv),
                              SizeModifier WXUNUSED(size),
                              CharType& outConv, SizeModifier& outSize) wxOVERRIDE
    {
        outConv = 's';
        outSize = Size_Default;
    }

    virtual void HandleChar(CharType WXUNUSED(conv),
                            SizeModifier WXUNUSED(size),
                            CharType& outConv, SizeModifier& outSize) wxOVERRIDE
    {
                        outConv = 'c';
        outSize = Size_Long;
    }
};
#endif 
#if !wxUSE_UNICODE class wxPrintfFormatConverterANSI : public wxFormatConverterBase<char>
{
    virtual void HandleString(CharType WXUNUSED(conv),
                              SizeModifier WXUNUSED(size),
                              CharType& outConv, SizeModifier& outSize)
    {
        outConv = 's';
        outSize = Size_Default;
    }

    virtual void HandleChar(CharType WXUNUSED(conv),
                            SizeModifier WXUNUSED(size),
                            CharType& outConv, SizeModifier& outSize)
    {
        outConv = 'c';
        outSize = Size_Default;
    }
};
#endif 
#ifndef __WINDOWS__

class wxScanfFormatConverterWchar : public wxFormatConverterBase<wchar_t>
{
    virtual void HandleString(CharType conv, SizeModifier size,
                              CharType& outConv, SizeModifier& outSize) wxOVERRIDE
    {
        outConv = 's';
        outSize = GetOutSize(conv == 'S', size);
    }

    virtual void HandleChar(CharType conv, SizeModifier size,
                            CharType& outConv, SizeModifier& outSize) wxOVERRIDE
    {
        outConv = 'c';
        outSize = GetOutSize(conv == 'C', size);
    }

    SizeModifier GetOutSize(bool convIsUpper, SizeModifier size)
    {
                if ( convIsUpper )
        {
            if ( size == Size_Long )
                return Size_Long;
            else
                return Size_Default;
        }
        else         {
            if ( size == Size_Default )
                return Size_Long;
            else
                return size;
        }
    }
};

const wxScopedWCharBuffer wxScanfConvertFormatW(const wchar_t *format)
{
    return wxScanfFormatConverterWchar().Convert(format);
}
#endif 


#if !wxUSE_UNICODE_WCHAR
const char* wxFormatString::InputAsChar()
{
    if ( m_char )
        return m_char.data();

                if ( m_str )
        return m_str->wx_str();

        if ( m_cstr )
        return m_cstr->AsInternal();

            wxASSERT( m_wchar );

    m_char = wxConvLibc.cWC2MB(m_wchar.data());

    return m_char.data();
}

const char* wxFormatString::AsChar()
{
    if ( !m_convertedChar )
#if !wxUSE_UNICODE         m_convertedChar = wxPrintfFormatConverterANSI().Convert(InputAsChar());
#else
        m_convertedChar = wxPrintfFormatConverterUtf8().Convert(InputAsChar());
#endif

    return m_convertedChar.data();
}
#endif 
#if wxUSE_UNICODE && !wxUSE_UTF8_LOCALE_ONLY
const wchar_t* wxFormatString::InputAsWChar()
{
    if ( m_wchar )
        return m_wchar.data();

#if wxUSE_UNICODE_WCHAR
    if ( m_str )
        return m_str->wc_str();
    if ( m_cstr )
        return m_cstr->AsInternal();
#else     if ( m_str )
    {
        m_wchar = m_str->wc_str();
        return m_wchar.data();
    }
    if ( m_cstr )
    {
        m_wchar = m_cstr->AsWCharBuf();
        return m_wchar.data();
    }
#endif 
            wxASSERT( m_char );

    m_wchar = wxConvLibc.cMB2WC(m_char.data());

    return m_wchar.data();
}

const wchar_t* wxFormatString::AsWChar()
{
    if ( !m_convertedWChar )
        m_convertedWChar = wxPrintfFormatConverterWchar().Convert(InputAsWChar());

    return m_convertedWChar.data();
}
#endif 
wxString wxFormatString::InputAsString() const
{
    if ( m_str )
        return *m_str;
    if ( m_cstr )
        return m_cstr->AsString();
    if ( m_wchar )
        return wxString(m_wchar);
    if ( m_char )
        return wxString(m_char);

    wxFAIL_MSG( "invalid wxFormatString - not initialized?" );
    return wxString();
}


namespace
{

template<typename CharType>
wxFormatString::ArgumentType DoGetArgumentType(const CharType *format,
                                               unsigned n)
{
    wxCHECK_MSG( format, wxFormatString::Arg_Unknown,
                 "empty format string not allowed here" );

    wxPrintfConvSpecParser<CharType> parser(format);

    if ( n > parser.nargs )
    {
                                                                return wxFormatString::Arg_Unused;
    }

    wxCHECK_MSG( parser.pspec[n-1] != NULL, wxFormatString::Arg_Unknown,
                 "requested argument not found - invalid format string?" );

    switch ( parser.pspec[n-1]->m_type )
    {
        case wxPAT_CHAR:
        case wxPAT_WCHAR:
            return wxFormatString::Arg_Char;

        case wxPAT_PCHAR:
        case wxPAT_PWCHAR:
            return wxFormatString::Arg_String;

        case wxPAT_INT:
            return wxFormatString::Arg_Int;
        case wxPAT_LONGINT:
            return wxFormatString::Arg_LongInt;
#ifdef wxLongLong_t
        case wxPAT_LONGLONGINT:
            return wxFormatString::Arg_LongLongInt;
#endif
        case wxPAT_SIZET:
            return wxFormatString::Arg_Size_t;

        case wxPAT_DOUBLE:
            return wxFormatString::Arg_Double;
        case wxPAT_LONGDOUBLE:
            return wxFormatString::Arg_LongDouble;

        case wxPAT_POINTER:
            return wxFormatString::Arg_Pointer;

        case wxPAT_NINT:
            return wxFormatString::Arg_IntPtr;
        case wxPAT_NSHORTINT:
            return wxFormatString::Arg_ShortIntPtr;
        case wxPAT_NLONGINT:
            return wxFormatString::Arg_LongIntPtr;

        case wxPAT_STAR:
                        return wxFormatString::Arg_Int;

        case wxPAT_INVALID:
                        break;
    }

        wxFAIL_MSG( "unexpected argument type" );
    return wxFormatString::Arg_Unknown;
}

} 
wxFormatString::ArgumentType wxFormatString::GetArgumentType(unsigned n) const
{
    if ( m_char )
        return DoGetArgumentType(m_char.data(), n);
    else if ( m_wchar )
        return DoGetArgumentType(m_wchar.data(), n);
    else if ( m_str )
        return DoGetArgumentType(m_str->wx_str(), n);
    else if ( m_cstr )
        return DoGetArgumentType(m_cstr->AsInternal(), n);

    wxFAIL_MSG( "unreachable code" );
    return Arg_Unknown;
}
