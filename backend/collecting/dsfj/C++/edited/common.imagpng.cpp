


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_IMAGE && wxUSE_LIBPNG

#include "wx/imagpng.h"
#include "wx/versioninfo.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/intl.h"
    #include "wx/palette.h"
    #include "wx/stream.h"
#endif

#include "png.h"

#include <string.h>


static unsigned char *InitAlpha(wxImage *image, png_uint_32 x, png_uint_32 y);

static inline
bool IsOpaque(unsigned char a)
{
    return a == 0xff;
}


wxIMPLEMENT_DYNAMIC_CLASS(wxPNGHandler,wxImageHandler);

#if wxUSE_STREAMS

#ifndef PNGLINKAGEMODE
    #ifdef PNGAPI
        #define PNGLINKAGEMODE PNGAPI
    #else
        #define PNGLINKAGEMODE LINKAGEMODE
    #endif
#endif



struct wxPNGInfoStruct
{
    jmp_buf jmpbuf;
    bool verbose;

    union
    {
        wxInputStream  *in;
        wxOutputStream *out;
    } stream;
};

#define WX_PNG_INFO(png_ptr) ((wxPNGInfoStruct*)png_get_io_ptr(png_ptr))


extern "C"
{

static void PNGLINKAGEMODE wx_PNG_stream_reader( png_structp png_ptr, png_bytep data,
                                                 png_size_t length )
{
    WX_PNG_INFO(png_ptr)->stream.in->Read(data, length);
}

static void PNGLINKAGEMODE wx_PNG_stream_writer( png_structp png_ptr, png_bytep data,
                                                 png_size_t length )
{
    WX_PNG_INFO(png_ptr)->stream.out->Write(data, length);
}

static void
PNGLINKAGEMODE wx_PNG_warning(png_structp png_ptr, png_const_charp message)
{
    wxPNGInfoStruct *info = png_ptr ? WX_PNG_INFO(png_ptr) : NULL;
    if ( !info || info->verbose )
    {
        wxLogWarning( wxString::FromAscii(message) );
    }
}

static void
PNGLINKAGEMODE wx_PNG_error(png_structp png_ptr, png_const_charp message)
{
    wx_PNG_warning(NULL, message);

                longjmp(WX_PNG_INFO(png_ptr)->jmpbuf, 1);
}

} 

unsigned char *InitAlpha(wxImage *image, png_uint_32 x, png_uint_32 y)
{
        image->SetAlpha();

    unsigned char *alpha = image->GetAlpha();

        png_uint_32 end = y * image->GetWidth() + x;
    for ( png_uint_32 i = 0; i < end; i++ )
    {
                *alpha++ = 0xff;
    }

    return alpha;
}


bool wxPNGHandler::DoCanRead( wxInputStream& stream )
{
    unsigned char hdr[4];

    if ( !stream.Read(hdr, WXSIZEOF(hdr)) )             return false;

    return memcmp(hdr, "\211PNG", WXSIZEOF(hdr)) == 0;
}

static
void CopyDataFromPNG(wxImage *image,
                     unsigned char **lines,
                     png_uint_32 width,
                     png_uint_32 height,
                     int color_type)
{
        unsigned char *alpha = NULL;

    unsigned char *ptrDst = image->GetData();
    if ( !(color_type & PNG_COLOR_MASK_COLOR) )
    {
                for ( png_uint_32 y = 0; y < height; y++ )
        {
            const unsigned char *ptrSrc = lines[y];
            for ( png_uint_32 x = 0; x < width; x++ )
            {
                unsigned char g = *ptrSrc++;
                unsigned char a = *ptrSrc++;

                                                if ( !IsOpaque(a) && !alpha )
                    alpha = InitAlpha(image, x, y);

                if ( alpha )
                    *alpha++ = a;

                *ptrDst++ = g;
                *ptrDst++ = g;
                *ptrDst++ = g;
            }
        }
    }
    else     {
        for ( png_uint_32 y = 0; y < height; y++ )
        {
            const unsigned char *ptrSrc = lines[y];
            for ( png_uint_32 x = 0; x < width; x++ )
            {
                unsigned char r = *ptrSrc++;
                unsigned char g = *ptrSrc++;
                unsigned char b = *ptrSrc++;
                unsigned char a = *ptrSrc++;

                                if ( !IsOpaque(a) && !alpha )
                    alpha = InitAlpha(image, x, y);

                if ( alpha )
                    *alpha++ = a;

                *ptrDst++ = r;
                *ptrDst++ = g;
                *ptrDst++ = b;
            }
        }
    }
}

