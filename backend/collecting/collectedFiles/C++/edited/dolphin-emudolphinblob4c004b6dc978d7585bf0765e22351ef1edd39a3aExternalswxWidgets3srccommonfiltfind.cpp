
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_STREAMS

#include "wx/stream.h"


const wxFilterClassFactory *
wxFilterClassFactory::Find(const wxString& protocol, wxStreamProtocolType type)
{
    for (const wxFilterClassFactory *f = GetFirst(); f; f = f->GetNext())
        if (f->CanHandle(protocol, type))
            return f;

    return NULL;
}

const wxFilterClassFactory *wxFilterClassFactory::GetFirst()
{
    if (!sm_first)
        wxUseFilterClasses();
    return sm_first;
}

#endif 