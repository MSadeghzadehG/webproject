

#include  "wx/wxprec.h"

#ifdef    __BORLANDC__
    #pragma hdrstop
#endif  
#if wxUSE_CONFIG && wxUSE_FILECONFIG

#ifndef   WX_PRECOMP
    #include  "wx/dynarray.h"
    #include  "wx/string.h"
    #include  "wx/intl.h"
    #include  "wx/log.h"
    #include  "wx/app.h"
    #include  "wx/utils.h"        #if wxUSE_STREAMS
        #include  "wx/stream.h"
    #endif #endif  
#include  "wx/file.h"
#include  "wx/textfile.h"
#include  "wx/memtext.h"
#include  "wx/config.h"
#include  "wx/fileconf.h"
#include  "wx/filefn.h"

#include "wx/base64.h"

#include  "wx/stdpaths.h"

#if defined(__WINDOWS__)
    #include "wx/msw/private.h"
#endif  
#include  <stdlib.h>
#include  <ctype.h>


#define FILECONF_TRACE_MASK wxT("fileconf")


static int LINKAGEMODE CompareEntries(wxFileConfigEntry *p1, wxFileConfigEntry *p2);
static int LINKAGEMODE CompareGroups(wxFileConfigGroup *p1, wxFileConfigGroup *p2);

static wxString FilterInValue(const wxString& str);
static wxString FilterOutValue(const wxString& str);

static wxString FilterInEntryName(const wxString& str);
static wxString FilterOutEntryName(const wxString& str);

static wxString GetAppName(const wxString& appname);



#ifdef WXMAKINGDLL_BASE
    WX_DEFINE_SORTED_USER_EXPORTED_ARRAY(wxFileConfigEntry *, ArrayEntries,
                                         WXDLLIMPEXP_BASE);
    WX_DEFINE_SORTED_USER_EXPORTED_ARRAY(wxFileConfigGroup *, ArrayGroups,
                                         WXDLLIMPEXP_BASE);
#else
    WX_DEFINE_SORTED_ARRAY(wxFileConfigEntry *, ArrayEntries);
    WX_DEFINE_SORTED_ARRAY(wxFileConfigGroup *, ArrayGroups);
#endif


class wxFileConfigLineList
{
public:
  void SetNext(wxFileConfigLineList *pNext)  { m_pNext = pNext; }
  void SetPrev(wxFileConfigLineList *pPrev)  { m_pPrev = pPrev; }

    wxFileConfigLineList(const wxString& str,
                       wxFileConfigLineList *pNext = NULL) : m_strLine(str)
    { SetNext(pNext); SetPrev(NULL); }

    wxFileConfigLineList *Next() const { return m_pNext;  }
  wxFileConfigLineList *Prev() const { return m_pPrev;  }

    void SetText(const wxString& str) { m_strLine = str;  }
  const wxString& Text() const { return m_strLine; }

private:
  wxString  m_strLine;                    wxFileConfigLineList *m_pNext,                               *m_pPrev;        
    wxDECLARE_NO_COPY_CLASS(wxFileConfigLineList);
};


class wxFileConfigEntry
{
private:
  wxFileConfigGroup *m_pParent; 
  wxString      m_strName,                      m_strValue;       bool          m_bImmutable:1,                 m_bHasValue:1;  
  int           m_nLine;        
      wxFileConfigLineList *m_pLine;

public:
  wxFileConfigEntry(wxFileConfigGroup *pParent,
                    const wxString& strName, int nLine);

    const wxString& Name()        const { return m_strName;    }
  const wxString& Value()       const { return m_strValue;   }
  wxFileConfigGroup *Group()    const { return m_pParent;    }
  bool            IsImmutable() const { return m_bImmutable; }
  bool            IsLocal()     const { return m_pLine != 0; }
  int             Line()        const { return m_nLine;      }
  wxFileConfigLineList *
                  GetLine()     const { return m_pLine;      }

    void SetValue(const wxString& strValue, bool bUser = true);
  void SetLine(wxFileConfigLineList *pLine);

    wxDECLARE_NO_COPY_CLASS(wxFileConfigEntry);
};


class wxFileConfigGroup
{
private:
  wxFileConfig *m_pConfig;            wxFileConfigGroup  *m_pParent;      ArrayEntries  m_aEntries;           ArrayGroups   m_aSubgroups;         wxString      m_strName;            wxFileConfigLineList *m_pLine;      wxFileConfigEntry *m_pLastEntry;    wxFileConfigGroup *m_pLastGroup;  
    bool DeleteSubgroup(wxFileConfigGroup *pGroup);

    void UpdateGroupAndSubgroupsLines();

public:
    wxFileConfigGroup(wxFileConfigGroup *pParent, const wxString& strName, wxFileConfig *);

    ~wxFileConfigGroup();

    const wxString& Name()    const { return m_strName; }
  wxFileConfigGroup    *Parent()  const { return m_pParent; }
  wxFileConfig   *Config()  const { return m_pConfig; }

  const ArrayEntries& Entries() const { return m_aEntries;   }
  const ArrayGroups&  Groups()  const { return m_aSubgroups; }
  bool  IsEmpty() const { return Entries().IsEmpty() && Groups().IsEmpty(); }

    wxFileConfigGroup *FindSubgroup(const wxString& name) const;
  wxFileConfigEntry *FindEntry   (const wxString& name) const;

    bool DeleteSubgroupByName(const wxString& name);
  bool DeleteEntry(const wxString& name);

    wxFileConfigGroup *AddSubgroup(const wxString& strName);
  wxFileConfigEntry *AddEntry   (const wxString& strName, int nLine = wxNOT_FOUND);

  void SetLine(wxFileConfigLineList *pLine);

    void Rename(const wxString& newName);

    wxString GetFullName() const;

    wxFileConfigLineList *GetGroupLine();                                                   wxFileConfigLineList *GetLastEntryLine();   wxFileConfigLineList *GetLastGroupLine(); 
    void SetLastEntry(wxFileConfigEntry *pEntry);
  void SetLastGroup(wxFileConfigGroup *pGroup)
    { m_pLastGroup = pGroup; }

  wxDECLARE_NO_COPY_CLASS(wxFileConfigGroup);
};



