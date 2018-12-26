
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/string.h"
#endif

#include "wx/strconv.h"
#include "wx/fontmap.h"

#ifdef __DARWIN__

#include "wx/osx/core/private/strconv_cf.h"
#include "wx/osx/core/cfref.h"





#if wxUSE_FONTMAP
WXDLLIMPEXP_BASE wxMBConv* new_wxMBConv_cf( const char* name)
{
    wxMBConv_cf *result = new wxMBConv_cf(name);
    if(!result->IsOk())
    {
        delete result;
        return NULL;
    }
    else
        return result;
}
#endif 
WXDLLIMPEXP_BASE wxMBConv* new_wxMBConv_cf(wxFontEncoding encoding)
{
    wxMBConv_cf *result = new wxMBConv_cf(encoding);
    if(!result->IsOk())
    {
        delete result;
        return NULL;
    }
    else
        return result;
}

#ifdef WORDS_BIGENDIAN
    static const CFStringEncoding wxCFStringEncodingWcharT = kCFStringEncodingUTF32BE;
#else
    static const CFStringEncoding wxCFStringEncodingWcharT = kCFStringEncodingUTF32LE;
#endif

    size_t wxMBConv_cf::ToWChar(wchar_t * dst, size_t dstSize, const char * src, size_t srcSize) const
    {
        wxCHECK(src, wxCONV_FAILED);

        
        if(srcSize == wxNO_LEN)
            srcSize = strlen(src) + 1;

                wxCFRef<CFStringRef> theString( CFStringCreateWithBytes (
                                                NULL,                                                 (const UInt8*)src,
                                                srcSize,
                                                m_encoding,
                                                false                                                 ));

        if ( theString == NULL )
            return wxCONV_FAILED;

                                wxCFRef<CFMutableStringRef>
            cfMutableString(CFStringCreateMutableCopy(NULL, 0, theString));
        CFStringNormalize(cfMutableString, kCFStringNormalizationFormC);
        theString = cfMutableString;

        



        if(CFStringIsEncodingAvailable(wxCFStringEncodingWcharT))
        {
            CFRange fullStringRange = CFRangeMake(0, CFStringGetLength(theString));
            CFIndex usedBufLen;

            CFIndex charsConverted = CFStringGetBytes(
                    theString,
                    fullStringRange,
                    wxCFStringEncodingWcharT,
                    0,
                    false,
                                        dstSize != 0?(UInt8*)dst:NULL,
                    dstSize * sizeof(wchar_t),
                    &usedBufLen);

            if(charsConverted < CFStringGetLength(theString))
                return wxCONV_FAILED;

            
            wxASSERT( (usedBufLen % sizeof(wchar_t)) == 0 );

                                    return usedBufLen / sizeof(wchar_t);
        }
        else
        {
                        
            size_t returnSize = CFStringGetLength(theString);

            if (dstSize == 0 || dst == NULL)
            {
                return returnSize;
            }

                                    CFRange fullStringRange = CFRangeMake(0, CFStringGetLength(theString));
            UniChar *szUniCharBuffer = new UniChar[fullStringRange.length];

            CFStringGetCharacters(theString, fullStringRange, szUniCharBuffer);

            wxMBConvUTF16 converter;
            returnSize = converter.ToWChar( dst, dstSize, (const char*)szUniCharBuffer, fullStringRange.length );
            delete [] szUniCharBuffer;

            return returnSize;
        }
            }

    size_t wxMBConv_cf::FromWChar(char *dst, size_t dstSize, const wchar_t *src, size_t srcSize) const
    {
        wxCHECK(src, wxCONV_FAILED);

        if(srcSize == wxNO_LEN)
            srcSize = wxStrlen(src) + 1;

                wxCFRef<CFStringRef> theString;


        if(CFStringIsEncodingAvailable(wxCFStringEncodingWcharT))
        {
            theString = wxCFRef<CFStringRef>(CFStringCreateWithBytes(
                    kCFAllocatorDefault,
                    (UInt8*)src,
                    srcSize * sizeof(wchar_t),
                    wxCFStringEncodingWcharT,
                    false));
        }
        else
        {
            wxMBConvUTF16 converter;
            size_t cbUniBuffer = converter.FromWChar( NULL, 0, src, srcSize );
            wxASSERT(cbUniBuffer % sizeof(UniChar));

                        UniChar *tmpUniBuffer = (UniChar*)malloc(cbUniBuffer);

            cbUniBuffer = converter.FromWChar( (char*) tmpUniBuffer, cbUniBuffer, src, srcSize );
            wxASSERT(cbUniBuffer % sizeof(UniChar));

            theString = wxCFRef<CFStringRef>(CFStringCreateWithCharactersNoCopy(
                        kCFAllocatorDefault,
                        tmpUniBuffer,
                        cbUniBuffer / sizeof(UniChar),
                        kCFAllocatorMalloc
                    ));

        }

        wxCHECK(theString != NULL, wxCONV_FAILED);

        CFIndex usedBufLen;

        CFIndex charsConverted = CFStringGetBytes(
                theString,
                CFRangeMake(0, CFStringGetLength(theString)),
                m_encoding,
                0,                 false,                 (UInt8*)dst,
                dstSize,
                &usedBufLen
            );

                        if( (charsConverted < CFStringGetLength(theString)) ||
                (dst && (size_t) usedBufLen > dstSize) )
            return wxCONV_FAILED;

        return usedBufLen;
    }

#endif 

