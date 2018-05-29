


#include "wx/wxprec.h"

#if defined(__BORLANDC__)
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/utils.h"
    #include "wx/wxcrtvararg.h"
#endif

#include "wx/dataobj.h"

#if wxUSE_OLE

#include "wx/scopedarray.h"
#include "wx/vector.h"
#include "wx/msw/private.h"         
#include <oleauto.h>
#include <shlobj.h>

#include "wx/msw/ole/oleutils.h"

#include "wx/msw/dib.h"

#ifndef CFSTR_SHELLURL
#define CFSTR_SHELLURL wxT("UniformResourceLocator")
#endif


#if wxDEBUG_LEVEL
    static const wxChar *GetTymedName(DWORD tymed);
#else     #define GetTymedName(tymed) wxEmptyString
#endif 
namespace
{

wxDataFormat HtmlFormatFixup(wxDataFormat format)
{
                                if ( !format.IsStandard() )
    {
        wxChar szBuf[256];
        if ( ::GetClipboardFormatName(format, szBuf, WXSIZEOF(szBuf)) )
        {
            if ( wxStrcmp(szBuf, wxT("HTML Format")) == 0 )
                format = wxDF_HTML;
        }
    }

    return format;
}

HGLOBAL wxGlobalClone(HGLOBAL hglobIn)
{
    HGLOBAL hglobOut = NULL;

    LPVOID pvIn = GlobalLock(hglobIn);
    if (pvIn)
    {
        SIZE_T cb = GlobalSize(hglobIn);
        hglobOut = GlobalAlloc(GMEM_FIXED, cb);
        if (hglobOut)
        {
            CopyMemory(hglobOut, pvIn, cb);
        }
        GlobalUnlock(hglobIn);
    }

    return hglobOut;
}

HRESULT wxCopyStgMedium(const STGMEDIUM *pmediumIn, STGMEDIUM *pmediumOut)
{
    HRESULT hres = S_OK;
    STGMEDIUM stgmOut = *pmediumIn;

    if (pmediumIn->pUnkForRelease == NULL &&
        !(pmediumIn->tymed & (TYMED_ISTREAM | TYMED_ISTORAGE)))
    {
                if (pmediumIn->tymed == TYMED_HGLOBAL)
        {
            stgmOut.hGlobal = wxGlobalClone(pmediumIn->hGlobal);
            if (!stgmOut.hGlobal)
            {
                hres = E_OUTOFMEMORY;
            }
        }
        else
        {
            hres = DV_E_TYMED;         }
    }

    if ( SUCCEEDED(hres) )
    {
        switch ( stgmOut.tymed )
        {
            case TYMED_ISTREAM:
                stgmOut.pstm->AddRef();
                break;

            case TYMED_ISTORAGE:
                stgmOut.pstg->AddRef();
                break;
        }

        if ( stgmOut.pUnkForRelease )
            stgmOut.pUnkForRelease->AddRef();

        *pmediumOut = stgmOut;
    }

    return hres;
}

} 

class wxIEnumFORMATETC : public IEnumFORMATETC
{
public:
    wxIEnumFORMATETC(const wxDataFormat* formats, ULONG nCount);
    virtual ~wxIEnumFORMATETC() { delete [] m_formats; }

        STDMETHODIMP Next(ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumFORMATETC **ppenum);

    DECLARE_IUNKNOWN_METHODS;

private:
    CLIPFORMAT *m_formats;      ULONG       m_nCount,                   m_nCurrent; 
    wxDECLARE_NO_COPY_CLASS(wxIEnumFORMATETC);
};


class wxIDataObject : public IDataObject
{
public:
    wxIDataObject(wxDataObject *pDataObject);
    virtual ~wxIDataObject();

                void SetDeleteFlag() { m_mustDelete = true; }

        STDMETHODIMP GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
    STDMETHODIMP GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium);
    STDMETHODIMP QueryGetData(FORMATETC *pformatetc);
    STDMETHODIMP GetCanonicalFormatEtc(FORMATETC *In, FORMATETC *pOut);
    STDMETHODIMP SetData(FORMATETC *pfetc, STGMEDIUM *pmedium, BOOL fRelease);
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFEtc);
    STDMETHODIMP DAdvise(FORMATETC *pfetc, DWORD ad, IAdviseSink *p, DWORD *pdw);
    STDMETHODIMP DUnadvise(DWORD dwConnection);
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA **ppenumAdvise);

    DECLARE_IUNKNOWN_METHODS;

private:
    wxDataObject *m_pDataObject;      
    bool m_mustDelete;

    wxDECLARE_NO_COPY_CLASS(wxIDataObject);

        
    class SystemDataEntry
    {
    public:
                SystemDataEntry(FORMATETC *pformatetc_, STGMEDIUM *pmedium_)
            : pformatetc(pformatetc_), pmedium(pmedium_)
        {
        }

        ~SystemDataEntry()
        {
            delete pformatetc;
            delete pmedium;
        }

        FORMATETC *pformatetc;
        STGMEDIUM *pmedium;
    };
    typedef wxVector<SystemDataEntry*> SystemData;

        bool GetSystemData(wxDataFormat format, STGMEDIUM*) const;

        bool HasSystemData(wxDataFormat format) const;

        HRESULT SaveSystemData(FORMATETC*, STGMEDIUM*, BOOL fRelease);

        SystemData m_systemData;
};

