
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_CONFIG && wxUSE_REGKEY

#include "wx/config.h"

#ifndef WX_PRECOMP
    #include  "wx/string.h"
    #include  "wx/intl.h"
    #include "wx/log.h"
    #include "wx/event.h"
    #include "wx/app.h"
#endif 
#include "wx/msw/registry.h"
#include "wx/msw/regconf.h"


#define SOFTWARE_KEY    wxString(wxT("Software\\"))


bool TryGetValue(const wxRegKey& key, const wxString& str, wxString& strVal)
{
  return key.IsOpened() && key.HasValue(str) && key.QueryValue(str, strVal);
}

bool TryGetValue(const wxRegKey& key, const wxString& str, long *plVal)
{
  return key.IsOpened() && key.HasValue(str) && key.QueryValue(str, plVal);
}

bool TryGetValue(const wxRegKey& key, const wxString& str, wxMemoryBuffer &plVal)
{
  return key.IsOpened() && key.HasValue(str) && key.QueryValue(str, plVal);
}


wxIMPLEMENT_ABSTRACT_CLASS(wxRegConfig, wxConfigBase);

wxRegConfig::wxRegConfig(const wxString& appName, const wxString& vendorName,
                         const wxString& strLocal, const wxString& strGlobal,
                         long style)
           : wxConfigBase(appName, vendorName, strLocal, strGlobal, style)
{
  wxString strRoot;

  bool bDoUseGlobal = (style & wxCONFIG_USE_GLOBAL_FILE) != 0;

        if ( strLocal.empty() || (strGlobal.empty() && bDoUseGlobal) )
  {
    if ( vendorName.empty() )
    {
      if ( wxTheApp )
        strRoot = wxTheApp->GetVendorName();
    }
    else
    {
      strRoot = vendorName;
    }

        if ( !strRoot.empty() )
    {
      strRoot += '\\';
    }

    if ( appName.empty() )
    {
      wxCHECK_RET( wxTheApp, wxT("No application name in wxRegConfig ctor!") );
      strRoot << wxTheApp->GetAppName();
    }
    else
    {
      strRoot << appName;
    }
  }
  
  wxString str = strLocal.empty() ? strRoot : strLocal;

          static const size_t MEMORY_PREALLOC = 512;

  m_keyLocalRoot.ReserveMemoryForName(MEMORY_PREALLOC);
  m_keyLocal.ReserveMemoryForName(MEMORY_PREALLOC);

  m_keyLocalRoot.SetName(wxRegKey::HKCU, SOFTWARE_KEY + str);
  m_keyLocal.SetName(m_keyLocalRoot, wxEmptyString);

  if ( bDoUseGlobal )
  {
    str = strGlobal.empty() ? strRoot : strGlobal;

    m_keyGlobalRoot.ReserveMemoryForName(MEMORY_PREALLOC);
    m_keyGlobal.ReserveMemoryForName(MEMORY_PREALLOC);

    m_keyGlobalRoot.SetName(wxRegKey::HKLM, SOFTWARE_KEY + str);
    m_keyGlobal.SetName(m_keyGlobalRoot, wxEmptyString);
  }

    m_keyLocalRoot.Create();

    m_keyLocal.Open();

      if ( bDoUseGlobal )
  {
    wxLogNull nolog;
    m_keyGlobalRoot.Open(wxRegKey::Read);
    m_keyGlobal.Open(wxRegKey::Read);
  }
}