static void AddConfFileExtIfNeeded(wxFileName& fn)
{
    if ( !fn.HasExt() )
    {
#if defined( __WXMAC__ )
        fn.SetName(fn.GetName() + wxT(" Preferences"));
#elif defined( __UNIX__ )
        fn.SetExt(wxT("conf"));
#else           fn.SetExt(wxT("ini"));
#endif      }
}

wxString wxFileConfig::GetGlobalDir()
{
    return wxStandardPaths::Get().GetConfigDir();
}

wxString wxFileConfig::GetLocalDir(int style)
{
    wxUnusedVar(style);

    wxStandardPathsBase& stdp = wxStandardPaths::Get();

            return style & wxCONFIG_USE_SUBDIR ? stdp.GetUserDataDir()
                                       : stdp.GetUserConfigDir();
}

wxFileName wxFileConfig::GetGlobalFile(const wxString& szFile)
{
    wxFileName fn(GetGlobalDir(), szFile);

    AddConfFileExtIfNeeded(fn);

    return fn;
}

wxFileName wxFileConfig::GetLocalFile(const wxString& szFile, int style)
{
    wxFileName fn(GetLocalDir(style), szFile);

#if defined( __UNIX__ ) && !defined( __WXMAC__ )
    if ( !(style & wxCONFIG_USE_SUBDIR) )
    {
                        fn.SetName(wxT('.') + fn.GetName());
    }
    else #endif     {
        AddConfFileExtIfNeeded(fn);
    }

    return fn;
}

wxIMPLEMENT_ABSTRACT_CLASS(wxFileConfig, wxConfigBase);

void wxFileConfig::Init()
{
    m_pCurrentGroup =
    m_pRootGroup    = new wxFileConfigGroup(NULL, wxEmptyString, this);

    m_linesHead =
    m_linesTail = NULL;

    
        if ( m_fnGlobalFile.IsOk() && m_fnGlobalFile.FileExists() )
    {
        wxTextFile fileGlobal(m_fnGlobalFile.GetFullPath());

        if ( fileGlobal.Open(*m_conv) )
        {
            Parse(fileGlobal, false );
            SetRootPath();
        }
        else
        {
            wxLogWarning(_("can't open global configuration file '%s'."), m_fnGlobalFile.GetFullPath().c_str());
        }
    }

        if ( m_fnLocalFile.IsOk() && m_fnLocalFile.FileExists() )
    {
        wxTextFile fileLocal(m_fnLocalFile.GetFullPath());
        if ( fileLocal.Open(*m_conv) )
        {
            Parse(fileLocal, true );
            SetRootPath();
        }
        else
        {
            const wxString path = m_fnLocalFile.GetFullPath();
            wxLogWarning(_("can't open user configuration file '%s'."),
                         path.c_str());

            if ( m_fnLocalFile.FileExists() )
            {
                wxLogWarning(_("Changes won't be saved to avoid overwriting the existing file \"%s\""),
                             path.c_str());
                m_fnLocalFile.Clear();
            }
        }
    }

    m_isDirty = false;
}

wxFileConfig::wxFileConfig(const wxString& appName, const wxString& vendorName,
                           const wxString& strLocal, const wxString& strGlobal,
                           long style,
                           const wxMBConv& conv)
            : wxConfigBase(::GetAppName(appName), vendorName,
                           strLocal, strGlobal,
                           style),
              m_fnLocalFile(strLocal),
              m_fnGlobalFile(strGlobal),
              m_conv(conv.Clone())
{
        if ( !m_fnLocalFile.IsOk() && (style & wxCONFIG_USE_LOCAL_FILE) )
        m_fnLocalFile = GetLocalFile(GetAppName(), style);

    if ( !m_fnGlobalFile.IsOk() && (style & wxCONFIG_USE_GLOBAL_FILE) )
        m_fnGlobalFile = GetGlobalFile(GetAppName());

            if ( m_fnLocalFile.IsOk() )
        SetStyle(GetStyle() | wxCONFIG_USE_LOCAL_FILE);

    if ( m_fnGlobalFile.IsOk() )
        SetStyle(GetStyle() | wxCONFIG_USE_GLOBAL_FILE);

            if ( !(style & wxCONFIG_USE_RELATIVE_PATH) )
    {
        if ( m_fnLocalFile.IsOk() )
            m_fnLocalFile.MakeAbsolute(GetLocalDir(style));

        if ( m_fnGlobalFile.IsOk() )
            m_fnGlobalFile.MakeAbsolute(GetGlobalDir());
    }

    SetUmask(-1);

    Init();
}

#if wxUSE_STREAMS

wxFileConfig::wxFileConfig(wxInputStream &inStream, const wxMBConv& conv)
            : m_conv(conv.Clone())
{
        SetStyle(GetStyle() | wxCONFIG_USE_LOCAL_FILE);

    m_pCurrentGroup =
    m_pRootGroup    = new wxFileConfigGroup(NULL, wxEmptyString, this);

    m_linesHead =
    m_linesTail = NULL;

        wxWxCharBuffer cbuf;
    static const size_t chunkLen = 1024;

    wxMemoryBuffer buf(chunkLen);
    do
    {
        inStream.Read(buf.GetAppendBuf(chunkLen), chunkLen);
        buf.UngetAppendBuf(inStream.LastRead());

        const wxStreamError err = inStream.GetLastError();

        if ( err != wxSTREAM_NO_ERROR && err != wxSTREAM_EOF )
        {
            wxLogError(_("Error reading config options."));
            break;
        }
    }
    while ( !inStream.Eof() );

#if wxUSE_UNICODE
    size_t len;
    cbuf = conv.cMB2WC((char *)buf.GetData(), buf.GetDataLen() + 1, &len);
    if ( !len && buf.GetDataLen() )
    {
        wxLogError(_("Failed to read config options."));
    }
#else         cbuf = wxCharBuffer::CreateNonOwned((char *)buf.GetData(), buf.GetDataLen());
#endif 
        if ( cbuf )
    {
                wxMemoryText memText;
        for ( const wxChar *s = cbuf; ; ++s )
        {
            const wxChar *e = s;
            while ( *e != '\0' && *e != '\n' && *e != '\r' )
                ++e;

                                    if ( e != s )
                memText.AddLine(wxString(s, e));

            if ( *e == '\0' )
                break;

                        if ( *e == '\r' && e[1] == '\n' )
                ++e;

            s = e;
        }

                Parse(memText, true );
    }

    SetRootPath();
    ResetDirty();
}

