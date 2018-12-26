


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/fontutil.h"

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/log.h"
    #include "wx/intl.h"
    #include "wx/wxcrtvararg.h"
    #include "wx/msw/private.h"
#endif 
#include "wx/encinfo.h"
#include "wx/fontmap.h"
#include "wx/tokenzr.h"

#ifndef HANGUL_CHARSET
#    define HANGUL_CHARSET  129
#endif




bool wxNativeEncodingInfo::FromString(const wxString& s)
{
    wxStringTokenizer tokenizer(s, wxT(";"));

    wxString encid = tokenizer.GetNextToken();

            long enc;
    if ( encid.ToLong(&enc) )
    {
                if ( enc < 0 || enc >= wxFONTENCODING_MAX )
            return false;

        encoding = (wxFontEncoding)enc;
    }
    else     {
#if wxUSE_FONTMAP
        encoding = wxFontMapper::GetEncodingFromName(encid);
        if ( encoding == wxFONTENCODING_MAX )
#endif         {
                        return false;
        }
    }

    facename = tokenizer.GetNextToken();

    wxString tmp = tokenizer.GetNextToken();
    if ( tmp.empty() )
    {
                                charset = ANSI_CHARSET;
    }
    else
    {
        if ( wxSscanf(tmp, wxT("%u"), &charset) != 1 )
        {
                        return false;
        }
    }

    return true;
}

wxString wxNativeEncodingInfo::ToString() const
{
    wxString s;

    s
#if wxUSE_FONTMAP
                        << wxFontMapper::GetEncodingName(encoding)
#else             << (long)encoding
#endif       << wxT(';') << facename;

        if ( charset != ANSI_CHARSET )
    {
         s << wxT(';') << charset;
    }

    return s;
}


bool wxGetNativeFontEncoding(wxFontEncoding encoding,
                             wxNativeEncodingInfo *info)
{
    wxCHECK_MSG( info, false, wxT("bad pointer in wxGetNativeFontEncoding") );

    if ( encoding == wxFONTENCODING_DEFAULT )
    {
        encoding = wxFont::GetDefaultEncoding();
    }

    extern WXDLLIMPEXP_BASE long wxEncodingToCharset(wxFontEncoding encoding);
    info->charset = wxEncodingToCharset(encoding);
    if ( info->charset == -1 )
        return false;

    info->encoding = encoding;

    return true;
}

bool wxTestFontEncoding(const wxNativeEncodingInfo& info)
{
        LOGFONT lf;
    wxZeroMemory(lf);       
    lf.lfCharSet = (BYTE)info.charset;
    wxStrlcpy(lf.lfFaceName, info.facename.c_str(), WXSIZEOF(lf.lfFaceName));

    HFONT hfont = ::CreateFontIndirect(&lf);
    if ( !hfont )
    {
                return false;
    }

    ::DeleteObject((HGDIOBJ)hfont);

    return true;
}


wxFontEncoding wxGetFontEncFromCharSet(int cs)
{
    wxFontEncoding fontEncoding;

    switch ( cs )
    {
        default:
            wxFAIL_MSG( wxT("unexpected Win32 charset") );
            
        case DEFAULT_CHARSET:
            fontEncoding = wxFONTENCODING_SYSTEM;
            break;

        case ANSI_CHARSET:
            fontEncoding = wxFONTENCODING_CP1252;
            break;

        case SYMBOL_CHARSET:
                        fontEncoding = wxFONTENCODING_MAX;
            break;

        case EASTEUROPE_CHARSET:
            fontEncoding = wxFONTENCODING_CP1250;
            break;

        case BALTIC_CHARSET:
            fontEncoding = wxFONTENCODING_CP1257;
            break;

        case RUSSIAN_CHARSET:
            fontEncoding = wxFONTENCODING_CP1251;
            break;

        case ARABIC_CHARSET:
            fontEncoding = wxFONTENCODING_CP1256;
            break;

        case GREEK_CHARSET:
            fontEncoding = wxFONTENCODING_CP1253;
            break;

        case HEBREW_CHARSET:
            fontEncoding = wxFONTENCODING_CP1255;
            break;

        case TURKISH_CHARSET:
            fontEncoding = wxFONTENCODING_CP1254;
            break;

        case THAI_CHARSET:
            fontEncoding = wxFONTENCODING_CP874;
            break;

        case SHIFTJIS_CHARSET:
            fontEncoding = wxFONTENCODING_CP932;
            break;

        case GB2312_CHARSET:
            fontEncoding = wxFONTENCODING_CP936;
            break;

        case HANGUL_CHARSET:
            fontEncoding = wxFONTENCODING_CP949;
            break;

        case CHINESEBIG5_CHARSET:
            fontEncoding = wxFONTENCODING_CP950;
            break;

        case VIETNAMESE_CHARSET:
            fontEncoding = wxFONTENCODING_CP1258;
            break;

        case JOHAB_CHARSET:
            fontEncoding = wxFONTENCODING_CP1361;
            break;

        case MAC_CHARSET:
            fontEncoding = wxFONTENCODING_MACROMAN;
            break;

        case OEM_CHARSET:
            fontEncoding = wxFONTENCODING_CP437;
            break;
    }

    return fontEncoding;
}


void wxFillLogFont(LOGFONT *logFont, const wxFont *font)
{
    wxNativeFontInfo fi;

        const wxNativeFontInfo *pFI = font->GetNativeFontInfo();
    if ( !pFI )
    {
                fi.InitFromFont(*font);

        pFI = &fi;
    }

        *logFont = pFI->lf;
}

wxFont wxCreateFontFromLogFont(const LOGFONT *logFont)
{
    wxNativeFontInfo info;

    info.lf = *logFont;

    return wxFont(info);
}

