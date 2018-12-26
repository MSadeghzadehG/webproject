


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/convauto.h"

wxFontEncoding wxConvAuto::ms_defaultMBEncoding = wxFONTENCODING_ISO8859_1;

namespace
{

const char BOM_UTF32BE[] = { '\x00', '\x00', '\xFE', '\xFF' };
const char BOM_UTF32LE[] = { '\xFF', '\xFE', '\x00', '\x00' };
const char BOM_UTF16BE[] = { '\xFE', '\xFF'                 };
const char BOM_UTF16LE[] = { '\xFF', '\xFE'                 };
const char BOM_UTF8[]    = { '\xEF', '\xBB', '\xBF'         };

} 


void wxConvAuto::SetFallbackEncoding(wxFontEncoding enc)
{
    wxASSERT_MSG( enc != wxFONTENCODING_DEFAULT,
                  wxT("wxFONTENCODING_DEFAULT doesn't make sense here") );

    ms_defaultMBEncoding = enc;
}


const char* wxConvAuto::GetBOMChars(wxBOM bom, size_t* count)
{
    wxCHECK_MSG( count , NULL, wxS("count pointer must be provided") );

    switch ( bom )
    {
        case wxBOM_UTF32BE: *count = WXSIZEOF(BOM_UTF32BE); return BOM_UTF32BE;
        case wxBOM_UTF32LE: *count = WXSIZEOF(BOM_UTF32LE); return BOM_UTF32LE;
        case wxBOM_UTF16BE: *count = WXSIZEOF(BOM_UTF16BE); return BOM_UTF16BE;
        case wxBOM_UTF16LE: *count = WXSIZEOF(BOM_UTF16LE); return BOM_UTF16LE;
        case wxBOM_UTF8   : *count = WXSIZEOF(BOM_UTF8   ); return BOM_UTF8;
        case wxBOM_Unknown:
        case wxBOM_None:
            wxFAIL_MSG( wxS("Invalid BOM type") );
            return NULL;
    }

    wxFAIL_MSG( wxS("Unknown BOM type") );
    return NULL;
}


wxBOM wxConvAuto::DetectBOM(const char *src, size_t srcLen)
{
                                                        
    switch ( srcLen )
    {
        case 0:
            return wxBOM_Unknown;

        case 1:
            if ( src[0] == '\x00' || src[0] == '\xFF' ||
                 src[0] == '\xFE' || src[0] == '\xEF')
            {
                                return wxBOM_Unknown;
            }
            break;

        case 2:
        case 3:
            if ( src[0] == '\xEF' && src[1] == '\xBB' )
            {
                if ( srcLen == 3 )
                    return src[2] == '\xBF' ? wxBOM_UTF8 : wxBOM_None;

                return wxBOM_Unknown;
            }

            if ( src[0] == '\xFE' && src[1] == '\xFF' )
                return wxBOM_UTF16BE;

            if ( src[0] == '\xFF' && src[1] == '\xFE' )
            {
                                                if ( srcLen == 3 && src[2] != '\x00' )
                    return wxBOM_UTF16LE;

                return wxBOM_Unknown;
            }

            if ( src[0] == '\x00' && src[1] == '\x00' )
            {
                                                if ( srcLen == 3 && src[2] != '\xFE' )
                    return wxBOM_None;

                return wxBOM_Unknown;
            }
            break;

        default:
                                    if ( src[0] == '\xEF' && src[1] == '\xBB' && src[2] == '\xBF' )
                return wxBOM_UTF8;

            if ( src[0] == '\x00' && src[1] == '\x00' &&
                 src[2] == '\xFE' && src[3] == '\xFF' )
                return wxBOM_UTF32BE;

            if ( src[0] == '\xFF' && src[1] == '\xFE' &&
                 src[2] == '\x00' && src[3] == '\x00' )
                return wxBOM_UTF32LE;

            if ( src[0] == '\xFE' && src[1] == '\xFF' )
                return wxBOM_UTF16BE;

            if ( src[0] == '\xFF' && src[1] == '\xFE' )
                return wxBOM_UTF16LE;
    }

    return wxBOM_None;
}

