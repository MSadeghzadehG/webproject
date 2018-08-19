
#ifndef _WX_MSW_DIB_H_
#define _WX_MSW_DIB_H_

class WXDLLIMPEXP_FWD_CORE wxPalette;

#include "wx/msw/private.h"

#if wxUSE_WXDIB

#ifdef __WXMSW__
    #include "wx/bitmap.h"
#endif 

class WXDLLIMPEXP_CORE wxDIB
{
public:
        
                        wxDIB(int width, int height, int depth)
        { Init(); (void)Create(width, height, depth); }

#ifdef __WXMSW__
        wxDIB(const wxBitmap& bmp)
        { Init(); (void)Create(bmp); }
#endif 
        wxDIB(HBITMAP hbmp)
        { Init(); (void)Create(hbmp); }

                wxDIB(const wxString& filename)
        { Init(); (void)Load(filename); }

        bool Create(int width, int height, int depth);
#ifdef __WXMSW__
    bool Create(const wxBitmap& bmp) { return Create(GetHbitmapOf(bmp)); }
#endif
    bool Create(HBITMAP hbmp);
    bool Load(const wxString& filename);

        ~wxDIB();


        
                HBITMAP CreateDDB(HDC hdc = 0) const;

            HBITMAP Detach() { HBITMAP hbmp = m_handle; m_handle = 0; return hbmp; }

#if defined(__WXMSW__) && wxUSE_PALETTE
        wxPalette *CreatePalette() const;
#endif 
        bool Save(const wxString& filename);


        
        bool IsOk() const { return m_handle != 0; }

        wxSize GetSize() const { DoGetObject(); return wxSize(m_width, m_height); }
    int GetWidth() const { DoGetObject(); return m_width; }
    int GetHeight() const { DoGetObject(); return m_height; }

        int GetDepth() const { DoGetObject(); return m_depth; }

        HBITMAP GetHandle() const { return m_handle; }

            unsigned char *GetData() const
        { DoGetObject(); return (unsigned char *)m_data; }


        
        
                static HBITMAP ConvertToBitmap(const BITMAPINFO *pbi,
                                   HDC hdc = 0,
                                   void *bits = NULL);

            static HGLOBAL ConvertFromBitmap(HBITMAP hbmp);

                        static size_t ConvertFromBitmap(BITMAPINFO *pbi, HBITMAP hbmp);


        
#if wxUSE_IMAGE
        enum PixelFormat
    {
        PixelFormat_PreMultiplied = 0,
        PixelFormat_NotPreMultiplied = 1
    };

                                wxDIB(const wxImage& image, PixelFormat pf = PixelFormat_PreMultiplied)
    {
        Init();
        (void)Create(image, pf);
    }

        bool Create(const wxImage& image, PixelFormat pf = PixelFormat_PreMultiplied);

    
        enum ConversionFlags
    {
                        Convert_AlphaAuto,
                        Convert_AlphaAlwaysIf32bpp
    };
    wxImage ConvertToImage(ConversionFlags flags = Convert_AlphaAuto) const;
#endif 

        
                static unsigned long GetLineSize(int width, int depth)
    {
        return ((width*depth + 31) & ~31) >> 3;
    }

private:
        void Init();

        void Free();

            bool CopyFromDDB(HBITMAP hbmp);


        HBITMAP m_handle;

                        
            void DoGetObject() const;

        void *m_data;

        int m_width,
        m_height,
        m_depth;

                bool m_ownsHandle;


        wxDIB(const wxDIB&);
    wxDIB& operator=(const wxDIB&);
};


inline
void wxDIB::Init()
{
    m_handle = 0;
    m_ownsHandle = true;

    m_data = NULL;

    m_width =
    m_height =
    m_depth = 0;
}

inline
void wxDIB::Free()
{
    if ( m_handle && m_ownsHandle )
    {
        if ( !::DeleteObject(m_handle) )
        {
            wxLogLastError(wxT("DeleteObject(hDIB)"));
        }

        Init();
    }
}

inline wxDIB::~wxDIB()
{
    Free();
}

#endif
    
#endif 
