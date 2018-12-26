
#ifndef _WX_STRCONV_H_
#define _WX_STRCONV_H_

#include "wx/defs.h"
#include "wx/chartype.h"
#include "wx/buffer.h"

#include <stdlib.h>

class WXDLLIMPEXP_FWD_BASE wxString;

#define wxCONV_FAILED ((size_t)-1)


class WXDLLIMPEXP_BASE wxMBConv
{
public:
                                                                                                        virtual size_t ToWChar(wchar_t *dst, size_t dstLen,
                           const char *src, size_t srcLen = wxNO_LEN) const;

    virtual size_t FromWChar(char *dst, size_t dstLen,
                             const wchar_t *src, size_t srcLen = wxNO_LEN) const;


                const wxWCharBuffer cMB2WC(const char *in) const;
    const wxCharBuffer cWC2MB(const wchar_t *in) const;

                                                                const wxWCharBuffer
        cMB2WC(const char *in, size_t inLen, size_t *outLen) const;
    const wxCharBuffer
        cWC2MB(const wchar_t *in, size_t inLen, size_t *outLen) const;

                const wxWCharBuffer cMB2WC(const wxScopedCharBuffer& in) const;
    const wxCharBuffer cWC2MB(const wxScopedWCharBuffer& in) const;

    #if wxUSE_UNICODE
    const wxWCharBuffer cMB2WX(const char *psz) const { return cMB2WC(psz); }
    const wxCharBuffer cWX2MB(const wchar_t *psz) const { return cWC2MB(psz); }
    const wchar_t* cWC2WX(const wchar_t *psz) const { return psz; }
    const wchar_t* cWX2WC(const wchar_t *psz) const { return psz; }
#else     const char* cMB2WX(const char *psz) const { return psz; }
    const char* cWX2MB(const char *psz) const { return psz; }
    const wxCharBuffer cWC2WX(const wchar_t *psz) const { return cWC2MB(psz); }
    const wxWCharBuffer cWX2WC(const char *psz) const { return cMB2WC(psz); }
#endif 
                                                virtual size_t GetMBNulLen() const { return 1; }

            static size_t GetMaxMBNulLen() { return 4 ; }

#if wxUSE_UNICODE_UTF8
                virtual bool IsUTF8() const { return false; }
#endif

                                                                    virtual size_t MB2WC(wchar_t *out, const char *in, size_t outLen) const;
    virtual size_t WC2MB(char *out, const wchar_t *in, size_t outLen) const;


        virtual wxMBConv *Clone() const = 0;

        virtual ~wxMBConv();
};


class WXDLLIMPEXP_BASE wxMBConvLibc : public wxMBConv
{
public:
    virtual size_t MB2WC(wchar_t *outputBuf, const char *psz, size_t outputSize) const wxOVERRIDE;
    virtual size_t WC2MB(char *outputBuf, const wchar_t *psz, size_t outputSize) const wxOVERRIDE;

    virtual wxMBConv *Clone() const wxOVERRIDE { return new wxMBConvLibc; }

#if wxUSE_UNICODE_UTF8
    virtual bool IsUTF8() const wxOVERRIDE { return wxLocaleIsUtf8; }
#endif
};

#ifdef __UNIX__


class WXDLLIMPEXP_BASE wxConvBrokenFileNames : public wxMBConv
{
public:
    wxConvBrokenFileNames(const wxString& charset);
    wxConvBrokenFileNames(const wxConvBrokenFileNames& conv)
        : wxMBConv(),
          m_conv(conv.m_conv ? conv.m_conv->Clone() : NULL)
    {
    }
    virtual ~wxConvBrokenFileNames() { delete m_conv; }

    virtual size_t MB2WC(wchar_t *out, const char *in, size_t outLen) const wxOVERRIDE
    {
        return m_conv->MB2WC(out, in, outLen);
    }

    virtual size_t WC2MB(char *out, const wchar_t *in, size_t outLen) const wxOVERRIDE
    {
        return m_conv->WC2MB(out, in, outLen);
    }

    virtual size_t GetMBNulLen() const wxOVERRIDE
    {
                return m_conv->GetMBNulLen();
    }

#if wxUSE_UNICODE_UTF8
    virtual bool IsUTF8() const wxOVERRIDE { return m_conv->IsUTF8(); }
#endif

    virtual wxMBConv *Clone() const wxOVERRIDE { return new wxConvBrokenFileNames(*this); }

private:
        wxMBConv *m_conv;

    wxDECLARE_NO_ASSIGN_CLASS(wxConvBrokenFileNames);
};

#endif 

