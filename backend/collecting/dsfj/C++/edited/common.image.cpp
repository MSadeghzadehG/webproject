
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_IMAGE

#include "wx/image.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/hash.h"
    #include "wx/utils.h"
    #include "wx/math.h"
    #include "wx/module.h"
    #include "wx/palette.h"
    #include "wx/intl.h"
    #include "wx/colour.h"
#endif

#include "wx/wfstream.h"
#include "wx/xpmdecod.h"

#include <string.h>

#define HAS_FILE_STREAMS (wxUSE_STREAMS && (wxUSE_FILE || wxUSE_FFILE))

#if HAS_FILE_STREAMS
    #if wxUSE_FFILE
        typedef wxFFileInputStream wxImageFileInputStream;
        typedef wxFFileOutputStream wxImageFileOutputStream;
    #elif wxUSE_FILE
        typedef wxFileInputStream wxImageFileInputStream;
        typedef wxFileOutputStream wxImageFileOutputStream;
    #endif #endif 
#if wxUSE_VARIANT
IMPLEMENT_VARIANT_OBJECT_EXPORTED_SHALLOWCMP(wxImage,WXDLLEXPORT)
#endif


wxList wxImage::sm_handlers;
wxImage wxNullImage;


class wxImageRefData: public wxObjectRefData
{
public:
    wxImageRefData();
    virtual ~wxImageRefData();

    int             m_width;
    int             m_height;
    wxBitmapType    m_type;
    unsigned char  *m_data;

    bool            m_hasMask;
    unsigned char   m_maskRed,m_maskGreen,m_maskBlue;

        unsigned char  *m_alpha;

    bool            m_ok;

        bool            m_static;

        bool            m_staticAlpha;

        int             m_loadFlags;
    static int      sm_defaultLoadFlags;

#if wxUSE_PALETTE
    wxPalette       m_palette;
#endif 
    wxArrayString   m_optionNames;
    wxArrayString   m_optionValues;

    wxDECLARE_NO_COPY_CLASS(wxImageRefData);
};

int wxImageRefData::sm_defaultLoadFlags = wxImage::Load_Verbose;

wxImageRefData::wxImageRefData()
{
    m_width = 0;
    m_height = 0;
    m_type = wxBITMAP_TYPE_INVALID;
    m_data =
    m_alpha = (unsigned char *) NULL;

    m_maskRed = 0;
    m_maskGreen = 0;
    m_maskBlue = 0;
    m_hasMask = false;

    m_ok = false;
    m_static =
    m_staticAlpha = false;

    m_loadFlags = sm_defaultLoadFlags;
}

wxImageRefData::~wxImageRefData()
{
    if ( !m_static )
        free( m_data );
    if ( !m_staticAlpha )
        free( m_alpha );
}



#define M_IMGDATA static_cast<wxImageRefData*>(m_refData)

wxIMPLEMENT_DYNAMIC_CLASS(wxImage, wxObject);

bool wxImage::Create(const char* const* xpmData)
{
#if wxUSE_XPM
    UnRef();

    wxXPMDecoder decoder;
    (*this) = decoder.ReadData(xpmData);
    return IsOk();
#else
    wxUnusedVar(xpmData);
    return false;
#endif
}

bool wxImage::Create( int width, int height, bool clear )
{
    UnRef();

    m_refData = new wxImageRefData();

    M_IMGDATA->m_data = (unsigned char *) malloc( width*height*3 );
    if (!M_IMGDATA->m_data)
    {
        UnRef();
        return false;
    }

    M_IMGDATA->m_width = width;
    M_IMGDATA->m_height = height;
    M_IMGDATA->m_ok = true;

    if (clear)
    {
        Clear();
    }

    return true;
}

bool wxImage::Create( int width, int height, unsigned char* data, bool static_data )
{
    UnRef();

    wxCHECK_MSG( data, false, wxT("NULL data in wxImage::Create") );

    m_refData = new wxImageRefData();

    M_IMGDATA->m_data = data;
    M_IMGDATA->m_width = width;
    M_IMGDATA->m_height = height;
    M_IMGDATA->m_ok = true;
    M_IMGDATA->m_static = static_data;

    return true;
}

bool wxImage::Create( int width, int height, unsigned char* data, unsigned char* alpha, bool static_data )
{
    UnRef();

    wxCHECK_MSG( data, false, wxT("NULL data in wxImage::Create") );

    m_refData = new wxImageRefData();

    M_IMGDATA->m_data = data;
    M_IMGDATA->m_alpha = alpha;
    M_IMGDATA->m_width = width;
    M_IMGDATA->m_height = height;
    M_IMGDATA->m_ok = true;
    M_IMGDATA->m_static = static_data;
    M_IMGDATA->m_staticAlpha = static_data;

    return true;
}

void wxImage::Destroy()
{
    UnRef();
}

void wxImage::Clear(unsigned char value)
{
    memset(M_IMGDATA->m_data, value, M_IMGDATA->m_width*M_IMGDATA->m_height*3);
}

wxObjectRefData* wxImage::CreateRefData() const
{
    return new wxImageRefData;
}

wxObjectRefData* wxImage::CloneRefData(const wxObjectRefData* that) const
{
    const wxImageRefData* refData = static_cast<const wxImageRefData*>(that);
    wxCHECK_MSG(refData->m_ok, NULL, wxT("invalid image") );

    wxImageRefData* refData_new = new wxImageRefData;
    refData_new->m_width = refData->m_width;
    refData_new->m_height = refData->m_height;
    refData_new->m_maskRed = refData->m_maskRed;
    refData_new->m_maskGreen = refData->m_maskGreen;
    refData_new->m_maskBlue = refData->m_maskBlue;
    refData_new->m_hasMask = refData->m_hasMask;
    refData_new->m_ok = true;
    unsigned size = unsigned(refData->m_width) * unsigned(refData->m_height);
    if (refData->m_alpha != NULL)
    {
        refData_new->m_alpha = (unsigned char*)malloc(size);
        memcpy(refData_new->m_alpha, refData->m_alpha, size);
    }
    size *= 3;
    refData_new->m_data = (unsigned char*)malloc(size);
    memcpy(refData_new->m_data, refData->m_data, size);
#if wxUSE_PALETTE
    refData_new->m_palette = refData->m_palette;
#endif
    refData_new->m_optionNames = refData->m_optionNames;
    refData_new->m_optionValues = refData->m_optionValues;
    return refData_new;
}

wxImage wxImage::MakeEmptyClone(int flags) const
{
    wxImage image;

    wxCHECK_MSG( IsOk(), image, wxS("invalid image") );

    long height = M_IMGDATA->m_height;
    long width  = M_IMGDATA->m_width;

    if ( flags & Clone_SwapOrientation )
        wxSwap( width, height );

    if ( !image.Create( width, height, false ) )
    {
        wxFAIL_MSG( wxS("unable to create image") );
        return image;
    }

    if ( M_IMGDATA->m_alpha )
    {
        image.SetAlpha();
        wxCHECK2_MSG( image.GetAlpha(), return wxImage(),
                      wxS("unable to create alpha channel") );
    }

    if ( M_IMGDATA->m_hasMask )
    {
        image.SetMaskColour( M_IMGDATA->m_maskRed,
                             M_IMGDATA->m_maskGreen,
                             M_IMGDATA->m_maskBlue );
    }

    return image;
}

wxImage wxImage::Copy() const
{
    wxImage image;

    wxCHECK_MSG( IsOk(), image, wxT("invalid image") );

    image.m_refData = CloneRefData(m_refData);

    return image;
}

wxImage wxImage::ShrinkBy( int xFactor , int yFactor ) const
{
    if( xFactor == 1 && yFactor == 1 )
        return *this;

    wxImage image;

    wxCHECK_MSG( IsOk(), image, wxT("invalid image") );

        wxCHECK_MSG( (xFactor > 0) && (yFactor > 0), image,
                 wxT("invalid new image size") );

    long old_height = M_IMGDATA->m_height,
         old_width  = M_IMGDATA->m_width;

    wxCHECK_MSG( (old_height > 0) && (old_width > 0), image,
                 wxT("invalid old image size") );

    long width = old_width / xFactor ;
    long height = old_height / yFactor ;

    image.Create( width, height, false );

    char unsigned *data = image.GetData();

    wxCHECK_MSG( data, image, wxT("unable to create image") );

    bool hasMask = false ;
    unsigned char maskRed = 0;
    unsigned char maskGreen = 0;
    unsigned char maskBlue = 0 ;

    const unsigned char *source_data = M_IMGDATA->m_data;
    unsigned char *target_data = data;
    const unsigned char *source_alpha = 0 ;
    unsigned char *target_alpha = 0 ;
    if (M_IMGDATA->m_hasMask)
    {
        hasMask = true ;
        maskRed = M_IMGDATA->m_maskRed;
        maskGreen = M_IMGDATA->m_maskGreen;
        maskBlue =M_IMGDATA->m_maskBlue ;

        image.SetMaskColour( M_IMGDATA->m_maskRed,
                             M_IMGDATA->m_maskGreen,
                             M_IMGDATA->m_maskBlue );
    }
    else
    {
        source_alpha = M_IMGDATA->m_alpha ;
        if ( source_alpha )
        {
            image.SetAlpha() ;
            target_alpha = image.GetAlpha() ;
        }
    }

    for (long y = 0; y < height; y++)
    {
        for (long x = 0; x < width; x++)
        {
            unsigned long avgRed = 0 ;
            unsigned long avgGreen = 0;
            unsigned long avgBlue = 0;
            unsigned long avgAlpha = 0 ;
            unsigned long counter = 0 ;
                        for ( int y1 = 0 ; y1 < yFactor ; ++y1 )
            {
                long y_offset = (y * yFactor + y1) * old_width;
                for ( int x1 = 0 ; x1 < xFactor ; ++x1 )
                {
                    const unsigned char *pixel = source_data + 3 * ( y_offset + x * xFactor + x1 ) ;
                    unsigned char red = pixel[0] ;
                    unsigned char green = pixel[1] ;
                    unsigned char blue = pixel[2] ;
                    unsigned char alpha = 255  ;
                    if ( source_alpha )
                        alpha = *(source_alpha + y_offset + x * xFactor + x1) ;
                    if ( !hasMask || red != maskRed || green != maskGreen || blue != maskBlue )
                    {
                        if ( alpha > 0 )
                        {
                            avgRed += red ;
                            avgGreen += green ;
                            avgBlue += blue ;
                        }
                        avgAlpha += alpha ;
                        counter++ ;
                    }
                }
            }
            if ( counter == 0 )
            {
                *(target_data++) = M_IMGDATA->m_maskRed ;
                *(target_data++) = M_IMGDATA->m_maskGreen ;
                *(target_data++) = M_IMGDATA->m_maskBlue ;
            }
            else
            {
                if ( source_alpha )
                    *(target_alpha++) = (unsigned char)(avgAlpha / counter ) ;
                *(target_data++) = (unsigned char)(avgRed / counter);
                *(target_data++) = (unsigned char)(avgGreen / counter);
                *(target_data++) = (unsigned char)(avgBlue / counter);
            }
        }
    }

        if ( HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_X) )
        image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X,
                (GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_X))/xFactor);
    if ( HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y) )
        image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y,
                (GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_Y))/yFactor);

    return image;
}

wxImage
wxImage::Scale( int width, int height, wxImageResizeQuality quality ) const
{
    wxImage image;

    wxCHECK_MSG( IsOk(), image, wxT("invalid image") );

        wxCHECK_MSG( (width > 0) && (height > 0), image,
                 wxT("invalid new image size") );

    long old_height = M_IMGDATA->m_height,
         old_width  = M_IMGDATA->m_width;
    wxCHECK_MSG( (old_height > 0) && (old_width > 0), image,
                 wxT("invalid old image size") );

            if ( old_width == width && old_height == height )
        return *this;

        switch ( quality )
    {
        case wxIMAGE_QUALITY_NEAREST:
            if ( old_width % width == 0 && old_width >= width &&
                old_height % height == 0 && old_height >= height )
            {
                return ShrinkBy( old_width / width , old_height / height );
            }

            image = ResampleNearest(width, height);
            break;

        case wxIMAGE_QUALITY_BILINEAR:
            image = ResampleBilinear(width, height);
            break;

        case wxIMAGE_QUALITY_BICUBIC:
            image = ResampleBicubic(width, height);
            break;

        case wxIMAGE_QUALITY_BOX_AVERAGE:
            image = ResampleBox(width, height);
            break;

        case wxIMAGE_QUALITY_HIGH:
            image = width < old_width && height < old_height
                        ? ResampleBox(width, height)
                        : ResampleBicubic(width, height);
            break;
    }

        if (M_IMGDATA->m_hasMask)
    {
        image.SetMaskColour( M_IMGDATA->m_maskRed,
                            M_IMGDATA->m_maskGreen,
                            M_IMGDATA->m_maskBlue );
    }

        if ( HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_X) )
        image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X,
                (GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_X)*width)/old_width);
    if ( HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y) )
        image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y,
                (GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_Y)*height)/old_height);

    return image;
}

