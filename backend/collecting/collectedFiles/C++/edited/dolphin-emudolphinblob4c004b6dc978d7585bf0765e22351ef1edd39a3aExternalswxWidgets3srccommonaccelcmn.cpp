


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_ACCEL

#ifndef WX_PRECOMP
    #include "wx/accel.h"
    #include "wx/string.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/crt.h"
#endif 
wxAcceleratorTable wxNullAcceleratorTable;


wxGCC_WARNING_SUPPRESS(missing-field-initializers)

static const struct wxKeyName
{
    wxKeyCode code;
    const char *name;
    const char *display_name;
} wxKeyNames[] =
{
    { WXK_DELETE,            wxTRANSLATE("Delete") },
    { WXK_DELETE,            wxTRANSLATE("Del") },
    { WXK_BACK,              wxTRANSLATE("Back"),           wxTRANSLATE("Backspace") },
    { WXK_INSERT,            wxTRANSLATE("Insert") },
    { WXK_INSERT,            wxTRANSLATE("Ins") },
    { WXK_RETURN,            wxTRANSLATE("Enter") },
    { WXK_RETURN,            wxTRANSLATE("Return") },
    { WXK_PAGEUP,            wxTRANSLATE("PageUp"),         wxTRANSLATE("Page Up") },
    { WXK_PAGEDOWN,          wxTRANSLATE("PageDown"),       wxTRANSLATE("Page Down") },
    { WXK_PAGEUP,            wxTRANSLATE("PgUp") },
    { WXK_PAGEDOWN,          wxTRANSLATE("PgDn") },
    { WXK_LEFT,              wxTRANSLATE("Left"),           wxTRANSLATE("Left") },
    { WXK_RIGHT,             wxTRANSLATE("Right"),          wxTRANSLATE("Right") },
    { WXK_UP,                wxTRANSLATE("Up"),             wxTRANSLATE("Up") },
    { WXK_DOWN,              wxTRANSLATE("Down"),           wxTRANSLATE("Down") },
    { WXK_HOME,              wxTRANSLATE("Home") },
    { WXK_END,               wxTRANSLATE("End") },
    { WXK_SPACE,             wxTRANSLATE("Space") },
    { WXK_TAB,               wxTRANSLATE("Tab") },
    { WXK_ESCAPE,            wxTRANSLATE("Esc") },
    { WXK_ESCAPE,            wxTRANSLATE("Escape") },
    { WXK_CANCEL,            wxTRANSLATE("Cancel") },
    { WXK_CLEAR,             wxTRANSLATE("Clear") },
    { WXK_MENU,              wxTRANSLATE("Menu") },
    { WXK_PAUSE,             wxTRANSLATE("Pause") },
    { WXK_CAPITAL,           wxTRANSLATE("Capital") },
    { WXK_SELECT,            wxTRANSLATE("Select") },
    { WXK_PRINT,             wxTRANSLATE("Print") },
    { WXK_EXECUTE,           wxTRANSLATE("Execute") },
    { WXK_SNAPSHOT,          wxTRANSLATE("Snapshot") },
    { WXK_HELP,              wxTRANSLATE("Help") },
    { WXK_ADD,               wxTRANSLATE("Add") },
    { WXK_SEPARATOR,         wxTRANSLATE("Separator") },
    { WXK_SUBTRACT,          wxTRANSLATE("Subtract") },
    { WXK_DECIMAL,           wxTRANSLATE("Decimal") },
    { WXK_DIVIDE,            wxTRANSLATE("Divide") },
    { WXK_NUMLOCK,           wxTRANSLATE("Num_lock"),       wxTRANSLATE("Num Lock") },
    { WXK_SCROLL,            wxTRANSLATE("Scroll_lock"),    wxTRANSLATE("Scroll Lock") },
    { WXK_NUMPAD_SPACE,      wxTRANSLATE("KP_Space"),       wxTRANSLATE("Num Space") },
    { WXK_NUMPAD_TAB,        wxTRANSLATE("KP_Tab"),         wxTRANSLATE("Num Tab") },
    { WXK_NUMPAD_ENTER,      wxTRANSLATE("KP_Enter"),       wxTRANSLATE("Num Enter") },
    { WXK_NUMPAD_HOME,       wxTRANSLATE("KP_Home"),        wxTRANSLATE("Num Home") },
    { WXK_NUMPAD_LEFT,       wxTRANSLATE("KP_Left"),        wxTRANSLATE("Num left") },
    { WXK_NUMPAD_UP,         wxTRANSLATE("KP_Up"),          wxTRANSLATE("Num Up") },
    { WXK_NUMPAD_RIGHT,      wxTRANSLATE("KP_Right"),       wxTRANSLATE("Num Right") },
    { WXK_NUMPAD_DOWN,       wxTRANSLATE("KP_Down"),        wxTRANSLATE("Num Down") },
    { WXK_NUMPAD_PAGEUP,     wxTRANSLATE("KP_PageUp"),      wxTRANSLATE("Num Page Up") },
    { WXK_NUMPAD_PAGEDOWN,   wxTRANSLATE("KP_PageDown"),    wxTRANSLATE("Num Page Down") },
    { WXK_NUMPAD_PAGEUP,     wxTRANSLATE("KP_Prior") },
    { WXK_NUMPAD_PAGEDOWN,   wxTRANSLATE("KP_Next") },
    { WXK_NUMPAD_END,        wxTRANSLATE("KP_End"),         wxTRANSLATE("Num End") },
    { WXK_NUMPAD_BEGIN,      wxTRANSLATE("KP_Begin"),       wxTRANSLATE("Num Begin") },
    { WXK_NUMPAD_INSERT,     wxTRANSLATE("KP_Insert"),      wxTRANSLATE("Num Insert") },
    { WXK_NUMPAD_DELETE,     wxTRANSLATE("KP_Delete"),      wxTRANSLATE("Num Delete") },
    { WXK_NUMPAD_EQUAL,      wxTRANSLATE("KP_Equal"),       wxTRANSLATE("Num =") },
    { WXK_NUMPAD_MULTIPLY,   wxTRANSLATE("KP_Multiply"),    wxTRANSLATE("Num *") },
    { WXK_NUMPAD_ADD,        wxTRANSLATE("KP_Add"),         wxTRANSLATE("Num +") },
    { WXK_NUMPAD_SEPARATOR,  wxTRANSLATE("KP_Separator"),   wxTRANSLATE("Num ,") },
    { WXK_NUMPAD_SUBTRACT,   wxTRANSLATE("KP_Subtract"),    wxTRANSLATE("Num -") },
    { WXK_NUMPAD_DECIMAL,    wxTRANSLATE("KP_Decimal"),     wxTRANSLATE("Num .") },
    { WXK_NUMPAD_DIVIDE,     wxTRANSLATE("KP_Divide"),      wxTRANSLATE("Num /") },
    { WXK_WINDOWS_LEFT,      wxTRANSLATE("Windows_Left") },
    { WXK_WINDOWS_RIGHT,     wxTRANSLATE("Windows_Right") },
    { WXK_WINDOWS_MENU,      wxTRANSLATE("Windows_Menu") },
    { WXK_COMMAND,           wxTRANSLATE("Command") },
};

