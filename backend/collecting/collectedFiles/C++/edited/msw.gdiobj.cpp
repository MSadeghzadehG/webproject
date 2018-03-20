
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/gdiobj.h"

#ifndef WX_PRECOMP
    #include <stdio.h>
    #include "wx/list.h"
    #include "wx/utils.h"
    #include "wx/app.h"
#endif

#include "wx/msw/private.h"

#define M_GDIDATA static_cast<wxGDIRefData*>(m_refData)