wxImage wxImage::ResampleNearest(int width, int height) const
{
    wxImage image;
    image.Create( width, height, false );

    unsigned char *data = image.GetData();

    wxCHECK_MSG( data, image, wxT("unable to create image") );

    const unsigned char *source_data = M_IMGDATA->m_data;
    unsigned char *target_data = data;
    const unsigned char *source_alpha = 0 ;
    unsigned char *target_alpha = 0 ;

    if ( !M_IMGDATA->m_hasMask )
    {
        source_alpha = M_IMGDATA->m_alpha ;
        if ( source_alpha )
        {
            image.SetAlpha() ;
            target_alpha = image.GetAlpha() ;
        }
    }

    long old_height = M_IMGDATA->m_height,
         old_width  = M_IMGDATA->m_width;
    long x_delta = (old_width<<16) / width;
    long y_delta = (old_height<<16) / height;

    unsigned char* dest_pixel = target_data;

    long y = 0;
    for ( long j = 0; j < height; j++ )
    {
        const unsigned char* src_line = &source_data[(y>>16)*old_width*3];
        const unsigned char* src_alpha_line = source_alpha ? &source_alpha[(y>>16)*old_width] : 0 ;

        long x = 0;
        for ( long i = 0; i < width; i++ )
        {
            const unsigned char* src_pixel = &src_line[(x>>16)*3];
            const unsigned char* src_alpha_pixel = source_alpha ? &src_alpha_line[(x>>16)] : 0 ;
            dest_pixel[0] = src_pixel[0];
            dest_pixel[1] = src_pixel[1];
            dest_pixel[2] = src_pixel[2];
            dest_pixel += 3;
            if ( source_alpha )
                *(target_alpha++) = *src_alpha_pixel ;
            x += x_delta;
        }

        y += y_delta;
    }

    return image;
}

namespace
{

struct BoxPrecalc
{
    int boxStart;
    int boxEnd;
};

inline int BoxBetween(int value, int low, int high)
{
    return wxMax(wxMin(value, high), low);
}

void ResampleBoxPrecalc(wxVector<BoxPrecalc>& boxes, int oldDim)
{
    const int newDim = boxes.size();
    const double scale_factor_1 = double(oldDim) / newDim;
    const int scale_factor_2 = (int)(scale_factor_1 / 2);

    for ( int dst = 0; dst < newDim; ++dst )
    {
                const int src_p = int(dst * scale_factor_1);

        BoxPrecalc& precalc = boxes[dst];
        precalc.boxStart = BoxBetween(int(src_p - scale_factor_1/2.0 + 1),
                                      0, oldDim - 1);
        precalc.boxEnd = BoxBetween(wxMax(precalc.boxStart + 1,
                                          int(src_p + scale_factor_2)),
                                    0, oldDim - 1);
    }
}

} 
wxImage wxImage::ResampleBox(int width, int height) const
{
                
    wxImage ret_image(width, height, false);

    wxVector<BoxPrecalc> vPrecalcs(height);
    wxVector<BoxPrecalc> hPrecalcs(width);

    ResampleBoxPrecalc(vPrecalcs, M_IMGDATA->m_height);
    ResampleBoxPrecalc(hPrecalcs, M_IMGDATA->m_width);


    const unsigned char* src_data = M_IMGDATA->m_data;
    const unsigned char* src_alpha = M_IMGDATA->m_alpha;
    unsigned char* dst_data = ret_image.GetData();
    unsigned char* dst_alpha = NULL;

    if ( src_alpha )
    {
        ret_image.SetAlpha();
        dst_alpha = ret_image.GetAlpha();
    }

    int averaged_pixels, src_pixel_index;
    double sum_r, sum_g, sum_b, sum_a;

    for ( int y = 0; y < height; y++ )             {
                const BoxPrecalc& vPrecalc = vPrecalcs[y];

        for ( int x = 0; x < width; x++ )              {
                        const BoxPrecalc& hPrecalc = hPrecalcs[x];

                        averaged_pixels = 0;
            sum_r = sum_g = sum_b = sum_a = 0.0;

            for ( int j = vPrecalc.boxStart; j <= vPrecalc.boxEnd; ++j )
            {
                for ( int i = hPrecalc.boxStart; i <= hPrecalc.boxEnd; ++i )
                {
                                        src_pixel_index = j * M_IMGDATA->m_width + i;

                    sum_r += src_data[src_pixel_index * 3 + 0];
                    sum_g += src_data[src_pixel_index * 3 + 1];
                    sum_b += src_data[src_pixel_index * 3 + 2];
                    if ( src_alpha )
                        sum_a += src_alpha[src_pixel_index];

                    averaged_pixels++;
                }
            }

                        dst_data[0] = (unsigned char)(sum_r / averaged_pixels);
            dst_data[1] = (unsigned char)(sum_g / averaged_pixels);
            dst_data[2] = (unsigned char)(sum_b / averaged_pixels);
            dst_data += 3;
            if ( src_alpha )
                *dst_alpha++ = (unsigned char)(sum_a / averaged_pixels);
        }
    }

    return ret_image;
}

namespace
{

struct BilinearPrecalc
{
    int offset1;
    int offset2;
    double dd;
    double dd1;
};

void ResampleBilinearPrecalc(wxVector<BilinearPrecalc>& precalcs, int oldDim)
{
    const int newDim = precalcs.size();
    const double scale_factor = double(oldDim) / newDim;
    const int srcpixmax = oldDim - 1;

    for ( int dsty = 0; dsty < newDim; dsty++ )
    {
                double srcpix = double(dsty) * scale_factor;
        double srcpix1 = int(srcpix);
        double srcpix2 = srcpix1 == srcpixmax ? srcpix1 : srcpix1 + 1.0;

        BilinearPrecalc& precalc = precalcs[dsty];

        precalc.dd = srcpix - (int)srcpix;
        precalc.dd1 = 1.0 - precalc.dd;
        precalc.offset1 = srcpix1 < 0.0
                            ? 0
                            : srcpix1 > srcpixmax
                                ? srcpixmax
                                : (int)srcpix1;
        precalc.offset2 = srcpix2 < 0.0
                            ? 0
                            : srcpix2 > srcpixmax
                                ? srcpixmax
                                : (int)srcpix2;
    }
}

} 
wxImage wxImage::ResampleBilinear(int width, int height) const
{
        wxImage ret_image(width, height, false);
    const unsigned char* src_data = M_IMGDATA->m_data;
    const unsigned char* src_alpha = M_IMGDATA->m_alpha;
    unsigned char* dst_data = ret_image.GetData();
    unsigned char* dst_alpha = NULL;

    if ( src_alpha )
    {
        ret_image.SetAlpha();
        dst_alpha = ret_image.GetAlpha();
    }

    wxVector<BilinearPrecalc> vPrecalcs(height);
    wxVector<BilinearPrecalc> hPrecalcs(width);
    ResampleBilinearPrecalc(vPrecalcs, M_IMGDATA->m_height);
    ResampleBilinearPrecalc(hPrecalcs, M_IMGDATA->m_width);

            double r1, g1, b1, a1 = 0;
    double r2, g2, b2, a2 = 0;

    for ( int dsty = 0; dsty < height; dsty++ )
    {
                const BilinearPrecalc& vPrecalc = vPrecalcs[dsty];
        const int y_offset1 = vPrecalc.offset1;
        const int y_offset2 = vPrecalc.offset2;
        const double dy = vPrecalc.dd;
        const double dy1 = vPrecalc.dd1;


        for ( int dstx = 0; dstx < width; dstx++ )
        {
                        const BilinearPrecalc& hPrecalc = hPrecalcs[dstx];

            const int x_offset1 = hPrecalc.offset1;
            const int x_offset2 = hPrecalc.offset2;
            const double dx = hPrecalc.dd;
            const double dx1 = hPrecalc.dd1;

            int src_pixel_index00 = y_offset1 * M_IMGDATA->m_width + x_offset1;
            int src_pixel_index01 = y_offset1 * M_IMGDATA->m_width + x_offset2;
            int src_pixel_index10 = y_offset2 * M_IMGDATA->m_width + x_offset1;
            int src_pixel_index11 = y_offset2 * M_IMGDATA->m_width + x_offset2;

                        r1 = src_data[src_pixel_index00 * 3 + 0] * dx1 + src_data[src_pixel_index01 * 3 + 0] * dx;
            g1 = src_data[src_pixel_index00 * 3 + 1] * dx1 + src_data[src_pixel_index01 * 3 + 1] * dx;
            b1 = src_data[src_pixel_index00 * 3 + 2] * dx1 + src_data[src_pixel_index01 * 3 + 2] * dx;
            if ( src_alpha )
                a1 = src_alpha[src_pixel_index00] * dx1 + src_alpha[src_pixel_index01] * dx;

                        r2 = src_data[src_pixel_index10 * 3 + 0] * dx1 + src_data[src_pixel_index11 * 3 + 0] * dx;
            g2 = src_data[src_pixel_index10 * 3 + 1] * dx1 + src_data[src_pixel_index11 * 3 + 1] * dx;
            b2 = src_data[src_pixel_index10 * 3 + 2] * dx1 + src_data[src_pixel_index11 * 3 + 2] * dx;
            if ( src_alpha )
                a2 = src_alpha[src_pixel_index10] * dx1 + src_alpha[src_pixel_index11] * dx;

            
            dst_data[0] = static_cast<unsigned char>(r1 * dy1 + r2 * dy + .5);
            dst_data[1] = static_cast<unsigned char>(g1 * dy1 + g2 * dy + .5);
            dst_data[2] = static_cast<unsigned char>(b1 * dy1 + b2 * dy + .5);
            dst_data += 3;

            if ( src_alpha )
                *dst_alpha++ = static_cast<unsigned char>(a1 * dy1 + a2 * dy);
        }
    }

    return ret_image;
}

static inline double spline_cube(double value)
{
    return value <= 0.0 ? 0.0 : value * value * value;
}

static inline double spline_weight(double value)
{
    return (spline_cube(value + 2) -
            4 * spline_cube(value + 1) +
            6 * spline_cube(value) -
            4 * spline_cube(value - 1)) / 6;
}