#endif 
void wxFileConfig::CleanUp()
{
    delete m_pRootGroup;

    wxFileConfigLineList *pCur = m_linesHead;
    while ( pCur != NULL ) {
        wxFileConfigLineList *pNext = pCur->Next();
        delete pCur;
        pCur = pNext;
    }
}

wxFileConfig::~wxFileConfig()
{
    Flush();

    CleanUp();

    delete m_conv;
}


void wxFileConfig::Parse(const wxTextBuffer& buffer, bool bLocal)
{

  size_t nLineCount = buffer.GetLineCount();

  for ( size_t n = 0; n < nLineCount; n++ )
  {
    wxString strLine = buffer[n];
        wxWxCharBuffer buf(strLine.c_str());
    const wxChar *pStart;
    const wxChar *pEnd;

        if ( bLocal )
      LineListAppend(strLine);


        for ( pStart = buf; wxIsspace(*pStart); pStart++ )
      ;

        if ( *pStart == wxT('\0')|| *pStart == wxT(';') || *pStart == wxT('#') )
      continue;

    if ( *pStart == wxT('[') ) {                pEnd = pStart;

      while ( *++pEnd != wxT(']') ) {
        if ( *pEnd == wxT('\\') ) {
                        pEnd++;
        }

        if ( *pEnd == wxT('\n') || *pEnd == wxT('\0') ) {
                        break;
        }
      }

      if ( *pEnd != wxT(']') ) {
        wxLogError(_("file '%s': unexpected character %c at line %d."),
                   buffer.GetName(), *pEnd, n + 1);
        continue;       }

            wxString strGroup;
      pStart++;
      strGroup << wxCONFIG_PATH_SEPARATOR
               << FilterInEntryName(wxString(pStart, pEnd - pStart));

            SetPath(strGroup);

      if ( bLocal )
      {
        if ( m_pCurrentGroup->Parent() )
          m_pCurrentGroup->Parent()->SetLastGroup(m_pCurrentGroup);
        m_pCurrentGroup->SetLine(m_linesTail);
      }

            bool bCont = true;
      while ( *++pEnd != wxT('\0') && bCont ) {
        switch ( *pEnd ) {
          case wxT('#'):
          case wxT(';'):
            bCont = false;
            break;

          case wxT(' '):
          case wxT('\t'):
                        break;

          default:
            wxLogWarning(_("file '%s', line %d: '%s' ignored after group header."),
                         buffer.GetName(), n + 1, pEnd);
            bCont = false;
        }
      }
    }
    else {                              pEnd = pStart;
      while ( *pEnd && *pEnd != wxT('=')  ) {
        if ( *pEnd == wxT('\\') ) {
                              pEnd++;
          if ( !*pEnd ) {
                        break;
          }
        }

        pEnd++;
      }

      wxString strKey(FilterInEntryName(wxString(pStart, pEnd).Trim()));

            while ( wxIsspace(*pEnd) )
        pEnd++;

      if ( *pEnd++ != wxT('=') ) {
        wxLogError(_("file '%s', line %d: '=' expected."),
                   buffer.GetName(), n + 1);
      }
      else {
        wxFileConfigEntry *pEntry = m_pCurrentGroup->FindEntry(strKey);

        if ( pEntry == NULL ) {
                    pEntry = m_pCurrentGroup->AddEntry(strKey, n);
        }
        else {
          if ( bLocal && pEntry->IsImmutable() ) {
                        wxLogWarning(_("file '%s', line %d: value for immutable key '%s' ignored."),
                         buffer.GetName(), n + 1, strKey.c_str());
            continue;
          }
                                                            else if ( !bLocal || pEntry->IsLocal() ) {
            wxLogWarning(_("file '%s', line %d: key '%s' was first found at line %d."),
                         buffer.GetName(), (int)n + 1, strKey.c_str(), pEntry->Line());

          }
        }

        if ( bLocal )
          pEntry->SetLine(m_linesTail);

                while ( wxIsspace(*pEnd) )
          pEnd++;

        wxString value = pEnd;
        if ( !(GetStyle() & wxCONFIG_USE_NO_ESCAPE_CHARACTERS) )
            value = FilterInValue(value);

        pEntry->SetValue(value, false);
      }
    }
  }
}


void wxFileConfig::SetRootPath()
{
    m_strPath.Empty();
    m_pCurrentGroup = m_pRootGroup;
}

bool
wxFileConfig::DoSetPath(const wxString& strPath, bool createMissingComponents)
{
    wxArrayString aParts;

    if ( strPath.empty() ) {
        SetRootPath();
        return true;
    }

    if ( strPath[0] == wxCONFIG_PATH_SEPARATOR ) {
                wxSplitPath(aParts, strPath);
    }
    else {
                wxString strFullPath = m_strPath;
        strFullPath << wxCONFIG_PATH_SEPARATOR << strPath;
        wxSplitPath(aParts, strFullPath);
    }

        size_t n;
    m_pCurrentGroup = m_pRootGroup;
    for ( n = 0; n < aParts.GetCount(); n++ ) {
        wxFileConfigGroup *pNextGroup = m_pCurrentGroup->FindSubgroup(aParts[n]);
        if ( pNextGroup == NULL )
        {
            if ( !createMissingComponents )
                return false;

            pNextGroup = m_pCurrentGroup->AddSubgroup(aParts[n]);
        }

        m_pCurrentGroup = pNextGroup;
    }

        m_strPath.Empty();
    for ( n = 0; n < aParts.GetCount(); n++ ) {
        m_strPath << wxCONFIG_PATH_SEPARATOR << aParts[n];
    }

    return true;
}

void wxFileConfig::SetPath(const wxString& strPath)
{
    DoSetPath(strPath, true );
}

const wxString& wxFileConfig::GetPath() const
{
    return m_strPath;
}


bool wxFileConfig::GetFirstGroup(wxString& str, long& lIndex) const
{
    lIndex = 0;
    return GetNextGroup(str, lIndex);
}

