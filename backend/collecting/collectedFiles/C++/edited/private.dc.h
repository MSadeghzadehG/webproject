
#ifndef _MSW_PRIVATE_DC_H_
#define _MSW_PRIVATE_DC_H_

#include "wx/msw/dc.h"
#include "wx/msw/wrapwin.h"

namespace wxMSWImpl
{


class wxTextColoursChanger
{
public:
    wxTextColoursChanger(HDC hdc, const wxMSWDCImpl& dc)
        : m_hdc(hdc)
    {
        Change(dc.GetTextForeground(), dc.GetTextBackground());
    }

    wxTextColoursChanger(HDC hdc, const wxColour& colFg, const wxColour& colBg)
        : m_hdc(hdc)
    {
        Change(colFg, colBg);
    }

    wxTextColoursChanger(HDC hdc, COLORREF colFg, COLORREF colBg)
        : m_hdc(hdc)
    {
        Change(colFg, colBg);
    }

    ~wxTextColoursChanger()
    {
        if ( m_oldColFg != CLR_INVALID )
            ::SetTextColor(m_hdc, m_oldColFg);
        if ( m_oldColBg != CLR_INVALID )
            ::SetBkColor(m_hdc, m_oldColBg);
    }

protected:
            wxTextColoursChanger(HDC hdc)
        : m_hdc(hdc)
    {
        m_oldColFg =
        m_oldColBg = CLR_INVALID;
    }

    void Change(const wxColour& colFg, const wxColour& colBg)
    {
        Change(colFg.IsOk() ? colFg.GetPixel() : CLR_INVALID,
               colBg.IsOk() ? colBg.GetPixel() : CLR_INVALID);
    }

    void Change(COLORREF colFg, COLORREF colBg)
    {
        if ( colFg != CLR_INVALID )
        {
            m_oldColFg = ::SetTextColor(m_hdc, colFg);
            if ( m_oldColFg == CLR_INVALID )
            {
                wxLogLastError(wxT("SetTextColor"));
            }
        }
        else
        {
            m_oldColFg = CLR_INVALID;
        }

        if ( colBg != CLR_INVALID )
        {
            m_oldColBg = ::SetBkColor(m_hdc, colBg);
            if ( m_oldColBg == CLR_INVALID )
            {
                wxLogLastError(wxT("SetBkColor"));
            }
        }
        else
        {
            m_oldColBg = CLR_INVALID;
        }
    }

private:
    const HDC m_hdc;
    COLORREF m_oldColFg,
             m_oldColBg;

    wxDECLARE_NO_COPY_CLASS(wxTextColoursChanger);
};

class wxBkModeChanger
{
public:
        wxBkModeChanger(HDC hdc, int mode)
        : m_hdc(hdc)
    {
        Change(mode);
    }

    ~wxBkModeChanger()
    {
        if ( m_oldMode )
            ::SetBkMode(m_hdc, m_oldMode);
    }

protected:
            wxBkModeChanger(HDC hdc) : m_hdc(hdc) { m_oldMode = 0; }

    void Change(int mode)
    {
        m_oldMode = ::SetBkMode(m_hdc, mode == wxBRUSHSTYLE_TRANSPARENT
                                        ? TRANSPARENT
                                        : OPAQUE);
        if ( !m_oldMode )
        {
            wxLogLastError(wxT("SetBkMode"));
        }
    }

private:
    const HDC m_hdc;
    int m_oldMode;

    wxDECLARE_NO_COPY_CLASS(wxBkModeChanger);
};

} 
#endif 
