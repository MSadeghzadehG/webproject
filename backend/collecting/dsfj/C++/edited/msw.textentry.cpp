


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/arrstr.h"
    #include "wx/string.h"
#endif 
#if wxUSE_TEXTCTRL || wxUSE_COMBOBOX

#include "wx/textentry.h"
#include "wx/textcompleter.h"
#include "wx/dynlib.h"

#include "wx/msw/private.h"

#if wxUSE_UXTHEME
    #include "wx/msw/uxtheme.h"
#endif

#include "wx/msw/wrapwin.h"
#include <shlwapi.h>

#define GetEditHwnd() ((HWND)(GetEditHWND()))


#if wxUSE_OLE
    #define HAS_AUTOCOMPLETE
#endif

#ifdef HAS_AUTOCOMPLETE

#include "wx/msw/ole/oleutils.h"
#include <shldisp.h>

#if defined(__MINGW32__) || defined(__CYGWIN__)
        #include <shlguid.h>

    #ifndef ACO_AUTOAPPEND
        #define ACO_AUTOAPPEND 0x02
    #endif
#endif

#ifndef ACO_UPDOWNKEYDROPSLIST
    #define ACO_UPDOWNKEYDROPSLIST 0x20
#endif

#ifndef SHACF_FILESYS_ONLY
    #define SHACF_FILESYS_ONLY 0x00000010
#endif

#ifndef SHACF_FILESYS_DIRS
    #define SHACF_FILESYS_DIRS 0x00000020
#endif

#include <initguid.h>

class IAutoCompleteDropDown : public IUnknown
{
public:
    virtual HRESULT wxSTDCALL GetDropDownStatus(DWORD *, LPWSTR *) = 0;
    virtual HRESULT wxSTDCALL ResetEnumerator() = 0;
};

namespace
{

DEFINE_GUID(wxIID_IAutoCompleteDropDown,
    0x3cd141f4, 0x3c6a, 0x11d2, 0xbc, 0xaa, 0x00, 0xc0, 0x4f, 0xd9, 0x29, 0xdb);

DEFINE_GUID(wxCLSID_AutoComplete,
    0x00bb2763, 0x6a77, 0x11d0, 0xa5, 0x35, 0x00, 0xc0, 0x4f, 0xd7, 0xd0, 0x62);

#ifndef ACDD_VISIBLE
    #define ACDD_VISIBLE 0x0001
#endif

class CSLock
{
public:
    CSLock(CRITICAL_SECTION& cs) : m_cs(&cs)
    {
        ::EnterCriticalSection(m_cs);
    }

    ~CSLock()
    {
        ::LeaveCriticalSection(m_cs);
    }

private:
    CRITICAL_SECTION * const m_cs;

    wxDECLARE_NO_COPY_CLASS(CSLock);
};

} 
class wxIEnumString : public IEnumString
{
public:
    wxIEnumString()
    {
        Init();
    }

    void ChangeCompleter(wxTextCompleter *completer)
    {
                {
            CSLock lock(m_csRestart);

            m_restart = TRUE;
        }

                                CSLock lock(m_csCompleter);

        m_completer = completer;
    }

    void UpdatePrefix(const wxString& prefix)
    {
        CSLock lock(m_csRestart);

                                        m_prefix = prefix;
        m_restart = TRUE;
    }