#ifdef __VISUALC__
    #pragma warning(disable:4611)
#endif 

bool
wxPNGHandler::LoadFile(wxImage *image,
                       wxInputStream& stream,
                       bool verbose,
                       int WXUNUSED(index))
{
        
    unsigned char **lines = NULL;
    png_infop info_ptr = (png_infop) NULL;
    wxPNGInfoStruct wxinfo;

    png_uint_32 i, width, height = 0;
    int bit_depth, color_type, interlace_type;

    wxinfo.verbose = verbose;
    wxinfo.stream.in = &stream;

    image->Destroy();

    png_structp png_ptr = png_create_read_struct
                          (
                            PNG_LIBPNG_VER_STRING,
                            NULL,
                            wx_PNG_error,
                            wx_PNG_warning
                          );
    if (!png_ptr)
        goto error;

            png_set_read_fn( png_ptr, &wxinfo, wx_PNG_stream_reader);

    info_ptr = png_create_info_struct( png_ptr );
    if (!info_ptr)
        goto error;

    if (setjmp(wxinfo.jmpbuf))
        goto error;

    png_read_info( png_ptr, info_ptr );
    png_get_IHDR( png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL );

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_expand( png_ptr );

        if (bit_depth < 8)
        png_set_expand( png_ptr );

    png_set_strip_16( png_ptr );
    png_set_packing( png_ptr );
    if (png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_expand( png_ptr );
    png_set_filler( png_ptr, 0xff, PNG_FILLER_AFTER );

    image->Create((int)width, (int)height, (bool) false );

    if (!image->IsOk())
        goto error;

            lines = (unsigned char **)calloc(height, sizeof(unsigned char *));
    if ( !lines )
        goto error;

    for (i = 0; i < height; i++)
    {
        if ((lines[i] = (unsigned char *)malloc( (size_t)(width * 4))) == NULL)
            goto error;
    }

    png_read_image( png_ptr, lines );
    png_read_end( png_ptr, info_ptr );

#if wxUSE_PALETTE
    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_colorp palette = NULL;
        int numPalette = 0;

        (void) png_get_PLTE(png_ptr, info_ptr, &palette, &numPalette);

        unsigned char* r = new unsigned char[numPalette];
        unsigned char* g = new unsigned char[numPalette];
        unsigned char* b = new unsigned char[numPalette];

        for (int j = 0; j < numPalette; j++)
        {
            r[j] = palette[j].red;
            g[j] = palette[j].green;
            b[j] = palette[j].blue;
        }

        image->SetPalette(wxPalette(numPalette, r, g, b));
        delete[] r;
        delete[] g;
        delete[] b;
    }
#endif 

        png_uint_32 resX, resY;
    int unitType;
    if (png_get_pHYs(png_ptr, info_ptr, &resX, &resY, &unitType)
        == PNG_INFO_pHYs)
    {
        wxImageResolution res = wxIMAGE_RESOLUTION_CM;

        switch (unitType)
        {
            default:
                wxLogWarning(_("Unknown PNG resolution unit %d"), unitType);
                wxFALLTHROUGH;

            case PNG_RESOLUTION_UNKNOWN:
                image->SetOption(wxIMAGE_OPTION_RESOLUTIONX, resX);
                image->SetOption(wxIMAGE_OPTION_RESOLUTIONY, resY);

                res = wxIMAGE_RESOLUTION_NONE;
                break;

            case PNG_RESOLUTION_METER:
                
                image->SetOption(wxIMAGE_OPTION_RESOLUTIONX,
                    wxString::FromCDouble((double) resX / 100.0, 2));
                image->SetOption(wxIMAGE_OPTION_RESOLUTIONY,
                    wxString::FromCDouble((double) resY / 100.0, 2));
                break;
        }

        image->SetOption(wxIMAGE_OPTION_RESOLUTIONUNIT, res);
    }


    png_destroy_read_struct( &png_ptr, &info_ptr, (png_infopp) NULL );

        CopyDataFromPNG(image, lines, width, height, color_type);

    for ( i = 0; i < height; i++ )
        free( lines[i] );
    free( lines );

    return true;

error:
    if (verbose)
    {
       wxLogError(_("Couldn't load a PNG image - file is corrupted or not enough memory."));
    }

    if ( image->IsOk() )
    {
        image->Destroy();
    }

    if ( lines )
    {
        for ( unsigned int n = 0; n < height; n++ )
            free( lines[n] );

        free( lines );
    }

    if ( png_ptr )
    {
        if ( info_ptr )
        {
            png_destroy_read_struct( &png_ptr, &info_ptr, (png_infopp) NULL );
            free(info_ptr);
        }
        else
            png_destroy_read_struct( &png_ptr, (png_infopp) NULL, (png_infopp) NULL );
    }
    return false;
}


