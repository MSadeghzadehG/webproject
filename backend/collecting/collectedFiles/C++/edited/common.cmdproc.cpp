


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/intl.h"
    #include "wx/string.h"
    #include "wx/menu.h"
    #include "wx/accel.h"
#endif 
#include "wx/cmdproc.h"


wxIMPLEMENT_CLASS(wxCommand, wxObject);
wxIMPLEMENT_DYNAMIC_CLASS(wxCommandProcessor, wxObject);


wxCommand::wxCommand(bool canUndoIt, const wxString& name)
{
    m_canUndo = canUndoIt;
    m_commandName = name;
}


wxCommandProcessor::wxCommandProcessor(int maxCommands)
{
    m_maxNoCommands = maxCommands;
#if wxUSE_MENUS
    m_commandEditMenu = NULL;
#endif 
#if wxUSE_ACCEL
    m_undoAccelerator = '\t' + wxAcceleratorEntry(wxACCEL_CTRL, 'Z').ToString();
    m_redoAccelerator = '\t' + wxAcceleratorEntry(wxACCEL_CTRL, 'Y').ToString();
#endif 
    m_lastSavedCommand =
    m_currentCommand = wxList::compatibility_iterator();
}

wxCommandProcessor::~wxCommandProcessor()
{
    ClearCommands();
}

bool wxCommandProcessor::DoCommand(wxCommand& cmd)
{
    return cmd.Do();
}

bool wxCommandProcessor::UndoCommand(wxCommand& cmd)
{
    return cmd.Undo();
}

bool wxCommandProcessor::Submit(wxCommand *command, bool storeIt)
{
    wxCHECK_MSG( command, false, wxT("no command in wxCommandProcessor::Submit") );

    if ( !DoCommand(*command) )
    {
                delete command;

        return false;
    }

    if ( storeIt )
        Store(command);
    else
        delete command;

    return true;
}

void wxCommandProcessor::Store(wxCommand *command)
{
    wxCHECK_RET( command, wxT("no command in wxCommandProcessor::Store") );

            if (!m_currentCommand)
        ClearCommands();
    else
    {
        wxList::compatibility_iterator node = m_currentCommand->GetNext();
        while (node)
        {
            wxList::compatibility_iterator next = node->GetNext();

                        if ( m_lastSavedCommand && m_lastSavedCommand == node )
                m_lastSavedCommand = wxList::compatibility_iterator();

            delete (wxCommand *)node->GetData();
            m_commands.Erase(node);

            node = next;
        }
    }

    if ( (int)m_commands.GetCount() == m_maxNoCommands )
    {
        wxList::compatibility_iterator firstNode = m_commands.GetFirst();

                if ( m_lastSavedCommand && m_lastSavedCommand == firstNode )
            m_lastSavedCommand = wxList::compatibility_iterator();

        wxCommand *firstCommand = (wxCommand *)firstNode->GetData();
        delete firstCommand;
        m_commands.Erase(firstNode);
    }

    m_commands.Append(command);
    m_currentCommand = m_commands.GetLast();
    SetMenuStrings();
}

bool wxCommandProcessor::Undo()
{
    wxCommand *command = GetCurrentCommand();
    if ( command && command->CanUndo() )
    {
        if ( UndoCommand(*command) )
        {
            m_currentCommand = m_currentCommand->GetPrevious();
            SetMenuStrings();
            return true;
        }
    }

    return false;
}

bool wxCommandProcessor::Redo()
{
    wxCommand *redoCommand = NULL;
    wxList::compatibility_iterator redoNode
#if !wxUSE_STD_CONTAINERS
        = NULL          #endif         ;

    if ( m_currentCommand )
    {
                if ( m_currentCommand->GetNext() )
        {
            redoCommand = (wxCommand *)m_currentCommand->GetNext()->GetData();
            redoNode = m_currentCommand->GetNext();
        }
    }
    else     {
        if (m_commands.GetCount() > 0)
        {
            redoCommand = (wxCommand *)m_commands.GetFirst()->GetData();
            redoNode = m_commands.GetFirst();
        }
    }

    if (redoCommand)
    {
        bool success = DoCommand(*redoCommand);
        if (success)
        {
            m_currentCommand = redoNode;
            SetMenuStrings();
            return true;
        }
    }
    return false;
}

