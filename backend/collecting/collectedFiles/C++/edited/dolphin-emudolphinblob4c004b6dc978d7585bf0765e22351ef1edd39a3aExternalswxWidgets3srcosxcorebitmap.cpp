
#include "wx/wxprec.h"

#include "wx/bitmap.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/dcmemory.h"
    #include "wx/icon.h"
    #include "wx/image.h"
#endif

#include "wx/metafile.h"
#include "wx/xpmdecod.h"

#include "wx/rawbmp.h"

#include "wx/filename.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxBitmap, wxGDIObject);
wxIMPLEMENT_DYNAMIC_CLASS(wxMask, wxObject);

#include "wx/osx/private.h"

CGColorSpaceRef wxMacGetGenericRGBColorSpace();
CGDataProviderRef wxMacCGDataProviderCreateWithMemoryBuffer( const wxMemoryBuffer& buf );



class WXDLLEXPORT wxBitmapRefData: public wxGDIRefData
{
    friend class WXDLLIMPEXP_FWD_CORE wxIcon;
    friend class WXDLLIMPEXP_FWD_CORE wxCursor;
public:
    wxBitmapRefData(int width , int height , int depth, double logicalscale);
    wxBitmapRefData(int width , int height , int depth);
    wxBitmapRefData(CGContextRef context);
    wxBitmapRefData(CGImageRef image, double scale);
    wxBitmapRefData();
    wxBitmapRefData(const wxBitmapRefData &tocopy);

    virtual ~wxBitmapRefData();

    virtual bool IsOk() const wxOVERRIDE { return m_ok; }

    void Free();
    void SetOk( bool isOk) { m_ok = isOk; }

    void SetWidth( int width ) { m_width = width; }
    void SetHeight( int height ) { m_height = height; }
    void SetDepth( int depth ) { m_depth = depth; }

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetDepth() const { return m_depth; }
    double GetScaleFactor() const { return m_scaleFactor; }
    void *GetRawAccess() const;
    void *BeginRawAccess();
    void EndRawAccess();

    bool HasAlpha() const { return m_hasAlpha; }
    void UseAlpha( bool useAlpha );

    bool IsTemplate() const { return m_isTemplate; }
    void SetTemplate(bool is) { m_isTemplate = is; }

public:
#if wxUSE_PALETTE
    wxPalette     m_bitmapPalette;
#endif 
    wxMask *      m_bitmapMask;     CGImageRef    CreateCGImage() const;

                        bool          HasNativeSize();

#ifndef __WXOSX_IPHONE__
            IconRef       GetIconRef();
#endif

    CGContextRef  GetBitmapContext() const;

    int           GetBytesPerRow() const { return m_bytesPerRow; }
    private :
    bool Create(int width , int height , int depth);
    bool Create(int width , int height , int depth, double logicalScale);
    bool Create( CGImageRef image, double scale );
    bool Create( CGContextRef bitmapcontext);
    void Init();

    int           m_width;
    int           m_height;
    int           m_bytesPerRow;
    int           m_depth;
    bool          m_hasAlpha;
    wxMemoryBuffer m_memBuf;
    int           m_rawAccessCount;
    bool          m_ok;
    mutable CGImageRef    m_cgImageRef;
    bool          m_isTemplate;

#ifndef __WXOSX_IPHONE__
    IconRef       m_iconRef;
#endif

    CGContextRef  m_hBitmap;
    double        m_scaleFactor;
};


#define wxOSX_USE_PREMULTIPLIED_ALPHA 1
static const int kBestByteAlignement = 16;
static const int kMaskBytesPerPixel = 1;

static int GetBestBytesPerRow( int rawBytes )
{
    return (((rawBytes)+kBestByteAlignement-1) & ~(kBestByteAlignement-1) );
}

#if wxUSE_GUI && !defined(__WXOSX_IPHONE__)


void wxMacCreateBitmapButton( ControlButtonContentInfo*info , const wxBitmap& bitmap , int forceType )
{
    memset( info , 0 , sizeof(ControlButtonContentInfo) ) ;
    if ( bitmap.IsOk() )
    {
        wxBitmapRefData * bmap = bitmap.GetBitmapData() ;
        if ( bmap == NULL )
            return ;

        if ( forceType == 0  )
        {
            forceType = kControlContentCGImageRef;
        }

        if ( forceType == kControlContentIconRef )
        {
            wxBitmap scaleBmp ;
            wxBitmapRefData* bmp = bmap ;

            if ( !bmap->HasNativeSize() )
            {
                                
                int w = bitmap.GetWidth() ;
                int h = bitmap.GetHeight() ;
                int sz = wxMax( w , h ) ;
                if ( sz == 24 || sz == 64 )
                {
                    scaleBmp = wxBitmap( bitmap.ConvertToImage().Scale( w * 2 , h * 2 ) ) ;
                    bmp = scaleBmp.GetBitmapData() ;
                }
            }

            info->contentType = kControlContentIconRef ;
            info->u.iconRef = bmp->GetIconRef() ;
            AcquireIconRef( info->u.iconRef ) ;
        }
        else if ( forceType == kControlContentCGImageRef )
        {
            info->contentType = kControlContentCGImageRef ;
            info->u.imageRef = (CGImageRef) bmap->CreateCGImage() ;
        }
    }
}

CGImageRef wxMacCreateCGImageFromBitmap( const wxBitmap& bitmap )
{
    wxBitmapRefData * bmap = bitmap.GetBitmapData() ;
    if ( bmap == NULL )
        return NULL ;
    return (CGImageRef) bmap->CreateCGImage();
}

void wxMacReleaseBitmapButton( ControlButtonContentInfo*info )
{
    if ( info->contentType == kControlContentIconRef )
    {
        ReleaseIconRef( info->u.iconRef ) ;
    }
    else if ( info->contentType == kControlNoContent )
    {
            }
    else if ( info->contentType == kControlContentPictHandle )
    {
            }
    else if ( info->contentType == kControlContentCGImageRef )
    {
        CGImageRelease( info->u.imageRef ) ;
    }
    else
    {
        wxFAIL_MSG(wxT("Unexpected bitmap type") ) ;
    }
}

#endif 
#define M_BITMAPDATA ((wxBitmapRefData *)m_refData)

void wxBitmapRefData::Init()
{
    m_width = 0 ;
    m_height = 0 ;
    m_depth = 0 ;
    m_bytesPerRow = 0;
    m_ok = false ;
    m_bitmapMask = NULL ;
    m_cgImageRef = NULL ;
    m_isTemplate = false;

#ifndef __WXOSX_IPHONE__
    m_iconRef = NULL ;
#endif
    m_hBitmap = NULL ;

    m_rawAccessCount = 0 ;
    m_hasAlpha = false;
    m_scaleFactor = 1.0;
}

