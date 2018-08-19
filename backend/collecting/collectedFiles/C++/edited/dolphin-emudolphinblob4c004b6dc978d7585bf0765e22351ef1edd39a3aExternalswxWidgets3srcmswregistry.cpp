
#include  "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_REGKEY

#ifndef WX_PRECOMP
    #include "wx/msw/wrapwin.h"
    #include "wx/string.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/crt.h"
    #include "wx/utils.h"
#endif

#include "wx/dynlib.h"
#include "wx/file.h"
#include "wx/wfstream.h"
#include "wx/msw/private.h"

#include  <stdlib.h>      
#ifndef _MAX_PATH
    #define _MAX_PATH 512
#endif

#define   HKEY_DEFINED    #include  "wx/msw/registry.h"

typedef unsigned char *RegString;
typedef BYTE* RegBinary;

#ifndef HKEY_PERFORMANCE_DATA
    #define HKEY_PERFORMANCE_DATA ((HKEY)0x80000004)
#endif

#ifndef HKEY_CURRENT_CONFIG
    #define HKEY_CURRENT_CONFIG ((HKEY)0x80000005)
#endif

#ifndef HKEY_DYN_DATA
    #define HKEY_DYN_DATA ((HKEY)0x80000006)
#endif

#ifndef KEY_WOW64_64KEY
    #define KEY_WOW64_64KEY 0x0100
#endif


static struct
{
  HKEY        hkey;
  const wxChar *szName;
  const wxChar *szShortName;
}
aStdKeys[] =
{
  { HKEY_CLASSES_ROOT,      wxT("HKEY_CLASSES_ROOT"),      wxT("HKCR") },
  { HKEY_CURRENT_USER,      wxT("HKEY_CURRENT_USER"),      wxT("HKCU") },
  { HKEY_LOCAL_MACHINE,     wxT("HKEY_LOCAL_MACHINE"),     wxT("HKLM") },
  { HKEY_USERS,             wxT("HKEY_USERS"),             wxT("HKU")  },   { HKEY_PERFORMANCE_DATA,  wxT("HKEY_PERFORMANCE_DATA"),  wxT("HKPD") },
  { HKEY_CURRENT_CONFIG,    wxT("HKEY_CURRENT_CONFIG"),    wxT("HKCC") },
  { HKEY_DYN_DATA,          wxT("HKEY_DYN_DATA"),          wxT("HKDD") }, };

#define   REG_SEPARATOR     wxT('\\')

#define   RESERVED        (0)


#define CONST_CAST    ((wxRegKey *)this)->

#define m_dwLastError   CONST_CAST m_dwLastError


static inline void RemoveTrailingSeparator(wxString& str);

static bool KeyExists(
    WXHKEY hRootKey,
    const wxString& szKey,
    wxRegKey::WOW64ViewMode viewMode = wxRegKey::WOW64ViewMode_Default);

static long GetMSWViewFlags(wxRegKey::WOW64ViewMode viewMode);

static long
GetMSWAccessFlags(wxRegKey::AccessMode mode, wxRegKey::WOW64ViewMode viewMode);

static wxString GetFullName(const wxRegKey *pKey);
static wxString GetFullName(const wxRegKey *pKey, const wxString& szValue);

static inline const wxChar *RegValueStr(const wxString& szValue);

static wxString GetTypeString(DWORD dwType)
{
#define REG_TYPE_TO_STR(type) case REG_ ## type: return wxS(#type)

    switch ( dwType )
    {
        REG_TYPE_TO_STR(NONE);
        REG_TYPE_TO_STR(SZ);
        REG_TYPE_TO_STR(EXPAND_SZ);
        REG_TYPE_TO_STR(BINARY);
        REG_TYPE_TO_STR(DWORD);
                REG_TYPE_TO_STR(DWORD_BIG_ENDIAN);
        REG_TYPE_TO_STR(LINK);
        REG_TYPE_TO_STR(MULTI_SZ);
        REG_TYPE_TO_STR(RESOURCE_LIST);
        REG_TYPE_TO_STR(FULL_RESOURCE_DESCRIPTOR);
        REG_TYPE_TO_STR(RESOURCE_REQUIREMENTS_LIST);
        REG_TYPE_TO_STR(QWORD);
        
        default:
            return wxString::Format(_("unknown (%lu)"), dwType);
    }
}



const size_t wxRegKey::nStdKeys = WXSIZEOF(aStdKeys);

const wxChar *wxRegKey::GetStdKeyName(size_t key)
{
    wxCHECK_MSG( key < nStdKeys, wxEmptyString, wxT("invalid key in wxRegKey::GetStdKeyName") );

  return aStdKeys[key].szName;
}

const wxChar *wxRegKey::GetStdKeyShortName(size_t key)
{
    wxCHECK( key < nStdKeys, wxEmptyString );

  return aStdKeys[key].szShortName;
}

