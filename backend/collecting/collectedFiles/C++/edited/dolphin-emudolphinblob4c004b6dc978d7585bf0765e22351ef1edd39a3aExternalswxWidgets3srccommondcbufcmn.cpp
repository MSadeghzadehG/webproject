


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/dcbuffer.h"

#ifndef WX_PRECOMP
    #include "wx/module.h"
#endif


wxIMPLEMENT_DYNAMIC_CLASS(wxBufferedDC, wxMemoryDC);
wxIMPLEMENT_ABSTRACT_CLASS(wxBufferedPaintDC, wxBufferedDC);


class wxSharedDCBufferManager : public wxModule
{
public:
    wxSharedDCBufferManager() { }

    virtual bool OnInit() wxOVERRIDE { return true; }
    virtual void OnExit() wxOVERRIDE { wxDELETE(ms_buffer); }

    static wxBitmap* GetBuffer(wxDC* dc, int w, int h)
    {
        if ( ms_usingSharedBuffer )
            return DoCreateBuffer(dc, w, h);

        if ( !ms_buffer ||
                w > ms_buffer->GetScaledWidth() ||
                    h > ms_buffer->GetScaledHeight() )
        {
            delete ms_buffer;

            ms_buffer = DoCreateBuffer(dc, w, h);
        }

        ms_usingSharedBuffer = true;
        return ms_buffer;
    }

    static void ReleaseBuffer(wxBitmap* buffer)
    {
        if ( buffer == ms_buffer )
        {
            wxASSERT_MSG( ms_usingSharedBuffer, wxT("shared buffer already released") );
            ms_usingSharedBuffer = false;
        }
        else
        {
            delete buffer;
        }
    }

private:
    static wxBitmap* DoCreateBuffer(wxDC* dc, int w, int h)
    {
        const double scale = dc ? dc->GetContentScaleFactor() : 1.0;
        wxBitmap* const buffer = new wxBitmap;

                        buffer->CreateScaled(wxMax(w, 1), wxMax(h, 1), -1, scale);

        return buffer;
    }

    static wxBitmap *ms_buffer;
    static bool ms_usingSharedBuffer;

    wxDECLARE_DYNAMIC_CLASS(wxSharedDCBufferManager);
};

wxBitmap* wxSharedDCBufferManager::ms_buffer = NULL;
bool wxSharedDCBufferManager::ms_usingSharedBuffer = false;

wxIMPLEMENT_DYNAMIC_CLASS(wxSharedDCBufferManager, wxModule);


void wxBufferedDC::UseBuffer(wxCoord w, wxCoord h)
{
    wxCHECK_RET( w >= -1 && h >= -1, "Invalid buffer size" );

    if ( !m_buffer || !m_buffer->IsOk() )
    {
        if ( w == -1 || h == -1 )
            m_dc->GetSize(&w, &h);

        m_buffer = wxSharedDCBufferManager::GetBuffer(m_dc, w, h);
        m_style |= wxBUFFER_USES_SHARED_BUFFER;
        m_area.Set(w,h);
    }
    else
        m_area = m_buffer->GetSize();

    SelectObject(*m_buffer);

            if ( m_dc && m_dc->IsOk() )
        CopyAttributes(*m_dc);
}

void wxBufferedDC::UnMask()
{
    wxCHECK_RET( m_dc, wxT("no underlying wxDC?") );
    wxASSERT_MSG( m_buffer && m_buffer->IsOk(), wxT("invalid backing store") );

    wxCoord x = 0,
            y = 0;

        SetUserScale(1.0, 1.0);

    if ( m_style & wxBUFFER_CLIENT_AREA )
        GetDeviceOrigin(&x, &y);

                        int width = m_area.GetWidth(),
        height = m_area.GetHeight();

    if (!(m_style & wxBUFFER_VIRTUAL_AREA))
    {
        int widthDC,
            heightDC;
        m_dc->GetSize(&widthDC, &heightDC);
        width = wxMin(width, widthDC);
        height = wxMin(height, heightDC);
    }

    const wxPoint origin = GetLogicalOrigin();
    m_dc->Blit(-origin.x, -origin.y, width, height, this, -x, -y);
    m_dc = NULL;

    if ( m_style & wxBUFFER_USES_SHARED_BUFFER )
        wxSharedDCBufferManager::ReleaseBuffer(m_buffer);
}
