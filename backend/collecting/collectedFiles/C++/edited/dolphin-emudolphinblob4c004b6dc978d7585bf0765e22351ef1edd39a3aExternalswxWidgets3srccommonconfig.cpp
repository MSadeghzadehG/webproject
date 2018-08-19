

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif  
#ifndef wxUSE_CONFIG_NATIVE
    #define wxUSE_CONFIG_NATIVE 1
#endif

#include "wx/config.h"

#ifndef WX_PRECOMP
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/app.h"
    #include "wx/utils.h"
    #include "wx/arrstr.h"
    #include "wx/math.h"
#endif 
#if wxUSE_CONFIG && ((wxUSE_FILE && wxUSE_TEXTFILE) || wxUSE_CONFIG_NATIVE)

#include "wx/apptrait.h"
#include "wx/file.h"

#include <stdlib.h>
#include <ctype.h>
#include <limits.h>     #include <float.h>      

wxConfigBase *wxConfigBase::ms_pConfig     = NULL;
bool          wxConfigBase::ms_bAutoCreate = true;



wxConfigBase *wxAppTraitsBase::CreateConfig()
{
    return new
    #if defined(__WINDOWS__) && wxUSE_CONFIG_NATIVE
        wxRegConfig(wxTheApp->GetAppName(), wxTheApp->GetVendorName());
    #else         wxFileConfig(wxTheApp->GetAppName());
    #endif
}

wxIMPLEMENT_ABSTRACT_CLASS(wxConfigBase, wxObject);

wxConfigBase::wxConfigBase(const wxString& appName,
                           const wxString& vendorName,
                           const wxString& WXUNUSED(localFilename),
                           const wxString& WXUNUSED(globalFilename),
                           long style)
            : m_appName(appName), m_vendorName(vendorName), m_style(style)
{
    m_bExpandEnvVars = true;
    m_bRecordDefaults = false;
}

wxConfigBase::~wxConfigBase()
{
    }

wxConfigBase *wxConfigBase::Set(wxConfigBase *pConfig)
{
  wxConfigBase *pOld = ms_pConfig;
  ms_pConfig = pConfig;
  return pOld;
}

wxConfigBase *wxConfigBase::Create()
{
  if ( ms_bAutoCreate && ms_pConfig == NULL ) {
    wxAppTraits * const traits = wxTheApp ? wxTheApp->GetTraits() : NULL;
    wxCHECK_MSG( traits, NULL, wxT("create wxApp before calling this") );

    ms_pConfig = traits->CreateConfig();
  }

  return ms_pConfig;
}


