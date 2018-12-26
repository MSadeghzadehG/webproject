


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_CLIPBOARD

#include "wx/clipbrd.h"

#ifndef WX_PRECOMP
    #include "wx/object.h"
    #include "wx/event.h"
    #include "wx/app.h"
    #include "wx/frame.h"
    #include "wx/bitmap.h"
    #include "wx/utils.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/dataobj.h"
#endif

#if wxUSE_METAFILE
    #include "wx/metafile.h"
#endif


#include <string.h>

#include "wx/msw/private.h"
#include "wx/msw/ole/oleutils.h"

#if wxUSE_WXDIB
    #include "wx/msw/dib.h"
#endif


#if wxUSE_OLE
        #define wxUSE_OLE_CLIPBOARD 1
#else         #define wxUSE_OLE_CLIPBOARD 0
#endif

#if wxUSE_OLE_CLIPBOARD
    #include <ole2.h>
#endif 


static bool gs_wxClipboardIsOpen = false;
static int gs_htmlcfid = 0;

bool wxOpenClipboard()
{
    wxCHECK_MSG( !gs_wxClipboardIsOpen, true, wxT("clipboard already opened.") );

    wxWindow *win = wxTheApp->GetTopWindow();
    if ( win )
    {
        gs_wxClipboardIsOpen = ::OpenClipboard((HWND)win->GetHWND()) != 0;

        if ( !gs_wxClipboardIsOpen )
        {
            wxLogSysError(_("Failed to open the clipboard."));
        }

        return gs_wxClipboardIsOpen;
    }
    else
    {
        wxLogDebug(wxT("Cannot open clipboard without a main window."));

        return false;
    }
}

bool wxCloseClipboard()
{
    wxCHECK_MSG( gs_wxClipboardIsOpen, false, wxT("clipboard is not opened") );

    gs_wxClipboardIsOpen = false;

    if ( ::CloseClipboard() == 0 )
    {
        wxLogSysError(_("Failed to close the clipboard."));

        return false;
    }

    return true;
}

bool wxEmptyClipboard()
{
    if ( ::EmptyClipboard() == 0 )
    {
        wxLogSysError(_("Failed to empty the clipboard."));

        return false;
    }

    return true;
}

bool wxIsClipboardOpened()
{
  return gs_wxClipboardIsOpen;
}

bool wxIsClipboardFormatAvailable(wxDataFormat dataFormat)
{
    wxDataFormat::NativeFormat cf = dataFormat.GetFormatId();
    if (cf == wxDF_HTML)
        cf = gs_htmlcfid;

    if ( ::IsClipboardFormatAvailable(cf) )
    {
                return true;
    }

        switch ( cf )
    {
                case CF_BITMAP:
            return ::IsClipboardFormatAvailable(CF_DIB) != 0;

#if wxUSE_ENH_METAFILE
        case CF_METAFILEPICT:
            return ::IsClipboardFormatAvailable(CF_ENHMETAFILE) != 0;
#endif 
        default:
            return false;
    }
}