bool
wxIDataObject::GetSystemData(wxDataFormat format, STGMEDIUM *pmedium) const
{
    for ( SystemData::const_iterator it = m_systemData.begin();
          it != m_systemData.end();
          ++it )
    {
        FORMATETC* formatEtc = (*it)->pformatetc;
        if ( formatEtc->cfFormat == format )
        {
            wxCopyStgMedium((*it)->pmedium, pmedium);
            return true;
        }
    }

    return false;
}

bool
wxIDataObject::HasSystemData(wxDataFormat format) const
{
    for ( SystemData::const_iterator it = m_systemData.begin();
          it != m_systemData.end();
          ++it )
    {
        FORMATETC* formatEtc = (*it)->pformatetc;
        if ( formatEtc->cfFormat == format )
            return true;
    }

    return false;
}

HRESULT
wxIDataObject::SaveSystemData(FORMATETC *pformatetc,
                                 STGMEDIUM *pmedium,
                                 BOOL fRelease)
{
    if ( pformatetc == NULL || pmedium == NULL )
        return E_INVALIDARG;

        for ( SystemData::iterator it = m_systemData.begin();
          it != m_systemData.end();
          ++it )
    {
        if ( pformatetc->tymed & (*it)->pformatetc->tymed &&
             pformatetc->dwAspect == (*it)->pformatetc->dwAspect &&
             pformatetc->cfFormat == (*it)->pformatetc->cfFormat )
        {
            delete (*it);
            m_systemData.erase(it);
            break;
        }
    }

        FORMATETC* pnewformatEtc = new FORMATETC;
    STGMEDIUM* pnewmedium = new STGMEDIUM;

    wxZeroMemory(*pnewformatEtc);
    wxZeroMemory(*pnewmedium);

        *pnewformatEtc = *pformatetc;

        if ( fRelease )
        *pnewmedium = *pmedium;
    else
        wxCopyStgMedium(pmedium, pnewmedium);

        m_systemData.push_back(new SystemDataEntry(pnewformatEtc, pnewmedium));

    return S_OK;
}



bool wxDataFormat::operator==(wxDataFormatId format) const
{
    return HtmlFormatFixup(*this).m_format == (NativeFormat)format;
}

bool wxDataFormat::operator!=(wxDataFormatId format) const
{
    return !(*this == format);
}

bool wxDataFormat::operator==(const wxDataFormat& format) const
{
    return HtmlFormatFixup(*this).m_format == HtmlFormatFixup(format).m_format;
}

bool wxDataFormat::operator!=(const wxDataFormat& format) const
{
    return !(*this == format);
}

void wxDataFormat::SetId(const wxString& format)
{
    m_format = (wxDataFormat::NativeFormat)::RegisterClipboardFormat(format.t_str());
    if ( !m_format )
    {
        wxLogError(_("Couldn't register clipboard format '%s'."), format);
    }
}

wxString wxDataFormat::GetId() const
{
    static const int max = 256;

    wxString s;

    wxCHECK_MSG( !IsStandard(), s,
                 wxT("name of predefined format cannot be retrieved") );

    int len = ::GetClipboardFormatName(m_format, wxStringBuffer(s, max), max);

    if ( !len )
    {
        wxLogError(_("The clipboard format '%d' doesn't exist."), m_format);
        return wxEmptyString;
    }

    return s;
}


BEGIN_IID_TABLE(wxIEnumFORMATETC)
    ADD_IID(Unknown)
    ADD_IID(EnumFORMATETC)
END_IID_TABLE;

IMPLEMENT_IUNKNOWN_METHODS(wxIEnumFORMATETC)

wxIEnumFORMATETC::wxIEnumFORMATETC(const wxDataFormat *formats, ULONG nCount)
{
    m_nCurrent = 0;
    m_nCount = nCount;
    m_formats = new CLIPFORMAT[nCount];
    for ( ULONG n = 0; n < nCount; n++ ) {
        if (formats[n].GetFormatId() != wxDF_HTML)
            m_formats[n] = formats[n].GetFormatId();
        else
            m_formats[n] = ::RegisterClipboardFormat(wxT("HTML Format"));
    }
}

