
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_DATAOBJ

#include "wx/dataobj.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
#endif

#include "wx/textbuf.h"


#include "wx/listimpl.cpp"

WX_DEFINE_LIST(wxSimpleDataObjectList)


static wxDataFormat dataFormatInvalid;
WXDLLEXPORT const wxDataFormat& wxFormatInvalid = dataFormatInvalid;



wxDataObjectBase::~wxDataObjectBase()
{
}

bool wxDataObjectBase::IsSupported(const wxDataFormat& format,
                                   Direction dir) const
{
    size_t nFormatCount = GetFormatCount( dir );
    if ( nFormatCount == 1 )
    {
        return format == GetPreferredFormat( dir );
    }
    else
    {
        wxDataFormat *formats = new wxDataFormat[nFormatCount];
        GetAllFormats( formats, dir );

        size_t n;
        for ( n = 0; n < nFormatCount; n++ )
        {
            if ( formats[n] == format )
                break;
        }

        delete [] formats;

                return n < nFormatCount;
    }
}


wxDataObjectComposite::wxDataObjectComposite()
{
    m_preferred = 0;
    m_receivedFormat = wxFormatInvalid;
}

wxDataObjectComposite::~wxDataObjectComposite()
{
    WX_CLEAR_LIST( wxSimpleDataObjectList, m_dataObjects );
}

wxDataObjectSimple *
wxDataObjectComposite::GetObject(const wxDataFormat& format, wxDataObjectBase::Direction dir) const
{
    wxSimpleDataObjectList::compatibility_iterator node = m_dataObjects.GetFirst();

    while ( node )
    {
        wxDataObjectSimple *dataObj = node->GetData();

        if (dataObj->IsSupported(format,dir))
          return dataObj;
        node = node->GetNext();
    }
    return NULL;
}

void wxDataObjectComposite::Add(wxDataObjectSimple *dataObject, bool preferred)
{
    if ( preferred )
        m_preferred = m_dataObjects.GetCount();

    m_dataObjects.Append( dataObject );
}

wxDataFormat wxDataObjectComposite::GetReceivedFormat() const
{
    return m_receivedFormat;
}

wxDataFormat
wxDataObjectComposite::GetPreferredFormat(Direction WXUNUSED(dir)) const
{
    wxSimpleDataObjectList::compatibility_iterator node = m_dataObjects.Item( m_preferred );

    wxCHECK_MSG( node, wxFormatInvalid, wxT("no preferred format") );

    wxDataObjectSimple* dataObj = node->GetData();

    return dataObj->GetFormat();
}

#if defined(__WXMSW__)

size_t wxDataObjectComposite::GetBufferOffset( const wxDataFormat& format )
{
    wxDataObjectSimple *dataObj = GetObject(format);

    wxCHECK_MSG( dataObj, 0,
                 wxT("unsupported format in wxDataObjectComposite"));

    return dataObj->GetBufferOffset( format );
}


const void* wxDataObjectComposite::GetSizeFromBuffer( const void* buffer,
                                                      size_t* size,
                                                      const wxDataFormat& format )
{
    wxDataObjectSimple *dataObj = GetObject(format);

    wxCHECK_MSG( dataObj, NULL,
                 wxT("unsupported format in wxDataObjectComposite"));

    return dataObj->GetSizeFromBuffer( buffer, size, format );
}


void* wxDataObjectComposite::SetSizeInBuffer( void* buffer, size_t size,
                                              const wxDataFormat& format )
{
    wxDataObjectSimple *dataObj = GetObject( format );

    wxCHECK_MSG( dataObj, NULL,
                 wxT("unsupported format in wxDataObjectComposite"));

    return dataObj->SetSizeInBuffer( buffer, size, format );
}

#endif

size_t wxDataObjectComposite::GetFormatCount(Direction dir) const
{
    size_t n = 0;

                wxSimpleDataObjectList::compatibility_iterator node;
    for ( node = m_dataObjects.GetFirst(); node; node = node->GetNext() )
        n += node->GetData()->GetFormatCount(dir);

    return n;
}

void wxDataObjectComposite::GetAllFormats(wxDataFormat *formats,
                                          Direction dir) const
{
    size_t index(0);
    wxSimpleDataObjectList::compatibility_iterator node;

    for ( node = m_dataObjects.GetFirst(); node; node = node->GetNext() )
    {
                                node->GetData()->GetAllFormats(formats+index, dir);
        index += node->GetData()->GetFormatCount(dir);
    }
}

size_t wxDataObjectComposite::GetDataSize(const wxDataFormat& format) const
{
    wxDataObjectSimple *dataObj = GetObject(format);

    wxCHECK_MSG( dataObj, 0,
                 wxT("unsupported format in wxDataObjectComposite"));

    return dataObj->GetDataSize();
}

bool wxDataObjectComposite::GetDataHere(const wxDataFormat& format,
                                        void *buf) const
{
    wxDataObjectSimple *dataObj = GetObject( format );

    wxCHECK_MSG( dataObj, false,
                 wxT("unsupported format in wxDataObjectComposite"));

    return dataObj->GetDataHere( buf );
}

