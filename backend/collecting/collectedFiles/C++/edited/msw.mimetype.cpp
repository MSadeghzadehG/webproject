
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_MIMETYPE

#include "wx/msw/mimetype.h"

#ifndef WX_PRECOMP
    #include "wx/dynarray.h"
    #include "wx/string.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/crt.h"
    #if wxUSE_GUI
        #include "wx/icon.h"
        #include "wx/msgdlg.h"
    #endif
#endif 
#include "wx/file.h"
#include "wx/iconloc.h"
#include "wx/confbase.h"
#include "wx/dynlib.h"

#ifdef __WINDOWS__
    #include "wx/msw/registry.h"
    #include "wx/msw/private.h"
    #include <shlwapi.h>
    #include <shlobj.h>

            #ifdef __VISUALC__
        #pragma comment(lib, "shlwapi")
    #endif
#endif 
#define wxASSOCF_NOTRUNCATE (static_cast<ASSOCF>(0x20))
#define wxASSOCSTR_DEFAULTICON (static_cast<ASSOCSTR>(15))

#include <ctype.h>

class WXDLLIMPEXP_FWD_CORE wxIcon;


static const wxStringCharType *CLASSES_ROOT_KEY = wxS("Software\\Classes\\");

static const wxChar *MIME_DATABASE_KEY = wxT("MIME\\Database\\Content Type\\");

static bool CanonicalizeParams(wxString& command)
{
        
                bool foundFilename = false;
    size_t len = command.length();
    for ( size_t n = 0; (n < len) && !foundFilename; n++ )
    {
        if ( command[n] == wxT('%') &&
                (n + 1 < len) &&
                (command[n + 1] == wxT('1') || command[n + 1] == wxT('L')) )
        {
                        command[n + 1] = wxT('s');

            foundFilename = true;
        }
    }

    if ( foundFilename )
    {
                                        command.Replace(" %*", "");
    }

    return foundFilename;
}

void wxFileTypeImpl::Init(const wxString& strFileType, const wxString& ext)
{
        wxCHECK_RET( !ext.empty(), wxT("needs an extension") );

    if ( ext[0u] != wxT('.') ) {
        m_ext = wxT('.');
    }
    m_ext << ext;

    m_strFileType = strFileType;
    if ( !strFileType ) {
        m_strFileType = m_ext.AfterFirst('.') + wxT("_auto_file");
    }

    m_suppressNotify = false;
}

wxString wxFileTypeImpl::GetVerbPath(const wxString& verb) const
{
    wxString path;
    path << m_strFileType << wxT("\\shell\\") << verb << wxT("\\command");
    return path;
}

size_t wxFileTypeImpl::GetAllCommands(wxArrayString *verbs,
                                      wxArrayString *commands,
                                      const wxFileType::MessageParameters& params) const
{
    wxCHECK_MSG( !m_ext.empty(), 0, wxT("GetAllCommands() needs an extension") );

    if ( m_strFileType.empty() )
    {
                wxFileTypeImpl *self = wxConstCast(this, wxFileTypeImpl);
        wxRegKey rkey(wxRegKey::HKCR, m_ext);
        if ( !rkey.Exists() || !rkey.QueryValue(wxEmptyString, self->m_strFileType) )
        {
            wxLogDebug(wxT("Can't get the filetype for extension '%s'."),
                       m_ext.c_str());

            return 0;
        }
    }

        size_t count = 0;
    wxRegKey rkey(wxRegKey::HKCR, m_strFileType  + wxT("\\shell"));
    long dummy;
    wxString verb;
    bool ok = rkey.GetFirstKey(verb, dummy);
    while ( ok )
    {
        wxString command = wxFileType::ExpandCommand(GetCommand(verb), params);

        
        if ( verb.CmpNoCase(wxT("open")) == 0 )
        {
            if ( verbs )
                verbs->Insert(verb, 0);
            if ( commands )
                commands->Insert(command, 0);
        }
        else         {
            if ( verbs )
                verbs->Add(verb);
            if ( commands )
                commands->Add(command);
        }

        count++;

        ok = rkey.GetNextKey(verb, dummy);
    }

    return count;
}