wxBitmapRefData::wxBitmapRefData(const wxBitmapRefData &tocopy) : wxGDIRefData()
{
    Init();
    Create(tocopy.m_width, tocopy.m_height, tocopy.m_depth, tocopy.m_scaleFactor);

    if (tocopy.m_bitmapMask)
        m_bitmapMask = new wxMask(*tocopy.m_bitmapMask);
    else if (tocopy.m_hasAlpha)
        UseAlpha(true);

    unsigned char* dest = (unsigned char*)GetRawAccess();
    unsigned char* source = (unsigned char*)tocopy.GetRawAccess();
    size_t numbytes = m_bytesPerRow * m_height;
    memcpy( dest, source, numbytes );
}

wxBitmapRefData::wxBitmapRefData()
{
    Init() ;
}

wxBitmapRefData::wxBitmapRefData( int w , int h , int d )
{
    Init() ;
    Create( w , h , d ) ;
}

wxBitmapRefData::wxBitmapRefData(int w , int h , int d, double logicalscale)
{
    Init() ;
    Create( w , h , d, logicalscale ) ;
}

wxBitmapRefData::wxBitmapRefData(CGContextRef context)
{
    Init();
    Create( context );
}

wxBitmapRefData::wxBitmapRefData(CGImageRef image, double scale)
{
    Init();
    Create( image, scale );
}    

bool wxBitmapRefData::Create(CGImageRef image, double scale)
{
    if ( image != NULL )
    {
        m_width = CGImageGetWidth(image);
        m_height = CGImageGetHeight(image);
        m_depth = 32;
        m_hBitmap = NULL;
        m_scaleFactor = scale;
        
        m_bytesPerRow = GetBestBytesPerRow( m_width * 4 ) ;
        size_t size = m_bytesPerRow * m_height ;
        void* data = m_memBuf.GetWriteBuf( size ) ;
        if ( data != NULL )
        {
            memset( data , 0 , size ) ;
            m_memBuf.UngetWriteBuf( size ) ;
            CGImageAlphaInfo alpha = CGImageGetAlphaInfo(image);
            if ( alpha == kCGImageAlphaNone || alpha == kCGImageAlphaNoneSkipFirst || alpha == kCGImageAlphaNoneSkipLast )
            {
                m_hBitmap = CGBitmapContextCreate((char*) data, m_width, m_height, 8, m_bytesPerRow, wxMacGetGenericRGBColorSpace(), kCGImageAlphaNoneSkipFirst );
            }
            else 
            {
                m_hasAlpha = true;
                m_hBitmap = CGBitmapContextCreate((char*) data, m_width, m_height, 8, m_bytesPerRow, wxMacGetGenericRGBColorSpace(), kCGImageAlphaPremultipliedFirst );
            }
            CGRect rect = CGRectMake(0,0,m_width,m_height);
            CGContextDrawImage(m_hBitmap, rect, image);
            
            wxASSERT_MSG( m_hBitmap , wxT("Unable to create CGBitmapContext context") ) ;
            CGContextTranslateCTM( m_hBitmap, 0,  m_height );
            CGContextScaleCTM( m_hBitmap, 1*m_scaleFactor, -1*m_scaleFactor );
        } 
    }
    m_ok = ( m_hBitmap != NULL ) ;
    
    return m_ok ;
    
}

bool wxBitmapRefData::Create(CGContextRef context)
{
    if ( context != NULL && CGBitmapContextGetData(context) )
    {
        m_hBitmap = context;
        m_bytesPerRow = CGBitmapContextGetBytesPerRow(context);
        m_width = CGBitmapContextGetWidth(context);
        m_height = CGBitmapContextGetHeight(context);
        m_depth = CGBitmapContextGetBitsPerPixel(context) ;

                wxASSERT( m_depth == 32 );
        
                CGRect userrect = CGRectMake(0, 0, 10, 10);
        CGRect devicerect;
        devicerect = CGContextConvertRectToDeviceSpace(context, userrect);
        m_scaleFactor = devicerect.size.height / userrect.size.height;
        
        CGImageAlphaInfo alpha = CGBitmapContextGetAlphaInfo(context);
        
        if ( alpha == kCGImageAlphaNone || alpha == kCGImageAlphaNoneSkipFirst || alpha == kCGImageAlphaNoneSkipLast )
        {
                    }
        else
        {
            m_hasAlpha = true;
        }
    }
    m_ok = ( m_hBitmap != NULL ) ;
    
    return m_ok ;
}

bool wxBitmapRefData::Create( int w , int h , int d )
{
    m_width = wxMax(1, w);
    m_height = wxMax(1, h);
    m_depth = d ;
    m_hBitmap = NULL ;

    m_bytesPerRow = GetBestBytesPerRow( m_width * 4 ) ;
    size_t size = m_bytesPerRow * m_height ;
    void* data = m_memBuf.GetWriteBuf( size ) ;
    if ( data != NULL )
    {
        memset( data , 0 , size ) ;
        m_memBuf.UngetWriteBuf( size ) ;

        m_hBitmap = CGBitmapContextCreate((char*) data, m_width, m_height, 8, m_bytesPerRow, wxMacGetGenericRGBColorSpace(), kCGImageAlphaNoneSkipFirst );
        wxASSERT_MSG( m_hBitmap , wxT("Unable to create CGBitmapContext context") ) ;
        CGContextTranslateCTM( m_hBitmap, 0,  m_height );
        CGContextScaleCTM( m_hBitmap, 1*m_scaleFactor, -1*m_scaleFactor );
    } 
    m_ok = ( m_hBitmap != NULL ) ;

    return m_ok ;
}

bool wxBitmapRefData::Create( int w , int h , int d, double logicalScale )
{
    m_scaleFactor = logicalScale;
    return Create(w*logicalScale,h*logicalScale,d);
}

void wxBitmapRefData::UseAlpha( bool use )
{
    if ( m_hasAlpha == use )
        return ;

    m_hasAlpha = use ;

    CGContextRelease( m_hBitmap );
    m_hBitmap = CGBitmapContextCreate((char*) m_memBuf.GetData(), m_width, m_height, 8, m_bytesPerRow, wxMacGetGenericRGBColorSpace(), m_hasAlpha ? kCGImageAlphaPremultipliedFirst : kCGImageAlphaNoneSkipFirst );
    wxASSERT_MSG( m_hBitmap , wxT("Unable to create CGBitmapContext context") ) ;
    CGContextTranslateCTM( m_hBitmap, 0,  m_height );
    CGContextScaleCTM( m_hBitmap, 1*m_scaleFactor, -1*m_scaleFactor );
}

void *wxBitmapRefData::GetRawAccess() const
{
    wxCHECK_MSG( IsOk(), NULL , wxT("invalid bitmap") ) ;
    return m_memBuf.GetData() ;
}

void *wxBitmapRefData::BeginRawAccess()
{
    wxCHECK_MSG( IsOk(), NULL, wxT("invalid bitmap") ) ;
    wxASSERT( m_rawAccessCount == 0 ) ;
#ifndef __WXOSX_IPHONE__
    wxASSERT_MSG( m_iconRef == NULL ,
                 wxT("Currently, modifing bitmaps that are used in controls already is not supported") ) ;
#endif
    ++m_rawAccessCount ;

            if ( m_cgImageRef )
    {
        CGImageRelease( m_cgImageRef ) ;
        m_cgImageRef = NULL ;
    }

    return m_memBuf.GetData() ;
}

