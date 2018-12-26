


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_CONTROLS

#include "wx/control.h"

#ifndef WX_PRECOMP
    #include "wx/dc.h"
    #include "wx/log.h"
    #include "wx/radiobut.h"
    #include "wx/statbmp.h"
    #include "wx/bitmap.h"
    #include "wx/utils.h"           #include "wx/settings.h"
#endif

#include "wx/private/markupparser.h"

const char wxControlNameStr[] = "control";


wxControlBase::~wxControlBase()
{
    }

bool wxControlBase::Create(wxWindow *parent,
                           wxWindowID id,
                           const wxPoint &pos,
                           const wxSize &size,
                           long style,
                           const wxValidator& wxVALIDATOR_PARAM(validator),
                           const wxString &name)
{
    bool ret = wxWindow::Create(parent, id, pos, size, style, name);

#if wxUSE_VALIDATORS
    if ( ret )
        SetValidator(validator);
#endif 
    return ret;
}

bool wxControlBase::CreateControl(wxWindowBase *parent,
                                  wxWindowID id,
                                  const wxPoint& pos,
                                  const wxSize& size,
                                  long style,
                                  const wxValidator& validator,
                                  const wxString& name)
{
                wxCHECK_MSG( parent, false, wxT("all controls must have parents") );

    if ( !CreateBase(parent, id, pos, size, style, validator, name) )
        return false;

    parent->AddChild(this);

    return true;
}

void wxControlBase::Command(wxCommandEvent& event)
{
    (void)GetEventHandler()->ProcessEvent(event);
}

void wxControlBase::InitCommandEvent(wxCommandEvent& event) const
{
    event.SetEventObject(const_cast<wxControlBase *>(this));

    
    switch ( m_clientDataType )
    {
        case wxClientData_Void:
            event.SetClientData(GetClientData());
            break;

        case wxClientData_Object:
            event.SetClientObject(GetClientObject());
            break;

        case wxClientData_None:
                        ;
    }
}

bool wxControlBase::SetFont(const wxFont& font)
{
    InvalidateBestSize();
    return wxWindow::SetFont(font);
}

void wxControlBase::DoUpdateWindowUI(wxUpdateUIEvent& event)
{
        wxWindowBase::DoUpdateWindowUI(event);

        if ( event.GetSetText() )
    {
        if ( event.GetText() != GetLabel() )
            SetLabel(event.GetText());
    }

            #if wxUSE_RADIOBTN
    if ( event.GetSetChecked() )
    {
        wxRadioButton *radiobtn = wxDynamicCastThis(wxRadioButton);
        if ( radiobtn )
            radiobtn->SetValue(event.GetChecked());
    }
#endif }

wxSize wxControlBase::DoGetSizeFromTextSize(int WXUNUSED(xlen),
                                            int WXUNUSED(ylen)) const
{
    return wxSize(-1, -1);
}


wxString wxControlBase::GetLabelText(const wxString& label)
{
        return wxStripMenuCodes(label, wxStrip_Mnemonics);
}


wxString wxControlBase::RemoveMnemonics(const wxString& str)
{
        return wxStripMenuCodes(str, wxStrip_Mnemonics);
}


wxString wxControlBase::EscapeMnemonics(const wxString& text)
{
    wxString label(text);
    label.Replace("&", "&&");
    return label;
}


int wxControlBase::FindAccelIndex(const wxString& label, wxString *labelOnly)
{
                static const wxChar MNEMONIC_PREFIX = wxT('&');

    if ( labelOnly )
    {
        labelOnly->Empty();
        labelOnly->Alloc(label.length());
    }

    int indexAccel = -1;
    for ( wxString::const_iterator pc = label.begin(); pc != label.end(); ++pc )
    {
        if ( *pc == MNEMONIC_PREFIX )
        {
            ++pc;             if ( pc == label.end() )
                break;
            else if ( *pc != MNEMONIC_PREFIX )
            {
                if ( indexAccel == -1 )
                {
                                        indexAccel = pc - label.begin() - 1;
                }
                else
                {
                    wxFAIL_MSG(wxT("duplicate accel char in control label"));
                }
            }
        }

        if ( labelOnly )
        {
            *labelOnly += *pc;
        }
    }

    return indexAccel;
}

wxBorder wxControlBase::GetDefaultBorder() const
{
    return wxBORDER_THEME;
}

 wxVisualAttributes
wxControlBase::GetCompositeControlsDefaultAttributes(wxWindowVariant WXUNUSED(variant))
{
    wxVisualAttributes attrs;
    attrs.font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    attrs.colFg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    attrs.colBg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);

    return attrs;
}


