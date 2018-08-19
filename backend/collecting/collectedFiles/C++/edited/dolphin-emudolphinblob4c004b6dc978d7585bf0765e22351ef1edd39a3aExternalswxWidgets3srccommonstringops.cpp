

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/stringops.h"
#endif


#if wxUSE_UNICODE_UTF8


const unsigned char wxStringOperationsUtf8::ms_utf8IterTable[256] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1,                                            
              2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  
        4, 4, 4, 4, 4,                                   
                               1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1   };



bool wxStringOperationsUtf8::IsValidUtf8String(const char *str, size_t len)
{
    if ( !str )
        return true; 
    const unsigned char *c = (const unsigned char*)str;
    const unsigned char * const end = (len == wxStringImpl::npos) ? NULL : c + len;

    for ( ; end != NULL ? c != end : *c; ++c )
    {
        unsigned char b = *c;

        if ( end != NULL )
        {
                                    if ( c + ms_utf8IterTable[*c] > end )
                return false;
        }

        if ( b <= 0x7F )             continue;

        else if ( b < 0xC2 )             return false;

                else if ( b <= 0xDF )         {
            b = *(++c);
            if ( !(b >= 0x80 && b <= 0xBF ) )
                return false;
        }

                else if ( b == 0xE0 )
        {
            b = *(++c);
            if ( !(b >= 0xA0 && b <= 0xBF ) )
                return false;
            b = *(++c);
            if ( !(b >= 0x80 && b <= 0xBF ) )
                return false;
        }
        else if ( b == 0xED )
        {
            b = *(++c);
            if ( !(b >= 0x80 && b <= 0x9F ) )
                return false;
            b = *(++c);
            if ( !(b >= 0x80 && b <= 0xBF ) )
                return false;
        }
        else if ( b <= 0xEF )         {
            for ( int i = 0; i < 2; ++i )
            {
                b = *(++c);
                if ( !(b >= 0x80 && b <= 0xBF ) )
                    return false;
            }
        }

                else if ( b == 0xF0 )
        {
            b = *(++c);
            if ( !(b >= 0x90 && b <= 0xBF ) )
                return false;
            for ( int i = 0; i < 2; ++i )
            {
                b = *(++c);
                if ( !(b >= 0x80 && b <= 0xBF ) )
                    return false;
            }
        }
        else if ( b <= 0xF3 )         {
            for ( int i = 0; i < 3; ++i )
            {
                b = *(++c);
                if ( !(b >= 0x80 && b <= 0xBF ) )
                    return false;
            }
        }
        else if ( b == 0xF4 )
        {
            b = *(++c);
            if ( !(b >= 0x80 && b <= 0x8F ) )
                return false;
            for ( int i = 0; i < 2; ++i )
            {
                b = *(++c);
                if ( !(b >= 0x80 && b <= 0xBF ) )
                    return false;
            }
        }
        else             return false;
    }

    return true;
}

wxUniChar::Utf8CharBuffer wxUniChar::AsUTF8() const
{
    Utf8CharBuffer buf = { "" };     char *out = buf.data;

    value_type code = GetValue();

                                            
    if ( code <= 0x7F )
    {
        out[1] = 0;
        out[0] = (char)code;
    }
    else if ( code <= 0x07FF )
    {
        out[2] = 0;
                        out[1] = 0x80 | (code & 0x3F);  code >>= 6;
        out[0] = 0xC0 | code;
    }
    else if ( code < 0xFFFF )
    {
        out[3] = 0;
        out[2] = 0x80 | (code & 0x3F);  code >>= 6;
        out[1] = 0x80 | (code & 0x3F);  code >>= 6;
        out[0] = 0xE0 | code;
    }
    else if ( code <= 0x10FFFF )
    {
        out[4] = 0;
        out[3] = 0x80 | (code & 0x3F);  code >>= 6;
        out[2] = 0x80 | (code & 0x3F);  code >>= 6;
        out[1] = 0x80 | (code & 0x3F);  code >>= 6;
        out[0] = 0xF0 | code;
    }
    else
    {
        wxFAIL_MSG( wxT("trying to encode undefined Unicode character") );
        out[0] = 0;
    }

    return buf;
}

wxUniChar
wxStringOperationsUtf8::DecodeNonAsciiChar(wxStringImpl::const_iterator i)
{
    wxASSERT( IsValidUtf8LeadByte(*i) );

    size_t len = GetUtf8CharLength(*i);
    wxASSERT_MSG( len <= 4, wxT("invalid UTF-8 sequence length") );

                                            
        static const unsigned char s_leadValueMask[4] =  { 0x7F, 0x1F, 0x0F, 0x07 };
#if wxDEBUG_LEVEL
        static const unsigned char s_leadMarkerMask[4] = { 0x80, 0xE0, 0xF0, 0xF8 };
    static const unsigned char s_leadMarkerVal[4] =  { 0x00, 0xC0, 0xE0, 0xF0 };
#endif

        wxASSERT_MSG( ((unsigned char)*i & s_leadMarkerMask[len-1]) ==
                  s_leadMarkerVal[len-1],
                  wxT("invalid UTF-8 lead byte") );
    wxUniChar::value_type code = (unsigned char)*i & s_leadValueMask[len-1];

            for ( ++i ; len > 1; --len, ++i )
    {
        wxASSERT_MSG( ((unsigned char)*i & 0xC0) == 0x80,
                      wxT("invalid UTF-8 byte") );

        code <<= 6;
        code |= (unsigned char)*i & 0x3F;
    }

    return wxUniChar(code);
}

wxCharBuffer wxStringOperationsUtf8::EncodeNChars(size_t n, const wxUniChar& ch)
{
    Utf8CharBuffer once(EncodeChar(ch));
        size_t len = ms_utf8IterTable[(unsigned char)once.data[0]];

    wxCharBuffer buf(n * len);
    char *ptr = buf.data();
    for ( size_t i = 0; i < n; i++, ptr += len )
    {
        memcpy(ptr, once.data, len);
    }

    return buf;
}

#endif 