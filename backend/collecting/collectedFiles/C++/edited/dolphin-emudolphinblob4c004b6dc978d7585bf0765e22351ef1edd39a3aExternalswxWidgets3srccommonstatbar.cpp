


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_STATUSBAR

#include "wx/statusbr.h"

#ifndef WX_PRECOMP
    #include "wx/frame.h"
#endif 
const char wxStatusBarNameStr[] = "statusBar";


bool wxStatusBarPane::SetText(const wxString& text)
{
    if ( text == m_text )
        return false;

    
    if ( !m_arrStack.empty() )
    {
        m_arrStack.back() = text;
    }

    m_text = text;

    return true;
}

bool wxStatusBarPane::PushText(const wxString& text)
{
        m_arrStack.push_back(m_text);

        if ( text == m_text )
        return false;

    m_text = text;

    return true;
}

bool wxStatusBarPane::PopText()
{
    wxCHECK_MSG( !m_arrStack.empty(), false, "no status message to pop" );

    const wxString text = m_arrStack.back();

    m_arrStack.pop_back();

    if ( text == m_text )
        return false;

    m_text = text;

    return true;
}


wxIMPLEMENT_DYNAMIC_CLASS(wxStatusBar, wxWindow);

#include "wx/arrimpl.cpp" WX_DEFINE_EXPORTED_OBJARRAY(wxStatusBarPaneArray)



wxStatusBarBase::wxStatusBarBase()
{
    m_bSameWidthForAllPanes = true;
}

wxStatusBarBase::~wxStatusBarBase()
{
            wxFrame *frame = wxDynamicCast(GetParent(), wxFrame);
    if ( frame && frame->GetStatusBar() == this )
        frame->SetStatusBar(NULL);
}


void wxStatusBarBase::SetFieldsCount(int number, const int *widths)
{
    wxCHECK_RET( number > 0, wxT("invalid field number in SetFieldsCount") );

    if ( (size_t)number > m_panes.GetCount() )
    {
        wxStatusBarPane newPane;

                        for (size_t i = m_panes.GetCount(); i < (size_t)number; ++i)
            m_panes.Add(newPane);
    }
    else if ( (size_t)number < m_panes.GetCount() )
    {
                m_panes.RemoveAt(number, m_panes.GetCount()-number);
    }

        SetStatusWidths(number, widths);
}

void wxStatusBarBase::SetStatusWidths(int WXUNUSED_UNLESS_DEBUG(n),
                                    const int widths[])
{
    wxASSERT_MSG( (size_t)n == m_panes.GetCount(), wxT("field number mismatch") );

    if (widths == NULL)
    {
                        m_bSameWidthForAllPanes = true;
    }
    else
    {
        for ( size_t i = 0; i < m_panes.GetCount(); i++ )
            m_panes[i].SetWidth(widths[i]);

        m_bSameWidthForAllPanes = false;
    }

        Refresh();
}

void wxStatusBarBase::SetStatusStyles(int WXUNUSED_UNLESS_DEBUG(n),
                                    const int styles[])
{
    wxCHECK_RET( styles, wxT("NULL pointer in SetStatusStyles") );

    wxASSERT_MSG( (size_t)n == m_panes.GetCount(), wxT("field number mismatch") );

    for ( size_t i = 0; i < m_panes.GetCount(); i++ )
        m_panes[i].SetStyle(styles[i]);

        Refresh();
}

wxArrayInt wxStatusBarBase::CalculateAbsWidths(wxCoord widthTotal) const
{
    wxArrayInt widths;

    if ( m_bSameWidthForAllPanes )
    {
                                        int widthToUse = widthTotal;

        for ( size_t i = m_panes.GetCount(); i > 0; i-- )
        {
                                    int w = widthToUse / i;
            widths.Add(w);
            widthToUse -= w;
        }
    }
    else     {
                        size_t nTotalWidth = 0,
            nVarCount = 0,
            i;

        for ( i = 0; i < m_panes.GetCount(); i++ )
        {
            if ( m_panes[i].GetWidth() >= 0 )
                nTotalWidth += m_panes[i].GetWidth();
            else
                nVarCount += -m_panes[i].GetWidth();
        }

                int widthExtra = widthTotal - nTotalWidth;

                for ( i = 0; i < m_panes.GetCount(); i++ )
        {
            if ( m_panes[i].GetWidth() >= 0 )
                widths.Add(m_panes[i].GetWidth());
            else
            {
                int nVarWidth = widthExtra > 0 ? (widthExtra * (-m_panes[i].GetWidth())) / nVarCount : 0;
                nVarCount += m_panes[i].GetWidth();
                widthExtra -= nVarWidth;
                widths.Add(nVarWidth);
            }
        }
    }

    return widths;
}


void wxStatusBarBase::SetStatusText(const wxString& text, int number)
{
    wxCHECK_RET( (unsigned)number < m_panes.size(),
                    "invalid status bar field index" );

    if ( m_panes[number].SetText(text) )
        DoUpdateStatusText(number);
}

wxString wxStatusBarBase::GetStatusText(int number) const
{
    wxCHECK_MSG( (unsigned)number < m_panes.size(), wxString(),
                    "invalid status bar field index" );

    return m_panes[number].GetText();
}

void wxStatusBarBase::SetEllipsizedFlag(int number, bool isEllipsized)
{
    wxCHECK_RET( (unsigned)number < m_panes.size(),
                    "invalid status bar field index" );

    m_panes[number].SetIsEllipsized(isEllipsized);
}


void wxStatusBarBase::PushStatusText(const wxString& text, int number)
{
    wxCHECK_RET( (unsigned)number < m_panes.size(),
                    "invalid status bar field index" );

    if ( m_panes[number].PushText(text) )
        DoUpdateStatusText(number);
}

void wxStatusBarBase::PopStatusText(int number)
{
    wxCHECK_RET( (unsigned)number < m_panes.size(),
                    "invalid status bar field index" );

    if ( m_panes[number].PopText() )
        DoUpdateStatusText(number);
}

#endif 