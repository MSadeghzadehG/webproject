
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_INICONF

#ifndef   WX_PRECOMP
    #include "wx/msw/wrapwin.h"
    #include "wx/dynarray.h"
    #include "wx/string.h"
    #include "wx/intl.h"
    #include "wx/event.h"
    #include "wx/app.h"
    #include "wx/utils.h"
    #include "wx/log.h"
#endif  
#include  "wx/config.h"
#include  "wx/file.h"

#include  "wx/msw/iniconf.h"


#define PATH_SEP_REPLACE  '_'


wxIMPLEMENT_ABSTRACT_CLASS(wxIniConfig, wxConfigBase);

wxIniConfig::wxIniConfig(const wxString& strAppName,
                         const wxString& strVendor,
                         const wxString& localFilename,
                         const wxString& globalFilename,
                         long style)
           : wxConfigBase(strAppName, strVendor, localFilename, globalFilename, style)

#if 0            : wxConfigBase((strAppName.empty() && wxTheApp) ? wxTheApp->GetAppName()
                                               : strAppName,
                          strVendor.empty() ? (wxTheApp ? wxTheApp->GetVendorName()
                                                  : strAppName)
                                      : strVendor,
                          localFilename, globalFilename, style)
#endif
{
    if (strAppName.empty() && wxTheApp)
        SetAppName(wxTheApp->GetAppName());
    if (strVendor.empty() && wxTheApp)
        SetVendorName(wxTheApp->GetVendorName());

    m_strLocalFilename = localFilename;
    if (m_strLocalFilename.empty())
    {
        m_strLocalFilename = GetAppName() + wxT(".ini");
    }

            if ( !wxIsPathSeparator(m_strLocalFilename[0u]) &&
        m_strLocalFilename.Find(wxT('.')) == wxNOT_FOUND )
    {
        m_strLocalFilename << wxT(".ini");
    }

        SetPath(wxEmptyString);
}

wxIniConfig::~wxIniConfig()
{
}


void wxIniConfig::SetPath(const wxString& strPath)
{
  wxArrayString aParts;

  if ( strPath.empty() ) {
      }
  else if ( strPath[0u] == wxCONFIG_PATH_SEPARATOR ) {
        wxSplitPath(aParts, strPath);
  }
  else {
        wxString strFullPath = GetPath();
    strFullPath << wxCONFIG_PATH_SEPARATOR << strPath;
    wxSplitPath(aParts, strFullPath);
  }

  size_t nPartsCount = aParts.Count();
  m_strPath.Empty();
  if ( nPartsCount == 0 ) {
        m_strGroup = PATH_SEP_REPLACE;
  }
  else {
        m_strGroup = aParts[0u];
    for ( size_t nPart = 1; nPart < nPartsCount; nPart++ ) {
      if ( nPart > 1 )
        m_strPath << PATH_SEP_REPLACE;
      m_strPath << aParts[nPart];
    }
  }

      wxASSERT( (m_strPath.empty() || m_strPath.Last() != PATH_SEP_REPLACE) &&
            (m_strGroup == wxString(PATH_SEP_REPLACE) ||
             m_strGroup.Last() != PATH_SEP_REPLACE) );
}

const wxString& wxIniConfig::GetPath() const
{
  static wxString s_str;

    s_str = wxCONFIG_PATH_SEPARATOR;

  if ( m_strGroup == wxString(PATH_SEP_REPLACE) ) {
      }
  else {
    s_str << m_strGroup;
    if ( !m_strPath.empty() )
      s_str << wxCONFIG_PATH_SEPARATOR;
    for ( const wxStringCharType *p = m_strPath.wx_str(); *p != '\0'; p++ ) {
      s_str << (*p == PATH_SEP_REPLACE ? wxCONFIG_PATH_SEPARATOR : *p);
    }
  }

  return s_str;
}

wxString wxIniConfig::GetPrivateKeyName(const wxString& szKey) const
{
  wxString strKey;

  if ( !m_strPath.empty() )
    strKey << m_strPath << PATH_SEP_REPLACE;

  strKey << szKey;

  return strKey;
}