typedef wxLongToLongHashMap PaletteMap;

static unsigned long PaletteMakeKey(const png_color_8& clr)
{
    return (wxImageHistogram::MakeKey(clr.red, clr.green, clr.blue) << 8) | clr.alpha;
}

static long PaletteFind(const PaletteMap& palette, const png_color_8& clr)
{
    unsigned long value = PaletteMakeKey(clr);
    PaletteMap::const_iterator it = palette.find(value);

    return (it != palette.end()) ? it->second : wxNOT_FOUND;
}

static long PaletteAdd(PaletteMap *palette, const png_color_8& clr)
{
    unsigned long value = PaletteMakeKey(clr);
    PaletteMap::const_iterator it = palette->find(value);
    size_t index;

    if (it == palette->end())
    {
        index = palette->size();
        (*palette)[value] = index;
    }
    else
    {
        index = it->second;
    }

    return index;
}


bool wxPNGHandler::SaveFile( wxImage *image, wxOutputStream& stream, bool verbose )
{
    wxPNGInfoStruct wxinfo;

    wxinfo.verbose = verbose;
    wxinfo.stream.out = &stream;

    png_structp png_ptr = png_create_write_struct
                          (
                            PNG_LIBPNG_VER_STRING,
                            NULL,
                            wx_PNG_error,
                            wx_PNG_warning
                          );
    if (!png_ptr)
    {
        if (verbose)
        {
           wxLogError(_("Couldn't save PNG image."));
        }
        return false;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        png_destroy_write_struct( &png_ptr, (png_infopp)NULL );
        if (verbose)
        {
           wxLogError(_("Couldn't save PNG image."));
        }
        return false;
    }

    if (setjmp(wxinfo.jmpbuf))
    {
        png_destroy_write_struct( &png_ptr, (png_infopp)NULL );
        if (verbose)
        {
           wxLogError(_("Couldn't save PNG image."));
        }
        return false;
    }

            png_set_write_fn( png_ptr, &wxinfo, wx_PNG_stream_writer, NULL);

    const int iHeight = image->GetHeight();
    const int iWidth = image->GetWidth();

    const bool bHasPngFormatOption
        = image->HasOption(wxIMAGE_OPTION_PNG_FORMAT);

    int iColorType = bHasPngFormatOption
                            ? image->GetOptionInt(wxIMAGE_OPTION_PNG_FORMAT)
                            : wxPNG_TYPE_COLOUR;

    bool bHasAlpha = image->HasAlpha();
    bool bHasMask = image->HasMask();

    bool bUsePalette = iColorType == wxPNG_TYPE_PALETTE
#if wxUSE_PALETTE
        || (!bHasPngFormatOption && image->HasPalette() )
#endif
    ;

    png_color_8 mask = { 0, 0, 0, 0, 0 };

    if (bHasMask)
    {
        mask.red   = image->GetMaskRed();
        mask.green = image->GetMaskGreen();
        mask.blue  = image->GetMaskBlue();
    }

    PaletteMap palette;

    if (bUsePalette)
    {
        png_color png_rgb  [PNG_MAX_PALETTE_LENGTH];
        png_byte  png_trans[PNG_MAX_PALETTE_LENGTH];

        const unsigned char *pColors = image->GetData();
        const unsigned char* pAlpha  = image->GetAlpha();

        if (bHasMask && !pAlpha)
        {
                        PaletteAdd(&palette, mask);
        }

        for (int y = 0; y < iHeight; y++)
        {
            for (int x = 0; x < iWidth; x++)
            {
                png_color_8 rgba;

                rgba.red   = *pColors++;
                rgba.green = *pColors++;
                rgba.blue  = *pColors++;
                rgba.gray  = 0;
                rgba.alpha = (pAlpha && !bHasMask) ? *pAlpha++ : 0;

                                long index = PaletteAdd(&palette, rgba);

                if (index < PNG_MAX_PALETTE_LENGTH)
                {
                                        png_rgb[index].red   = rgba.red;
                    png_rgb[index].green = rgba.green;
                    png_rgb[index].blue  = rgba.blue;
                    png_trans[index]     = rgba.alpha;
                }
                else
                {
                    bUsePalette = false;
                    break;
                }
            }
        }

        if (bUsePalette)
        {
            png_set_PLTE(png_ptr, info_ptr, png_rgb, palette.size());

            if (bHasMask && !pAlpha)
            {
                wxASSERT(PaletteFind(palette, mask) == 0);
                png_trans[0] = 0;
                png_set_tRNS(png_ptr, info_ptr, png_trans, 1, NULL);
            }
            else if (pAlpha && !bHasMask)
            {
                png_set_tRNS(png_ptr, info_ptr, png_trans, palette.size(), NULL);
            }
        }
    }

    
    if (!bUsePalette && iColorType == wxPNG_TYPE_PALETTE)
    {
        iColorType = wxPNG_TYPE_COLOUR;
    }

    bool bUseAlpha = !bUsePalette && (bHasAlpha || bHasMask);

    int iPngColorType;

    if (bUsePalette)
    {
        iPngColorType = PNG_COLOR_TYPE_PALETTE;
        iColorType = wxPNG_TYPE_PALETTE;
    }
    else if ( iColorType==wxPNG_TYPE_COLOUR )
    {
        iPngColorType = bUseAlpha ? PNG_COLOR_TYPE_RGB_ALPHA
                                  : PNG_COLOR_TYPE_RGB;
    }
    else
    {
        iPngColorType = bUseAlpha ? PNG_COLOR_TYPE_GRAY_ALPHA
                                  : PNG_COLOR_TYPE_GRAY;
    }

    if (image->HasOption(wxIMAGE_OPTION_PNG_FILTER))
        png_set_filter( png_ptr, PNG_FILTER_TYPE_BASE, image->GetOptionInt(wxIMAGE_OPTION_PNG_FILTER) );

    if (image->HasOption(wxIMAGE_OPTION_PNG_COMPRESSION_LEVEL))
        png_set_compression_level( png_ptr, image->GetOptionInt(wxIMAGE_OPTION_PNG_COMPRESSION_LEVEL) );

    if (image->HasOption(wxIMAGE_OPTION_PNG_COMPRESSION_MEM_LEVEL))
        png_set_compression_mem_level( png_ptr, image->GetOptionInt(wxIMAGE_OPTION_PNG_COMPRESSION_MEM_LEVEL) );

    if (image->HasOption(wxIMAGE_OPTION_PNG_COMPRESSION_STRATEGY))
        png_set_compression_strategy( png_ptr, image->GetOptionInt(wxIMAGE_OPTION_PNG_COMPRESSION_STRATEGY) );

    if (image->HasOption(wxIMAGE_OPTION_PNG_COMPRESSION_BUFFER_SIZE))
        png_set_compression_buffer_size( png_ptr, image->GetOptionInt(wxIMAGE_OPTION_PNG_COMPRESSION_BUFFER_SIZE) );

    int iBitDepth = !bUsePalette && image->HasOption(wxIMAGE_OPTION_PNG_BITDEPTH)
                            ? image->GetOptionInt(wxIMAGE_OPTION_PNG_BITDEPTH)
                            : 8;

    png_set_IHDR( png_ptr, info_ptr, image->GetWidth(), image->GetHeight(),
                  iBitDepth, iPngColorType,
                  PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                  PNG_FILTER_TYPE_BASE);

    int iElements;
    png_color_8 sig_bit;

    if ( iPngColorType & PNG_COLOR_MASK_COLOR )
    {
        sig_bit.red =
        sig_bit.green =
        sig_bit.blue = (png_byte)iBitDepth;
        iElements = 3;
    }
    else     {
        sig_bit.gray = (png_byte)iBitDepth;
        iElements = 1;
    }

    if ( bUseAlpha )
    {
        sig_bit.alpha = (png_byte)iBitDepth;
        iElements++;
    }

    if ( iBitDepth == 16 )
        iElements *= 2;

        int resX, resY;
    switch ( GetResolutionFromOptions(*image, &resX, &resY) )
    {
        case wxIMAGE_RESOLUTION_INCHES:
            {
                const double INCHES_IN_METER = 10000.0 / 254;
                resX = int(resX * INCHES_IN_METER);
                resY = int(resY * INCHES_IN_METER);
            }
            break;

        case wxIMAGE_RESOLUTION_CM:
            resX *= 100;
            resY *= 100;
            break;

        case wxIMAGE_RESOLUTION_NONE:
            break;

        default:
            wxFAIL_MSG( wxT("unsupported image resolution units") );
    }

    if ( resX && resY )
        png_set_pHYs( png_ptr, info_ptr, resX, resY, PNG_RESOLUTION_METER );

    png_set_sBIT( png_ptr, info_ptr, &sig_bit );
    png_write_info( png_ptr, info_ptr );
    png_set_shift( png_ptr, &sig_bit );
    png_set_packing( png_ptr );

    unsigned char *
        data = (unsigned char *)malloc( image->GetWidth() * iElements );
    if ( !data )
    {
        png_destroy_write_struct( &png_ptr, (png_infopp)NULL );
        return false;
    }

    const unsigned char *
        pAlpha = (const unsigned char *)(bHasAlpha ? image->GetAlpha() : NULL);

    const unsigned char *pColors = image->GetData();

    for (int y = 0; y != iHeight; ++y)
    {
        unsigned char *pData = data;
        for (int x = 0; x != iWidth; x++)
        {
            png_color_8 clr;
            clr.red   = *pColors++;
            clr.green = *pColors++;
            clr.blue  = *pColors++;
            clr.gray  = 0;
            clr.alpha = (bUsePalette && pAlpha) ? *pAlpha++ : 0; 
            switch ( iColorType )
            {
                default:
                    wxFAIL_MSG( wxT("unknown wxPNG_TYPE_XXX") );
                    wxFALLTHROUGH;

                case wxPNG_TYPE_COLOUR:
                    *pData++ = clr.red;
                    if ( iBitDepth == 16 )
                        *pData++ = 0;
                    *pData++ = clr.green;
                    if ( iBitDepth == 16 )
                        *pData++ = 0;
                    *pData++ = clr.blue;
                    if ( iBitDepth == 16 )
                        *pData++ = 0;
                    break;

                case wxPNG_TYPE_GREY:
                    {
                                                                        unsigned uiColor =
                            (unsigned) (76.544*(unsigned)clr.red +
                                        150.272*(unsigned)clr.green +
                                        36.864*(unsigned)clr.blue);

                        *pData++ = (unsigned char)((uiColor >> 8) & 0xFF);
                        if ( iBitDepth == 16 )
                            *pData++ = (unsigned char)(uiColor & 0xFF);
                    }
                    break;

                case wxPNG_TYPE_GREY_RED:
                    *pData++ = clr.red;
                    if ( iBitDepth == 16 )
                        *pData++ = 0;
                    break;

                case wxPNG_TYPE_PALETTE:
                    *pData++ = (unsigned char) PaletteFind(palette, clr);
                    break;
            }

            if ( bUseAlpha )
            {
                unsigned char uchAlpha = 255;
                if ( bHasAlpha )
                    uchAlpha = *pAlpha++;

                if ( bHasMask )
                {
                    if ( (clr.red == mask.red)
                            && (clr.green == mask.green)
                                && (clr.blue == mask.blue) )
                        uchAlpha = 0;
                }

                *pData++ = uchAlpha;
                if ( iBitDepth == 16 )
                    *pData++ = 0;
            }
        }

        png_bytep row_ptr = data;
        png_write_rows( png_ptr, &row_ptr, 1 );
    }

    free(data);
    png_write_end( png_ptr, info_ptr );
    png_destroy_write_struct( &png_ptr, (png_infopp)&info_ptr );

    return true;
}

#ifdef __VISUALC__
    #pragma warning(default:4611)
#endif 

#endif  
 wxVersionInfo wxPNGHandler::GetLibraryVersionInfo()
{
            wxString str = png_get_header_version(NULL) + 1;
    str.Replace("\n", "");

    return wxVersionInfo("libpng",
                         PNG_LIBPNG_VER_MAJOR,
                         PNG_LIBPNG_VER_MINOR,
                         PNG_LIBPNG_VER_RELEASE,
                         str);
}

#endif  