namespace
{

struct BicubicPrecalc
{
    double weight[4];
    int offset[4];
};

void ResampleBicubicPrecalc(wxVector<BicubicPrecalc> &aWeight, int oldDim)
{
    const int newDim = aWeight.size();
    for ( int dstd = 0; dstd < newDim; dstd++ )
    {
                const double srcpixd = static_cast<double>(dstd * oldDim) / newDim;
        const double dd = srcpixd - static_cast<int>(srcpixd);

        BicubicPrecalc &precalc = aWeight[dstd];

        for ( int k = -1; k <= 2; k++ )
        {
            precalc.offset[k + 1] = srcpixd + k < 0.0
                ? 0
                : srcpixd + k >= oldDim
                    ? oldDim - 1
                    : static_cast<int>(srcpixd + k);

            precalc.weight[k + 1] = spline_weight(k - dd);
        }
    }
}

} 
wxImage wxImage::ResampleBicubic(int width, int height) const
{
                                                
                                                
    wxImage ret_image;

    ret_image.Create(width, height, false);

    const unsigned char* src_data = M_IMGDATA->m_data;
    const unsigned char* src_alpha = M_IMGDATA->m_alpha;
    unsigned char* dst_data = ret_image.GetData();
    unsigned char* dst_alpha = NULL;

    if ( src_alpha )
    {
        ret_image.SetAlpha();
        dst_alpha = ret_image.GetAlpha();
    }

        wxVector<BicubicPrecalc> vPrecalcs(height);
    wxVector<BicubicPrecalc> hPrecalcs(width);

    ResampleBicubicPrecalc(vPrecalcs, M_IMGDATA->m_height);
    ResampleBicubicPrecalc(hPrecalcs, M_IMGDATA->m_width);

    for ( int dsty = 0; dsty < height; dsty++ )
    {
                const BicubicPrecalc& vPrecalc = vPrecalcs[dsty];

        for ( int dstx = 0; dstx < width; dstx++ )
        {
                        const BicubicPrecalc& hPrecalc = hPrecalcs[dstx];

                        double sum_r = 0, sum_g = 0, sum_b = 0, sum_a = 0;

                        for ( int k = -1; k <= 2; k++ )
            {
                                const int y_offset = vPrecalc.offset[k + 1];

                                for ( int i = -1; i <= 2; i++ )
                {
                                        const int x_offset = hPrecalc.offset[i + 1];

                                                            int src_pixel_index = y_offset*M_IMGDATA->m_width + x_offset;

                                                                                const double
                        pixel_weight = vPrecalc.weight[k + 1] * hPrecalc.weight[i + 1];

                                                            sum_r += src_data[src_pixel_index * 3 + 0] * pixel_weight;
                    sum_g += src_data[src_pixel_index * 3 + 1] * pixel_weight;
                    sum_b += src_data[src_pixel_index * 3 + 2] * pixel_weight;
                    if ( src_alpha )
                        sum_a += src_alpha[src_pixel_index] * pixel_weight;
                }
            }

                                    dst_data[0] = (unsigned char)(sum_r + 0.5);
            dst_data[1] = (unsigned char)(sum_g + 0.5);
            dst_data[2] = (unsigned char)(sum_b + 0.5);
            dst_data += 3;

            if ( src_alpha )
                *dst_alpha++ = (unsigned char)sum_a;
        }
    }

    return ret_image;
}

wxImage wxImage::BlurHorizontal(int blurRadius) const
{
    wxImage ret_image(MakeEmptyClone());

    wxCHECK( ret_image.IsOk(), ret_image );

    const unsigned char* src_data = M_IMGDATA->m_data;
    unsigned char* dst_data = ret_image.GetData();
    const unsigned char* src_alpha = M_IMGDATA->m_alpha;
    unsigned char* dst_alpha = ret_image.GetAlpha();

        const int blurArea = blurRadius*2 + 1;

            for ( int y = 0; y < M_IMGDATA->m_height; y++ )
    {
                long sum_r = 0,
             sum_g = 0,
             sum_b = 0,
             sum_a = 0;

        long pixel_idx;
        const unsigned char *src;
        unsigned char *dst;

                        for ( int kernel_x = -blurRadius; kernel_x <= blurRadius; kernel_x++ )
        {
                                                if ( kernel_x < 0 )
                pixel_idx = y * M_IMGDATA->m_width;
            else
                pixel_idx = kernel_x + y * M_IMGDATA->m_width;

            src = src_data + pixel_idx*3;
            sum_r += src[0];
            sum_g += src[1];
            sum_b += src[2];
            if ( src_alpha )
                sum_a += src_alpha[pixel_idx];
        }

        dst = dst_data + y * M_IMGDATA->m_width*3;
        dst[0] = (unsigned char)(sum_r / blurArea);
        dst[1] = (unsigned char)(sum_g / blurArea);
        dst[2] = (unsigned char)(sum_b / blurArea);
        if ( src_alpha )
            dst_alpha[y * M_IMGDATA->m_width] = (unsigned char)(sum_a / blurArea);

                        for ( int x = 1; x < M_IMGDATA->m_width; x++ )
        {
                                    if ( x - blurRadius - 1 < 0 )
                pixel_idx = y * M_IMGDATA->m_width;
            else
                pixel_idx = (x - blurRadius - 1) + y * M_IMGDATA->m_width;

                                    src = src_data + pixel_idx*3;
            sum_r -= src[0];
            sum_g -= src[1];
            sum_b -= src[2];
            if ( src_alpha )
                sum_a -= src_alpha[pixel_idx];

                        if ( x + blurRadius > M_IMGDATA->m_width - 1 )
                pixel_idx = M_IMGDATA->m_width - 1 + y * M_IMGDATA->m_width;
            else
                pixel_idx = x + blurRadius + y * M_IMGDATA->m_width;

                        src = src_data + pixel_idx*3;
            sum_r += src[0];
            sum_g += src[1];
            sum_b += src[2];
            if ( src_alpha )
                sum_a += src_alpha[pixel_idx];

                        dst = dst_data + x*3 + y*M_IMGDATA->m_width*3;
            dst[0] = (unsigned char)(sum_r / blurArea);
            dst[1] = (unsigned char)(sum_g / blurArea);
            dst[2] = (unsigned char)(sum_b / blurArea);
            if ( src_alpha )
                dst_alpha[x + y * M_IMGDATA->m_width] = (unsigned char)(sum_a / blurArea);
        }
    }

    return ret_image;
}

wxImage wxImage::BlurVertical(int blurRadius) const
{
    wxImage ret_image(MakeEmptyClone());

    wxCHECK( ret_image.IsOk(), ret_image );

    const unsigned char* src_data = M_IMGDATA->m_data;
    unsigned char* dst_data = ret_image.GetData();
    const unsigned char* src_alpha = M_IMGDATA->m_alpha;
    unsigned char* dst_alpha = ret_image.GetAlpha();

        const int blurArea = blurRadius*2 + 1;

            for ( int x = 0; x < M_IMGDATA->m_width; x++ )
    {
                long sum_r = 0,
             sum_g = 0,
             sum_b = 0,
             sum_a = 0;

        long pixel_idx;
        const unsigned char *src;
        unsigned char *dst;

                        for ( int kernel_y = -blurRadius; kernel_y <= blurRadius; kernel_y++ )
        {
                                                if ( kernel_y < 0 )
                pixel_idx = x;
            else
                pixel_idx = x + kernel_y * M_IMGDATA->m_width;

            src = src_data + pixel_idx*3;
            sum_r += src[0];
            sum_g += src[1];
            sum_b += src[2];
            if ( src_alpha )
                sum_a += src_alpha[pixel_idx];
        }

        dst = dst_data + x*3;
        dst[0] = (unsigned char)(sum_r / blurArea);
        dst[1] = (unsigned char)(sum_g / blurArea);
        dst[2] = (unsigned char)(sum_b / blurArea);
        if ( src_alpha )
            dst_alpha[x] = (unsigned char)(sum_a / blurArea);

                        for ( int y = 1; y < M_IMGDATA->m_height; y++ )
        {
                                    if ( y - blurRadius - 1 < 0 )
                pixel_idx = x;
            else
                pixel_idx = x + (y - blurRadius - 1) * M_IMGDATA->m_width;

                        src = src_data + pixel_idx*3;
            sum_r -= src[0];
            sum_g -= src[1];
            sum_b -= src[2];
            if ( src_alpha )
                sum_a -= src_alpha[pixel_idx];

                                    if ( y + blurRadius > M_IMGDATA->m_height - 1 )
                pixel_idx = x + (M_IMGDATA->m_height - 1) * M_IMGDATA->m_width;
            else
                pixel_idx = x + (blurRadius + y) * M_IMGDATA->m_width;

                        src = src_data + pixel_idx*3;
            sum_r += src[0];
            sum_g += src[1];
            sum_b += src[2];
            if ( src_alpha )
                sum_a += src_alpha[pixel_idx];

                        dst = dst_data + (x + y * M_IMGDATA->m_width) * 3;
            dst[0] = (unsigned char)(sum_r / blurArea);
            dst[1] = (unsigned char)(sum_g / blurArea);
            dst[2] = (unsigned char)(sum_b / blurArea);
            if ( src_alpha )
                dst_alpha[x + y * M_IMGDATA->m_width] = (unsigned char)(sum_a / blurArea);
        }
    }

    return ret_image;
}

wxImage wxImage::Blur(int blurRadius) const
{
    wxImage ret_image;
    ret_image.Create(M_IMGDATA->m_width, M_IMGDATA->m_height, false);

        ret_image = BlurHorizontal(blurRadius);
    ret_image = ret_image.BlurVertical(blurRadius);

    return ret_image;
}

wxImage wxImage::Rotate90( bool clockwise ) const
{
    wxImage image(MakeEmptyClone(Clone_SwapOrientation));

    wxCHECK( image.IsOk(), image );

    long height = M_IMGDATA->m_height;
    long width  = M_IMGDATA->m_width;

    if ( HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_X) )
    {
        int hot_x = GetOptionInt( wxIMAGE_OPTION_CUR_HOTSPOT_X );
        image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y,
                        clockwise ? hot_x : width - 1 - hot_x);
    }

    if ( HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y) )
    {
        int hot_y = GetOptionInt( wxIMAGE_OPTION_CUR_HOTSPOT_Y );
        image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X,
                        clockwise ? height - 1 - hot_y : hot_y);
    }

    unsigned char *data = image.GetData();
    unsigned char *target_data;

                    for (long ii = 0; ii < width; )
    {
        long next_ii = wxMin(ii + 21, width);

        for (long j = 0; j < height; j++)
        {
            const unsigned char *source_data
                                     = M_IMGDATA->m_data + (j*width + ii)*3;

            for (long i = ii; i < next_ii; i++)
            {
                if ( clockwise )
                {
                    target_data = data + ((i + 1)*height - j - 1)*3;
                }
                else
                {
                    target_data = data + (height*(width - 1 - i) + j)*3;
                }
                memcpy( target_data, source_data, 3 );
                source_data += 3;
            }
        }

        ii = next_ii;
    }

    const unsigned char *source_alpha = M_IMGDATA->m_alpha;

    if ( source_alpha )
    {
        unsigned char *alpha_data = image.GetAlpha();
        unsigned char *target_alpha = 0 ;

        for (long ii = 0; ii < width; )
        {
            long next_ii = wxMin(ii + 64, width);

            for (long j = 0; j < height; j++)
            {
                source_alpha = M_IMGDATA->m_alpha + j*width + ii;

                for (long i = ii; i < next_ii; i++)
                {
                    if ( clockwise )
                    {
                        target_alpha = alpha_data + (i+1)*height - j - 1;
                    }
                    else
                    {
                        target_alpha = alpha_data + height*(width - i - 1) + j;
                    }

                    *target_alpha = *source_alpha++;
                }
            }

            ii = next_ii;
        }
    }

    return image;
}

wxImage wxImage::Rotate180() const
{
    wxImage image(MakeEmptyClone());

    wxCHECK( image.IsOk(), image );

    long height = M_IMGDATA->m_height;
    long width  = M_IMGDATA->m_width;

    if ( HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_X) )
    {
        image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X,
                        width - 1 - GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_X));
    }

    if ( HasOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y) )
    {
        image.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y,
                        height - 1 - GetOptionInt(wxIMAGE_OPTION_CUR_HOTSPOT_Y));
    }

    unsigned char *data = image.GetData();
    unsigned char *alpha = image.GetAlpha();
    const unsigned char *source_data = M_IMGDATA->m_data;
    unsigned char *target_data = data + width * height * 3;

    for (long j = 0; j < height; j++)
    {
        for (long i = 0; i < width; i++)
        {
            target_data -= 3;
            memcpy( target_data, source_data, 3 );
            source_data += 3;
        }
    }

    if ( alpha )
    {
        const unsigned char *src_alpha = M_IMGDATA->m_alpha;
        unsigned char *dest_alpha = alpha + width * height;

        for (long j = 0; j < height; ++j)
        {
            for (long i = 0; i < width; ++i)
            {
                *(--dest_alpha) = *(src_alpha++);
            }
        }
    }

    return image;
}

