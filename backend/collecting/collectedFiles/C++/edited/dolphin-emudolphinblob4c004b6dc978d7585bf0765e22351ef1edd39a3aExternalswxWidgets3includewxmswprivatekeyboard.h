
#ifndef _WX_MSW_PRIVATE_KEYBOARD_H_
#define _WX_MSW_PRIVATE_KEYBOARD_H_

#include "wx/defs.h"

namespace wxMSWKeyboard
{


WXDLLIMPEXP_CORE int VKToWX(WXWORD vk, WXLPARAM lParam = 0, wchar_t *uc = NULL);

WXDLLIMPEXP_CORE WXWORD WXToVK(int id, bool *isExtended = NULL);

} 
#endif 