


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/modalhook.h"

#ifndef WX_PRECOMP
#endif 
wxModalDialogHook::Hooks wxModalDialogHook::ms_hooks;



void wxModalDialogHook::Register()
{
#if wxDEBUG_LEVEL
    for ( Hooks::const_iterator it = ms_hooks.begin();
          it != ms_hooks.end();
          ++it)
    {
        if ( *it == this )
        {
            wxFAIL_MSG( wxS("Registering already registered hook?") );
            return;
        }
    }
#endif 
    ms_hooks.insert(ms_hooks.begin(), this);
}

void wxModalDialogHook::Unregister()
{
    if ( !DoUnregister() )
    {
        wxFAIL_MSG( wxS("Unregistering not registered hook?") );
    }
}

bool wxModalDialogHook::DoUnregister()
{
    for ( Hooks::iterator it = ms_hooks.begin();
          it != ms_hooks.end();
          ++it )
    {
        if ( *it == this )
        {
            ms_hooks.erase(it);
            return true;
        }
    }

    return false;
}



int wxModalDialogHook::CallEnter(wxDialog* dialog)
{
                    const Hooks hooks = ms_hooks;

    for ( Hooks::const_iterator it = hooks.begin(); it != hooks.end(); ++it )
    {
        const int rc = (*it)->Enter(dialog);
        if ( rc != wxID_NONE )
        {
                                    return rc;
        }
    }

    return wxID_NONE;
}


void wxModalDialogHook::CallExit(wxDialog* dialog)
{
        const Hooks hooks = ms_hooks;

    for ( Hooks::const_iterator it = hooks.begin(); it != hooks.end(); ++it )
    {
        (*it)->Exit(dialog);
    }
}