wxRegKey::StdKey wxRegKey::ExtractKeyName(wxString& strKey)
{
  wxString strRoot = strKey.BeforeFirst(REG_SEPARATOR);

  size_t ui;
  for ( ui = 0; ui < nStdKeys; ui++ ) {
    if ( strRoot.CmpNoCase(aStdKeys[ui].szName) == 0 ||
         strRoot.CmpNoCase(aStdKeys[ui].szShortName) == 0 ) {
      break;
    }
  }

  if ( ui == nStdKeys ) {
    wxFAIL_MSG(wxT("invalid key prefix in wxRegKey::ExtractKeyName."));

    ui = HKCR;
  }
  else {
    strKey = strKey.After(REG_SEPARATOR);
    if ( !strKey.empty() && strKey.Last() == REG_SEPARATOR )
      strKey.Truncate(strKey.Len() - 1);
  }

  return (StdKey)ui;
}

wxRegKey::StdKey wxRegKey::GetStdKeyFromHkey(WXHKEY hkey)
{
  for ( size_t ui = 0; ui < nStdKeys; ui++ ) {
    if ( aStdKeys[ui].hkey == (HKEY)hkey )
      return (StdKey)ui;
  }

  wxFAIL_MSG(wxT("non root hkey passed to wxRegKey::GetStdKeyFromHkey."));

  return HKCR;
}


wxRegKey::wxRegKey(WOW64ViewMode viewMode) : m_viewMode(viewMode)
{
  m_hRootKey = (WXHKEY) aStdKeys[HKCR].hkey;

  Init();
}

wxRegKey::wxRegKey(const wxString& strKey, WOW64ViewMode viewMode)
    : m_strKey(strKey), m_viewMode(viewMode)
{
  m_hRootKey  = (WXHKEY) aStdKeys[ExtractKeyName(m_strKey)].hkey;

  Init();
}

wxRegKey::wxRegKey(StdKey keyParent,
                   const wxString& strKey,
                   WOW64ViewMode viewMode)
    : m_strKey(strKey), m_viewMode(viewMode)
{
  RemoveTrailingSeparator(m_strKey);
  m_hRootKey  = (WXHKEY) aStdKeys[keyParent].hkey;

  Init();
}

wxRegKey::wxRegKey(const wxRegKey& keyParent, const wxString& strKey)
    : m_strKey(keyParent.m_strKey), m_viewMode(keyParent.GetView())
{
    if ( !m_strKey.empty() &&
       (strKey.empty() || strKey[0] != REG_SEPARATOR) ) {
      m_strKey += REG_SEPARATOR;
  }

  m_strKey += strKey;
  RemoveTrailingSeparator(m_strKey);

  m_hRootKey  = keyParent.m_hRootKey;

  Init();
}

wxRegKey::~wxRegKey()
{
  Close();
}


void wxRegKey::SetName(const wxString& strKey)
{
  Close();

  m_strKey = strKey;
  m_hRootKey = (WXHKEY) aStdKeys[ExtractKeyName(m_strKey)].hkey;
}

void wxRegKey::SetName(StdKey keyParent, const wxString& strKey)
{
  Close();

  m_strKey = strKey;
  RemoveTrailingSeparator(m_strKey);
  m_hRootKey = (WXHKEY) aStdKeys[keyParent].hkey;
}

void wxRegKey::SetName(const wxRegKey& keyParent, const wxString& strKey)
{
  Close();

  
            m_strKey.clear();
  m_strKey += keyParent.m_strKey;
  if ( !strKey.empty() && strKey[0] != REG_SEPARATOR )
    m_strKey += REG_SEPARATOR;
  m_strKey += strKey;

  RemoveTrailingSeparator(m_strKey);

  m_hRootKey = keyParent.m_hRootKey;
}

void wxRegKey::SetHkey(WXHKEY hKey)
{
  Close();

  m_hKey = hKey;

    m_hRootKey = HKEY_LOCAL_MACHINE;

        m_mode = Write;

    m_strKey.clear();
  m_dwLastError = 0;
}


bool wxRegKey::Exists() const
{
    return IsOpened()
      ? true
      : KeyExists(m_hRootKey, m_strKey, m_viewMode);
}

wxString wxRegKey::GetName(bool bShortPrefix) const
{
  StdKey key = GetStdKeyFromHkey((WXHKEY) m_hRootKey);
  wxString str = bShortPrefix ? aStdKeys[key].szShortName
                              : aStdKeys[key].szName;
  if ( !m_strKey.empty() )
    str << wxT("\\") << m_strKey;

  return str;
}