wxImage wxImage::Mirror( bool horizontally ) const
{
    wxImage image(MakeEmptyClone());

    wxCHECK( image.IsOk(), image );

    long height = M_IMGDATA->m_height;
    long width  = M_IMGDATA->m_width;

    unsigned char *data = image.GetData();
    unsigned char *alpha = image.GetAlpha();
    const unsigned char *source_data = M_IMGDATA->m_data;
    unsigned char *target_data;

    if (horizontally)
    {
        for (long j = 0; j < height; j++)
        {
            data += width*3;
            target_data = data-3;
            for (long i = 0; i < width; i++)
            {
                memcpy( target_data, source_data, 3 );
                source_data += 3;
                target_data -= 3;
            }
        }

        if (alpha != NULL)
        {
                                    const unsigned char *src_alpha = M_IMGDATA->m_alpha;
                                                unsigned char *dest_alpha = alpha + width;

            for (long jj = 0; jj < height; ++jj)
            {
                for (long i = 0; i < width; ++i) {
                    *(--dest_alpha) = *(src_alpha++);                 }
                dest_alpha += 2 * width;             }
        }
    }
    else
    {
        for (long i = 0; i < height; i++)
        {
            target_data = data + 3*width*(height-1-i);
            memcpy( target_data, source_data, (size_t)3*width );
            source_data += 3*width;
        }

        if ( alpha )
        {
                                    const unsigned char *src_alpha = M_IMGDATA->m_alpha;
                                    unsigned char *dest_alpha = alpha + width * height;

            for (long jj = 0; jj < height; ++jj)
            {
                dest_alpha -= width;
                memcpy( dest_alpha, src_alpha, (size_t)width );
                src_alpha += width;
            }
        }
    }

    return image;
}

wxImage wxImage::GetSubImage( const wxRect &rect ) const
{
    wxImage image;

    wxCHECK_MSG( IsOk(), image, wxT("invalid image") );

    wxCHECK_MSG( (rect.GetLeft()>=0) && (rect.GetTop()>=0) &&
                 (rect.GetRight()<=GetWidth()) && (rect.GetBottom()<=GetHeight()),
                 image, wxT("invalid subimage size") );

    const int subwidth = rect.GetWidth();
    const int subheight = rect.GetHeight();

    image.Create( subwidth, subheight, false );

    const unsigned char *src_data = GetData();
    const unsigned char *src_alpha = M_IMGDATA->m_alpha;
    unsigned char *subdata = image.GetData();
    unsigned char *subalpha = NULL;

    wxCHECK_MSG( subdata, image, wxT("unable to create image") );

    if ( src_alpha ) {
        image.SetAlpha();
        subalpha = image.GetAlpha();
        wxCHECK_MSG( subalpha, image, wxT("unable to create alpha channel"));
    }

    if (M_IMGDATA->m_hasMask)
        image.SetMaskColour( M_IMGDATA->m_maskRed, M_IMGDATA->m_maskGreen, M_IMGDATA->m_maskBlue );

    const int width = GetWidth();
    const int pixsoff = rect.GetLeft() + width * rect.GetTop();

    src_data += 3 * pixsoff;
    src_alpha += pixsoff; 
    for (long j = 0; j < subheight; ++j)
    {
        memcpy( subdata, src_data, 3 * subwidth );
        subdata += 3 * subwidth;
        src_data += 3 * width;
        if (subalpha != NULL) {
            memcpy( subalpha, src_alpha, subwidth );
            subalpha += subwidth;
            src_alpha += width;
        }
    }

    return image;
}

wxImage wxImage::Size( const wxSize& size, const wxPoint& pos,
                       int r_, int g_, int b_ ) const
{
    wxImage image;

    wxCHECK_MSG( IsOk(), image, wxT("invalid image") );
    wxCHECK_MSG( (size.GetWidth() > 0) && (size.GetHeight() > 0), image, wxT("invalid size") );

    int width = GetWidth(), height = GetHeight();
    image.Create(size.GetWidth(), size.GetHeight(), false);

    unsigned char r = (unsigned char)r_;
    unsigned char g = (unsigned char)g_;
    unsigned char b = (unsigned char)b_;
    if ((r_ == -1) && (g_ == -1) && (b_ == -1))
    {
        GetOrFindMaskColour( &r, &g, &b );
        image.SetMaskColour(r, g, b);
    }

    image.SetRGB(wxRect(), r, g, b);

                            
        wxRect srcRect(0, 0, width, height);
    wxRect dstRect(-pos, size);

    srcRect.Intersect(dstRect);

    if (!srcRect.IsEmpty())
    {
                        wxPoint ptInsert = srcRect.GetTopLeft() + pos;

        if ((srcRect.GetWidth() == width) && (srcRect.GetHeight() == height))
            image.Paste(*this, ptInsert.x, ptInsert.y);
        else
            image.Paste(GetSubImage(srcRect), ptInsert.x, ptInsert.y);
    }

    return image;
}

void wxImage::Paste( const wxImage &image, int x, int y )
{
    wxCHECK_RET( IsOk(), wxT("invalid image") );
    wxCHECK_RET( image.IsOk(), wxT("invalid image") );

    AllocExclusive();

    int xx = 0;
    int yy = 0;
    int width = image.GetWidth();
    int height = image.GetHeight();

    if (x < 0)
    {
        xx = -x;
        width += x;
    }
    if (y < 0)
    {
        yy = -y;
        height += y;
    }

    if ((x+xx)+width > M_IMGDATA->m_width)
        width = M_IMGDATA->m_width - (x+xx);
    if ((y+yy)+height > M_IMGDATA->m_height)
        height = M_IMGDATA->m_height - (y+yy);

    if (width < 1) return;
    if (height < 1) return;

                    if ( !image.HasMask() ||
        ((HasMask() &&
         (GetMaskRed()==image.GetMaskRed()) &&
         (GetMaskGreen()==image.GetMaskGreen()) &&
         (GetMaskBlue()==image.GetMaskBlue()))) )
    {
        const unsigned char* source_data = image.GetData() + 3*(xx + yy*image.GetWidth());
        int source_step = image.GetWidth()*3;

        unsigned char* target_data = GetData() + 3*((x+xx) + (y+yy)*M_IMGDATA->m_width);
        int target_step = M_IMGDATA->m_width*3;
        for (int j = 0; j < height; j++)
        {
            memcpy( target_data, source_data, width*3 );
            source_data += source_step;
            target_data += target_step;
        }
    }

        if ( image.HasAlpha() )
    {
        if ( !HasAlpha() )
            InitAlpha();

        const unsigned char* source_data = image.GetAlpha() + xx + yy*image.GetWidth();
        int source_step = image.GetWidth();

        unsigned char* target_data = GetAlpha() + (x+xx) + (y+yy)*M_IMGDATA->m_width;
        int target_step = M_IMGDATA->m_width;

        for (int j = 0; j < height; j++,
                                    source_data += source_step,
                                    target_data += target_step)
        {
            memcpy( target_data, source_data, width );
        }
    }

    if (!HasMask() && image.HasMask())
    {
        unsigned char r = image.GetMaskRed();
        unsigned char g = image.GetMaskGreen();
        unsigned char b = image.GetMaskBlue();

        const unsigned char* source_data = image.GetData() + 3*(xx + yy*image.GetWidth());
        int source_step = image.GetWidth()*3;

        unsigned char* target_data = GetData() + 3*((x+xx) + (y+yy)*M_IMGDATA->m_width);
        int target_step = M_IMGDATA->m_width*3;

        for (int j = 0; j < height; j++)
        {
            for (int i = 0; i < width*3; i+=3)
            {
                if ((source_data[i]   != r) ||
                    (source_data[i+1] != g) ||
                    (source_data[i+2] != b))
                {
                    memcpy( target_data+i, source_data+i, 3 );
                }
            }
            source_data += source_step;
            target_data += target_step;
        }
    }
}

void wxImage::Replace( unsigned char r1, unsigned char g1, unsigned char b1,
                       unsigned char r2, unsigned char g2, unsigned char b2 )
{
    wxCHECK_RET( IsOk(), wxT("invalid image") );

    AllocExclusive();

    unsigned char *data = GetData();

    const int w = GetWidth();
    const int h = GetHeight();

    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
        {
            if ((data[0] == r1) && (data[1] == g1) && (data[2] == b1))
            {
                data[0] = r2;
                data[1] = g2;
                data[2] = b2;
            }
            data += 3;
        }
}

wxImage wxImage::ConvertToGreyscale(void) const
{
    return ConvertToGreyscale(0.299, 0.587, 0.114);
}

wxImage wxImage::ConvertToGreyscale(double weight_r, double weight_g, double weight_b) const
{
    wxImage image;
    wxCHECK_MSG(IsOk(), image, "invalid image");

    const int w = M_IMGDATA->m_width;
    const int h = M_IMGDATA->m_height;
    size_t size = size_t(w) * h;
    image.Create(w, h, false);
    const unsigned char* alpha = M_IMGDATA->m_alpha;
    if (alpha)
    {
        image.SetAlpha();
        memcpy(image.GetAlpha(), alpha, size);
    }
    const unsigned char mask_r = M_IMGDATA->m_maskRed;
    const unsigned char mask_g = M_IMGDATA->m_maskGreen;
    const unsigned char mask_b = M_IMGDATA->m_maskBlue;
    const bool hasMask = M_IMGDATA->m_hasMask;
    if (hasMask)
        image.SetMaskColour(mask_r, mask_g, mask_b);

    const unsigned char* src = M_IMGDATA->m_data;
    unsigned char* dst = image.GetData();
    while (size--)
    {
        unsigned char r = *src++;
        unsigned char g = *src++;
        unsigned char b = *src++;
        if (!hasMask || r != mask_r || g != mask_g || b != mask_b)
            wxColour::MakeGrey(&r, &g, &b, weight_r, weight_g, weight_b);
        *dst++ = r;
        *dst++ = g;
        *dst++ = b;
    }
    return image;
}

wxImage wxImage::ConvertToMono( unsigned char r, unsigned char g, unsigned char b ) const
{
    wxImage image;

    wxCHECK_MSG( IsOk(), image, wxT("invalid image") );

    image.Create( M_IMGDATA->m_width, M_IMGDATA->m_height, false );

    unsigned char *data = image.GetData();

    wxCHECK_MSG( data, image, wxT("unable to create image") );

    if (M_IMGDATA->m_hasMask)
    {
        if (M_IMGDATA->m_maskRed == r && M_IMGDATA->m_maskGreen == g &&
                                         M_IMGDATA->m_maskBlue == b)
            image.SetMaskColour( 255, 255, 255 );
        else
            image.SetMaskColour( 0, 0, 0 );
    }

    long size = M_IMGDATA->m_height * M_IMGDATA->m_width;

    unsigned char *srcd = M_IMGDATA->m_data;
    unsigned char *tard = image.GetData();

    for ( long i = 0; i < size; i++, srcd += 3, tard += 3 )
    {
        bool on = (srcd[0] == r) && (srcd[1] == g) && (srcd[2] == b);
        wxColourBase::MakeMono(tard + 0, tard + 1, tard + 2, on);
    }

    return image;
}

wxImage wxImage::ConvertToDisabled(unsigned char brightness) const
{
    wxImage image;
    wxCHECK_MSG(IsOk(), image, "invalid image");

    const int w = M_IMGDATA->m_width;
    const int h = M_IMGDATA->m_height;
    size_t size = size_t(w) * h;
    image.Create(w, h, false);
    const unsigned char* alpha = M_IMGDATA->m_alpha;
    if (alpha)
    {
        image.SetAlpha();
        memcpy(image.GetAlpha(), alpha, size);
    }
    const unsigned char mask_r = M_IMGDATA->m_maskRed;
    const unsigned char mask_g = M_IMGDATA->m_maskGreen;
    const unsigned char mask_b = M_IMGDATA->m_maskBlue;
    const bool hasMask = M_IMGDATA->m_hasMask;
    if (hasMask)
        image.SetMaskColour(mask_r, mask_g, mask_b);

    const unsigned char* src = M_IMGDATA->m_data;
    unsigned char* dst = image.GetData();
    while (size--)
    {
        unsigned char r = *src++;
        unsigned char g = *src++;
        unsigned char b = *src++;
        if (!hasMask || r != mask_r || g != mask_g || b != mask_b)
            wxColour::MakeDisabled(&r, &g, &b, brightness);
        *dst++ = r;
        *dst++ = g;
        *dst++ = b;
    }
    return image;
}

int wxImage::GetWidth() const
{
    wxCHECK_MSG( IsOk(), 0, wxT("invalid image") );

    return M_IMGDATA->m_width;
}

int wxImage::GetHeight() const
{
    wxCHECK_MSG( IsOk(), 0, wxT("invalid image") );

    return M_IMGDATA->m_height;
}

wxBitmapType wxImage::GetType() const
{
    wxCHECK_MSG( IsOk(), wxBITMAP_TYPE_INVALID, wxT("invalid image") );

    return M_IMGDATA->m_type;
}

