


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_FONTMAP

#include "wx/fontmap.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/log.h"
    #include "wx/intl.h"
    #include "wx/msgdlg.h"
    #include "wx/choicdlg.h"
#endif 
#if wxUSE_CONFIG
    #include "wx/config.h"
#endif 
#if defined(__WXMSW__)
  #include  "wx/msw/private.h"    #include  "wx/msw/winundef.h"
#endif

#include "wx/fmappriv.h"
#include "wx/fontutil.h"
#include "wx/fontdlg.h"
#include "wx/encinfo.h"

#include "wx/encconv.h"


wxBEGIN_ENUM( wxFontEncoding )
wxENUM_MEMBER( wxFONTENCODING_SYSTEM )
wxENUM_MEMBER( wxFONTENCODING_DEFAULT )

wxENUM_MEMBER( wxFONTENCODING_ISO8859_1 )
wxENUM_MEMBER( wxFONTENCODING_ISO8859_2 )
wxENUM_MEMBER( wxFONTENCODING_ISO8859_3 )
wxENUM_MEMBER( wxFONTENCODING_ISO8859_4 )
wxENUM_MEMBER( wxFONTENCODING_ISO8859_5 )
wxENUM_MEMBER( wxFONTENCODING_ISO8859_6 )
wxENUM_MEMBER( wxFONTENCODING_ISO8859_7 )
wxENUM_MEMBER( wxFONTENCODING_ISO8859_8 )
wxENUM_MEMBER( wxFONTENCODING_ISO8859_9 )
wxENUM_MEMBER( wxFONTENCODING_ISO8859_10 )
wxENUM_MEMBER( wxFONTENCODING_ISO8859_11 )
wxENUM_MEMBER( wxFONTENCODING_ISO8859_12 )
wxENUM_MEMBER( wxFONTENCODING_ISO8859_13 )
wxENUM_MEMBER( wxFONTENCODING_ISO8859_14 )
wxENUM_MEMBER( wxFONTENCODING_ISO8859_15 )
wxENUM_MEMBER( wxFONTENCODING_ISO8859_MAX )
wxENUM_MEMBER( wxFONTENCODING_KOI8 )
wxENUM_MEMBER( wxFONTENCODING_KOI8_U )
wxENUM_MEMBER( wxFONTENCODING_ALTERNATIVE )
wxENUM_MEMBER( wxFONTENCODING_BULGARIAN )
wxENUM_MEMBER( wxFONTENCODING_CP437 )
wxENUM_MEMBER( wxFONTENCODING_CP850 )
wxENUM_MEMBER( wxFONTENCODING_CP852 )
wxENUM_MEMBER( wxFONTENCODING_CP855 )
wxENUM_MEMBER( wxFONTENCODING_CP866 )

wxENUM_MEMBER( wxFONTENCODING_CP874 )
wxENUM_MEMBER( wxFONTENCODING_CP932 )
wxENUM_MEMBER( wxFONTENCODING_CP936 )
wxENUM_MEMBER( wxFONTENCODING_CP949 )
wxENUM_MEMBER( wxFONTENCODING_CP950 )
wxENUM_MEMBER( wxFONTENCODING_CP1250 )
wxENUM_MEMBER( wxFONTENCODING_CP1251 )
wxENUM_MEMBER( wxFONTENCODING_CP1252 )
wxENUM_MEMBER( wxFONTENCODING_CP1253 )
wxENUM_MEMBER( wxFONTENCODING_CP1254 )
wxENUM_MEMBER( wxFONTENCODING_CP1255 )
wxENUM_MEMBER( wxFONTENCODING_CP1256 )
wxENUM_MEMBER( wxFONTENCODING_CP1257 )
wxENUM_MEMBER( wxFONTENCODING_CP1258 )
wxENUM_MEMBER( wxFONTENCODING_CP1361 )
wxENUM_MEMBER( wxFONTENCODING_CP12_MAX )
wxENUM_MEMBER( wxFONTENCODING_UTF7 )
wxENUM_MEMBER( wxFONTENCODING_UTF8 )
wxENUM_MEMBER( wxFONTENCODING_GB2312 )
wxENUM_MEMBER( wxFONTENCODING_BIG5 )
wxENUM_MEMBER( wxFONTENCODING_SHIFT_JIS )
wxENUM_MEMBER( wxFONTENCODING_EUC_JP )
wxENUM_MEMBER( wxFONTENCODING_UNICODE )
wxEND_ENUM( wxFontEncoding )


#if wxUSE_CONFIG

static const wxChar* FONTMAPPER_FONT_FROM_ENCODING_PATH = wxT("Encodings");
static const wxChar* FONTMAPPER_FONT_DONT_ASK = wxT("none");

