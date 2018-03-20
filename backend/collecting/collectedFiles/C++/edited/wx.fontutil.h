

#ifndef _WX_FONTUTIL_H_
#define _WX_FONTUTIL_H_


#include "wx/font.h"        
#if defined(__WXMSW__)
    #include "wx/msw/wrapwin.h"
#endif

#if defined(__WXQT__)
    #include <QtGui/QFont>
#endif

class WXDLLIMPEXP_FWD_BASE wxArrayString;
struct WXDLLIMPEXP_FWD_CORE wxNativeEncodingInfo;

#if defined(_WX_X_FONTLIKE)

enum wxXLFDField
{
    wxXLFD_FOUNDRY,         wxXLFD_FAMILY,          wxXLFD_WEIGHT,          wxXLFD_SLANT,           wxXLFD_SETWIDTH,        wxXLFD_ADDSTYLE,        wxXLFD_PIXELSIZE,       wxXLFD_POINTSIZE,       wxXLFD_RESX,            wxXLFD_RESY,
    wxXLFD_SPACING,         wxXLFD_AVGWIDTH,        wxXLFD_REGISTRY,        wxXLFD_ENCODING,        wxXLFD_MAX
};

#endif 


class WXDLLIMPEXP_CORE wxNativeFontInfo
{
public:
#if wxUSE_PANGO
    PangoFontDescription *description;

            bool m_underlined;
    bool m_strikethrough;
#elif defined(_WX_X_FONTLIKE)
        private:
        wxString     fontElements[wxXLFD_MAX];

        wxString     xFontName;

        bool         m_isDefault;

        inline bool HasElements() const;

public:
        bool FromXFontName(const wxString& xFontName);

        bool IsDefault() const { return m_isDefault; }

        wxString GetXFontName() const;

        wxString GetXFontComponent(wxXLFDField field) const;

        void SetXFontComponent(wxXLFDField field, const wxString& value);

        void SetXFontName(const wxString& xFontName);
#elif defined(__WXMSW__)
    wxNativeFontInfo(const LOGFONT& lf_) : lf(lf_) { }

    LOGFONT      lf;
#elif defined(__WXOSX__)
public:
    wxNativeFontInfo(const wxNativeFontInfo& info) { Init(info); }
    wxNativeFontInfo( int size,
                  wxFontFamily family,
                  wxFontStyle style,
                  wxFontWeight weight,
                  bool underlined,
                  bool strikethrough,
                  const wxString& faceName,
                  wxFontEncoding encoding)
    {
        Init(size, family, style, weight,
             underlined, strikethrough,
             faceName, encoding);
    }

    ~wxNativeFontInfo() { Free(); }

    wxNativeFontInfo& operator=(const wxNativeFontInfo& info)
    {
        if (this != &info)
        {
            Free();
            Init(info);
        }
        return *this;
    }

    void Init(CTFontDescriptorRef descr);
    void Init(const wxNativeFontInfo& info);
    void Init(int size,
                  wxFontFamily family,
                  wxFontStyle style,
                  wxFontWeight weight,
                  bool underlined,
                  bool strikethrough,
                  const wxString& faceName ,
                  wxFontEncoding encoding);

    void Free();
    void EnsureValid();

    static void UpdateNamesMap(const wxString& familyname, CTFontDescriptorRef descr);
    static void UpdateNamesMap(const wxString& familyname, CTFontRef font);

    bool m_descriptorValid;

#if wxOSX_USE_ATSU_TEXT
    bool            m_atsuFontValid;
        wxUint32        m_atsuFontID;
        wxInt16         m_atsuAdditionalQDStyles;
#endif

    int           m_pointSize;
    wxFontFamily  m_family;
    wxFontStyle   m_style;
    wxFontWeight  m_weight;
    bool          m_underlined;
    bool          m_strikethrough;
    wxString      m_faceName;
    wxFontEncoding m_encoding;
public :
#elif defined(__WXQT__)
    QFont m_qtFont;
#else                     #define wxNO_NATIVE_FONTINFO

    int           pointSize;
    wxFontFamily  family;
    wxFontStyle   style;
    wxFontWeight  weight;
    bool          underlined;
    bool          strikethrough;
    wxString      faceName;
    wxFontEncoding encoding;
#endif 
        wxNativeFontInfo() { Init(); }

#if wxUSE_PANGO
private:
    void Init(const wxNativeFontInfo& info);
    void Free();

public:
    wxNativeFontInfo(const wxNativeFontInfo& info) { Init(info); }
    ~wxNativeFontInfo() { Free(); }

    wxNativeFontInfo& operator=(const wxNativeFontInfo& info)
    {
        if (this != &info)
        {
            Free();
            Init(info);
        }
        return *this;
    }
#endif 
        void Init();

        void InitFromFont(const wxFont& font)
    {
                SetStyle((wxFontStyle)font.GetStyle());
        SetWeight((wxFontWeight)font.GetWeight());
        SetUnderlined(font.GetUnderlined());
        SetStrikethrough(font.GetStrikethrough());
#if defined(__WXMSW__)
        if ( font.IsUsingSizeInPixels() )
            SetPixelSize(font.GetPixelSize());
        else
            SetPointSize(font.GetPointSize());
#else
        SetPointSize(font.GetPointSize());
#endif

                SetFamily((wxFontFamily)font.GetFamily());
        const wxString& facename = font.GetFaceName();
        if ( !facename.empty() )
        {
            SetFaceName(facename);
        }

                        SetEncoding(font.GetEncoding());
    }

        int GetPointSize() const;
    wxSize GetPixelSize() const;
    wxFontStyle GetStyle() const;
    wxFontWeight GetWeight() const;
    bool GetUnderlined() const;
    bool GetStrikethrough() const;
    wxString GetFaceName() const;
    wxFontFamily GetFamily() const;
    wxFontEncoding GetEncoding() const;

    void SetPointSize(int pointsize);
    void SetPixelSize(const wxSize& pixelSize);
    void SetStyle(wxFontStyle style);
    void SetWeight(wxFontWeight weight);
    void SetUnderlined(bool underlined);
    void SetStrikethrough(bool strikethrough);
    bool SetFaceName(const wxString& facename);
    void SetFamily(wxFontFamily family);
    void SetEncoding(wxFontEncoding encoding);

                    void SetFaceName(const wxArrayString &facenames);


            bool FromString(const wxString& s);
    wxString ToString() const;

                bool FromUserString(const wxString& s);
    wxString ToUserString() const;
};


WXDLLIMPEXP_CORE bool wxGetNativeFontEncoding(wxFontEncoding encoding,
                                              wxNativeEncodingInfo *info);

WXDLLIMPEXP_CORE bool wxTestFontEncoding(const wxNativeEncodingInfo& info);


#ifdef _WX_X_FONTLIKE
    #include "wx/unix/fontutil.h"
#endif 
#endif 