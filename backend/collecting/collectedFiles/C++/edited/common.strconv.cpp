
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif  
#ifndef WX_PRECOMP
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/utils.h"
    #include "wx/hashmap.h"
#endif

#include "wx/strconv.h"

#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#if defined(__WIN32__)
    #include "wx/msw/private.h"
    #include "wx/msw/missing.h"
    #define wxHAVE_WIN32_MB2WC
#endif

#ifdef HAVE_ICONV
    #include <iconv.h>
    #include "wx/thread.h"
#endif

#include "wx/encconv.h"
#include "wx/fontmap.h"

#ifdef __DARWIN__
#include "wx/osx/core/private/strconv_cf.h"
#endif 

#define TRACE_STRCONV wxT("strconv")

#if SIZEOF_WCHAR_T == 2
    #define WC_UTF16
#endif



static bool NotAllNULs(const char *p, size_t n)
{
    while ( n && *p++ == '\0' )
        n--;

    return n != 0;
}


static size_t encode_utf16(wxUint32 input, wxUint16 *output)
{
    if (input <= 0xffff)
    {
        if (output)
            *output = (wxUint16) input;

        return 1;
    }
    else if (input >= 0x110000)
    {
        return wxCONV_FAILED;
    }
    else
    {
        if (output)
        {
            *output++ = (wxUint16) ((input >> 10) + 0xd7c0);
            *output = (wxUint16) ((input & 0x3ff) + 0xdc00);
        }

        return 2;
    }
}

static size_t decode_utf16(const wxUint16* input, wxUint32& output)
{
    if ((*input < 0xd800) || (*input > 0xdfff))
    {
        output = *input;
        return 1;
    }
    else if ((input[1] < 0xdc00) || (input[1] > 0xdfff))
    {
        output = *input;
        return wxCONV_FAILED;
    }
    else
    {
        output = ((input[0] - 0xd7c0) << 10) + (input[1] - 0xdc00);
        return 2;
    }
}

static wxUint32 wxDecodeSurrogate(const wxChar16 **pSrc)
{
    wxUint32 out;
    const size_t
        n = decode_utf16(reinterpret_cast<const wxUint16 *>(*pSrc), out);
    if ( n == wxCONV_FAILED )
        *pSrc = NULL;
    else
        *pSrc += n;

    return out;
}


size_t
wxMBConv::ToWChar(wchar_t *dst, size_t dstLen,
                  const char *src, size_t srcLen) const
{
                                        
        size_t dstWritten = 0;

        size_t nulLen = 0;  
                    wxCharBuffer bufTmp;
    const char *srcEnd;
    if ( srcLen != wxNO_LEN )
    {
                nulLen = GetMBNulLen();
        if ( nulLen == wxCONV_FAILED )
            return wxCONV_FAILED;

                if ( srcLen < nulLen || NotAllNULs(src + srcLen - nulLen, nulLen) )
        {
                        bufTmp = wxCharBuffer(srcLen + nulLen - 1 );
            char * const p = bufTmp.data();
            memcpy(p, src, srcLen);
            for ( char *s = p + srcLen; s < p + srcLen + nulLen; s++ )
                *s = '\0';

            src = bufTmp;
        }

        srcEnd = src + srcLen;
    }
    else     {
        srcEnd = NULL;
    }

                                                                for ( ;; )
    {
                size_t lenChunk = MB2WC(NULL, src, 0);
        if ( lenChunk == wxCONV_FAILED )
            return wxCONV_FAILED;

        dstWritten += lenChunk;
        if ( !srcEnd )
            dstWritten++;

        if ( dst )
        {
            if ( dstWritten > dstLen )
                return wxCONV_FAILED;

                        if ( MB2WC(dst, src, lenChunk + 1) == wxCONV_FAILED )
                return wxCONV_FAILED;

            dst += lenChunk;
            if ( !srcEnd )
                dst++;
        }

        if ( !srcEnd )
        {
                                    break;
        }

                                while ( NotAllNULs(src, nulLen) )
        {
                                                            src += nulLen;
        }

                        if ( src == srcEnd )
            break;

                dstWritten++;
        if ( dst )
            dst++;

        src += nulLen; 
        if ( src >= srcEnd )
            break;
    }

    return dstWritten;
}

size_t
wxMBConv::FromWChar(char *dst, size_t dstLen,
                    const wchar_t *src, size_t srcLen) const
{
        size_t dstWritten = 0;

                const bool isNulTerminated = srcLen == wxNO_LEN;

            wxWCharBuffer bufTmp;
    if ( isNulTerminated )
    {
        srcLen = wxWcslen(src) + 1;
    }
    else if ( srcLen != 0 && src[srcLen - 1] != L'\0' )
    {
                bufTmp = wxWCharBuffer(srcLen);
        memcpy(bufTmp.data(), src, srcLen * sizeof(wchar_t));
        src = bufTmp;
    }

    const size_t lenNul = GetMBNulLen();
    for ( const wchar_t * const srcEnd = src + srcLen;
          src < srcEnd;
          src++  )
    {
                size_t lenChunk = WC2MB(NULL, src, 0);
        if ( lenChunk == wxCONV_FAILED )
            return wxCONV_FAILED;

        dstWritten += lenChunk;

        const wchar_t * const
            chunkEnd = isNulTerminated ? srcEnd - 1 : src + wxWcslen(src);

                                if ( chunkEnd < srcEnd )
            dstWritten += lenNul;

        if ( dst )
        {
            if ( dstWritten > dstLen )
                return wxCONV_FAILED;

                                                            wxCharBuffer dstBuf;
            char *dstTmp;
            if ( chunkEnd == srcEnd )
            {
                dstBuf = wxCharBuffer(lenChunk + lenNul - 1);
                dstTmp = dstBuf.data();
            }
            else
            {
                dstTmp = dst;
            }

            if ( WC2MB(dstTmp, src, lenChunk + lenNul) == wxCONV_FAILED )
                return wxCONV_FAILED;

            if ( dstTmp != dst )
            {
                                                memcpy(dst, dstTmp, lenChunk);

                                                break;
            }

            dst += lenChunk;
            if ( chunkEnd < srcEnd )
                dst += lenNul;
        }

        src = chunkEnd;
    }

    return dstWritten;
}

size_t wxMBConv::MB2WC(wchar_t *outBuff, const char *inBuff, size_t outLen) const
{
    size_t rc = ToWChar(outBuff, outLen, inBuff);
    if ( rc != wxCONV_FAILED )
    {
                        rc--;
    }

    return rc;
}

size_t wxMBConv::WC2MB(char *outBuff, const wchar_t *inBuff, size_t outLen) const
{
    size_t rc = FromWChar(outBuff, outLen, inBuff);
    if ( rc != wxCONV_FAILED )
    {
        rc -= GetMBNulLen();
    }

    return rc;
}

wxMBConv::~wxMBConv()
{
    }

const wxWCharBuffer wxMBConv::cMB2WC(const char *psz) const
{
    if ( psz )
    {
                const size_t nLen = ToWChar(NULL, 0, psz);
        if ( nLen != wxCONV_FAILED )
        {
                        wxWCharBuffer buf(nLen - 1 );

                        if ( ToWChar(buf.data(), nLen, psz) != wxCONV_FAILED )
                return buf;
        }
    }

    return wxWCharBuffer();
}

const wxCharBuffer wxMBConv::cWC2MB(const wchar_t *pwz) const
{
    if ( pwz )
    {
        const size_t nLen = FromWChar(NULL, 0, pwz);
        if ( nLen != wxCONV_FAILED )
        {
            wxCharBuffer buf(nLen - 1);
            if ( FromWChar(buf.data(), nLen, pwz) != wxCONV_FAILED )
                return buf;
        }
    }

    return wxCharBuffer();
}

const wxWCharBuffer
wxMBConv::cMB2WC(const char *inBuff, size_t inLen, size_t *outLen) const
{
    const size_t dstLen = ToWChar(NULL, 0, inBuff, inLen);
    if ( dstLen != wxCONV_FAILED )
    {
                                wxWCharBuffer wbuf(dstLen);
        if ( ToWChar(wbuf.data(), dstLen, inBuff, inLen) != wxCONV_FAILED )
        {
            if ( outLen )
            {
                *outLen = dstLen;

                                                                                                if ( inLen == wxNO_LEN )
                    (*outLen)--;
            }

            return wbuf;
        }
    }

    if ( outLen )
        *outLen = 0;

    return wxWCharBuffer();
}

const wxCharBuffer
wxMBConv::cWC2MB(const wchar_t *inBuff, size_t inLen, size_t *outLen) const
{
    size_t dstLen = FromWChar(NULL, 0, inBuff, inLen);
    if ( dstLen != wxCONV_FAILED )
    {
        const size_t nulLen = GetMBNulLen();

                        wxCharBuffer buf(dstLen + nulLen - 1);
        memset(buf.data() + dstLen, 0, nulLen);

                                dstLen = FromWChar(buf.data(), dstLen, inBuff, inLen);
        if ( dstLen != wxCONV_FAILED )
        {
            if ( outLen )
            {
                *outLen = dstLen;

                if ( inLen == wxNO_LEN )
                {
                                                            *outLen -= nulLen;
                }
            }

            return buf;
        }
    }

    if ( outLen )
        *outLen = 0;

    return wxCharBuffer();
}