#endif 

class ReentrancyBlocker
{
public:
    ReentrancyBlocker(bool& flag) : m_flagOld(flag), m_flag(flag)
        { m_flag = true; }
    ~ReentrancyBlocker() { m_flag = m_flagOld; }

private:
    bool m_flagOld;
    bool& m_flag;

    wxDECLARE_NO_COPY_CLASS(ReentrancyBlocker);
};



wxFontMapper::wxFontMapper()
{
    m_windowParent = NULL;
}

wxFontMapper::~wxFontMapper()
{
}


wxFontMapper *wxFontMapper::Get()
{
    wxFontMapperBase *fontmapper = wxFontMapperBase::Get();
    wxASSERT_MSG( !fontmapper->IsDummy(),
                 wxT("GUI code requested a wxFontMapper but we only have a wxFontMapperBase.") );

                return (wxFontMapper *)fontmapper;
}

wxFontEncoding
wxFontMapper::CharsetToEncoding(const wxString& charset, bool interactive)
{
        int encoding = wxFontMapperBase::NonInteractiveCharsetToEncoding(charset);

        if ( encoding == wxFONTENCODING_UNKNOWN )
    {
                        encoding = wxFONTENCODING_SYSTEM;
    }
#if wxUSE_CHOICEDLG
    else if ( (encoding == wxFONTENCODING_SYSTEM) && interactive )
    {
        
                wxString title(m_titleDialog);
        if ( !title )
            title << wxTheApp->GetAppDisplayName() << _(": unknown charset");

                wxString msg;
        msg.Printf(_("The charset '%s' is unknown. You may select\nanother charset to replace it with or choose\n[Cancel] if it cannot be replaced"), charset);

                const size_t count = GetSupportedEncodingsCount();

        wxString *encodingNamesTranslated = new wxString[count];

        for ( size_t i = 0; i < count; i++ )
        {
            encodingNamesTranslated[i] = GetEncodingDescription(GetEncoding(i));
        }

                wxWindow *parent = m_windowParent;
        if ( !parent )
            parent = wxTheApp->GetTopWindow();

                int n = wxGetSingleChoiceIndex(msg, title,
                                       count,
                                       encodingNamesTranslated,
                                       parent);

        delete [] encodingNamesTranslated;

        if ( n != -1 )
        {
            encoding = GetEncoding(n);
        }

#if wxUSE_CONFIG && wxUSE_FILECONFIG
                wxFontMapperPathChanger path(this, FONTMAPPER_CHARSET_PATH);
        if ( path.IsOk() )
        {
            wxConfigBase *config = GetConfig();

                                    long value = n == -1 ? (long)wxFONTENCODING_UNKNOWN : (long)encoding;
            if ( !config->Write(charset, value) )
            {
                wxLogError(_("Failed to remember the encoding for the charset '%s'."), charset);
            }
        }
#endif     }
#else
    wxUnusedVar(interactive);
#endif 
    return (wxFontEncoding)encoding;
}


bool wxFontMapper::TestAltEncoding(const wxString& configEntry,
                                   wxFontEncoding encReplacement,
                                   wxNativeEncodingInfo *info)
{
    if ( wxGetNativeFontEncoding(encReplacement, info) &&
         wxTestFontEncoding(*info) )
    {
#if wxUSE_CONFIG && wxUSE_FILECONFIG
                wxFontMapperPathChanger path(this, FONTMAPPER_FONT_FROM_ENCODING_PATH);

        if ( path.IsOk() )
        {
            GetConfig()->Write(configEntry, info->ToString());
        }
#else
        wxUnusedVar(configEntry);
#endif         return true;
    }

    return false;
}

bool wxFontMapper::GetAltForEncoding(wxFontEncoding encoding,
                                     wxNativeEncodingInfo *info,
                                     const wxString& facename,
                                     bool interactive)
{
#if wxUSE_GUI
                        
            static bool s_inGetAltForEncoding = false;

    if ( interactive && s_inGetAltForEncoding )
        return false;

    ReentrancyBlocker blocker(s_inGetAltForEncoding);
#endif 
    wxCHECK_MSG( info, false, wxT("bad pointer in GetAltForEncoding") );

    info->facename = facename;

    if ( encoding == wxFONTENCODING_DEFAULT )
    {
        encoding = wxFont::GetDefaultEncoding();
    }

                    if ( encoding == wxFONTENCODING_SYSTEM )
    {
        wxLogFatalError(_("can't load any font, aborting"));

            }

    wxString configEntry,
             encName = GetEncodingName(encoding);
    if ( !facename.empty() )
    {
        configEntry = facename + wxT("_");
    }
    configEntry += encName;

#if wxUSE_CONFIG && wxUSE_FILECONFIG
        wxString fontinfo;
    wxFontMapperPathChanger path(this, FONTMAPPER_FONT_FROM_ENCODING_PATH);
    if ( path.IsOk() )
    {
        fontinfo = GetConfig()->Read(configEntry);
    }

                if ( fontinfo == FONTMAPPER_FONT_DONT_ASK )
    {
        interactive = false;
    }
    else     {
        if ( !fontinfo.empty() && !facename.empty() )
        {
                        fontinfo = GetConfig()->Read(encName);
        }

        if ( !fontinfo.empty() )
        {
            if ( info->FromString(fontinfo) )
            {
                if ( wxTestFontEncoding(*info) )
                {
                                        return true;
                }
                                            }
            else
            {
                wxLogDebug(wxT("corrupted config data: string '%s' is not a valid font encoding info"),
                           fontinfo);
            }
        }
            }
#endif 
            wxFontEncodingArray equiv = wxEncodingConverter::GetAllEquivalents(encoding);
    size_t count = equiv.GetCount();
    bool foundEquivEncoding = false;
    wxFontEncoding equivEncoding = wxFONTENCODING_SYSTEM;
    if ( count )
    {
        for ( size_t i = 0; i < count && !foundEquivEncoding; i++ )
        {
                        if ( equiv[i] == encoding )
                continue;

            if ( TestAltEncoding(configEntry, equiv[i], info) )
            {
                equivEncoding = equiv[i];

                foundEquivEncoding = true;
            }
        }
    }

    #if wxUSE_FONTDLG
    if ( interactive )
    {
        wxString title(m_titleDialog);
        if ( !title )
            title << wxTheApp->GetAppDisplayName() << _(": unknown encoding");

                wxString encDesc = GetEncodingDescription(encoding),
                 msg;
        if ( foundEquivEncoding )
        {
                        msg.Printf(_("No font for displaying text in encoding '%s' found,\nbut an alternative encoding '%s' is available.\nDo you want to use this encoding (otherwise you will have to choose another one)?"),
                       encDesc, GetEncodingDescription(equivEncoding));
        }
        else
        {
            msg.Printf(_("No font for displaying text in encoding '%s' found.\nWould you like to select a font to be used for this encoding\n(otherwise the text in this encoding will not be shown correctly)?"),
                       encDesc);
        }

                        int answer = foundEquivEncoding ? wxNO : wxYES;

        if ( wxMessageBox(msg, title,
                          wxICON_QUESTION | wxYES_NO,
                          m_windowParent) == answer )
        {
            wxFontData data;
            data.SetEncoding(encoding);
            data.EncodingInfo() = *info;
            wxFontDialog dialog(m_windowParent, data);
            if ( dialog.ShowModal() == wxID_OK )
            {
                wxFontData retData = dialog.GetFontData();

                *info = retData.EncodingInfo();
                info->encoding = retData.GetEncoding();

#if wxUSE_CONFIG && wxUSE_FILECONFIG
                                wxFontMapperPathChanger path2(this,
                                              FONTMAPPER_FONT_FROM_ENCODING_PATH);
                if ( path2.IsOk() )
                {
                    GetConfig()->Write(configEntry, info->ToString());
                }
#endif 
                return true;
            }
                    }
        else
        {
                                                #if wxUSE_CONFIG && wxUSE_FILECONFIG
            wxFontMapperPathChanger path2(this,
                                          FONTMAPPER_FONT_FROM_ENCODING_PATH);
            if ( path2.IsOk() )
            {
                GetConfig()->Write
                             (
                                configEntry,
                                foundEquivEncoding
                                    ? (const wxChar*)info->ToString().c_str()
                                    : FONTMAPPER_FONT_DONT_ASK
                             );
            }
#endif         }
    }
    #else
    wxUnusedVar(equivEncoding);
#endif 
    return foundEquivEncoding;
}

bool wxFontMapper::GetAltForEncoding(wxFontEncoding encoding,
                                     wxFontEncoding *encodingAlt,
                                     const wxString& facename,
                                     bool interactive)
{
    wxCHECK_MSG( encodingAlt, false,
                    wxT("wxFontEncoding::GetAltForEncoding(): NULL pointer") );

    wxNativeEncodingInfo info;
    if ( !GetAltForEncoding(encoding, &info, facename, interactive) )
        return false;

    *encodingAlt = info.encoding;

    return true;
}

bool wxFontMapper::IsEncodingAvailable(wxFontEncoding encoding,
                                       const wxString& facename)
{
    wxNativeEncodingInfo info;

    if ( !wxGetNativeFontEncoding(encoding, &info) )
        return false;

    info.facename = facename;
    return wxTestFontEncoding(info);
}

#endif 