bool wxRegKey::GetKeyInfo(size_t *pnSubKeys,
                          size_t *pnMaxKeyLen,
                          size_t *pnValues,
                          size_t *pnMaxValueLen) const
{
    wxASSERT_MSG( IsOpened(), wxT("key should be opened in GetKeyInfo") );

    #ifdef __WIN64__
  DWORD dwSubKeys = 0,
        dwMaxKeyLen = 0,
        dwValues = 0,
        dwMaxValueLen = 0;

  #define REG_PARAM(name) &dw##name
#else   #define REG_PARAM(name)   (LPDWORD)(pn##name)
#endif


  m_dwLastError = ::RegQueryInfoKey
                  (
                    (HKEY) m_hKey,
                    NULL,                                       NULL,                                       RESERVED,
                    REG_PARAM(SubKeys),                         REG_PARAM(MaxKeyLen),                       NULL,                                       REG_PARAM(Values),                          REG_PARAM(MaxValueLen),                     NULL,                                       NULL,                                       NULL                                      );

#ifdef __WIN64__
  if ( pnSubKeys )
    *pnSubKeys = dwSubKeys;
  if ( pnMaxKeyLen )
    *pnMaxKeyLen = dwMaxKeyLen;
  if ( pnValues )
    *pnValues = dwValues;
  if ( pnMaxValueLen )
    *pnMaxValueLen = dwMaxValueLen;
#endif 
#undef REG_PARAM

  if ( m_dwLastError != ERROR_SUCCESS ) {
    wxLogSysError(m_dwLastError, _("Can't get info about registry key '%s'"),
                  GetName().c_str());
    return false;
  }

  return true;
}


bool wxRegKey::Open(AccessMode mode)
{
    if ( IsOpened() )
    {
        if ( mode <= m_mode )
            return true;

                Close();
    }

    HKEY tmpKey;
    m_dwLastError = ::RegOpenKeyEx
                    (
                        (HKEY) m_hRootKey,
                        m_strKey.t_str(),
                        RESERVED,
                        GetMSWAccessFlags(mode, m_viewMode),
                        &tmpKey
                    );

    if ( m_dwLastError != ERROR_SUCCESS )
    {
        wxLogSysError(m_dwLastError, _("Can't open registry key '%s'"),
                      GetName().c_str());
        return false;
    }

    m_hKey = (WXHKEY) tmpKey;
    m_mode = mode;

    return true;
}

bool wxRegKey::Create(bool bOkIfExists)
{
    if ( !bOkIfExists && Exists() )
    return false;

  if ( IsOpened() )
    return true;

  HKEY tmpKey;
  DWORD disposition;
    m_dwLastError = RegCreateKeyEx((HKEY) m_hRootKey, m_strKey.t_str(),
      0,          NULL,       REG_OPTION_NON_VOLATILE,       GetMSWAccessFlags(wxRegKey::Write, m_viewMode),
      NULL,       &tmpKey,
      &disposition);

  if ( m_dwLastError != ERROR_SUCCESS ) {
    wxLogSysError(m_dwLastError, _("Can't create registry key '%s'"),
                  GetName().c_str());
    return false;
  }
  else
  {
    m_hKey = (WXHKEY) tmpKey;
    return true;
  }
}

bool wxRegKey::Close()
{
  if ( IsOpened() ) {
    m_dwLastError = RegCloseKey((HKEY) m_hKey);
    m_hKey = 0;

    if ( m_dwLastError != ERROR_SUCCESS ) {
      wxLogSysError(m_dwLastError, _("Can't close registry key '%s'"),
                    GetName().c_str());

      return false;
    }
  }

  return true;
}

bool
wxRegKey::RenameValue(const wxString& szValueOld, const wxString& szValueNew)
{
    bool ok = true;
    if ( HasValue(szValueNew) ) {
        wxLogError(_("Registry value '%s' already exists."), szValueNew);

        ok = false;
    }

    if ( !ok ||
         !CopyValue(szValueOld, *this, szValueNew) ||
         !DeleteValue(szValueOld) ) {
        wxLogError(_("Failed to rename registry value '%s' to '%s'."),
                   szValueOld, szValueNew);

        return false;
    }

    return true;
}

bool wxRegKey::CopyValue(const wxString& szValue,
                         wxRegKey& keyDst,
                         const wxString& szValueNew)
{
    wxString valueNew(szValueNew);
    if ( valueNew.empty() ) {
                valueNew = szValue;
    }

    switch ( GetValueType(szValue) ) {
        case Type_String:
        case Type_Expand_String:
            {
                wxString strVal;
                return QueryRawValue(szValue, strVal) &&
                       keyDst.SetValue(valueNew, strVal);
            }

        case Type_Dword:
        
            {
                long dwVal;
                return QueryValue(szValue, &dwVal) &&
                       keyDst.SetValue(valueNew, dwVal);
            }

        case Type_Binary:
        {
            wxMemoryBuffer buf;
            return QueryValue(szValue,buf) &&
                   keyDst.SetValue(valueNew,buf);
        }

                                        case Type_None:
        case Type_Dword_big_endian:
        case Type_Link:
        case Type_Multi_String:
        case Type_Resource_list:
        case Type_Full_resource_descriptor:
        case Type_Resource_requirements_list:
        default:
            wxLogError(_("Can't copy values of unsupported type %d."),
                       GetValueType(szValue));
            return false;
    }
}

