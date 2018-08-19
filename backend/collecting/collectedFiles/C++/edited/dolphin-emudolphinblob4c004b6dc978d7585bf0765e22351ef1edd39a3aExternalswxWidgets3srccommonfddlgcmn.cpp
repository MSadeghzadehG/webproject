


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_FINDREPLDLG

#ifndef WX_PRECOMP
#endif

#include "wx/fdrepdlg.h"


wxIMPLEMENT_DYNAMIC_CLASS(wxFindDialogEvent, wxCommandEvent);

wxDEFINE_EVENT( wxEVT_FIND, wxFindDialogEvent );
wxDEFINE_EVENT( wxEVT_FIND_NEXT, wxFindDialogEvent );
wxDEFINE_EVENT( wxEVT_FIND_REPLACE, wxFindDialogEvent );
wxDEFINE_EVENT( wxEVT_FIND_REPLACE_ALL, wxFindDialogEvent );
wxDEFINE_EVENT( wxEVT_FIND_CLOSE, wxFindDialogEvent );



void wxFindReplaceData::Init()
{
    m_Flags = 0;
}


wxFindReplaceDialogBase::~wxFindReplaceDialogBase()
{
}

void wxFindReplaceDialogBase::Send(wxFindDialogEvent& event)
{
    
    m_FindReplaceData->m_Flags = event.GetFlags();
    m_FindReplaceData->m_FindWhat = event.GetFindString();
    if ( HasFlag(wxFR_REPLACEDIALOG) &&
         (event.GetEventType() == wxEVT_FIND_REPLACE ||
          event.GetEventType() == wxEVT_FIND_REPLACE_ALL) )
    {
        m_FindReplaceData->m_ReplaceWith = event.GetReplaceString();
    }

        if ( event.GetEventType() == wxEVT_FIND_NEXT )
    {
        if ( m_FindReplaceData->m_FindWhat != m_lastSearch )
        {
            event.SetEventType(wxEVT_FIND);

            m_lastSearch = m_FindReplaceData->m_FindWhat;
        }
    }

    if ( !GetEventHandler()->ProcessEvent(event) )
    {
                                        (void)GetParent()->GetEventHandler()->ProcessEvent(event);
    }
}

#endif 
