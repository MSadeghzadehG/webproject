
#ifndef _WX_FONTMAPPER_H_
#define _WX_FONTMAPPER_H_


#if wxUSE_FONTMAP

#include "wx/fontenc.h"         
#if wxUSE_GUI
    #include "wx/fontutil.h"    #endif 
#if wxUSE_CONFIG && wxUSE_FILECONFIG
    class WXDLLIMPEXP_FWD_BASE wxConfigBase;
#endif 
class WXDLLIMPEXP_FWD_CORE wxFontMapper;

#if wxUSE_GUI
    class WXDLLIMPEXP_FWD_CORE wxWindow;
#endif 


class WXDLLIMPEXP_BASE wxFontMapperBase
{
public:
        
        wxFontMapperBase();

        virtual ~wxFontMapperBase();

                    static wxFontMapperBase *Get();

        static wxFontMapper *Set(wxFontMapper *mapper);

        static void Reset();


        
                        virtual wxFontEncoding CharsetToEncoding(const wxString& charset,
                                             bool interactive = true);

        
        static size_t GetSupportedEncodingsCount();

        static wxFontEncoding GetEncoding(size_t n);

            static wxString GetEncodingName(wxFontEncoding encoding);

        static const wxChar** GetAllEncodingNames(wxFontEncoding encoding);

                static wxString GetEncodingDescription(wxFontEncoding encoding);

                    static wxFontEncoding GetEncodingFromName(const wxString& name);


                    
#if wxUSE_CONFIG && wxUSE_FILECONFIG
        void SetConfigPath(const wxString& prefix);

        static const wxString& GetDefaultConfigPath();
#endif 

            virtual bool IsDummy() { return true; }

protected:
#if wxUSE_CONFIG && wxUSE_FILECONFIG
            wxConfigBase *GetConfig();

            const wxString& GetConfigPath();

                            bool ChangePath(const wxString& pathNew, wxString *pathOld);

        void RestorePath(const wxString& pathOld);

        wxConfigBase *m_configDummy;

    wxString m_configRootPath;
#endif 
                        int NonInteractiveCharsetToEncoding(const wxString& charset);

private:
        static wxFontMapper *sm_instance;

    friend class wxFontMapperPathChanger;

    wxDECLARE_NO_COPY_CLASS(wxFontMapperBase);
};


#if wxUSE_GUI

class WXDLLIMPEXP_CORE wxFontMapper : public wxFontMapperBase
{
public:
        wxFontMapper();

        virtual ~wxFontMapper();

        
            virtual wxFontEncoding CharsetToEncoding(const wxString& charset,
                                             bool interactive = true);

                    virtual bool GetAltForEncoding(wxFontEncoding encoding,
                                   wxNativeEncodingInfo *info,
                                   const wxString& facename = wxEmptyString,
                                   bool interactive = true);

            bool GetAltForEncoding(wxFontEncoding encoding,
                           wxFontEncoding *alt_encoding,
                           const wxString& facename = wxEmptyString,
                           bool interactive = true);

                    virtual bool IsEncodingAvailable(wxFontEncoding encoding,
                                     const wxString& facename = wxEmptyString);


        
        void SetDialogParent(wxWindow *parent) { m_windowParent = parent; }

        void SetDialogTitle(const wxString& title) { m_titleDialog = title; }

            static wxFontMapper *Get();

        virtual bool IsDummy() { return false; }

protected:
                                bool TestAltEncoding(const wxString& configEntry,
                         wxFontEncoding encReplacement,
                         wxNativeEncodingInfo *info);

        wxString m_titleDialog;

        wxWindow *m_windowParent;

private:
    wxDECLARE_NO_COPY_CLASS(wxFontMapper);
};

#endif 

#define wxTheFontMapper (wxFontMapper::Get())

#else 
#if wxUSE_GUI
        #include "wx/fontutil.h"
#endif

#endif 
#endif 