bool wxRegKey::Rename(const wxString& szNewName)
{
    wxCHECK_MSG( !m_strKey.empty(), false, wxT("registry hives can't be renamed") );

    if ( !Exists() ) {
        wxLogError(_("Registry key '%s' does not exist, cannot rename it."),
                   GetFullName(this));

        return false;
    }

        bool inSameHive = !wxStrchr(szNewName, REG_SEPARATOR);

        wxRegKey keyDst;

    if ( inSameHive ) {
                wxString strKey = m_strKey.BeforeLast(REG_SEPARATOR);
        if ( !strKey.empty() ) {
                        strKey += REG_SEPARATOR;
        }

        strKey += szNewName;

        keyDst.SetName(GetStdKeyFromHkey(m_hRootKey), strKey);
    }
    else {
                keyDst.SetName(szNewName);
    }

    bool ok = keyDst.Create(false );
    if ( !ok ) {
        wxLogError(_("Registry key '%s' already exists."),
                   GetFullName(&keyDst));
    }
    else {
        ok = Copy(keyDst) && DeleteSelf();
    }

    if ( !ok ) {
        wxLogError(_("Failed to rename the registry key '%s' to '%s'."),
                   GetFullName(this), GetFullName(&keyDst));
    }
    else {
        m_hRootKey = keyDst.m_hRootKey;
        m_strKey = keyDst.m_strKey;
    }

    return ok;
}

bool wxRegKey::Copy(const wxString& szNewName)
{
        wxRegKey keyDst(szNewName);
    bool ok = keyDst.Create(false );
    if ( ok ) {
        ok = Copy(keyDst);

                if ( !ok ) {
            (void)keyDst.DeleteSelf();
        }
    }

    return ok;
}

bool wxRegKey::Copy(wxRegKey& keyDst)
{
    bool ok = true;

        wxString strKey;
    long lIndex;
    bool bCont = GetFirstKey(strKey, lIndex);
    while ( ok && bCont ) {
        wxRegKey key(*this, strKey);
        wxString keyName;
        keyName << GetFullName(&keyDst) << REG_SEPARATOR << strKey;
        ok = key.Copy(keyName);

        if ( ok )
            bCont = GetNextKey(strKey, lIndex);
        else
            wxLogError(_("Failed to copy the registry subkey '%s' to '%s'."),
                   GetFullName(&key), keyName.c_str());

    }

        wxString strVal;
    bCont = GetFirstValue(strVal, lIndex);
    while ( ok && bCont ) {
        ok = CopyValue(strVal, keyDst);

        if ( !ok ) {
            wxLogSysError(m_dwLastError,
                          _("Failed to copy registry value '%s'"),
                          strVal.c_str());
        }
        else {
            bCont = GetNextValue(strVal, lIndex);
        }
    }

    if ( !ok ) {
        wxLogError(_("Failed to copy the contents of registry key '%s' to '%s'."),
                   GetFullName(this), GetFullName(&keyDst));
    }

    return ok;
}

bool wxRegKey::DeleteSelf()
{
  {
    wxLogNull nolog;
    if ( !Open() ) {
            return true;
    }
  }

        if ( m_strKey.empty() ||
       ((m_hRootKey != (WXHKEY) aStdKeys[HKCR].hkey) &&
        (m_strKey.Find(REG_SEPARATOR) == wxNOT_FOUND)) ) {
      wxLogError(_("Registry key '%s' is needed for normal system operation,\ndeleting it will leave your system in unusable state:\noperation aborted."),
                 GetFullName(this));

      return false;
  }

      wxArrayString astrSubkeys;

  wxString strKey;
  long lIndex;
  bool bCont = GetFirstKey(strKey, lIndex);
  while ( bCont ) {
    astrSubkeys.Add(strKey);

    bCont = GetNextKey(strKey, lIndex);
  }

  size_t nKeyCount = astrSubkeys.Count();
  for ( size_t nKey = 0; nKey < nKeyCount; nKey++ ) {
    wxRegKey key(*this, astrSubkeys[nKey]);
    if ( !key.DeleteSelf() )
      return false;
  }

    Close();

  #if wxUSE_DYNLIB_CLASS
  wxDynamicLibrary dllAdvapi32(wxT("advapi32"));
    if(dllAdvapi32.HasSymbol(wxT("RegDeleteKeyEx")))
  {
    typedef LONG (WINAPI *RegDeleteKeyEx_t)(HKEY, LPCTSTR, REGSAM, DWORD);
    wxDYNLIB_FUNCTION(RegDeleteKeyEx_t, RegDeleteKeyEx, dllAdvapi32);

    m_dwLastError = (*pfnRegDeleteKeyEx)((HKEY) m_hRootKey, m_strKey.t_str(),
        GetMSWViewFlags(m_viewMode),
        0);      }
  else
#endif   {
    m_dwLastError = RegDeleteKey((HKEY) m_hRootKey, m_strKey.t_str());
  }

  if ( m_dwLastError != ERROR_SUCCESS &&
          m_dwLastError != ERROR_FILE_NOT_FOUND ) {
    wxLogSysError(m_dwLastError, _("Can't delete key '%s'"),
                  GetName().c_str());
    return false;
  }

  return true;
}