STDMETHODIMP wxIEnumFORMATETC::Next(ULONG      celt,
                                    FORMATETC *rgelt,
                                    ULONG     *pceltFetched)
{
    wxLogTrace(wxTRACE_OleCalls, wxT("wxIEnumFORMATETC::Next"));

    ULONG numFetched = 0;
    while (m_nCurrent < m_nCount && numFetched < celt) {
        FORMATETC format;
        format.cfFormat = m_formats[m_nCurrent++];
        format.ptd      = NULL;
        format.dwAspect = DVASPECT_CONTENT;
        format.lindex   = -1;
        format.tymed    = TYMED_HGLOBAL;

        *rgelt++ = format;
        numFetched++;
    }

    if (pceltFetched)
        *pceltFetched = numFetched;

    return numFetched == celt ? S_OK : S_FALSE;
}

STDMETHODIMP wxIEnumFORMATETC::Skip(ULONG celt)
{
    wxLogTrace(wxTRACE_OleCalls, wxT("wxIEnumFORMATETC::Skip"));

    m_nCurrent += celt;
    if ( m_nCurrent < m_nCount )
        return S_OK;

        m_nCurrent -= celt;

    return S_FALSE;
}

STDMETHODIMP wxIEnumFORMATETC::Reset()
{
    wxLogTrace(wxTRACE_OleCalls, wxT("wxIEnumFORMATETC::Reset"));

    m_nCurrent = 0;

    return S_OK;
}

STDMETHODIMP wxIEnumFORMATETC::Clone(IEnumFORMATETC **ppenum)
{
    wxLogTrace(wxTRACE_OleCalls, wxT("wxIEnumFORMATETC::Clone"));

        wxIEnumFORMATETC *pNew = new wxIEnumFORMATETC(NULL, 0);
    pNew->m_nCount = m_nCount;
    pNew->m_formats = new CLIPFORMAT[m_nCount];
    for ( ULONG n = 0; n < m_nCount; n++ ) {
        pNew->m_formats[n] = m_formats[n];
    }
    pNew->AddRef();
    *ppenum = pNew;

    return S_OK;
}


BEGIN_IID_TABLE(wxIDataObject)
    ADD_IID(Unknown)
    ADD_IID(DataObject)
END_IID_TABLE;

IMPLEMENT_IUNKNOWN_METHODS(wxIDataObject)

wxIDataObject::wxIDataObject(wxDataObject *pDataObject)
{
    m_pDataObject = pDataObject;
    m_mustDelete = false;
}

wxIDataObject::~wxIDataObject()
{
        for ( SystemData::iterator it = m_systemData.begin();
          it != m_systemData.end();
          ++it )
    {
        delete (*it);
    }

    if ( m_mustDelete )
    {
        delete m_pDataObject;
    }
}

STDMETHODIMP wxIDataObject::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    wxLogTrace(wxTRACE_OleCalls, wxT("wxIDataObject::GetData"));

        HRESULT hr = QueryGetData(pformatetcIn);
    if ( FAILED(hr) )
        return hr;

            wxDataFormat format = (wxDataFormat::NativeFormat)pformatetcIn->cfFormat;
    format = HtmlFormatFixup(format);

        if ( GetSystemData(format, pmedium) )
    {
                return S_OK;
    }

    switch ( format )
    {
        case wxDF_BITMAP:
            pmedium->tymed = TYMED_GDI;
            break;

        case wxDF_ENHMETAFILE:
            pmedium->tymed = TYMED_ENHMF;
            break;

        case wxDF_METAFILE:
            pmedium->hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
                                           sizeof(METAFILEPICT));
            if ( !pmedium->hGlobal ) {
                wxLogLastError(wxT("GlobalAlloc"));
                return E_OUTOFMEMORY;
            }
            pmedium->tymed = TYMED_MFPICT;
            break;
        default:
                        size_t size = m_pDataObject->GetDataSize(format);
            if ( !size ) {
                                wxLogDebug(wxT("Invalid data size - can't be 0"));

                return DV_E_FORMATETC;
            }

                        size += m_pDataObject->GetBufferOffset( format );

            HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, size);
            if ( hGlobal == NULL ) {
                wxLogLastError(wxT("GlobalAlloc"));
                return E_OUTOFMEMORY;
            }

                        pmedium->tymed   = TYMED_HGLOBAL;
            pmedium->hGlobal = hGlobal;
    }

    pmedium->pUnkForRelease = NULL;

        hr = GetDataHere(pformatetcIn, pmedium);
    if ( FAILED(hr) ) {
                if ( pmedium->tymed & (TYMED_HGLOBAL | TYMED_MFPICT) ) {
            GlobalFree(pmedium->hGlobal);
        }

        return hr;
    }

    return S_OK;
}

