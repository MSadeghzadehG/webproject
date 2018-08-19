
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_STREAMS

#if wxUSE_ZLIB
#include "wx/zstream.h"
#endif


void wxUseFilterClasses()
{
#if wxUSE_ZLIB
    wxZlibClassFactory();
    wxGzipClassFactory();
#endif
}

#endif 