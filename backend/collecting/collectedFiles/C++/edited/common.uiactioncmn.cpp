
#include "wx/wxprec.h"

#if wxUSE_UIACTIONSIMULATOR

#include "wx/uiaction.h"

#include "wx/ctrlsub.h"

#ifdef wxNO_RTTI
    #include "wx/choice.h"
    #include "wx/combobox.h"
    #include "wx/listbox.h"
#endif 

bool wxUIActionSimulator::MouseClick(int button)
{
    MouseDown(button);
    MouseUp(button);

    return true;
}

#ifndef __WXOSX__

bool wxUIActionSimulator::MouseDblClick(int button)
{
    MouseDown(button);
    MouseUp(button);
    MouseDown(button);
    MouseUp(button);

    return true;
}

bool wxUIActionSimulator::MouseDragDrop(long x1, long y1, long x2, long y2,
                                   int button)
{
    MouseMove(x1, y1);
    MouseDown(button);
    MouseMove(x2, y2);
    MouseUp(button);
    
    return true;
}

#endif

bool
wxUIActionSimulator::Key(int keycode, int modifiers, bool isDown)
{
    wxASSERT_MSG( !(modifiers & wxMOD_META ),
        "wxMOD_META is not implemented" );
    wxASSERT_MSG( !(modifiers & wxMOD_WIN ),
        "wxMOD_WIN is not implemented" );

    if ( isDown )
        SimulateModifiers(modifiers, true);

    bool rc = DoKey(keycode, modifiers, isDown);

    if ( !isDown )
        SimulateModifiers(modifiers, false);

    return rc;
}

void wxUIActionSimulator::SimulateModifiers(int modifiers, bool isDown)
{
    if ( modifiers & wxMOD_SHIFT )
        DoKey(WXK_SHIFT, modifiers, isDown);
    if ( modifiers & wxMOD_ALT )
        DoKey(WXK_ALT, modifiers, isDown);
    if ( modifiers & wxMOD_CONTROL )
        DoKey(WXK_CONTROL, modifiers, isDown);
}

bool wxUIActionSimulator::Char(int keycode, int modifiers)
{
    Key(keycode, modifiers, true);
    Key(keycode, modifiers, false);

    return true;
}

static bool MapUnshifted(char& ch)
{
    const char* const unshifted =
        "`1234567890-=\\"
        "[]"
        ";'"
        ",./"
        ;

    const char* const shifted =
        "~!@#$%^&*()_+|"
        "{}"
        ":\""
        "<>?"
        ;

    wxCOMPILE_TIME_ASSERT( sizeof(unshifted) == sizeof(shifted),
                           ShiftedUnshiftedKeysMismatch );

    const char* const p = strchr(shifted, ch);
    if ( !p )
        return false;

    ch = *(unshifted + (p - shifted));

    return true;
}

bool wxUIActionSimulator::Text(const char *s)
{
    while ( *s != '\0' )
    {
        char ch = *s++;

                        int modifiers = 0;
        if ( isupper(ch) || MapUnshifted(ch) )
            modifiers |= wxMOD_SHIFT;

        if ( !Char(ch, modifiers) )
            return false;
    }

    return true;
}

bool wxUIActionSimulator::Select(const wxString& text)
{
    wxWindow* const focus = wxWindow::FindFocus();
    if ( !focus )
        return false;

        #ifdef wxNO_RTTI
    wxItemContainer* container = NULL;

    if ( wxComboBox* combo = wxDynamicCast(focus, wxComboBox) )
        container = combo;
    else if ( wxChoice* choice = wxDynamicCast(focus, wxChoice) )
        container = choice;
    else if ( wxListBox* listbox = wxDynamicCast(focus, wxListBox) )
        container = listbox;
#else     wxItemContainer* const container = dynamic_cast<wxItemContainer*>(focus);
#endif 
    if ( !container )
        return false;

                    if ( container->GetSelection() != 0 )
    {
        Char(WXK_HOME);
        wxYield();

                if ( container->GetSelection() != 0 )
            container->SetSelection(0);
    }

        for ( ;; )
    {
        if ( container->GetStringSelection() == text )
            return true;

                                        const int current = container->GetSelection();

        Char(WXK_DOWN);
        wxYield();

        if ( container->GetSelection() == current )
            break;
    }

    return false;
}

#endif 