void wxBitmapRefData::EndRawAccess()
{
    wxCHECK_RET( IsOk() , wxT("invalid bitmap") ) ;
    wxASSERT( m_rawAccessCount == 1 ) ;

    --m_rawAccessCount ;
}

bool wxBitmapRefData::HasNativeSize()
{
    int w = GetWidth() ;
    int h = GetHeight() ;
    int sz = wxMax( w , h ) ;

    return ( sz == 128 || sz == 48 || sz == 32 || sz == 16 );
}

#ifndef __WXOSX_IPHONE__
IconRef wxBitmapRefData::GetIconRef()
{
    if ( m_iconRef == NULL )
    {
        
        IconFamilyHandle iconFamily = (IconFamilyHandle) NewHandle( 0 );

        int w = GetWidth() ;
        int h = GetHeight() ;
        int sz = wxMax( w , h ) ;

        OSType dataType = 0 ;
        OSType maskType = 0 ;

                                if (sz <= 16)
            sz = 16;
        else if ( sz <= 32)
            sz = 32;
        else if ( sz <= 48)
            sz = 48;
        else if ( sz <= 128)
            sz = 128;
        else if ( sz <= 256)
            sz = 256;
        else if ( sz <= 512)
            sz = 512;
        else if ( sz <= 1024)
            sz = 1024;
        
        switch (sz)
        {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
            case 1024:
                dataType = kIconServices1024PixelDataARGB;
                break;
#endif
            case 512:
                dataType = kIconServices512PixelDataARGB;
                break;
                
            case 256:
                dataType = kIconServices256PixelDataARGB;
                break;
 
            case 128:
                dataType = kIconServices128PixelDataARGB ;
                break;

            case 48:
                dataType = kIconServices48PixelDataARGB ;
                break;

            case 32:
                dataType = kIconServices32PixelDataARGB ;
                break;

            case 16:
                dataType = kIconServices16PixelDataARGB ;
                break;

            default:
                break;
        }

        if ( dataType != 0 )
        {
            if (  maskType == 0 )
            {
                size_t datasize = sz * sz * 4 ;
                Handle data = NewHandle( datasize ) ;
                HLock( data ) ;
                unsigned char* ptr = (unsigned char*) *data ;
                memset( ptr, 0, datasize );
                bool hasAlpha = HasAlpha() ;
                wxMask *mask = m_bitmapMask ;
                unsigned char * sourcePtr = (unsigned char*) GetRawAccess() ;
                unsigned char * masksourcePtr = mask ? (unsigned char*) mask->GetRawAccess() : NULL ;

                for ( int y = 0 ; y < h ; ++y, sourcePtr += m_bytesPerRow , masksourcePtr += mask ? mask->GetBytesPerRow() : 0 )
                {
                    unsigned char * source = sourcePtr;
                    unsigned char * masksource = masksourcePtr;
                    unsigned char * dest = ptr + y * sz * 4 ;
                    unsigned char a, r, g, b;

                    for ( int x = 0 ; x < w ; ++x )
                    {
                        a = *source ++ ;
                        r = *source ++ ;
                        g = *source ++ ;
                        b = *source ++ ;

                        if ( mask )
                        {
                            a = 0xFF - *masksource++ ;
                        }
                        else if ( !hasAlpha )
                            a = 0xFF ;
                        else
                        {
#if wxOSX_USE_PREMULTIPLIED_ALPHA
                                                        if ( a != 0xFF && a!= 0 )
                            {
                                r = r * 255 / a;
                                g = g * 255 / a;
                                b = b * 255 / a;
                            }
#endif
                        }
                        *dest++ = a ;
                        *dest++ = r ;
                        *dest++ = g ;
                        *dest++ = b ;

                     }
                }
                HUnlock( data );

                OSStatus err = SetIconFamilyData( iconFamily, dataType , data );
                if ( err != noErr )
                {
                    wxFAIL_MSG("Error when adding bitmap");
                }

                DisposeHandle( data );
            }
            else
            {
                
                Handle data = NULL ;
                Handle maskdata = NULL ;
                unsigned char * maskptr = NULL ;
                unsigned char * ptr = NULL ;
                size_t datasize, masksize ;

                datasize = sz * sz * 4 ;
                data = NewHandle( datasize ) ;
                HLock( data ) ;
                ptr = (unsigned char*) *data ;
                memset( ptr , 0, datasize ) ;

                masksize = sz * sz ;
                maskdata = NewHandle( masksize ) ;
                HLock( maskdata ) ;
                maskptr = (unsigned char*) *maskdata ;
                memset( maskptr , 0 , masksize ) ;

                bool hasAlpha = HasAlpha() ;
                wxMask *mask = m_bitmapMask ;
                unsigned char * sourcePtr = (unsigned char*) GetRawAccess() ;
                unsigned char * masksourcePtr = mask ? (unsigned char*) mask->GetRawAccess() : NULL ;

                for ( int y = 0 ; y < h ; ++y, sourcePtr += m_bytesPerRow , masksourcePtr += mask ? mask->GetBytesPerRow() : 0 )
                {
                    unsigned char * source = sourcePtr;
                    unsigned char * masksource = masksourcePtr;
                    unsigned char * dest = ptr + y * sz * 4 ;
                    unsigned char * maskdest = maskptr + y * sz ;
                    unsigned char a, r, g, b;

                    for ( int x = 0 ; x < w ; ++x )
                    {
                        a = *source ++ ;
                        r = *source ++ ;
                        g = *source ++ ;
                        b = *source ++ ;

                        *dest++ = 0 ;
                        *dest++ = r ;
                        *dest++ = g ;
                        *dest++ = b ;

                        if ( mask )
                            *maskdest++ = 0xFF - *masksource++ ;
                        else if ( hasAlpha )
                            *maskdest++ = a ;
                        else
                            *maskdest++ = 0xFF ;
                    }
                }

                OSStatus err = SetIconFamilyData( iconFamily, dataType , data ) ;
                wxASSERT_MSG( err == noErr , wxT("Error when adding bitmap") ) ;

                err = SetIconFamilyData( iconFamily, maskType , maskdata ) ;
                wxASSERT_MSG( err == noErr , wxT("Error when adding mask") ) ;

                HUnlock( data ) ;
                HUnlock( maskdata ) ;
                DisposeHandle( data ) ;
                DisposeHandle( maskdata ) ;
            }
        }
        
                HLock((Handle) iconFamily);
        OSStatus err = GetIconRefFromIconFamilyPtr( *iconFamily, GetHandleSize((Handle) iconFamily), &m_iconRef );
        HUnlock((Handle) iconFamily);
        DisposeHandle( (Handle) iconFamily ) ;

        wxCHECK_MSG( err == noErr, NULL, wxT("Error when constructing icon ref") );
    }

    return m_iconRef ;
}

#endif

