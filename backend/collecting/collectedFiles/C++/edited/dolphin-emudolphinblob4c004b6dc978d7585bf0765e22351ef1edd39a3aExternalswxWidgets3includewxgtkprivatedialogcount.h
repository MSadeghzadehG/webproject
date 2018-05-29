
#ifndef _WX_GTK_PRIVATE_DIALOGCOUNT_H_
#define _WX_GTK_PRIVATE_DIALOGCOUNT_H_

#include "wx/defs.h"

extern int wxOpenModalDialogsCount;


class wxOpenModalDialogLocker
{
public:
    wxOpenModalDialogLocker()
    {
        wxOpenModalDialogsCount++;
    }

    ~wxOpenModalDialogLocker()
    {
        wxOpenModalDialogsCount--;
    }

private:
    wxDECLARE_NO_COPY_CLASS(wxOpenModalDialogLocker);
};

#endif 