#define IMPLEMENT_READ_FOR_TYPE(name, type, deftype, extra)                 \
    bool wxConfigBase::Read(const wxString& key, type *val) const           \
    {                                                                       \
        wxCHECK_MSG( val, false, wxT("wxConfig::Read(): NULL parameter") );  \
                                                                            \
        if ( !DoRead##name(key, val) )                                      \
            return false;                                                   \
                                                                            \
        *val = extra(*val);                                                 \
                                                                            \
        return true;                                                        \
    }                                                                       \
                                                                            \
    bool wxConfigBase::Read(const wxString& key,                            \
                            type *val,                                      \
                            deftype defVal) const                           \
    {                                                                       \
        wxCHECK_MSG( val, false, wxT("wxConfig::Read(): NULL parameter") );  \
                                                                            \
        bool read = DoRead##name(key, val);                                 \
        if ( !read )                                                        \
        {                                                                   \
            if ( IsRecordingDefaults() )                                    \
            {                                                               \
                ((wxConfigBase *)this)->DoWrite##name(key, defVal);         \
            }                                                               \
                                                                            \
            *val = defVal;                                                  \
        }                                                                   \
                                                                            \
        *val = extra(*val);                                                 \
                                                                            \
        return read;                                                        \
    }


IMPLEMENT_READ_FOR_TYPE(String, wxString, const wxString&, ExpandEnvVars)
IMPLEMENT_READ_FOR_TYPE(Long, long, long, long)
IMPLEMENT_READ_FOR_TYPE(Double, double, double, double)
IMPLEMENT_READ_FOR_TYPE(Bool, bool, bool, bool)

#undef IMPLEMENT_READ_FOR_TYPE

bool wxConfigBase::Read(const wxString& key, int *pi) const
{
    long l = *pi;
    bool r = Read(key, &l);
    wxASSERT_MSG( l < INT_MAX, wxT("int overflow in wxConfig::Read") );
    *pi = (int)l;
    return r;
}

bool wxConfigBase::Read(const wxString& key, int *pi, int defVal) const
{
    long l = *pi;
    bool r = Read(key, &l, defVal);
    wxASSERT_MSG( l < INT_MAX, wxT("int overflow in wxConfig::Read") );
    *pi = (int)l;
    return r;
}

bool wxConfigBase::Read(const wxString& key, float* val) const
{
    wxCHECK_MSG( val, false, wxT("wxConfig::Read(): NULL parameter") );

    double temp;
    if ( !Read(key, &temp) )
        return false;

    wxCHECK_MSG( fabs(temp) <= FLT_MAX, false,
                     wxT("float overflow in wxConfig::Read") );
    wxCHECK_MSG( (temp == 0.0) || (fabs(temp) >= FLT_MIN), false,
                     wxT("float underflow in wxConfig::Read") );

    *val = static_cast<float>(temp);

    return true;
}

bool wxConfigBase::Read(const wxString& key, float* val, float defVal) const
{
    wxCHECK_MSG( val, false, wxT("wxConfig::Read(): NULL parameter") );

    if ( Read(key, val) )
        return true;

    *val = defVal;
    return false;
}

bool wxConfigBase::DoReadBool(const wxString& key, bool* val) const
{
    wxCHECK_MSG( val, false, wxT("wxConfig::Read(): NULL parameter") );

    long l;
    if ( !DoReadLong(key, &l) )
        return false;

    if ( l != 0 && l != 1 )
    {
                                wxLogWarning(_("Invalid value %ld for a boolean key \"%s\" in "
                       "config file."),
                     l, key);
    }

    *val = l != 0;

    return true;
}

bool wxConfigBase::DoReadDouble(const wxString& key, double* val) const
{
    wxString str;
    if ( Read(key, &str) )
    {
        if ( str.ToCDouble(val) )
            return true;

                                if ( str.ToDouble(val) )
            return true;
    }

    return false;
}

wxString wxConfigBase::ExpandEnvVars(const wxString& str) const
{
    wxString tmp;     if (IsExpandingEnvVars())
        tmp = wxExpandEnvVars(str);
    else
        tmp = str;
    return tmp;
}


bool wxConfigBase::DoWriteDouble(const wxString& key, double val)
{
                return DoWriteString(key, wxString::FromCDouble(val));
}

bool wxConfigBase::DoWriteBool(const wxString& key, bool value)
{
    return DoWriteLong(key, value ? 1l : 0l);
}


wxConfigPathChanger::wxConfigPathChanger(const wxConfigBase *pContainer,
                                         const wxString& strEntry)
{
  m_bChanged = false;
  m_pContainer = const_cast<wxConfigBase *>(pContainer);

      wxString strPath = strEntry.BeforeLast(wxCONFIG_PATH_SEPARATOR, &m_strName);

    if ( strPath.empty() &&
       ((!strEntry.empty()) && strEntry[0] == wxCONFIG_PATH_SEPARATOR) )
  {
    strPath = wxCONFIG_PATH_SEPARATOR;
  }

  if ( !strPath.empty() )
  {
    if ( m_pContainer->GetPath() != strPath )
    {
                m_bChanged = true;

        
        m_strOldPath = m_pContainer->GetPath().wc_str();
        if ( *m_strOldPath.c_str() != wxCONFIG_PATH_SEPARATOR )
          m_strOldPath += wxCONFIG_PATH_SEPARATOR;
        m_pContainer->SetPath(strPath);
    }
  }
}

void wxConfigPathChanger::UpdateIfDeleted()
{
        if ( !m_bChanged )
        return;

        while ( !m_pContainer->HasGroup(m_strOldPath) )
    {
        m_strOldPath = m_strOldPath.BeforeLast(wxCONFIG_PATH_SEPARATOR);
        if ( m_strOldPath.empty() )
            m_strOldPath = wxCONFIG_PATH_SEPARATOR;
    }
}

wxConfigPathChanger::~wxConfigPathChanger()
{
    if ( m_bChanged ) {
    m_pContainer->SetPath(m_strOldPath);
  }
}


wxString wxConfigBase::RemoveTrailingSeparator(const wxString& key)
{
    wxString path(key);

        while ( path.length() > 1 )
    {
        if ( *path.rbegin() != wxCONFIG_PATH_SEPARATOR )
            break;

        path.erase(path.end() - 1);
    }

    return path;
}

#endif 


enum Bracket
{
  Bracket_None,
  Bracket_Normal  = ')',
  Bracket_Curly   = '}',
#ifdef  __WINDOWS__
  Bracket_Windows = '%',    #endif
  Bracket_Max
};

wxString wxExpandEnvVars(const wxString& str)
{
  wxString strResult;
  strResult.Alloc(str.length());

  size_t m;
  for ( size_t n = 0; n < str.length(); n++ ) {
    switch ( str[n].GetValue() ) {
#ifdef __WINDOWS__
      case wxT('%'):
#endif       case wxT('$'):
        {
          Bracket bracket;
          #ifdef __WINDOWS__
            if ( str[n] == wxT('%') )
              bracket = Bracket_Windows;
            else
          #endif           if ( n == str.length() - 1 ) {
            bracket = Bracket_None;
          }
          else {
            switch ( str[n + 1].GetValue() ) {
              case wxT('('):
                bracket = Bracket_Normal;
                n++;                                   break;

              case wxT('{'):
                bracket = Bracket_Curly;
                n++;                                   break;

              default:
                bracket = Bracket_None;
            }
          }

          m = n + 1;

          while ( m < str.length() && (wxIsalnum(str[m]) || str[m] == wxT('_')) )
            m++;

          wxString strVarName(str.c_str() + n + 1, m - n - 1);

                              bool expanded = false;
          wxString tmp;
          if (wxGetEnv(strVarName, &tmp))
          {
              strResult += tmp;
              expanded = true;
          }
          else
          {
                        #ifdef  __WINDOWS__
              if ( bracket != Bracket_Windows )
            #endif
                if ( bracket != Bracket_None )
                  strResult << str[n - 1];
            strResult << str[n] << strVarName;
          }

                    if ( bracket != Bracket_None ) {
            if ( m == str.length() || str[m] != (wxChar)bracket ) {
                                                                                                  #ifndef __WINDOWS__
                wxLogWarning(_("Environment variables expansion failed: missing '%c' at position %u in '%s'."),
                             (char)bracket, (unsigned int) (m + 1), str.c_str());
              #endif             }
            else {
                            if ( !expanded )
                strResult << (wxChar)bracket;
              m++;
            }
          }

          n = m - 1;          }
        break;

      case wxT('\\'):
                if ( n != str.length() - 1 &&
                (str[n + 1] == wxT('%') || str[n + 1] == wxT('$')) ) {
          strResult += str[++n];

          break;
        }
        wxFALLTHROUGH;

      default:
        strResult += str[n];
    }
  }

  return strResult;
}

void wxSplitPath(wxArrayString& aParts, const wxString& path)
{
  aParts.clear();

  wxString strCurrent;
  wxString::const_iterator pc = path.begin();
  for ( ;; ) {
    if ( pc == path.end() || *pc == wxCONFIG_PATH_SEPARATOR ) {
      if ( strCurrent == wxT(".") ) {
              }
      else if ( strCurrent == wxT("..") ) {
                if ( aParts.size() == 0 )
        {
          wxLogWarning(_("'%s' has extra '..', ignored."), path);
        }
        else
        {
          aParts.erase(aParts.end() - 1);
        }

        strCurrent.Empty();
      }
      else if ( !strCurrent.empty() ) {
        aParts.push_back(strCurrent);
        strCurrent.Empty();
      }
              
      if ( pc == path.end() )
        break;
    }
    else
      strCurrent += *pc;

    ++pc;
  }
}
