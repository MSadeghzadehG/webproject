
#ifndef _WX_MSW_WRAPSHL_H_
#define _WX_MSW_WRAPSHL_H_

#include "wx/msw/wrapwin.h"

#include <shlobj.h>

#include "wx/msw/winundef.h"

#include "wx/log.h"


class wxItemIdList
{
public:
        wxItemIdList(LPITEMIDLIST pidl)
    {
        m_pidl = pidl;
    }

    static void Free(LPITEMIDLIST pidl)
    {
        if ( pidl )
        {
            LPMALLOC pMalloc;
            SHGetMalloc(&pMalloc);
            if ( pMalloc )
            {
                pMalloc->Free(pidl);
                pMalloc->Release();
            }
            else
            {
                wxLogLastError(wxT("SHGetMalloc"));
            }
        }
    }

    ~wxItemIdList()
    {
        Free(m_pidl);
    }

        operator LPITEMIDLIST() const { return m_pidl; }

        wxString GetPath() const
    {
        wxString path;
        if ( !SHGetPathFromIDList(m_pidl, wxStringBuffer(path, MAX_PATH)) )
        {
            wxLogLastError(wxT("SHGetPathFromIDList"));
        }

        return path;
    }

private:
    LPITEMIDLIST m_pidl;

    wxDECLARE_NO_COPY_CLASS(wxItemIdList);
};

extern bool wxEnableFileNameAutoComplete(HWND hwnd);

#endif 
