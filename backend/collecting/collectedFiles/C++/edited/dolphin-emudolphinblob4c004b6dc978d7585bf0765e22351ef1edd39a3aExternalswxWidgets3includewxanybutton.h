
#ifndef _WX_ANYBUTTON_H_BASE_
#define _WX_ANYBUTTON_H_BASE_

#include "wx/defs.h"

#ifdef wxHAS_ANY_BUTTON


#define wxBU_LEFT            0x0040
#define wxBU_TOP             0x0080
#define wxBU_RIGHT           0x0100
#define wxBU_BOTTOM          0x0200
#define wxBU_ALIGN_MASK      ( wxBU_LEFT | wxBU_TOP | wxBU_RIGHT | wxBU_BOTTOM )

#define wxBU_NOAUTODRAW      0x0000
#define wxBU_AUTODRAW        0x0004

#define wxBU_EXACTFIT        0x0001

#define wxBU_NOTEXT          0x0002


#include "wx/bitmap.h"
#include "wx/control.h"


class WXDLLIMPEXP_CORE wxAnyButtonBase : public wxControl
{
public:
    wxAnyButtonBase() { }

            void SetBitmap(const wxBitmap& bitmap, wxDirection dir = wxLEFT)
    {
        SetBitmapLabel(bitmap);
        SetBitmapPosition(dir);
    }

    wxBitmap GetBitmap() const { return DoGetBitmap(State_Normal); }

                                    void SetBitmapLabel(const wxBitmap& bitmap)
        { DoSetBitmap(bitmap, State_Normal); }
    void SetBitmapPressed(const wxBitmap& bitmap)
        { DoSetBitmap(bitmap, State_Pressed); }
    void SetBitmapDisabled(const wxBitmap& bitmap)
        { DoSetBitmap(bitmap, State_Disabled); }
    void SetBitmapCurrent(const wxBitmap& bitmap)
        { DoSetBitmap(bitmap, State_Current); }
    void SetBitmapFocus(const wxBitmap& bitmap)
        { DoSetBitmap(bitmap, State_Focused); }

    wxBitmap GetBitmapLabel() const { return DoGetBitmap(State_Normal); }
    wxBitmap GetBitmapPressed() const { return DoGetBitmap(State_Pressed); }
    wxBitmap GetBitmapDisabled() const { return DoGetBitmap(State_Disabled); }
    wxBitmap GetBitmapCurrent() const { return DoGetBitmap(State_Current); }
    wxBitmap GetBitmapFocus() const { return DoGetBitmap(State_Focused); }


        void SetBitmapMargins(wxCoord x, wxCoord y) { DoSetBitmapMargins(x, y); }
    void SetBitmapMargins(const wxSize& sz) { DoSetBitmapMargins(sz.x, sz.y); }
    wxSize GetBitmapMargins() { return DoGetBitmapMargins(); }

            void SetBitmapPosition(wxDirection dir);


                virtual bool ShouldInheritColours() const wxOVERRIDE { return false; }

    #if WXWIN_COMPATIBILITY_2_8
    void SetImageLabel(const wxBitmap& bitmap) { SetBitmap(bitmap); }
    void SetImageMargins(wxCoord x, wxCoord y) { SetBitmapMargins(x, y); }
#endif 
                wxBitmap GetBitmapSelected() const { return GetBitmapPressed(); }
    wxBitmap GetBitmapHover() const { return GetBitmapCurrent(); }

    void SetBitmapSelected(const wxBitmap& bitmap) { SetBitmapPressed(bitmap); }
    void SetBitmapHover(const wxBitmap& bitmap) { SetBitmapCurrent(bitmap); }


                        enum State
    {
        State_Normal,
        State_Current,            State_Pressed,            State_Disabled,
        State_Focused,
        State_Max
    };

            virtual State GetNormalState() const
    {
        return State_Normal;
    }

            bool DontShowLabel() const
    {
        return HasFlag(wxBU_NOTEXT) || GetLabel().empty();
    }

        bool ShowsLabel() const
    {
        return !DontShowLabel();
    }

protected:
        virtual wxBorder GetDefaultBorder() const wxOVERRIDE { return wxBORDER_NONE; }

    virtual wxBitmap DoGetBitmap(State WXUNUSED(which)) const
        { return wxBitmap(); }
    virtual void DoSetBitmap(const wxBitmap& WXUNUSED(bitmap),
                             State WXUNUSED(which))
        { }

    virtual wxSize DoGetBitmapMargins() const
        { return wxSize(0, 0); }

    virtual void DoSetBitmapMargins(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y))
        { }

    virtual void DoSetBitmapPosition(wxDirection WXUNUSED(dir))
        { }

    virtual bool DoGetAuthNeeded() const { return false; }
    virtual void DoSetAuthNeeded(bool WXUNUSED(show)) { }


    wxDECLARE_NO_COPY_CLASS(wxAnyButtonBase);
};

#if defined(__WXUNIVERSAL__)
    #include "wx/univ/anybutton.h"
#elif defined(__WXMSW__)
    #include "wx/msw/anybutton.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/anybutton.h"
#elif defined(__WXMAC__)
    #include "wx/osx/anybutton.h"
#elif defined(__WXQT__)
    #include "wx/qt/anybutton.h"
#else
    typedef wxAnyButtonBase wxAnyButton;
#endif

#endif 
#endif 