class WXDLLIMPEXP_BASE wxMBConvUTF7 : public wxMBConv
{
public:
    wxMBConvUTF7() { }

        
    virtual size_t ToWChar(wchar_t *dst, size_t dstLen,
                           const char *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;
    virtual size_t FromWChar(char *dst, size_t dstLen,
                             const wchar_t *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;

    virtual wxMBConv *Clone() const wxOVERRIDE { return new wxMBConvUTF7; }

private:
            struct StateMode
    {
        enum Mode
        {
            Direct,                 Shifted             };
    };

                class DecoderState : private StateMode
    {
    private:
                        Mode mode;

    public:
                DecoderState() { mode = Direct; }

                void ToDirect() { mode = Direct; }
        void ToShifted() { mode = Shifted; accum = bit = 0; isLSB = false; }

        bool IsDirect() const { return mode == Direct; }
        bool IsShifted() const { return mode == Shifted; }


        
        unsigned int accum;         unsigned int bit;           unsigned char msb;          bool isLSB;             };

    DecoderState m_stateDecoder;


            class EncoderState : private StateMode
    {
    private:
        Mode mode;

    public:
        EncoderState() { mode = Direct; }

        void ToDirect() { mode = Direct; }
        void ToShifted() { mode = Shifted; accum = bit = 0; }

        bool IsDirect() const { return mode == Direct; }
        bool IsShifted() const { return mode == Shifted; }

        unsigned int accum;
        unsigned int bit;
    };

    EncoderState m_stateEncoder;
};


class WXDLLIMPEXP_BASE wxMBConvStrictUTF8 : public wxMBConv
{
public:
    
    virtual size_t ToWChar(wchar_t *dst, size_t dstLen,
                           const char *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;
    virtual size_t FromWChar(char *dst, size_t dstLen,
                             const wchar_t *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;

    virtual wxMBConv *Clone() const wxOVERRIDE { return new wxMBConvStrictUTF8(); }

#if wxUSE_UNICODE_UTF8
            virtual bool IsUTF8() const wxOVERRIDE { return true; }
#endif
};

class WXDLLIMPEXP_BASE wxMBConvUTF8 : public wxMBConvStrictUTF8
{
public:
    enum
    {
        MAP_INVALID_UTF8_NOT = 0,
        MAP_INVALID_UTF8_TO_PUA = 1,
        MAP_INVALID_UTF8_TO_OCTAL = 2
    };

    wxMBConvUTF8(int options = MAP_INVALID_UTF8_NOT) : m_options(options) { }

    virtual size_t ToWChar(wchar_t *dst, size_t dstLen,
                           const char *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;
    virtual size_t FromWChar(char *dst, size_t dstLen,
                             const wchar_t *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;

    virtual wxMBConv *Clone() const wxOVERRIDE { return new wxMBConvUTF8(m_options); }

#if wxUSE_UNICODE_UTF8
            virtual bool IsUTF8() const wxOVERRIDE { return m_options == MAP_INVALID_UTF8_NOT; }
#endif

private:
    int m_options;
};


class WXDLLIMPEXP_BASE wxMBConvUTF16Base : public wxMBConv
{
public:
    enum { BYTES_PER_CHAR = 2 };

    virtual size_t GetMBNulLen() const wxOVERRIDE { return BYTES_PER_CHAR; }

protected:
                    static size_t GetLength(const char *src, size_t srcLen);
};


class WXDLLIMPEXP_BASE wxMBConvUTF16LE : public wxMBConvUTF16Base
{
public:
    virtual size_t ToWChar(wchar_t *dst, size_t dstLen,
                           const char *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;
    virtual size_t FromWChar(char *dst, size_t dstLen,
                             const wchar_t *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;
    virtual wxMBConv *Clone() const wxOVERRIDE { return new wxMBConvUTF16LE; }
};


class WXDLLIMPEXP_BASE wxMBConvUTF16BE : public wxMBConvUTF16Base
{
public:
    virtual size_t ToWChar(wchar_t *dst, size_t dstLen,
                           const char *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;
    virtual size_t FromWChar(char *dst, size_t dstLen,
                             const wchar_t *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;
    virtual wxMBConv *Clone() const wxOVERRIDE { return new wxMBConvUTF16BE; }
};


class WXDLLIMPEXP_BASE wxMBConvUTF32Base : public wxMBConv
{
public:
    enum { BYTES_PER_CHAR = 4 };

    virtual size_t GetMBNulLen() const wxOVERRIDE { return BYTES_PER_CHAR; }

protected:
                static size_t GetLength(const char *src, size_t srcLen);
};


class WXDLLIMPEXP_BASE wxMBConvUTF32LE : public wxMBConvUTF32Base
{
public:
    virtual size_t ToWChar(wchar_t *dst, size_t dstLen,
                           const char *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;
    virtual size_t FromWChar(char *dst, size_t dstLen,
                             const wchar_t *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;
    virtual wxMBConv *Clone() const wxOVERRIDE { return new wxMBConvUTF32LE; }
};


class WXDLLIMPEXP_BASE wxMBConvUTF32BE : public wxMBConvUTF32Base
{
public:
    virtual size_t ToWChar(wchar_t *dst, size_t dstLen,
                           const char *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;
    virtual size_t FromWChar(char *dst, size_t dstLen,
                             const wchar_t *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;
    virtual wxMBConv *Clone() const wxOVERRIDE { return new wxMBConvUTF32BE; }
};


#include "wx/fontenc.h"

class WXDLLIMPEXP_BASE wxCSConv : public wxMBConv
{
public:
            wxCSConv(const wxString& charset);
    wxCSConv(wxFontEncoding encoding);

    wxCSConv(const wxCSConv& conv);
    virtual ~wxCSConv();

    wxCSConv& operator=(const wxCSConv& conv);

    virtual size_t ToWChar(wchar_t *dst, size_t dstLen,
                           const char *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;
    virtual size_t FromWChar(char *dst, size_t dstLen,
                             const wchar_t *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;
    virtual size_t GetMBNulLen() const wxOVERRIDE;

#if wxUSE_UNICODE_UTF8
    virtual bool IsUTF8() const wxOVERRIDE;
#endif

    virtual wxMBConv *Clone() const wxOVERRIDE { return new wxCSConv(*this); }

    void Clear();

        bool IsOk() const;

private:
        void Init();

            wxMBConv *DoCreate() const;

            void SetName(const char *charset);

                        void SetEncoding(wxFontEncoding encoding);


                                    char *m_name;
    wxFontEncoding m_encoding;

            wxMBConv *m_convReal;
};


class WXDLLIMPEXP_BASE wxWhateverWorksConv : public wxMBConv
{
public:
    wxWhateverWorksConv()
    {
    }

                        virtual size_t
    ToWChar(wchar_t *dst, size_t dstLen,
            const char *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;

                    virtual size_t
    FromWChar(char *dst, size_t dstLen,
              const wchar_t *src, size_t srcLen = wxNO_LEN) const wxOVERRIDE;

    virtual wxMBConv *Clone() const wxOVERRIDE
    {
        return new wxWhateverWorksConv();
    }
};


#define WX_DECLARE_GLOBAL_CONV(klass, name)                             \
    extern WXDLLIMPEXP_DATA_BASE(klass*) name##Ptr;                     \
    extern WXDLLIMPEXP_BASE klass* wxGet_##name##Ptr();                 \
    inline klass& wxGet_##name()                                        \
    {                                                                   \
        if ( !name##Ptr )                                               \
            name##Ptr = wxGet_##name##Ptr();                            \
        return *name##Ptr;                                              \
    }


WX_DECLARE_GLOBAL_CONV(wxMBConv, wxConvLibc)
#define wxConvLibc wxGet_wxConvLibc()

WX_DECLARE_GLOBAL_CONV(wxCSConv, wxConvISO8859_1)
#define wxConvISO8859_1 wxGet_wxConvISO8859_1()

WX_DECLARE_GLOBAL_CONV(wxMBConvStrictUTF8, wxConvUTF8)
#define wxConvUTF8 wxGet_wxConvUTF8()

WX_DECLARE_GLOBAL_CONV(wxMBConvUTF7, wxConvUTF7)
#define wxConvUTF7 wxGet_wxConvUTF7()

WX_DECLARE_GLOBAL_CONV(wxWhateverWorksConv, wxConvWhateverWorks)
#define wxConvWhateverWorks wxGet_wxConvWhateverWorks()

extern WXDLLIMPEXP_DATA_BASE(wxMBConv *) wxConvFileName;

#define wxConvFile (*wxConvFileName)

extern WXDLLIMPEXP_DATA_BASE(wxMBConv *) wxConvCurrent;

WX_DECLARE_GLOBAL_CONV(wxCSConv, wxConvLocal)
#define wxConvLocal wxGet_wxConvLocal()

extern WXDLLIMPEXP_DATA_BASE(wxMBConv *) wxConvUI;

#undef WX_DECLARE_GLOBAL_CONV


#ifdef WORDS_BIGENDIAN
    typedef wxMBConvUTF16BE wxMBConvUTF16;
    typedef wxMBConvUTF32BE wxMBConvUTF32;
#else
    typedef wxMBConvUTF16LE wxMBConvUTF16;
    typedef wxMBConvUTF32LE wxMBConvUTF32;
#endif


#if wxMBFILES && wxUSE_UNICODE
    #define wxFNCONV(name) wxConvFileName->cWX2MB(name)
    #define wxFNSTRINGCAST wxMBSTRINGCAST
#else
#if defined(__WXOSX__) && wxMBFILES
    #define wxFNCONV(name) wxConvFileName->cWC2MB( wxConvLocal.cWX2WC(name) )
#else
    #define wxFNCONV(name) name
#endif
    #define wxFNSTRINGCAST WXSTRINGCAST
#endif


#if wxUSE_UNICODE
    #define wxConvertWX2MB(s)   wxConvCurrent->cWX2MB(s)
    #define wxConvertMB2WX(s)   wxConvCurrent->cMB2WX(s)

                
    inline wxWCharBuffer wxSafeConvertMB2WX(const char *s)
    {
        return wxConvWhateverWorks.cMB2WC(s);
    }

    inline wxCharBuffer wxSafeConvertWX2MB(const wchar_t *ws)
    {
        return wxConvWhateverWorks.cWC2MB(ws);
    }
#else         #define wxConvertWX2MB(s)   (s)
    #define wxConvertMB2WX(s)   (s)
    #define wxSafeConvertMB2WX(s) (s)
    #define wxSafeConvertWX2MB(s) (s)
#endif 
#endif 