void wxImage::SetType(wxBitmapType type)
{
    wxCHECK_RET( IsOk(), "must create the image before setting its type");

        wxASSERT_MSG( type != wxBITMAP_TYPE_MAX, "invalid bitmap type" );

    M_IMGDATA->m_type = type;
}

long wxImage::XYToIndex(int x, int y) const
{
    if ( IsOk() &&
            x >= 0 && y >= 0 &&
                x < M_IMGDATA->m_width && y < M_IMGDATA->m_height )
    {
        return y*M_IMGDATA->m_width + x;
    }

    return -1;
}

void wxImage::SetRGB( int x, int y, unsigned char r, unsigned char g, unsigned char b )
{
    long pos = XYToIndex(x, y);
    wxCHECK_RET( pos != -1, wxT("invalid image coordinates") );

    AllocExclusive();

    pos *= 3;

    M_IMGDATA->m_data[ pos   ] = r;
    M_IMGDATA->m_data[ pos+1 ] = g;
    M_IMGDATA->m_data[ pos+2 ] = b;
}

void wxImage::SetRGB( const wxRect& rect_, unsigned char r, unsigned char g, unsigned char b )
{
    wxCHECK_RET( IsOk(), wxT("invalid image") );

    AllocExclusive();

    wxRect rect(rect_);
    wxRect imageRect(0, 0, GetWidth(), GetHeight());
    if ( rect == wxRect() )
    {
        rect = imageRect;
    }
    else
    {
        wxCHECK_RET( imageRect.Contains(rect.GetTopLeft()) &&
                     imageRect.Contains(rect.GetBottomRight()),
                     wxT("invalid bounding rectangle") );
    }

    int x1 = rect.GetLeft(),
        y1 = rect.GetTop(),
        x2 = rect.GetRight() + 1,
        y2 = rect.GetBottom() + 1;

    unsigned char *data wxDUMMY_INITIALIZE(NULL);
    int x, y, width = GetWidth();
    for (y = y1; y < y2; y++)
    {
        data = M_IMGDATA->m_data + (y*width + x1)*3;
        for (x = x1; x < x2; x++)
        {
            *data++ = r;
            *data++ = g;
            *data++ = b;
        }
    }
}

unsigned char wxImage::GetRed( int x, int y ) const
{
    long pos = XYToIndex(x, y);
    wxCHECK_MSG( pos != -1, 0, wxT("invalid image coordinates") );

    pos *= 3;

    return M_IMGDATA->m_data[pos];
}

unsigned char wxImage::GetGreen( int x, int y ) const
{
    long pos = XYToIndex(x, y);
    wxCHECK_MSG( pos != -1, 0, wxT("invalid image coordinates") );

    pos *= 3;

    return M_IMGDATA->m_data[pos+1];
}

unsigned char wxImage::GetBlue( int x, int y ) const
{
    long pos = XYToIndex(x, y);
    wxCHECK_MSG( pos != -1, 0, wxT("invalid image coordinates") );

    pos *= 3;

    return M_IMGDATA->m_data[pos+2];
}

bool wxImage::IsOk() const
{
            wxImageRefData *data = M_IMGDATA;
    return data && data->m_ok && data->m_width && data->m_height;
}

unsigned char *wxImage::GetData() const
{
    wxCHECK_MSG( IsOk(), (unsigned char *)NULL, wxT("invalid image") );

    return M_IMGDATA->m_data;
}

void wxImage::SetData( unsigned char *data, bool static_data  )
{
    wxCHECK_RET( IsOk(), wxT("invalid image") );

    wxImageRefData *newRefData = new wxImageRefData();

    newRefData->m_width = M_IMGDATA->m_width;
    newRefData->m_height = M_IMGDATA->m_height;
    newRefData->m_data = data;
    newRefData->m_ok = true;
    newRefData->m_maskRed = M_IMGDATA->m_maskRed;
    newRefData->m_maskGreen = M_IMGDATA->m_maskGreen;
    newRefData->m_maskBlue = M_IMGDATA->m_maskBlue;
    newRefData->m_hasMask = M_IMGDATA->m_hasMask;
    newRefData->m_static = static_data;

    UnRef();

    m_refData = newRefData;
}

void wxImage::SetData( unsigned char *data, int new_width, int new_height, bool static_data )
{
    wxImageRefData *newRefData = new wxImageRefData();

    if (m_refData)
    {
        newRefData->m_width = new_width;
        newRefData->m_height = new_height;
        newRefData->m_data = data;
        newRefData->m_ok = true;
        newRefData->m_maskRed = M_IMGDATA->m_maskRed;
        newRefData->m_maskGreen = M_IMGDATA->m_maskGreen;
        newRefData->m_maskBlue = M_IMGDATA->m_maskBlue;
        newRefData->m_hasMask = M_IMGDATA->m_hasMask;
    }
    else
    {
        newRefData->m_width = new_width;
        newRefData->m_height = new_height;
        newRefData->m_data = data;
        newRefData->m_ok = true;
    }
    newRefData->m_static = static_data;

    UnRef();

    m_refData = newRefData;
}


void wxImage::SetAlpha(int x, int y, unsigned char alpha)
{
    wxCHECK_RET( HasAlpha(), wxT("no alpha channel") );

    long pos = XYToIndex(x, y);
    wxCHECK_RET( pos != -1, wxT("invalid image coordinates") );

    AllocExclusive();

    M_IMGDATA->m_alpha[pos] = alpha;
}

unsigned char wxImage::GetAlpha(int x, int y) const
{
    wxCHECK_MSG( HasAlpha(), 0, wxT("no alpha channel") );

    long pos = XYToIndex(x, y);
    wxCHECK_MSG( pos != -1, 0, wxT("invalid image coordinates") );

    return M_IMGDATA->m_alpha[pos];
}

bool
wxImage::ConvertColourToAlpha(unsigned char r, unsigned char g, unsigned char b)
{
    SetAlpha(NULL);

    const int w = M_IMGDATA->m_width;
    const int h = M_IMGDATA->m_height;

    unsigned char *alpha = GetAlpha();
    unsigned char *data = GetData();

    for ( int y = 0; y < h; y++ )
    {
        for ( int x = 0; x < w; x++ )
        {
            *alpha++ = *data;
            *data++ = r;
            *data++ = g;
            *data++ = b;
        }
    }

    return true;
}

void wxImage::SetAlpha( unsigned char *alpha, bool static_data )
{
    wxCHECK_RET( IsOk(), wxT("invalid image") );

    AllocExclusive();

    if ( !alpha )
    {
        alpha = (unsigned char *)malloc(M_IMGDATA->m_width*M_IMGDATA->m_height);
    }

    if( !M_IMGDATA->m_staticAlpha )
        free(M_IMGDATA->m_alpha);

    M_IMGDATA->m_alpha = alpha;
    M_IMGDATA->m_staticAlpha = static_data;
}

unsigned char *wxImage::GetAlpha() const
{
    wxCHECK_MSG( IsOk(), (unsigned char *)NULL, wxT("invalid image") );

    return M_IMGDATA->m_alpha;
}

void wxImage::InitAlpha()
{
    wxCHECK_RET( !HasAlpha(), wxT("image already has an alpha channel") );

        SetAlpha();

    unsigned char *alpha = M_IMGDATA->m_alpha;
    const size_t lenAlpha = M_IMGDATA->m_width * M_IMGDATA->m_height;

    if ( HasMask() )
    {
                const unsigned char * const alphaEnd = alpha + lenAlpha;

        const unsigned char mr = M_IMGDATA->m_maskRed;
        const unsigned char mg = M_IMGDATA->m_maskGreen;
        const unsigned char mb = M_IMGDATA->m_maskBlue;
        for ( unsigned char *src = M_IMGDATA->m_data;
              alpha < alphaEnd;
              src += 3, alpha++ )
        {
            *alpha = (src[0] == mr && src[1] == mg && src[2] == mb)
                            ? wxIMAGE_ALPHA_TRANSPARENT
                            : wxIMAGE_ALPHA_OPAQUE;
        }

        M_IMGDATA->m_hasMask = false;
    }
    else     {
                memset(alpha, wxIMAGE_ALPHA_OPAQUE, lenAlpha);
    }
}

void wxImage::ClearAlpha()
{
    wxCHECK_RET( HasAlpha(), wxT("image already doesn't have an alpha channel") );

    AllocExclusive();

    if ( !M_IMGDATA->m_staticAlpha )
        free( M_IMGDATA->m_alpha );

    M_IMGDATA->m_alpha = NULL;
}



void wxImage::SetMaskColour( unsigned char r, unsigned char g, unsigned char b )
{
    wxCHECK_RET( IsOk(), wxT("invalid image") );

    AllocExclusive();

    M_IMGDATA->m_maskRed = r;
    M_IMGDATA->m_maskGreen = g;
    M_IMGDATA->m_maskBlue = b;
    M_IMGDATA->m_hasMask = true;
}

bool wxImage::GetOrFindMaskColour( unsigned char *r, unsigned char *g, unsigned char *b ) const
{
    wxCHECK_MSG( IsOk(), false, wxT("invalid image") );

    if (M_IMGDATA->m_hasMask)
    {
        if (r) *r = M_IMGDATA->m_maskRed;
        if (g) *g = M_IMGDATA->m_maskGreen;
        if (b) *b = M_IMGDATA->m_maskBlue;
        return true;
    }
    else
    {
        FindFirstUnusedColour(r, g, b);
        return false;
    }
}

unsigned char wxImage::GetMaskRed() const
{
    wxCHECK_MSG( IsOk(), 0, wxT("invalid image") );

    return M_IMGDATA->m_maskRed;
}

unsigned char wxImage::GetMaskGreen() const
{
    wxCHECK_MSG( IsOk(), 0, wxT("invalid image") );

    return M_IMGDATA->m_maskGreen;
}

unsigned char wxImage::GetMaskBlue() const
{
    wxCHECK_MSG( IsOk(), 0, wxT("invalid image") );

    return M_IMGDATA->m_maskBlue;
}

void wxImage::SetMask( bool mask )
{
    wxCHECK_RET( IsOk(), wxT("invalid image") );

    AllocExclusive();

    M_IMGDATA->m_hasMask = mask;
}

bool wxImage::HasMask() const
{
    wxCHECK_MSG( IsOk(), false, wxT("invalid image") );

    return M_IMGDATA->m_hasMask;
}

bool wxImage::IsTransparent(int x, int y, unsigned char threshold) const
{
    long pos = XYToIndex(x, y);
    wxCHECK_MSG( pos != -1, false, wxT("invalid image coordinates") );

        if ( M_IMGDATA->m_hasMask )
    {
        const unsigned char *p = M_IMGDATA->m_data + 3*pos;
        if ( p[0] == M_IMGDATA->m_maskRed &&
                p[1] == M_IMGDATA->m_maskGreen &&
                    p[2] == M_IMGDATA->m_maskBlue )
        {
            return true;
        }
    }

        if ( M_IMGDATA->m_alpha )
    {
        if ( M_IMGDATA->m_alpha[pos] < threshold )
        {
                        return true;
        }
    }

        return false;
}

bool wxImage::SetMaskFromImage(const wxImage& mask,
                               unsigned char mr, unsigned char mg, unsigned char mb)
{
        if ( (M_IMGDATA->m_height != mask.GetHeight() ) || (M_IMGDATA->m_width != mask.GetWidth () ) )
    {
        wxLogError( _("Image and mask have different sizes.") );
        return false;
    }

        unsigned char r,g,b ;
    if (!FindFirstUnusedColour(&r, &g, &b))
    {
        wxLogError( _("No unused colour in image being masked.") );
        return false ;
    }

    AllocExclusive();

    unsigned char *imgdata = GetData();
    unsigned char *maskdata = mask.GetData();

    const int w = GetWidth();
    const int h = GetHeight();

    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            if ((maskdata[0] == mr) && (maskdata[1]  == mg) && (maskdata[2] == mb))
            {
                imgdata[0] = r;
                imgdata[1] = g;
                imgdata[2] = b;
            }
            imgdata  += 3;
            maskdata += 3;
        }
    }

    SetMaskColour(r, g, b);
    SetMask(true);

    return true;
}