void wxRegConfig::SetPath(const wxString& strPath)
{
        wxString strOldPath = m_strPath;

#ifdef WX_DEBUG_SET_PATH     wxString m_strPathAlt;

    {
        wxArrayString aParts;

                        if ( strPath.empty() || (strPath[0] == wxCONFIG_PATH_SEPARATOR) ) {
                        wxSplitPath(aParts, strPath);
        }
        else {
                        wxString strFullPath = GetPath();
            strFullPath << wxCONFIG_PATH_SEPARATOR << strPath;
            wxSplitPath(aParts, strFullPath);
        }

                wxString strRegPath;
        m_strPathAlt.Empty();
        for ( size_t n = 0; n < aParts.Count(); n++ ) {
            strRegPath << '\\' << aParts[n];
            m_strPathAlt << wxCONFIG_PATH_SEPARATOR << aParts[n];
        }
    }
#endif 
        if ( strPath.empty() )
    {
        m_strPath = wxCONFIG_PATH_SEPARATOR;
    }
    else     {
                wxString strFullPath;
        if ( strPath[0u] == wxCONFIG_PATH_SEPARATOR )
        {
                        strFullPath = strPath;
        }
        else         {
            strFullPath.reserve(2*m_strPath.length());

            strFullPath << m_strPath;
            if ( strFullPath.Len() == 0 ||
                 strFullPath.Last() != wxCONFIG_PATH_SEPARATOR )
                strFullPath << wxCONFIG_PATH_SEPARATOR;
            strFullPath << strPath;
        }

        
                size_t totalSlashes = 0;

                                                        int posLastSlash = -1;

        const wxChar *src = strFullPath.c_str();
        size_t len = strFullPath.length();
        const wxChar *end = src + len;

        wxStringBufferLength buf(m_strPath, len);
        wxChar *dst = buf;
        wxChar *start = dst;

        for ( ; src < end; src++, dst++ )
        {
            if ( *src == wxCONFIG_PATH_SEPARATOR )
            {
                
                                                if ( src[1] == wxT('.') && src[2] == wxT('.') &&
                     (src + 3 == end || src[3] == wxCONFIG_PATH_SEPARATOR) )
                {
                    if ( !totalSlashes )
                    {
                        wxLogWarning(_("'%s' has extra '..', ignored."),
                                     strFullPath.c_str());
                    }
                    else                     {
                                                if ( posLastSlash == -1 )
                        {
                                                                                    
                                                        dst--;
                            while ( *dst != wxCONFIG_PATH_SEPARATOR )
                            {
                                dst--;
                            }
                        }
                        else                         {
                                                        dst = start + posLastSlash;

                                                        posLastSlash = -1;
                        }

                                                wxASSERT_MSG( *dst == wxCONFIG_PATH_SEPARATOR,
                                      wxT("error in wxRegConfig::SetPath") );

                                                dst--;

                                                totalSlashes--;
                    }

                                        src += 2;
                }
                else                 {
                    if ( (dst == start) || (dst[-1] != wxCONFIG_PATH_SEPARATOR) )
                    {
                        *dst = wxCONFIG_PATH_SEPARATOR;

                        posLastSlash = dst - start;

                        totalSlashes++;
                    }
                    else                     {
                                                                        dst--;
                    }
                }
            }
            else             {
                                *dst = *src;
            }
        }

                if ( dst[-1] == wxCONFIG_PATH_SEPARATOR && (dst != start + 1) )
        {
                                    dst--;
        }

        *dst = wxT('\0');
        buf.SetLength(dst - start);
    }

#ifdef WX_DEBUG_SET_PATH
    wxASSERT( m_strPath == m_strPathAlt );
#endif

    if ( m_strPath == strOldPath )
        return;

        wxString strRegPath;
    if ( !m_strPath.empty() )
    {
        size_t len = m_strPath.length();

        const wxChar *src = m_strPath.c_str();
        wxStringBufferLength buf(strRegPath, len);
        wxChar *dst = buf;

        const wxChar *end = src + len;
        for ( ; src < end; src++, dst++ )
        {
            if ( *src == wxCONFIG_PATH_SEPARATOR )
                *dst = wxT('\\');
            else
                *dst = *src;
        }

        buf.SetLength(len);
    }

            #if 0
                    if ( m_keyLocal.Exists() && LocalKey().IsEmpty() )
    {
        m_keyLocal.DeleteSelf();
    }
#endif 
        m_keyLocal.SetName(m_keyLocalRoot, strRegPath);

    if ( GetStyle() & wxCONFIG_USE_GLOBAL_FILE )
    {
      m_keyGlobal.SetName(m_keyGlobalRoot, strRegPath);

      wxLogNull nolog;
      m_keyGlobal.Open(wxRegKey::Read);
    }
}




#define LOCAL_MASK        0x8000
#define IS_LOCAL_INDEX(l) (((l) & LOCAL_MASK) != 0)

bool wxRegConfig::GetFirstGroup(wxString& str, long& lIndex) const
{
  lIndex = 0;
  return GetNextGroup(str, lIndex);
}

bool wxRegConfig::GetNextGroup(wxString& str, long& lIndex) const
{
    if ( m_keyGlobal.IsOpened() && !IS_LOCAL_INDEX(lIndex) ) {
        while ( m_keyGlobal.GetNextKey(str, lIndex) ) {
      if ( !m_keyLocal.Exists() || !LocalKey().HasSubKey(str) ) {
                return true;
      }
    }

        lIndex |= LOCAL_MASK;
  }

    if ( !m_keyLocal.Exists() )
      return false;

      lIndex &= ~LOCAL_MASK;
  bool bOk = LocalKey().GetNextKey(str, lIndex);
  lIndex |= LOCAL_MASK;

  return bOk;
}