const wxWCharBuffer wxMBConv::cMB2WC(const wxScopedCharBuffer& buf) const
{
    const size_t srcLen = buf.length();
    if ( srcLen )
    {
        const size_t dstLen = ToWChar(NULL, 0, buf, srcLen);
        if ( dstLen != wxCONV_FAILED )
        {
            wxWCharBuffer wbuf(dstLen);
            wbuf.data()[dstLen] = L'\0';
            if ( ToWChar(wbuf.data(), dstLen, buf, srcLen) != wxCONV_FAILED )
                return wbuf;
        }
    }

    return wxScopedWCharBuffer::CreateNonOwned(L"", 0);
}

const wxCharBuffer wxMBConv::cWC2MB(const wxScopedWCharBuffer& wbuf) const
{
    const size_t srcLen = wbuf.length();
    if ( srcLen )
    {
        const size_t dstLen = FromWChar(NULL, 0, wbuf, srcLen);
        if ( dstLen != wxCONV_FAILED )
        {
            wxCharBuffer buf(dstLen);
            buf.data()[dstLen] = '\0';
            if ( FromWChar(buf.data(), dstLen, wbuf, srcLen) != wxCONV_FAILED )
                return buf;
        }
    }

    return wxScopedCharBuffer::CreateNonOwned("", 0);
}


size_t wxMBConvLibc::MB2WC(wchar_t *buf, const char *psz, size_t n) const
{
    return wxMB2WC(buf, psz, n);
}

size_t wxMBConvLibc::WC2MB(char *buf, const wchar_t *psz, size_t n) const
{
    return wxWC2MB(buf, psz, n);
}


#ifdef __UNIX__

wxConvBrokenFileNames::wxConvBrokenFileNames(const wxString& charset)
{
    if ( wxStricmp(charset, wxT("UTF-8")) == 0 ||
         wxStricmp(charset, wxT("UTF8")) == 0  )
        m_conv = new wxMBConvUTF8(wxMBConvUTF8::MAP_INVALID_UTF8_TO_PUA);
    else
        m_conv = new wxCSConv(charset);
}

#endif 


static const unsigned char utf7unb64[] =
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0x3e, 0xff, 0xff, 0xff, 0x3f,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
    0x3c, 0x3d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
    0x31, 0x32, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

size_t wxMBConvUTF7::ToWChar(wchar_t *dst, size_t dstLen,
                             const char *src, size_t srcLen) const
{
    DecoderState stateOrig,
                *statePtr;
    if ( srcLen == wxNO_LEN )
    {
                srcLen = strlen(src) + 1;

                        statePtr = &stateOrig;
    }
    else     {
        statePtr = const_cast<DecoderState *>(&m_stateDecoder);

                stateOrig = m_stateDecoder;
    }

        DecoderState& state = *statePtr;


            size_t len = 0;

    const char * const srcEnd = src + srcLen;

    while ( (src < srcEnd) && (!dst || (len < dstLen)) )
    {
        const unsigned char cc = *src++;

        if ( state.IsShifted() )
        {
            const unsigned char dc = utf7unb64[cc];
            if ( dc == 0xff )
            {
                                                                                                if ( state.isLSB || state.bit > 4 ||
                        (state.accum & ((1 << state.bit) - 1)) )
                {
                    if ( !len )
                        state = stateOrig;

                    return wxCONV_FAILED;
                }

                state.ToDirect();

                                                if ( cc == '-' )
                    continue;
            }
            else             {
                                state.bit += 6;
                state.accum <<= 6;
                state.accum += dc;

                if ( state.bit >= 8 )
                {
                                        state.bit -= 8;
                    unsigned char b = (state.accum >> state.bit) & 0x00ff;

                    if ( state.isLSB )
                    {
                                                if ( dst )
                            *dst++ = (state.msb << 8) | b;
                        len++;
                        state.isLSB = false;
                    }
                    else                     {
                                                state.msb = b;
                        state.isLSB = true;
                    }
                }
            }
        }

        if ( state.IsDirect() )
        {
                        if ( cc == '+' )
            {
                if ( *src == '-' )
                {
                                        if ( dst )
                        *dst++ = '+';
                    len++;
                    src++;
                }
                else if ( utf7unb64[(unsigned)*src] == 0xff )
                {
                                        if ( !len )
                        state = stateOrig;

                    return wxCONV_FAILED;
                }
                else                 {
                    state.ToShifted();
                }
            }
            else             {
                                                if ( cc >= 0x7f || (cc < ' ' &&
                      !(cc == '\0' || cc == '\t' || cc == '\r' || cc == '\n')) )
                    return wxCONV_FAILED;

                if ( dst )
                    *dst++ = cc;
                len++;
            }
        }
    }

    if ( !len )
    {
                                state = stateOrig;

        return wxCONV_FAILED;
    }

    return len;
}

static const unsigned char utf7enb64[] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

static const unsigned char utf7encode[128] =
{
    0, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 3, 3, 2, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 3, 0, 0, 0, 3,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 3, 3
};

static inline bool wxIsUTF7Direct(wchar_t wc)
{
    return wc < 0x80 && utf7encode[wc] < 1;
}

size_t wxMBConvUTF7::FromWChar(char *dst, size_t dstLen,
                               const wchar_t *src, size_t srcLen) const
{
    EncoderState stateOrig,
                *statePtr;
    if ( srcLen == wxNO_LEN )
    {
                        statePtr = &stateOrig;

        srcLen = wxWcslen(src) + 1;
    }
    else     {
        stateOrig = m_stateEncoder;
        statePtr = const_cast<EncoderState *>(&m_stateEncoder);
    }

    EncoderState& state = *statePtr;


    size_t len = 0;

    const wchar_t * const srcEnd = src + srcLen;
    while ( src < srcEnd && (!dst || len < dstLen) )
    {
        wchar_t cc = *src++;
        if ( wxIsUTF7Direct(cc) )
        {
            if ( state.IsShifted() )
            {
                                if ( state.bit )
                {
                    if ( dst )
                        *dst++ = utf7enb64[((state.accum % 16) << (6 - state.bit)) % 64];
                    len++;
                }

                state.ToDirect();

                if ( dst )
                    *dst++ = '-';
                len++;
            }

            if ( dst )
                *dst++ = (char)cc;
            len++;
        }
        else if ( cc == '+' && state.IsDirect() )
        {
            if ( dst )
            {
                *dst++ = '+';
                *dst++ = '-';
            }

            len += 2;
        }
#ifndef WC_UTF16
        else if (((wxUint32)cc) > 0xffff)
        {
                        return wxCONV_FAILED;
        }
#endif
        else
        {
            if ( state.IsDirect() )
            {
                state.ToShifted();

                if ( dst )
                    *dst++ = '+';
                len++;
            }

                        for ( ;; )
            {
                for ( unsigned lsb = 0; lsb < 2; lsb++ )
                {
                    state.accum <<= 8;
                    state.accum += lsb ? cc & 0xff : (cc & 0xff00) >> 8;

                    for (state.bit += 8; state.bit >= 6; )
                    {
                        state.bit -= 6;
                        if ( dst )
                            *dst++ = utf7enb64[(state.accum >> state.bit) % 64];
                        len++;
                    }
                }

                if ( src == srcEnd || wxIsUTF7Direct(cc = *src) )
                    break;

                src++;
            }
        }
    }

                if ( !dst )
        state = stateOrig;

    return len;
}


static const wxUint32 utf8_max[]=
    { 0x7f, 0x7ff, 0xffff, 0x1fffff, 0x3ffffff, 0x7fffffff, 0xffffffff };

const wxUint32 wxUnicodePUA = 0x100000;
const wxUint32 wxUnicodePUAEnd = wxUnicodePUA + 256;

const unsigned char tableUtf8Lengths[256] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,      0, 0,                                            
              2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  
        4, 4, 4, 4, 4,                                   
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   };

size_t
wxMBConvStrictUTF8::ToWChar(wchar_t *dst, size_t dstLen,
                            const char *src, size_t srcLen) const
{
    wchar_t *out = dstLen ? dst : NULL;
    size_t written = 0;

    if ( srcLen == wxNO_LEN )
        srcLen = strlen(src) + 1;

    for ( const char *p = src; ; p++ )
    {
        if ( (srcLen == wxNO_LEN ? !*p : !srcLen) )
        {
                                    if ( srcLen == wxNO_LEN )
            {
                if ( out )
                {
                    if ( !dstLen )
                        break;

                    *out = L'\0';
                }

                written++;
            }

            return written;
        }

        if ( out && !dstLen-- )
            break;

        wxUint32 code;
        unsigned char c = *p;

        if ( c < 0x80 )
        {
            if ( srcLen == 0 )                 break;

            if ( srcLen != wxNO_LEN )
                srcLen--;

            code = c;
        }
        else
        {
            unsigned len = tableUtf8Lengths[c];
            if ( !len )
                break;

            if ( srcLen < len )                 break;

            if ( srcLen != wxNO_LEN )
                srcLen -= len;

                                                                                                                                    
                                    static const unsigned char leadValueMask[] = { 0x7F, 0x1F, 0x0F, 0x07 };

            len--; 
            code = c & leadValueMask[len];

                                    for ( ; len; --len )
            {
                c = *++p;
                if ( (c & 0xC0) != 0x80 )
                    return wxCONV_FAILED;

                code <<= 6;
                code |= c & 0x3F;
            }
        }

#ifdef WC_UTF16
                if ( encode_utf16(code, (wxUint16 *)out) == 2 )
        {
            if ( out )
                out++;
            written++;
        }
#else         if ( out )
            *out = code;
#endif 
        if ( out )
            out++;

        written++;
    }

    return wxCONV_FAILED;
}

