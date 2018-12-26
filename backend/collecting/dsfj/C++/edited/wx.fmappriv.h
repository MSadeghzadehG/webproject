
#ifndef _WX_FMAPPRIV_H_
#define _WX_FMAPPRIV_H_


enum { wxFONTENCODING_UNKNOWN = -2 };

#if wxUSE_CONFIG

#define FONTMAPPER_ROOT_PATH wxT("/wxWindows/FontMapper")
#define FONTMAPPER_CHARSET_PATH wxT("Charsets")
#define FONTMAPPER_CHARSET_ALIAS_PATH wxT("Aliases")

#endif 

#if wxUSE_CONFIG && wxUSE_FILECONFIG

class wxFontMapperPathChanger
{
public:
    wxFontMapperPathChanger(wxFontMapperBase *fontMapper, const wxString& path)
    {
        m_fontMapper = fontMapper;
        m_ok = m_fontMapper->ChangePath(path, &m_pathOld);
    }

    bool IsOk() const { return m_ok; }

    ~wxFontMapperPathChanger()
    {
        if ( IsOk() )
            m_fontMapper->RestorePath(m_pathOld);
    }

private:
        wxFontMapperBase *m_fontMapper;

        wxString m_pathOld;

        bool m_ok;


    wxDECLARE_NO_COPY_CLASS(wxFontMapperPathChanger);
};

#endif 
#endif 