void wxConvAuto::InitFromBOM(wxBOM bomType)
{
    m_consumedBOM = false;

    switch ( bomType )
    {
        case wxBOM_Unknown:
            wxFAIL_MSG( "shouldn't be called for this BOM type" );
            break;

        case wxBOM_None:
                        break;

        case wxBOM_UTF32BE:
            m_conv = new wxMBConvUTF32BE;
            m_ownsConv = true;
            break;

        case wxBOM_UTF32LE:
            m_conv = new wxMBConvUTF32LE;
            m_ownsConv = true;
            break;

        case wxBOM_UTF16BE:
            m_conv = new wxMBConvUTF16BE;
            m_ownsConv = true;
            break;

        case wxBOM_UTF16LE:
            m_conv = new wxMBConvUTF16LE;
            m_ownsConv = true;
            break;

        case wxBOM_UTF8:
            InitWithUTF8();
            break;

        default:
            wxFAIL_MSG( "unknown BOM type" );
    }

    if ( !m_conv )
    {
                                InitWithUTF8();
        m_consumedBOM = true;     }
}

void wxConvAuto::SkipBOM(const char **src, size_t *len) const
{
    int ofs;
    switch ( m_bomType )
    {
        case wxBOM_Unknown:
            wxFAIL_MSG( "shouldn't be called for this BOM type" );
            return;

        case wxBOM_None:
            ofs = 0;
            break;

        case wxBOM_UTF32BE:
        case wxBOM_UTF32LE:
            ofs = 4;
            break;

        case wxBOM_UTF16BE:
        case wxBOM_UTF16LE:
            ofs = 2;
            break;

        case wxBOM_UTF8:
            ofs = 3;
            break;

        default:
            wxFAIL_MSG( "unknown BOM type" );
            return;
    }

    *src += ofs;
    if ( *len != (size_t)-1 )
        *len -= ofs;
}

bool wxConvAuto::InitFromInput(const char *src, size_t len)
{
    m_bomType = DetectBOM(src, len == wxNO_LEN ? strlen(src) : len);
    if ( m_bomType == wxBOM_Unknown )
        return false;

    InitFromBOM(m_bomType);

    return true;
}

size_t
wxConvAuto::ToWChar(wchar_t *dst, size_t dstLen,
                    const char *src, size_t srcLen) const
{
                        wxConvAuto *self = const_cast<wxConvAuto *>(this);


    if ( !m_conv )
    {
        if ( !self->InitFromInput(src, srcLen) )
        {
                                                return wxCONV_FAILED;
        }
    }

    if ( !m_consumedBOM )
    {
        SkipBOM(&src, &srcLen);
        if ( srcLen == 0 )
        {
                                                                                                return wxCONV_FAILED;
        }
    }

        size_t rc = m_conv->ToWChar(dst, dstLen, src, srcLen);
    if ( rc == wxCONV_FAILED && m_bomType == wxBOM_None )
    {
                        if ( m_encDefault != wxFONTENCODING_MAX )
        {
            if ( m_ownsConv )
                delete m_conv;

            self->m_conv = new wxCSConv(m_encDefault == wxFONTENCODING_DEFAULT
                                            ? GetFallbackEncoding()
                                            : m_encDefault);
            self->m_ownsConv = true;

            rc = m_conv->ToWChar(dst, dstLen, src, srcLen);
        }
    }

        if ( rc != wxCONV_FAILED && dst && !m_consumedBOM )
        self->m_consumedBOM = true;

    return rc;
}

size_t
wxConvAuto::FromWChar(char *dst, size_t dstLen,
                      const wchar_t *src, size_t srcLen) const
{
    if ( !m_conv )
    {
                const_cast<wxConvAuto *>(this)->InitWithUTF8();
    }

    return m_conv->FromWChar(dst, dstLen, src, srcLen);
}