wxGCC_WARNING_RESTORE(missing-field-initializers)

static inline bool CompareAccelString(const wxString& str, const char *accel)
{
    return str.CmpNoCase(accel) == 0
#if wxUSE_INTL
            || str.CmpNoCase(wxGetTranslation(accel)) == 0
#endif
            ;
}

static int IsNumberedAccelKey(const wxString& str,
                              const char *prefix,
                              wxKeyCode prefixCode,
                              unsigned first,
                              unsigned last)
{
    const size_t lenPrefix = wxStrlen(prefix);
    if ( !CompareAccelString(str.Left(lenPrefix), prefix) )
        return 0;

    unsigned long num;
    if ( !str.Mid(lenPrefix).ToULong(&num) )
        return 0;

    if ( num < first || num > last )
    {
                        wxLogDebug(wxT("Invalid key string \"%s\""), str.c_str());
        return 0;
    }

    return prefixCode + num - first;
}


bool
wxAcceleratorEntry::ParseAccel(const wxString& text, int *flagsOut, int *keyOut)
{
        wxString label = text;
    label.Trim(true);

                        int posTab = label.Find(wxT('\t'));
    if ( posTab == wxNOT_FOUND )
        posTab = 0;
    else
        posTab++;

        int accelFlags = wxACCEL_NORMAL;
    wxString current;
    for ( size_t n = (size_t)posTab; n < label.length(); n++ )
    {
        if ( (label[n] == '+') || (label[n] == '-') )
        {
            if ( CompareAccelString(current, wxTRANSLATE("ctrl")) )
                accelFlags |= wxACCEL_CTRL;
            else if ( CompareAccelString(current, wxTRANSLATE("alt")) )
                accelFlags |= wxACCEL_ALT;
            else if ( CompareAccelString(current, wxTRANSLATE("shift")) )
                accelFlags |= wxACCEL_SHIFT;
            else if ( CompareAccelString(current, wxTRANSLATE("rawctrl")) )
                accelFlags |= wxACCEL_RAW_CTRL;
            else             {
                                                                                if ( current.empty() )
                {
                    current += label[n];

                                        continue;
                }
                else
                {
                    wxLogDebug(wxT("Unknown accel modifier: '%s'"),
                               current.c_str());
                }
            }

            current.clear();
        }
        else         {
            current += (wxChar) wxTolower(label[n]);
        }
    }

    int keyCode;
    const size_t len = current.length();
    switch ( len )
    {
        case 0:
            wxLogDebug(wxT("No accel key found, accel string ignored."));
            return false;

        case 1:
                        keyCode = current[0U];

                                                if ( accelFlags != wxACCEL_NORMAL )
                keyCode = wxToupper(keyCode);
            break;

        default:
            keyCode = IsNumberedAccelKey(current, wxTRANSLATE("F"),
                                         WXK_F1, 1, 12);
            if ( !keyCode )
            {
                for ( size_t n = 0; n < WXSIZEOF(wxKeyNames); n++ )
                {
                    const wxKeyName& kn = wxKeyNames[n];
                    if ( CompareAccelString(current, kn.name) )
                    {
                        keyCode = kn.code;
                        break;
                    }
                }
            }

            if ( !keyCode )
                keyCode = IsNumberedAccelKey(current, wxTRANSLATE("KP_"),
                                             WXK_NUMPAD0, 0, 9);
            if ( !keyCode )
                keyCode = IsNumberedAccelKey(current, wxTRANSLATE("SPECIAL"),
                                             WXK_SPECIAL1, 1, 20);

            if ( !keyCode )
            {
                wxLogDebug(wxT("Unrecognized accel key '%s', accel string ignored."),
                           current.c_str());
                return false;
            }
    }


    wxASSERT_MSG( keyCode, wxT("logic error: should have key code here") );

    if ( flagsOut )
        *flagsOut = accelFlags;
    if ( keyOut )
        *keyOut = keyCode;

    return true;
}