#if wxUSE_MARKUP


wxString wxControlBase::RemoveMarkup(const wxString& markup)
{
    return wxMarkupParser::Strip(markup);
}

bool wxControlBase::DoSetLabelMarkup(const wxString& markup)
{
    const wxString label = RemoveMarkup(markup);
    if ( label.empty() && !markup.empty() )
        return false;

    SetLabel(label);

    return true;
}

#endif 

#define wxELLIPSE_REPLACEMENT       wxS("...")

namespace
{

struct EllipsizeCalculator
{
    EllipsizeCalculator(const wxString& s, const wxDC& dc,
                        int maxFinalWidthPx, int replacementWidthPx)
        : 
          m_initialCharToRemove(0),
          m_nCharsToRemove(0),
          m_outputNeedsUpdate(true),
          m_str(s),
          m_dc(dc),
          m_maxFinalWidthPx(maxFinalWidthPx),
          m_replacementWidthPx(replacementWidthPx)
    {
        m_isOk = dc.GetPartialTextExtents(s, m_charOffsetsPx);
        wxASSERT( m_charOffsetsPx.GetCount() == s.length() );
    }

    bool IsOk() const { return m_isOk; }

    bool EllipsizationNotNeeded() const
    {
                        return m_charOffsetsPx.Last() <= m_maxFinalWidthPx;
    }

    void Init(size_t initialCharToRemove, size_t nCharsToRemove)
    {
        m_initialCharToRemove = initialCharToRemove;
        m_nCharsToRemove = nCharsToRemove;
    }

    void RemoveFromEnd()
    {
        m_nCharsToRemove++;
    }

    void RemoveFromStart()
    {
        m_initialCharToRemove--;
        m_nCharsToRemove++;
    }

    size_t GetFirstRemoved() const { return m_initialCharToRemove; }
    size_t GetLastRemoved() const { return m_initialCharToRemove + m_nCharsToRemove - 1; }

    const wxString& GetEllipsizedText()
    {
        if ( m_outputNeedsUpdate )
        {
            wxASSERT(m_initialCharToRemove <= m_str.length() - 1);              wxASSERT(m_nCharsToRemove >= 1 && m_nCharsToRemove <= m_str.length() - m_initialCharToRemove);  
                                                                        m_output = m_str;
            m_output.replace(m_initialCharToRemove, m_nCharsToRemove, wxELLIPSE_REPLACEMENT);
        }

        return m_output;
    }

    bool IsShortEnough()
    {
        if ( m_nCharsToRemove == m_str.length() )
            return true; 
                                                                                
        int estimatedWidth = m_replacementWidthPx; 
                if ( m_initialCharToRemove > 0 )
            estimatedWidth += m_charOffsetsPx[m_initialCharToRemove - 1];

        
        if ( GetLastRemoved() < m_str.length() )
           estimatedWidth += m_charOffsetsPx.Last() - m_charOffsetsPx[GetLastRemoved()];

        if ( estimatedWidth > m_maxFinalWidthPx )
            return false;

        return m_dc.GetTextExtent(GetEllipsizedText()).GetWidth() <= m_maxFinalWidthPx;
    }

    
                    
        size_t m_initialCharToRemove;
        size_t m_nCharsToRemove;

    wxString m_output;
    bool m_outputNeedsUpdate;

        wxString m_str;
    const wxDC& m_dc;
    int m_maxFinalWidthPx;
    int m_replacementWidthPx;
    wxArrayInt m_charOffsetsPx;

    bool m_isOk;
};

} 