bool wxSetClipboardData(wxDataFormat dataFormat,
                        const void *data,
                        int width, int height)
{
    HANDLE handle = 0; 
    switch (dataFormat)
    {
        case wxDF_BITMAP:
            {
                wxBitmap *bitmap = (wxBitmap *)data;

                HDC hdcMem = CreateCompatibleDC((HDC) NULL);
                HDC hdcSrc = CreateCompatibleDC((HDC) NULL);
                HBITMAP old = (HBITMAP)
                    ::SelectObject(hdcSrc, (HBITMAP)bitmap->GetHBITMAP());
                HBITMAP hBitmap = CreateCompatibleBitmap(hdcSrc,
                                                         bitmap->GetWidth(),
                                                         bitmap->GetHeight());
                if (!hBitmap)
                {
                    SelectObject(hdcSrc, old);
                    DeleteDC(hdcMem);
                    DeleteDC(hdcSrc);
                    return false;
                }

                HBITMAP old1 = (HBITMAP) SelectObject(hdcMem, hBitmap);
                BitBlt(hdcMem, 0, 0, bitmap->GetWidth(), bitmap->GetHeight(),
                       hdcSrc, 0, 0, SRCCOPY);

                                SelectObject(hdcMem, old1);

                                handle = ::SetClipboardData(CF_BITMAP, hBitmap);

                                SelectObject(hdcSrc, old);
                DeleteDC(hdcSrc);
                DeleteDC(hdcMem);
                break;
            }

#if wxUSE_WXDIB
        case wxDF_DIB:
            {
                wxBitmap *bitmap = (wxBitmap *)data;

                if ( bitmap && bitmap->IsOk() )
                {
                    wxDIB dib(*bitmap);
                    if ( dib.IsOk() )
                    {
                        handle = ::SetClipboardData(CF_DIB, dib.Detach());
                    }
                }
                break;
            }
#endif

            #if wxUSE_METAFILE && !defined(wxMETAFILE_IS_ENH)
        case wxDF_METAFILE:
            {
                wxMetafile *wxMF = (wxMetafile *)data;
                HANDLE data = GlobalAlloc(GHND, sizeof(METAFILEPICT) + 1);
                METAFILEPICT *mf = (METAFILEPICT *)GlobalLock(data);

                mf->mm = wxMF->GetWindowsMappingMode();
                mf->xExt = width;
                mf->yExt = height;
                mf->hMF = (HMETAFILE) wxMF->GetHMETAFILE();
                GlobalUnlock(data);
                wxMF->SetHMETAFILE((WXHANDLE) NULL);

                handle = SetClipboardData(CF_METAFILEPICT, data);
                break;
            }
#endif 
#if wxUSE_ENH_METAFILE
        case wxDF_ENHMETAFILE:
            {
                wxEnhMetaFile *emf = (wxEnhMetaFile *)data;
                wxEnhMetaFile emfCopy = *emf;

                handle = SetClipboardData(CF_ENHMETAFILE,
                                          (void *)emfCopy.GetHENHMETAFILE());
            }
            break;
#endif 
        case CF_SYLK:
        case CF_DIF:
        case CF_TIFF:
        case CF_PALETTE:
        default:
            {
                wxLogError(_("Unsupported clipboard format."));
                return false;
            }

        case wxDF_OEMTEXT:
            dataFormat = wxDF_TEXT;
            
        case wxDF_TEXT:
            {
                char *s = (char *)data;

                width = strlen(s) + 1;
                height = 1;
                DWORD l = (width * height);
                HANDLE hGlobalMemory = GlobalAlloc(GHND, l);
                if ( hGlobalMemory )
                {
                    LPSTR lpGlobalMemory = (LPSTR)GlobalLock(hGlobalMemory);

                    memcpy(lpGlobalMemory, s, l);

                    GlobalUnlock(hGlobalMemory);
                }

                handle = SetClipboardData(dataFormat, hGlobalMemory);
                break;
            }

        case wxDF_HTML:
            {
                char* html = (char *)data;

                                char *buf = new char [400 + strlen(html)];
                if(!buf) return false;

                                strcpy(buf,
                    "Version:0.9\r\n"
                    "StartHTML:00000000\r\n"
                    "EndHTML:00000000\r\n"
                    "StartFragment:00000000\r\n"
                    "EndFragment:00000000\r\n"
                    "<html><body>\r\n"
                    "<!--StartFragment -->\r\n");

                                strcat(buf, html);
                strcat(buf, "\r\n");
                                strcat(buf,
                    "<!--EndFragment-->\r\n"
                    "</body>\r\n"
                    "</html>");

                                                                                char *ptr = strstr(buf, "StartHTML");
                sprintf(ptr+10, "%08u", (unsigned)(strstr(buf, "<html>") - buf));
                *(ptr+10+8) = '\r';

                ptr = strstr(buf, "EndHTML");
                sprintf(ptr+8, "%08u", (unsigned)strlen(buf));
                *(ptr+8+8) = '\r';

                ptr = strstr(buf, "StartFragment");
                sprintf(ptr+14, "%08u", (unsigned)(strstr(buf, "<!--StartFrag") - buf));
                *(ptr+14+8) = '\r';

                ptr = strstr(buf, "EndFragment");
                sprintf(ptr+12, "%08u", (unsigned)(strstr(buf, "<!--EndFrag") - buf));
                *(ptr+12+8) = '\r';

                                
                                HGLOBAL hText = GlobalAlloc(GMEM_MOVEABLE |GMEM_DDESHARE, strlen(buf)+4);

                                ptr = (char *)GlobalLock(hText);
                strcpy(ptr, buf);
                GlobalUnlock(hText);

                handle = ::SetClipboardData(gs_htmlcfid, hText);

                                GlobalFree(hText);

                                delete [] buf;
                break;
            }
    }

    if ( handle == 0 )
    {
        wxLogSysError(_("Failed to set clipboard data."));

        return false;
    }

    return true;
}