size_t
wxMBConvStrictUTF8::FromWChar(char *dst, size_t dstLen,
                              const wchar_t *src, size_t srcLen) const
{
    char *out = dstLen ? dst : NULL;
    size_t written = 0;

    for ( const wchar_t *wp = src; ; wp++ )
    {
        if ( (srcLen == wxNO_LEN ? !*wp : !srcLen) )
        {
                                    if ( srcLen == wxNO_LEN )
            {
                if ( out )
                {
                    if ( !dstLen )
                        break;

                    *out = '\0';
                }

                written++;
            }

            return written;
        }

        if ( srcLen != wxNO_LEN )
            srcLen--;

        wxUint32 code;
#ifdef WC_UTF16
                                                        wxUint16 tmp[2];
        tmp[0] = wp[0];
        tmp[1] = srcLen != 0 ? wp[1] : 0;
        switch ( decode_utf16(tmp, code) )
        {
            case 1:
                                break;

            case 2:
                                wp++;
                if ( srcLen != wxNO_LEN )
                    srcLen--;
                break;

            case wxCONV_FAILED:
                return wxCONV_FAILED;
        }
#else         code = *wp & 0x7fffffff;
#endif

        unsigned len;
        if ( code <= 0x7F )
        {
            len = 1;
            if ( out )
            {
                if ( dstLen < len )
                    break;

                out[0] = (char)code;
            }
        }
        else if ( code <= 0x07FF )
        {
            len = 2;
            if ( out )
            {
                if ( dstLen < len )
                    break;

                                                out[1] = 0x80 | (code & 0x3F);  code >>= 6;
                out[0] = 0xC0 | code;
            }
        }
        else if ( code < 0xFFFF )
        {
            len = 3;
            if ( out )
            {
                if ( dstLen < len )
                    break;

                out[2] = 0x80 | (code & 0x3F);  code >>= 6;
                out[1] = 0x80 | (code & 0x3F);  code >>= 6;
                out[0] = 0xE0 | code;
            }
        }
        else if ( code <= 0x10FFFF )
        {
            len = 4;
            if ( out )
            {
                if ( dstLen < len )
                    break;

                out[3] = 0x80 | (code & 0x3F);  code >>= 6;
                out[2] = 0x80 | (code & 0x3F);  code >>= 6;
                out[1] = 0x80 | (code & 0x3F);  code >>= 6;
                out[0] = 0xF0 | code;
            }
        }
        else
        {
            wxFAIL_MSG( wxT("trying to encode undefined Unicode character") );
            break;
        }

        if ( out )
        {
            out += len;
            dstLen -= len;
        }

        written += len;
    }

        return wxCONV_FAILED;
}

