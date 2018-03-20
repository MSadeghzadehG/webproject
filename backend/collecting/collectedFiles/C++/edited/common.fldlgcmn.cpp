
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_FILEDLG

#include "wx/filedlg.h"
#include "wx/dirdlg.h"
#include "wx/filename.h"

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/intl.h"
    #include "wx/window.h"
#endif 
extern WXDLLEXPORT_DATA(const char) wxFileDialogNameStr[] = "filedlg";
extern WXDLLEXPORT_DATA(const char) wxFileSelectorPromptStr[] = "Select a file";
extern WXDLLEXPORT_DATA(const char) wxFileSelectorDefaultWildcardStr[] =
#if defined(__WXMSW__)
    "*.*"
#else     "*"
#endif
    ;


wxIMPLEMENT_DYNAMIC_CLASS(wxFileDialogBase, wxDialog);

void wxFileDialogBase::Init()
{
    m_filterIndex = 0;
    m_windowStyle = 0;
    m_extraControl = NULL;
    m_extraControlCreator = NULL;
}

bool wxFileDialogBase::Create(wxWindow *parent,
                              const wxString& message,
                              const wxString& defaultDir,
                              const wxString& defaultFile,
                              const wxString& wildCard,
                              long style,
                              const wxPoint& WXUNUSED(pos),
                              const wxSize& WXUNUSED(sz),
                              const wxString& WXUNUSED(name))
{
    m_message = message;
    m_dir = defaultDir;
    m_fileName = defaultFile;
    m_wildCard = wildCard;

    m_parent = parent;

#ifdef __WXOSX__
    
    style &= ~wxFD_FILE_MUST_EXIST;
#endif

    m_windowStyle = style;
    m_filterIndex = 0;

    if (!HasFdFlag(wxFD_OPEN) && !HasFdFlag(wxFD_SAVE))
        m_windowStyle |= wxFD_OPEN;     
        wxASSERT_MSG( !(HasFdFlag(wxFD_SAVE) && HasFdFlag(wxFD_OPEN)),
                  wxT("can't specify both wxFD_SAVE and wxFD_OPEN at once") );

    wxASSERT_MSG( !HasFdFlag(wxFD_SAVE) ||
                    (!HasFdFlag(wxFD_MULTIPLE) && !HasFdFlag(wxFD_FILE_MUST_EXIST)),
                   wxT("wxFD_MULTIPLE or wxFD_FILE_MUST_EXIST can't be used with wxFD_SAVE" ) );

    wxASSERT_MSG( !HasFdFlag(wxFD_OPEN) || !HasFdFlag(wxFD_OVERWRITE_PROMPT),
                  wxT("wxFD_OVERWRITE_PROMPT can't be used with wxFD_OPEN") );

    if ( wildCard.empty() || wildCard == wxFileSelectorDefaultWildcardStr )
    {
        m_wildCard = wxString::Format(_("All files (%s)|%s"),
                                      wxFileSelectorDefaultWildcardStr,
                                      wxFileSelectorDefaultWildcardStr);
    }
    else     {
                if ( m_wildCard.Find(wxT('|')) == wxNOT_FOUND )
        {
            wxString::size_type nDot = m_wildCard.find(wxT("*."));
            if ( nDot != wxString::npos )
                nDot++;
            else
                nDot = 0;

            m_wildCard = wxString::Format
                         (
                            _("%s files (%s)|%s"),
                            wildCard.c_str() + nDot,
                            wildCard.c_str(),
                            wildCard.c_str()
                         );
        }
    }

    return true;
}

wxString wxFileDialogBase::AppendExtension(const wxString &filePath,
                                           const wxString &extensionList)
{
        wxString fileName = filePath.AfterLast(wxFILE_SEP_PATH);

        int idx_dot = fileName.Find(wxT('.'), true);
    if ((idx_dot != wxNOT_FOUND) && (idx_dot < (int)fileName.length() - 1))
        return filePath;

        wxString ext = extensionList.BeforeFirst(wxT(';'));

        int idx_ext_dot = ext.Find(wxT('.'), true);
    if ((idx_ext_dot == wxNOT_FOUND) || (idx_ext_dot == (int)ext.length() - 1))
        return filePath;
    else
        ext = ext.AfterLast(wxT('.'));

        if ((ext.Find(wxT('*')) != wxNOT_FOUND) ||
        (ext.Find(wxT('?')) != wxNOT_FOUND) ||
        (ext.Strip(wxString::both).empty()))
        return filePath;

        if (filePath.Last() != wxT('.'))
        ext = wxT(".") + ext;

    return filePath + ext;
}

bool wxFileDialogBase::SetExtraControlCreator(ExtraControlCreatorFunction creator)
{
    wxCHECK_MSG( !m_extraControlCreator, false,
                 "wxFileDialog::SetExtraControl() called second time" );

    m_extraControlCreator = creator;
    return SupportsExtraControl();
}