CGImageRef wxBitmapRefData::CreateCGImage() const
{
    wxASSERT( m_ok ) ;
    wxASSERT( m_rawAccessCount >= 0 ) ;
    CGImageRef image ;
    if ( m_rawAccessCount > 0 || m_cgImageRef == NULL )
    {
        if ( m_depth != 1 && m_bitmapMask == NULL )
        {
#if 0
                                    if ( m_bitmapMask )
            {
                CGImageRef tempImage = CGBitmapContextCreateImage( m_hBitmap );
                CGImageRef tempMask = CGBitmapContextCreateImage((CGContextRef) m_bitmapMask->GetHBITMAP() );
                image = CGImageCreateWithMask( tempImage, tempMask );
                CGImageRelease(tempMask);
                CGImageRelease(tempImage);
            }
            else
#endif
                image = CGBitmapContextCreateImage( m_hBitmap );
        }
        else
        {
            size_t imageSize = m_height * m_bytesPerRow ;
            void * dataBuffer = m_memBuf.GetData() ;
            int w = m_width ;
            int h = m_height ;
            CGImageAlphaInfo alphaInfo = kCGImageAlphaNoneSkipFirst ;
            wxMemoryBuffer membuf;

            if ( m_bitmapMask )
            {
                alphaInfo = kCGImageAlphaFirst ;
                unsigned char *destalphastart = (unsigned char*) membuf.GetWriteBuf( imageSize ) ;
                memcpy( destalphastart , dataBuffer , imageSize ) ;
                unsigned char *sourcemaskstart = (unsigned char *) m_bitmapMask->GetRawAccess() ;
                int maskrowbytes = m_bitmapMask->GetBytesPerRow() ;
                for ( int y = 0 ; y < h ; ++y , destalphastart += m_bytesPerRow, sourcemaskstart += maskrowbytes)
                {
                    unsigned char *sourcemask = sourcemaskstart ;
                    unsigned char *destalpha = destalphastart ;
                    for ( int x = 0 ; x < w ; ++x , sourcemask += kMaskBytesPerPixel , destalpha += 4 )
                    {
                        *destalpha = 0xFF - *sourcemask ;
                    }
                }
                membuf.UngetWriteBuf( imageSize );
            }
            else
            {
                if ( m_hasAlpha )
                {
#if wxOSX_USE_PREMULTIPLIED_ALPHA
                    alphaInfo = kCGImageAlphaPremultipliedFirst ;
#else
                    alphaInfo = kCGImageAlphaFirst ;
#endif
                }

                membuf = m_memBuf;
            }

            CGDataProviderRef dataProvider = NULL ;
            if ( m_depth == 1 )
            {
                                wxMemoryBuffer maskBuf;
                unsigned char * maskBufData = (unsigned char*) maskBuf.GetWriteBuf( m_width * m_height );
                unsigned char * bufData = (unsigned char *) membuf.GetData() ;
                                size_t i = 0;
                for( int y = 0 ; y < m_height ; bufData+= m_bytesPerRow, ++y )
                {
                    unsigned char *bufDataIter = bufData+3;
                    for ( int x = 0 ; x < m_width ; bufDataIter += 4, ++x, ++i )
                    {
                        maskBufData[i] = *bufDataIter;
                    }
                }
                maskBuf.UngetWriteBuf( m_width * m_height );

                dataProvider =
                    wxMacCGDataProviderCreateWithMemoryBuffer( maskBuf );

                image = ::CGImageMaskCreate( w, h, 8, 8, m_width , dataProvider, NULL, false );
            }
            else
            {
                CGColorSpaceRef colorSpace = wxMacGetGenericRGBColorSpace();
                dataProvider = wxMacCGDataProviderCreateWithMemoryBuffer( membuf );
                image =
                    ::CGImageCreate(
                                    w, h, 8 , 32 , m_bytesPerRow , colorSpace, alphaInfo ,
                                    dataProvider, NULL , false , kCGRenderingIntentDefault );
            }
            CGDataProviderRelease( dataProvider);
        }
    }
    else
    {
        image = m_cgImageRef ;
        CGImageRetain( image ) ;
    }

    if ( m_rawAccessCount == 0 && m_cgImageRef == NULL)
    {
                m_cgImageRef = image ;
        CGImageRetain( image ) ;
    }

    return image ;
}

CGContextRef wxBitmapRefData::GetBitmapContext() const
{
    return m_hBitmap;
}

void wxBitmapRefData::Free()
{
    wxASSERT_MSG( m_rawAccessCount == 0 , wxT("Bitmap still selected when destroyed") ) ;

    if ( m_cgImageRef )
    {
        CGImageRelease( m_cgImageRef ) ;
        m_cgImageRef = NULL ;
    }
#ifndef __WXOSX_IPHONE__
    if ( m_iconRef )
    {
        ReleaseIconRef( m_iconRef ) ;
        m_iconRef = NULL ;
    }
#endif
    if ( m_hBitmap )
    {
        CGContextRelease(m_hBitmap);
        m_hBitmap = NULL ;
    }

    wxDELETE(m_bitmapMask);
}

wxBitmapRefData::~wxBitmapRefData()
{
    Free() ;
}




bool wxBitmap::CopyFromIcon(const wxIcon& icon)
{
    bool created = false ;
    int w = icon.GetWidth() ;
    int h = icon.GetHeight() ;

    Create( w , h ) ;
    if ( !created )
    {
        wxMemoryDC dc ;
        dc.SelectObject( *this ) ;
        dc.DrawIcon( icon , 0 , 0 ) ;
        dc.SelectObject( wxNullBitmap ) ;
    }

    return true;
}

wxBitmap::wxBitmap(const char bits[], int the_width, int the_height, int no_bits)
{
    wxBitmapRefData* bitmapRefData;

    m_refData = bitmapRefData = new wxBitmapRefData( the_width , the_height , no_bits ) ;

    if (bitmapRefData->IsOk())
    {
        if ( no_bits == 1 )
        {
            int linesize = the_width / 8;
            if ( the_width % 8 )
                linesize++;

            unsigned char* linestart = (unsigned char*) bits ;
            unsigned char* destptr = (unsigned char*) BeginRawAccess() ;

            for ( int y = 0 ; y < the_height ; ++y , linestart += linesize, destptr += M_BITMAPDATA->GetBytesPerRow() )
            {
                unsigned char* destination = destptr;
                int index, bit, mask;

                for ( int x = 0 ; x < the_width ; ++x )
                {
                    index = x / 8 ;
                    bit = x % 8 ;
                    mask = 1 << bit ;

                    if ( linestart[index] & mask )
                    {
                        *destination++ = 0xFF ;
                        *destination++ = 0 ;
                        *destination++ = 0 ;
                        *destination++ = 0 ;
                    }
                    else
                    {
                        *destination++ = 0xFF ;
                        *destination++ = 0xFF ;
                        *destination++ = 0xFF ;
                        *destination++ = 0xFF ;
                    }
                }
            }

            EndRawAccess() ;
        }
        else
        {
            wxFAIL_MSG(wxT("multicolor BITMAPs not yet implemented"));
        }
    } 
}