void *wxGetClipboardData(wxDataFormat dataFormat, long *len)
{
    void *retval = NULL;

    switch ( dataFormat )
    {
        case wxDF_BITMAP:
            {
                BITMAP bm;
                HBITMAP hBitmap = (HBITMAP) GetClipboardData(CF_BITMAP);
                if (!hBitmap)
                    break;

                HDC hdcMem = CreateCompatibleDC((HDC) NULL);
                HDC hdcSrc = CreateCompatibleDC((HDC) NULL);

                HBITMAP old = (HBITMAP) ::SelectObject(hdcSrc, hBitmap);
                GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);

                HBITMAP hNewBitmap = CreateBitmapIndirect(&bm);

                if (!hNewBitmap)
                {
                    SelectObject(hdcSrc, old);
                    DeleteDC(hdcMem);
                    DeleteDC(hdcSrc);
                    break;
                }

                HBITMAP old1 = (HBITMAP) SelectObject(hdcMem, hNewBitmap);
                BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight,
                       hdcSrc, 0, 0, SRCCOPY);

                                SelectObject(hdcMem, old1);

                                SelectObject(hdcSrc, old);
                DeleteDC(hdcSrc);
                DeleteDC(hdcMem);

                                wxBitmap *wxBM = new wxBitmap;
                wxBM->SetHBITMAP((WXHBITMAP) hNewBitmap);
                wxBM->SetWidth(bm.bmWidth);
                wxBM->SetHeight(bm.bmHeight);
                wxBM->SetDepth(bm.bmPlanes);
                retval = wxBM;
                break;
            }
        case wxDF_METAFILE:
        case CF_SYLK:
        case CF_DIF:
        case CF_TIFF:
        case CF_PALETTE:
        case wxDF_DIB:
            wxLogError(_("Unsupported clipboard format."));
            return NULL;

        case wxDF_OEMTEXT:
            dataFormat = wxDF_TEXT;
            
        case wxDF_TEXT:
            {
                HANDLE hGlobalMemory = ::GetClipboardData(dataFormat);
                if (!hGlobalMemory)
                    break;

                DWORD hsize = ::GlobalSize(hGlobalMemory);
                if (len)
                    *len = hsize;

                char *s = new char[hsize];
                if (!s)
                    break;

                LPSTR lpGlobalMemory = (LPSTR) GlobalLock(hGlobalMemory);

                memcpy(s, lpGlobalMemory, hsize);

                GlobalUnlock(hGlobalMemory);

                retval = s;
                break;
            }

        default:
            {
                HANDLE hGlobalMemory = ::GetClipboardData(dataFormat);
                if ( !hGlobalMemory )
                    break;

                DWORD size = ::GlobalSize(hGlobalMemory);
                if ( len )
                    *len = size;

                void *buf = malloc(size);
                if ( !buf )
                    break;

                LPSTR lpGlobalMemory = (LPSTR) GlobalLock(hGlobalMemory);

                memcpy(buf, lpGlobalMemory, size);

                GlobalUnlock(hGlobalMemory);

                retval = buf;
                break;
            }
    }

    if ( !retval )
    {
        wxLogSysError(_("Failed to retrieve data from the clipboard."));
    }

    return retval;
}

wxDataFormat wxEnumClipboardFormats(wxDataFormat dataFormat)
{
  return (wxDataFormat::NativeFormat)::EnumClipboardFormats(dataFormat);
}

int wxRegisterClipboardFormat(wxChar *formatName)
{
  return ::RegisterClipboardFormat(formatName);
}

bool wxGetClipboardFormatName(wxDataFormat dataFormat,
                              wxChar *formatName,
                              int maxCount)
{
  return ::GetClipboardFormatName((int)dataFormat, formatName, maxCount) > 0;
}


wxIMPLEMENT_DYNAMIC_CLASS(wxClipboard, wxObject);

wxClipboard::wxClipboard()
{
#if wxUSE_OLE_CLIPBOARD
    wxOleInitialize();
#endif

    m_lastDataObject = NULL;
    m_isOpened = false;
}

wxClipboard::~wxClipboard()
{
    if ( m_lastDataObject )
    {
        Clear();
    }

#if wxUSE_OLE_CLIPBOARD
    wxOleUninitialize();
#endif
}

