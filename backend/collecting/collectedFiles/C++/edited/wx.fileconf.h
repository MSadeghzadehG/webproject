
#ifndef   _FILECONF_H
#define   _FILECONF_H

#include "wx/defs.h"

#if wxUSE_CONFIG

#include "wx/textfile.h"
#include "wx/string.h"
#include "wx/confbase.h"
#include "wx/filename.h"



class WXDLLIMPEXP_FWD_BASE wxFileConfigGroup;
class WXDLLIMPEXP_FWD_BASE wxFileConfigEntry;
class WXDLLIMPEXP_FWD_BASE wxFileConfigLineList;

#if wxUSE_STREAMS
class WXDLLIMPEXP_FWD_BASE wxInputStream;
class WXDLLIMPEXP_FWD_BASE wxOutputStream;
#endif 
class WXDLLIMPEXP_BASE wxFileConfig : public wxConfigBase
{
public:
                      static wxFileName GetGlobalFile(const wxString& szFile);
  static wxFileName GetLocalFile(const wxString& szFile, int style = 0);

  static wxString GetGlobalFileName(const wxString& szFile)
  {
      return GetGlobalFile(szFile).GetFullPath();
  }

  static wxString GetLocalFileName(const wxString& szFile, int style = 0)
  {
      return GetLocalFile(szFile, style).GetFullPath();
  }

            wxFileConfig(const wxString& appName = wxEmptyString,
               const wxString& vendorName = wxEmptyString,
               const wxString& localFilename = wxEmptyString,
               const wxString& globalFilename = wxEmptyString,
               long style = wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_GLOBAL_FILE,
               const wxMBConv& conv = wxConvAuto());

#if wxUSE_STREAMS
      wxFileConfig(wxInputStream &inStream, const wxMBConv& conv = wxConvAuto());
#endif 
      virtual ~wxFileConfig();

    #ifdef __UNIX__
  void SetUmask(int mode) { m_umask = mode; }
#else   void SetUmask(int WXUNUSED(mode)) { }
#endif 
    virtual void SetPath(const wxString& strPath) wxOVERRIDE;
  virtual const wxString& GetPath() const wxOVERRIDE;

  virtual bool GetFirstGroup(wxString& str, long& lIndex) const wxOVERRIDE;
  virtual bool GetNextGroup (wxString& str, long& lIndex) const wxOVERRIDE;
  virtual bool GetFirstEntry(wxString& str, long& lIndex) const wxOVERRIDE;
  virtual bool GetNextEntry (wxString& str, long& lIndex) const wxOVERRIDE;

  virtual size_t GetNumberOfEntries(bool bRecursive = false) const wxOVERRIDE;
  virtual size_t GetNumberOfGroups(bool bRecursive = false) const wxOVERRIDE;

  virtual bool HasGroup(const wxString& strName) const wxOVERRIDE;
  virtual bool HasEntry(const wxString& strName) const wxOVERRIDE;

  virtual bool Flush(bool bCurrentOnly = false) wxOVERRIDE;

  virtual bool RenameEntry(const wxString& oldName, const wxString& newName) wxOVERRIDE;
  virtual bool RenameGroup(const wxString& oldName, const wxString& newName) wxOVERRIDE;

  virtual bool DeleteEntry(const wxString& key, bool bGroupIfEmptyAlso = true) wxOVERRIDE;
  virtual bool DeleteGroup(const wxString& szKey) wxOVERRIDE;
  virtual bool DeleteAll() wxOVERRIDE;

  #if wxUSE_STREAMS
        virtual bool Save(wxOutputStream& os, const wxMBConv& conv = wxConvAuto());
#endif 
public:
    wxFileConfigLineList *LineListAppend(const wxString& str);
  wxFileConfigLineList *LineListInsert(const wxString& str,
                           wxFileConfigLineList *pLine);      void      LineListRemove(wxFileConfigLineList *pLine);
  bool      LineListIsEmpty();

protected:
  virtual bool DoReadString(const wxString& key, wxString *pStr) const wxOVERRIDE;
  virtual bool DoReadLong(const wxString& key, long *pl) const wxOVERRIDE;
#if wxUSE_BASE64
  virtual bool DoReadBinary(const wxString& key, wxMemoryBuffer* buf) const wxOVERRIDE;
#endif 
  virtual bool DoWriteString(const wxString& key, const wxString& szValue) wxOVERRIDE;
  virtual bool DoWriteLong(const wxString& key, long lValue) wxOVERRIDE;
#if wxUSE_BASE64
  virtual bool DoWriteBinary(const wxString& key, const wxMemoryBuffer& buf) wxOVERRIDE;
#endif 
private:
    static wxString GetGlobalDir();
  static wxString GetLocalDir(int style = 0);

      void Init();

    void CleanUp();

    void Parse(const wxTextBuffer& buffer, bool bLocal);

    void SetRootPath();

      bool DoSetPath(const wxString& strPath, bool createMissingComponents);

    void SetDirty() { m_isDirty = true; }
  void ResetDirty() { m_isDirty = false; }
  bool IsDirty() const { return m_isDirty; }


      wxFileConfigLineList *m_linesHead,                           *m_linesTail;    
  wxFileName  m_fnLocalFile,                          m_fnGlobalFile;             wxString    m_strPath;                
  wxFileConfigGroup *m_pRootGroup,                          *m_pCurrentGroup;   
  wxMBConv    *m_conv;

#ifdef __UNIX__
  int m_umask;                          #endif 
  bool m_isDirty;                       
  wxDECLARE_NO_COPY_CLASS(wxFileConfig);
  wxDECLARE_ABSTRACT_CLASS(wxFileConfig);
};

#endif
  
#endif
  