STDMETHODIMP wxIDataObject::GetDataHere(FORMATETC *pformatetc,
                                        STGMEDIUM *pmedium)
{
    wxLogTrace(wxTRACE_OleCalls, wxT("wxIDataObject::GetDataHere"));

        switch ( pmedium->tymed )
    {
        case TYMED_GDI:
            if ( !m_pDataObject->GetDataHere(wxDF_BITMAP, &pmedium->hBitmap) )
                return E_UNEXPECTED;
            break;

        case TYMED_ENHMF:
            if ( !m_pDataObject->GetDataHere(wxDF_ENHMETAFILE,
                                             &pmedium->hEnhMetaFile) )
                return E_UNEXPECTED;
            break;

        case TYMED_MFPICT:
            
        case TYMED_HGLOBAL:
            {
                                HGLOBAL hGlobal = pmedium->hGlobal;
                void *pBuf = GlobalLock(hGlobal);
                if ( pBuf == NULL ) {
                    wxLogLastError(wxT("GlobalLock"));
                    return E_OUTOFMEMORY;
                }

                wxDataFormat format = pformatetc->cfFormat;

                                pBuf = m_pDataObject->SetSizeInBuffer
                                      (
                                        pBuf,
                                        ::GlobalSize(hGlobal),
                                        format
                                      );

                if ( !m_pDataObject->GetDataHere(format, pBuf) )
                    return E_UNEXPECTED;

                GlobalUnlock(hGlobal);
            }
            break;

        default:
            return DV_E_TYMED;
    }

    return S_OK;
}


STDMETHODIMP wxIDataObject::SetData(FORMATETC *pformatetc,
                                    STGMEDIUM *pmedium,
                                    BOOL       fRelease)
{
    wxLogTrace(wxTRACE_OleCalls, wxT("wxIDataObject::SetData"));

    switch ( pmedium->tymed )
    {
        case TYMED_GDI:
            m_pDataObject->SetData(wxDF_BITMAP, 0, &pmedium->hBitmap);
            break;

        case TYMED_ENHMF:
            m_pDataObject->SetData(wxDF_ENHMETAFILE, 0, &pmedium->hEnhMetaFile);
            break;

        case TYMED_ISTREAM:
                        if ( !m_pDataObject->IsSupported(pformatetc->cfFormat,
                                             wxDataObject::Set) )
            {
                                                return SaveSystemData(pformatetc, pmedium, fRelease);
            }
            break;

        case TYMED_MFPICT:
                    case TYMED_HGLOBAL:
            {
                wxDataFormat format = pformatetc->cfFormat;

                format = HtmlFormatFixup(format);

                                if ( !m_pDataObject->IsSupported(format, wxDataObject::Set) ) {
                                                            return SaveSystemData(pformatetc, pmedium, fRelease);
                }

                                const void *pBuf = GlobalLock(pmedium->hGlobal);
                if ( pBuf == NULL ) {
                    wxLogLastError(wxT("GlobalLock"));

                    return E_OUTOFMEMORY;
                }

                                                                                                size_t size;
                switch ( format )
                {
                    case wxDF_HTML:
                    case CF_TEXT:
                    case CF_OEMTEXT:
                        size = strlen((const char *)pBuf);
                        break;
#if !(defined(__BORLANDC__) && (__BORLANDC__ < 0x500))
                    case CF_UNICODETEXT:
#if ( defined(__BORLANDC__) && (__BORLANDC__ > 0x530) )
                        size = std::wcslen((const wchar_t *)pBuf) * sizeof(wchar_t);
#else
                        size = wxWcslen((const wchar_t *)pBuf) * sizeof(wchar_t);
#endif
                        break;
#endif
                    case CF_BITMAP:
                    case CF_HDROP:
                                                                        size = 0;
                        break;

                    case CF_DIB:
                                                                        size = 0;
                        break;

                    case CF_METAFILEPICT:
                        size = sizeof(METAFILEPICT);
                        break;
                    default:
                        pBuf = m_pDataObject->
                                    GetSizeFromBuffer(pBuf, &size, format);
                        size -= m_pDataObject->GetBufferOffset(format);
                }

                bool ok = m_pDataObject->SetData(format, size, pBuf);

                GlobalUnlock(pmedium->hGlobal);

                if ( !ok ) {
                    return E_UNEXPECTED;
                }
            }
            break;

        default:
            return DV_E_TYMED;
    }

    if ( fRelease ) {
                        switch ( pmedium->tymed )
        {
            case TYMED_GDI:
                pmedium->hBitmap = 0;
                break;

            case TYMED_MFPICT:
                pmedium->hMetaFilePict = 0;
                break;

            case TYMED_ENHMF:
                pmedium->hEnhMetaFile = 0;
                break;
        }

        ReleaseStgMedium(pmedium);
    }

    return S_OK;
}