void wxFileTypeImpl::MSWNotifyShell()
{
    if (!m_suppressNotify)
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT, NULL, NULL);
}

void wxFileTypeImpl::MSWSuppressNotifications(bool supress)
{
    m_suppressNotify = supress;
}


bool wxFileTypeImpl::EnsureExtKeyExists()
{
    wxRegKey rkey(wxRegKey::HKCU, CLASSES_ROOT_KEY + m_ext);
    if ( !rkey.Exists() )
    {
        if ( !rkey.Create() || !rkey.SetValue(wxEmptyString, m_strFileType) )
        {
            wxLogError(_("Failed to create registry entry for '%s' files."),
                       m_ext.c_str());
            return false;
        }
    }

    return true;
}


static
wxString wxAssocQueryString(ASSOCSTR assoc,
                            wxString ext,
                            const wxString& verb = wxString())
{
    DWORD dwSize = MAX_PATH;
    TCHAR bufOut[MAX_PATH] = { 0 };

    if ( ext.empty() || ext[0] != '.' )
        ext.Prepend('.');

    HRESULT hr = ::AssocQueryString
                 (
                    wxASSOCF_NOTRUNCATE,                    assoc,                                  ext.t_str(),                            verb.empty() ? NULL
                                 : static_cast<const TCHAR*>(verb.t_str()),
                    bufOut,                                 &dwSize                              );

            if ( hr != S_OK )
    {
                                                if ( hr != HRESULT_FROM_WIN32(ERROR_NO_ASSOCIATION) &&
                hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) )
        {
            wxLogApiError("AssocQueryString", hr);
        }

        return wxString();
    }

    return wxString(bufOut);
}


wxString wxFileTypeImpl::GetCommand(const wxString& verb) const
{
    wxString command = wxAssocQueryString(ASSOCSTR_COMMAND, m_ext, verb);
    bool foundFilename = CanonicalizeParams(command);

#if wxUSE_IPC
    wxString ddeCommand = wxAssocQueryString(ASSOCSTR_DDECOMMAND, m_ext);
    wxString ddeTopic = wxAssocQueryString(ASSOCSTR_DDETOPIC, m_ext);

    if ( !ddeCommand.empty() && !ddeTopic.empty() )
    {
        wxString ddeServer = wxAssocQueryString( ASSOCSTR_DDEAPPLICATION, m_ext );

        ddeCommand.Replace(wxT("%1"), wxT("%s"));

        if ( ddeTopic.empty() )
            ddeTopic = wxT("System");

                                command.Prepend(wxT("WX_DDE#"));
        command << wxT('#') << ddeServer
                << wxT('#') << ddeTopic
                << wxT('#') << ddeCommand;
    }
    else
#endif     if ( !foundFilename && !command.empty() )
    {
                                                command << wxT(" %s");
    }

    return command;
}


wxString
wxFileTypeImpl::GetExpandedCommand(const wxString & verb,
                                   const wxFileType::MessageParameters& params) const
{
    wxString cmd = GetCommand(verb);

            if ( cmd.empty() && (verb == wxT("open")) )
        cmd = GetCommand(wxT("show"));

    return wxFileType::ExpandCommand(cmd, params);
}


bool wxFileTypeImpl::GetExtensions(wxArrayString& extensions)
{
    if ( m_ext.empty() ) {
                        return false;
    }
    else {
        extensions.Empty();
        extensions.Add(m_ext);

                return true;
    }
}

bool wxFileTypeImpl::GetMimeType(wxString *mimeType) const
{
        wxLogNull nolog;
    wxRegKey key(wxRegKey::HKCR, m_ext);

    return key.Open(wxRegKey::Read) &&
                key.QueryValue(wxT("Content Type"), *mimeType);
}