bool wxRegConfig::GetFirstEntry(wxString& str, long& lIndex) const
{
  lIndex = 0;
  return GetNextEntry(str, lIndex);
}

bool wxRegConfig::GetNextEntry(wxString& str, long& lIndex) const
{
    if ( m_keyGlobal.IsOpened() && !IS_LOCAL_INDEX(lIndex) ) {
        while ( m_keyGlobal.GetNextValue(str, lIndex) ) {
      if ( !m_keyLocal.Exists() || !LocalKey().HasValue(str) ) {
                return true;
      }
    }

        lIndex |= LOCAL_MASK;
  }

    if ( !m_keyLocal.Exists() )
      return false;

      lIndex &= ~LOCAL_MASK;
  bool bOk = LocalKey().GetNextValue(str, lIndex);
  lIndex |= LOCAL_MASK;

  return bOk;
}

size_t wxRegConfig::GetNumberOfEntries(bool WXUNUSED(bRecursive)) const
{
  size_t nEntries = 0;

    wxString str;
  long l;
  bool bCont = ((wxRegConfig*)this)->GetFirstEntry(str, l);
  while ( bCont ) {
    nEntries++;

    bCont = ((wxRegConfig*)this)->GetNextEntry(str, l);
  }

  return nEntries;
}

size_t wxRegConfig::GetNumberOfGroups(bool WXUNUSED(bRecursive)) const
{
  size_t nGroups = 0;

    wxString str;
  long l;
  bool bCont = ((wxRegConfig*)this)->GetFirstGroup(str, l);
  while ( bCont ) {
    nGroups++;

    bCont = ((wxRegConfig*)this)->GetNextGroup(str, l);
  }

  return nGroups;
}


bool wxRegConfig::HasGroup(const wxString& key) const
{
    wxConfigPathChanger path(this, key);

    wxString strName(path.Name());

    return (m_keyLocal.Exists() && LocalKey().HasSubKey(strName)) ||
           m_keyGlobal.HasSubKey(strName);
}

bool wxRegConfig::HasEntry(const wxString& key) const
{
    wxConfigPathChanger path(this, key);

    wxString strName(path.Name());

    return (m_keyLocal.Exists() && LocalKey().HasValue(strName)) ||
           m_keyGlobal.HasValue(strName);
}

wxConfigBase::EntryType wxRegConfig::GetEntryType(const wxString& key) const
{
    wxConfigPathChanger path(this, key);

    wxString strName(path.Name());

    bool isNumeric;
    if ( m_keyLocal.Exists() && LocalKey().HasValue(strName) )
        isNumeric = m_keyLocal.IsNumericValue(strName);
    else if ( m_keyGlobal.HasValue(strName) )
        isNumeric = m_keyGlobal.IsNumericValue(strName);
    else
        return wxConfigBase::Type_Unknown;

    return isNumeric ? wxConfigBase::Type_Integer : wxConfigBase::Type_String;
}


bool wxRegConfig::DoReadString(const wxString& key, wxString *pStr) const
{
    wxCHECK_MSG( pStr, false, wxT("wxRegConfig::Read(): NULL param") );

  wxConfigPathChanger path(this, key);

  bool bQueryGlobal = true;

      if ( IsImmutable(path.Name()) ) {
    if ( TryGetValue(m_keyGlobal, path.Name(), *pStr) ) {
      if ( m_keyLocal.Exists() && LocalKey().HasValue(path.Name()) ) {
        wxLogWarning(wxT("User value for immutable key '%s' ignored."),
                   path.Name().c_str());
      }

      return true;
    }
    else {
            bQueryGlobal = false;
    }
  }

    if ( (m_keyLocal.Exists() && TryGetValue(LocalKey(), path.Name(), *pStr)) ||
       (bQueryGlobal && TryGetValue(m_keyGlobal, path.Name(), *pStr)) ) {
    return true;
  }

  return false;
}