wxAcceleratorEntry *wxAcceleratorEntry::Create(const wxString& str)
{
    const wxString accelStr = str.AfterFirst('\t');
    if ( accelStr.empty() )
    {
                                return NULL;
    }

    int flags,
        keyCode;
    if ( !ParseAccel(accelStr, &flags, &keyCode) )
        return NULL;

    return new wxAcceleratorEntry(flags, keyCode);
}

bool wxAcceleratorEntry::FromString(const wxString& str)
{
    return ParseAccel(str, &m_flags, &m_keyCode);
}

namespace
{

wxString PossiblyLocalize(const wxString& str, bool localize)
{
    return localize ? wxGetTranslation(str) : str;
}

}

wxString wxAcceleratorEntry::AsPossiblyLocalizedString(bool localized) const
{
    wxString text;

    int flags = GetFlags();
    if ( flags & wxACCEL_ALT )
        text += PossiblyLocalize(wxTRANSLATE("Alt+"), localized);
    if ( flags & wxACCEL_CTRL )
        text += PossiblyLocalize(wxTRANSLATE("Ctrl+"), localized);
    if ( flags & wxACCEL_SHIFT )
        text += PossiblyLocalize(wxTRANSLATE("Shift+"), localized);
#if defined(__WXMAC__)
    if ( flags & wxACCEL_RAW_CTRL )
        text += PossiblyLocalize(wxTRANSLATE("RawCtrl+"), localized);
#endif
    
    const int code = GetKeyCode();

    if ( code >= WXK_F1 && code <= WXK_F12 )
        text << PossiblyLocalize(wxTRANSLATE("F"), localized)
             << code - WXK_F1 + 1;
    else if ( code >= WXK_NUMPAD0 && code <= WXK_NUMPAD9 )
        text << PossiblyLocalize(wxTRANSLATE("KP_"), localized)
             << code - WXK_NUMPAD0;
    else if ( code >= WXK_SPECIAL1 && code <= WXK_SPECIAL20 )
        text << PossiblyLocalize(wxTRANSLATE("SPECIAL"), localized)
             << code - WXK_SPECIAL1 + 1;
    else     {
        size_t n;
        for ( n = 0; n < WXSIZEOF(wxKeyNames); n++ )
        {
            const wxKeyName& kn = wxKeyNames[n];
            if ( code == kn.code )
            {
                text << PossiblyLocalize(kn.display_name ? kn.display_name : kn.name, localized);
                break;
            }
        }

        if ( n == WXSIZEOF(wxKeyNames) )
        {
                        if (
#if !wxUSE_UNICODE
                                                   wxIsascii(code) &&
#endif                     wxIsprint(code) )
            {
                text << (wxChar)code;
            }
            else
            {
                wxFAIL_MSG( wxT("unknown keyboard accelerator code") );
            }
        }
    }

    return text;
}

wxAcceleratorEntry *wxGetAccelFromString(const wxString& label)
{
    return wxAcceleratorEntry::Create(label);
}

#endif 


