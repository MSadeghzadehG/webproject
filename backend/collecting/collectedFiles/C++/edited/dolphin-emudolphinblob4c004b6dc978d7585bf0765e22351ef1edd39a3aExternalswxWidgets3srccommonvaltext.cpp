
#include "wx/wxprec.h"

#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#if wxUSE_VALIDATORS && (wxUSE_TEXTCTRL || wxUSE_COMBOBOX)

#include "wx/valtext.h"

#ifndef WX_PRECOMP
  #include <stdio.h>
  #include "wx/textctrl.h"
  #include "wx/combobox.h"
  #include "wx/utils.h"
  #include "wx/msgdlg.h"
  #include "wx/intl.h"
#endif

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "wx/combo.h"


static bool wxIsNumeric(const wxString& val)
{
    for ( wxString::const_iterator i = val.begin(); i != val.end(); ++i )
    {
                        if ((!wxIsdigit(*i)) &&
            (*i != wxS('.')) && (*i != wxS(',')) && (*i != wxS('e')) &&
            (*i != wxS('E')) && (*i != wxS('+')) && (*i != wxS('-')))
            return false;
    }
    return true;
}


wxIMPLEMENT_DYNAMIC_CLASS(wxTextValidator, wxValidator);
wxBEGIN_EVENT_TABLE(wxTextValidator, wxValidator)
    EVT_CHAR(wxTextValidator::OnChar)
wxEND_EVENT_TABLE()

wxTextValidator::wxTextValidator(long style, wxString *val)
{
    m_stringValue = val;
    SetStyle(style);
}

wxTextValidator::wxTextValidator(const wxTextValidator& val)
    : wxValidator()
{
    Copy(val);
}