wxBitmap::wxBitmap(const void* data, wxBitmapType type, int width, int height, int depth)
{
    (void) Create(data, type, width, height, depth);
}

wxBitmap::wxBitmap(int width, int height, const wxDC& dc)
{
    (void)Create(width, height, dc);
}

wxBitmap::wxBitmap(const wxString& filename, wxBitmapType type)
{
    LoadFile(filename, type);
}

wxBitmap::wxBitmap(CGImageRef image, double scale)
{
    (void) Create(image,scale);
}

wxGDIRefData* wxBitmap::CreateGDIRefData() const
{
    return new wxBitmapRefData;
}

wxGDIRefData* wxBitmap::CloneGDIRefData(const wxGDIRefData* data) const
{
    return new wxBitmapRefData(*static_cast<const wxBitmapRefData *>(data));
}

void * wxBitmap::GetRawAccess() const
{
    wxCHECK_MSG( IsOk() , NULL , wxT("invalid bitmap") ) ;

    return M_BITMAPDATA->GetRawAccess() ;
}

void * wxBitmap::BeginRawAccess()
{
    wxCHECK_MSG( IsOk() , NULL , wxT("invalid bitmap") ) ;

    return M_BITMAPDATA->BeginRawAccess() ;
}

void wxBitmap::EndRawAccess()
{
    wxCHECK_RET( IsOk() , wxT("invalid bitmap") ) ;

    M_BITMAPDATA->EndRawAccess() ;
}

CGImageRef wxBitmap::CreateCGImage() const
{
    wxCHECK_MSG( IsOk(), NULL , wxT("invalid bitmap") ) ;

    return M_BITMAPDATA->CreateCGImage() ;
}

#ifndef  __WXOSX_IPHONE__
IconRef wxBitmap::GetIconRef() const
{
    wxCHECK_MSG( IsOk(), NULL , wxT("invalid bitmap") ) ;

    return M_BITMAPDATA->GetIconRef() ;
}

IconRef wxBitmap::CreateIconRef() const
{
    IconRef icon = GetIconRef();
    verify_noerr( AcquireIconRef(icon) );
    return icon;
}
#endif

#if wxOSX_USE_COCOA

wxBitmap::wxBitmap(WX_NSImage image)
{
    (void)Create(image);
}

bool wxBitmap::Create(WX_NSImage image)
{
    bool isTemplate;
    if (!Create(wxOSXCreateBitmapContextFromNSImage(image, &isTemplate)))
        return false;
    M_BITMAPDATA->SetTemplate(isTemplate);
    return true;
}

wxBitmap::wxBitmap(CGContextRef bitmapcontext)
{
    (void)Create(bitmapcontext);
}

bool wxBitmap::Create(CGContextRef bitmapcontext)
{
    UnRef();
    
    m_refData = new wxBitmapRefData( bitmapcontext );
    
    return M_BITMAPDATA->IsOk() ;
}

WX_NSImage wxBitmap::GetNSImage() const
{
    wxCFRef< CGImageRef > cgimage(CreateCGImage());
    return wxOSXGetNSImageFromCGImage( cgimage, GetScaleFactor(), M_BITMAPDATA->IsTemplate() );
}

#endif

#if wxOSX_USE_IPHONE

WX_UIImage wxBitmap::GetUIImage() const
{
    wxCFRef< CGImageRef > cgimage(CreateCGImage());
    return wxOSXGetUIImageFromCGImage( cgimage );
}

#endif
wxBitmap wxBitmap::GetSubBitmap(const wxRect &rect) const
{
    wxCHECK_MSG( IsOk() &&
                (rect.x >= 0) && (rect.y >= 0) &&
                (rect.x+rect.width <= GetWidth()) &&
                (rect.y+rect.height <= GetHeight()),
                wxNullBitmap, wxT("invalid bitmap or bitmap region") );

    wxBitmap ret;
    double scale = GetScaleFactor();
    ret.CreateScaled( rect.width, rect.height, GetDepth(), scale );
    wxASSERT_MSG( ret.IsOk(), wxT("GetSubBitmap error") );

    int destwidth = rect.width*scale ;
    int destheight = rect.height*scale ;

    {
        unsigned char *sourcedata = (unsigned char*) GetRawAccess() ;
        unsigned char *destdata = (unsigned char*) ret.BeginRawAccess() ;
        wxASSERT((sourcedata != NULL) && (destdata != NULL));

        if ( (sourcedata != NULL) && (destdata != NULL) )
        {
            int sourcelinesize = GetBitmapData()->GetBytesPerRow() ;
            int destlinesize = ret.GetBitmapData()->GetBytesPerRow() ;
            unsigned char *source = sourcedata + int(rect.x * scale * 4 + rect.y *scale * sourcelinesize) ;
            unsigned char *dest = destdata ;

            for (int yy = 0; yy < destheight; ++yy, source += sourcelinesize , dest += destlinesize)
            {
                memcpy( dest , source , destlinesize ) ;
            }
        }
        ret.EndRawAccess() ;
    }


    if ( M_BITMAPDATA->m_bitmapMask )
    {
        wxMemoryBuffer maskbuf ;
        int rowBytes = GetBestBytesPerRow( destwidth * kMaskBytesPerPixel );
        size_t maskbufsize = rowBytes * destheight ;

        int sourcelinesize = M_BITMAPDATA->m_bitmapMask->GetBytesPerRow() ;
        int destlinesize = rowBytes ;

        unsigned char *source = (unsigned char *) M_BITMAPDATA->m_bitmapMask->GetRawAccess() ;
        unsigned char *destdata = (unsigned char * ) maskbuf.GetWriteBuf( maskbufsize ) ;
        wxASSERT( (source != NULL) && (destdata != NULL) ) ;

        if ( (source != NULL) && (destdata != NULL) )
        {
            source += rect.x * kMaskBytesPerPixel + rect.y * sourcelinesize ;
            unsigned char *dest = destdata ;

            for (int yy = 0; yy < destheight; ++yy, source += sourcelinesize , dest += destlinesize)
            {
                memcpy( dest , source , destlinesize ) ;
            }

            maskbuf.UngetWriteBuf( maskbufsize ) ;
        }
        ret.SetMask( new wxMask( maskbuf , destwidth , destheight , rowBytes ) ) ;
    }
    else if ( HasAlpha() )
        ret.UseAlpha() ;

    return ret;
}

bool wxBitmap::Create(int w, int h, int d)
{
    UnRef();

    wxCHECK_MSG(w > 0 && h > 0, false, "invalid bitmap size");

    if ( d < 0 )
        d = wxDisplayDepth() ;

    m_refData = new wxBitmapRefData( w , h , d );

    return M_BITMAPDATA->IsOk() ;
}

bool wxBitmap::Create(int w, int h, const wxDC& dc)
{
    double factor = dc.GetContentScaleFactor();
    return CreateScaled(w,h,wxBITMAP_SCREEN_DEPTH, factor);
}

