
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_IMAGE && wxUSE_LIBJPEG

#include "wx/imagjpeg.h"
#include "wx/versioninfo.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/app.h"
    #include "wx/intl.h"
    #include "wx/bitmap.h"
    #include "wx/module.h"
#endif

#ifdef wxHACK_BOOLEAN
    #define HAVE_BOOLEAN
    #define boolean wxHACK_BOOLEAN
#endif

extern "C"
{
    #if defined(__WXMSW__)
        #define XMD_H
    #endif
    #include "jpeglib.h"
}

#ifndef HAVE_WXJPEG_BOOLEAN
typedef boolean wxjpeg_boolean;
#endif

#include "wx/filefn.h"
#include "wx/wfstream.h"

#include <string.h>
#include <setjmp.h>


#ifdef JPEG_METHOD_LINKAGE
    #define CPP_METHODDEF(type) extern "C" METHODDEF(type)
#else     #define CPP_METHODDEF(type) METHODDEF(type)
#endif


wxIMPLEMENT_DYNAMIC_CLASS(wxJPEGHandler,wxImageHandler);

#if wxUSE_STREAMS


#define JPEG_IO_BUFFER_SIZE   2048

typedef struct {
    struct jpeg_source_mgr pub;   

    JOCTET* buffer;               
    wxInputStream *stream;
} wx_source_mgr;

typedef wx_source_mgr * wx_src_ptr;

