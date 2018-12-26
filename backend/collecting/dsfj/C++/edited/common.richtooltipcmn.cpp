


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_RICHTOOLTIP

#ifndef WX_PRECOMP
    #include "wx/icon.h"
#endif 
#include "wx/private/richtooltip.h"


wxRichToolTip::wxRichToolTip(const wxString& title,
                             const wxString& message) :
    m_impl(wxRichToolTipImpl::Create(title, message))
{
}

void
wxRichToolTip::SetBackgroundColour(const wxColour& col, const wxColour& colEnd)
{
    m_impl->SetBackgroundColour(col, colEnd);
}

void wxRichToolTip::SetIcon(int icon)
{
    m_impl->SetStandardIcon(icon);
}

void wxRichToolTip::SetIcon(const wxIcon& icon)
{
    m_impl->SetCustomIcon(icon);
}

void wxRichToolTip::SetTimeout(unsigned milliseconds,
                               unsigned millisecondsDelay)
{
    m_impl->SetTimeout(milliseconds, millisecondsDelay);
}

void wxRichToolTip::SetTipKind(wxTipKind tipKind)
{
    m_impl->SetTipKind(tipKind);
}

void wxRichToolTip::SetTitleFont(const wxFont& font)
{
    m_impl->SetTitleFont(font);
}

void wxRichToolTip::ShowFor(wxWindow* win, const wxRect* rect)
{
    wxCHECK_RET( win, wxS("Must have a valid window") );

    m_impl->ShowFor(win, rect);
}

wxRichToolTip::~wxRichToolTip()
{
    delete m_impl;
}

#endif 