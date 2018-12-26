
#ifndef _WX_MSW_OWNERDRAWNBUTTON_H_
#define _WX_MSW_OWNERDRAWNBUTTON_H_


class WXDLLIMPEXP_CORE wxMSWOwnerDrawnButtonBase
{
protected:
        wxMSWOwnerDrawnButtonBase(wxWindow* win) :
        m_win(win)
    {
        m_isPressed =
        m_isHot = false;
    }

                                        #if wxCHECK_GCC_VERSION(4, 0)
    ~wxMSWOwnerDrawnButtonBase() { }
#endif 
            void MSWMakeOwnerDrawnIfNecessary(const wxColour& colFg);

        bool MSWIsOwnerDrawn() const;

            bool MSWDrawButton(WXDRAWITEMSTRUCT *item);


    
        virtual int MSWGetButtonStyle() const = 0;

            virtual void MSWOnButtonResetOwnerDrawn() { }

                    virtual int MSWGetButtonCheckedFlag() const = 0;

            virtual void
        MSWDrawButtonBitmap(wxDC& dc, const wxRect& rect, int flags) = 0;


private:
        void MSWMakeOwnerDrawn(bool ownerDrawn);

        void OnMouseEnterOrLeave(wxMouseEvent& event);
    void OnMouseLeft(wxMouseEvent& event);
    void OnFocus(wxFocusEvent& event);


        wxWindow* const m_win;

        bool m_isPressed;

        bool m_isHot;


    wxDECLARE_NO_COPY_CLASS(wxMSWOwnerDrawnButtonBase);
};

template <class T>
class wxMSWOwnerDrawnButton
    : public T,
      private wxMSWOwnerDrawnButtonBase
{
private:
    typedef T Base;

public:
    wxMSWOwnerDrawnButton() : wxMSWOwnerDrawnButtonBase(this)
    {
    }

    virtual bool SetForegroundColour(const wxColour& colour) wxOVERRIDE
    {
        if ( !Base::SetForegroundColour(colour) )
            return false;

        MSWMakeOwnerDrawnIfNecessary(colour);

        return true;
    }

    virtual bool MSWOnDraw(WXDRAWITEMSTRUCT *item) wxOVERRIDE
    {
        return MSWDrawButton(item) || Base::MSWOnDraw(item);
    }

protected:
    bool IsOwnerDrawn() const { return MSWIsOwnerDrawn(); }
};

#endif 