bool wxFileConfig::GetNextGroup (wxString& str, long& lIndex) const
{
    if ( size_t(lIndex) < m_pCurrentGroup->Groups().GetCount() ) {
        str = m_pCurrentGroup->Groups()[(size_t)lIndex++]->Name();
        return true;
    }
    else
        return false;
}

bool wxFileConfig::GetFirstEntry(wxString& str, long& lIndex) const
{
    lIndex = 0;
    return GetNextEntry(str, lIndex);
}

bool wxFileConfig::GetNextEntry (wxString& str, long& lIndex) const
{
    if ( size_t(lIndex) < m_pCurrentGroup->Entries().GetCount() ) {
        str = m_pCurrentGroup->Entries()[(size_t)lIndex++]->Name();
        return true;
    }
    else
        return false;
}

size_t wxFileConfig::GetNumberOfEntries(bool bRecursive) const
{
    size_t n = m_pCurrentGroup->Entries().GetCount();
    if ( bRecursive ) {
        wxFileConfig * const self = const_cast<wxFileConfig *>(this);

        wxFileConfigGroup *pOldCurrentGroup = m_pCurrentGroup;
        size_t nSubgroups = m_pCurrentGroup->Groups().GetCount();
        for ( size_t nGroup = 0; nGroup < nSubgroups; nGroup++ ) {
            self->m_pCurrentGroup = m_pCurrentGroup->Groups()[nGroup];
            n += GetNumberOfEntries(true);
            self->m_pCurrentGroup = pOldCurrentGroup;
        }
    }

    return n;
}

size_t wxFileConfig::GetNumberOfGroups(bool bRecursive) const
{
    size_t n = m_pCurrentGroup->Groups().GetCount();
    if ( bRecursive ) {
        wxFileConfig * const self = const_cast<wxFileConfig *>(this);

        wxFileConfigGroup *pOldCurrentGroup = m_pCurrentGroup;
        size_t nSubgroups = m_pCurrentGroup->Groups().GetCount();
        for ( size_t nGroup = 0; nGroup < nSubgroups; nGroup++ ) {
            self->m_pCurrentGroup = m_pCurrentGroup->Groups()[nGroup];
            n += GetNumberOfGroups(true);
            self->m_pCurrentGroup = pOldCurrentGroup;
        }
    }

    return n;
}


bool wxFileConfig::HasGroup(const wxString& strName) const
{
            if ( strName.empty() )
        return false;

    const wxString pathOld = GetPath();

    wxFileConfig *self = const_cast<wxFileConfig *>(this);
    const bool
        rc = self->DoSetPath(strName, false );

    self->SetPath(pathOld);

    return rc;
}

bool wxFileConfig::HasEntry(const wxString& entry) const
{
        wxString path = entry.BeforeLast(wxCONFIG_PATH_SEPARATOR);

        if ( path.empty() && *entry.c_str() == wxCONFIG_PATH_SEPARATOR )
    {
        path = wxCONFIG_PATH_SEPARATOR;
    }

            wxString pathOld;
    wxFileConfig * const self = const_cast<wxFileConfig *>(this);
    if ( !path.empty() )
    {
        pathOld = GetPath();
        if ( pathOld.empty() )
            pathOld = wxCONFIG_PATH_SEPARATOR;

        if ( !self->DoSetPath(path, false ) )
        {
            return false;
        }
    }

        const bool exists = m_pCurrentGroup->FindEntry(
                            entry.AfterLast(wxCONFIG_PATH_SEPARATOR)) != NULL;

        if ( !pathOld.empty() )
    {
        self->SetPath(pathOld);
    }

    return exists;
}


bool wxFileConfig::DoReadString(const wxString& key, wxString* pStr) const
{
    wxConfigPathChanger path(this, key);

    wxFileConfigEntry *pEntry = m_pCurrentGroup->FindEntry(path.Name());
    if (pEntry == NULL) {
        return false;
    }

    *pStr = pEntry->Value();

    return true;
}

bool wxFileConfig::DoReadLong(const wxString& key, long *pl) const
{
    wxString str;
    if ( !Read(key, &str) )
        return false;

        str.Trim();

    return str.ToLong(pl);
}

#if wxUSE_BASE64

bool wxFileConfig::DoReadBinary(const wxString& key, wxMemoryBuffer* buf) const
{
    wxCHECK_MSG( buf, false, wxT("NULL buffer") );

    wxString str;
    if ( !Read(key, &str) )
        return false;

    *buf = wxBase64Decode(str);
    return true;
}

#endif 
bool wxFileConfig::DoWriteString(const wxString& key, const wxString& szValue)
{
    wxConfigPathChanger     path(this, key);
    wxString                strName = path.Name();

    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("  Writing String '%s' = '%s' to Group '%s'"),
                strName.c_str(),
                szValue.c_str(),
                GetPath().c_str() );

    if ( strName.empty() )
    {
            
        wxASSERT_MSG( szValue.empty(), wxT("can't set value of a group!") );

            
        wxLogTrace( FILECONF_TRACE_MASK,
                    wxT("  Creating group %s"),
                    m_pCurrentGroup->Name().c_str() );

        SetDirty();

                        (void)m_pCurrentGroup->GetGroupLine();
    }
    else
    {
                if ( strName[0u] == wxCONFIG_IMMUTABLE_PREFIX )
        {
            wxLogError( _("Config entry name cannot start with '%c'."),
                        wxCONFIG_IMMUTABLE_PREFIX);
            return false;
        }

        wxFileConfigEntry   *pEntry = m_pCurrentGroup->FindEntry(strName);

        if ( pEntry == 0 )
        {
            wxLogTrace( FILECONF_TRACE_MASK,
                        wxT("  Adding Entry %s"),
                        strName.c_str() );
            pEntry = m_pCurrentGroup->AddEntry(strName);
        }

        wxLogTrace( FILECONF_TRACE_MASK,
                    wxT("  Setting value %s"),
                    szValue.c_str() );
        pEntry->SetValue(szValue);

        SetDirty();
    }

    return true;
}

bool wxFileConfig::DoWriteLong(const wxString& key, long lValue)
{
  return Write(key, wxString::Format(wxT("%ld"), lValue));
}

#if wxUSE_BASE64

