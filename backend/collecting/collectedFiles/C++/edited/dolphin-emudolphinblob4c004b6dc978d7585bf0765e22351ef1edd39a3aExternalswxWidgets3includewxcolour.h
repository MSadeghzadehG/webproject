
#ifndef _WX_COLOUR_H_BASE_
#define _WX_COLOUR_H_BASE_

#include "wx/defs.h"
#include "wx/gdiobj.h"

class WXDLLIMPEXP_FWD_CORE wxColour;

#define DEFINE_STD_WXCOLOUR_CONSTRUCTORS                                      \
    wxColour() { Init(); }                                                    \
    wxColour(ChannelType red,                                                 \
             ChannelType green,                                               \
             ChannelType blue,                                                \
             ChannelType alpha = wxALPHA_OPAQUE)                              \
        { Init(); Set(red, green, blue, alpha); }                             \
    wxColour(unsigned long colRGB) { Init(); Set(colRGB    ); }               \
    wxColour(const wxString& colourName) { Init(); Set(colourName); }         \
    wxColour(const char *colourName) { Init(); Set(colourName); }             \
    wxColour(const wchar_t *colourName) { Init(); Set(colourName); }


enum {
    wxC2S_NAME             = 1,       wxC2S_CSS_SYNTAX       = 2,       wxC2S_HTML_SYNTAX      = 4    };

const unsigned char wxALPHA_TRANSPARENT = 0;
const unsigned char wxALPHA_OPAQUE = 0xff;

#define wxTransparentColour wxColour(0, 0, 0, wxALPHA_TRANSPARENT)
#define wxTransparentColor wxTransparentColour


#if wxUSE_VARIANT
#include "wx/variant.h"
DECLARE_VARIANT_OBJECT_EXPORTED(wxColour,WXDLLIMPEXP_CORE)
#endif



#if defined( __WXMAC__ ) || defined( __WXMSW__ )
#define wxCOLOUR_IS_GDIOBJECT 0
#else
#define wxCOLOUR_IS_GDIOBJECT 1
#endif

class WXDLLIMPEXP_CORE wxColourBase : public
#if wxCOLOUR_IS_GDIOBJECT
    wxGDIObject
#else
    wxObject
#endif
{
public:
        typedef unsigned char ChannelType;

    wxColourBase() {}
    virtual ~wxColourBase() {}


        
    void Set(ChannelType red,
             ChannelType green,
             ChannelType blue,
             ChannelType alpha = wxALPHA_OPAQUE)
        { InitRGBA(red, green, blue, alpha); }

        bool Set(const wxString &str)
        { return FromString(str); }

    void Set(unsigned long colRGB)
    {
                        Set((ChannelType)(0xFF & colRGB),
            (ChannelType)(0xFF & (colRGB >> 8)),
            (ChannelType)(0xFF & (colRGB >> 16)));
    }



        
    virtual ChannelType Red() const = 0;
    virtual ChannelType Green() const = 0;
    virtual ChannelType Blue() const = 0;
    virtual ChannelType Alpha() const
        { return wxALPHA_OPAQUE ; }

        virtual wxString GetAsString(long flags = wxC2S_NAME | wxC2S_CSS_SYNTAX) const;

    void SetRGB(wxUint32 colRGB)
    {
        Set((ChannelType)(0xFF & colRGB),
            (ChannelType)(0xFF & (colRGB >> 8)),
            (ChannelType)(0xFF & (colRGB >> 16)));
    }

    void SetRGBA(wxUint32 colRGBA)
    {
        Set((ChannelType)(0xFF & colRGBA),
            (ChannelType)(0xFF & (colRGBA >> 8)),
            (ChannelType)(0xFF & (colRGBA >> 16)),
            (ChannelType)(0xFF & (colRGBA >> 24)));
    }

    wxUint32 GetRGB() const
        { return Red() | (Green() << 8) | (Blue() << 16); }

    wxUint32 GetRGBA() const
        { return Red() | (Green() << 8) | (Blue() << 16) | (Alpha() << 24); }

#if !wxCOLOUR_IS_GDIOBJECT
    virtual bool IsOk() const= 0;

            bool Ok() const { return IsOk(); }
#endif

        
        
    static void          MakeMono    (unsigned char* r, unsigned char* g, unsigned char* b, bool on);
    static void          MakeDisabled(unsigned char* r, unsigned char* g, unsigned char* b, unsigned char brightness = 255);
    static void          MakeGrey    (unsigned char* r, unsigned char* g, unsigned char* b);     static void          MakeGrey    (unsigned char* r, unsigned char* g, unsigned char* b,
                                      double weight_r, double weight_g, double weight_b);     static unsigned char AlphaBlend  (unsigned char fg, unsigned char bg, double alpha);
    static void          ChangeLightness(unsigned char* r, unsigned char* g, unsigned char* b, int ialpha);

    wxColour ChangeLightness(int ialpha) const;
    wxColour& MakeDisabled(unsigned char brightness = 255);

protected:
            void Init() { }

    virtual void
    InitRGBA(ChannelType r, ChannelType g, ChannelType b, ChannelType a) = 0;

    virtual bool FromString(const wxString& s);

#if wxCOLOUR_IS_GDIOBJECT
                virtual wxGDIRefData *CreateGDIRefData() const wxOVERRIDE
    {
        wxFAIL_MSG( "must be overridden if used" );

        return NULL;
    }

    virtual wxGDIRefData *CloneGDIRefData(const wxGDIRefData *WXUNUSED(data)) const wxOVERRIDE
    {
        wxFAIL_MSG( "must be overridden if used" );

        return NULL;
    }
#endif
};


WXDLLIMPEXP_CORE wxString wxToString(const wxColourBase& col);
WXDLLIMPEXP_CORE bool wxFromString(const wxString& str, wxColourBase* col);



#if defined(__WXMSW__)
    #include "wx/msw/colour.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/colour.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/colour.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/colour.h"
#elif defined(__WXDFB__)
    #include "wx/generic/colour.h"
#elif defined(__WXX11__)
    #include "wx/x11/colour.h"
#elif defined(__WXMAC__)
    #include "wx/osx/colour.h"
#elif defined(__WXQT__)
    #include "wx/qt/colour.h"
#endif

#define wxColor wxColour

#endif 