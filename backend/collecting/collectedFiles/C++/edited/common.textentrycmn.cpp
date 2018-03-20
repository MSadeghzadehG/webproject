


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_TEXTCTRL || wxUSE_COMBOBOX

#ifndef WX_PRECOMP
    #include "wx/window.h"
    #include "wx/dataobj.h"
#endif 
#include "wx/textentry.h"
#include "wx/textcompleter.h"
#include "wx/clipbrd.h"


class WXDLLIMPEXP_CORE wxTextEntryHintData
{
public:
    wxTextEntryHintData(wxTextEntryBase *entry, wxWindow *win)
        : m_entry(entry),
          m_win(win),
          m_text(m_entry->GetValue())
    {
        win->Bind(wxEVT_SET_FOCUS, &wxTextEntryHintData::OnSetFocus, this);
        win->Bind(wxEVT_KILL_FOCUS, &wxTextEntryHintData::OnKillFocus, this);
        win->Bind(wxEVT_TEXT, &wxTextEntryHintData::OnTextChanged, this);
    }

    
            const wxString& GetText() const { return m_text; }

                    void SetHintString(const wxString& hint)
    {
        m_hint = hint;

        if ( !m_win->HasFocus() )
            ShowHintIfAppropriate();
            }

    const wxString& GetHintString() const { return m_hint; }

                        void HandleTextUpdate(const wxString& text)
    {
        m_text = text;

                        RestoreTextColourIfNecessary();
    }

private:
            void ShowHintIfAppropriate()
    {
                if ( !m_text.empty() )
            return;

                        m_colFg = m_win->GetForegroundColour();
        m_win->SetForegroundColour(*wxLIGHT_GREY);

        m_entry->DoSetValue(m_hint, wxTextEntryBase::SetValue_NoEvent);
    }

            void RestoreTextColourIfNecessary()
    {
        if ( m_colFg.IsOk() )
        {
            m_win->SetForegroundColour(m_colFg);
            m_colFg = wxColour();
        }
    }

    void OnSetFocus(wxFocusEvent& event)
    {
                        if ( m_text.empty() )
        {
            RestoreTextColourIfNecessary();

            m_entry->DoSetValue(wxString(), wxTextEntryBase::SetValue_NoEvent);
        }

        event.Skip();
    }

    void OnKillFocus(wxFocusEvent& event)
    {
                ShowHintIfAppropriate();

        event.Skip();
    }

    void OnTextChanged(wxCommandEvent& event)
    {
                                                        HandleTextUpdate(m_entry->DoGetValue());

        event.Skip();
    }


        wxTextEntryBase * const m_entry;
    wxWindow * const m_win;

        wxColour m_colFg;

        wxString m_hint;

        wxString m_text;


    wxDECLARE_NO_COPY_CLASS(wxTextEntryHintData);
};


wxTextEntryBase::~wxTextEntryBase()
{
    delete m_hintData;
}


wxString wxTextEntryBase::GetValue() const
{
    return m_hintData ? m_hintData->GetText() : DoGetValue();
}

wxString wxTextEntryBase::GetRange(long from, long to) const
{
    wxString sel;
    wxString value = GetValue();

    if ( from < to && (long)value.length() >= to )
    {
        sel = value.substr(from, to - from);
    }

    return sel;
}


void wxTextEntryBase::ChangeValue(const wxString& value)
{
    DoSetValue(value, SetValue_NoEvent);

            if ( m_hintData )
        m_hintData->HandleTextUpdate(value);
}

void wxTextEntryBase::AppendText(const wxString& text)
{
    SetInsertionPointEnd();
    WriteText(text);
}

void wxTextEntryBase::DoSetValue(const wxString& value, int flags)
{
    if ( value != DoGetValue() )
    {
        EventsSuppressor noeventsIf(this, !(flags & SetValue_SendEvent));

        SelectAll();
        WriteText(value);

        SetInsertionPoint(0);
    }
    else     {
                        if ( flags & SetValue_SendEvent )
            SendTextUpdatedEvent(GetEditableWindow());
    }
}