wxString wxControlBase::DoEllipsizeSingleLine(const wxString& curLine, const wxDC& dc,
                                              wxEllipsizeMode mode, int maxFinalWidthPx,
                                              int replacementWidthPx)
{
    wxASSERT_MSG(replacementWidthPx > 0, "Invalid parameters");
    wxASSERT_LEVEL_2_MSG(!curLine.Contains('\n'),
                         "Use Ellipsize() instead!");

    wxASSERT_MSG( mode != wxELLIPSIZE_NONE, "shouldn't be called at all then" );

        
    if (maxFinalWidthPx <= 0)
        return wxEmptyString;

    size_t len = curLine.length();
    if (len <= 1 )
        return curLine;

    EllipsizeCalculator calc(curLine, dc, maxFinalWidthPx, replacementWidthPx);

    if ( !calc.IsOk() )
        return curLine;

    if ( calc.EllipsizationNotNeeded() )
        return curLine;

        switch (mode)
    {
        case wxELLIPSIZE_START:
            {
                calc.Init(0, 1);
                while ( !calc.IsShortEnough() )
                    calc.RemoveFromEnd();

                                if ( calc.m_nCharsToRemove == len )
                    return wxString(wxELLIPSE_REPLACEMENT) + curLine[len-1];

                break;
            }

        case wxELLIPSIZE_MIDDLE:
            {
                
                                                                                                
                calc.Init(len/2, 0);

                bool removeFromStart = true;

                while ( !calc.IsShortEnough() )
                {
                    const bool canRemoveFromStart = calc.GetFirstRemoved() > 0;
                    const bool canRemoveFromEnd = calc.GetLastRemoved() < len - 1;

                    if ( !canRemoveFromStart && !canRemoveFromEnd )
                    {
                                                break;
                    }

                                                            removeFromStart = !removeFromStart;
                    if ( removeFromStart && !canRemoveFromStart )
                        removeFromStart = false;
                    else if ( !removeFromStart && !canRemoveFromEnd )
                        removeFromStart = true;

                    if ( removeFromStart )
                        calc.RemoveFromStart();
                    else
                        calc.RemoveFromEnd();
                }

                                                                if ( calc.m_nCharsToRemove == len ||
                     calc.m_nCharsToRemove == len - 1 )
                {
                    return curLine[0] + wxString(wxELLIPSE_REPLACEMENT);
                }
            }
            break;

        case wxELLIPSIZE_END:
            {
                calc.Init(len - 1, 1);
                while ( !calc.IsShortEnough() )
                    calc.RemoveFromStart();

                                if ( calc.m_nCharsToRemove == len )
                    return curLine[0] + wxString(wxELLIPSE_REPLACEMENT);

                break;
            }

        case wxELLIPSIZE_NONE:
        default:
            wxFAIL_MSG("invalid ellipsize mode");
            return curLine;
    }

    return calc.GetEllipsizedText();
}


wxString wxControlBase::Ellipsize(const wxString& label, const wxDC& dc,
                                  wxEllipsizeMode mode, int maxFinalWidth,
                                  int flags)
{
    if (mode == wxELLIPSIZE_NONE)
        return label;

    wxString ret;

                int replacementWidth = dc.GetTextExtent(wxELLIPSE_REPLACEMENT).GetWidth();

        wxString curLine;
    for ( wxString::const_iterator pc = label.begin(); ; ++pc )
    {
        if ( pc == label.end() || *pc == wxS('\n') )
        {
            curLine = DoEllipsizeSingleLine(curLine, dc, mode, maxFinalWidth,
                                            replacementWidth);

                        ret << curLine;
            if ( pc == label.end() )
                break;

            ret << *pc;
            curLine.clear();
        }
                else if ( *pc == wxS('&') && (flags & wxELLIPSIZE_FLAGS_PROCESS_MNEMONICS) )
        {
                        wxString::const_iterator next = pc + 1;
            if ( next != label.end() && *next == wxS('&') )
                curLine += wxS('&');                              }
                else if ( *pc == wxS('\t') && (flags & wxELLIPSIZE_FLAGS_EXPAND_TABS) )
        {
                        curLine += wxS("      ");
        }
        else
        {
            curLine += *pc;
        }
    }

    return ret;
}


#if wxUSE_STATBMP

wxStaticBitmapBase::~wxStaticBitmapBase()
{
    }

wxSize wxStaticBitmapBase::DoGetBestSize() const
{
    wxSize best;
    wxBitmap bmp = GetBitmap();
    if ( bmp.IsOk() )
        best = bmp.GetScaledSize();
    else
                best = wxSize(16, 16);
    CacheBestSize(best);
    return best;
}

#endif 
#endif 