void wxTextValidator::SetStyle(long style)
{
    m_validatorStyle = style;

#if wxDEBUG_LEVEL
    int check;
    check = (int)HasFlag(wxFILTER_ALPHA) + (int)HasFlag(wxFILTER_ALPHANUMERIC) +
            (int)HasFlag(wxFILTER_DIGITS) + (int)HasFlag(wxFILTER_NUMERIC);
    wxASSERT_MSG(check <= 1,
        "It makes sense to use only one of the wxFILTER_ALPHA/wxFILTER_ALPHANUMERIC/"
        "wxFILTER_SIMPLE_NUMBER/wxFILTER_NUMERIC styles");

    wxASSERT_MSG(((int)HasFlag(wxFILTER_INCLUDE_LIST) + (int)HasFlag(wxFILTER_INCLUDE_CHAR_LIST) <= 1) &&
                 ((int)HasFlag(wxFILTER_EXCLUDE_LIST) + (int)HasFlag(wxFILTER_EXCLUDE_CHAR_LIST) <= 1),
        "Using both wxFILTER_[IN|EX]CLUDE_LIST _and_ wxFILTER_[IN|EX]CLUDE_CHAR_LIST "
        "doesn't work since wxTextValidator internally uses the same array for both");

    check = (int)HasFlag(wxFILTER_INCLUDE_LIST) + (int)HasFlag(wxFILTER_INCLUDE_CHAR_LIST) +
            (int)HasFlag(wxFILTER_EXCLUDE_LIST) + (int)HasFlag(wxFILTER_EXCLUDE_CHAR_LIST);
    wxASSERT_MSG(check <= 1,
        "Using both an include/exclude list may lead to unexpected results");
#endif }

bool wxTextValidator::Copy(const wxTextValidator& val)
{
    wxValidator::Copy(val);

    m_validatorStyle = val.m_validatorStyle;
    m_stringValue = val.m_stringValue;

    m_includes    = val.m_includes;
    m_excludes    = val.m_excludes;

    return true;
}

wxTextEntry *wxTextValidator::GetTextEntry()
{
#if wxUSE_TEXTCTRL
    if (wxDynamicCast(m_validatorWindow, wxTextCtrl))
    {
        return (wxTextCtrl*)m_validatorWindow;
    }
#endif

#if wxUSE_COMBOBOX
    if (wxDynamicCast(m_validatorWindow, wxComboBox))
    {
        return (wxComboBox*)m_validatorWindow;
    }
#endif

#if wxUSE_COMBOCTRL
    if (wxDynamicCast(m_validatorWindow, wxComboCtrl))
    {
        return (wxComboCtrl*)m_validatorWindow;
    }
#endif

    wxFAIL_MSG(
        "wxTextValidator can only be used with wxTextCtrl, wxComboBox, "
        "or wxComboCtrl"
    );

    return NULL;
}

bool wxTextValidator::Validate(wxWindow *parent)
{
        if ( !m_validatorWindow->IsEnabled() )
        return true;

    wxTextEntry * const text = GetTextEntry();
    if ( !text )
        return false;

    wxString val(text->GetValue());

    wxString errormsg;

            if ( HasFlag(wxFILTER_EMPTY) && val.empty() )
        errormsg = _("Required information entry is empty.");
    else if ( HasFlag(wxFILTER_INCLUDE_LIST) && m_includes.Index(val) == wxNOT_FOUND )
        errormsg = wxString::Format(_("'%s' is not one of the valid strings"), val);
    else if ( HasFlag(wxFILTER_EXCLUDE_LIST) && m_excludes.Index(val) != wxNOT_FOUND )
        errormsg = wxString::Format(_("'%s' is one of the invalid strings"), val);
    else if ( !(errormsg = IsValid(val)).empty() )
    {
                wxString buf;
        buf.Printf(errormsg, val.c_str());
        errormsg = buf;
    }

    if ( !errormsg.empty() )
    {
        m_validatorWindow->SetFocus();
        wxMessageBox(errormsg, _("Validation conflict"),
                     wxOK | wxICON_EXCLAMATION, parent);

        return false;
    }

    return true;
}

bool wxTextValidator::TransferToWindow()
{
    if ( m_stringValue )
    {
        wxTextEntry * const text = GetTextEntry();
        if ( !text )
            return false;

        text->SetValue(*m_stringValue);
    }

    return true;
}

bool wxTextValidator::TransferFromWindow()
{
    if ( m_stringValue )
    {
        wxTextEntry * const text = GetTextEntry();
        if ( !text )
            return false;

        *m_stringValue = text->GetValue();
    }

    return true;
}

namespace
{

bool CheckString(bool (*func)(const wxUniChar&), const wxString& str)
{
    for ( wxString::const_iterator i = str.begin(); i != str.end(); ++i )
    {
        if ( !func(*i) )
            return false;
    }

    return true;
}

} 
wxString wxTextValidator::IsValid(const wxString& val) const
{
    
    if ( HasFlag(wxFILTER_ASCII) && !val.IsAscii() )
        return _("'%s' should only contain ASCII characters.");
    if ( HasFlag(wxFILTER_ALPHA) && !CheckString(wxIsalpha, val) )
        return _("'%s' should only contain alphabetic characters.");
    if ( HasFlag(wxFILTER_ALPHANUMERIC) && !CheckString(wxIsalnum, val) )
        return _("'%s' should only contain alphabetic or numeric characters.");
    if ( HasFlag(wxFILTER_DIGITS) && !CheckString(wxIsdigit, val) )
        return _("'%s' should only contain digits.");
    if ( HasFlag(wxFILTER_NUMERIC) && !wxIsNumeric(val) )
        return _("'%s' should be numeric.");
    if ( HasFlag(wxFILTER_INCLUDE_CHAR_LIST) && !ContainsOnlyIncludedCharacters(val) )
        return _("'%s' doesn't consist only of valid characters");
    if ( HasFlag(wxFILTER_EXCLUDE_CHAR_LIST) && ContainsExcludedCharacters(val) )
        return _("'%s' contains illegal characters");

    return wxEmptyString;
}

bool wxTextValidator::ContainsOnlyIncludedCharacters(const wxString& val) const
{
    for ( wxString::const_iterator i = val.begin(); i != val.end(); ++i )
        if (m_includes.Index((wxString) *i) == wxNOT_FOUND)
                        return false;

        return true;
}

bool wxTextValidator::ContainsExcludedCharacters(const wxString& val) const
{
    for ( wxString::const_iterator i = val.begin(); i != val.end(); ++i )
        if (m_excludes.Index((wxString) *i) != wxNOT_FOUND)
                        return true;

        return false;
}

void wxTextValidator::SetCharIncludes(const wxString& chars)
{
    wxArrayString arr;

    for ( wxString::const_iterator i = chars.begin(); i != chars.end(); ++i )
        arr.Add(*i);

    SetIncludes(arr);
}

void wxTextValidator::SetCharExcludes(const wxString& chars)
{
    wxArrayString arr;

    for ( wxString::const_iterator i = chars.begin(); i != chars.end(); ++i )
        arr.Add(*i);

    SetExcludes(arr);
}

void wxTextValidator::OnChar(wxKeyEvent& event)
{
        event.Skip();

    if (!m_validatorWindow)
        return;

#if wxUSE_UNICODE
        int keyCode = event.GetUnicodeKey();
#else     int keyCode = event.GetKeyCode();
    if (keyCode > WXK_START)
        return;
#endif 
        if (keyCode < WXK_SPACE || keyCode == WXK_DELETE)
        return;

    wxString str((wxUniChar)keyCode, 1);
    if (IsValid(str).empty())
        return;

    if ( !wxValidator::IsSilent() )
        wxBell();

        event.Skip(false);
}


#endif
  