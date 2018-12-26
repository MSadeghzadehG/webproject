
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/ustring.h"

#ifndef WX_PRECOMP
    #include "wx/crt.h"
    #include "wx/log.h"
#endif

wxUString &wxUString::assignFromAscii( const char *str )
{
   size_type len = wxStrlen( str );

   wxU32CharBuffer buffer( len );
   wxChar32 *ptr = buffer.data();

   size_type i;
   for (i = 0; i < len; i++)
   {
       *ptr = *str;
       ptr++;
       str++;
   }

   return assign( buffer );
}

wxUString &wxUString::assignFromAscii( const char *str, size_type n )
{
   size_type len = 0;
   const char *s = str;
   while (len < n && *s)
   {
       len++;
       s++;
   }

   wxU32CharBuffer buffer( len );
   wxChar32 *ptr = buffer.data();

   size_type i;
   for (i = 0; i < len; i++)
   {
       *ptr = *str;
       ptr++;
       str++;
   }

   return *this;
}


const unsigned char tableUtf8Lengths[256] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,      0, 0,                                            
              2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  
        4, 4, 4, 4, 4,                                   
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   };

wxUString &wxUString::assignFromUTF8( const char *str )
{
    if (!str)
        return assign( wxUString() );

    size_type ucs4_len = 0;
    const char *p = str;
    while (*p)
    {
        unsigned char c = *p;
        size_type len = tableUtf8Lengths[c];
        if (!len)
           return assign( wxUString() );          ucs4_len++;
        p += len;
    }

    wxU32CharBuffer buffer( ucs4_len );
    wxChar32 *out = buffer.data();

    p = str;
    while (*p)
    {
        unsigned char c = *p;
        if (c < 0x80)
        {
            *out = c;
            p++;
        }
        else
        {
            size_type len = tableUtf8Lengths[c];  
                                                                                                                                    
                                    static const unsigned char leadValueMask[] = { 0x7F, 0x1F, 0x0F, 0x07 };

                        static const unsigned char leadMarkerMask[] = { 0x80, 0xE0, 0xF0, 0xF8 };
            static const unsigned char leadMarkerVal[] = { 0x00, 0xC0, 0xE0, 0xF0 };

            len--; 
                        if ( (c & leadMarkerMask[len]) != leadMarkerVal[len] )
                break;

            wxChar32 code = c & leadValueMask[len];

                                    for ( ; len; --len )
            {
                c = *++p;
                if ( (c & 0xC0) != 0x80 )
                    return assign( wxUString() );  
                code <<= 6;
                code |= c & 0x3F;
            }

            *out = code;
            p++;
        }
        out++;
    }

    return assign( buffer.data() );
}

wxUString &wxUString::assignFromUTF8( const char *str, size_type n )
{
    if (!str)
        return assign( wxUString() );

    size_type ucs4_len = 0;
    size_type utf8_pos = 0;
    const char *p = str;
    while (*p)
    {
        unsigned char c = *p;
        size_type len = tableUtf8Lengths[c];
        if (!len)
           return assign( wxUString() );          if (utf8_pos + len > n)
            break;
        utf8_pos += len;
        ucs4_len ++;
        p += len;
    }

    wxU32CharBuffer buffer( ucs4_len );
    wxChar32 *out = buffer.data();

    utf8_pos = 0;
    p = str;
    while (*p)
    {
        unsigned char c = *p;
        if (c < 0x80)
        {
            if (utf8_pos + 1 > n)
                break;
            utf8_pos++;

            *out = c;
            p++;
        }
        else
        {
            size_type len = tableUtf8Lengths[c];              if (utf8_pos + len > n)
                break;
            utf8_pos += len;

                                                                                                                                    
                                    static const unsigned char leadValueMask[] = { 0x7F, 0x1F, 0x0F, 0x07 };

                        static const unsigned char leadMarkerMask[] = { 0x80, 0xE0, 0xF0, 0xF8 };
            static const unsigned char leadMarkerVal[] = { 0x00, 0xC0, 0xE0, 0xF0 };

            len--; 
                        if ( (c & leadMarkerMask[len]) != leadMarkerVal[len] )
                break;

            wxChar32 code = c & leadValueMask[len];

                                    for ( ; len; --len )
            {
                c = *++p;
                if ( (c & 0xC0) != 0x80 )
                    return assign( wxUString() );  
                code <<= 6;
                code |= c & 0x3F;
            }

            *out = code;
            p++;
        }
        out++;
    }

    *out = 0;

    return assign( buffer.data() );
}