bool wxCommandProcessor::CanUndo() const
{
    wxCommand *command = GetCurrentCommand();

    return command && command->CanUndo();
}

bool wxCommandProcessor::CanRedo() const
{
    if (m_currentCommand && !m_currentCommand->GetNext())
        return false;

    if (m_currentCommand && m_currentCommand->GetNext())
        return true;

    if (!m_currentCommand && (m_commands.GetCount() > 0))
        return true;

    return false;
}

void wxCommandProcessor::Initialize()
{
    m_currentCommand = m_commands.GetLast();
    SetMenuStrings();
}

void wxCommandProcessor::SetMenuStrings()
{
#if wxUSE_MENUS
    if (m_commandEditMenu)
    {
        wxString undoLabel = GetUndoMenuLabel();
        wxString redoLabel = GetRedoMenuLabel();

        m_commandEditMenu->SetLabel(wxID_UNDO, undoLabel);
        m_commandEditMenu->Enable(wxID_UNDO, CanUndo());

        m_commandEditMenu->SetLabel(wxID_REDO, redoLabel);
        m_commandEditMenu->Enable(wxID_REDO, CanRedo());
    }
#endif }

wxString wxCommandProcessor::GetUndoMenuLabel() const
{
    wxString buf;
    if (m_currentCommand)
    {
        wxCommand *command = (wxCommand *)m_currentCommand->GetData();
        wxString commandName(command->GetName());
        if (commandName.empty()) commandName = _("Unnamed command");
        bool canUndo = command->CanUndo();
        if (canUndo)
            buf = wxString(_("&Undo ")) + commandName + m_undoAccelerator;
        else
            buf = wxString(_("Can't &Undo ")) + commandName + m_undoAccelerator;
    }
    else
    {
        buf = _("&Undo") + m_undoAccelerator;
    }

    return buf;
}

wxString wxCommandProcessor::GetRedoMenuLabel() const
{
    wxString buf;
    if (m_currentCommand)
    {
                if (m_currentCommand->GetNext())
        {
            wxCommand *redoCommand = (wxCommand *)m_currentCommand->GetNext()->GetData();
            wxString redoCommandName(redoCommand->GetName());
            if (redoCommandName.empty()) redoCommandName = _("Unnamed command");
            buf = wxString(_("&Redo ")) + redoCommandName + m_redoAccelerator;
        }
        else
        {
            buf = _("&Redo") + m_redoAccelerator;
        }
    }
    else
    {
        if (m_commands.GetCount() == 0)
        {
            buf = _("&Redo") + m_redoAccelerator;
        }
        else
        {
                                    wxCommand *redoCommand = (wxCommand *)m_commands.GetFirst()->GetData();
            wxString redoCommandName(redoCommand->GetName());
            if (redoCommandName.empty()) redoCommandName = _("Unnamed command");
            buf = wxString(_("&Redo ")) + redoCommandName + m_redoAccelerator;
        }
    }
    return buf;
}

void wxCommandProcessor::ClearCommands()
{
    wxList::compatibility_iterator node = m_commands.GetFirst();
    while (node)
    {
        wxCommand *command = (wxCommand *)node->GetData();
        delete command;
        m_commands.Erase(node);
        node = m_commands.GetFirst();
    }

    m_currentCommand = wxList::compatibility_iterator();
    m_lastSavedCommand = wxList::compatibility_iterator();
}

bool wxCommandProcessor::IsDirty() const
{
    if ( !m_lastSavedCommand )
    {
                                                return !!m_currentCommand;
    }

    if ( !m_currentCommand )
    {
                        return true;
    }

        return m_currentCommand != m_lastSavedCommand;
}