wxString wxIniConfig::GetKeyName(const wxString& szKey) const
{
  wxString strKey;

  if ( m_strGroup != wxString(PATH_SEP_REPLACE) )
    strKey << m_strGroup << PATH_SEP_REPLACE;
  if ( !m_strPath.empty() )
    strKey << m_strPath << PATH_SEP_REPLACE;

  strKey << szKey;

  return strKey;
}


bool wxIniConfig::GetFirstGroup(wxString& WXUNUSED(str), long& WXUNUSED(lIndex)) const
{
    wxFAIL_MSG("not implemented");

    return false;
}

bool wxIniConfig::GetNextGroup (wxString& WXUNUSED(str), long& WXUNUSED(lIndex)) const
{
    wxFAIL_MSG("not implemented");

    return false;
}

bool wxIniConfig::GetFirstEntry(wxString& WXUNUSED(str), long& WXUNUSED(lIndex)) const
{
    wxFAIL_MSG("not implemented");

    return false;
}

bool wxIniConfig::GetNextEntry (wxString& WXUNUSED(str), long& WXUNUSED(lIndex)) const
{
    wxFAIL_MSG("not implemented");

    return false;
}


size_t wxIniConfig::GetNumberOfEntries(bool WXUNUSED(bRecursive)) const
{
    wxFAIL_MSG("not implemented");

    return (size_t)-1;
}

size_t wxIniConfig::GetNumberOfGroups(bool WXUNUSED(bRecursive)) const
{
    wxFAIL_MSG("not implemented");

    return (size_t)-1;
}

bool wxIniConfig::HasGroup(const wxString& WXUNUSED(strName)) const
{
    wxFAIL_MSG("not implemented");

    return false;
}

bool wxIniConfig::HasEntry(const wxString& WXUNUSED(strName)) const
{
    wxFAIL_MSG("not implemented");

    return false;
}

bool wxIniConfig::IsEmpty() const
{
    wxChar szBuf[1024];

    GetPrivateProfileString(m_strGroup.t_str(), NULL, wxT(""),
                            szBuf, WXSIZEOF(szBuf),
                            m_strLocalFilename.t_str());
    if ( !wxIsEmpty(szBuf) )
        return false;

    GetProfileString(m_strGroup.t_str(), NULL, wxT(""), szBuf, WXSIZEOF(szBuf));
    if ( !wxIsEmpty(szBuf) )
        return false;

    return true;
}


bool wxIniConfig::DoReadString(const wxString& szKey, wxString *pstr) const
{
  wxConfigPathChanger path(this, szKey);
  wxString strKey = GetPrivateKeyName(path.Name());

  wxChar szBuf[1024]; 
  
    GetPrivateProfileString(m_strGroup.t_str(), strKey.t_str(), wxT(""),
                          szBuf, WXSIZEOF(szBuf),
                          m_strLocalFilename.t_str());
  if ( wxIsEmpty(szBuf) ) {
        wxString strKey = GetKeyName(path.Name());
    GetProfileString(m_strGroup.t_str(), strKey.t_str(),
                     wxT(""), szBuf, WXSIZEOF(szBuf));
  }

  if ( wxIsEmpty(szBuf) )
    return false;

  *pstr = szBuf;
  return true;
}

bool wxIniConfig::DoReadLong(const wxString& szKey, long *pl) const
{
  wxConfigPathChanger path(this, szKey);
  wxString strKey = GetPrivateKeyName(path.Name());

    
  static const int nMagic  = 17;   static const int nMagic2 = 28;   long lVal = GetPrivateProfileInt(m_strGroup.t_str(), strKey.t_str(),
                                   nMagic, m_strLocalFilename.t_str());
  if ( lVal != nMagic ) {
        *pl = lVal;
    return true;
  }

    lVal = GetPrivateProfileInt(m_strGroup.t_str(), strKey.t_str(),
                              nMagic2, m_strLocalFilename.t_str());
  if ( lVal != nMagic2 ) {
        *pl = lVal;
    return true;
  }

      #if 0
   *pl = GetProfileInt(GetVendorName(), GetKeyName(szKey), *pl);

  return true;
#endif
  return false ;
}