bool wxImage::ConvertAlphaToMask(unsigned char threshold)
{
    if ( !HasAlpha() )
        return false;

    unsigned char mr, mg, mb;
    if ( !FindFirstUnusedColour(&mr, &mg, &mb) )
    {
        wxLogError( _("No unused colour in image being masked.") );
        return false;
    }

    return ConvertAlphaToMask(mr, mg, mb, threshold);
}

bool wxImage::ConvertAlphaToMask(unsigned char mr,
                                 unsigned char mg,
                                 unsigned char mb,
                                 unsigned char threshold)
{
    if ( !HasAlpha() )
        return false;

    AllocExclusive();

    SetMask(true);
    SetMaskColour(mr, mg, mb);

    unsigned char *imgdata = GetData();
    unsigned char *alphadata = GetAlpha();

    int w = GetWidth();
    int h = GetHeight();

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++, imgdata += 3, alphadata++)
        {
            if (*alphadata < threshold)
            {
                imgdata[0] = mr;
                imgdata[1] = mg;
                imgdata[2] = mb;
            }
        }
    }

    if ( !M_IMGDATA->m_staticAlpha )
        free(M_IMGDATA->m_alpha);

    M_IMGDATA->m_alpha = NULL;
    M_IMGDATA->m_staticAlpha = false;

    return true;
}


#if wxUSE_PALETTE

bool wxImage::HasPalette() const
{
    if (!IsOk())
        return false;

    return M_IMGDATA->m_palette.IsOk();
}

const wxPalette& wxImage::GetPalette() const
{
    wxCHECK_MSG( IsOk(), wxNullPalette, wxT("invalid image") );

    return M_IMGDATA->m_palette;
}

void wxImage::SetPalette(const wxPalette& palette)
{
    wxCHECK_RET( IsOk(), wxT("invalid image") );

    AllocExclusive();

    M_IMGDATA->m_palette = palette;
}

#endif 

void wxImage::SetOption(const wxString& name, const wxString& value)
{
    AllocExclusive();

    int idx = M_IMGDATA->m_optionNames.Index(name, false);
    if ( idx == wxNOT_FOUND )
    {
        M_IMGDATA->m_optionNames.Add(name);
        M_IMGDATA->m_optionValues.Add(value);
    }
    else
    {
        M_IMGDATA->m_optionNames[idx] = name;
        M_IMGDATA->m_optionValues[idx] = value;
    }
}

void wxImage::SetOption(const wxString& name, int value)
{
    wxString valStr;
    valStr.Printf(wxT("%d"), value);
    SetOption(name, valStr);
}

wxString wxImage::GetOption(const wxString& name) const
{
    if ( !M_IMGDATA )
        return wxEmptyString;

    int idx = M_IMGDATA->m_optionNames.Index(name, false);
    if ( idx == wxNOT_FOUND )
        return wxEmptyString;
    else
        return M_IMGDATA->m_optionValues[idx];
}

int wxImage::GetOptionInt(const wxString& name) const
{
    return wxAtoi(GetOption(name));
}

bool wxImage::HasOption(const wxString& name) const
{
    return M_IMGDATA ? M_IMGDATA->m_optionNames.Index(name, false) != wxNOT_FOUND
                     : false;
}



void wxImage::SetDefaultLoadFlags(int flags)
{
    wxImageRefData::sm_defaultLoadFlags = flags;
}


int wxImage::GetDefaultLoadFlags()
{
    return wxImageRefData::sm_defaultLoadFlags;
}

void wxImage::SetLoadFlags(int flags)
{
    AllocExclusive();

    M_IMGDATA->m_loadFlags = flags;
}

int wxImage::GetLoadFlags() const
{
    return M_IMGDATA ? M_IMGDATA->m_loadFlags : wxImageRefData::sm_defaultLoadFlags;
}

#if defined(__WINDOWS__) && wxUSE_WXDIB && wxUSE_IMAGE
    #define HAS_LOAD_FROM_RESOURCE
#endif

#ifdef HAS_LOAD_FROM_RESOURCE

#include "wx/msw/dib.h"
#include "wx/msw/private.h"

static wxImage LoadImageFromResource(const wxString &name, wxBitmapType type)
{
    AutoHBITMAP
        hBitmap,
        hMask;

    if ( type == wxBITMAP_TYPE_BMP_RESOURCE )
    {
        hBitmap.Init( ::LoadBitmap(wxGetInstance(), name.t_str()) );

        if ( !hBitmap )
        {
            wxLogError(_("Failed to load bitmap \"%s\" from resources."), name);
        }
    }
    else if ( type == wxBITMAP_TYPE_ICO_RESOURCE )
    {
        const HICON hIcon = ::LoadIcon(wxGetInstance(), name.t_str());

        if ( !hIcon )
        {
            wxLogError(_("Failed to load icon \"%s\" from resources."), name);
        }
        else
        {
            AutoIconInfo info;
            if ( !info.GetFrom(hIcon) )
                return wxImage();

            hBitmap.Init(info.hbmColor);
            hMask.Init(info.hbmMask);

                                    info.hbmColor =
            info.hbmMask = 0;
        }
    }
    else if ( type == wxBITMAP_TYPE_CUR_RESOURCE )
    {
        wxLogDebug(wxS("Loading cursors from resources is not implemented."));
    }
    else
    {
        wxFAIL_MSG(wxS("Invalid bitmap resource type."));
    }

    if ( !hBitmap )
        return wxImage();

    wxImage image = wxDIB(hBitmap).ConvertToImage();
    if ( hMask )
    {
        const wxImage mask = wxDIB(hMask).ConvertToImage();
        image.SetMaskFromImage(mask, 255, 255, 255);
    }
    else
    {
                image.SetMaskColour(0xc0, 0xc0, 0xc0);
    }

            if ( !image.HasAlpha() )
        image.InitAlpha();

    return image;
}

#endif 
bool wxImage::LoadFile( const wxString& filename,
                        wxBitmapType type,
                        int WXUNUSED_UNLESS_STREAMS(index) )
{
#ifdef HAS_LOAD_FROM_RESOURCE
    if (   type == wxBITMAP_TYPE_BMP_RESOURCE
        || type == wxBITMAP_TYPE_ICO_RESOURCE
        || type == wxBITMAP_TYPE_CUR_RESOURCE)
    {
        const wxImage image = ::LoadImageFromResource(filename, type);
        if ( image.IsOk() )
        {
            *this = image;
            return true;
        }
    }
#endif 
#if HAS_FILE_STREAMS
    wxImageFileInputStream stream(filename);
    if ( stream.IsOk() )
    {
        wxBufferedInputStream bstream( stream );
        if ( LoadFile(bstream, type, index) )
            return true;
    }

    wxLogError(_("Failed to load image from file \"%s\"."), filename);
#endif 
    return false;
}

bool wxImage::LoadFile( const wxString& WXUNUSED_UNLESS_STREAMS(filename),
                        const wxString& WXUNUSED_UNLESS_STREAMS(mimetype),
                        int WXUNUSED_UNLESS_STREAMS(index) )
{
#if HAS_FILE_STREAMS
    wxImageFileInputStream stream(filename);
    if ( stream.IsOk() )
    {
        wxBufferedInputStream bstream( stream );
        if ( LoadFile(bstream, mimetype, index) )
            return true;
    }

    wxLogError(_("Failed to load image from file \"%s\"."), filename);
#endif 
    return false;
}


bool wxImage::SaveFile( const wxString& filename ) const
{
    wxString ext = filename.AfterLast('.').Lower();

    wxImageHandler *handler = FindHandler(ext, wxBITMAP_TYPE_ANY);
    if ( !handler)
    {
       wxLogError(_("Can't save image to file '%s': unknown extension."),
                  filename);
       return false;
    }

    return SaveFile(filename, handler->GetType());
}

bool wxImage::SaveFile( const wxString& WXUNUSED_UNLESS_STREAMS(filename),
                        wxBitmapType WXUNUSED_UNLESS_STREAMS(type) ) const
{
#if HAS_FILE_STREAMS
    wxCHECK_MSG( IsOk(), false, wxT("invalid image") );

    ((wxImage*)this)->SetOption(wxIMAGE_OPTION_FILENAME, filename);

    wxImageFileOutputStream stream(filename);

    if ( stream.IsOk() )
    {
        wxBufferedOutputStream bstream( stream );
        return SaveFile(bstream, type);
    }
#endif 
    return false;
}

bool wxImage::SaveFile( const wxString& WXUNUSED_UNLESS_STREAMS(filename),
                        const wxString& WXUNUSED_UNLESS_STREAMS(mimetype) ) const
{
#if HAS_FILE_STREAMS
    wxCHECK_MSG( IsOk(), false, wxT("invalid image") );

    ((wxImage*)this)->SetOption(wxIMAGE_OPTION_FILENAME, filename);

    wxImageFileOutputStream stream(filename);

    if ( stream.IsOk() )
    {
        wxBufferedOutputStream bstream( stream );
        return SaveFile(bstream, mimetype);
    }
#endif 
    return false;
}

bool wxImage::CanRead( const wxString& WXUNUSED_UNLESS_STREAMS(name) )
{
#if HAS_FILE_STREAMS
    wxImageFileInputStream stream(name);
    return CanRead(stream);
#else
    return false;
#endif
}

int wxImage::GetImageCount( const wxString& WXUNUSED_UNLESS_STREAMS(name),
                            wxBitmapType WXUNUSED_UNLESS_STREAMS(type) )
{
#if HAS_FILE_STREAMS
    wxImageFileInputStream stream(name);
    if (stream.IsOk())
        return GetImageCount(stream, type);
#endif

  return 0;
}

#if wxUSE_STREAMS

bool wxImage::CanRead( wxInputStream &stream )
{
    const wxList& list = GetHandlers();

    for ( wxList::compatibility_iterator node = list.GetFirst(); node; node = node->GetNext() )
    {
        wxImageHandler *handler=(wxImageHandler*)node->GetData();
        if (handler->CanRead( stream ))
            return true;
    }

    return false;
}

int wxImage::GetImageCount( wxInputStream &stream, wxBitmapType type )
{
    wxImageHandler *handler;

    if ( type == wxBITMAP_TYPE_ANY )
    {
        const wxList& list = GetHandlers();

        for ( wxList::compatibility_iterator node = list.GetFirst();
              node;
              node = node->GetNext() )
        {
             handler = (wxImageHandler*)node->GetData();
             if ( handler->CanRead(stream) )
             {
                 const int count = handler->GetImageCount(stream);
                 if ( count >= 0 )
                     return count;
             }

        }

        wxLogWarning(_("No handler found for image type."));
        return 0;
    }

    handler = FindHandler(type);

    if ( !handler )
    {
        wxLogWarning(_("No image handler for type %d defined."), type);
        return false;
    }

    if ( handler->CanRead(stream) )
    {
        return handler->GetImageCount(stream);
    }
    else
    {
        wxLogError(_("Image file is not of type %d."), type);
        return 0;
    }
}

bool wxImage::DoLoad(wxImageHandler& handler, wxInputStream& stream, int index)
{
            const unsigned maxWidth = GetOptionInt(wxIMAGE_OPTION_MAX_WIDTH),
                   maxHeight = GetOptionInt(wxIMAGE_OPTION_MAX_HEIGHT);

                wxFileOffset posOld = wxInvalidOffset;
    if ( stream.IsSeekable() )
        posOld = stream.TellI();

    if ( !handler.LoadFile(this, stream,
                           (M_IMGDATA->m_loadFlags & Load_Verbose) != 0, index) )
    {
        if ( posOld != wxInvalidOffset )
            stream.SeekI(posOld);

        return false;
    }

        if ( maxWidth || maxHeight )
    {
        const unsigned widthOrig = GetWidth(),
                       heightOrig = GetHeight();

                unsigned width = widthOrig,
                 height = heightOrig;
        while ( (maxWidth && width > maxWidth) ||
                    (maxHeight && height > maxHeight) )
        {
            width /= 2;
            height /= 2;
        }

        if ( width != widthOrig || height != heightOrig )
        {
                                    int widthOrigOption = GetOptionInt(wxIMAGE_OPTION_ORIGINAL_WIDTH),
                heightOrigOption = GetOptionInt(wxIMAGE_OPTION_ORIGINAL_HEIGHT);

            Rescale(width, height, wxIMAGE_QUALITY_HIGH);

            SetOption(wxIMAGE_OPTION_ORIGINAL_WIDTH, widthOrigOption ? widthOrigOption : widthOrig);
            SetOption(wxIMAGE_OPTION_ORIGINAL_HEIGHT, heightOrigOption ? heightOrigOption : heightOrig);
        }
    }

        M_IMGDATA->m_type = handler.GetType();

    return true;
}