bool wxRegKey::DeleteKey(const wxString& szKey)
{
  if ( !Open() )
    return false;

  wxRegKey key(*this, szKey);
  return key.DeleteSelf();
}

bool wxRegKey::DeleteValue(const wxString& szValue)
{
    if ( !Open() )
        return false;

    m_dwLastError = RegDeleteValue((HKEY) m_hKey, RegValueStr(szValue));

        if ( (m_dwLastError != ERROR_SUCCESS) &&
         (m_dwLastError != ERROR_FILE_NOT_FOUND) )
    {
        wxLogSysError(m_dwLastError, _("Can't delete value '%s' from key '%s'"),
                      szValue, GetName().c_str());
        return false;
    }

    return true;
}


bool wxRegKey::HasValue(const wxString& szValue) const
{
        wxLogNull nolog;

    if ( !CONST_CAST Open(Read) )
        return false;

    LONG dwRet = ::RegQueryValueEx((HKEY) m_hKey,
                                   RegValueStr(szValue),
                                   RESERVED,
                                   NULL, NULL, NULL);
    return dwRet == ERROR_SUCCESS;
}

bool wxRegKey::HasValues() const
{
    wxLogNull nolog;

    wxString str;
  long     l;
  return CONST_CAST GetFirstValue(str, l);
}

bool wxRegKey::HasSubkeys() const
{
    wxLogNull nolog;

    wxString str;
  long     l;
  return CONST_CAST GetFirstKey(str, l);
}

bool wxRegKey::HasSubKey(const wxString& szKey) const
{
    wxLogNull nolog;

  if ( !CONST_CAST Open(Read) )
    return false;

  return KeyExists(m_hKey, szKey, m_viewMode);
}

wxRegKey::ValueType wxRegKey::GetValueType(const wxString& szValue) const
{
    if ( ! CONST_CAST Open(Read) )
      return Type_None;

    DWORD dwType;
    m_dwLastError = RegQueryValueEx((HKEY) m_hKey, RegValueStr(szValue), RESERVED,
                                    &dwType, NULL, NULL);
    if ( m_dwLastError != ERROR_SUCCESS ) {
      wxLogSysError(m_dwLastError, _("Can't read value of key '%s'"),
                    GetName().c_str());
      return Type_None;
    }

    return (ValueType)dwType;
}

bool wxRegKey::SetValue(const wxString& szValue, long lValue)
{
  if ( CONST_CAST Open() ) {
    m_dwLastError = RegSetValueEx((HKEY) m_hKey, RegValueStr(szValue),
                                  (DWORD) RESERVED, REG_DWORD,
                                  (RegString)&lValue, sizeof(lValue));
    if ( m_dwLastError == ERROR_SUCCESS )
      return true;
  }

  wxLogSysError(m_dwLastError, _("Can't set value of '%s'"),
                GetFullName(this, szValue));
  return false;
}

bool wxRegKey::QueryValue(const wxString& szValue, long *plValue) const
{
  if ( CONST_CAST Open(Read) ) {
    DWORD dwType, dwSize = sizeof(DWORD);
    RegString pBuf = (RegString)plValue;
    m_dwLastError = RegQueryValueEx((HKEY) m_hKey, RegValueStr(szValue),
                                    RESERVED,
                                    &dwType, pBuf, &dwSize);
    if ( m_dwLastError != ERROR_SUCCESS ) {
      wxLogSysError(m_dwLastError, _("Can't read value of key '%s'"),
                    GetName().c_str());
      return false;
    }

        if ( dwType != REG_DWORD_LITTLE_ENDIAN && dwType != REG_DWORD_BIG_ENDIAN ) {
      wxLogError(_("Registry value \"%s\" is not numeric (but of type %s)"),
                 GetFullName(this, szValue), GetTypeString(dwType));
      return false;
    }

    return true;
  }
  else
    return false;
}

bool wxRegKey::SetValue(const wxString& szValue, const wxMemoryBuffer& buffer)
{
#ifdef __TWIN32__
  wxFAIL_MSG("RegSetValueEx not implemented by TWIN32");
  return false;
#else
  if ( CONST_CAST Open() ) {
    m_dwLastError = RegSetValueEx((HKEY) m_hKey, RegValueStr(szValue),
                                  (DWORD) RESERVED, REG_BINARY,
                                  (RegBinary)buffer.GetData(),buffer.GetDataLen());
    if ( m_dwLastError == ERROR_SUCCESS )
      return true;
  }

  wxLogSysError(m_dwLastError, _("Can't set value of '%s'"),
                GetFullName(this, szValue));
  return false;
#endif
}

