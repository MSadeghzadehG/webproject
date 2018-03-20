
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_IMAGE

#ifndef WX_PRECOMP
    #include "wx/image.h"
#endif


void wxInitAllImageHandlers()
{
#if wxUSE_LIBPNG
  wxImage::AddHandler( new wxPNGHandler );
#endif
#if wxUSE_LIBJPEG
  wxImage::AddHandler( new wxJPEGHandler );
#endif
#if wxUSE_LIBTIFF
  wxImage::AddHandler( new wxTIFFHandler );
#endif
#if wxUSE_GIF
  wxImage::AddHandler( new wxGIFHandler );
#endif
#if wxUSE_PNM
  wxImage::AddHandler( new wxPNMHandler );
#endif
#if wxUSE_PCX
  wxImage::AddHandler( new wxPCXHandler );
#endif
#if wxUSE_IFF
  wxImage::AddHandler( new wxIFFHandler );
#endif
#if wxUSE_ICO_CUR
  wxImage::AddHandler( new wxICOHandler );
  wxImage::AddHandler( new wxCURHandler );
  wxImage::AddHandler( new wxANIHandler );
#endif
#if wxUSE_TGA
  wxImage::AddHandler( new wxTGAHandler );
#endif
#if wxUSE_XPM
  wxImage::AddHandler( new wxXPMHandler );
#endif
}

#endif 