bool wxImage::LoadFile( wxInputStream& stream, wxBitmapType type, int index )
{
    AllocExclusive();

    wxImageHandler *handler;

    if ( type == wxBITMAP_TYPE_ANY )
    {
        if ( !stream.IsSeekable() )
        {
                                                            wxLogError(_("Can't automatically determine the image format "
                         "for non-seekable input."));
            return false;
        }

        const wxList& list = GetHandlers();
        for ( wxList::compatibility_iterator node = list.GetFirst();
              node;
              node = node->GetNext() )
        {
             handler = (wxImageHandler*)node->GetData();
             if ( handler->CanRead(stream) && DoLoad(*handler, stream, index) )
                 return true;
        }

        wxLogWarning( _("Unknown image data format.") );

        return false;
    }
    
    handler = FindHandler(type);
    if ( !handler )
    {
        wxLogWarning( _("No image handler for type %d defined."), type );
        return false;
    }

    if ( stream.IsSeekable() && !handler->CanRead(stream) )
    {
        wxLogError(_("This is not a %s."), handler->GetName());
        return false;
    }

    return DoLoad(*handler, stream, index);
}

bool wxImage::LoadFile( wxInputStream& stream, const wxString& mimetype, int index )
{
    UnRef();

    m_refData = new wxImageRefData;

    wxImageHandler *handler = FindHandlerMime(mimetype);

    if ( !handler )
    {
        wxLogWarning( _("No image handler for type %s defined."), mimetype.GetData() );
        return false;
    }

    if ( stream.IsSeekable() && !handler->CanRead(stream) )
    {
        wxLogError(_("Image is not of type %s."), mimetype);
        return false;
    }

    return DoLoad(*handler, stream, index);
}

bool wxImage::DoSave(wxImageHandler& handler, wxOutputStream& stream) const
{
    wxImage * const self = const_cast<wxImage *>(this);
    if ( !handler.SaveFile(self, stream) )
        return false;

    M_IMGDATA->m_type = handler.GetType();
    return true;
}

bool wxImage::SaveFile( wxOutputStream& stream, wxBitmapType type ) const
{
    wxCHECK_MSG( IsOk(), false, wxT("invalid image") );

    wxImageHandler *handler = FindHandler(type);
    if ( !handler )
    {
        wxLogWarning( _("No image handler for type %d defined."), type );
        return false;
    }

    return DoSave(*handler, stream);
}

bool wxImage::SaveFile( wxOutputStream& stream, const wxString& mimetype ) const
{
    wxCHECK_MSG( IsOk(), false, wxT("invalid image") );

    wxImageHandler *handler = FindHandlerMime(mimetype);
    if ( !handler )
    {
        wxLogWarning( _("No image handler for type %s defined."), mimetype.GetData() );
        return false;
    }

    return DoSave(*handler, stream);
}

#endif 

void wxImage::AddHandler( wxImageHandler *handler )
{
        if (FindHandler( handler->GetType() ) == 0)
    {
        sm_handlers.Append( handler );
    }
    else
    {
                                        
        wxLogDebug( wxT("Adding duplicate image handler for '%s'"),
                    handler->GetName().c_str() );
        delete handler;
    }
}

void wxImage::InsertHandler( wxImageHandler *handler )
{
        if (FindHandler( handler->GetType() ) == 0)
    {
        sm_handlers.Insert( handler );
    }
    else
    {
                wxLogDebug( wxT("Inserting duplicate image handler for '%s'"),
                    handler->GetName().c_str() );
        delete handler;
    }
}

bool wxImage::RemoveHandler( const wxString& name )
{
    wxImageHandler *handler = FindHandler(name);
    if (handler)
    {
        sm_handlers.DeleteObject(handler);
        delete handler;
        return true;
    }
    else
        return false;
}

wxImageHandler *wxImage::FindHandler( const wxString& name )
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while (node)
    {
        wxImageHandler *handler = (wxImageHandler*)node->GetData();
        if (handler->GetName().Cmp(name) == 0) return handler;

        node = node->GetNext();
    }
    return NULL;
}

wxImageHandler *wxImage::FindHandler( const wxString& extension, wxBitmapType bitmapType )
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while (node)
    {
        wxImageHandler *handler = (wxImageHandler*)node->GetData();
        if ((bitmapType == wxBITMAP_TYPE_ANY) || (handler->GetType() == bitmapType))
        {
            if (handler->GetExtension() == extension)
                return handler;
            if (handler->GetAltExtensions().Index(extension, false) != wxNOT_FOUND)
                return handler;
        }
        node = node->GetNext();
    }
    return NULL;
}

wxImageHandler *wxImage::FindHandler(wxBitmapType bitmapType )
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while (node)
    {
        wxImageHandler *handler = (wxImageHandler *)node->GetData();
        if (handler->GetType() == bitmapType) return handler;
        node = node->GetNext();
    }
    return NULL;
}

wxImageHandler *wxImage::FindHandlerMime( const wxString& mimetype )
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while (node)
    {
        wxImageHandler *handler = (wxImageHandler *)node->GetData();
        if (handler->GetMimeType().IsSameAs(mimetype, false)) return handler;
        node = node->GetNext();
    }
    return NULL;
}