bool wxRegKey::QueryValue(const wxString& szValue, wxMemoryBuffer& buffer) const
{
  if ( CONST_CAST Open(Read) ) {
        DWORD dwType, dwSize;
    m_dwLastError = RegQueryValueEx((HKEY) m_hKey, RegValueStr(szValue),
                                    RESERVED,
                                    &dwType, NULL, &dwSize);

    if ( m_dwLastError == ERROR_SUCCESS ) {
        if ( dwType != REG_BINARY ) {
          wxLogError(_("Registry value \"%s\" is not binary (but of type %s)"),
                     GetFullName(this, szValue), GetTypeString(dwType));
          return false;
        }

        if ( dwSize ) {
            const RegBinary pBuf = (RegBinary)buffer.GetWriteBuf(dwSize);
            m_dwLastError = RegQueryValueEx((HKEY) m_hKey,
                                            RegValueStr(szValue),
                                            RESERVED,
                                            &dwType,
                                            pBuf,
                                            &dwSize);
            buffer.UngetWriteBuf(dwSize);
        } else {
            buffer.SetDataLen(0);
        }
    }


    if ( m_dwLastError != ERROR_SUCCESS ) {
      wxLogSysError(m_dwLastError, _("Can't read value of key '%s'"),
                    GetName().c_str());
      return false;
    }
    return true;
  }
  return false;
}



bool wxRegKey::QueryValue(const wxString& szValue,
                          wxString& strValue,
                          bool raw) const
{
    if ( CONST_CAST Open(Read) )
    {

                DWORD dwType=REG_NONE, dwSize=0;
        m_dwLastError = RegQueryValueEx((HKEY) m_hKey,
                                        RegValueStr(szValue),
                                        RESERVED,
                                        &dwType, NULL, &dwSize);
        if ( m_dwLastError == ERROR_SUCCESS )
        {
            if ( dwType != REG_SZ && dwType != REG_EXPAND_SZ )
            {
                wxLogError(_("Registry value \"%s\" is not text (but of type %s)"),
                             GetFullName(this, szValue), GetTypeString(dwType));
                return false;
            }

                        DWORD chars = dwSize / sizeof(wxChar);
            if ( !chars )
            {
                                                strValue.Empty();
            }
            else
            {
                                {
                    wxStringBufferLength strBuf(strValue, chars);
                    m_dwLastError = RegQueryValueEx((HKEY) m_hKey,
                                                    RegValueStr(szValue),
                                                    RESERVED,
                                                    &dwType,
                                                    (RegString)(wxChar*)strBuf,
                                                    &dwSize);

                                                                                if ( strBuf[chars - 1] == '\0' )
                        chars--;

                    strBuf.SetLength(chars);
                }

                                if ( (dwType == REG_EXPAND_SZ) && !raw )
                {
                    DWORD dwExpSize = ::ExpandEnvironmentStrings(strValue.t_str(), NULL, 0);
                    bool ok = dwExpSize != 0;
                    if ( ok )
                    {
                        wxString strExpValue;
                        ok = ::ExpandEnvironmentStrings(strValue.t_str(),
                                                        wxStringBuffer(strExpValue, dwExpSize),
                                                        dwExpSize
                                                        ) != 0;
                        strValue = strExpValue;
                    }

                    if ( !ok )
                    {
                        wxLogLastError(wxT("ExpandEnvironmentStrings"));
                    }
                }
            }

            if ( m_dwLastError == ERROR_SUCCESS )
              return true;
        }
    }

    wxLogSysError(m_dwLastError, _("Can't read value of '%s'"),
                  GetFullName(this, szValue));
    return false;
}

bool wxRegKey::SetValue(const wxString& szValue, const wxString& strValue)
{
  if ( CONST_CAST Open() ) {
      m_dwLastError = RegSetValueEx((HKEY) m_hKey,
                                    RegValueStr(szValue),
                                    (DWORD) RESERVED, REG_SZ,
                                    (RegString)wxMSW_CONV_LPCTSTR(strValue),
                                    (strValue.Len() + 1)*sizeof(wxChar));
      if ( m_dwLastError == ERROR_SUCCESS )
        return true;
  }

  wxLogSysError(m_dwLastError, _("Can't set value of '%s'"),
                GetFullName(this, szValue));
  return false;
}

wxString wxRegKey::QueryDefaultValue() const
{
  wxString str;
  QueryValue(wxEmptyString, str, false);
  return str;
}


bool wxRegKey::GetFirstValue(wxString& strValueName, long& lIndex)
{
  if ( !Open(Read) )
    return false;

  lIndex = 0;
  return GetNextValue(strValueName, lIndex);
}

bool wxRegKey::GetNextValue(wxString& strValueName, long& lIndex) const
{
  wxASSERT( IsOpened() );

    if ( lIndex == -1 )
    return false;

    wxChar  szValueName[1024];                      DWORD dwValueLen = WXSIZEOF(szValueName);

    m_dwLastError = RegEnumValue((HKEY) m_hKey, lIndex++,
                                 szValueName, &dwValueLen,
                                 RESERVED,
                                 NULL,                                             NULL,                                             NULL);           
    if ( m_dwLastError != ERROR_SUCCESS ) {
      if ( m_dwLastError == ERROR_NO_MORE_ITEMS ) {
        m_dwLastError = ERROR_SUCCESS;
        lIndex = -1;
      }
      else {
        wxLogSysError(m_dwLastError, _("Can't enumerate values of key '%s'"),
                      GetName().c_str());
      }

      return false;
    }

    strValueName = szValueName;

  return true;
}