bool wxDataObjectComposite::SetData(const wxDataFormat& format,
                                    size_t len,
                                    const void *buf)
{
    wxDataObjectSimple *dataObj = GetObject( format );

    wxCHECK_MSG( dataObj, false,
                 wxT("unsupported format in wxDataObjectComposite"));

    m_receivedFormat = format;

                    return dataObj->SetData( format, len, buf );
}


#ifdef wxNEEDS_UTF8_FOR_TEXT_DATAOBJ


#if wxUSE_UNICODE_WCHAR

static inline wxMBConv& GetConv(const wxDataFormat& format)
{
        return format == wxDF_UNICODETEXT ? wxConvUTF8 : wxConvLibc;
}

size_t wxTextDataObject::GetDataSize(const wxDataFormat& format) const
{
    wxCharBuffer buffer = GetConv(format).cWX2MB( GetText().c_str() );

    return buffer ? strlen( buffer ) : 0;
}

bool wxTextDataObject::GetDataHere(const wxDataFormat& format, void *buf) const
{
    if ( !buf )
        return false;

    wxCharBuffer buffer = GetConv(format).cWX2MB( GetText().c_str() );
    if ( !buffer )
        return false;

    memcpy( (char*) buf, buffer, GetDataSize(format) );
    
    return true;
}

bool wxTextDataObject::SetData(const wxDataFormat& format,
                               size_t len, const void *buf)
{
    if ( buf == NULL )
        return false;

    wxWCharBuffer buffer = GetConv(format).cMB2WC((const char*)buf, len, NULL);

    SetText( buffer );

    return true;
}

#else 
size_t wxTextDataObject::GetDataSize(const wxDataFormat& format) const
{
    const wxString& text = GetText();
    if ( format == wxDF_UNICODETEXT || wxLocaleIsUtf8 )
    {
        return text.utf8_length();
    }
    else     {
        const wxCharBuffer buf(wxConvLocal.cWC2MB(text.wc_str()));
        return buf ? strlen(buf) : 0;
    }
}

bool wxTextDataObject::GetDataHere(const wxDataFormat& format, void *buf) const
{
    if ( !buf )
        return false;

    const wxString& text = GetText();
    if ( format == wxDF_UNICODETEXT || wxLocaleIsUtf8 )
    {
        memcpy(buf, text.utf8_str(), text.utf8_length());
    }
    else     {
        const wxCharBuffer bufLocal(wxConvLocal.cWC2MB(text.wc_str()));
        if ( !bufLocal )
            return false;

        memcpy(buf, bufLocal, strlen(bufLocal));
    }

    return true;
}

bool wxTextDataObject::SetData(const wxDataFormat& format,
                               size_t len, const void *buf_)
{
    const char * const buf = static_cast<const char *>(buf_);

    if ( buf == NULL )
        return false;

    if ( format == wxDF_UNICODETEXT || wxLocaleIsUtf8 )
    {
                                                SetText(wxString::FromUTF8(buf, len));
    }
    else     {
        SetText(wxConvLocal.cMB2WC(buf, len, NULL));
    }

    return true;
}

#endif 
#elif defined(wxNEEDS_UTF16_FOR_TEXT_DATAOBJ)

namespace
{

inline wxMBConv& GetConv(const wxDataFormat& format)
{
    static wxMBConvUTF16 s_UTF16Converter;

    return format == wxDF_UNICODETEXT ? static_cast<wxMBConv&>(s_UTF16Converter)
                                      : static_cast<wxMBConv&>(wxConvLocal);
}

} 
size_t wxTextDataObject::GetDataSize(const wxDataFormat& format) const
{
    return GetConv(format).WC2MB(NULL, GetText().wc_str(), 0);
}

bool wxTextDataObject::GetDataHere(const wxDataFormat& format, void *buf) const
{
    if ( buf == NULL )
        return false;

    wxCharBuffer buffer(GetConv(format).cWX2MB(GetText().c_str()));

    memcpy(buf, buffer.data(), buffer.length());

    return true;
}

bool wxTextDataObject::SetData(const wxDataFormat& format,
                               size_t WXUNUSED(len),
                               const void *buf)
{
    if ( buf == NULL )
        return false;

    SetText(GetConv(format).cMB2WX(static_cast<const char*>(buf)));

    return true;
}

#else 

size_t wxTextDataObject::GetDataSize() const
{
    return (wxTextBuffer::Translate(GetText()).length() + 1)*sizeof(wxChar);
}

bool wxTextDataObject::GetDataHere(void *buf) const
{
    const wxString textNative = wxTextBuffer::Translate(GetText());

            wxTmemcpy(static_cast<wxChar*>(buf),
              textNative.t_str(),
              textNative.length() + 1);

    return true;
}

bool wxTextDataObject::SetData(size_t len, const void *buf)
{
    const wxString
        text = wxString(static_cast<const wxChar*>(buf), len/sizeof(wxChar));
    SetText(wxTextBuffer::Translate(text, wxTextFileType_Unix));

    return true;
}