void wxTextEntryBase::Replace(long from, long to, const wxString& value)
{
    {
        EventsSuppressor noevents(this);
        Remove(from, to);
    }

    SetInsertionPoint(from);
    WriteText(value);
}


bool wxTextEntryBase::HasSelection() const
{
    long from, to;
    GetSelection(&from, &to);

    return from < to;
}

void wxTextEntryBase::RemoveSelection()
{
    long from, to;
    GetSelection(& from, & to);
    if (from != -1 && to != -1)
        Remove(from, to);
}

wxString wxTextEntryBase::GetStringSelection() const
{
    long from, to;
    GetSelection(&from, &to);

    return GetRange(from, to);
}


bool wxTextEntryBase::CanCopy() const
{
    return HasSelection();
}

bool wxTextEntryBase::CanCut() const
{
    return CanCopy() && IsEditable();
}

bool wxTextEntryBase::CanPaste() const
{
    if ( IsEditable() )
    {
#if wxUSE_CLIPBOARD
                if ( wxTheClipboard->IsSupported(wxDF_TEXT)
#if wxUSE_UNICODE
                || wxTheClipboard->IsSupported(wxDF_UNICODETEXT)
#endif            )
        {
            return true;
        }
#endif     }

    return false;
}


#ifndef wxHAS_NATIVE_TEXT_FORCEUPPER

namespace
{

struct ForceUpperFunctor
{
    explicit ForceUpperFunctor(wxTextEntryBase* entry)
        : m_entry(entry)
    {
    }

    void operator()(wxCommandEvent& event)
    {
        event.Skip();
        m_entry->ConvertToUpperCase();
    }

    wxTextEntryBase* const m_entry;
};

} 
#endif 
void wxTextEntryBase::ConvertToUpperCase()
{
    const wxString& valueOld = GetValue();
    const wxString& valueNew = valueOld.Upper();

    if ( valueNew != valueOld )
    {
        long from, to;
        GetSelection(&from, &to);
        ChangeValue(valueNew);
        SetSelection(from, to);
    }
}

void wxTextEntryBase::ForceUpper()
{
        #ifndef wxHAS_NATIVE_TEXT_FORCEUPPER
    wxWindow* const win = GetEditableWindow();
    wxCHECK_RET( win, wxS("can't be called before creating the window") );

        ConvertToUpperCase();

        win->Bind(wxEVT_TEXT, ForceUpperFunctor(this));
#endif }


bool wxTextEntryBase::SetHint(const wxString& hint)
{
    if ( !hint.empty() )
    {
        if ( !m_hintData )
            m_hintData = new wxTextEntryHintData(this, GetEditableWindow());

        m_hintData->SetHintString(hint);
    }
    else if ( m_hintData )
    {
                delete m_hintData;
        m_hintData = NULL;
    }
    
    return true;
}

wxString wxTextEntryBase::GetHint() const
{
    return m_hintData ? m_hintData->GetHintString() : wxString();
}


bool wxTextEntryBase::DoSetMargins(const wxPoint& WXUNUSED(pt))
{
    return false;
}

wxPoint wxTextEntryBase::DoGetMargins() const
{
    return wxPoint(-1, -1);
}



bool wxTextEntryBase::SendTextUpdatedEvent(wxWindow *win)
{
    wxCHECK_MSG( win, false, "can't send an event without a window" );

    wxCommandEvent event(wxEVT_TEXT, win->GetId());

                    
    event.SetEventObject(win);
    return win->HandleWindowEvent(event);
}


wxTextCompleter::~wxTextCompleter()
{
}

bool wxTextCompleterSimple::Start(const wxString& prefix)
{
    m_index = 0;
    m_completions.clear();
    GetCompletions(prefix, m_completions);

    return !m_completions.empty();
}

wxString wxTextCompleterSimple::GetNext()
{
    if ( m_index == m_completions.size() )
        return wxString();

    return m_completions[m_index++];
}

bool wxTextEntryBase::DoAutoCompleteCustom(wxTextCompleter *completer)
{
                delete completer;

    return false;
}

#endif 