bool wxFileConfig::DoWriteBinary(const wxString& key, const wxMemoryBuffer& buf)
{
  return Write(key, wxBase64Encode(buf));
}

#endif 
bool wxFileConfig::Flush(bool )
{
  if ( !IsDirty() || !m_fnLocalFile.GetFullPath() )
    return true;

    wxCHANGE_UMASK(m_umask);

  wxTempFile file(m_fnLocalFile.GetFullPath());

  if ( !file.IsOpened() )
  {
    wxLogError(_("can't open user configuration file."));
    return false;
  }

    wxString filetext;
  filetext.reserve(4096);
  for ( wxFileConfigLineList *p = m_linesHead; p != NULL; p = p->Next() )
  {
    filetext << p->Text() << wxTextFile::GetEOL();
  }

  if ( !file.Write(filetext, *m_conv) )
  {
    wxLogError(_("can't write user configuration file."));
    return false;
  }

  if ( !file.Commit() )
  {
      wxLogError(_("Failed to update user configuration file."));

      return false;
  }

  ResetDirty();

  return true;
}

#if wxUSE_STREAMS

bool wxFileConfig::Save(wxOutputStream& os, const wxMBConv& conv)
{
        for ( wxFileConfigLineList *p = m_linesHead; p != NULL; p = p->Next() )
    {
        wxString line = p->Text();
        line += wxTextFile::GetEOL();

        wxCharBuffer buf(line.mb_str(conv));
        if ( !os.Write(buf, strlen(buf)) )
        {
            wxLogError(_("Error saving user configuration data."));

            return false;
        }
    }

    ResetDirty();

    return true;
}

#endif 

bool wxFileConfig::RenameEntry(const wxString& oldName,
                               const wxString& newName)
{
    wxASSERT_MSG( oldName.find(wxCONFIG_PATH_SEPARATOR) == wxString::npos,
                   wxT("RenameEntry(): paths are not supported") );

        wxFileConfigEntry *oldEntry = m_pCurrentGroup->FindEntry(oldName);
    if ( !oldEntry )
        return false;

        if ( m_pCurrentGroup->FindEntry(newName) )
        return false;

        wxString value = oldEntry->Value();
    if ( !m_pCurrentGroup->DeleteEntry(oldName) )
        return false;

    SetDirty();

    wxFileConfigEntry *newEntry = m_pCurrentGroup->AddEntry(newName);
    newEntry->SetValue(value);

    return true;
}

bool wxFileConfig::RenameGroup(const wxString& oldName,
                               const wxString& newName)
{
        wxFileConfigGroup *group = m_pCurrentGroup->FindSubgroup(oldName);
    if ( !group )
        return false;

        if ( m_pCurrentGroup->FindSubgroup(newName) )
        return false;

    group->Rename(newName);

    SetDirty();

    return true;
}


bool wxFileConfig::DeleteEntry(const wxString& key, bool bGroupIfEmptyAlso)
{
  wxConfigPathChanger path(this, key);

  if ( !m_pCurrentGroup->DeleteEntry(path.Name()) )
    return false;

  SetDirty();

  if ( bGroupIfEmptyAlso && m_pCurrentGroup->IsEmpty() ) {
    if ( m_pCurrentGroup != m_pRootGroup ) {
      wxFileConfigGroup *pGroup = m_pCurrentGroup;
      SetPath(wxT(".."));        m_pCurrentGroup->DeleteSubgroupByName(pGroup->Name());
    }
      }

  return true;
}

bool wxFileConfig::DeleteGroup(const wxString& key)
{
  wxConfigPathChanger path(this, RemoveTrailingSeparator(key));

  if ( !m_pCurrentGroup->DeleteSubgroupByName(path.Name()) )
      return false;

  path.UpdateIfDeleted();

  SetDirty();

  return true;
}

bool wxFileConfig::DeleteAll()
{
  CleanUp();

  if ( m_fnLocalFile.IsOk() )
  {
      if ( m_fnLocalFile.FileExists() &&
           !wxRemoveFile(m_fnLocalFile.GetFullPath()) )
      {
          wxLogSysError(_("can't delete user configuration file '%s'"),
                        m_fnLocalFile.GetFullPath().c_str());
          return false;
      }
  }

  Init();

  return true;
}


    
wxFileConfigLineList *wxFileConfig::LineListAppend(const wxString& str)
{
    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("    ** Adding Line '%s'"),
                str.c_str() );
    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("        head: %s"),
                ((m_linesHead) ? (const wxChar*)m_linesHead->Text().c_str()
                               : wxEmptyString) );
    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("        tail: %s"),
                ((m_linesTail) ? (const wxChar*)m_linesTail->Text().c_str()
                               : wxEmptyString) );

    wxFileConfigLineList *pLine = new wxFileConfigLineList(str);

    if ( m_linesTail == NULL )
    {
                m_linesHead = pLine;
    }
    else
    {
                m_linesTail->SetNext(pLine);
        pLine->SetPrev(m_linesTail);
    }

    m_linesTail = pLine;

    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("        head: %s"),
                ((m_linesHead) ? (const wxChar*)m_linesHead->Text().c_str()
                               : wxEmptyString) );
    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("        tail: %s"),
                ((m_linesTail) ? (const wxChar*)m_linesTail->Text().c_str()
                               : wxEmptyString) );

    return m_linesTail;
}

wxFileConfigLineList *wxFileConfig::LineListInsert(const wxString& str,
                                                   wxFileConfigLineList *pLine)
{
    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("    ** Inserting Line '%s' after '%s'"),
                str.c_str(),
                ((pLine) ? (const wxChar*)pLine->Text().c_str()
                         : wxEmptyString) );
    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("        head: %s"),
                ((m_linesHead) ? (const wxChar*)m_linesHead->Text().c_str()
                               : wxEmptyString) );
    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("        tail: %s"),
                ((m_linesTail) ? (const wxChar*)m_linesTail->Text().c_str()
                               : wxEmptyString) );

    if ( pLine == m_linesTail )
        return LineListAppend(str);

    wxFileConfigLineList *pNewLine = new wxFileConfigLineList(str);
    if ( pLine == NULL )
    {
                pNewLine->SetNext(m_linesHead);
        m_linesHead->SetPrev(pNewLine);
        m_linesHead = pNewLine;
    }
    else
    {
                wxFileConfigLineList *pNext = pLine->Next();
        pNewLine->SetNext(pNext);
        pNewLine->SetPrev(pLine);
        pNext->SetPrev(pNewLine);
        pLine->SetNext(pNewLine);
    }

    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("        head: %s"),
                ((m_linesHead) ? (const wxChar*)m_linesHead->Text().c_str()
                               : wxEmptyString) );
    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("        tail: %s"),
                ((m_linesTail) ? (const wxChar*)m_linesTail->Text().c_str()
                               : wxEmptyString) );

    return pNewLine;
}