STDMETHODIMP wxIDataObject::QueryGetData(FORMATETC *pformatetc)
{
        if ( pformatetc == NULL ) {
        wxLogTrace(wxTRACE_OleCalls,
                   wxT("wxIDataObject::QueryGetData: invalid ptr."));

        return E_INVALIDARG;
    }

        if ( pformatetc->lindex != -1 ) {
        wxLogTrace(wxTRACE_OleCalls,
                   wxT("wxIDataObject::QueryGetData: bad lindex %ld"),
                   pformatetc->lindex);

        return DV_E_LINDEX;
    }

        if ( pformatetc->dwAspect != DVASPECT_CONTENT ) {
        wxLogTrace(wxTRACE_OleCalls,
                   wxT("wxIDataObject::QueryGetData: bad dwAspect %ld"),
                   pformatetc->dwAspect);

        return DV_E_DVASPECT;
    }

        wxDataFormat format = pformatetc->cfFormat;
    format = HtmlFormatFixup(format);

    if ( m_pDataObject->IsSupportedFormat(format) ) {
        wxLogTrace(wxTRACE_OleCalls, wxT("wxIDataObject::QueryGetData: %s ok"),
                   wxGetFormatName(format));
    }
    else if ( HasSystemData(format) )
    {
        wxLogTrace(wxTRACE_OleCalls, wxT("wxIDataObject::QueryGetData: %s ok (system data)"),
                   wxGetFormatName(format));
                return S_OK;
    }
    else {
        wxLogTrace(wxTRACE_OleCalls,
                   wxT("wxIDataObject::QueryGetData: %s unsupported"),
                   wxGetFormatName(format));

        return DV_E_FORMATETC;
    }

        DWORD tymed = pformatetc->tymed;
    if ( (format == wxDF_BITMAP && !(tymed & TYMED_GDI)) &&
         !(tymed & TYMED_HGLOBAL) ) {
                wxLogTrace(wxTRACE_OleCalls,
                   wxT("wxIDataObject::QueryGetData: %s != %s"),
                   GetTymedName(tymed),
                   GetTymedName(format == wxDF_BITMAP ? TYMED_GDI
                                                      : TYMED_HGLOBAL));

        return DV_E_TYMED;
    }

    return S_OK;
}

STDMETHODIMP wxIDataObject::GetCanonicalFormatEtc(FORMATETC *WXUNUSED(pFormatetcIn),
                                                  FORMATETC *pFormatetcOut)
{
    wxLogTrace(wxTRACE_OleCalls, wxT("wxIDataObject::GetCanonicalFormatEtc"));

        if ( pFormatetcOut != NULL )
        pFormatetcOut->ptd = NULL;

    return DATA_S_SAMEFORMATETC;
}

STDMETHODIMP wxIDataObject::EnumFormatEtc(DWORD dwDir,
                                          IEnumFORMATETC **ppenumFormatEtc)
{
    wxLogTrace(wxTRACE_OleCalls, wxT("wxIDataObject::EnumFormatEtc"));

    wxDataObject::Direction dir = dwDir == DATADIR_GET ? wxDataObject::Get
                                                       : wxDataObject::Set;

        const size_t ourFormatCount = m_pDataObject->GetFormatCount(dir);
    const size_t sysFormatCount = m_systemData.size();

    const ULONG
        nFormatCount = wx_truncate_cast(ULONG, ourFormatCount + sysFormatCount);

        wxScopedArray<wxDataFormat> formats(nFormatCount);

        m_pDataObject->GetAllFormats(formats.get(), dir);

        for ( size_t j = 0; j < sysFormatCount; j++ )
    {
        SystemDataEntry* entry = m_systemData[j];
        wxDataFormat& format = formats[ourFormatCount + j];
        format = entry->pformatetc->cfFormat;
    }

    wxIEnumFORMATETC *pEnum = new wxIEnumFORMATETC(formats.get(), nFormatCount);
    pEnum->AddRef();
    *ppenumFormatEtc = pEnum;

    return S_OK;
}