void wxClipboard::Clear()
{
    if ( IsUsingPrimarySelection() )
        return;

#if wxUSE_OLE_CLIPBOARD
    if (m_lastDataObject)
    {
                HRESULT hr = OleIsCurrentClipboard(m_lastDataObject);
        if (S_OK == hr)
        {
            hr = OleSetClipboard(NULL);
            if ( FAILED(hr) )
            {
                wxLogApiError(wxT("OleSetClipboard(NULL)"), hr);
            }
        }
        m_lastDataObject = NULL;
    }
#endif }

bool wxClipboard::Flush()
{
#if wxUSE_OLE_CLIPBOARD
    if (m_lastDataObject)
    {
                HRESULT hr = OleIsCurrentClipboard(m_lastDataObject);
        m_lastDataObject = NULL;
        if (S_OK == hr)
        {
            hr = OleFlushClipboard();
            if ( FAILED(hr) )
            {
                wxLogApiError(wxT("OleFlushClipboard"), hr);

                return false;
            }
            return true;
        }
    }
    return false;
#else     return false;
#endif }

bool wxClipboard::Open()
{
        if(!gs_htmlcfid)
        gs_htmlcfid = RegisterClipboardFormat(wxT("HTML Format"));

        m_isOpened = true;
#if wxUSE_OLE_CLIPBOARD
    return true;
#else
    return wxOpenClipboard();
#endif
}

bool wxClipboard::IsOpened() const
{
#if wxUSE_OLE_CLIPBOARD
    return m_isOpened;
#else
    return wxIsClipboardOpened();
#endif
}

bool wxClipboard::SetData( wxDataObject *data )
{
    if ( IsUsingPrimarySelection() )
        return false;

#if !wxUSE_OLE_CLIPBOARD
    (void)wxEmptyClipboard();
#endif 
    if ( data )
        return AddData(data);
    else
        return true;
}

bool wxClipboard::AddData( wxDataObject *data )
{
    if ( IsUsingPrimarySelection() )
        return false;

    wxCHECK_MSG( data, false, wxT("data is invalid") );

#if wxUSE_OLE_CLIPBOARD
    HRESULT hr = OleSetClipboard(data->GetInterface());
    if ( FAILED(hr) )
    {
        wxLogSysError(hr, _("Failed to put data on the clipboard"));

        
        return false;
    }

                    m_lastDataObject = data->GetInterface();

                        data->SetAutoDelete();

    return true;
#elif wxUSE_DATAOBJ
    wxCHECK_MSG( wxIsClipboardOpened(), false, wxT("clipboard not open") );

    wxDataFormat format = data->GetPreferredFormat();

    switch ( format )
    {
        case wxDF_TEXT:
        case wxDF_OEMTEXT:
        {
            wxTextDataObject* textDataObject = (wxTextDataObject*) data;
            wxString str(textDataObject->GetText());
            return wxSetClipboardData(format, str.c_str());
        }

        case wxDF_BITMAP:
        case wxDF_DIB:
        {
            wxBitmapDataObject* bitmapDataObject = (wxBitmapDataObject*) data;
            wxBitmap bitmap(bitmapDataObject->GetBitmap());
            return wxSetClipboardData(data->GetPreferredFormat(), &bitmap);
        }

#if wxUSE_METAFILE
        case wxDF_METAFILE:
        {
#if 1
                        wxLogError(wxT("Not implemented because wxMetafileDataObject does not contain width and height values."));
            return false;
#else
            wxMetafileDataObject* metaFileDataObject =
                (wxMetafileDataObject*) data;
            wxMetafile metaFile = metaFileDataObject->GetMetafile();
            return wxSetClipboardData(wxDF_METAFILE, &metaFile,
                                      metaFileDataObject->GetWidth(),
                                      metaFileDataObject->GetHeight());
#endif
        }
#endif 
        default:
        {
                        wxLogError(wxT("Not implemented."));
            return false;
        }
    }
#else     return false;
#endif }

void wxClipboard::Close()
{
    m_isOpened = false;
    #if !wxUSE_OLE_CLIPBOARD
    wxCloseClipboard();
#endif
}

bool wxClipboard::IsSupported( const wxDataFormat& format )
{
    return !IsUsingPrimarySelection() && wxIsClipboardFormatAvailable(format);
}

bool wxClipboard::GetData( wxDataObject& data )
{
    if ( IsUsingPrimarySelection() )
        return false;

#if wxUSE_OLE_CLIPBOARD
    IDataObject *pDataObject = NULL;
    HRESULT hr = OleGetClipboard(&pDataObject);
    if ( FAILED(hr) || !pDataObject )
    {
        wxLogSysError(hr, _("Failed to get data from the clipboard"));

        return false;
    }

        size_t nFormats = data.GetFormatCount(wxDataObject::Set);
    wxDataFormat format;
    wxDataFormat *formats;
    if ( nFormats == 1 )
    {
                formats = &format;
    }
    else
    {
                formats = new wxDataFormat[nFormats];
    }

    data.GetAllFormats(formats, wxDataObject::Set);

        FORMATETC formatEtc;
    CLIPFORMAT cf;
    bool result = false;

            #if wxDEBUG_LEVEL >= 2
        IEnumFORMATETC *pEnumFormatEtc = NULL;
    hr = pDataObject->EnumFormatEtc(DATADIR_GET, &pEnumFormatEtc);
    if ( FAILED(hr) || !pEnumFormatEtc )
    {
        wxLogSysError(hr,
                      _("Failed to retrieve the supported clipboard formats"));
    }
    else
    {
                for ( ;; )
        {
            ULONG nCount;
            hr = pEnumFormatEtc->Next(1, &formatEtc, &nCount);

                        if ( hr != S_OK )
            {
                                break;
            }

            cf = formatEtc.cfFormat;

            wxLogTrace(wxTRACE_OleCalls,
                       wxT("Object on the clipboard supports format %s."),
                       wxDataObject::GetFormatName(cf));
        }

        pEnumFormatEtc->Release();
    }
#endif 
    STGMEDIUM medium;
        for ( size_t n = 0; !result && (n < nFormats); n++ )
    {
                cf = formats[n].GetFormatId();

        if (cf == wxDF_HTML)
            cf = gs_htmlcfid;
                        if ( !::IsClipboardFormatAvailable(cf) )
            continue;

        formatEtc.cfFormat = cf;
        formatEtc.ptd      = NULL;
        formatEtc.dwAspect = DVASPECT_CONTENT;
        formatEtc.lindex   = -1;

                switch ( formatEtc.cfFormat )
        {
            case CF_BITMAP:
                formatEtc.tymed = TYMED_GDI;
                break;

            case CF_METAFILEPICT:
                formatEtc.tymed = TYMED_MFPICT;
                break;

            case CF_ENHMETAFILE:
                formatEtc.tymed = TYMED_ENHMF;
                break;

            default:
                formatEtc.tymed = TYMED_HGLOBAL;
        }

                hr = pDataObject->GetData(&formatEtc, &medium);
        if ( FAILED(hr) )
        {
                        if ( formatEtc.cfFormat == CF_BITMAP )
            {
                formatEtc.tymed = TYMED_HGLOBAL;
                hr = pDataObject->GetData(&formatEtc, &medium);
            }
        }

        if ( SUCCEEDED(hr) )
        {
                        hr = data.GetInterface()->SetData(&formatEtc, &medium, true);
            if ( FAILED(hr) )
            {
                wxLogDebug(wxT("Failed to set data in wxIDataObject"));

                                                ReleaseStgMedium(&medium);
            }
            else
            {
                result = true;
            }
        }
            }

    if ( formats != &format )
    {
        delete [] formats;
    }
    
        pDataObject->Release();

    return result;
#elif wxUSE_DATAOBJ
    wxCHECK_MSG( wxIsClipboardOpened(), false, wxT("clipboard not open") );

    wxDataFormat format = data.GetPreferredFormat();
    switch ( format )
    {
        case wxDF_TEXT:
        case wxDF_OEMTEXT:
        {
            wxTextDataObject& textDataObject = (wxTextDataObject &)data;
            char* s = (char*)wxGetClipboardData(format);
            if ( !s )
                return false;

            textDataObject.SetText(wxString::FromAscii(s));
            delete [] s;

            return true;
        }

        case wxDF_BITMAP:
        case wxDF_DIB:
        {
            wxBitmapDataObject& bitmapDataObject = (wxBitmapDataObject &)data;
            wxBitmap* bitmap = (wxBitmap *)wxGetClipboardData(data.GetPreferredFormat());
            if ( !bitmap )
                return false;

            bitmapDataObject.SetBitmap(*bitmap);
            delete bitmap;

            return true;
        }
#if wxUSE_METAFILE
        case wxDF_METAFILE:
        {
            wxMetafileDataObject& metaFileDataObject = (wxMetafileDataObject &)data;
            wxMetafile* metaFile = (wxMetafile *)wxGetClipboardData(wxDF_METAFILE);
            if ( !metaFile )
                return false;

            metaFileDataObject.SetMetafile(*metaFile);
            delete metaFile;

            return true;
        }
#endif     }
    return false;
#else     wxFAIL_MSG( wxT("no clipboard implementation") );
    return false;
#endif }

#endif 