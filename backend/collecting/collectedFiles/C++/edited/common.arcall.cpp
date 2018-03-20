
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_ARCHIVE_STREAMS

#if wxUSE_ZIPSTREAM
#include "wx/zipstrm.h"
#endif
#if wxUSE_TARSTREAM
#include "wx/tarstrm.h"
#endif


void wxUseArchiveClasses()
{
#if wxUSE_ZIPSTREAM
    wxZipClassFactory();
#endif
#if wxUSE_TARSTREAM
    wxTarClassFactory();
#endif
}

#endif 