size_t wxMBConvUTF8::ToWChar(wchar_t *buf, size_t n,
                             const char *psz, size_t srcLen) const
{
    if ( m_options == MAP_INVALID_UTF8_NOT )
        return wxMBConvStrictUTF8::ToWChar(buf, n, psz, srcLen);

    size_t len = 0;

            const bool isNulTerminated = srcLen == wxNO_LEN;
    while ((isNulTerminated ? *psz : srcLen--) && ((!buf) || (len < n)))
    {
        const char *opsz = psz;
        bool invalid = false;
        unsigned char cc = *psz++, fc = cc;
        unsigned cnt;
        for (cnt = 0; fc & 0x80; cnt++)
            fc <<= 1;

        if (!cnt)
        {
                        if (buf)
                *buf++ = cc;
            len++;

                        if ((m_options & MAP_INVALID_UTF8_TO_OCTAL)
                    && cc == '\\' && (!buf || len < n))
            {
                if (buf)
                    *buf++ = cc;
                len++;
            }
        }
        else
        {
            cnt--;
            if (!cnt)
            {
                                invalid = true;
            }
            else
            {
                unsigned ocnt = cnt - 1;
                wxUint32 res = cc & (0x3f >> cnt);
                while (cnt--)
                {
                    if (!isNulTerminated && !srcLen)
                    {
                                                                        invalid = true;
                        break;
                    }

                    cc = *psz;
                    if ((cc & 0xC0) != 0x80)
                    {
                                                invalid = true;
                        break;
                    }

                    psz++;
                    if (!isNulTerminated)
                        srcLen--;
                    res = (res << 6) | (cc & 0x3f);
                }

                if (invalid || res <= utf8_max[ocnt])
                {
                                        invalid = true;
                }
                else if ((m_options & MAP_INVALID_UTF8_TO_PUA) &&
                        res >= wxUnicodePUA && res < wxUnicodePUAEnd)
                {
                                                                                invalid = true;
                }
                else
                {
#ifdef WC_UTF16
                                        size_t pa = encode_utf16(res, (wxUint16 *)buf);
                    if (pa == wxCONV_FAILED)
                    {
                        invalid = true;
                    }
                    else
                    {
                        if (buf)
                            buf += pa;
                        len += pa;
                    }
#else                     if (buf)
                        *buf++ = (wchar_t)res;
                    len++;
#endif                 }
            }

            if (invalid)
            {
                if (m_options & MAP_INVALID_UTF8_TO_PUA)
                {
                    while (opsz < psz && (!buf || len < n))
                    {
#ifdef WC_UTF16
                                                size_t pa = encode_utf16((unsigned char)*opsz + wxUnicodePUA, (wxUint16 *)buf);
                        wxASSERT(pa != wxCONV_FAILED);
                        if (buf)
                            buf += pa;
                        opsz++;
                        len += pa;
#else
                        if (buf)
                            *buf++ = (wchar_t)(wxUnicodePUA + (unsigned char)*opsz);
                        opsz++;
                        len++;
#endif
                    }
                }
                else if (m_options & MAP_INVALID_UTF8_TO_OCTAL)
                {
                    while (opsz < psz && (!buf || len < n))
                    {
                        if ( buf && len + 3 < n )
                        {
                            unsigned char on = *opsz;
                            *buf++ = L'\\';
                            *buf++ = (wchar_t)( L'0' + on / 0100 );
                            *buf++ = (wchar_t)( L'0' + (on % 0100) / 010 );
                            *buf++ = (wchar_t)( L'0' + on % 010 );
                        }

                        opsz++;
                        len += 4;
                    }
                }
                else                 {
                    return wxCONV_FAILED;
                }
            }
        }
    }

    if ( isNulTerminated )
    {
                if ( buf && (len < n) )
            *buf = 0;

                len++;
    }

    return len;
}

static inline bool isoctal(wchar_t wch)
{
    return L'0' <= wch && wch <= L'7';
}

size_t wxMBConvUTF8::FromWChar(char *buf, size_t n,
                               const wchar_t *psz, size_t srcLen) const
{
    if ( m_options == MAP_INVALID_UTF8_NOT )
        return wxMBConvStrictUTF8::FromWChar(buf, n, psz, srcLen);

    size_t len = 0;

            const bool isNulTerminated = srcLen == wxNO_LEN;
    while ((isNulTerminated ? *psz : srcLen--) && ((!buf) || (len < n)))
    {
        wxUint32 cc;

#ifdef WC_UTF16
                size_t pa = decode_utf16((const wxUint16 *)psz, cc);

                        psz += (pa == wxCONV_FAILED) ? 1 : pa;
        if ( pa == 2 && !isNulTerminated )
            srcLen--;
#else
        cc = (*psz++) & 0x7fffffff;
#endif

        if ( (m_options & MAP_INVALID_UTF8_TO_PUA)
                && cc >= wxUnicodePUA && cc < wxUnicodePUAEnd )
        {
            if (buf)
                *buf++ = (char)(cc - wxUnicodePUA);
            len++;
        }
        else if ( (m_options & MAP_INVALID_UTF8_TO_OCTAL)
                    && cc == L'\\' && psz[0] == L'\\' )
        {
            if (buf)
                *buf++ = (char)cc;
            psz++;
            len++;
        }
        else if ( (m_options & MAP_INVALID_UTF8_TO_OCTAL) &&
                    cc == L'\\' &&
                        isoctal(psz[0]) && isoctal(psz[1]) && isoctal(psz[2]) )
        {
            if (buf)
            {
                *buf++ = (char) ((psz[0] - L'0') * 0100 +
                                 (psz[1] - L'0') * 010 +
                                 (psz[2] - L'0'));
            }

            psz += 3;
            len++;
        }
        else
        {
            unsigned cnt;
            for (cnt = 0; cc > utf8_max[cnt]; cnt++)
            {
            }

            if (!cnt)
            {
                                if (buf)
                    *buf++ = (char) cc;
                len++;
            }
            else
            {
                len += cnt + 1;
                if (buf)
                {
                    *buf++ = (char) ((-128 >> cnt) | ((cc >> (cnt * 6)) & (0x3f >> cnt)));
                    while (cnt--)
                        *buf++ = (char) (0x80 | ((cc >> (cnt * 6)) & 0x3f));
                }
            }
        }
    }

    if ( isNulTerminated )
    {
                if ( buf && (len < n) )
            *buf = 0;

                len++;
    }

    return len;
}


#ifdef WORDS_BIGENDIAN
    #define wxMBConvUTF16straight wxMBConvUTF16BE
    #define wxMBConvUTF16swap     wxMBConvUTF16LE
#else
    #define wxMBConvUTF16swap     wxMBConvUTF16BE
    #define wxMBConvUTF16straight wxMBConvUTF16LE
#endif


size_t wxMBConvUTF16Base::GetLength(const char *src, size_t srcLen)
{
    if ( srcLen == wxNO_LEN )
    {
                const wxUint16 *inBuff = reinterpret_cast<const wxUint16 *>(src);
        for ( srcLen = 1; *inBuff++; srcLen++ )
            ;

        srcLen *= BYTES_PER_CHAR;
    }
    else     {
                if ( srcLen % BYTES_PER_CHAR )
            return wxCONV_FAILED;
    }

    return srcLen;
}

#ifdef WC_UTF16


size_t
wxMBConvUTF16straight::ToWChar(wchar_t *dst, size_t dstLen,
                               const char *src, size_t srcLen) const
{
            srcLen = GetLength(src, srcLen);
    if ( srcLen == wxNO_LEN )
        return wxCONV_FAILED;

    const size_t inLen = srcLen / BYTES_PER_CHAR;
    if ( dst )
    {
        if ( dstLen < inLen )
            return wxCONV_FAILED;

        memcpy(dst, src, srcLen);
    }

    return inLen;
}

size_t
wxMBConvUTF16straight::FromWChar(char *dst, size_t dstLen,
                                 const wchar_t *src, size_t srcLen) const
{
    if ( srcLen == wxNO_LEN )
        srcLen = wxWcslen(src) + 1;

    srcLen *= BYTES_PER_CHAR;

    if ( dst )
    {
        if ( dstLen < srcLen )
            return wxCONV_FAILED;

        memcpy(dst, src, srcLen);
    }

    return srcLen;
}


size_t
wxMBConvUTF16swap::ToWChar(wchar_t *dst, size_t dstLen,
                           const char *src, size_t srcLen) const
{
    srcLen = GetLength(src, srcLen);
    if ( srcLen == wxNO_LEN )
        return wxCONV_FAILED;

    srcLen /= BYTES_PER_CHAR;

    if ( dst )
    {
        if ( dstLen < srcLen )
            return wxCONV_FAILED;

        const wxUint16 *inBuff = reinterpret_cast<const wxUint16 *>(src);
        for ( size_t n = 0; n < srcLen; n++, inBuff++ )
        {
            *dst++ = wxUINT16_SWAP_ALWAYS(*inBuff);
        }
    }

    return srcLen;
}

size_t
wxMBConvUTF16swap::FromWChar(char *dst, size_t dstLen,
                             const wchar_t *src, size_t srcLen) const
{
    if ( srcLen == wxNO_LEN )
        srcLen = wxWcslen(src) + 1;

    srcLen *= BYTES_PER_CHAR;

    if ( dst )
    {
        if ( dstLen < srcLen )
            return wxCONV_FAILED;

        wxUint16 *outBuff = reinterpret_cast<wxUint16 *>(dst);
        for ( size_t n = 0; n < srcLen; n += BYTES_PER_CHAR, src++ )
        {
            *outBuff++ = wxUINT16_SWAP_ALWAYS(*src);
        }
    }

    return srcLen;
}

#else 

size_t
wxMBConvUTF16straight::ToWChar(wchar_t *dst, size_t dstLen,
                               const char *src, size_t srcLen) const
{
    srcLen = GetLength(src, srcLen);
    if ( srcLen == wxNO_LEN )
        return wxCONV_FAILED;

    const size_t inLen = srcLen / BYTES_PER_CHAR;
    size_t outLen = 0;
    const wxUint16 *inBuff = reinterpret_cast<const wxUint16 *>(src);
    for ( const wxUint16 * const inEnd = inBuff + inLen; inBuff < inEnd; )
    {
        const wxUint32 ch = wxDecodeSurrogate(&inBuff);
        if ( !inBuff )
            return wxCONV_FAILED;

        outLen++;

        if ( dst )
        {
            if ( outLen > dstLen )
                return wxCONV_FAILED;

            *dst++ = ch;
        }
    }


    return outLen;
}

size_t
wxMBConvUTF16straight::FromWChar(char *dst, size_t dstLen,
                                 const wchar_t *src, size_t srcLen) const
{
    if ( srcLen == wxNO_LEN )
        srcLen = wxWcslen(src) + 1;

    size_t outLen = 0;
    wxUint16 *outBuff = reinterpret_cast<wxUint16 *>(dst);
    for ( size_t n = 0; n < srcLen; n++ )
    {
        wxUint16 cc[2] = { 0 };
        const size_t numChars = encode_utf16(*src++, cc);
        if ( numChars == wxCONV_FAILED )
            return wxCONV_FAILED;

        outLen += numChars * BYTES_PER_CHAR;
        if ( outBuff )
        {
            if ( outLen > dstLen )
                return wxCONV_FAILED;

            *outBuff++ = cc[0];
            if ( numChars == 2 )
            {
                                *outBuff++ = cc[1];
            }
        }
    }

    return outLen;
}


size_t
wxMBConvUTF16swap::ToWChar(wchar_t *dst, size_t dstLen,
                           const char *src, size_t srcLen) const
{
    srcLen = GetLength(src, srcLen);
    if ( srcLen == wxNO_LEN )
        return wxCONV_FAILED;

    const size_t inLen = srcLen / BYTES_PER_CHAR;
    size_t outLen = 0;
    const wxUint16 *inBuff = reinterpret_cast<const wxUint16 *>(src);
    for ( const wxUint16 * const inEnd = inBuff + inLen; inBuff < inEnd; )
    {
        wxUint32 ch;
        wxUint16 tmp[2];

        tmp[0] = wxUINT16_SWAP_ALWAYS(*inBuff);
        if ( ++inBuff < inEnd )
        {
                        tmp[1] = wxUINT16_SWAP_ALWAYS(*inBuff);
        }
        else         {
                                                tmp[1] = 0;
        }

        const size_t numChars = decode_utf16(tmp, ch);
        if ( numChars == wxCONV_FAILED )
            return wxCONV_FAILED;

        if ( numChars == 2 )
            inBuff++;

        outLen++;

        if ( dst )
        {
            if ( outLen > dstLen )
                return wxCONV_FAILED;

            *dst++ = ch;
        }
    }


    return outLen;
}

size_t
wxMBConvUTF16swap::FromWChar(char *dst, size_t dstLen,
                             const wchar_t *src, size_t srcLen) const
{
    if ( srcLen == wxNO_LEN )
        srcLen = wxWcslen(src) + 1;

    size_t outLen = 0;
    wxUint16 *outBuff = reinterpret_cast<wxUint16 *>(dst);
    for ( const wchar_t *srcEnd = src + srcLen; src < srcEnd; src++ )
    {
        wxUint16 cc[2] = { 0 };
        const size_t numChars = encode_utf16(*src, cc);
        if ( numChars == wxCONV_FAILED )
            return wxCONV_FAILED;

        outLen += numChars * BYTES_PER_CHAR;
        if ( outBuff )
        {
            if ( outLen > dstLen )
                return wxCONV_FAILED;

            *outBuff++ = wxUINT16_SWAP_ALWAYS(cc[0]);
            if ( numChars == 2 )
            {
                                *outBuff++ = wxUINT16_SWAP_ALWAYS(cc[1]);
            }
        }
    }

    return outLen;
}

#endif 


#ifdef WORDS_BIGENDIAN
    #define wxMBConvUTF32straight  wxMBConvUTF32BE
    #define wxMBConvUTF32swap      wxMBConvUTF32LE
#else
    #define wxMBConvUTF32swap      wxMBConvUTF32BE
    #define wxMBConvUTF32straight  wxMBConvUTF32LE
#endif


WXDLLIMPEXP_DATA_BASE(wxMBConvUTF32LE) wxConvUTF32LE;
WXDLLIMPEXP_DATA_BASE(wxMBConvUTF32BE) wxConvUTF32BE;


size_t wxMBConvUTF32Base::GetLength(const char *src, size_t srcLen)
{
    if ( srcLen == wxNO_LEN )
    {
                const wxUint32 *inBuff = reinterpret_cast<const wxUint32 *>(src);
        for ( srcLen = 1; *inBuff++; srcLen++ )
            ;

        srcLen *= BYTES_PER_CHAR;
    }
    else     {
                if ( srcLen % BYTES_PER_CHAR )
            return wxCONV_FAILED;
    }

    return srcLen;
}

#ifdef WC_UTF16


size_t
wxMBConvUTF32straight::ToWChar(wchar_t *dst, size_t dstLen,
                               const char *src, size_t srcLen) const
{
    srcLen = GetLength(src, srcLen);
    if ( srcLen == wxNO_LEN )
        return wxCONV_FAILED;

    const wxUint32 *inBuff = reinterpret_cast<const wxUint32 *>(src);
    const size_t inLen = srcLen / BYTES_PER_CHAR;
    size_t outLen = 0;
    for ( size_t n = 0; n < inLen; n++ )
    {
        wxUint16 cc[2] = { 0 };
        const size_t numChars = encode_utf16(*inBuff++, cc);
        if ( numChars == wxCONV_FAILED )
            return wxCONV_FAILED;

        outLen += numChars;
        if ( dst )
        {
            if ( outLen > dstLen )
                return wxCONV_FAILED;

            *dst++ = cc[0];
            if ( numChars == 2 )
            {
                                *dst++ = cc[1];
            }
        }
    }

    return outLen;
}

size_t
wxMBConvUTF32straight::FromWChar(char *dst, size_t dstLen,
                                 const wchar_t *src, size_t srcLen) const
{
    if ( srcLen == wxNO_LEN )
        srcLen = wxWcslen(src) + 1;

    wxUint32 *outBuff = reinterpret_cast<wxUint32 *>(dst);
    size_t outLen = 0;
    for ( const wchar_t * const srcEnd = src + srcLen; src < srcEnd; )
    {
        const wxUint32 ch = wxDecodeSurrogate(&src);
        if ( !src )
            return wxCONV_FAILED;

        outLen += BYTES_PER_CHAR;

        if ( outBuff )
        {
            if ( outLen > dstLen )
                return wxCONV_FAILED;

            *outBuff++ = ch;
        }
    }

    return outLen;
}


size_t
wxMBConvUTF32swap::ToWChar(wchar_t *dst, size_t dstLen,
                           const char *src, size_t srcLen) const
{
    srcLen = GetLength(src, srcLen);
    if ( srcLen == wxNO_LEN )
        return wxCONV_FAILED;

    const wxUint32 *inBuff = reinterpret_cast<const wxUint32 *>(src);
    const size_t inLen = srcLen / BYTES_PER_CHAR;
    size_t outLen = 0;
    for ( size_t n = 0; n < inLen; n++, inBuff++ )
    {
        wxUint16 cc[2] = { 0 };
        const size_t numChars = encode_utf16(wxUINT32_SWAP_ALWAYS(*inBuff), cc);
        if ( numChars == wxCONV_FAILED )
            return wxCONV_FAILED;

        outLen += numChars;
        if ( dst )
        {
            if ( outLen > dstLen )
                return wxCONV_FAILED;

            *dst++ = cc[0];
            if ( numChars == 2 )
            {
                                *dst++ = cc[1];
            }
        }
    }

    return outLen;
}

size_t
wxMBConvUTF32swap::FromWChar(char *dst, size_t dstLen,
                             const wchar_t *src, size_t srcLen) const
{
    if ( srcLen == wxNO_LEN )
        srcLen = wxWcslen(src) + 1;

    wxUint32 *outBuff = reinterpret_cast<wxUint32 *>(dst);
    size_t outLen = 0;
    for ( const wchar_t * const srcEnd = src + srcLen; src < srcEnd; )
    {
        const wxUint32 ch = wxDecodeSurrogate(&src);
        if ( !src )
            return wxCONV_FAILED;

        outLen += BYTES_PER_CHAR;

        if ( outBuff )
        {
            if ( outLen > dstLen )
                return wxCONV_FAILED;

            *outBuff++ = wxUINT32_SWAP_ALWAYS(ch);
        }
    }

    return outLen;
}

#else 

size_t
wxMBConvUTF32straight::ToWChar(wchar_t *dst, size_t dstLen,
                               const char *src, size_t srcLen) const
{
        srcLen = GetLength(src, srcLen);
    if ( srcLen == wxNO_LEN )
        return wxCONV_FAILED;

    const size_t inLen = srcLen/BYTES_PER_CHAR;
    if ( dst )
    {
        if ( dstLen < inLen )
            return wxCONV_FAILED;

        memcpy(dst, src, srcLen);
    }

    return inLen;
}

size_t
wxMBConvUTF32straight::FromWChar(char *dst, size_t dstLen,
                                 const wchar_t *src, size_t srcLen) const
{
    if ( srcLen == wxNO_LEN )
        srcLen = wxWcslen(src) + 1;

    srcLen *= BYTES_PER_CHAR;

    if ( dst )
    {
        if ( dstLen < srcLen )
            return wxCONV_FAILED;

        memcpy(dst, src, srcLen);
    }

    return srcLen;
}


size_t
wxMBConvUTF32swap::ToWChar(wchar_t *dst, size_t dstLen,
                           const char *src, size_t srcLen) const
{
    srcLen = GetLength(src, srcLen);
    if ( srcLen == wxNO_LEN )
        return wxCONV_FAILED;

    srcLen /= BYTES_PER_CHAR;

    if ( dst )
    {
        if ( dstLen < srcLen )
            return wxCONV_FAILED;

        const wxUint32 *inBuff = reinterpret_cast<const wxUint32 *>(src);
        for ( size_t n = 0; n < srcLen; n++, inBuff++ )
        {
            *dst++ = wxUINT32_SWAP_ALWAYS(*inBuff);
        }
    }

    return srcLen;
}

size_t
wxMBConvUTF32swap::FromWChar(char *dst, size_t dstLen,
                             const wchar_t *src, size_t srcLen) const
{
    if ( srcLen == wxNO_LEN )
        srcLen = wxWcslen(src) + 1;

    srcLen *= BYTES_PER_CHAR;

    if ( dst )
    {
        if ( dstLen < srcLen )
            return wxCONV_FAILED;

        wxUint32 *outBuff = reinterpret_cast<wxUint32 *>(dst);
        for ( size_t n = 0; n < srcLen; n += BYTES_PER_CHAR, src++ )
        {
            *outBuff++ = wxUINT32_SWAP_ALWAYS(*src);
        }
    }

    return srcLen;
}

#endif 


#ifdef HAVE_ICONV

#if defined(__GLIBC__) && __GLIBC__ == 2 && __GLIBC_MINOR__ <= 1
#define ICONV_FAILED(cres, bufLeft) ((cres == (size_t)-1) && \
                                     (errno != E2BIG || bufLeft != 0))