bool wxRegConfig::DoReadLong(const wxString& key, long *plResult) const
{
    wxCHECK_MSG( plResult, false, wxT("wxRegConfig::Read(): NULL param") );

  wxConfigPathChanger path(this, key);

  bool bQueryGlobal = true;

      if ( IsImmutable(path.Name()) ) {
    if ( TryGetValue(m_keyGlobal, path.Name(), plResult) ) {
      if ( m_keyLocal.Exists() && LocalKey().HasValue(path.Name()) ) {
        wxLogWarning(wxT("User value for immutable key '%s' ignored."),
                     path.Name().c_str());
      }

      return true;
    }
    else {
            bQueryGlobal = false;
    }
  }

    if ( (m_keyLocal.Exists() && TryGetValue(LocalKey(), path.Name(), plResult)) ||
       (bQueryGlobal && TryGetValue(m_keyGlobal, path.Name(), plResult)) ) {
    return true;
  }

  return false;
}

bool wxRegConfig::DoReadBinary(const wxString& key, wxMemoryBuffer *buf) const
{
    wxCHECK_MSG( buf, false, wxT("wxRegConfig::Read(): NULL param") );

  wxConfigPathChanger path(this, key);

  bool bQueryGlobal = true;

      if ( IsImmutable(path.Name()) ) {
    if ( TryGetValue(m_keyGlobal, path.Name(), *buf) ) {
      if ( m_keyLocal.Exists() && LocalKey().HasValue(path.Name()) ) {
        wxLogWarning(wxT("User value for immutable key '%s' ignored."),
                   path.Name().c_str());
      }

      return true;
    }
    else {
            bQueryGlobal = false;
    }
  }

    if ( (m_keyLocal.Exists() && TryGetValue(LocalKey(), path.Name(), *buf)) ||
       (bQueryGlobal && TryGetValue(m_keyGlobal, path.Name(), *buf)) ) {
    return true;
  }

  return false;
}

bool wxRegConfig::DoWriteString(const wxString& key, const wxString& szValue)
{
  wxConfigPathChanger path(this, key);

  if ( IsImmutable(path.Name()) ) {
    wxLogError(wxT("Can't change immutable entry '%s'."), path.Name().c_str());
    return false;
  }

  return LocalKey().SetValue(path.Name(), szValue);
}

bool wxRegConfig::DoWriteLong(const wxString& key, long lValue)
{
  wxConfigPathChanger path(this, key);

  if ( IsImmutable(path.Name()) ) {
    wxLogError(wxT("Can't change immutable entry '%s'."), path.Name().c_str());
    return false;
  }

  return LocalKey().SetValue(path.Name(), lValue);
}

bool wxRegConfig::DoWriteBinary(const wxString& key, const wxMemoryBuffer& buf)
{
  wxConfigPathChanger path(this, key);

  if ( IsImmutable(path.Name()) ) {
    wxLogError(wxT("Can't change immutable entry '%s'."), path.Name().c_str());
    return false;
  }

  return LocalKey().SetValue(path.Name(), buf);
}


bool wxRegConfig::RenameEntry(const wxString& oldName, const wxString& newName)
{
        if ( !HasEntry(oldName) )
        return false;

        if ( HasEntry(newName) )
        return false;

    return m_keyLocal.RenameValue(oldName, newName);
}

bool wxRegConfig::RenameGroup(const wxString& oldName, const wxString& newName)
{
        if ( !HasGroup(oldName) )
        return false;

        if ( HasGroup(newName) )
        return false;

    return wxRegKey(m_keyLocal, oldName).Rename(newName);
}


bool wxRegConfig::DeleteEntry(const wxString& value, bool bGroupIfEmptyAlso)
{
  wxConfigPathChanger path(this, value);

  if ( m_keyLocal.Exists() ) {
    if ( !m_keyLocal.DeleteValue(path.Name()) )
      return false;

    if ( bGroupIfEmptyAlso && m_keyLocal.IsEmpty() ) {
      wxString strKey = GetPath().AfterLast(wxCONFIG_PATH_SEPARATOR);
      SetPath(wxT(".."));        return LocalKey().DeleteKey(strKey);
    }
  }

  return true;
}

bool wxRegConfig::DeleteGroup(const wxString& key)
{
  wxConfigPathChanger path(this, RemoveTrailingSeparator(key));

  if ( !m_keyLocal.Exists() )
  {
            return true;
  }

  if ( !LocalKey().DeleteKey(path.Name()) )
      return false;

  path.UpdateIfDeleted();

  return true;
}

bool wxRegConfig::DeleteAll()
{
  m_keyLocal.Close();
  m_keyGlobal.Close();

  bool bOk = m_keyLocalRoot.DeleteSelf();

      if ( bOk && m_keyGlobalRoot.IsOpened() )
    bOk = m_keyGlobalRoot.DeleteSelf();

  return bOk;
}

#endif 