void wxFileConfig::LineListRemove(wxFileConfigLineList *pLine)
{
    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("    ** Removing Line '%s'"),
                pLine->Text().c_str() );
    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("        head: %s"),
                ((m_linesHead) ? (const wxChar*)m_linesHead->Text().c_str()
                               : wxEmptyString) );
    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("        tail: %s"),
                ((m_linesTail) ? (const wxChar*)m_linesTail->Text().c_str()
                               : wxEmptyString) );

    wxFileConfigLineList    *pPrev = pLine->Prev(),
                            *pNext = pLine->Next();

        
    if ( pPrev == NULL )
        m_linesHead = pNext;
    else
        pPrev->SetNext(pNext);

        
    if ( pNext == NULL )
        m_linesTail = pPrev;
    else
        pNext->SetPrev(pPrev);

    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("        head: %s"),
                ((m_linesHead) ? (const wxChar*)m_linesHead->Text().c_str()
                               : wxEmptyString) );
    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("        tail: %s"),
                ((m_linesTail) ? (const wxChar*)m_linesTail->Text().c_str()
                               : wxEmptyString) );

    delete pLine;
}

bool wxFileConfig::LineListIsEmpty()
{
    return m_linesHead == NULL;
}



wxFileConfigGroup::wxFileConfigGroup(wxFileConfigGroup *pParent,
                                       const wxString& strName,
                                       wxFileConfig *pConfig)
                         : m_aEntries(CompareEntries),
                           m_aSubgroups(CompareGroups),
                           m_strName(strName)
{
  m_pConfig = pConfig;
  m_pParent = pParent;
  m_pLine   = NULL;

  m_pLastEntry = NULL;
  m_pLastGroup = NULL;
}

wxFileConfigGroup::~wxFileConfigGroup()
{
    size_t n, nCount = m_aEntries.GetCount();
  for ( n = 0; n < nCount; n++ )
    delete m_aEntries[n];

    nCount = m_aSubgroups.GetCount();
  for ( n = 0; n < nCount; n++ )
    delete m_aSubgroups[n];
}


void wxFileConfigGroup::SetLine(wxFileConfigLineList *pLine)
{
            wxASSERT_MSG( !m_pParent || !m_pLine || !pLine,
                   wxT("changing line for a non-root group?") );

    m_pLine = pLine;
}



wxFileConfigLineList *wxFileConfigGroup::GetGroupLine()
{
    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("  GetGroupLine() for Group '%s'"),
                Name().c_str() );

    if ( !m_pLine )
    {
        wxLogTrace( FILECONF_TRACE_MASK,
                    wxT("    Getting Line item pointer") );

        wxFileConfigGroup   *pParent = Parent();

                if ( pParent )
        {
            wxLogTrace( FILECONF_TRACE_MASK,
                        wxT("    checking parent '%s'"),
                        pParent->Name().c_str() );

            wxString    strFullName;

                        strFullName << wxT("[")
                        << FilterOutEntryName(GetFullName().c_str() + 1)
                        << wxT("]");
            m_pLine = m_pConfig->LineListInsert(strFullName,
                                                pParent->GetLastGroupLine());
            pParent->SetLastGroup(this);          }
                    }

    return m_pLine;
}

wxFileConfigLineList *wxFileConfigGroup::GetLastGroupLine()
{
            if ( m_pLastGroup )
    {
        wxFileConfigLineList *pLine = m_pLastGroup->GetLastGroupLine();

        wxASSERT_MSG( pLine, wxT("last group must have !NULL associated line") );

        return pLine;
    }

        return GetLastEntryLine();
}

wxFileConfigLineList *wxFileConfigGroup::GetLastEntryLine()
{
    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("  GetLastEntryLine() for Group '%s'"),
                Name().c_str() );

    if ( m_pLastEntry )
    {
        wxFileConfigLineList    *pLine = m_pLastEntry->GetLine();

        wxASSERT_MSG( pLine, wxT("last entry must have !NULL associated line") );

        return pLine;
    }

        return GetGroupLine();
}

void wxFileConfigGroup::SetLastEntry(wxFileConfigEntry *pEntry)
{
    m_pLastEntry = pEntry;

    if ( !m_pLine )
    {
                                wxASSERT_MSG( !m_pParent, wxT("unexpected for non root group") );

                m_pLine = pEntry->GetLine();
    }
}


void wxFileConfigGroup::UpdateGroupAndSubgroupsLines()
{
        wxFileConfigLineList *line = GetGroupLine();
    wxCHECK_RET( line, wxT("a non root group must have a corresponding line!") );

        line->SetText(wxString::Format(wxT("[%s]"), GetFullName().c_str() + 1));


        const size_t nCount = m_aSubgroups.GetCount();
    for ( size_t n = 0; n < nCount; n++ )
    {
        m_aSubgroups[n]->UpdateGroupAndSubgroupsLines();
    }
}

void wxFileConfigGroup::Rename(const wxString& newName)
{
    wxCHECK_RET( m_pParent, wxT("the root group can't be renamed") );

    if ( newName == m_strName )
        return;

            m_pParent->m_aSubgroups.Remove(this);

    m_strName = newName;

    m_pParent->m_aSubgroups.Add(this);

        UpdateGroupAndSubgroupsLines();
}

wxString wxFileConfigGroup::GetFullName() const
{
    wxString fullname;
    if ( Parent() )
        fullname = Parent()->GetFullName() + wxCONFIG_PATH_SEPARATOR + Name();

    return fullname;
}