void wxImage::InitStandardHandlers()
{
#if wxUSE_STREAMS
    AddHandler(new wxBMPHandler);
#endif }

void wxImage::CleanUpHandlers()
{
    wxList::compatibility_iterator node = sm_handlers.GetFirst();
    while (node)
    {
        wxImageHandler *handler = (wxImageHandler *)node->GetData();
        wxList::compatibility_iterator next = node->GetNext();
        delete handler;
        node = next;
    }

    sm_handlers.Clear();
}

wxString wxImage::GetImageExtWildcard()
{
    wxString fmts;

    wxList& Handlers = wxImage::GetHandlers();
    wxList::compatibility_iterator Node = Handlers.GetFirst();
    while ( Node )
    {
        wxImageHandler* Handler = (wxImageHandler*)Node->GetData();
        fmts += wxT("*.") + Handler->GetExtension();
        for (size_t i = 0; i < Handler->GetAltExtensions().size(); i++)
            fmts += wxT(";*.") + Handler->GetAltExtensions()[i];
        Node = Node->GetNext();
        if ( Node ) fmts += wxT(";");
    }

    return wxT("(") + fmts + wxT(")|") + fmts;
}

wxImage::HSVValue wxImage::RGBtoHSV(const RGBValue& rgb)
{
    const double red = rgb.red / 255.0,
                 green = rgb.green / 255.0,
                 blue = rgb.blue / 255.0;

            double minimumRGB = red;
    if ( green < minimumRGB )
        minimumRGB = green;
    if ( blue < minimumRGB )
        minimumRGB = blue;

    enum { RED, GREEN, BLUE } chMax = RED;
    double maximumRGB = red;
    if ( green > maximumRGB )
    {
        chMax = GREEN;
        maximumRGB = green;
    }
    if ( blue > maximumRGB )
    {
        chMax = BLUE;
        maximumRGB = blue;
    }

    const double value = maximumRGB;

    double hue = 0.0, saturation;
    const double deltaRGB = maximumRGB - minimumRGB;
    if ( wxIsNullDouble(deltaRGB) )
    {
                hue = 0.0;
        saturation = 0.0;
    }
    else
    {
        switch ( chMax )
        {
            case RED:
                hue = (green - blue) / deltaRGB;
                break;

            case GREEN:
                hue = 2.0 + (blue - red) / deltaRGB;
                break;

            case BLUE:
                hue = 4.0 + (red - green) / deltaRGB;
                break;
        }

        hue /= 6.0;

        if ( hue < 0.0 )
            hue += 1.0;

        saturation = deltaRGB / maximumRGB;
    }

    return HSVValue(hue, saturation, value);
}

wxImage::RGBValue wxImage::HSVtoRGB(const HSVValue& hsv)
{
    double red, green, blue;

    if ( wxIsNullDouble(hsv.saturation) )
    {
                red = hsv.value;
        green = hsv.value;
        blue = hsv.value;
    }
    else     {
        double hue = hsv.hue * 6.0;              int i = (int)floor(hue);
        double f = hue - i;                  double p = hsv.value * (1.0 - hsv.saturation);

        switch (i)
        {
            case 0:
                red = hsv.value;
                green = hsv.value * (1.0 - hsv.saturation * (1.0 - f));
                blue = p;
                break;

            case 1:
                red = hsv.value * (1.0 - hsv.saturation * f);
                green = hsv.value;
                blue = p;
                break;

            case 2:
                red = p;
                green = hsv.value;
                blue = hsv.value * (1.0 - hsv.saturation * (1.0 - f));
                break;

            case 3:
                red = p;
                green = hsv.value * (1.0 - hsv.saturation * f);
                blue = hsv.value;
                break;

            case 4:
                red = hsv.value * (1.0 - hsv.saturation * (1.0 - f));
                green = p;
                blue = hsv.value;
                break;

            default:                    red = hsv.value;
                green = p;
                blue = hsv.value * (1.0 - hsv.saturation * f);
                break;
        }
    }

    return RGBValue((unsigned char)(red * 255.0),
                    (unsigned char)(green * 255.0),
                    (unsigned char)(blue * 255.0));
}


void wxImage::RotateHue(double angle)
{
    AllocExclusive();

    unsigned char *srcBytePtr;
    unsigned char *dstBytePtr;
    unsigned long count;
    wxImage::HSVValue hsv;
    wxImage::RGBValue rgb;

    wxASSERT (angle >= -1.0 && angle <= 1.0);
    count = M_IMGDATA->m_width * M_IMGDATA->m_height;
    if ( count > 0 && !wxIsNullDouble(angle) )
    {
        srcBytePtr = M_IMGDATA->m_data;
        dstBytePtr = srcBytePtr;
        do
        {
            rgb.red = *srcBytePtr++;
            rgb.green = *srcBytePtr++;
            rgb.blue = *srcBytePtr++;
            hsv = RGBtoHSV(rgb);

            hsv.hue = hsv.hue + angle;
            if (hsv.hue > 1.0)
                hsv.hue = hsv.hue - 1.0;
            else if (hsv.hue < 0.0)
                hsv.hue = hsv.hue + 1.0;

            rgb = HSVtoRGB(hsv);
            *dstBytePtr++ = rgb.red;
            *dstBytePtr++ = rgb.green;
            *dstBytePtr++ = rgb.blue;
        } while (--count != 0);
    }
}


wxIMPLEMENT_ABSTRACT_CLASS(wxImageHandler, wxObject);

#if wxUSE_STREAMS
int wxImageHandler::GetImageCount( wxInputStream& stream )
{
        
    if ( !stream.IsSeekable() )
        return false;        
    wxFileOffset posOld = stream.TellI();
    int n = DoGetImageCount(stream);

        if ( stream.SeekI(posOld) == wxInvalidOffset )
    {
        wxLogDebug(wxT("Failed to rewind the stream in wxImageHandler!"));

                return false;
    }

    return n;
}

bool wxImageHandler::CanRead( const wxString& name )
{
    wxImageFileInputStream stream(name);
    if ( !stream.IsOk() )
    {
        wxLogError(_("Failed to check format of image file \"%s\"."), name);

        return false;
    }

    return CanRead(stream);
}

bool wxImageHandler::CallDoCanRead(wxInputStream& stream)
{
        
    if ( !stream.IsSeekable() )
        return false;        
    wxFileOffset posOld = stream.TellI();
    bool ok = DoCanRead(stream);

        if ( stream.SeekI(posOld) == wxInvalidOffset )
    {
        wxLogDebug(wxT("Failed to rewind the stream in wxImageHandler!"));

                return false;
    }

    return ok;
}

#endif 

wxImageResolution
wxImageHandler::GetResolutionFromOptions(const wxImage& image, int *x, int *y)
{
    wxCHECK_MSG( x && y, wxIMAGE_RESOLUTION_NONE, wxT("NULL pointer") );

    if ( image.HasOption(wxIMAGE_OPTION_RESOLUTIONX) &&
         image.HasOption(wxIMAGE_OPTION_RESOLUTIONY) )
    {
        *x = image.GetOptionInt(wxIMAGE_OPTION_RESOLUTIONX);
        *y = image.GetOptionInt(wxIMAGE_OPTION_RESOLUTIONY);
    }
    else if ( image.HasOption(wxIMAGE_OPTION_RESOLUTION) )
    {
        *x =
        *y = image.GetOptionInt(wxIMAGE_OPTION_RESOLUTION);
    }
    else     {
        *x =
        *y = 0;

        return wxIMAGE_RESOLUTION_NONE;
    }

        int resUnit = image.GetOptionInt(wxIMAGE_OPTION_RESOLUTIONUNIT);
    if ( !resUnit )
    {
                resUnit = wxIMAGE_RESOLUTION_INCHES;
    }

    return (wxImageResolution)resUnit;
}


bool
wxImageHistogram::FindFirstUnusedColour(unsigned char *r,
                                        unsigned char *g,
                                        unsigned char *b,
                                        unsigned char r2,
                                        unsigned char g2,
                                        unsigned char b2) const
{
    unsigned long key = MakeKey(r2, g2, b2);

    while ( find(key) != end() )
    {
                r2++;
        if ( r2 >= 255 )
        {
            r2 = 0;
            g2++;
            if ( g2 >= 255 )
            {
                g2 = 0;
                b2++;
                if ( b2 >= 255 )
                {
                    wxLogError(_("No unused colour in image.") );
                    return false;
                }
            }
        }

        key = MakeKey(r2, g2, b2);
    }

    if ( r )
        *r = r2;
    if ( g )
        *g = g2;
    if ( b )
        *b = b2;

    return true;
}

bool
wxImage::FindFirstUnusedColour(unsigned char *r,
                               unsigned char *g,
                               unsigned char *b,
                               unsigned char r2,
                               unsigned char g2,
                               unsigned char b2) const
{
    wxImageHistogram histogram;

    ComputeHistogram(histogram);

    return histogram.FindFirstUnusedColour(r, g, b, r2, g2, b2);
}



unsigned long wxImage::CountColours( unsigned long stopafter ) const
{
    wxHashTable h;
    wxObject dummy;
    unsigned char r, g, b;
    unsigned char *p;
    unsigned long size, nentries, key;

    p = GetData();
    size = GetWidth() * GetHeight();
    nentries = 0;

    for (unsigned long j = 0; (j < size) && (nentries <= stopafter) ; j++)
    {
        r = *(p++);
        g = *(p++);
        b = *(p++);
        key = wxImageHistogram::MakeKey(r, g, b);

        if (h.Get(key) == NULL)
        {
            h.Put(key, &dummy);
            nentries++;
        }
    }

    return nentries;
}


unsigned long wxImage::ComputeHistogram( wxImageHistogram &h ) const
{
    unsigned char *p = GetData();
    unsigned long nentries = 0;

    h.clear();

    const unsigned long size = GetWidth() * GetHeight();

    unsigned char r, g, b;
    for ( unsigned long n = 0; n < size; n++ )
    {
        r = *p++;
        g = *p++;
        b = *p++;

        wxImageHistogramEntry& entry = h[wxImageHistogram::MakeKey(r, g, b)];

        if ( entry.value++ == 0 )
            entry.index = nentries++;
    }

    return nentries;
}



static const double wxROTATE_EPSILON = 1e-10;


static inline wxRealPoint
wxRotatePoint(const wxRealPoint& p, double cos_angle, double sin_angle,
              const wxRealPoint& p0)
{
    return wxRealPoint(p0.x + (p.x - p0.x) * cos_angle - (p.y - p0.y) * sin_angle,
                       p0.y + (p.y - p0.y) * cos_angle + (p.x - p0.x) * sin_angle);
}

static inline wxRealPoint
wxRotatePoint(double x, double y, double cos_angle, double sin_angle,
              const wxRealPoint & p0)
{
    return wxRotatePoint (wxRealPoint(x,y), cos_angle, sin_angle, p0);
}

wxImage wxImage::Rotate(double angle,
                        const wxPoint& centre_of_rotation,
                        bool interpolating,
                        wxPoint *offset_after_rotation) const
{
        angle = -angle;

    const bool has_alpha = HasAlpha();

    const int w = GetWidth();
    const int h = GetHeight();

    int i;

        unsigned char ** data = new unsigned char * [h];
    data[0] = GetData();
    for (i = 1; i < h; i++)
        data[i] = data[i - 1] + (3 * w);

        unsigned char ** alpha = NULL;
    if (has_alpha)
    {
        alpha = new unsigned char * [h];
        alpha[0] = GetAlpha();
        for (i = 1; i < h; i++)
            alpha[i] = alpha[i - 1] + w;
    }

        const double cos_angle = cos(angle);
    const double sin_angle = sin(angle);

            
    const wxRealPoint p0(centre_of_rotation.x, centre_of_rotation.y);

    wxRealPoint p1 = wxRotatePoint (0, 0, cos_angle, sin_angle, p0);
    wxRealPoint p2 = wxRotatePoint (0, h, cos_angle, sin_angle, p0);
    wxRealPoint p3 = wxRotatePoint (w, 0, cos_angle, sin_angle, p0);
    wxRealPoint p4 = wxRotatePoint (w, h, cos_angle, sin_angle, p0);

    int x1a = (int) floor (wxMin (wxMin(p1.x, p2.x), wxMin(p3.x, p4.x)));
    int y1a = (int) floor (wxMin (wxMin(p1.y, p2.y), wxMin(p3.y, p4.y)));
    int x2a = (int) ceil (wxMax (wxMax(p1.x, p2.x), wxMax(p3.x, p4.x)));
    int y2a = (int) ceil (wxMax (wxMax(p1.y, p2.y), wxMax(p3.y, p4.y)));

        wxImage rotated (x2a - x1a + 1, y2a - y1a + 1, false);
        if (has_alpha)
        rotated.SetAlpha();

    if (offset_after_rotation != NULL)
    {
        *offset_after_rotation = wxPoint (x1a, y1a);
    }

            unsigned char *dst = rotated.GetData();

    unsigned char *alpha_dst = has_alpha ? rotated.GetAlpha() : NULL;

            unsigned char blank_r = 0;
    unsigned char blank_g = 0;
    unsigned char blank_b = 0;

    if (HasMask())
    {
        blank_r = GetMaskRed();
        blank_g = GetMaskGreen();
        blank_b = GetMaskBlue();
        rotated.SetMaskColour( blank_r, blank_g, blank_b );
    }

            
    const int rH = rotated.GetHeight();
    const int rW = rotated.GetWidth();

            if (interpolating)
    {
        for (int y = 0; y < rH; y++)
        {
            for (int x = 0; x < rW; x++)
            {
                wxRealPoint src = wxRotatePoint (x + x1a, y + y1a, cos_angle, -sin_angle, p0);

                if (-0.25 < src.x && src.x < w - 0.75 &&
                    -0.25 < src.y && src.y < h - 0.75)
                {
                                                                                int x1, y1, x2, y2;

                    if (0 < src.x && src.x < w - 1)
                    {
                        x1 = wxRound(floor(src.x));
                        x2 = wxRound(ceil(src.x));
                    }
                    else                        {
                        x1 = x2 = wxRound (src.x);
                    }

                    if (0 < src.y && src.y < h - 1)
                    {
                        y1 = wxRound(floor(src.y));
                        y2 = wxRound(ceil(src.y));
                    }
                    else
                    {
                        y1 = y2 = wxRound (src.y);
                    }

                                        
                                                                                
                    const double d1 = (src.x - x1) * (src.x - x1) + (src.y - y1) * (src.y - y1);
                    const double d2 = (src.x - x2) * (src.x - x2) + (src.y - y1) * (src.y - y1);
                    const double d3 = (src.x - x2) * (src.x - x2) + (src.y - y2) * (src.y - y2);
                    const double d4 = (src.x - x1) * (src.x - x1) + (src.y - y2) * (src.y - y2);

                                        
                                        
                                        if (d1 < wxROTATE_EPSILON)
                    {
                        unsigned char *p = data[y1] + (3 * x1);
                        *(dst++) = *(p++);
                        *(dst++) = *(p++);
                        *(dst++) = *p;

                        if (has_alpha)
                            *(alpha_dst++) = *(alpha[y1] + x1);
                    }
                    else if (d2 < wxROTATE_EPSILON)
                    {
                        unsigned char *p = data[y1] + (3 * x2);
                        *(dst++) = *(p++);
                        *(dst++) = *(p++);
                        *(dst++) = *p;

                        if (has_alpha)
                            *(alpha_dst++) = *(alpha[y1] + x2);
                    }
                    else if (d3 < wxROTATE_EPSILON)
                    {
                        unsigned char *p = data[y2] + (3 * x2);
                        *(dst++) = *(p++);
                        *(dst++) = *(p++);
                        *(dst++) = *p;

                        if (has_alpha)
                            *(alpha_dst++) = *(alpha[y2] + x2);
                    }
                    else if (d4 < wxROTATE_EPSILON)
                    {
                        unsigned char *p = data[y2] + (3 * x1);
                        *(dst++) = *(p++);
                        *(dst++) = *(p++);
                        *(dst++) = *p;

                        if (has_alpha)
                            *(alpha_dst++) = *(alpha[y2] + x1);
                    }
                    else
                    {
                                                unsigned char *v1 = data[y1] + (3 * x1);
                        unsigned char *v2 = data[y1] + (3 * x2);
                        unsigned char *v3 = data[y2] + (3 * x2);
                        unsigned char *v4 = data[y2] + (3 * x1);

                        const double w1 = 1/d1, w2 = 1/d2, w3 = 1/d3, w4 = 1/d4;

                        
                        *(dst++) = (unsigned char)
                            ( (w1 * *(v1++) + w2 * *(v2++) +
                               w3 * *(v3++) + w4 * *(v4++)) /
                              (w1 + w2 + w3 + w4) );
                        *(dst++) = (unsigned char)
                            ( (w1 * *(v1++) + w2 * *(v2++) +
                               w3 * *(v3++) + w4 * *(v4++)) /
                              (w1 + w2 + w3 + w4) );
                        *(dst++) = (unsigned char)
                            ( (w1 * *v1 + w2 * *v2 +
                               w3 * *v3 + w4 * *v4) /
                              (w1 + w2 + w3 + w4) );

                        if (has_alpha)
                        {
                            v1 = alpha[y1] + (x1);
                            v2 = alpha[y1] + (x2);
                            v3 = alpha[y2] + (x2);
                            v4 = alpha[y2] + (x1);

                            *(alpha_dst++) = (unsigned char)
                                ( (w1 * *v1 + w2 * *v2 +
                                   w3 * *v3 + w4 * *v4) /
                                  (w1 + w2 + w3 + w4) );
                        }
                    }
                }
                else
                {
                    *(dst++) = blank_r;
                    *(dst++) = blank_g;
                    *(dst++) = blank_b;

                    if (has_alpha)
                        *(alpha_dst++) = 0;
                }
            }
        }
    }
    else     {
        for (int y = 0; y < rH; y++)
        {
            for (int x = 0; x < rW; x++)
            {
                wxRealPoint src = wxRotatePoint (x + x1a, y + y1a, cos_angle, -sin_angle, p0);

                const int xs = wxRound (src.x);                      const int ys = wxRound (src.y);      
                if (0 <= xs && xs < w && 0 <= ys && ys < h)
                {
                    unsigned char *p = data[ys] + (3 * xs);
                    *(dst++) = *(p++);
                    *(dst++) = *(p++);
                    *(dst++) = *p;

                    if (has_alpha)
                        *(alpha_dst++) = *(alpha[ys] + (xs));
                }
                else
                {
                    *(dst++) = blank_r;
                    *(dst++) = blank_g;
                    *(dst++) = blank_b;

                    if (has_alpha)
                        *(alpha_dst++) = 255;
                }
            }
        }
    }

    delete [] data;
    delete [] alpha;

    return rotated;
}






class wxImageModule: public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxImageModule);
public:
    wxImageModule() {}
    bool OnInit() wxOVERRIDE { wxImage::InitStandardHandlers(); return true; }
    void OnExit() wxOVERRIDE { wxImage::CleanUpHandlers(); }
};

wxIMPLEMENT_DYNAMIC_CLASS(wxImageModule, wxModule);


#endif 