bool wxRegKey::GetFirstKey(wxString& strKeyName, long& lIndex)
{
  if ( !Open(Read) )
    return false;

  lIndex = 0;
  return GetNextKey(strKeyName, lIndex);
}

bool wxRegKey::GetNextKey(wxString& strKeyName, long& lIndex) const
{
  wxASSERT( IsOpened() );

    if ( lIndex == -1 )
    return false;

  wxChar szKeyName[_MAX_PATH + 1];

  m_dwLastError = RegEnumKey((HKEY) m_hKey, lIndex++, szKeyName, WXSIZEOF(szKeyName));

  if ( m_dwLastError != ERROR_SUCCESS ) {
    if ( m_dwLastError == ERROR_NO_MORE_ITEMS ) {
      m_dwLastError = ERROR_SUCCESS;
      lIndex = -1;
    }
    else {
      wxLogSysError(m_dwLastError, _("Can't enumerate subkeys of key '%s'"),
                    GetName().c_str());
    }

    return false;
  }

  strKeyName = szKeyName;
  return true;
}

bool wxRegKey::IsNumericValue(const wxString& szValue) const
{
    ValueType type = GetValueType(szValue);
    switch ( type ) {
        case Type_Dword:
        
        case Type_Dword_big_endian:
            return true;

        default:
            return false;
    }
}


#if wxUSE_STREAMS

static inline bool WriteAsciiChar(wxOutputStream& ostr, char ch)
{
    ostr.PutC(ch);
    return ostr.IsOk();
}

static inline bool WriteAsciiEOL(wxOutputStream& ostr)
{
        return WriteAsciiChar(ostr, '\n');
}

static inline bool WriteAsciiString(wxOutputStream& ostr, const char *p)
{
    return ostr.Write(p, strlen(p)).IsOk();
}

static inline bool WriteAsciiString(wxOutputStream& ostr, const wxString& s)
{
#if wxUSE_UNICODE
    wxCharBuffer name(s.mb_str());
    ostr.Write(name, strlen(name));
#else
    ostr.Write(s.mb_str(), s.length());
#endif

    return ostr.IsOk();
}

#endif 
bool wxRegKey::Export(const wxString& filename) const
{
#if wxUSE_FFILE && wxUSE_STREAMS
    if ( wxFile::Exists(filename) )
    {
        wxLogError(_("Exporting registry key: file \"%s\" already exists and won't be overwritten."),
                   filename.c_str());
        return false;
    }

    wxFFileOutputStream ostr(filename, wxT("w"));

    return ostr.IsOk() && Export(ostr);
#else
    wxUnusedVar(filename);
    return false;
#endif
}

#if wxUSE_STREAMS
bool wxRegKey::Export(wxOutputStream& ostr) const
{
        if ( !WriteAsciiString(ostr, "REGEDIT4\n\n") )
        return false;

    return DoExport(ostr);
}
#endif 
static
wxString
FormatAsHex(const void *data,
            size_t size,
            wxRegKey::ValueType type = wxRegKey::Type_Binary)
{
    wxString value(wxT("hex"));

            if ( type != wxRegKey::Type_Binary )
        value << wxT('(') << type << wxT(')');
    value << wxT(':');

        value.reserve(3*size + 10);
    const char * const p = static_cast<const char *>(data);
    for ( size_t n = 0; n < size; n++ )
    {
                                if ( n )
            value << wxT(',');

        value << wxString::Format(wxT("%02x"), (unsigned char)p[n]);
    }

    return value;
}

static inline
wxString FormatAsHex(const wxString& value, wxRegKey::ValueType type)
{
    return FormatAsHex(value.c_str(), value.length() + 1, type);
}

wxString wxRegKey::FormatValue(const wxString& name) const
{
    wxString rhs;
    const ValueType type = GetValueType(name);
    switch ( type )
    {
        case Type_String:
        case Type_Expand_String:
            {
                wxString value;
                if ( !QueryRawValue(name, value) )
                    break;

                                                rhs.reserve(value.length() + 2);
                rhs = wxT('"');

                                bool useHex = false;
                for ( wxString::const_iterator p = value.begin();
                      p != value.end() && !useHex; ++p )
                {
                    switch ( (*p).GetValue() )
                    {
                        case wxT('\n'):
                                                        useHex = true;
                            break;

                        case wxT('"'):
                        case wxT('\\'):
                                                        rhs += wxT('\\');
                            
                        default:
                            rhs += *p;
                    }
                }

                if ( useHex )
                    rhs = FormatAsHex(value, Type_String);
                else
                    rhs += wxT('"');
            }
            break;

        case Type_Dword:
        
            {
                long value;
                if ( !QueryValue(name, &value) )
                    break;

                rhs.Printf(wxT("dword:%08x"), (unsigned int)value);
            }
            break;

        case Type_Multi_String:
            {
                wxString value;
                if ( !QueryRawValue(name, value) )
                    break;

                rhs = FormatAsHex(value, type);
            }
            break;

        case Type_Binary:
            {
                wxMemoryBuffer buf;
                if ( !QueryValue(name, buf) )
                    break;

                rhs = FormatAsHex(buf.GetData(), buf.GetDataLen());
            }
            break;

                case Type_None:
        case Type_Dword_big_endian:
        case Type_Link:
        case Type_Resource_list:
        case Type_Full_resource_descriptor:
        case Type_Resource_requirements_list:
        default:
            wxLogWarning(_("Can't export value of unsupported type %d."), type);
    }

    return rhs;
}

