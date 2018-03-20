


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_STDPATHS

#include "wx/stdpaths.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/wxcrt.h"
    #include "wx/utils.h"
#endif 
#include "wx/filename.h"
#include "wx/log.h"
#include "wx/textfile.h"

#if defined( __LINUX__ ) || defined( __VMS )
    #include <unistd.h>
#endif


void wxStandardPaths::SetInstallPrefix(const wxString& prefix)
{
    m_prefix = prefix;
}

wxString wxStandardPaths::GetUserConfigDir() const
{
    return wxFileName::GetHomeDir();
}



#ifdef __VMS

wxString wxStandardPaths::GetInstallPrefix() const
{
    if ( m_prefix.empty() )
    {
        const_cast<wxStandardPaths *>(this)->m_prefix = wxT("/sys$system");
    }

    return m_prefix;
}

wxString wxStandardPaths::GetConfigDir() const
{
   return wxT("/sys$manager");
}

wxString wxStandardPaths::GetDataDir() const
{
   return AppendAppInfo(GetInstallPrefix() + wxT("/sys$share"));
}

wxString wxStandardPaths::GetLocalDataDir() const
{
   return AppendAppInfo(wxT("/sys$manager"));
}

wxString wxStandardPaths::GetUserDataDir() const
{
   return wxFileName::GetHomeDir();
}

wxString wxStandardPaths::GetPluginsDir() const
{
    return wxString(); }

wxString
wxStandardPaths::GetLocalizedResourcesDir(const wxString& lang,
                                          ResourceCat category) const
{
    return wxStandardPathsBase::GetLocalizedResourcesDir(lang, category);
}

wxString wxStandardPaths::GetExecutablePath() const
{
    return wxStandardPathsBase::GetExecutablePath();
}

#else 

wxString wxStandardPaths::GetExecutablePath() const
{
#ifdef __LINUX__
    wxString exeStr;

    char buf[4096];
    int result = readlink("/proc/self/exe", buf, WXSIZEOF(buf) - 1);
    if ( result != -1 )
    {
        buf[result] = '\0'; 
                                if ( strcmp(buf, "(deleted)") != 0 )
            exeStr = wxString(buf, wxConvLibc);
    }

    if ( exeStr.empty() )
    {
                                                        wxGetEnv(wxT("   "), &exeStr);
    }

    if ( !exeStr.empty() )
        return exeStr;
#endif 
    return wxStandardPathsBase::GetExecutablePath();
}

void wxStandardPaths::DetectPrefix()
{
        wxString exeStr = GetExecutablePath();
    if ( !exeStr.empty() )
    {
                size_t pos = exeStr.rfind(wxT("/bin/"));
        if ( pos != wxString::npos )
            m_prefix.assign(exeStr, 0, pos);
    }

    if ( m_prefix.empty() )
    {
        m_prefix = wxT("/usr/local");
    }
}

wxString wxStandardPaths::GetInstallPrefix() const
{
    if ( m_prefix.empty() )
    {
        wxStandardPaths *pathPtr = const_cast<wxStandardPaths *>(this);
        pathPtr->DetectPrefix();
    }

    return m_prefix;
}


wxString wxStandardPaths::GetConfigDir() const
{
   return wxT("/etc");
}

wxString wxStandardPaths::GetDataDir() const
{
                    static const wxString
      envOverride(
        getenv(
            ("WX_" + wxTheApp->GetAppName().Upper() + "_DATA_DIR").c_str()
        )
      );

    if ( !envOverride.empty() )
        return envOverride;

   return AppendAppInfo(GetInstallPrefix() + wxT("/share"));
}

wxString wxStandardPaths::GetLocalDataDir() const
{
   return AppendAppInfo(wxT("/etc"));
}

wxString wxStandardPaths::GetUserDataDir() const
{
   return AppendAppInfo(wxFileName::GetHomeDir() + wxT("/."));
}

wxString wxStandardPaths::GetPluginsDir() const
{
    return AppendAppInfo(GetInstallPrefix() + wxT("/lib"));
}

wxString
wxStandardPaths::GetLocalizedResourcesDir(const wxString& lang,
                                          ResourceCat category) const
{
    if ( category != ResourceCat_Messages )
        return wxStandardPathsBase::GetLocalizedResourcesDir(lang, category);

    return GetInstallPrefix() + wxT("/share/locale/") + lang + wxT("/LC_MESSAGES");
}

wxString wxStandardPaths::GetUserDir(Dir userDir) const
{
    {
        wxLogNull logNull;
        wxString homeDir = wxFileName::GetHomeDir();
        wxString configPath;
        if (wxGetenv(wxT("XDG_CONFIG_HOME")))
            configPath = wxGetenv(wxT("XDG_CONFIG_HOME"));
        else
            configPath = homeDir + wxT("/.config");
        wxString dirsFile = configPath + wxT("/user-dirs.dirs");
        if (wxFileExists(dirsFile))
        {
            wxString userDirId;
            switch (userDir)
            {
                case Dir_Desktop:
                    userDirId = "XDG_DESKTOP_DIR";
                    break;
                case Dir_Downloads:
                    userDirId = "XDG_DOWNLOAD_DIR";
                    break;
                case Dir_Music:
                    userDirId = "XDG_MUSIC_DIR";
                    break;
                case Dir_Pictures:
                    userDirId = "XDG_PICTURES_DIR";
                    break;
                case Dir_Videos:
                    userDirId = "XDG_VIDEOS_DIR";
                    break;
                default:
                    userDirId = "XDG_DOCUMENTS_DIR";
                    break;
            }

            wxTextFile textFile;
            if (textFile.Open(dirsFile))
            {
                size_t i;
                for (i = 0; i < textFile.GetLineCount(); i++)
                {
                    wxString line(textFile[i]);
                    int pos = line.Find(userDirId);
                    if (pos != wxNOT_FOUND)
                    {
                        wxString value = line.AfterFirst(wxT('='));
                        value.Replace(wxT("$HOME"), homeDir);
                        value.Trim(true);
                        value.Trim(false);
                                                value.Replace("\"", "", true );
                        if (!value.IsEmpty() && wxDirExists(value))
                            return value;
                        else
                            break;
                    }
                }
            }
        }
    }

    return wxStandardPathsBase::GetUserDir(userDir);
}

#endif 
#endif 