wxUString &wxUString::assignFromUTF16( const wxChar16* str, size_type n )
{
    if (!str)
        return assign( wxUString() );

    size_type ucs4_len = 0;
    size_type utf16_pos = 0;
    const wxChar16 *p = str;
    while (*p)
    {
        size_type len;
        if ((*p < 0xd800) || (*p > 0xdfff))
        {
            len = 1;
        }
        else if ((p[1] < 0xdc00) || (p[1] > 0xdfff))
        {
            return assign( wxUString() );          }
        else
        {
           len = 2;
        }

        if (utf16_pos + len > n)
            break;

        ucs4_len++;
        p += len;
        utf16_pos += len;
    }

    wxU32CharBuffer buffer( ucs4_len );
    wxChar32 *out = buffer.data();

    utf16_pos = 0;

    p = str;
    while (*p)
    {
        if ((*p < 0xd800) || (*p > 0xdfff))
        {
            if (utf16_pos + 1 > n)
                break;

            *out = *p;
            p++;
            utf16_pos++;
        }
        else
        {
            if (utf16_pos + 2 > n)
                break;

           *out = ((p[0] - 0xd7c0) << 10) + (p[1] - 0xdc00);
           p += 2;
           utf16_pos += 2;
        }
        out++;
    }

    return assign( buffer.data() );
}

wxUString &wxUString::assignFromUTF16( const wxChar16* str )
{
    if (!str)
        return assign( wxUString() );

    size_type ucs4_len = 0;
    const wxChar16 *p = str;
    while (*p)
    {
        size_type len;
        if ((*p < 0xd800) || (*p > 0xdfff))
        {
            len = 1;
        }
        else if ((p[1] < 0xdc00) || (p[1] > 0xdfff))
        {
            return assign( wxUString() );          }
        else
        {
           len = 2;
        }

        ucs4_len++;
        p += len;
    }

    wxU32CharBuffer buffer( ucs4_len );
    wxChar32 *out = buffer.data();

    p = str;
    while (*p)
    {
        if ((*p < 0xd800) || (*p > 0xdfff))
        {
            *out = *p;
            p++;
        }
        else
        {
           *out = ((p[0] - 0xd7c0) << 10) + (p[1] - 0xdc00);
           p += 2;
        }
        out++;
    }

    return assign( buffer.data() );
}

wxUString &wxUString::assignFromCString( const char* str )
{
    if (!str)
        return assign( wxUString() );

    wxScopedWCharBuffer buffer = wxConvLibc.cMB2WC( str );

    return assign( buffer );
}

wxUString &wxUString::assignFromCString( const char* str, const wxMBConv &conv )
{
    if (!str)
        return assign( wxUString() );

    wxScopedWCharBuffer buffer = conv.cMB2WC( str );

    return assign( buffer );
}

wxScopedCharBuffer wxUString::utf8_str() const
{
    size_type utf8_length = 0;
    const wxChar32 *ptr = data();

    while (*ptr)
    {
        wxChar32 code = *ptr;
        ptr++;

        if ( code <= 0x7F )
        {
            utf8_length++;
        }
        else if ( code <= 0x07FF )
        {
            utf8_length += 2;
        }
        else if ( code < 0xFFFF )
        {
            utf8_length += 3;
        }
        else if ( code <= 0x10FFFF )
        {
            utf8_length += 4;
        }
        else
        {
                    }
    }

    wxCharBuffer result( utf8_length );

    char *out = result.data();

    ptr = data();
    while (*ptr)
    {
        wxChar32 code = *ptr;
        ptr++;

        if ( code <= 0x7F )
        {
            out[0] = (char)code;
            out++;
        }
        else if ( code <= 0x07FF )
        {
            out[1] = 0x80 | (code & 0x3F);  code >>= 6;
            out[0] = 0xC0 | code;
            out += 2;
        }
        else if ( code < 0xFFFF )
        {
            out[2] = 0x80 | (code & 0x3F);  code >>= 6;
            out[1] = 0x80 | (code & 0x3F);  code >>= 6;
            out[0] = 0xE0 | code;
            out += 3;
        }
        else if ( code <= 0x10FFFF )
        {
            out[3] = 0x80 | (code & 0x3F);  code >>= 6;
            out[2] = 0x80 | (code & 0x3F);  code >>= 6;
            out[1] = 0x80 | (code & 0x3F);  code >>= 6;
            out[0] = 0xF0 | code;
            out += 4;
        }
        else
        {
                    }
    }

    return result;
}

wxScopedU16CharBuffer wxUString::utf16_str() const
{
    size_type utf16_length = 0;
    const wxChar32 *ptr = data();

    while (*ptr)
    {
        wxChar32 code = *ptr;
        ptr++;

        
        if (code < 0x10000)
           utf16_length++;
        else
           utf16_length += 2;
    }

    wxU16CharBuffer result( utf16_length );
    wxChar16 *out = result.data();

    ptr = data();

    while (*ptr)
    {
        wxChar32 code = *ptr;
        ptr++;

        
        if (code < 0x10000)
        {
           out[0] = code;
           out++;
        }
        else
        {
           out[0] = (code - 0x10000) / 0x400 + 0xd800;
           out[1] = (code - 0x10000) % 0x400 + 0xdc00;
           out += 2;
        }
    }

    return result;
}