#if wxUSE_STREAMS

bool wxRegKey::DoExportValue(wxOutputStream& ostr, const wxString& name) const
{
                wxString value = FormatValue(name);
    if ( value.empty() )
    {
        wxLogWarning(_("Ignoring value \"%s\" of the key \"%s\"."),
                     name.c_str(), GetName().c_str());
        return true;
    }

        
        if ( name.empty() )
    {
        if ( !WriteAsciiChar(ostr, '@') )
            return false;
    }
    else     {
        if ( !WriteAsciiChar(ostr, '"') ||
                !WriteAsciiString(ostr, name) ||
                    !WriteAsciiChar(ostr, '"') )
            return false;
    }

    if ( !WriteAsciiChar(ostr, '=') )
        return false;

    return WriteAsciiString(ostr, value) && WriteAsciiEOL(ostr);
}

bool wxRegKey::DoExport(wxOutputStream& ostr) const
{
        if ( !WriteAsciiChar(ostr, '[') )
        return false;

    if ( !WriteAsciiString(ostr, GetName(false )) )
        return false;

    if ( !WriteAsciiChar(ostr, ']') || !WriteAsciiEOL(ostr) )
        return false;

        long dummy;
    wxString name;
    wxRegKey& self = const_cast<wxRegKey&>(*this);
    bool cont = self.GetFirstValue(name, dummy);
    while ( cont )
    {
        if ( !DoExportValue(ostr, name) )
            return false;

        cont = GetNextValue(name, dummy);
    }

        if ( !WriteAsciiEOL(ostr) )
        return false;

        cont = self.GetFirstKey(name, dummy);
    while ( cont )
    {
        wxRegKey subkey(*this, name);
        if ( !subkey.DoExport(ostr) )
            return false;

        cont = GetNextKey(name, dummy);
    }

    return true;
}

#endif 

bool KeyExists(WXHKEY hRootKey,
               const wxString& szKey,
               wxRegKey::WOW64ViewMode viewMode)
{
        if ( szKey.empty() )
        return true;

    HKEY hkeyDummy;
    if ( ::RegOpenKeyEx
         (
            (HKEY)hRootKey,
            szKey.t_str(),
            RESERVED,
                        GetMSWAccessFlags(wxRegKey::Read, viewMode),
            &hkeyDummy
         ) == ERROR_SUCCESS )
    {
        ::RegCloseKey(hkeyDummy);

        return true;
    }

    return false;
}

long GetMSWViewFlags(wxRegKey::WOW64ViewMode viewMode)
{
    long samWOW64ViewMode = 0;

    switch ( viewMode )
    {
        case wxRegKey::WOW64ViewMode_32:
#ifdef __WIN64__                samWOW64ViewMode = KEY_WOW64_32KEY;
#endif             break;

        case wxRegKey::WOW64ViewMode_64:
#ifndef __WIN64__                           if ( wxIsPlatform64Bit() )
                samWOW64ViewMode = KEY_WOW64_64KEY;
#endif             break;

        default:
            wxFAIL_MSG("Unknown registry view.");
            
        case wxRegKey::WOW64ViewMode_Default:
                                    ;
    }

    return samWOW64ViewMode;
}

long GetMSWAccessFlags(wxRegKey::AccessMode mode,
    wxRegKey::WOW64ViewMode viewMode)
{
    long sam = mode == wxRegKey::Read ? KEY_READ : KEY_ALL_ACCESS;

    sam |= GetMSWViewFlags(viewMode);

    return sam;
}

wxString GetFullName(const wxRegKey *pKey, const wxString& szValue)
{
  wxString str(pKey->GetName());
  if ( !szValue.empty() )
    str << wxT("\\") << szValue;

  return str;
}

wxString GetFullName(const wxRegKey *pKey)
{
  return pKey->GetName();
}

inline void RemoveTrailingSeparator(wxString& str)
{
  if ( !str.empty() && str.Last() == REG_SEPARATOR )
    str.Truncate(str.Len() - 1);
}

inline const wxChar *RegValueStr(const wxString& szValue)
{
    return szValue.empty() ? (const wxChar*)NULL : szValue.t_str();
}

#endif 