bool wxFileTypeImpl::GetMimeTypes(wxArrayString& mimeTypes) const
{
    wxString s;

    if ( !GetMimeType(&s) )
    {
        return false;
    }

    mimeTypes.Clear();
    mimeTypes.Add(s);
    return true;
}


bool wxFileTypeImpl::GetIcon(wxIconLocation *iconLoc) const
{
    wxString strIcon = wxAssocQueryString(wxASSOCSTR_DEFAULTICON, m_ext);

    if ( !strIcon.empty() )
    {
        wxString strFullPath = strIcon.BeforeLast(wxT(',')),
        strIndex = strIcon.AfterLast(wxT(','));

                        if ( strFullPath.empty() ) {
            strFullPath = strIndex;
            strIndex = wxT("0");
        }

                                if ( strFullPath.StartsWith('"') && strFullPath.EndsWith('"') )
            strFullPath = strFullPath.substr(1, strFullPath.length() - 2);

        if ( iconLoc )
        {
            iconLoc->SetFileName(wxExpandEnvVars(strFullPath));

            iconLoc->SetIndex(wxAtoi(strIndex));
        }

      return true;
    }

        return false;
}

bool wxFileTypeImpl::GetDescription(wxString *desc) const
{
        wxLogNull nolog;
    wxRegKey key(wxRegKey::HKCR, m_strFileType);

    if ( key.Open(wxRegKey::Read) ) {
                if ( key.QueryValue(wxEmptyString, *desc) ) {
            return true;
        }
    }

    return false;
}

wxFileType *
wxMimeTypesManagerImpl::CreateFileType(const wxString& filetype, const wxString& ext)
{
    wxFileType *fileType = new wxFileType;
    fileType->m_impl->Init(filetype, ext);
    return fileType;
}

wxFileType *
wxMimeTypesManagerImpl::GetFileTypeFromExtension(const wxString& ext)
{
        wxString str;
    if ( ext[0u] != wxT('.') ) {
        str = wxT('.');
    }
    str << ext;

        wxLogNull nolog;

    bool knownExtension = false;

    wxString strFileType;
    wxRegKey key(wxRegKey::HKCR, str);
    if ( key.Open(wxRegKey::Read) ) {
                if ( key.QueryValue(wxEmptyString, strFileType) ) {
                        return CreateFileType(strFileType, ext);
        }
        else {
                                                knownExtension = true;
        }
    }

    if ( !knownExtension )
    {
                return NULL;
    }

    return CreateFileType(wxEmptyString, ext);
}

wxFileType *
wxMimeTypesManagerImpl::GetFileTypeFromMimeType(const wxString& mimeType)
{
    wxString strKey = MIME_DATABASE_KEY;
    strKey << mimeType;

        wxLogNull nolog;

    wxString ext;
    wxRegKey key(wxRegKey::HKCR, strKey);
    if ( key.Open(wxRegKey::Read) ) {
        if ( key.QueryValue(wxT("Extension"), ext) ) {
            return GetFileTypeFromExtension(ext);
        }
    }

        return NULL;
}

size_t wxMimeTypesManagerImpl::EnumAllFileTypes(wxArrayString& mimetypes)
{
        wxRegKey key(wxRegKey::HKCR, MIME_DATABASE_KEY);

    wxString type;
    long cookie;
    bool cont = key.GetFirstKey(type, cookie);
    while ( cont )
    {
        mimetypes.Add(type);

        cont = key.GetNextKey(type, cookie);
    }

    return mimetypes.GetCount();
}


