


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/filehistory.h"

#if wxUSE_FILE_HISTORY

#include "wx/menu.h"
#include "wx/confbase.h"
#include "wx/filename.h"



namespace
{

wxString GetMRUEntryLabel(int n, const wxString& path)
{
        wxString pathInMenu(path);
    pathInMenu.Replace("&", "&&");

    return wxString::Format("&%d %s", n + 1, pathInMenu);
}

} 

wxIMPLEMENT_DYNAMIC_CLASS(wxFileHistory, wxObject);

wxFileHistoryBase::wxFileHistoryBase(size_t maxFiles, wxWindowID idBase)
{
    m_fileMaxFiles = maxFiles;
    m_idBase = idBase;
}


wxString wxFileHistoryBase::NormalizeFileName(const wxFileName& fn)
{
                        wxFileName fnNorm(fn);
    fnNorm.Normalize(wxPATH_NORM_DOTS |
                     wxPATH_NORM_TILDE |
                     wxPATH_NORM_CASE |
                     wxPATH_NORM_ABSOLUTE);
    return fnNorm.GetFullPath();
}

void wxFileHistoryBase::AddFileToHistory(const wxString& file)
{
                        const wxFileName fnNew(file);
    const wxString newFile = NormalizeFileName(fnNew);
    size_t i,
           numFiles = m_fileHistory.size();
    for ( i = 0; i < numFiles; i++ )
    {
        if ( newFile == NormalizeFileName(m_fileHistory[i]) )
        {
                        RemoveFileFromHistory(i);
            numFiles--;
            break;
        }
    }

        if ( numFiles == m_fileMaxFiles )
    {
        RemoveFileFromHistory(--numFiles);
    }

        for ( wxList::compatibility_iterator node = m_fileMenus.GetFirst();
        node;
        node = node->GetNext() )
    {
        wxMenu * const menu = (wxMenu *)node->GetData();

        if ( !numFiles && menu->GetMenuItemCount() )
            menu->AppendSeparator();

                        menu->Append(m_idBase + numFiles, " ");
    }

        m_fileHistory.insert(m_fileHistory.begin(), file);
    numFiles++;

        for ( i = 0; i < numFiles; i++ )
    {
                const wxFileName fnOld(m_fileHistory[i]);

        wxString pathInMenu;
        if ( (fnOld.GetPath() == fnNew.GetPath()) && fnOld.HasName() )
        {
            pathInMenu = fnOld.GetFullName();
        }
        else         {
                        pathInMenu = m_fileHistory[i];
        }

        for ( wxList::compatibility_iterator node = m_fileMenus.GetFirst();
              node;
              node = node->GetNext() )
        {
            wxMenu * const menu = (wxMenu *)node->GetData();

            menu->SetLabel(m_idBase + i, GetMRUEntryLabel(i, pathInMenu));
        }
    }
}

void wxFileHistoryBase::RemoveFileFromHistory(size_t i)
{
    size_t numFiles = m_fileHistory.size();
    wxCHECK_RET( i < numFiles,
                 wxT("invalid index in wxFileHistoryBase::RemoveFileFromHistory") );

        m_fileHistory.RemoveAt(i);
    numFiles--;

    for ( wxList::compatibility_iterator node = m_fileMenus.GetFirst();
          node;
          node = node->GetNext() )
    {
        wxMenu * const menu = (wxMenu *) node->GetData();

                for ( size_t j = i; j < numFiles; j++ )
        {
            menu->SetLabel(m_idBase + j, GetMRUEntryLabel(j, m_fileHistory[j]));
        }

                const wxWindowID lastItemId = m_idBase + numFiles;
        if ( menu->FindItem(lastItemId) )
            menu->Delete(lastItemId);

                if ( m_fileHistory.empty() )
        {
            const wxMenuItemList::compatibility_iterator
                nodeLast = menu->GetMenuItems().GetLast();
            if ( nodeLast )
            {
                wxMenuItem * const lastMenuItem = nodeLast->GetData();
                if ( lastMenuItem->IsSeparator() )
                    menu->Delete(lastMenuItem);
            }
                    }
    }
}

void wxFileHistoryBase::UseMenu(wxMenu *menu)
{
    if ( !m_fileMenus.Member(menu) )
        m_fileMenus.Append(menu);
}

void wxFileHistoryBase::RemoveMenu(wxMenu *menu)
{
    m_fileMenus.DeleteObject(menu);
}

#if wxUSE_CONFIG
void wxFileHistoryBase::Load(const wxConfigBase& config)
{
    RemoveExistingHistory();

    m_fileHistory.Clear();

    wxString buf;
    buf.Printf(wxT("file%d"), 1);

    wxString historyFile;
    while ((m_fileHistory.GetCount() < m_fileMaxFiles) &&
           config.Read(buf, &historyFile) && !historyFile.empty())
    {
        m_fileHistory.Add(historyFile);

        buf.Printf(wxT("file%d"), (int)m_fileHistory.GetCount()+1);
        historyFile = wxEmptyString;
    }

    AddFilesToMenu();
}

void wxFileHistoryBase::Save(wxConfigBase& config)
{
    size_t i;
    for (i = 0; i < m_fileMaxFiles; i++)
    {
        wxString buf;
        buf.Printf(wxT("file%d"), (int)i+1);
        if (i < m_fileHistory.GetCount())
            config.Write(buf, wxString(m_fileHistory[i]));
        else
            config.Write(buf, wxEmptyString);
    }
}
#endif 
void wxFileHistoryBase::AddFilesToMenu()
{
    if ( m_fileHistory.empty() )
        return;

    for ( wxList::compatibility_iterator node = m_fileMenus.GetFirst();
          node;
          node = node->GetNext() )
    {
        AddFilesToMenu((wxMenu *) node->GetData());
    }
}

void wxFileHistoryBase::AddFilesToMenu(wxMenu* menu)
{
    if ( m_fileHistory.empty() )
        return;

    if ( menu->GetMenuItemCount() )
        menu->AppendSeparator();

    for ( size_t i = 0; i < m_fileHistory.GetCount(); i++ )
    {
        menu->Append(m_idBase + i, GetMRUEntryLabel(i, m_fileHistory[i]));
    }
}

void wxFileHistoryBase::RemoveExistingHistory()
{
    size_t count = m_fileHistory.GetCount();
    if ( !count )
        return;

    for ( wxList::compatibility_iterator node = m_fileMenus.GetFirst();
          node;
          node = node->GetNext() )
    {
        wxMenu * const menu = static_cast<wxMenu *>(node->GetData());

                        for ( size_t n = 0; n <= count; n++ )
        {
            const wxMenuItemList::compatibility_iterator
                nodeLast = menu->GetMenuItems().GetLast();
            if ( nodeLast )
            {
                wxMenuItem * const lastMenuItem = nodeLast->GetData();
                menu->Delete(lastMenuItem);
            }
        }
    }
}

#endif 