wxFileConfigEntry *
wxFileConfigGroup::FindEntry(const wxString& name) const
{
  size_t i,
       lo = 0,
       hi = m_aEntries.GetCount();
  int res;
  wxFileConfigEntry *pEntry;

  while ( lo < hi ) {
    i = (lo + hi)/2;
    pEntry = m_aEntries[i];

    #if wxCONFIG_CASE_SENSITIVE
      res = pEntry->Name().compare(name);
    #else
      res = pEntry->Name().CmpNoCase(name);
    #endif

    if ( res > 0 )
      hi = i;
    else if ( res < 0 )
      lo = i + 1;
    else
      return pEntry;
  }

  return NULL;
}

wxFileConfigGroup *
wxFileConfigGroup::FindSubgroup(const wxString& name) const
{
  size_t i,
       lo = 0,
       hi = m_aSubgroups.GetCount();
  int res;
  wxFileConfigGroup *pGroup;

  while ( lo < hi ) {
    i = (lo + hi)/2;
    pGroup = m_aSubgroups[i];

    #if wxCONFIG_CASE_SENSITIVE
      res = pGroup->Name().compare(name);
    #else
      res = pGroup->Name().CmpNoCase(name);
    #endif

    if ( res > 0 )
      hi = i;
    else if ( res < 0 )
      lo = i + 1;
    else
      return pGroup;
  }

  return NULL;
}


wxFileConfigEntry *wxFileConfigGroup::AddEntry(const wxString& strName, int nLine)
{
    wxASSERT( FindEntry(strName) == 0 );

    wxFileConfigEntry   *pEntry = new wxFileConfigEntry(this, strName, nLine);

    m_aEntries.Add(pEntry);
    return pEntry;
}

wxFileConfigGroup *wxFileConfigGroup::AddSubgroup(const wxString& strName)
{
    wxASSERT( FindSubgroup(strName) == 0 );

    wxFileConfigGroup   *pGroup = new wxFileConfigGroup(this, strName, m_pConfig);

    m_aSubgroups.Add(pGroup);
    return pGroup;
}




bool wxFileConfigGroup::DeleteSubgroupByName(const wxString& name)
{
    wxFileConfigGroup * const pGroup = FindSubgroup(name);

    return pGroup ? DeleteSubgroup(pGroup) : false;
}

bool wxFileConfigGroup::DeleteSubgroup(wxFileConfigGroup *pGroup)
{
    wxCHECK_MSG( pGroup, false, wxT("deleting non existing group?") );

    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("Deleting group '%s' from '%s'"),
                pGroup->Name().c_str(),
                Name().c_str() );

    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("  (m_pLine) = prev: %p, this %p, next %p"),
                m_pLine ? static_cast<void*>(m_pLine->Prev()) : 0,
                static_cast<void*>(m_pLine),
                m_pLine ? static_cast<void*>(m_pLine->Next()) : 0 );
    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("  text: '%s'"),
                m_pLine ? (const wxChar*)m_pLine->Text().c_str()
                        : wxEmptyString );

        size_t nCount = pGroup->m_aEntries.GetCount();

    wxLogTrace(FILECONF_TRACE_MASK,
               wxT("Removing %lu entries"), (unsigned long)nCount );

    for ( size_t nEntry = 0; nEntry < nCount; nEntry++ )
    {
        wxFileConfigLineList *pLine = pGroup->m_aEntries[nEntry]->GetLine();

        if ( pLine )
        {
            wxLogTrace( FILECONF_TRACE_MASK,
                        wxT("    '%s'"),
                        pLine->Text().c_str() );
            m_pConfig->LineListRemove(pLine);
        }
    }

        nCount = pGroup->m_aSubgroups.GetCount();

    wxLogTrace( FILECONF_TRACE_MASK,
                wxT("Removing %lu subgroups"), (unsigned long)nCount );

    for ( size_t nGroup = 0; nGroup < nCount; nGroup++ )
    {
        pGroup->DeleteSubgroup(pGroup->m_aSubgroups[0]);
    }

        wxFileConfigLineList *pLine = pGroup->m_pLine;
    if ( pLine )
    {
        wxLogTrace( FILECONF_TRACE_MASK,
                    wxT("  Removing line for group '%s' : '%s'"),
                    pGroup->Name().c_str(),
                    pLine->Text().c_str() );
        wxLogTrace( FILECONF_TRACE_MASK,
                    wxT("  Removing from group '%s' : '%s'"),
                    Name().c_str(),
                    ((m_pLine) ? (const wxChar*)m_pLine->Text().c_str()
                               : wxEmptyString) );

                        if ( pGroup == m_pLastGroup )
        {
            wxLogTrace( FILECONF_TRACE_MASK,
                        wxT("  Removing last group") );

                                                const size_t nSubgroups = m_aSubgroups.GetCount();

            m_pLastGroup = NULL;
            for ( wxFileConfigLineList *pl = pLine->Prev();
                  pl && !m_pLastGroup;
                  pl = pl->Prev() )
            {
                                for ( size_t n = 0; n < nSubgroups; n++ )
                {
                                                            if ( m_aSubgroups[n]->m_pLine == pl )
                    {
                        m_pLastGroup = m_aSubgroups[n];
                        break;
                    }
                }

                if ( pl == m_pLine )
                    break;
            }
        }

        m_pConfig->LineListRemove(pLine);
    }
    else
    {
        wxLogTrace( FILECONF_TRACE_MASK,
                    wxT("  No line entry for Group '%s'?"),
                    pGroup->Name().c_str() );
    }

    m_aSubgroups.Remove(pGroup);
    delete pGroup;

    return true;
}

bool wxFileConfigGroup::DeleteEntry(const wxString& name)
{
  wxFileConfigEntry *pEntry = FindEntry(name);
  if ( !pEntry )
  {
            return false;
  }

  wxFileConfigLineList *pLine = pEntry->GetLine();
  if ( pLine != NULL ) {
            if ( pEntry == m_pLastEntry ) {
            wxASSERT( m_pLine != NULL );  
            wxFileConfigEntry *pNewLast = NULL;
      const wxFileConfigLineList * const
        pNewLastLine = m_pLastEntry->GetLine()->Prev();
      const size_t nEntries = m_aEntries.GetCount();
      for ( size_t n = 0; n < nEntries; n++ ) {
        if ( m_aEntries[n]->GetLine() == pNewLastLine ) {
          pNewLast = m_aEntries[n];
          break;
        }
      }

                  m_pLastEntry = pNewLast;

                  if ( pLine == m_pLine )
          SetLine(NULL);
    }

    m_pConfig->LineListRemove(pLine);
  }

  m_aEntries.Remove(pEntry);
  delete pEntry;

  return true;
}