bool wxFileDialogBase::CreateExtraControl()
{
    if (!m_extraControlCreator || m_extraControl)
        return false;
    m_extraControl = (*m_extraControlCreator)(this);
    return true;
}

wxSize wxFileDialogBase::GetExtraControlSize()
{
    if ( !m_extraControlCreator )
        return wxDefaultSize;

                wxDialog dlg(NULL, wxID_ANY, "");
    return (*m_extraControlCreator)(&dlg)->GetSize();
}

void wxFileDialogBase::SetPath(const wxString& path)
{
    wxString ext;
    wxFileName::SplitPath(path, &m_dir, &m_fileName, &ext);
    if ( !ext.empty() )
    {
        SetFilterIndexFromExt(ext);

        m_fileName << wxT('.') << ext;
    }

    m_path = path;
}

void wxFileDialogBase::SetDirectory(const wxString& dir)
{
    m_dir = dir;
    m_path = wxFileName(m_dir, m_fileName).GetFullPath();
}

void wxFileDialogBase::SetFilename(const wxString& name)
{
    m_fileName = name;
    m_path = wxFileName(m_dir, m_fileName).GetFullPath();
}

void wxFileDialogBase::SetFilterIndexFromExt(const wxString& ext)
{
        if ( !ext.empty() && m_wildCard.find(wxT('|')) != wxString::npos )
    {
        int filterIndex = -1;

        wxArrayString descriptions, filters;
                (void)wxParseCommonDialogsFilter(m_wildCard, descriptions, filters);
        for (size_t n=0; n<filters.GetCount(); n++)
        {
            if (filters[n].Contains(ext))
            {
                filterIndex = n;
                break;
            }
        }

        if (filterIndex >= 0)
            SetFilterIndex(filterIndex);
    }
}


wxString wxFileSelector(const wxString& title,
                        const wxString& defaultDir,
                        const wxString& defaultFileName,
                        const wxString& defaultExtension,
                        const wxString& filter,
                        int flags,
                        wxWindow *parent,
                        int x, int y)
{
                            
        
    wxString filter2;
    if ( !defaultExtension.empty() && filter.empty() )
        filter2 = wxString(wxT("*.")) + defaultExtension;
    else if ( !filter.empty() )
        filter2 = filter;

    wxFileDialog fileDialog(parent, title, defaultDir,
                            defaultFileName, filter2,
                            flags, wxPoint(x, y));

    fileDialog.SetFilterIndexFromExt(defaultExtension);

    wxString filename;
    if ( fileDialog.ShowModal() == wxID_OK )
    {
        filename = fileDialog.GetPath();
    }

    return filename;
}


wxString wxFileSelectorEx(const wxString& title,
                          const wxString& defaultDir,
                          const wxString& defaultFileName,
                          int*            defaultFilterIndex,
                          const wxString& filter,
                          int             flags,
                          wxWindow*       parent,
                          int             x,
                          int             y)

{
    wxFileDialog fileDialog(parent,
                            title,
                            defaultDir,
                            defaultFileName,
                            filter,
                            flags, wxPoint(x, y));

    wxString filename;
    if ( fileDialog.ShowModal() == wxID_OK )
    {
        if ( defaultFilterIndex )
            *defaultFilterIndex = fileDialog.GetFilterIndex();

        filename = fileDialog.GetPath();
    }

    return filename;
}


static wxString wxDefaultFileSelector(bool load,
                                      const wxString& what,
                                      const wxString& extension,
                                      const wxString& default_name,
                                      wxWindow *parent)
{
    wxString prompt;
    wxString str;
    if (load)
        str = _("Load %s file");
    else
        str = _("Save %s file");
    prompt.Printf(str, what);

    wxString wild;
    wxString ext;
    if ( !extension.empty() )
    {
        if ( extension[0u] == wxT('.') )
            ext = extension.substr(1);
        else
            ext = extension;

        wild.Printf(wxT("*.%s"), ext);
    }
    else     {
        wild = wxFileSelectorDefaultWildcardStr;
    }

    return wxFileSelector(prompt, wxEmptyString, default_name, ext, wild,
                          load ? (wxFD_OPEN | wxFD_FILE_MUST_EXIST) : wxFD_SAVE,
                          parent);
}


WXDLLEXPORT wxString wxLoadFileSelector(const wxString& what,
                                        const wxString& extension,
                                        const wxString& default_name,
                                        wxWindow *parent)
{
    return wxDefaultFileSelector(true, what, extension, default_name, parent);
}


WXDLLEXPORT wxString wxSaveFileSelector(const wxString& what,
                                        const wxString& extension,
                                        const wxString& default_name,
                                        wxWindow *parent)
{
    return wxDefaultFileSelector(false, what, extension, default_name, parent);
}



#endif 