bool wxBitmap::CreateScaled(int w, int h, int d, double logicalScale)
{
    UnRef();
    
    if ( d < 0 )
        d = wxDisplayDepth() ;
    
    m_refData = new wxBitmapRefData( w , h , d, logicalScale );
    
    return M_BITMAPDATA->IsOk() ;
}

bool wxBitmap::Create(CGImageRef image, double scale)
{
    UnRef();
    
    m_refData = new wxBitmapRefData( image, scale );
    
    return M_BITMAPDATA->IsOk() ;
}

bool wxBitmap::LoadFile(const wxString& filename, wxBitmapType type)
{
    UnRef();

    wxBitmapHandler *handler = FindHandler(type);

    if ( handler )
    {
        m_refData = new wxBitmapRefData;

        return handler->LoadFile(this, filename, type, -1, -1);
    }
    else
    {
#if wxUSE_IMAGE
        double scale = 1.0;
        wxString fname = filename;
        
        if  ( type == wxBITMAP_TYPE_PNG )
        {
            if ( wxOSXGetMainScreenContentScaleFactor() > 1.9 )
            {
                wxFileName fn(filename);
                fn.MakeAbsolute();
                fn.SetName(fn.GetName()+"@2x");
                
                if ( fn.Exists() )
                {
                    fname = fn.GetFullPath();
                    scale = 2.0;
                }
            }
        }

        wxImage loadimage(fname, type);
        if (loadimage.IsOk())
        {
            *this = wxBitmap(loadimage,-1,scale);

            return true;
        }
#endif
    }

    wxLogWarning(wxT("no bitmap handler for type %d defined."), type);

    return false;
}

bool wxBitmap::Create(const void* data, wxBitmapType type, int width, int height, int depth)
{
    UnRef();

    m_refData = new wxBitmapRefData;

    wxBitmapHandler *handler = FindHandler(type);

    if ( handler == NULL )
    {
        wxLogWarning(wxT("no bitmap handler for type %d defined."), type);

        return false;
    }

    return handler->Create(this, data, type, width, height, depth);
}

#if wxUSE_IMAGE

wxBitmap::wxBitmap(const wxImage& image, int depth, double scale)
{
    wxCHECK_RET( image.IsOk(), wxT("invalid image") );

        int width = image.GetWidth();
    int height = image.GetHeight();

    wxBitmapRefData* bitmapRefData;

    m_refData = bitmapRefData = new wxBitmapRefData( width/scale, height/scale, depth, scale) ;

    if ( bitmapRefData->IsOk())
    {
        
        bool hasAlpha = false ;

        if ( image.HasMask() )
        {
                    }
        else
        {
            hasAlpha = image.HasAlpha() ;
        }

        if ( hasAlpha )
            UseAlpha() ;

        unsigned char* destinationstart = (unsigned char*) BeginRawAccess() ;
        unsigned char* data = image.GetData();
        if ( destinationstart != NULL && data != NULL )
        {
            const unsigned char *alpha = hasAlpha ? image.GetAlpha() : NULL ;
            for (int y = 0; y < height; destinationstart += M_BITMAPDATA->GetBytesPerRow(), y++)
            {
                unsigned char * destination = destinationstart;
                for (int x = 0; x < width; x++)
                {
                    if ( hasAlpha )
                    {
                        const unsigned char a = *alpha++;
                        *destination++ = a ;

    #if wxOSX_USE_PREMULTIPLIED_ALPHA
                        *destination++ = ((*data++) * a + 127) / 255 ;
                        *destination++ = ((*data++) * a + 127) / 255 ;
                        *destination++ = ((*data++) * a + 127) / 255 ;
    #else
                        *destination++ = *data++ ;
                        *destination++ = *data++ ;
                        *destination++ = *data++ ;
    #endif
                    }
                    else
                    {
                        *destination++ = 0xFF ;
                        *destination++ = *data++ ;
                        *destination++ = *data++ ;
                        *destination++ = *data++ ;
                    }
                }
            }

            EndRawAccess() ;
        }
        if ( image.HasMask() )
            SetMask( new wxMask( *this , wxColour( image.GetMaskRed() , image.GetMaskGreen() , image.GetMaskBlue() ) ) ) ;
    } 
}

wxImage wxBitmap::ConvertToImage() const
{
    wxImage image;

    wxCHECK_MSG( IsOk(), wxNullImage, wxT("invalid bitmap") );

        int width = GetWidth();
    int height = GetHeight();
    image.Create( width, height );

    unsigned char *data = image.GetData();
    wxCHECK_MSG( data, wxNullImage, wxT("Could not allocate data for image") );

    unsigned char* sourcestart = (unsigned char*) GetRawAccess() ;

    bool hasAlpha = false ;
    bool hasMask = false ;
    int maskBytesPerRow = 0 ;
    unsigned char *alpha = NULL ;
    unsigned char *mask = NULL ;

    if ( HasAlpha() )
        hasAlpha = true ;

    if ( GetMask() )
    {
        hasMask = true ;
        mask = (unsigned char*) GetMask()->GetRawAccess() ;
        maskBytesPerRow = GetMask()->GetBytesPerRow() ;
    }

    if ( hasAlpha )
    {
        image.SetAlpha() ;
        alpha = image.GetAlpha() ;
    }

    int index = 0;

                static const int MASK_RED = 1;
    static const int MASK_GREEN = 2;
    static const int MASK_BLUE = 3;
    static const int MASK_BLUE_REPLACEMENT = 2;

    for (int yy = 0; yy < height; yy++ , sourcestart += M_BITMAPDATA->GetBytesPerRow() , mask += maskBytesPerRow )
    {
        unsigned char * maskp = mask ;
        const wxUint32 * source = (wxUint32*)sourcestart;
        unsigned char a, r, g, b;

        for (int xx = 0; xx < width; xx++)
        {
            const wxUint32 color = *source++;
#ifdef WORDS_BIGENDIAN
            a = ((color&0xFF000000) >> 24) ;
            r = ((color&0x00FF0000) >> 16) ;
            g = ((color&0x0000FF00) >> 8) ;
            b = (color&0x000000FF);
#else
            b = ((color&0xFF000000) >> 24) ;
            g = ((color&0x00FF0000) >> 16) ;
            r = ((color&0x0000FF00) >> 8) ;
            a = (color&0x000000FF);
#endif
            if ( hasMask )
            {
                if ( *maskp++ == 0xFF )
                {
                    r = MASK_RED ;
                    g = MASK_GREEN ;
                    b = MASK_BLUE ;
                }
                else if ( r == MASK_RED && g == MASK_GREEN && b == MASK_BLUE )
                    b = MASK_BLUE_REPLACEMENT ;
            }
            else if ( hasAlpha )
            {
                *alpha++ = a ;
#if wxOSX_USE_PREMULTIPLIED_ALPHA
                                if ( a != 0xFF && a!= 0 )
                {
                    r = r * 255 / a;
                    g = g * 255 / a;
                    b = b * 255 / a;
                }
#endif
            }

            data[index    ] = r ;
            data[index + 1] = g ;
            data[index + 2] = b ;

            index += 3;
        }
    }

    if ( hasMask )
        image.SetMaskColour( MASK_RED, MASK_GREEN, MASK_BLUE );

    return image;
}

