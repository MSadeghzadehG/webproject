


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_VALIDATORS && wxUSE_TEXTCTRL

#ifndef WX_PRECOMP
    #include "wx/textctrl.h"
    #include "wx/combobox.h"
#endif

#include "wx/valnum.h"
#include "wx/numformatter.h"


wxBEGIN_EVENT_TABLE(wxNumValidatorBase, wxValidator)
    EVT_CHAR(wxNumValidatorBase::OnChar)
    EVT_KILL_FOCUS(wxNumValidatorBase::OnKillFocus)
wxEND_EVENT_TABLE()

int wxNumValidatorBase::GetFormatFlags() const
{
    int flags = wxNumberFormatter::Style_None;
    if ( m_style & wxNUM_VAL_THOUSANDS_SEPARATOR )
        flags |= wxNumberFormatter::Style_WithThousandsSep;
    if ( m_style & wxNUM_VAL_NO_TRAILING_ZEROES )
        flags |= wxNumberFormatter::Style_NoTrailingZeroes;

    return flags;
}

wxTextEntry *wxNumValidatorBase::GetTextEntry() const
{
#if wxUSE_TEXTCTRL
    if ( wxTextCtrl *text = wxDynamicCast(m_validatorWindow, wxTextCtrl) )
        return text;
#endif 
#if wxUSE_COMBOBOX
    if ( wxComboBox *combo = wxDynamicCast(m_validatorWindow, wxComboBox) )
        return combo;
#endif 
    wxFAIL_MSG("Can only be used with wxTextCtrl or wxComboBox");

    return NULL;
}

void
wxNumValidatorBase::GetCurrentValueAndInsertionPoint(wxString& val,
                                                             int& pos) const
{
    wxTextEntry * const control = GetTextEntry();
    if ( !control )
        return;

    val = control->GetValue();
    pos = control->GetInsertionPoint();

    long selFrom, selTo;
    control->GetSelection(&selFrom, &selTo);

    const long selLen = selTo - selFrom;
    if ( selLen )
    {
                val.erase(selFrom, selLen);

                        if ( pos > selFrom )
        {
            if ( pos >= selTo )
                pos -= selLen;
            else
                pos = selFrom;
        }
    }
}

bool wxNumValidatorBase::IsMinusOk(const wxString& val, int pos) const
{
        if ( pos != 0 )
        return false;

        if ( !val.empty() && val[0] == '-' )
        return false;

    return true;
}

void wxNumValidatorBase::OnChar(wxKeyEvent& event)
{
            event.Skip();

    if ( !m_validatorWindow )
        return;

#if wxUSE_UNICODE
    const int ch = event.GetUnicodeKey();
    if ( ch == WXK_NONE )
    {
                        return;
    }
#else     const int ch = event.GetKeyCode();
    if ( ch > WXK_DELETE )
    {
                return;
    }
#endif 
    if ( ch < WXK_SPACE || ch == WXK_DELETE )
    {
                return;
    }

        wxString val;
    int pos;
    GetCurrentValueAndInsertionPoint(val, pos);

    if ( !IsCharOk(val, pos, ch) )
    {
        if ( !wxValidator::IsSilent() )
            wxBell();

                event.Skip(false);
    }
}

void wxNumValidatorBase::OnKillFocus(wxFocusEvent& event)
{
    wxTextEntry * const control = GetTextEntry();
    if ( !control )
        return;

                            wxTextCtrl * const text = wxDynamicCast(m_validatorWindow, wxTextCtrl);
    const bool wasModified = text ? text->IsModified() : false;

    control->ChangeValue(NormalizeString(control->GetValue()));

    if ( wasModified )
        text->MarkDirty();

    event.Skip();
}


wxString wxIntegerValidatorBase::ToString(LongestValueType value) const
{
    return wxNumberFormatter::ToString(value, GetFormatFlags());
}

bool
wxIntegerValidatorBase::FromString(const wxString& s, LongestValueType *value)
{
    return wxNumberFormatter::FromString(s, value);
}

bool
wxIntegerValidatorBase::IsCharOk(const wxString& val, int pos, wxChar ch) const
{
        if ( ch == '-' )
    {
                                                                                return m_min < 0 && IsMinusOk(val, pos);
    }

            if ( ch < '0' || ch > '9' )
        return false;

        LongestValueType value;
    if ( !FromString(GetValueAfterInsertingChar(val, pos, ch), &value) )
        return false;

    return IsInRange(value);
}


wxString wxFloatingPointValidatorBase::ToString(LongestValueType value) const
{
    return wxNumberFormatter::ToString(value, m_precision, GetFormatFlags());
}

bool
wxFloatingPointValidatorBase::FromString(const wxString& s,
                                         LongestValueType *value)
{
    return wxNumberFormatter::FromString(s, value);
}

bool
wxFloatingPointValidatorBase::IsCharOk(const wxString& val,
                                       int pos,
                                       wxChar ch) const
{
        if ( ch == '-' )
        return m_min < 0 && IsMinusOk(val, pos);

    const wxChar separator = wxNumberFormatter::GetDecimalSeparator();
    if ( ch == separator )
    {
        if ( val.find(separator) != wxString::npos )
        {
                        return false;
        }

                if ( pos == 0 && !val.empty() && val[0] == '-' )
            return false;

                                                return true;
    }

        if ( ch < '0' || ch > '9' )
        return false;

        const wxString newval(GetValueAfterInsertingChar(val, pos, ch));

    LongestValueType value;
    if ( !FromString(newval, &value) )
        return false;

        const size_t posSep = newval.find(separator);
    if ( posSep != wxString::npos && newval.length() - posSep - 1 > m_precision )
        return false;

        return IsInRange(value);
}

#endif 