#endif 

size_t wxHTMLDataObject::GetDataSize() const
{
            const wxString& htmlStr = GetHTML();
    const wxScopedCharBuffer buffer(htmlStr.utf8_str());

    size_t size = buffer.length();

#ifdef __WXMSW__
            size += 400;
#endif

    return size;
}

bool wxHTMLDataObject::GetDataHere(void *buf) const
{
    if ( !buf )
        return false;

        const wxString& htmlStr = GetHTML();
    const wxScopedCharBuffer html(htmlStr.utf8_str());
    if ( !html )
        return false;

    char* const buffer = static_cast<char*>(buf);

#ifdef __WXMSW__
    
            strcpy(buffer,
        "Version:0.9\r\n"
        "StartHTML:00000000\r\n"
        "EndHTML:00000000\r\n"
        "StartFragment:00000000\r\n"
        "EndFragment:00000000\r\n"
        "<html><body>\r\n"
        "<!--StartFragment -->\r\n");

        strcat(buffer, html);
    strcat(buffer, "\r\n");
        strcat(buffer,
        "<!--EndFragment-->\r\n"
        "</body>\r\n"
        "</html>");

                    char *ptr = strstr(buffer, "StartHTML");
    sprintf(ptr+10, "%08u", (unsigned)(strstr(buffer, "<html>") - buffer));
    *(ptr+10+8) = '\r';

    ptr = strstr(buffer, "EndHTML");
    sprintf(ptr+8, "%08u", (unsigned)strlen(buffer));
    *(ptr+8+8) = '\r';

    ptr = strstr(buffer, "StartFragment");
    sprintf(ptr+14, "%08u", (unsigned)(strstr(buffer, "<!--StartFrag") - buffer));
    *(ptr+14+8) = '\r';

    ptr = strstr(buffer, "EndFragment");
    sprintf(ptr+12, "%08u", (unsigned)(strstr(buffer, "<!--EndFrag") - buffer));
    *(ptr+12+8) = '\r';
#else
    strcpy(buffer, html);
#endif 
    return true;
}

bool wxHTMLDataObject::SetData(size_t WXUNUSED(len), const void *buf)
{
    if ( buf == NULL )
        return false;

        wxString html = wxString::FromUTF8(static_cast<const char*>(buf));

#ifdef __WXMSW__
            int fragmentStart = html.rfind("StartFragment");
    int fragmentEnd = html.rfind("EndFragment");

    if (fragmentStart != wxNOT_FOUND && fragmentEnd != wxNOT_FOUND)
    {
        int startCommentEnd = html.find("-->", fragmentStart) + 3;
        int endCommentStart = html.rfind("<!--", fragmentEnd);

        if (startCommentEnd != wxNOT_FOUND && endCommentStart != wxNOT_FOUND)
            html = html.Mid(startCommentEnd, endCommentStart - startCommentEnd);
    }
#endif 
    SetHTML( html );

    return true;
}



wxCustomDataObject::wxCustomDataObject(const wxDataFormat& format)
    : wxDataObjectSimple(format)
{
    m_data = NULL;
    m_size = 0;
}

wxCustomDataObject::~wxCustomDataObject()
{
    Free();
}

void wxCustomDataObject::TakeData(size_t size, void *data)
{
    Free();

    m_size = size;
    m_data = data;
}

void *wxCustomDataObject::Alloc(size_t size)
{
    return (void *)new char[size];
}

void wxCustomDataObject::Free()
{
    delete [] (char*)m_data;
    m_size = 0;
    m_data = NULL;
}

size_t wxCustomDataObject::GetDataSize() const
{
    return GetSize();
}

bool wxCustomDataObject::GetDataHere(void *buf) const
{
    if ( buf == NULL )
        return false;

    void *data = GetData();
    if ( data == NULL )
        return false;

    memcpy( buf, data, GetSize() );

    return true;
}

bool wxCustomDataObject::SetData(size_t size, const void *buf)
{
    Free();

    m_data = Alloc(size);
    if ( m_data == NULL )
        return false;

    m_size = size;
    memcpy( m_data, buf, m_size );

    return true;
}


#if wxUSE_DRAG_AND_DROP

#include "wx/dnd.h"



wxTextDropTarget::wxTextDropTarget()
{
    SetDataObject(new wxTextDataObject);
}

wxDragResult wxTextDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
    if ( !GetData() )
        return wxDragNone;

    wxTextDataObject *dobj = (wxTextDataObject *)m_dataObject;
    return OnDropText( x, y, dobj->GetText() ) ? def : wxDragNone;
}


wxFileDropTarget::wxFileDropTarget()
{
    SetDataObject(new wxFileDataObject);
}

wxDragResult wxFileDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
    if ( !GetData() )
        return wxDragNone;

    wxFileDataObject *dobj = (wxFileDataObject *)m_dataObject;
    return OnDropFiles( x, y, dobj->GetFilenames() ) ? def : wxDragNone;
}

#endif 
#endif 