#endif 
bool wxBitmap::SaveFile( const wxString& filename,
    wxBitmapType type, const wxPalette *palette ) const
{
    bool success = false;
    wxBitmapHandler *handler = FindHandler(type);

    if ( handler )
    {
        success = handler->SaveFile(this, filename, type, palette);
    }
    else
    {
#if wxUSE_IMAGE
        wxImage image = ConvertToImage();
        success = image.SaveFile(filename, type);
#else
        wxLogWarning(wxT("no bitmap handler for type %d defined."), type);
#endif
    }

    return success;
}

int wxBitmap::GetHeight() const
{
   wxCHECK_MSG( IsOk(), -1, wxT("invalid bitmap") );

   return M_BITMAPDATA->GetHeight();
}

int wxBitmap::GetWidth() const
{
   wxCHECK_MSG( IsOk(), -1, wxT("invalid bitmap") );

   return M_BITMAPDATA->GetWidth() ;
}

double wxBitmap::GetScaleFactor() const
{
    wxCHECK_MSG( IsOk(), -1, wxT("invalid bitmap") );
    
    return M_BITMAPDATA->GetScaleFactor() ;
}

int wxBitmap::GetDepth() const
{
   wxCHECK_MSG( IsOk(), -1, wxT("invalid bitmap") );

   return M_BITMAPDATA->GetDepth();
}

wxMask *wxBitmap::GetMask() const
{
   wxCHECK_MSG( IsOk(), NULL, wxT("invalid bitmap") );

   return M_BITMAPDATA->m_bitmapMask;
}

bool wxBitmap::HasAlpha() const
{
   wxCHECK_MSG( IsOk(), false , wxT("invalid bitmap") );

   return M_BITMAPDATA->HasAlpha() ;
}

void wxBitmap::SetWidth(int w)
{
    AllocExclusive();
    M_BITMAPDATA->SetWidth(w);
}

void wxBitmap::SetHeight(int h)
{
    AllocExclusive();
    M_BITMAPDATA->SetHeight(h);
}

void wxBitmap::SetDepth(int d)
{
    AllocExclusive();
    M_BITMAPDATA->SetDepth(d);
}

void wxBitmap::SetOk(bool isOk)
{
    AllocExclusive();
    M_BITMAPDATA->SetOk(isOk);
}

#if wxUSE_PALETTE
wxPalette *wxBitmap::GetPalette() const
{
   wxCHECK_MSG( IsOk(), NULL, wxT("Invalid bitmap  GetPalette()") );

   return &M_BITMAPDATA->m_bitmapPalette;
}

void wxBitmap::SetPalette(const wxPalette& palette)
{
    AllocExclusive();
    M_BITMAPDATA->m_bitmapPalette = palette ;
}
#endif 
void wxBitmap::SetMask(wxMask *mask)
{
    AllocExclusive();
        delete M_BITMAPDATA->m_bitmapMask;

    M_BITMAPDATA->m_bitmapMask = mask ;
}

WXHBITMAP wxBitmap::GetHBITMAP(WXHBITMAP* mask) const
{
    wxUnusedVar(mask);

    return WXHBITMAP(M_BITMAPDATA->GetBitmapContext());
}


wxMask::wxMask()
{
    Init() ;
}

wxMask::wxMask(const wxMask &tocopy) : wxObject()
{
    Init();

    m_bytesPerRow = tocopy.m_bytesPerRow;
    m_width = tocopy.m_width;
    m_height = tocopy.m_height;

    size_t size = m_bytesPerRow * m_height;
    unsigned char* dest = (unsigned char*)m_memBuf.GetWriteBuf( size );
    unsigned char* source = (unsigned char*)tocopy.m_memBuf.GetData();
    memcpy( dest, source, size );
    m_memBuf.UngetWriteBuf( size ) ;
    RealizeNative() ;
}

wxMask::wxMask( const wxBitmap& bitmap, const wxColour& colour )
{
    Init() ;
    Create( bitmap, colour );
}

wxMask::wxMask( const wxBitmap& bitmap )
{
    Init() ;
    Create( bitmap );
}


wxMask::wxMask( const wxMemoryBuffer& data, int width , int height , int bytesPerRow )
{
    Init() ;
    Create( data, width , height , bytesPerRow );
}

wxMask::~wxMask()
{
    if ( m_maskBitmap )
    {
        CGContextRelease( (CGContextRef) m_maskBitmap );
        m_maskBitmap = NULL ;
    }
}

void wxMask::Init()
{
    m_width = m_height = m_bytesPerRow = 0 ;
    m_maskBitmap = NULL ;
}

void *wxMask::GetRawAccess() const
{
    return m_memBuf.GetData() ;
}


void wxMask::RealizeNative()
{
    if ( m_maskBitmap )
    {
        CGContextRelease( (CGContextRef) m_maskBitmap );
       m_maskBitmap = NULL ;
    }

    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceGray();
            
    m_maskBitmap = CGBitmapContextCreate((char*) m_memBuf.GetData(), m_width, m_height, 8, m_bytesPerRow, colorspace,
        kCGImageAlphaNone );
    CGColorSpaceRelease( colorspace );
    wxASSERT_MSG( m_maskBitmap , wxT("Unable to create CGBitmapContext context") ) ;
}


bool wxMask::Create(const wxMemoryBuffer& data,int width , int height , int bytesPerRow)
{
    m_memBuf = data ;
    m_width = width ;
    m_height = height ;
    m_bytesPerRow = bytesPerRow ;

    wxASSERT( data.GetDataLen() == (size_t)(height * bytesPerRow) ) ;

    RealizeNative() ;

    return true ;
}

bool wxMask::Create(const wxBitmap& bitmap)
{
    m_width = bitmap.GetWidth() ;
    m_height = bitmap.GetHeight() ;
    m_bytesPerRow = GetBestBytesPerRow( m_width * kMaskBytesPerPixel ) ;

    size_t size = m_bytesPerRow * m_height ;
    unsigned char * destdatabase = (unsigned char*) m_memBuf.GetWriteBuf( size ) ;
    wxASSERT( destdatabase != NULL ) ;
    
    if ( destdatabase )
    {
        memset( destdatabase , 0 , size ) ;
        unsigned char * srcdata = (unsigned char*) bitmap.GetRawAccess() ;

        for ( int y = 0 ; y < m_height ; ++y , destdatabase += m_bytesPerRow )
        {
            unsigned char *destdata = destdatabase ;
            unsigned char r, g, b;

            for ( int x = 0 ; x < m_width ; ++x )
            {
                srcdata++ ;
                r = *srcdata++ ;
                g = *srcdata++ ;
                b = *srcdata++ ;

                if ( ( r + g + b ) > 0x10 )
                    *destdata++ = 0xFF ;
                else
                    *destdata++ = 0x00 ;
            }
        }
    }

    m_memBuf.UngetWriteBuf( size ) ;
    RealizeNative() ;

    return true;
}