    virtual HRESULT STDMETHODCALLTYPE Next(ULONG celt,
                                           LPOLESTR *rgelt,
                                           ULONG *pceltFetched)
    {
        if ( !rgelt || (!pceltFetched && celt > 1) )
            return E_POINTER;

        ULONG pceltFetchedDummy;
        if ( !pceltFetched )
            pceltFetched = &pceltFetchedDummy;

        *pceltFetched = 0;

        CSLock lock(m_csCompleter);

        if ( !RestartIfNeeded() )
            return S_FALSE;

        while ( celt-- )
        {
                        if ( m_restart )
                return S_FALSE;

            const wxString s = m_completer->GetNext();
            if ( s.empty() )
                return S_FALSE;

            const wxWX2WCbuf wcbuf = s.wc_str();
            const size_t size = (wcslen(wcbuf) + 1)*sizeof(wchar_t);
            void *olestr = CoTaskMemAlloc(size);
            if ( !olestr )
                return E_OUTOFMEMORY;

            memcpy(olestr, wcbuf, size);

            *rgelt++ = static_cast<LPOLESTR>(olestr);

            ++(*pceltFetched);
        }

        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE Skip(ULONG celt)
    {
        if ( !celt )
            return E_INVALIDARG;

        CSLock lock(m_csCompleter);

        if ( !RestartIfNeeded() )
            return S_FALSE;

        while ( celt-- )
        {
            if ( m_restart )
                return S_FALSE;

            if ( m_completer->GetNext().empty() )
                return S_FALSE;
        }

        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE Reset()
    {
        CSLock lock(m_csRestart);

        m_restart = TRUE;

        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE Clone(IEnumString **ppEnum)
    {
        if ( !ppEnum )
            return E_POINTER;

        CSLock lock(m_csCompleter);

        wxIEnumString * const e = new wxIEnumString;
        e->AddRef();

        e->ChangeCompleter(m_completer);

        *ppEnum = e;

        return S_OK;
    }

    DECLARE_IUNKNOWN_METHODS;

private:
                    virtual ~wxIEnumString()
    {
        ::DeleteCriticalSection(&m_csRestart);
        ::DeleteCriticalSection(&m_csCompleter);
    }

        void Init()
    {
        ::InitializeCriticalSection(&m_csCompleter);
        ::InitializeCriticalSection(&m_csRestart);

        m_completer = NULL;
        m_restart = FALSE;
    }

                        bool RestartIfNeeded()
    {
        bool rc = true;
        for ( ;; )
        {
            wxString prefix;
            LONG restart;
            {
                CSLock lock(m_csRestart);

                prefix = m_prefix;
                restart = m_restart;

                m_restart = FALSE;
            }               
            if ( !restart )
                break;

            rc = m_completer->Start(prefix);
        }

        return rc;
    }


                CRITICAL_SECTION m_csCompleter;

            wxTextCompleter *m_completer;


                    CRITICAL_SECTION m_csRestart;

            wxString m_prefix;

                LONG m_restart;


    wxDECLARE_NO_COPY_CLASS(wxIEnumString);
};

BEGIN_IID_TABLE(wxIEnumString)
    ADD_IID(Unknown)
    ADD_IID(EnumString)
END_IID_TABLE;

IMPLEMENT_IUNKNOWN_METHODS(wxIEnumString)


class wxTextAutoCompleteData
{
public:
        wxTextAutoCompleteData(wxTextEntry *entry)
        : m_entry(entry),
          m_win(entry->GetEditableWindow())
    {
        m_autoComplete = NULL;
        m_autoCompleteDropDown = NULL;
        m_enumStrings = NULL;

        m_fixedCompleter = NULL;
        m_customCompleter = NULL;

        m_connectedCharEvent = false;

                                HRESULT hr = CoCreateInstance
                     (
                        wxCLSID_AutoComplete,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_IAutoComplete,
                        reinterpret_cast<void **>(&m_autoComplete)
                     );
        if ( FAILED(hr) )
        {
            wxLogApiError(wxT("CoCreateInstance(CLSID_AutoComplete)"), hr);
            return;
        }

                m_enumStrings = new wxIEnumString;
        m_enumStrings->AddRef();
        hr = m_autoComplete->Init(m_entry->GetEditHWND(), m_enumStrings,
                                  NULL, NULL);
        if ( FAILED(hr) )
        {
            wxLogApiError(wxT("IAutoComplete::Init"), hr);

            m_enumStrings->Release();
            m_enumStrings = NULL;

            return;
        }

                                                        hr = m_autoComplete->QueryInterface
                             (
                               wxIID_IAutoCompleteDropDown,
                               reinterpret_cast<void **>(&m_autoCompleteDropDown)
                             );
        if ( FAILED(hr) )
        {
            wxLogApiError(wxT("IAutoComplete::QI(IAutoCompleteDropDown)"), hr);
            return;
        }

                IAutoComplete2 *pAutoComplete2 = NULL;
        hr = m_autoComplete->QueryInterface
                             (
                               IID_IAutoComplete2,
                               reinterpret_cast<void **>(&pAutoComplete2)
                             );
        if ( SUCCEEDED(hr) )
        {
            pAutoComplete2->SetOptions(ACO_AUTOSUGGEST |
                                       ACO_AUTOAPPEND |
                                       ACO_UPDOWNKEYDROPSLIST);
            pAutoComplete2->Release();
        }

        m_win->Bind(wxEVT_CHAR_HOOK, &wxTextAutoCompleteData::OnCharHook, this);
    }

    ~wxTextAutoCompleteData()
    {
        delete m_customCompleter;
        delete m_fixedCompleter;

        if ( m_enumStrings )
            m_enumStrings->Release();
        if ( m_autoCompleteDropDown )
            m_autoCompleteDropDown->Release();
        if ( m_autoComplete )
            m_autoComplete->Release();
    }

            bool IsOk() const
    {
        return m_autoComplete && m_autoCompleteDropDown && m_enumStrings;
    }

    void ChangeStrings(const wxArrayString& strings)
    {
        if ( !m_fixedCompleter )
            m_fixedCompleter = new wxTextCompleterFixed;

        m_fixedCompleter->SetCompletions(strings);

        m_enumStrings->ChangeCompleter(m_fixedCompleter);

        DoRefresh();
    }

        bool ChangeCustomCompleter(wxTextCompleter *completer)
    {
                        m_enumStrings->ChangeCompleter(completer);

        delete m_customCompleter;
        m_customCompleter = completer;

        if ( m_customCompleter )
        {
                                                if ( !m_connectedCharEvent )
            {
                m_connectedCharEvent = true;

                                                                                                                                                                                                                m_win->Bind(wxEVT_AFTER_CHAR,
                            &wxTextAutoCompleteData::OnAfterChar, this);
            }

            UpdateStringsFromCustomCompleter();
        }

        return true;
    }

    void DisableCompletion()
    {
                                        ChangeStrings(wxArrayString());
    }

private:
            void DoRefresh()
    {
        m_enumStrings->Reset();

                                                                        m_autoCompleteDropDown->ResetEnumerator();
    }

            void UpdateStringsFromCustomCompleter()
    {
                                long from, to;
        m_entry->GetSelection(&from, &to);

        if ( to == from )
            from = m_entry->GetLastPosition(); 
        const wxString prefix = m_entry->GetRange(0, from);

        m_enumStrings->UpdatePrefix(prefix);

        DoRefresh();
    }

    void OnAfterChar(wxKeyEvent& event)
    {
                                if ( m_customCompleter && event.GetKeyCode() != WXK_BACK )
            UpdateStringsFromCustomCompleter();

        event.Skip();
    }

    void OnCharHook(wxKeyEvent& event)
    {
                                        if ( event.GetKeyCode() == WXK_ESCAPE )
        {
            DWORD dwFlags = 0;
            if ( SUCCEEDED(m_autoCompleteDropDown->GetDropDownStatus(&dwFlags,
                                                                     NULL))
                    && dwFlags == ACDD_VISIBLE )
            {
                ::SendMessage(GetHwndOf(m_win), WM_KEYDOWN, WXK_ESCAPE, 0);

                                return;
            }
        }

        event.Skip();
    }

        wxTextEntry * const m_entry;

        wxWindow * const m_win;

        IAutoComplete *m_autoComplete;

        IAutoCompleteDropDown *m_autoCompleteDropDown;

        wxIEnumString *m_enumStrings;

        wxTextCompleterFixed *m_fixedCompleter;

        wxTextCompleter *m_customCompleter;

        bool m_connectedCharEvent;


    wxDECLARE_NO_COPY_CLASS(wxTextAutoCompleteData);
};

#endif 


wxTextEntry::wxTextEntry()
{
#ifdef HAS_AUTOCOMPLETE
    m_autoCompleteData = NULL;
#endif }

wxTextEntry::~wxTextEntry()
{
#ifdef HAS_AUTOCOMPLETE
    delete m_autoCompleteData;
#endif }


void wxTextEntry::WriteText(const wxString& text)
{
    ::SendMessage(GetEditHwnd(), EM_REPLACESEL, 0, wxMSW_CONV_LPARAM(text));
}

wxString wxTextEntry::DoGetValue() const
{
    return wxGetWindowText(GetEditHWND());
}

void wxTextEntry::Remove(long from, long to)
{
    DoSetSelection(from, to, SetSel_NoScroll);
    WriteText(wxString());
}


void wxTextEntry::Copy()
{
    ::SendMessage(GetEditHwnd(), WM_COPY, 0, 0);
}

void wxTextEntry::Cut()
{
    ::SendMessage(GetEditHwnd(), WM_CUT, 0, 0);
}

void wxTextEntry::Paste()
{
    ::SendMessage(GetEditHwnd(), WM_PASTE, 0, 0);
}


void wxTextEntry::Undo()
{
    ::SendMessage(GetEditHwnd(), EM_UNDO, 0, 0);
}

void wxTextEntry::Redo()
{
        Undo();
    return;
}

bool wxTextEntry::CanUndo() const
{
    return ::SendMessage(GetEditHwnd(), EM_CANUNDO, 0, 0) != 0;
}

bool wxTextEntry::CanRedo() const
{
        return CanUndo();
}


void wxTextEntry::SetInsertionPoint(long pos)
{
            if ( pos == -1 )
        pos = GetLastPosition();

            DoSetSelection(pos, pos);
}

long wxTextEntry::GetInsertionPoint() const
{
    long from;
    GetSelection(&from, NULL);
    return from;
}

long wxTextEntry::GetLastPosition() const
{
    return ::SendMessage(GetEditHwnd(), EM_LINELENGTH, 0, 0);
}

void wxTextEntry::DoSetSelection(long from, long to, int WXUNUSED(flags))
{
            if ( (from == -1) && (to == -1) )
    {
        from = 0;
    }

    ::SendMessage(GetEditHwnd(), EM_SETSEL, from, to);
}

void wxTextEntry::GetSelection(long *from, long *to) const
{
    DWORD dwStart, dwEnd;
    ::SendMessage(GetEditHwnd(), EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);

    if ( from )
        *from = dwStart;
    if ( to )
        *to = dwEnd;
}


#if wxUSE_OLE

#ifdef HAS_AUTOCOMPLETE

#if wxUSE_DYNLIB_CLASS

bool wxTextEntry::DoAutoCompleteFileNames(int flags)
{
    DWORD dwFlags = 0;
    if ( flags & wxFILE )
        dwFlags |= SHACF_FILESYS_ONLY;
    else if ( flags & wxDIR )
        dwFlags |= SHACF_FILESYS_DIRS;
    else
    {
        wxFAIL_MSG(wxS("No flags for file name auto completion?"));
        return false;
    }

    HRESULT hr = ::SHAutoComplete(GetEditHwnd(), dwFlags);
    if ( FAILED(hr) )
    {
        wxLogApiError(wxT("SHAutoComplete()"), hr);

        return false;
    }

            if ( m_autoCompleteData )
        m_autoCompleteData->DisableCompletion();

    return true;
}

#endif 
wxTextAutoCompleteData *wxTextEntry::GetOrCreateCompleter()
{
    if ( !m_autoCompleteData )
    {
        wxTextAutoCompleteData * const ac = new wxTextAutoCompleteData(this);
        if ( ac->IsOk() )
            m_autoCompleteData = ac;
        else
            delete ac;
    }

    return m_autoCompleteData;
}

bool wxTextEntry::DoAutoCompleteStrings(const wxArrayString& choices)
{
    wxTextAutoCompleteData * const ac = GetOrCreateCompleter();
    if ( !ac )
        return false;

    ac->ChangeStrings(choices);

    return true;
}

bool wxTextEntry::DoAutoCompleteCustom(wxTextCompleter *completer)
{
        if ( !completer )
    {
        if ( m_autoCompleteData )
            m_autoCompleteData->DisableCompletion();
            }
    else     {
        wxTextAutoCompleteData * const ac = GetOrCreateCompleter();
        if ( !ac )
        {
                                    delete completer;
            return false;
        }

                if ( !ac->ChangeCustomCompleter(completer) )
            return false;
    }

    return true;
}

#else 

bool wxTextEntry::DoAutoCompleteFileNames(int flags)
{
    return wxTextEntryBase::DoAutoCompleteFileNames(flags);
}

bool wxTextEntry::DoAutoCompleteStrings(const wxArrayString& choices)
{
    return wxTextEntryBase::DoAutoCompleteStrings(choices);
}

bool wxTextEntry::DoAutoCompleteCustom(wxTextCompleter *completer)
{
    return wxTextEntryBase::DoAutoCompleteCustom(completer);
}

#endif 
#endif 

bool wxTextEntry::IsEditable() const
{
    return !(::GetWindowLong(GetEditHwnd(), GWL_STYLE) & ES_READONLY);
}

void wxTextEntry::SetEditable(bool editable)
{
    ::SendMessage(GetEditHwnd(), EM_SETREADONLY, !editable, 0);
}


void wxTextEntry::SetMaxLength(unsigned long len)
{
    if ( len >= 0xffff )
    {
                        len = 0;
    }

    ::SendMessage(GetEditHwnd(), EM_LIMITTEXT, len, 0);
}

void wxTextEntry::ForceUpper()
{
    ConvertToUpperCase();

    const HWND hwnd = GetEditHwnd();
    const LONG styleOld = ::GetWindowLong(hwnd, GWL_STYLE);
    ::SetWindowLong(hwnd, GWL_STYLE, styleOld | ES_UPPERCASE);
}


#if wxUSE_UXTHEME

#ifndef EM_SETCUEBANNER
    #define EM_SETCUEBANNER 0x1501
    #define EM_GETCUEBANNER 0x1502
#endif

bool wxTextEntry::SetHint(const wxString& hint)
{
    if ( wxGetWinVersion() >= wxWinVersion_Vista && wxUxThemeEngine::GetIfActive() )
    {
                                                if ( ::SendMessage(GetEditHwnd(), EM_SETCUEBANNER,
                             TRUE, (LPARAM)(const wchar_t *)hint.wc_str()) )
            return true;
    }

    return wxTextEntryBase::SetHint(hint);
}

wxString wxTextEntry::GetHint() const
{
    if ( wxUxThemeEngine::GetIfActive() )
    {
        wchar_t buf[256];
        if ( ::SendMessage(GetEditHwnd(), EM_GETCUEBANNER,
                            (WPARAM)buf, WXSIZEOF(buf)) )
            return wxString(buf);
    }

    return wxTextEntryBase::GetHint();
}


#endif 

bool wxTextEntry::DoSetMargins(const wxPoint& margins)
{
    bool res = true;

    if ( margins.x != -1 )
    {
                                ::SendMessage(GetEditHwnd(), EM_SETMARGINS,
                      EC_LEFTMARGIN | EC_RIGHTMARGIN,
                      MAKELONG(margins.x, margins.x));
    }

    if ( margins.y != -1 )
    {
        res = false;
    }

    return res;
}

wxPoint wxTextEntry::DoGetMargins() const
{
    LRESULT lResult = ::SendMessage(GetEditHwnd(), EM_GETMARGINS,
                                    0, 0);
    int left = LOWORD(lResult);
    int top = -1;
    return wxPoint(left, top);
}

#endif 