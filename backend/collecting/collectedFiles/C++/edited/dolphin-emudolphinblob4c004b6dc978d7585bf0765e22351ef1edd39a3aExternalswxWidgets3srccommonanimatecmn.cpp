


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_ANIMATIONCTRL

#include "wx/animate.h"
#include "wx/bitmap.h"
#include "wx/log.h"
#include "wx/brush.h"
#include "wx/image.h"
#include "wx/dcmemory.h"

const char wxAnimationCtrlNameStr[] = "animationctrl";

wxAnimation wxNullAnimation;

wxIMPLEMENT_ABSTRACT_CLASS(wxAnimationBase, wxObject);
wxIMPLEMENT_ABSTRACT_CLASS(wxAnimationCtrlBase, wxControl);



void wxAnimationCtrlBase::UpdateStaticImage()
{
    if (!m_bmpStaticReal.IsOk() || !m_bmpStatic.IsOk())
        return;

        const wxSize &sz = GetClientSize();
    if (sz.GetWidth() != m_bmpStaticReal.GetWidth() ||
        sz.GetHeight() != m_bmpStaticReal.GetHeight())
    {
        if (!m_bmpStaticReal.IsOk() ||
            m_bmpStaticReal.GetWidth() != sz.GetWidth() ||
            m_bmpStaticReal.GetHeight() != sz.GetHeight())
        {
                        if (!m_bmpStaticReal.Create(sz.GetWidth(), sz.GetHeight(),
                                        m_bmpStatic.GetDepth()))
            {
                wxLogDebug(wxT("Cannot create the static bitmap"));
                m_bmpStatic = wxNullBitmap;
                return;
            }
        }

        if (m_bmpStatic.GetWidth() <= sz.GetWidth() &&
            m_bmpStatic.GetHeight() <= sz.GetHeight())
        {
                        wxBrush brush(GetBackgroundColour());
            wxMemoryDC dc;
            dc.SelectObject(m_bmpStaticReal);
            dc.SetBackground(brush);
            dc.Clear();

                        dc.DrawBitmap(m_bmpStatic,
                        (sz.GetWidth()-m_bmpStatic.GetWidth())/2,
                        (sz.GetHeight()-m_bmpStatic.GetHeight())/2,
                        true  );
        }
        else
        {
                        wxImage temp(m_bmpStatic.ConvertToImage());
            temp.Rescale(sz.GetWidth(), sz.GetHeight(), wxIMAGE_QUALITY_HIGH);
            m_bmpStaticReal = wxBitmap(temp);
        }
    }
}

void wxAnimationCtrlBase::SetInactiveBitmap(const wxBitmap &bmp)
{
    m_bmpStatic = bmp;
    m_bmpStaticReal = bmp;

            if ( !IsPlaying() )
        DisplayStaticImage();
}

#endif      