bool wxMask::Create(const wxBitmap& bitmap, const wxColour& colour)
{
    m_width = bitmap.GetWidth() ;
    m_height = bitmap.GetHeight() ;
    m_bytesPerRow = GetBestBytesPerRow( m_width * kMaskBytesPerPixel ) ;

    size_t size = m_bytesPerRow * m_height ;
    unsigned char * destdatabase = (unsigned char*) m_memBuf.GetWriteBuf( size ) ;
    wxASSERT( destdatabase != NULL ) ;
    if ( destdatabase != NULL)
    {
        memset( destdatabase , 0 , size ) ;
        unsigned char * srcdatabase = (unsigned char*) bitmap.GetRawAccess() ;
        size_t sourceBytesRow = bitmap.GetBitmapData()->GetBytesPerRow();

        for ( int y = 0 ; y < m_height ; ++y , srcdatabase+= sourceBytesRow, destdatabase += m_bytesPerRow)
        {
            unsigned char *srcdata = srcdatabase ;
            unsigned char *destdata = destdatabase ;
            unsigned char r, g, b;

            for ( int x = 0 ; x < m_width ; ++x )
            {
                srcdata++ ;
                r = *srcdata++ ;
                g = *srcdata++ ;
                b = *srcdata++ ;

                if ( colour == wxColour( r , g , b ) )
                    *destdata++ = 0xFF ;
                else
                    *destdata++ = 0x00 ;
            }
        }
    }
    m_memBuf.UngetWriteBuf( size ) ;
    RealizeNative() ;

    return true;
}

wxBitmap wxMask::GetBitmap() const
{
    wxBitmap bitmap(m_width, m_height, 1);
    unsigned char* dst = static_cast<unsigned char*>(bitmap.BeginRawAccess());
    const int dst_stride = bitmap.GetBitmapData()->GetBytesPerRow();
    const unsigned char* src = static_cast<unsigned char*>(GetRawAccess());
    for (int j = 0; j < m_height; j++, src += m_bytesPerRow, dst += dst_stride)
    {
        unsigned char* d = dst;
        for (int i = 0; i < m_width; i++)
        {
            const unsigned char byte = src[i];
            *d++ = 0xff;
            *d++ = byte;
            *d++ = byte;
            *d++ = byte;
        }
    }
    bitmap.EndRawAccess();
    return bitmap;
}

WXHBITMAP wxMask::GetHBITMAP() const
{
    return m_maskBitmap ;
}


class WXDLLEXPORT wxBundleResourceHandler: public wxBitmapHandler
{
    wxDECLARE_ABSTRACT_CLASS(wxBundleResourceHandler);
    
public:
    inline wxBundleResourceHandler()
    {
    }
    
    virtual bool LoadFile(wxBitmap *bitmap,
                          const wxString& name,
                          wxBitmapType type,
                          int desiredWidth,
                          int desiredHeight) wxOVERRIDE;
};

wxIMPLEMENT_ABSTRACT_CLASS(wxBundleResourceHandler, wxBitmapHandler);

class WXDLLEXPORT wxPNGResourceHandler: public wxBundleResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxPNGResourceHandler);

public:
    inline wxPNGResourceHandler()
    {
        SetName(wxT("PNG resource"));
        SetExtension("PNG");
        SetType(wxBITMAP_TYPE_PNG_RESOURCE);
    }
};

wxIMPLEMENT_DYNAMIC_CLASS(wxPNGResourceHandler, wxBundleResourceHandler);

class WXDLLEXPORT wxJPEGResourceHandler: public wxBundleResourceHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxJPEGResourceHandler);

public:
    inline wxJPEGResourceHandler()
    {
        SetName(wxT("JPEG resource"));
        SetExtension("JPEG");
        SetType(wxBITMAP_TYPE_JPEG_RESOURCE);
    }
};

wxIMPLEMENT_DYNAMIC_CLASS(wxJPEGResourceHandler, wxBundleResourceHandler);

bool wxBundleResourceHandler::LoadFile(wxBitmap *bitmap,
                                     const wxString& name,
                                     wxBitmapType WXUNUSED(type),
                                     int WXUNUSED(desiredWidth),
                                     int WXUNUSED(desiredHeight))
{
    wxString ext = GetExtension().Lower();
    wxCFStringRef resname(name);
    wxCFStringRef resname2x(name+"@2x");
    wxCFStringRef restype(ext);
    double scale = 1.0;
    
    wxCFRef<CFURLRef> imageURL;
    
    if ( wxOSXGetMainScreenContentScaleFactor() > 1.9 )
    {
        imageURL.reset(CFBundleCopyResourceURL(CFBundleGetMainBundle(), resname2x, restype, NULL));
        scale = 2.0;
    }
    
    if ( imageURL.get() == NULL )
    {
        imageURL.reset(CFBundleCopyResourceURL(CFBundleGetMainBundle(), resname, restype, NULL));
        scale = 1.0;
    }
    
    if ( imageURL.get() != NULL )
    {
                wxCFRef<CGDataProviderRef> provider(CGDataProviderCreateWithURL (imageURL) );
        CGImageRef image = NULL;
        
        if ( ext == "jpeg" )
            image = CGImageCreateWithJPEGDataProvider (provider, NULL, true,
                                                   kCGRenderingIntentDefault);
        else if ( ext == "png" )
            image = CGImageCreateWithPNGDataProvider (provider, NULL, true,
                                                       kCGRenderingIntentDefault);
        if ( image != NULL )
        {
            bitmap->Create(image,scale);
            CGImageRelease(image);
        }
    }
        
    return false ;
}


wxBitmap wxBitmapHelpers::NewFromPNGData(const void* data, size_t size)
{
    wxCFRef<CGDataProviderRef>
        provider(CGDataProviderCreateWithData(NULL, data, size, NULL) );
    wxCFRef<CGImageRef>
        image(CGImageCreateWithPNGDataProvider(provider, NULL, true,
                                                kCGRenderingIntentDefault));

    return wxBitmap(image);
}

void wxBitmap::InitStandardHandlers()
{
#if wxOSX_USE_COCOA_OR_CARBON
    AddHandler( new wxICONResourceHandler ) ;
#endif
    AddHandler( new wxPNGResourceHandler );
    AddHandler( new wxJPEGResourceHandler );
}


void *wxBitmap::GetRawData(wxPixelDataBase& data, int WXUNUSED(bpp))
{
    if ( !IsOk() )
                return NULL;

    data.m_width = GetWidth() ;
    data.m_height = GetHeight() ;
    data.m_stride = GetBitmapData()->GetBytesPerRow() ;

    return BeginRawAccess() ;
}

void wxBitmap::UngetRawData(wxPixelDataBase& WXUNUSED(dataBase))
{
    EndRawAccess() ;
}

void wxBitmap::UseAlpha()
{
            M_BITMAPDATA->UseAlpha( true );
}