STDMETHODIMP wxIDataObject::DAdvise(FORMATETC   *WXUNUSED(pformatetc),
                                    DWORD        WXUNUSED(advf),
                                    IAdviseSink *WXUNUSED(pAdvSink),
                                    DWORD       *WXUNUSED(pdwConnection))
{
  return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP wxIDataObject::DUnadvise(DWORD WXUNUSED(dwConnection))
{
  return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP wxIDataObject::EnumDAdvise(IEnumSTATDATA **WXUNUSED(ppenumAdvise))
{
  return OLE_E_ADVISENOTSUPPORTED;
}


wxDataObject::wxDataObject()
{
    m_pIDataObject = new wxIDataObject(this);
    m_pIDataObject->AddRef();
}

wxDataObject::~wxDataObject()
{
    ReleaseInterface(m_pIDataObject);
}

void wxDataObject::SetAutoDelete()
{
    ((wxIDataObject *)m_pIDataObject)->SetDeleteFlag();
    m_pIDataObject->Release();

        m_pIDataObject = NULL;
}

size_t wxDataObject::GetBufferOffset(const wxDataFormat& format )
{
        return NeedsVerbatimData(format) ? 0 : sizeof(size_t);
}

const void *wxDataObject::GetSizeFromBuffer(const void *buffer,
                                            size_t *size,
                                            const wxDataFormat& WXUNUSED(format))
{
            const size_t realsz = ::HeapSize(::GetProcessHeap(), 0,
                                     const_cast<void*>(buffer));
    if ( realsz == (size_t)-1 )
    {
                wxLogApiError(wxT("HeapSize"), 0);
        return NULL;
    }

    *size = realsz;

    return buffer;
}

void* wxDataObject::SetSizeInBuffer( void* buffer, size_t size,
                                      const wxDataFormat& format )
{
    size_t* p = (size_t *)buffer;
    if ( !NeedsVerbatimData(format) )
    {
                *p++ = size;
    }

    return p;
}

#if wxDEBUG_LEVEL

const wxChar *wxDataObject::GetFormatName(wxDataFormat format)
{
        #ifdef __VISUALC__
        #pragma warning(disable:4063)
    #endif 
    static wxChar s_szBuf[256];
    switch ( format ) {
        case CF_TEXT:         return wxT("CF_TEXT");
        case CF_BITMAP:       return wxT("CF_BITMAP");
        case CF_SYLK:         return wxT("CF_SYLK");
        case CF_DIF:          return wxT("CF_DIF");
        case CF_TIFF:         return wxT("CF_TIFF");
        case CF_OEMTEXT:      return wxT("CF_OEMTEXT");
        case CF_DIB:          return wxT("CF_DIB");
        case CF_PALETTE:      return wxT("CF_PALETTE");
        case CF_PENDATA:      return wxT("CF_PENDATA");
        case CF_RIFF:         return wxT("CF_RIFF");
        case CF_WAVE:         return wxT("CF_WAVE");
        case CF_UNICODETEXT:  return wxT("CF_UNICODETEXT");
        case CF_METAFILEPICT: return wxT("CF_METAFILEPICT");
        case CF_ENHMETAFILE:  return wxT("CF_ENHMETAFILE");
        case CF_LOCALE:       return wxT("CF_LOCALE");
        case CF_HDROP:        return wxT("CF_HDROP");

        default:
            if ( !::GetClipboardFormatName(format, s_szBuf, WXSIZEOF(s_szBuf)) )
            {
                                wxSprintf(s_szBuf, wxT("unknown CF (0x%04x)"), format.GetFormatId());
            }

            return s_szBuf;
    }

    #ifdef __VISUALC__
        #pragma warning(default:4063)
    #endif }

#endif 

size_t wxBitmapDataObject::GetDataSize() const
{
#if wxUSE_WXDIB
    return wxDIB::ConvertFromBitmap(NULL, GetHbitmapOf(GetBitmap()));
#else
    return 0;
#endif
}

bool wxBitmapDataObject::GetDataHere(void *buf) const
{
#if wxUSE_WXDIB
    BITMAPINFO * const pbi = (BITMAPINFO *)buf;

    return wxDIB::ConvertFromBitmap(pbi, GetHbitmapOf(GetBitmap())) != 0;
#else
    wxUnusedVar(buf);
    return false;
#endif
}

bool wxBitmapDataObject::SetData(size_t WXUNUSED(len), const void *buf)
{
#if wxUSE_WXDIB
    const BITMAPINFO * const pbmi = (const BITMAPINFO *)buf;

    HBITMAP hbmp = wxDIB::ConvertToBitmap(pbmi);

    wxCHECK_MSG( hbmp, FALSE, wxT("pasting/dropping invalid bitmap") );

    const BITMAPINFOHEADER * const pbmih = &pbmi->bmiHeader;
    wxBitmap bitmap(pbmih->biWidth, pbmih->biHeight, pbmih->biBitCount);
    bitmap.SetHBITMAP((WXHBITMAP)hbmp);

    
    SetBitmap(bitmap);

    return true;
#else
    wxUnusedVar(buf);
    return false;
#endif
}



size_t wxBitmapDataObject2::GetDataSize() const
{
    return 0;
}

bool wxBitmapDataObject2::GetDataHere(void *pBuf) const
{
        *(WXHBITMAP *)pBuf = GetBitmap().GetHBITMAP();

    return true;
}

bool wxBitmapDataObject2::SetData(size_t WXUNUSED(len), const void *pBuf)
{
    HBITMAP hbmp = *(HBITMAP *)pBuf;

    BITMAP bmp;
    if ( !GetObject(hbmp, sizeof(BITMAP), &bmp) )
    {
        wxLogLastError(wxT("GetObject(HBITMAP)"));
    }

    wxBitmap bitmap(bmp.bmWidth, bmp.bmHeight, bmp.bmPlanes);
    bitmap.SetHBITMAP((WXHBITMAP)hbmp);

    if ( !bitmap.IsOk() ) {
        wxFAIL_MSG(wxT("pasting/dropping invalid bitmap"));

        return false;
    }

    SetBitmap(bitmap);

    return true;
}

#if 0

size_t wxBitmapDataObject::GetDataSize(const wxDataFormat& format) const
{
    if ( format.GetFormatId() == CF_DIB )
    {
                ScreenHDC hdc;

                wxASSERT_MSG( !m_bitmap.GetSelectedInto(),
                      wxT("can't copy bitmap selected into wxMemoryDC") );

                BITMAPINFO bi;
        if ( !GetDIBits(hdc, (HBITMAP)m_bitmap.GetHBITMAP(), 0, 0,
                        NULL, &bi, DIB_RGB_COLORS) )
        {
            wxLogLastError(wxT("GetDIBits(NULL)"));

            return 0;
        }

        return sizeof(BITMAPINFO) + bi.bmiHeader.biSizeImage;
    }
    else     {
                return 0;
    }
}

bool wxBitmapDataObject::GetDataHere(const wxDataFormat& format,
                                     void *pBuf) const
{
    wxASSERT_MSG( m_bitmap.IsOk(), wxT("copying invalid bitmap") );

    HBITMAP hbmp = (HBITMAP)m_bitmap.GetHBITMAP();
    if ( format.GetFormatId() == CF_DIB )
    {
                ScreenHDC hdc;

                wxASSERT_MSG( !m_bitmap.GetSelectedInto(),
                      wxT("can't copy bitmap selected into wxMemoryDC") );

                BITMAPINFO *pbi = (BITMAPINFO *)pBuf;
        if ( !GetDIBits(hdc, hbmp, 0, 0, NULL, pbi, DIB_RGB_COLORS) )
        {
            wxLogLastError(wxT("GetDIBits(NULL)"));

            return 0;
        }

                if ( !GetDIBits(hdc, hbmp, 0, pbi->bmiHeader.biHeight, pbi + 1,
                        pbi, DIB_RGB_COLORS) )
        {
            wxLogLastError(wxT("GetDIBits"));

            return false;
        }
    }
    else     {
                *(HBITMAP *)pBuf = hbmp;
    }

    return true;
}

bool wxBitmapDataObject::SetData(const wxDataFormat& format,
                                 size_t size, const void *pBuf)
{
    HBITMAP hbmp;
    if ( format.GetFormatId() == CF_DIB )
    {
                        ScreenHDC hdc;

        BITMAPINFO *pbmi = (BITMAPINFO *)pBuf;
        BITMAPINFOHEADER *pbmih = &pbmi->bmiHeader;
        hbmp = CreateDIBitmap(hdc, pbmih, CBM_INIT,
                              pbmi + 1, pbmi, DIB_RGB_COLORS);
        if ( !hbmp )
        {
            wxLogLastError(wxT("CreateDIBitmap"));
        }

        m_bitmap.SetWidth(pbmih->biWidth);
        m_bitmap.SetHeight(pbmih->biHeight);
    }
    else     {
                hbmp = *(HBITMAP *)pBuf;

        BITMAP bmp;
        if ( !GetObject(hbmp, sizeof(BITMAP), &bmp) )
        {
            wxLogLastError(wxT("GetObject(HBITMAP)"));
        }

        m_bitmap.SetWidth(bmp.bmWidth);
        m_bitmap.SetHeight(bmp.bmHeight);
        m_bitmap.SetDepth(bmp.bmPlanes);
    }

    m_bitmap.SetHBITMAP((WXHBITMAP)hbmp);

    wxASSERT_MSG( m_bitmap.IsOk(), wxT("pasting invalid bitmap") );

    return true;
}

#endif 

bool wxFileDataObject::SetData(size_t WXUNUSED(size),
                               const void *pData)
{
    m_filenames.Empty();

                            HDROP hdrop = (HDROP)pData;   
        UINT nFiles = ::DragQueryFile(hdrop, (unsigned)-1, NULL, 0u);

    wxCHECK_MSG ( nFiles != (UINT)-1, FALSE, wxT("wrong HDROP handle") );

        wxString str;
    UINT len, n;
    for ( n = 0; n < nFiles; n++ ) {
                len = ::DragQueryFile(hdrop, n, NULL, 0) + 1;

        UINT len2 = ::DragQueryFile(hdrop, n, wxStringBuffer(str, len), len);
        m_filenames.Add(str);

        if ( len2 != len - 1 ) {
            wxLogDebug(wxT("In wxFileDropTarget::OnDrop DragQueryFile returned\
 %d characters, %d expected."), len2, len - 1);
        }
    }

    return true;
}

void wxFileDataObject::AddFile(const wxString& file)
{
                m_filenames.Add(file);
}

size_t wxFileDataObject::GetDataSize() const
{
        
        if ( m_filenames.empty() )
        return 0;

    static const size_t sizeOfChar = sizeof(wxChar);

        size_t sz = sizeof(DROPFILES) + sizeOfChar;

    const size_t count = m_filenames.size();
    for ( size_t i = 0; i < count; i++ )
    {
                size_t len = m_filenames[i].length();

        sz += (len + 1) * sizeOfChar;
    }

    return sz;
}

bool wxFileDataObject::GetDataHere(void *pData) const
{
        
        if ( !pData || m_filenames.empty() )
        return false;

        LPDROPFILES pDrop = (LPDROPFILES) pData;

        pDrop->pFiles = sizeof(DROPFILES);
    pDrop->fNC = FALSE;                     pDrop->fWide = wxUSE_UNICODE;

    const size_t sizeOfChar = pDrop->fWide ? sizeof(wchar_t) : 1;

        BYTE *pbuf = (BYTE *)(pDrop + 1);

    const size_t count = m_filenames.size();
    for ( size_t i = 0; i < count; i++ )
    {
                size_t len = m_filenames[i].length();
        memcpy(pbuf, m_filenames[i].t_str(), len*sizeOfChar);

        pbuf += len*sizeOfChar;

        memset(pbuf, 0, sizeOfChar);
        pbuf += sizeOfChar;
    }

        memset(pbuf, 0, sizeOfChar);

    return true;
}


#if defined(__WINE__) && defined(CFSTR_SHELLURL) && wxUSE_UNICODE
#undef CFSTR_SHELLURL
#define CFSTR_SHELLURL wxT("CFSTR_SHELLURL")
#endif

class CFSTR_SHELLURLDataObject : public wxCustomDataObject
{
public:
    CFSTR_SHELLURLDataObject() : wxCustomDataObject(CFSTR_SHELLURL) {}

    virtual size_t GetBufferOffset( const wxDataFormat& WXUNUSED(format) )
    {
        return 0;
    }

    virtual const void* GetSizeFromBuffer( const void* buffer, size_t* size,
                                           const wxDataFormat& WXUNUSED(format) )
    {
                *size = strlen( (const char*)buffer );

        return buffer;
    }

    virtual void* SetSizeInBuffer( void* buffer, size_t WXUNUSED(size),
                                   const wxDataFormat& WXUNUSED(format) )
    {
        return buffer;
    }

    wxDECLARE_NO_COPY_CLASS(CFSTR_SHELLURLDataObject);
};



wxURLDataObject::wxURLDataObject(const wxString& url)
{
                Add(new wxTextDataObject);
    Add(new CFSTR_SHELLURLDataObject());

        m_dataObjectLast = NULL;

    if ( !url.empty() )
        SetURL(url);
}

bool wxURLDataObject::SetData(const wxDataFormat& format,
                              size_t len,
                              const void *buf)
{
    m_dataObjectLast = GetObject(format);

    wxCHECK_MSG( m_dataObjectLast, FALSE,
                 wxT("unsupported format in wxURLDataObject"));

    return m_dataObjectLast->SetData(len, buf);
}

wxString wxURLDataObject::GetURL() const
{
    wxString url;
    wxCHECK_MSG( m_dataObjectLast, url, wxT("no data in wxURLDataObject") );

    if ( m_dataObjectLast->GetPreferredFormat() == CFSTR_SHELLURL )
    {
        const size_t len = m_dataObjectLast->GetDataSize();
        if ( !len )
            return wxString();

                #if wxUSE_UNICODE
        wxCharBuffer buf(len);

        if ( m_dataObjectLast->GetDataHere(buf.data()) )
            url = buf;
#else                 m_dataObjectLast->GetDataHere(wxStringBuffer(url, len));
#endif     }
    else     {
        url = static_cast<wxTextDataObject *>(m_dataObjectLast)->GetText();
    }

    return url;
}

void wxURLDataObject::SetURL(const wxString& url)
{
    wxCharBuffer urlMB(url.mb_str());
    if ( urlMB )
    {
        const size_t len = strlen(urlMB);

#if !wxUSE_UNICODE
                        SetData(wxDF_TEXT, len, urlMB);
#endif 
                                SetData(wxDataFormat(CFSTR_SHELLURL), len + 1, urlMB);
    }

#if wxUSE_UNICODE
    SetData(wxDF_UNICODETEXT, url.length()*sizeof(wxChar), url.wc_str());
#endif
}


#if wxDEBUG_LEVEL

static const wxChar *GetTymedName(DWORD tymed)
{
    static wxChar s_szBuf[128];
    switch ( tymed ) {
        case TYMED_HGLOBAL:   return wxT("TYMED_HGLOBAL");
        case TYMED_FILE:      return wxT("TYMED_FILE");
        case TYMED_ISTREAM:   return wxT("TYMED_ISTREAM");
        case TYMED_ISTORAGE:  return wxT("TYMED_ISTORAGE");
        case TYMED_GDI:       return wxT("TYMED_GDI");
        case TYMED_MFPICT:    return wxT("TYMED_MFPICT");
        case TYMED_ENHMF:     return wxT("TYMED_ENHMF");
        default:
            wxSprintf(s_szBuf, wxT("type of media format %ld (unknown)"), tymed);
            return s_szBuf;
    }
}

#endif 
#else 

#if wxUSE_DATAOBJ

wxDataObject::wxDataObject()
{
}

wxDataObject::~wxDataObject()
{
}

void wxDataObject::SetAutoDelete()
{
}

const wxChar *wxDataObject::GetFormatName(wxDataFormat WXUNUSED(format))
{
    return NULL;
}

#endif 
#endif 