bool wxIniConfig::DoWriteString(const wxString& szKey, const wxString& szValue)
{
  wxConfigPathChanger path(this, szKey);
  wxString strKey = GetPrivateKeyName(path.Name());

  bool bOk = WritePrivateProfileString(m_strGroup.t_str(), strKey.t_str(),
                                       szValue.t_str(),
                                       m_strLocalFilename.t_str()) != 0;

  if ( !bOk )
  {
    wxLogLastError(wxT("WritePrivateProfileString"));
  }

  return bOk;
}

bool wxIniConfig::DoWriteLong(const wxString& szKey, long lValue)
{
  return Write(szKey, wxString::Format(wxT("%ld"), lValue));
}

bool wxIniConfig::DoReadBinary(const wxString& WXUNUSED(key),
                               wxMemoryBuffer * WXUNUSED(buf)) const
{
    wxFAIL_MSG("not implemented");

    return false;
}

bool wxIniConfig::DoWriteBinary(const wxString& WXUNUSED(key),
                                const wxMemoryBuffer& WXUNUSED(buf))
{
    wxFAIL_MSG("not implemented");

    return false;
}

bool wxIniConfig::Flush(bool )
{
    return WritePrivateProfileString(NULL, NULL, NULL,
                                   m_strLocalFilename.t_str()) != 0;
}


bool wxIniConfig::DeleteEntry(const wxString& szKey, bool bGroupIfEmptyAlso)
{
    wxConfigPathChanger path(this, szKey);
  wxString strKey = GetPrivateKeyName(path.Name());

  if (WritePrivateProfileString(m_strGroup.t_str(), strKey.t_str(),
                                NULL, m_strLocalFilename.t_str()) == 0)
    return false;

  if ( !bGroupIfEmptyAlso || !IsEmpty() )
    return true;

    bool bOk = WritePrivateProfileString(m_strGroup.t_str(), NULL,
                                       NULL, m_strLocalFilename.t_str()) != 0;

  if ( !bOk )
  {
    wxLogLastError(wxT("WritePrivateProfileString"));
  }

  return bOk;
}

bool wxIniConfig::DeleteGroup(const wxString& szKey)
{
  wxConfigPathChanger path(this, szKey);

      bool bOk = WritePrivateProfileString(path.Name().t_str(), NULL,
                                       NULL, m_strLocalFilename.t_str()) != 0;

  if ( !bOk )
  {
    wxLogLastError(wxT("WritePrivateProfileString"));
  }

  return bOk;
}

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

bool wxIniConfig::DeleteAll()
{
    WriteProfileString(GetVendorName().t_str(), NULL, NULL);

    wxChar szBuf[MAX_PATH];
  size_t nRc = GetWindowsDirectory(szBuf, WXSIZEOF(szBuf));
  if ( nRc == 0 )
  {
    wxLogLastError(wxT("GetWindowsDirectory"));
  }
  else if ( nRc > WXSIZEOF(szBuf) )
  {
    wxFAIL_MSG(wxT("buffer is too small for Windows directory."));
  }

  wxString strFile = szBuf;
  strFile << '\\' << m_strLocalFilename;

  if ( wxFile::Exists(strFile) && !wxRemoveFile(strFile) ) {
    wxLogSysError(_("Can't delete the INI file '%s'"), strFile.c_str());
    return false;
  }

  return true;
}

bool wxIniConfig::RenameEntry(const wxString& WXUNUSED(oldName),
                              const wxString& WXUNUSED(newName))
{
        return false;
}

bool wxIniConfig::RenameGroup(const wxString& WXUNUSED(oldName),
                              const wxString& WXUNUSED(newName))
{
        return false;
}

#endif 