wxFileConfigEntry::wxFileConfigEntry(wxFileConfigGroup *pParent,
                                       const wxString& strName,
                                       int nLine)
                         : m_strName(strName)
{
  wxASSERT( !strName.empty() );

  m_pParent = pParent;
  m_nLine   = nLine;
  m_pLine   = NULL;

  m_bHasValue = false;

  m_bImmutable = strName[0] == wxCONFIG_IMMUTABLE_PREFIX;
  if ( m_bImmutable )
    m_strName.erase(0, 1);  }


void wxFileConfigEntry::SetLine(wxFileConfigLineList *pLine)
{
  if ( m_pLine != NULL ) {
    wxLogWarning(_("entry '%s' appears more than once in group '%s'"),
                 Name().c_str(), m_pParent->GetFullName().c_str());
  }

  m_pLine = pLine;
  Group()->SetLastEntry(this);
}

void wxFileConfigEntry::SetValue(const wxString& strValue, bool bUser)
{
    if ( bUser && IsImmutable() )
    {
        wxLogWarning( _("attempt to change immutable key '%s' ignored."),
                      Name().c_str());
        return;
    }

            if ( m_bHasValue && strValue == m_strValue )
        return;

    m_bHasValue = true;
    m_strValue = strValue;

    if ( bUser )
    {
        wxString strValFiltered;

        if ( Group()->Config()->GetStyle() & wxCONFIG_USE_NO_ESCAPE_CHARACTERS )
        {
            strValFiltered = strValue;
        }
        else {
            strValFiltered = FilterOutValue(strValue);
        }

        wxString    strLine;
        strLine << FilterOutEntryName(m_strName) << wxT('=') << strValFiltered;

        if ( m_pLine )
        {
                        m_pLine->SetText(strLine);
        }
        else         {
                                                            wxFileConfigLineList *line = Group()->GetLastEntryLine();
            m_pLine = Group()->Config()->LineListInsert(strLine, line);

            Group()->SetLastEntry(this);
        }
    }
}



int CompareEntries(wxFileConfigEntry *p1, wxFileConfigEntry *p2)
{
#if wxCONFIG_CASE_SENSITIVE
    return p1->Name().compare(p2->Name());
#else
    return p1->Name().CmpNoCase(p2->Name());
#endif
}

int CompareGroups(wxFileConfigGroup *p1, wxFileConfigGroup *p2)
{
#if wxCONFIG_CASE_SENSITIVE
    return p1->Name().compare(p2->Name());
#else
    return p1->Name().CmpNoCase(p2->Name());
#endif
}


static wxString FilterInValue(const wxString& str)
{
    wxString strResult;
    if ( str.empty() )
        return strResult;

    strResult.reserve(str.length());

    wxString::const_iterator i = str.begin();
    const bool bQuoted = *i == '"';
    if ( bQuoted )
        ++i;

    for ( const wxString::const_iterator end = str.end(); i != end; ++i )
    {
        if ( *i == wxT('\\') )
        {
            if ( ++i == end )
            {
                wxLogWarning(_("trailing backslash ignored in '%s'"), str.c_str());
                break;
            }

            switch ( (*i).GetValue() )
            {
                case wxT('n'):
                    strResult += wxT('\n');
                    break;

                case wxT('r'):
                    strResult += wxT('\r');
                    break;

                case wxT('t'):
                    strResult += wxT('\t');
                    break;

                case wxT('\\'):
                    strResult += wxT('\\');
                    break;

                case wxT('"'):
                    strResult += wxT('"');
                    break;
            }
        }
        else         {
            if ( *i != wxT('"') || !bQuoted )
            {
                strResult += *i;
            }
            else if ( i != end - 1 )
            {
                wxLogWarning(_("unexpected \" at position %d in '%s'."),
                             i - str.begin(), str.c_str());
            }
                    }
    }

    return strResult;
}

static wxString FilterOutValue(const wxString& str)
{
   if ( !str )
      return str;

  wxString strResult;
  strResult.Alloc(str.Len());

    bool bQuote = wxIsspace(str[0]) || str[0] == wxT('"');

  if ( bQuote )
    strResult += wxT('"');

  wxChar c;
  for ( size_t n = 0; n < str.Len(); n++ ) {
    switch ( str[n].GetValue() ) {
      case wxT('\n'):
        c = wxT('n');
        break;

      case wxT('\r'):
        c = wxT('r');
        break;

      case wxT('\t'):
        c = wxT('t');
        break;

      case wxT('\\'):
        c = wxT('\\');
        break;

      case wxT('"'):
        if ( bQuote ) {
          c = wxT('"');
          break;
        }
        wxFALLTHROUGH;

      default:
        strResult += str[n];
        continue;       }

        strResult << wxT('\\') << c;
  }

  if ( bQuote )
    strResult += wxT('"');

  return strResult;
}

static wxString FilterInEntryName(const wxString& str)
{
  wxString strResult;
  strResult.Alloc(str.Len());

  for ( const wxChar *pc = str.c_str(); *pc != '\0'; pc++ ) {
    if ( *pc == wxT('\\') ) {
            if ( *++pc == wxT('\0') )
        break;
    }

    strResult += *pc;
  }

  return strResult;
}

static wxString FilterOutEntryName(const wxString& str)
{
  wxString strResult;
  strResult.Alloc(str.Len());

  for ( const wxChar *pc = str.c_str(); *pc != wxT('\0'); pc++ ) {
    const wxChar c = *pc;

                            if (
#if !wxUSE_UNICODE
            ((unsigned char)c < 127) &&
#endif          !wxIsalnum(c) && !wxStrchr(wxT("@_/-!.*%()"), c) )
    {
      strResult += wxT('\\');
    }

    strResult += c;
  }

  return strResult;
}

static wxString GetAppName(const wxString& appName)
{
    if ( !appName && wxTheApp )
        return wxTheApp->GetAppName();
    else
        return appName;
}

#endif 