extern "C"
{

CPP_METHODDEF(void) wx_init_source ( j_decompress_ptr WXUNUSED(cinfo) )
{
}

CPP_METHODDEF(wxjpeg_boolean) wx_fill_input_buffer ( j_decompress_ptr cinfo )
{
    wx_src_ptr src = (wx_src_ptr) cinfo->src;

    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = src->stream->Read(src->buffer, JPEG_IO_BUFFER_SIZE).LastRead();

    if (src->pub.bytes_in_buffer == 0)     {
                src->buffer[0] = 0xFF;
        src->buffer[1] = JPEG_EOI;
        src->pub.bytes_in_buffer = 2;
    }
    return TRUE;
}

CPP_METHODDEF(void) wx_skip_input_data ( j_decompress_ptr cinfo, long num_bytes )
{
    if (num_bytes > 0)
    {
        wx_src_ptr src = (wx_src_ptr) cinfo->src;

        while (num_bytes > (long)src->pub.bytes_in_buffer)
        {
            num_bytes -= (long) src->pub.bytes_in_buffer;
            src->pub.fill_input_buffer(cinfo);
        }
        src->pub.next_input_byte += (size_t) num_bytes;
        src->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
}

CPP_METHODDEF(void) wx_term_source ( j_decompress_ptr cinfo )
{
    wx_src_ptr src = (wx_src_ptr) cinfo->src;

    if (src->pub.bytes_in_buffer > 0)
        src->stream->SeekI(-(long)src->pub.bytes_in_buffer, wxFromCurrent);
    delete[] src->buffer;
}



#ifdef __VISUALC__
                            #pragma warning(push)
    #pragma warning(disable: 4324)
#endif

struct wx_error_mgr : public jpeg_error_mgr
{
  jmp_buf setjmp_buffer;    
};

#ifdef __VISUALC__
    #pragma warning(pop)
#endif



CPP_METHODDEF(void) wx_error_exit (j_common_ptr cinfo)
{
  
  wx_error_mgr * const jerr = (wx_error_mgr *) cinfo->err;

  
  
  (*cinfo->err->output_message) (cinfo);

  
  longjmp(jerr->setjmp_buffer, 1);
}


CPP_METHODDEF(void) wx_ignore_message (j_common_ptr WXUNUSED(cinfo))
{
}

void wx_jpeg_io_src( j_decompress_ptr cinfo, wxInputStream& infile )
{
    wx_src_ptr src;

    if (cinfo->src == NULL) {    
        cinfo->src = (struct jpeg_source_mgr *)
            (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
            sizeof(wx_source_mgr));
    }
    src = (wx_src_ptr) cinfo->src;
    src->pub.bytes_in_buffer = 0; 
    src->buffer = new JOCTET[JPEG_IO_BUFFER_SIZE];
    src->pub.next_input_byte = NULL; 
    src->stream = &infile;

    src->pub.init_source = wx_init_source;
    src->pub.fill_input_buffer = wx_fill_input_buffer;
    src->pub.skip_input_data = wx_skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart; 
    src->pub.term_source = wx_term_source;
}

} 
static inline void wx_cmyk_to_rgb(unsigned char* rgb, const unsigned char* cmyk)
{
    int k = 255 - cmyk[3];
    int k2 = cmyk[3];
    int c;

    c = k + k2 * (255 - cmyk[0]) / 255;
    rgb[0] = (unsigned char)((c > 255) ? 0 : (255 - c));

    c = k + k2 * (255 - cmyk[1]) / 255;
    rgb[1] = (unsigned char)((c > 255) ? 0 : (255 - c));

    c = k + k2 * (255 - cmyk[2]) / 255;
    rgb[2] = (unsigned char)((c > 255) ? 0 : (255 - c));
}

#ifdef __VISUALC__
    #pragma warning(disable:4611)
#endif 

bool wxJPEGHandler::LoadFile( wxImage *image, wxInputStream& stream, bool verbose, int WXUNUSED(index) )
{
    wxCHECK_MSG( image, false, "NULL image pointer" );

    struct jpeg_decompress_struct cinfo;
    wx_error_mgr jerr;
    unsigned char *ptr;

        const unsigned maxWidth = image->GetOptionInt(wxIMAGE_OPTION_MAX_WIDTH),
                   maxHeight = image->GetOptionInt(wxIMAGE_OPTION_MAX_HEIGHT);
    image->Destroy();

    cinfo.err = jpeg_std_error( &jerr );
    jerr.error_exit = wx_error_exit;

    if (!verbose)
        cinfo.err->output_message = wx_ignore_message;

    
    if (setjmp(jerr.setjmp_buffer)) {
      
      if (verbose)
      {
        wxLogError(_("JPEG: Couldn't load - file is probably corrupted."));
      }
      (cinfo.src->term_source)(&cinfo);
      jpeg_destroy_decompress(&cinfo);
      if (image->IsOk()) image->Destroy();
      return false;
    }

    jpeg_create_decompress( &cinfo );
    wx_jpeg_io_src( &cinfo, stream );
    jpeg_read_header( &cinfo, TRUE );

    int bytesPerPixel;
    if ((cinfo.out_color_space == JCS_CMYK) || (cinfo.out_color_space == JCS_YCCK))
    {
        cinfo.out_color_space = JCS_CMYK;
        bytesPerPixel = 4;
    }
    else     {
        cinfo.out_color_space = JCS_RGB;
        bytesPerPixel = 3;
    }

        if ( maxWidth > 0 || maxHeight > 0 )
    {
        unsigned& scale = cinfo.scale_denom;
        while ( (maxWidth && (cinfo.image_width / scale > maxWidth)) ||
                    (maxHeight && (cinfo.image_height / scale > maxHeight)) )
        {
            scale *= 2;
        }
    }

    jpeg_start_decompress( &cinfo );

    image->Create( cinfo.output_width, cinfo.output_height );
    if (!image->IsOk()) {
        jpeg_finish_decompress( &cinfo );
        jpeg_destroy_decompress( &cinfo );
        return false;
    }
    image->SetMask( false );
    ptr = image->GetData();

    unsigned stride = cinfo.output_width * bytesPerPixel;
    JSAMPARRAY tempbuf = (*cinfo.mem->alloc_sarray)
                            ((j_common_ptr) &cinfo, JPOOL_IMAGE, stride, 1 );

    while ( cinfo.output_scanline < cinfo.output_height )
    {
        jpeg_read_scanlines( &cinfo, tempbuf, 1 );
        if (cinfo.out_color_space == JCS_RGB)
        {
            memcpy( ptr, tempbuf[0], stride );
            ptr += stride;
        }
        else         {
            const unsigned char* inptr = (const unsigned char*) tempbuf[0];
            for (size_t i = 0; i < cinfo.output_width; i++)
            {
                wx_cmyk_to_rgb(ptr, inptr);
                ptr += 3;
                inptr += 4;
            }
        }
    }

        if ( cinfo.saw_JFIF_marker )
    {
        image->SetOption(wxIMAGE_OPTION_RESOLUTIONX, cinfo.X_density);
        image->SetOption(wxIMAGE_OPTION_RESOLUTIONY, cinfo.Y_density);

                        image->SetOption(wxIMAGE_OPTION_RESOLUTIONUNIT, cinfo.density_unit);
    }

    if ( cinfo.image_width != cinfo.output_width || cinfo.image_height != cinfo.output_height )
    {
                image->SetOption(wxIMAGE_OPTION_ORIGINAL_WIDTH, cinfo.image_width);
        image->SetOption(wxIMAGE_OPTION_ORIGINAL_HEIGHT, cinfo.image_height);
    }

    jpeg_finish_decompress( &cinfo );
    jpeg_destroy_decompress( &cinfo );
    return true;
}

typedef struct {
    struct jpeg_destination_mgr pub;

    wxOutputStream *stream;
    JOCTET * buffer;
} wx_destination_mgr;

typedef wx_destination_mgr * wx_dest_ptr;

#define OUTPUT_BUF_SIZE  4096    

extern "C"
{

CPP_METHODDEF(void) wx_init_destination (j_compress_ptr cinfo)
{
    wx_dest_ptr dest = (wx_dest_ptr) cinfo->dest;

    
    dest->buffer = (JOCTET *)
        (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
        OUTPUT_BUF_SIZE * sizeof(JOCTET));
    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}

CPP_METHODDEF(wxjpeg_boolean) wx_empty_output_buffer (j_compress_ptr cinfo)
{
    wx_dest_ptr dest = (wx_dest_ptr) cinfo->dest;

    dest->stream->Write(dest->buffer, OUTPUT_BUF_SIZE);
    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
    return TRUE;
}

CPP_METHODDEF(void) wx_term_destination (j_compress_ptr cinfo)
{
    wx_dest_ptr dest = (wx_dest_ptr) cinfo->dest;
    size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;
    
    if (datacount > 0)
        dest->stream->Write(dest->buffer, datacount);
}

GLOBAL(void) wx_jpeg_io_dest (j_compress_ptr cinfo, wxOutputStream& outfile)
{
    wx_dest_ptr dest;

    if (cinfo->dest == NULL) {    
        cinfo->dest = (struct jpeg_destination_mgr *)
            (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
            sizeof(wx_destination_mgr));
    }

    dest = (wx_dest_ptr) cinfo->dest;
    dest->pub.init_destination = wx_init_destination;
    dest->pub.empty_output_buffer = wx_empty_output_buffer;
    dest->pub.term_destination = wx_term_destination;
    dest->stream = &outfile;
}

} 
bool wxJPEGHandler::SaveFile( wxImage *image, wxOutputStream& stream, bool verbose )
{
    struct jpeg_compress_struct cinfo;
    wx_error_mgr jerr;
    JSAMPROW row_pointer[1];    
    JSAMPLE *image_buffer;
    int stride;                

    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = wx_error_exit;

    if (!verbose)
        cinfo.err->output_message = wx_ignore_message;

    
    if (setjmp(jerr.setjmp_buffer))
    {
        
         if (verbose)
         {
            wxLogError(_("JPEG: Couldn't save image."));
         }
         jpeg_destroy_compress(&cinfo);
         return false;
    }

    jpeg_create_compress(&cinfo);
    wx_jpeg_io_dest(&cinfo, stream);

    cinfo.image_width = image->GetWidth();
    cinfo.image_height = image->GetHeight();
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);

                            if (image->HasOption(wxIMAGE_OPTION_QUALITY))
        jpeg_set_quality(&cinfo, image->GetOptionInt(wxIMAGE_OPTION_QUALITY), TRUE);

        int resX, resY;
    wxImageResolution res = GetResolutionFromOptions(*image, &resX, &resY);
    if ( res != wxIMAGE_RESOLUTION_NONE )
    {
        cinfo.X_density = resX;
        cinfo.Y_density = resY;

                        cinfo.density_unit = res;
    }

    jpeg_start_compress(&cinfo, TRUE);

    stride = cinfo.image_width * 3;    
    image_buffer = image->GetData();
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &image_buffer[cinfo.next_scanline * stride];
        jpeg_write_scanlines( &cinfo, row_pointer, 1 );
    }
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return true;
}

#ifdef __VISUALC__
    #pragma warning(default:4611)
#endif 

bool wxJPEGHandler::DoCanRead( wxInputStream& stream )
{
    unsigned char hdr[2];

    if ( !stream.Read(hdr, WXSIZEOF(hdr)) )             return false;

    return hdr[0] == 0xFF && hdr[1] == 0xD8;
}

#endif   
 wxVersionInfo wxJPEGHandler::GetLibraryVersionInfo()
{
    return wxVersionInfo("libjpeg", JPEG_LIB_VERSION/10, JPEG_LIB_VERSION%10);
}

#endif   