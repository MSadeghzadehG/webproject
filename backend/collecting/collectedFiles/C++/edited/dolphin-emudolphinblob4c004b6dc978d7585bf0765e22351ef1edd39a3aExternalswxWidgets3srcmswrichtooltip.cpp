


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_RICHTOOLTIP

#ifndef WX_PRECOMP
    #include "wx/treectrl.h"
#endif 
#include "wx/private/richtooltip.h"
#include "wx/generic/private/richtooltip.h"
#include "wx/msw/private.h"
#include "wx/msw/uxtheme.h"


#ifndef TTI_NONE
enum
{
    TTI_NONE,
    TTI_INFO,
    TTI_WARNING,
    TTI_ERROR
};
#endif 
#ifndef Edit_ShowBalloonTip
struct EDITBALLOONTIP
{
    DWORD cbStruct;
    LPCWSTR pszTitle;
    LPCWSTR pszText;
    int ttiIcon;
};

#define Edit_ShowBalloonTip(hwnd, pebt) \
    (BOOL)::SendMessage((hwnd), 0x1503 , 0, (LPARAM)(pebt))

#endif 

class wxRichToolTipMSWImpl : public wxRichToolTipGenericImpl
{
public:
    wxRichToolTipMSWImpl(const wxString& title, const wxString& message) :
        wxRichToolTipGenericImpl(title, message)
    {
                m_canUseNative = true;

        m_ttiIcon = TTI_NONE;
    }

    virtual void SetBackgroundColour(const wxColour& col,
                                     const wxColour& colEnd)
    {
                m_canUseNative = false;

        wxRichToolTipGenericImpl::SetBackgroundColour(col, colEnd);
    }

    virtual void SetCustomIcon(const wxIcon& icon)
    {
                m_canUseNative = false;

        wxRichToolTipGenericImpl::SetCustomIcon(icon);
    }

    virtual void SetStandardIcon(int icon)
    {
        wxRichToolTipGenericImpl::SetStandardIcon(icon);
        if ( !m_canUseNative )
            return;

        switch ( icon & wxICON_MASK )
        {
            case wxICON_WARNING:
                m_ttiIcon = TTI_WARNING;
                break;

            case wxICON_ERROR:
                m_ttiIcon = TTI_ERROR;
                break;

            case wxICON_INFORMATION:
                m_ttiIcon = TTI_INFO;
                break;

            case wxICON_QUESTION:
                wxFAIL_MSG("Question icon doesn't make sense for a tooltip");
                break;

            case wxICON_NONE:
                m_ttiIcon = TTI_NONE;
                break;
        }
    }

    virtual void SetTimeout(unsigned millisecondsTimeout,
                            unsigned millisecondsDelay)
    {
                        m_canUseNative = false;

        wxRichToolTipGenericImpl::SetTimeout(millisecondsTimeout,
                                             millisecondsDelay);
    }

    virtual void SetTipKind(wxTipKind tipKind)
    {
                if ( tipKind != wxTipKind_Auto )
            m_canUseNative = false;

        wxRichToolTipGenericImpl::SetTipKind(tipKind);
    }

    virtual void SetTitleFont(const wxFont& font)
    {
                m_canUseNative = false;

        wxRichToolTipGenericImpl::SetTitleFont(font);
    }

    virtual void ShowFor(wxWindow* win, const wxRect* rect)
    {
                                        if ( m_canUseNative && !rect )
        {
            wxTextCtrl* const text = wxDynamicCast(win, wxTextCtrl);
            if ( text )
            {
                EDITBALLOONTIP ebt;
                ebt.cbStruct = sizeof(EDITBALLOONTIP);
                ebt.pszTitle = m_title.wc_str();
                ebt.pszText = m_message.wc_str();
                ebt.ttiIcon = m_ttiIcon;
                if ( Edit_ShowBalloonTip(GetHwndOf(text), &ebt) )
                    return;
            }
        }

                                wxRichToolTipGenericImpl::ShowFor(win, rect);
    }

private:
            bool m_canUseNative;

        int m_ttiIcon;
};


wxRichToolTipImpl*
wxRichToolTipImpl::Create(const wxString& title, const wxString& message)
{
            if ( wxUxThemeEngine::GetIfActive() )
        return new wxRichToolTipMSWImpl(title, message);

    return new wxRichToolTipGenericImpl(title, message);
}

#endif 