wxFileType *wxMimeTypesManagerImpl::Associate(const wxFileTypeInfo& ftInfo)
{
    wxCHECK_MSG( !ftInfo.GetExtensions().empty(), NULL,
                 wxT("Associate() needs extension") );

    bool ok;
    size_t iExtCount = 0;
    wxString filetype;
    wxString extWithDot;

    wxString ext = ftInfo.GetExtensions()[iExtCount];

    wxCHECK_MSG( !ext.empty(), NULL,
                 wxT("Associate() needs non empty extension") );

    if ( ext[0u] != wxT('.') )
        extWithDot = wxT('.');
    extWithDot += ext;

            const wxString& filetypeOrig = ftInfo.GetShortDesc();

    wxRegKey key(wxRegKey::HKCU, CLASSES_ROOT_KEY + extWithDot);
    if ( !key.Exists() )
    {
                ok = key.Create();
        if ( ok )
        {

            if ( filetypeOrig.empty() )
            {
                                filetype << extWithDot.c_str() + 1 << wxT("_file");
            }
            else
            {
                                filetype = filetypeOrig;
            }

            key.SetValue(wxEmptyString, filetype);
        }
    }
    else
    {
                if (!filetypeOrig.empty())
        {
            filetype = filetypeOrig;
            key.SetValue(wxEmptyString, filetype);
        }
        else
        {
            key.QueryValue(wxEmptyString, filetype);
        }
    }

        const wxString& mimetype = ftInfo.GetMimeType();
    if ( !mimetype.empty() )
    {
                ok = key.SetValue(wxT("Content Type"), mimetype);

        if ( ok )
        {
                        wxString strKey = MIME_DATABASE_KEY;
            strKey << mimetype;
            wxRegKey keyMIME(wxRegKey::HKCU, CLASSES_ROOT_KEY + strKey);
            ok = keyMIME.Create();

            if ( ok )
            {
                                keyMIME.SetValue(wxT("Extension"), extWithDot);
            }
        }
    }


    
    for (iExtCount=1; iExtCount < ftInfo.GetExtensionsCount(); iExtCount++ )
    {
        ext = ftInfo.GetExtensions()[iExtCount];
        if ( ext[0u] != wxT('.') )
           extWithDot = wxT('.');
        extWithDot += ext;

        wxRegKey key2(wxRegKey::HKCU, CLASSES_ROOT_KEY + extWithDot);
        if ( !key2.Exists() )
            key2.Create();
        key2.SetValue(wxEmptyString, filetype);

                const wxString& mimetype2 = ftInfo.GetMimeType();
        if ( !mimetype2.empty() )
        {
                        ok = key2.SetValue(wxT("Content Type"), mimetype2);

            if ( ok )
            {
                                wxString strKey = MIME_DATABASE_KEY;
                strKey << mimetype2;
                wxRegKey keyMIME(wxRegKey::HKCU, CLASSES_ROOT_KEY + strKey);
                ok = keyMIME.Create();

                if ( ok )
                {
                                        keyMIME.SetValue(wxT("Extension"), extWithDot);
                }
            }
        }

    } 
            wxRegKey keyFT(wxRegKey::HKCU, CLASSES_ROOT_KEY + filetype);
    keyFT.Create();

    wxFileType *ft = CreateFileType(filetype, extWithDot);

    if (ft)
    {
        ft->m_impl->MSWSuppressNotifications(true);
        if (! ftInfo.GetOpenCommand ().empty() ) ft->SetCommand (ftInfo.GetOpenCommand (), wxT("open"  ) );
        if (! ftInfo.GetPrintCommand().empty() ) ft->SetCommand (ftInfo.GetPrintCommand(), wxT("print" ) );
                if (! ftInfo.GetDescription ().empty() ) ft->m_impl->SetDescription (ftInfo.GetDescription ()) ;
        if (! ftInfo.GetIconFile().empty() ) ft->SetDefaultIcon (ftInfo.GetIconFile(), ftInfo.GetIconIndex() );

        ft->m_impl->MSWSuppressNotifications(false);
        ft->m_impl->MSWNotifyShell();
    }

    return ft;
}

