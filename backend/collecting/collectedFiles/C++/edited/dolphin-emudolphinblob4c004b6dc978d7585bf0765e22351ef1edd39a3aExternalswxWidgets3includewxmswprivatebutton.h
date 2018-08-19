
#ifndef _WX_MSW_PRIVATE_BUTTON_H_
#define _WX_MSW_PRIVATE_BUTTON_H_

#ifndef BS_PUSHLIKE
    #define BS_PUSHLIKE 0x00001000L
#endif

#ifndef BST_UNCHECKED
    #define BST_UNCHECKED 0x0000
#endif

#ifndef BST_CHECKED
    #define BST_CHECKED 0x0001
#endif

#ifndef BST_INDETERMINATE
    #define BST_INDETERMINATE 0x0002
#endif

namespace wxMSWButton
{

inline int GetMultilineStyle(const wxString& label)
{
    return label.find(wxT('\n')) == wxString::npos ? 0 : BS_MULTILINE;
}

void UpdateMultilineStyle(HWND hwnd, const wxString& label);

enum
{
    Size_AuthNeeded = 1,
    Size_ExactFit   = 2
};


WXDLLIMPEXP_CORE wxSize
GetFittingSize(wxWindow *win, const wxSize& sizeLabel, int flags = 0);

wxSize ComputeBestFittingSize(wxControl *btn, int flags = 0);

wxSize IncreaseToStdSizeAndCache(wxControl *btn, const wxSize& size);

inline wxSize ComputeBestSize(wxControl *btn, int flags = 0)
{
    return IncreaseToStdSizeAndCache(btn, ComputeBestFittingSize(btn, flags));
}

} 
#endif 