#else
#define ICONV_FAILED(cres, bufLeft)  (cres == (size_t)-1)
#endif

#define ICONV_CHAR_CAST(x)  ((ICONV_CONST char **)(x))

#define ICONV_T_INVALID ((iconv_t)-1)

#if SIZEOF_WCHAR_T == 4
    #define WC_BSWAP    wxUINT32_SWAP_ALWAYS
    #define WC_ENC      wxFONTENCODING_UTF32
#elif SIZEOF_WCHAR_T == 2
    #define WC_BSWAP    wxUINT16_SWAP_ALWAYS
    #define WC_ENC      wxFONTENCODING_UTF16
#else         #error "Unknown sizeof(wchar_t): please report this to wx-dev@lists.wxwindows.org"
#endif


class wxMBConv_iconv : public wxMBConv
{
public:
    wxMBConv_iconv(const char *name);
    virtual ~wxMBConv_iconv();

        virtual size_t ToWChar(wchar_t *dst, size_t dstLen,
                           const char *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;
    virtual size_t FromWChar(char *dst, size_t dstLen,
                             const wchar_t *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;
    virtual size_t GetMBNulLen() const wxOVERRIDE;

#if wxUSE_UNICODE_UTF8
    virtual bool IsUTF8() const wxOVERRIDE;
#endif

    virtual wxMBConv *Clone() const wxOVERRIDE
    {
        wxMBConv_iconv *p = new wxMBConv_iconv(m_name);
        p->m_minMBCharWidth = m_minMBCharWidth;
        return p;
    }

    bool IsOk() const
        { return (m2w != ICONV_T_INVALID) && (w2m != ICONV_T_INVALID); }

protected:
            iconv_t m2w,
            w2m;

#if wxUSE_THREADS
        wxMutex m_iconvMutex;
#endif

private:
            static wxString ms_wcCharsetName;

            static bool ms_wcNeedsSwap;


        const char *m_name;

            size_t m_minMBCharWidth;
};

WXDLLIMPEXP_BASE wxMBConv* new_wxMBConv_iconv( const char* name )
{
    wxMBConv_iconv* result = new wxMBConv_iconv( name );
    if ( !result->IsOk() )
    {
        delete result;
        return 0;
    }

    return result;
}

wxString wxMBConv_iconv::ms_wcCharsetName;
bool wxMBConv_iconv::ms_wcNeedsSwap = false;

wxMBConv_iconv::wxMBConv_iconv(const char *name)
              : m_name(wxStrdup(name))
{
    m_minMBCharWidth = 0;

        if ( ms_wcCharsetName.empty() )
    {
        wxLogTrace(TRACE_STRCONV, wxT("Looking for wide char codeset:"));

#if wxUSE_FONTMAP
        const wxChar *const *names = wxFontMapperBase::GetAllEncodingNames(WC_ENC);
#else         static const wxChar *const names_static[] =
        {
#if SIZEOF_WCHAR_T == 4
            wxT("UCS-4"),
#elif SIZEOF_WCHAR_T == 2
            wxT("UCS-2"),
#endif
            NULL
        };
        const wxChar *const *names = names_static;
#endif 
        for ( ; *names && ms_wcCharsetName.empty(); ++names )
        {
            const wxString nameCS(*names);

                        wxString nameXE(nameCS);

#ifdef WORDS_BIGENDIAN
                nameXE += wxT("BE");
#else                 nameXE += wxT("LE");
#endif

            wxLogTrace(TRACE_STRCONV, wxT("  trying charset \"%s\""),
                       nameXE.c_str());

            m2w = iconv_open(nameXE.ToAscii(), name);
            if ( m2w == ICONV_T_INVALID )
            {
                                wxLogTrace(TRACE_STRCONV, wxT("  trying charset \"%s\""),
                           nameCS.c_str());
                m2w = iconv_open(nameCS.ToAscii(), name);

                                if ( m2w != ICONV_T_INVALID )
                {
                    char    buf[2], *bufPtr;
                    wchar_t wbuf[2];
                    size_t  insz, outsz;
                    size_t  res;

                    buf[0] = 'A';
                    buf[1] = 0;
                    wbuf[0] = 0;
                    insz = 2;
                    outsz = SIZEOF_WCHAR_T * 2;
                    char* wbufPtr = (char*)wbuf;
                    bufPtr = buf;

                    res = iconv(
                        m2w, ICONV_CHAR_CAST(&bufPtr), &insz,
                        &wbufPtr, &outsz);

                    if (ICONV_FAILED(res, insz))
                    {
                        wxLogLastError(wxT("iconv"));
                        wxLogError(_("Conversion to charset '%s' doesn't work."),
                                   nameCS.c_str());
                    }
                    else                     {
                        ms_wcCharsetName = nameCS;
                        ms_wcNeedsSwap = wbuf[0] != (wchar_t)buf[0];
                    }
                }
            }
            else             {
                ms_wcCharsetName = nameXE;
            }
        }

        wxLogTrace(TRACE_STRCONV,
                   wxT("iconv wchar_t charset is \"%s\"%s"),
                   ms_wcCharsetName.empty() ? wxString("<none>")
                                            : ms_wcCharsetName,
                   ms_wcNeedsSwap ? wxT(" (needs swap)")
                                  : wxT(""));
    }
    else     {
        m2w = iconv_open(ms_wcCharsetName.ToAscii(), name);
    }

    if ( ms_wcCharsetName.empty() )
    {
        w2m = ICONV_T_INVALID;
    }
    else
    {
        w2m = iconv_open(name, ms_wcCharsetName.ToAscii());
        if ( w2m == ICONV_T_INVALID )
        {
            wxLogTrace(TRACE_STRCONV,
                       wxT("\"%s\" -> \"%s\" works but not the converse!?"),
                       ms_wcCharsetName.c_str(), name);
        }
    }
}

wxMBConv_iconv::~wxMBConv_iconv()
{
    free(const_cast<char *>(m_name));

    if ( m2w != ICONV_T_INVALID )
        iconv_close(m2w);
    if ( w2m != ICONV_T_INVALID )
        iconv_close(w2m);
}

size_t
wxMBConv_iconv::ToWChar(wchar_t *dst, size_t dstLen,
                        const char *src, size_t srcLen) const
{
    if ( srcLen == wxNO_LEN )
    {
                                const size_t nulLen = GetMBNulLen();
        switch ( nulLen )
        {
            default:
                return wxCONV_FAILED;

            case 1:
                srcLen = strlen(src);                 break;

            case 2:
            case 4:
                                                                const char *p;
                for ( p = src; NotAllNULs(p, nulLen); p += nulLen )
                    ;
                srcLen = p - src;
                break;
        }

                                srcLen += nulLen;
    }

            dstLen *= SIZEOF_WCHAR_T;

#if wxUSE_THREADS
                            wxMutexLocker lock(wxConstCast(this, wxMBConv_iconv)->m_iconvMutex);
#endif 
    size_t res, cres;
    const char *pszPtr = src;

    if ( dst )
    {
        char* bufPtr = (char*)dst;

                size_t dstLenOrig = dstLen;
        cres = iconv(m2w,
                     ICONV_CHAR_CAST(&pszPtr), &srcLen,
                     &bufPtr, &dstLen);

                        res = (dstLenOrig - dstLen) / SIZEOF_WCHAR_T;

        if (ms_wcNeedsSwap)
        {
                        for ( unsigned i = 0; i < res; i++ )
                dst[i] = WC_BSWAP(dst[i]);
        }
    }
    else     {
                wchar_t tbuf[256];
        res = 0;

        do
        {
            char* bufPtr = (char*)tbuf;
            dstLen = 8 * SIZEOF_WCHAR_T;

            cres = iconv(m2w,
                         ICONV_CHAR_CAST(&pszPtr), &srcLen,
                         &bufPtr, &dstLen );

            res += 8 - (dstLen / SIZEOF_WCHAR_T);
        }
        while ((cres == (size_t)-1) && (errno == E2BIG));
    }

    if (ICONV_FAILED(cres, srcLen))
    {
                wxLogTrace(TRACE_STRCONV, wxT("iconv failed: %s"), wxSysErrorMsg(wxSysErrorCode()));
        return wxCONV_FAILED;
    }

    return res;
}

size_t wxMBConv_iconv::FromWChar(char *dst, size_t dstLen,
                                 const wchar_t *src, size_t srcLen) const
{
#if wxUSE_THREADS
        wxMutexLocker lock(wxConstCast(this, wxMBConv_iconv)->m_iconvMutex);
#endif

    if ( srcLen == wxNO_LEN )
        srcLen = wxWcslen(src) + 1;

    size_t inbuflen = srcLen * SIZEOF_WCHAR_T;
    size_t outbuflen = dstLen;
    size_t res, cres;

    wchar_t *tmpbuf = 0;

    if (ms_wcNeedsSwap)
    {
                                tmpbuf = (wchar_t *)malloc(inbuflen);
        for ( size_t i = 0; i < srcLen; i++ )
            tmpbuf[i] = WC_BSWAP(src[i]);

        src = tmpbuf;
    }

    char* inbuf = (char*)src;
    if ( dst )
    {
                cres = iconv(w2m, ICONV_CHAR_CAST(&inbuf), &inbuflen, &dst, &outbuflen);

        res = dstLen - outbuflen;
    }
    else     {
                char tbuf[256];
        res = 0;
        do
        {
            dst = tbuf;
            outbuflen = WXSIZEOF(tbuf);

            cres = iconv(w2m, ICONV_CHAR_CAST(&inbuf), &inbuflen, &dst, &outbuflen);

            res += WXSIZEOF(tbuf) - outbuflen;
        }
        while ((cres == (size_t)-1) && (errno == E2BIG));
    }

    if (ms_wcNeedsSwap)
    {
        free(tmpbuf);
    }

    if (ICONV_FAILED(cres, inbuflen))
    {
        wxLogTrace(TRACE_STRCONV, wxT("iconv failed: %s"), wxSysErrorMsg(wxSysErrorCode()));
        return wxCONV_FAILED;
    }

    return res;
}

size_t wxMBConv_iconv::GetMBNulLen() const
{
    if ( m_minMBCharWidth == 0 )
    {
        wxMBConv_iconv * const self = wxConstCast(this, wxMBConv_iconv);

#if wxUSE_THREADS
                wxMutexLocker lock(self->m_iconvMutex);
#endif

        const wchar_t *wnul = L"";
        char buf[8];         size_t inLen = sizeof(wchar_t),
               outLen = WXSIZEOF(buf);
        char *inBuff = (char *)wnul;
        char *outBuff = buf;
        if ( iconv(w2m, ICONV_CHAR_CAST(&inBuff), &inLen, &outBuff, &outLen) == (size_t)-1 )
        {
            self->m_minMBCharWidth = (size_t)-1;
        }
        else         {
            self->m_minMBCharWidth = outBuff - buf;
        }
    }

    return m_minMBCharWidth;
}

#if wxUSE_UNICODE_UTF8
bool wxMBConv_iconv::IsUTF8() const
{
    return wxStricmp(m_name, "UTF-8") == 0 ||
           wxStricmp(m_name, "UTF8") == 0;
}
#endif

#endif 


#ifdef wxHAVE_WIN32_MB2WC

#if wxUSE_FONTMAP
extern WXDLLIMPEXP_BASE long wxCharsetToCodepage(const char *charset);
extern WXDLLIMPEXP_BASE long wxEncodingToCodepage(wxFontEncoding encoding);
#endif

class wxMBConv_win32 : public wxMBConv
{
public:
    wxMBConv_win32()
    {
        m_CodePage = CP_ACP;
        m_minMBCharWidth = 0;
    }

    wxMBConv_win32(const wxMBConv_win32& conv)
        : wxMBConv()
    {
        m_CodePage = conv.m_CodePage;
        m_minMBCharWidth = conv.m_minMBCharWidth;
    }

#if wxUSE_FONTMAP
    wxMBConv_win32(const char* name)
    {
        m_CodePage = wxCharsetToCodepage(name);
        m_minMBCharWidth = 0;
    }

    wxMBConv_win32(wxFontEncoding encoding)
    {
        m_CodePage = wxEncodingToCodepage(encoding);
        m_minMBCharWidth = 0;
    }
#endif 
    virtual size_t MB2WC(wchar_t *buf, const char *psz, size_t n) const
    {
                                                                                if ( m_CodePage == CP_UTF8 )
        {
            return wxMBConvUTF8().MB2WC(buf, psz, n);
        }

        if ( m_CodePage == CP_UTF7 )
        {
            return wxMBConvUTF7().MB2WC(buf, psz, n);
        }

        const size_t len = ::MultiByteToWideChar
                             (
                                m_CodePage,                                     MB_ERR_INVALID_CHARS,                                  psz,                                            -1,                                             buf,                                            buf ? n : 0                                  );
        if ( !len )
            return wxCONV_FAILED;

                                return len - 1;
    }

    virtual size_t WC2MB(char *buf, const wchar_t *pwz, size_t n) const
    {
        
        BOOL usedDef wxDUMMY_INITIALIZE(false);
        BOOL *pUsedDef;
        int flags;
        if ( m_CodePage < 50000 )
        {
                        flags = WC_NO_BEST_FIT_CHARS;
            pUsedDef = &usedDef;
        }
        else         {
            flags = 0;
            pUsedDef = NULL;
        }

        const size_t len = ::WideCharToMultiByte
                             (
                                m_CodePage,                                     flags,                                          pwz,                                            -1,                                             buf,                                            buf ? n : 0,                                    NULL,                                           pUsedDef                                     );

        if ( !len )
        {
                        return wxCONV_FAILED;
        }

                if ( flags )
        {
                                    if ( usedDef )
                return wxCONV_FAILED;
        }
        else         {
                                                wxCharBuffer bufDef;
            if ( !buf )
            {
                bufDef = wxCharBuffer(len);
                buf = bufDef.data();
                if ( !::WideCharToMultiByte(m_CodePage, flags, pwz, -1,
                                            buf, len, NULL, NULL) )
                    return wxCONV_FAILED;
            }

            if ( !n )
                n = wcslen(pwz);
            wxWCharBuffer wcBuf(n);
            if ( MB2WC(wcBuf.data(), buf, n + 1) == wxCONV_FAILED ||
                    wcscmp(wcBuf, pwz) != 0 )
            {
                                                return wxCONV_FAILED;
            }
        }

                return len - 1;
    }

    virtual size_t GetMBNulLen() const
    {
        if ( m_minMBCharWidth == 0 )
        {
            int len = ::WideCharToMultiByte
                        (
                            m_CodePage,                                 0,                                          L"",                                        1,                                          NULL,                                       0,                                          NULL,                                       NULL                                    );

            wxMBConv_win32 * const self = wxConstCast(this, wxMBConv_win32);
            switch ( len )
            {
                default:
                    wxLogDebug(wxT("Unexpected NUL length %d"), len);
                    self->m_minMBCharWidth = (size_t)-1;
                    break;

                case 0:
                    self->m_minMBCharWidth = (size_t)-1;
                    break;

                case 1:
                case 2:
                case 4:
                    self->m_minMBCharWidth = len;
                    break;
            }
        }

        return m_minMBCharWidth;
    }

    virtual wxMBConv *Clone() const { return new wxMBConv_win32(*this); }

    bool IsOk() const { return m_CodePage != -1; }

private:
        long m_CodePage;

            size_t m_minMBCharWidth;
};

#endif 


#if wxUSE_FONTMAP

class wxMBConv_wxwin : public wxMBConv
{
private:
    void Init()
    {
                        m_ok = (m_enc < wxFONTENCODING_MACMIN || m_enc > wxFONTENCODING_MACMAX) &&
               m2w.Init(m_enc, wxFONTENCODING_UNICODE) &&
               w2m.Init(wxFONTENCODING_UNICODE, m_enc);
    }

public:
            wxMBConv_wxwin(const char* name)
    {
        if (name)
            m_enc = wxFontMapperBase::Get()->CharsetToEncoding(name, false);
        else
            m_enc = wxFONTENCODING_SYSTEM;

        Init();
    }

    wxMBConv_wxwin(wxFontEncoding enc)
    {
        m_enc = enc;

        Init();
    }

    size_t MB2WC(wchar_t *buf, const char *psz, size_t WXUNUSED(n)) const wxOVERRIDE
    {
        size_t inbuf = strlen(psz);
        if (buf)
        {
            if (!m2w.Convert(psz, buf))
                return wxCONV_FAILED;
        }
        return inbuf;
    }

    size_t WC2MB(char *buf, const wchar_t *psz, size_t WXUNUSED(n)) const wxOVERRIDE
    {
        const size_t inbuf = wxWcslen(psz);
        if (buf)
        {
            if (!w2m.Convert(psz, buf))
                return wxCONV_FAILED;
        }

        return inbuf;
    }

    virtual size_t GetMBNulLen() const wxOVERRIDE
    {
        switch ( m_enc )
        {
            case wxFONTENCODING_UTF16BE:
            case wxFONTENCODING_UTF16LE:
                return 2;

            case wxFONTENCODING_UTF32BE:
            case wxFONTENCODING_UTF32LE:
                return 4;

            default:
                return 1;
        }
    }

    virtual wxMBConv *Clone() const wxOVERRIDE { return new wxMBConv_wxwin(m_enc); }

    bool IsOk() const { return m_ok; }

public:
    wxFontEncoding m_enc;
    wxEncodingConverter m2w, w2m;

private:
        bool m_ok;

    wxDECLARE_NO_COPY_CLASS(wxMBConv_wxwin);
};

WXDLLIMPEXP_BASE wxMBConv* new_wxMBConv_wxwin( const char* name )
{
    wxMBConv_wxwin* result = new wxMBConv_wxwin( name );
    if ( !result->IsOk() )
    {
        delete result;
        return 0;
    }

    return result;
}

#endif 

void wxCSConv::Init()
{
    m_name = NULL;
    m_convReal =  NULL;
}

void wxCSConv::SetEncoding(wxFontEncoding encoding)
{
    switch ( encoding )
    {
        case wxFONTENCODING_MAX:
        case wxFONTENCODING_SYSTEM:
            if ( m_name )
            {
                                m_encoding = wxFONTENCODING_SYSTEM;
            }
            else             {
                                                #if wxUSE_INTL
                m_encoding = wxLocale::GetSystemEncoding();
                if ( m_encoding == wxFONTENCODING_SYSTEM )
#endif                     m_encoding = wxFONTENCODING_ISO8859_1;
            }
            break;

        case wxFONTENCODING_DEFAULT:
                        m_encoding = wxFONTENCODING_ISO8859_1;
            break;

        default:
                        m_encoding = encoding;
    }
}

wxCSConv::wxCSConv(const wxString& charset)
{
    Init();

    if ( !charset.empty() )
    {
        SetName(charset.ToAscii());
    }

#if wxUSE_FONTMAP
    SetEncoding(wxFontMapperBase::GetEncodingFromName(charset));
#else
    SetEncoding(wxFONTENCODING_SYSTEM);
#endif

    m_convReal = DoCreate();
}

wxCSConv::wxCSConv(wxFontEncoding encoding)
{
    if ( encoding == wxFONTENCODING_MAX || encoding == wxFONTENCODING_DEFAULT )
    {
        wxFAIL_MSG( wxT("invalid encoding value in wxCSConv ctor") );

        encoding = wxFONTENCODING_SYSTEM;
    }

    Init();

    SetEncoding(encoding);

    m_convReal = DoCreate();
}

wxCSConv::~wxCSConv()
{
    Clear();
}

wxCSConv::wxCSConv(const wxCSConv& conv)
        : wxMBConv()
{
    Init();

    SetName(conv.m_name);
    SetEncoding(conv.m_encoding);

    m_convReal = DoCreate();
}

wxCSConv& wxCSConv::operator=(const wxCSConv& conv)
{
    Clear();

    SetName(conv.m_name);
    SetEncoding(conv.m_encoding);

    m_convReal = DoCreate();

    return *this;
}

void wxCSConv::Clear()
{
    free(m_name);
    m_name = NULL;

    wxDELETE(m_convReal);
}

void wxCSConv::SetName(const char *charset)
{
    if ( charset )
        m_name = wxStrdup(charset);
}

#if wxUSE_FONTMAP

WX_DECLARE_HASH_MAP( wxFontEncoding, wxString, wxIntegerHash, wxIntegerEqual,
                     wxEncodingNameCache );

static wxEncodingNameCache gs_nameCache;
#endif

wxMBConv *wxCSConv::DoCreate() const
{
#if wxUSE_FONTMAP
    wxLogTrace(TRACE_STRCONV,
               wxT("creating conversion for %s"),
               (m_name ? m_name
                       : (const char*)wxFontMapperBase::GetEncodingName(m_encoding).mb_str()));
#endif 
                if ( m_encoding == wxFONTENCODING_ISO8859_1 )
    {
                return NULL;
    }

                            
    #ifdef HAVE_ICONV
#if !wxUSE_FONTMAP
    if ( m_name )
#endif     {
#if wxUSE_FONTMAP
        wxFontEncoding encoding(m_encoding);
#endif

        if ( m_name )
        {
            wxMBConv_iconv *conv = new wxMBConv_iconv(m_name);
            if ( conv->IsOk() )
                return conv;

            delete conv;

#if wxUSE_FONTMAP
            encoding =
                wxFontMapperBase::Get()->CharsetToEncoding(m_name, false);
#endif         }
#if wxUSE_FONTMAP
        {
            const wxEncodingNameCache::iterator it = gs_nameCache.find(encoding);
            if ( it != gs_nameCache.end() )
            {
                if ( it->second.empty() )
                    return NULL;

                wxMBConv_iconv *conv = new wxMBConv_iconv(it->second.ToAscii());
                if ( conv->IsOk() )
                    return conv;

                delete conv;
            }

            const wxChar* const* names = wxFontMapperBase::GetAllEncodingNames(encoding);
                                                                        if ( names[0] != NULL )
            {
                for ( ; *names; ++names )
                {
                                                            wxString name(*names);
                    wxMBConv_iconv *conv = new wxMBConv_iconv(name.ToAscii());
                    if ( conv->IsOk() )
                    {
                        gs_nameCache[encoding] = *names;
                        return conv;
                    }

                    delete conv;
                }

                gs_nameCache[encoding] = wxT("");             }
        }
#endif     }
#endif 
#ifdef wxHAVE_WIN32_MB2WC
    {
#if wxUSE_FONTMAP
        wxMBConv_win32 *conv = m_name ? new wxMBConv_win32(m_name)
                                      : new wxMBConv_win32(m_encoding);
        if ( conv->IsOk() )
            return conv;

        delete conv;
#else
        return NULL;
#endif
    }
#endif 
#ifdef __DARWIN__
    {
                if ( m_name || ( m_encoding < wxFONTENCODING_UTF16BE ||
            ( m_encoding >= wxFONTENCODING_MACMIN && m_encoding <= wxFONTENCODING_MACMAX ) ) )
        {
#if wxUSE_FONTMAP
            wxMBConv_cf *conv = m_name ? new wxMBConv_cf(m_name)
                                          : new wxMBConv_cf(m_encoding);
#else
            wxMBConv_cf *conv = new wxMBConv_cf(m_encoding);
#endif

            if ( conv->IsOk() )
                 return conv;

            delete conv;
        }
    }
#endif 
        wxFontEncoding enc = m_encoding;
#if wxUSE_FONTMAP
    if ( enc == wxFONTENCODING_SYSTEM && m_name )
    {
                                enc = wxFontMapperBase::Get()->CharsetToEncoding(m_name, false);
    }
#endif 
    switch ( enc )
    {
        case wxFONTENCODING_UTF7:
             return new wxMBConvUTF7;

        case wxFONTENCODING_UTF8:
             return new wxMBConvUTF8;

        case wxFONTENCODING_UTF16BE:
             return new wxMBConvUTF16BE;

        case wxFONTENCODING_UTF16LE:
             return new wxMBConvUTF16LE;

        case wxFONTENCODING_UTF32BE:
             return new wxMBConvUTF32BE;

        case wxFONTENCODING_UTF32LE:
             return new wxMBConvUTF32LE;

        default:
                          break;
    }

    #if wxUSE_FONTMAP
    {
        wxMBConv_wxwin *conv = m_name ? new wxMBConv_wxwin(m_name)
                                      : new wxMBConv_wxwin(m_encoding);
        if ( conv->IsOk() )
            return conv;

        delete conv;
    }

    wxLogTrace(TRACE_STRCONV,
               wxT("encoding \"%s\" is not supported by this system"),
               (m_name ? wxString(m_name)
                       : wxFontMapperBase::GetEncodingName(m_encoding)));
#endif 
    return NULL;
}

bool wxCSConv::IsOk() const
{
        if ( m_encoding == wxFONTENCODING_ISO8859_1 )
        return true; 
            return m_convReal != NULL;
}

size_t wxCSConv::ToWChar(wchar_t *dst, size_t dstLen,
                         const char *src, size_t srcLen) const
{
    if (m_convReal)
        return m_convReal->ToWChar(dst, dstLen, src, srcLen);

        if ( srcLen == wxNO_LEN )
        srcLen = strlen(src) + 1; 
    if ( dst )
    {
        if ( dstLen < srcLen )
            return wxCONV_FAILED;

        for ( size_t n = 0; n < srcLen; n++ )
            dst[n] = (unsigned char)(src[n]);
    }

    return srcLen;
}

size_t wxCSConv::FromWChar(char *dst, size_t dstLen,
                           const wchar_t *src, size_t srcLen) const
{
    if (m_convReal)
        return m_convReal->FromWChar(dst, dstLen, src, srcLen);

        if ( srcLen == wxNO_LEN )
        srcLen = wxWcslen(src) + 1;

    if ( dst )
    {
        if ( dstLen < srcLen )
            return wxCONV_FAILED;

        for ( size_t n = 0; n < srcLen; n++ )
        {
            if ( src[n] > 0xFF )
                return wxCONV_FAILED;

            dst[n] = (char)src[n];
        }

    }
    else     {
        for ( size_t n = 0; n < srcLen; n++ )
        {
            if ( src[n] > 0xFF )
                return wxCONV_FAILED;
        }
    }

    return srcLen;
}

size_t wxCSConv::GetMBNulLen() const
{
    if ( m_convReal )
        return m_convReal->GetMBNulLen();

        return 1;
}

#if wxUSE_UNICODE_UTF8
bool wxCSConv::IsUTF8() const
{
    if ( m_convReal )
        return m_convReal->IsUTF8();

        return false;
}
#endif



size_t
wxWhateverWorksConv::ToWChar(wchar_t *dst, size_t dstLen,
                             const char *src, size_t srcLen) const
{
    size_t rc = wxConvUTF8.ToWChar(dst, dstLen, src, srcLen);
    if ( rc != wxCONV_FAILED )
        return rc;

    rc = wxConvLibc.ToWChar(dst, dstLen, src, srcLen);
    if ( rc != wxCONV_FAILED )
        return rc;

    rc = wxConvISO8859_1.ToWChar(dst, dstLen, src, srcLen);

    return rc;
}

size_t
wxWhateverWorksConv::FromWChar(char *dst, size_t dstLen,
                               const wchar_t *src, size_t srcLen) const
{
    size_t rc = wxConvLibc.FromWChar(dst, dstLen, src, srcLen);
    if ( rc != wxCONV_FAILED )
        return rc;

    rc = wxConvUTF8.FromWChar(dst, dstLen, src, srcLen);

    return rc;
}



#undef wxConvLibc
#undef wxConvUTF8
#undef wxConvUTF7
#undef wxConvWhateverWorks
#undef wxConvLocal
#undef wxConvISO8859_1

#define WX_DEFINE_GLOBAL_CONV2(klass, impl_klass, name, ctor_args)      \
    WXDLLIMPEXP_DATA_BASE(klass*) name##Ptr = NULL;                     \
    WXDLLIMPEXP_BASE klass* wxGet_##name##Ptr()                         \
    {                                                                   \
        static impl_klass name##Obj ctor_args;                          \
        return &name##Obj;                                              \
    }                                                                   \
        \
        \
                                               \
    static klass* gs_##name##instance = wxGet_##name##Ptr()

#define WX_DEFINE_GLOBAL_CONV(klass, name, ctor_args) \
    WX_DEFINE_GLOBAL_CONV2(klass, klass, name, ctor_args)

#ifdef __INTELC__
        #pragma warning(disable: 177)
#endif 
#ifdef __WINDOWS__
    WX_DEFINE_GLOBAL_CONV2(wxMBConv, wxMBConv_win32, wxConvLibc, wxEMPTY_PARAMETER_VALUE);
#elif 0     WX_DEFINE_GLOBAL_CONV2(wxMBConv, wxMBConv_cf, wxConvLibc,  (wxFONTENCODING_UTF8));
#else
    WX_DEFINE_GLOBAL_CONV2(wxMBConv, wxMBConvLibc, wxConvLibc, wxEMPTY_PARAMETER_VALUE);
#endif

WX_DEFINE_GLOBAL_CONV(wxMBConvStrictUTF8, wxConvUTF8, ;);
WX_DEFINE_GLOBAL_CONV(wxMBConvUTF7, wxConvUTF7, ;);
WX_DEFINE_GLOBAL_CONV(wxWhateverWorksConv, wxConvWhateverWorks, ;);

WX_DEFINE_GLOBAL_CONV(wxCSConv, wxConvLocal, (wxFONTENCODING_SYSTEM));
WX_DEFINE_GLOBAL_CONV(wxCSConv, wxConvISO8859_1, (wxFONTENCODING_ISO8859_1));

WXDLLIMPEXP_DATA_BASE(wxMBConv *) wxConvCurrent = wxGet_wxConvLibcPtr();
WXDLLIMPEXP_DATA_BASE(wxMBConv *) wxConvUI = wxGet_wxConvLocalPtr();

#ifdef __DARWIN__
static wxMBConv_cf wxConvMacUTF8DObj(wxFONTENCODING_UTF8);
#endif

WXDLLIMPEXP_DATA_BASE(wxMBConv *) wxConvFileName =
#ifdef __DARWIN__
                                    &wxConvMacUTF8DObj;
#else                                     wxGet_wxConvWhateverWorksPtr();
#endif 