bool wxFileTypeImpl::SetCommand(const wxString& cmd,
                                const wxString& verb,
                                bool WXUNUSED(overwriteprompt))
{
    wxCHECK_MSG( !m_ext.empty() && !verb.empty(), false,
                 wxT("SetCommand() needs an extension and a verb") );

    if ( !EnsureExtKeyExists() )
        return false;

    wxRegKey rkey(wxRegKey::HKCU, CLASSES_ROOT_KEY + GetVerbPath(verb));

                bool result = rkey.Create() && rkey.SetValue(wxEmptyString, cmd + wxT(" \"%1\"") );

    if ( result )
        MSWNotifyShell();

    return result;
}

bool wxFileTypeImpl::SetDefaultIcon(const wxString& cmd, int index)
{
    wxCHECK_MSG( !m_ext.empty(), false, wxT("SetDefaultIcon() needs extension") );
    wxCHECK_MSG( !m_strFileType.empty(), false, wxT("File key not found") );

    if ( !EnsureExtKeyExists() )
        return false;

    wxRegKey rkey(wxRegKey::HKCU,
                  CLASSES_ROOT_KEY + m_strFileType + wxT("\\DefaultIcon"));

    bool result = rkey.Create() &&
           rkey.SetValue(wxEmptyString,
                         wxString::Format(wxT("%s,%d"), cmd.c_str(), index));

    if ( result )
        MSWNotifyShell();

    return result;
}

bool wxFileTypeImpl::SetDescription (const wxString& desc)
{
    wxCHECK_MSG( !m_strFileType.empty(), false, wxT("File key not found") );
    wxCHECK_MSG( !desc.empty(), false, wxT("No file description supplied") );

    if ( !EnsureExtKeyExists() )
        return false;

    wxRegKey rkey(wxRegKey::HKCU, CLASSES_ROOT_KEY + m_strFileType );

    return rkey.Create() &&
           rkey.SetValue(wxEmptyString, desc);
}


bool wxFileTypeImpl::Unassociate()
{
    MSWSuppressNotifications(true);
    bool result = true;
    if ( !RemoveOpenCommand() )
        result = false;
    if ( !RemoveDefaultIcon() )
        result = false;
    if ( !RemoveMimeType() )
        result = false;
    if ( !RemoveDescription() )
        result = false;

    MSWSuppressNotifications(false);
    MSWNotifyShell();

    return result;
}

bool wxFileTypeImpl::RemoveOpenCommand()
{
   return RemoveCommand(wxT("open"));
}

bool wxFileTypeImpl::RemoveCommand(const wxString& verb)
{
    wxCHECK_MSG( !m_ext.empty() && !verb.empty(), false,
                 wxT("RemoveCommand() needs an extension and a verb") );

    wxRegKey rkey(wxRegKey::HKCU, CLASSES_ROOT_KEY + GetVerbPath(verb));

        bool result = !rkey.Exists() || rkey.DeleteSelf();

    if ( result )
        MSWNotifyShell();

    return result;
}

bool wxFileTypeImpl::RemoveMimeType()
{
    wxCHECK_MSG( !m_ext.empty(), false, wxT("RemoveMimeType() needs extension") );

    wxRegKey rkey(wxRegKey::HKCU, CLASSES_ROOT_KEY + m_ext);
    return !rkey.Exists() || rkey.DeleteSelf();
}

bool wxFileTypeImpl::RemoveDefaultIcon()
{
    wxCHECK_MSG( !m_ext.empty(), false,
                 wxT("RemoveDefaultIcon() needs extension") );

    wxRegKey rkey (wxRegKey::HKCU,
                   CLASSES_ROOT_KEY + m_strFileType  + wxT("\\DefaultIcon"));
    bool result = !rkey.Exists() || rkey.DeleteSelf();

    if ( result )
        MSWNotifyShell();

    return result;
}

bool wxFileTypeImpl::RemoveDescription()
{
    wxCHECK_MSG( !m_ext.empty(), false,
                 wxT("RemoveDescription() needs extension") );

    wxRegKey rkey (wxRegKey::HKCU, CLASSES_ROOT_KEY + m_strFileType );
    return !rkey.Exists() || rkey.DeleteSelf();
}

#endif 