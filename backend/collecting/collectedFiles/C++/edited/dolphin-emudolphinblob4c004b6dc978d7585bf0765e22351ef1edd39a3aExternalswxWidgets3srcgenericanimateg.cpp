
#include "wx/wxprec.h"

#ifdef __BORLANDC__
  #pragma hdrstop
#endif  
#if wxUSE_ANIMATIONCTRL

#include "wx/animate.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/image.h"
    #include "wx/dcmemory.h"
    #include "wx/dcclient.h"
    #include "wx/module.h"
#endif

#include "wx/wfstream.h"
#include "wx/gifdecod.h"
#include "wx/anidecod.h"

#include "wx/listimpl.cpp"
WX_DEFINE_LIST(wxAnimationDecoderList)

wxAnimationDecoderList wxAnimation::sm_handlers;



wxIMPLEMENT_DYNAMIC_CLASS(wxAnimation, wxAnimationBase);
#define M_ANIMDATA      static_cast<wxAnimationDecoder*>(m_refData)

wxSize wxAnimation::GetSize() const
{
    wxCHECK_MSG( IsOk(), wxDefaultSize, wxT("invalid animation") );

    return M_ANIMDATA->GetAnimationSize();
}

unsigned int wxAnimation::GetFrameCount() const
{
    wxCHECK_MSG( IsOk(), 0, wxT("invalid animation") );

    return M_ANIMDATA->GetFrameCount();
}

wxImage wxAnimation::GetFrame(unsigned int i) const
{
    wxCHECK_MSG( IsOk(), wxNullImage, wxT("invalid animation") );

    wxImage ret;
    if (!M_ANIMDATA->ConvertToImage(i, &ret))
        return wxNullImage;
    return ret;
}

int wxAnimation::GetDelay(unsigned int i) const
{
    wxCHECK_MSG( IsOk(), 0, wxT("invalid animation") );

    return M_ANIMDATA->GetDelay(i);
}

wxPoint wxAnimation::GetFramePosition(unsigned int frame) const
{
    wxCHECK_MSG( IsOk(), wxDefaultPosition, wxT("invalid animation") );

    return M_ANIMDATA->GetFramePosition(frame);
}

wxSize wxAnimation::GetFrameSize(unsigned int frame) const
{
    wxCHECK_MSG( IsOk(), wxDefaultSize, wxT("invalid animation") );

    return M_ANIMDATA->GetFrameSize(frame);
}

wxAnimationDisposal wxAnimation::GetDisposalMethod(unsigned int frame) const
{
    wxCHECK_MSG( IsOk(), wxANIM_UNSPECIFIED, wxT("invalid animation") );

    return M_ANIMDATA->GetDisposalMethod(frame);
}

wxColour wxAnimation::GetTransparentColour(unsigned int frame) const
{
    wxCHECK_MSG( IsOk(), wxNullColour, wxT("invalid animation") );

    return M_ANIMDATA->GetTransparentColour(frame);
}

wxColour wxAnimation::GetBackgroundColour() const
{
    wxCHECK_MSG( IsOk(), wxNullColour, wxT("invalid animation") );

    return M_ANIMDATA->GetBackgroundColour();
}

bool wxAnimation::LoadFile(const wxString& filename, wxAnimationType type)
{
    wxFileInputStream stream(filename);
    if ( !stream.IsOk() )
        return false;

    return Load(stream, type);
}

bool wxAnimation::Load(wxInputStream &stream, wxAnimationType type)
{
    UnRef();

    const wxAnimationDecoder *handler;
    if ( type == wxANIMATION_TYPE_ANY )
    {
        for ( wxAnimationDecoderList::compatibility_iterator node = sm_handlers.GetFirst();
              node; node = node->GetNext() )
        {
            handler=(const wxAnimationDecoder*)node->GetData();

            if ( handler->CanRead(stream) )
            {
                                                m_refData = handler->Clone();
                return M_ANIMDATA->Load(stream);
            }
        }

        wxLogWarning( _("No handler found for animation type.") );
        return false;
    }

    handler = FindHandler(type);

    if (handler == NULL)
    {
        wxLogWarning( _("No animation handler for type %ld defined."), type );

        return false;
    }


            m_refData = handler->Clone();

    if (stream.IsSeekable() && !M_ANIMDATA->CanRead(stream))
    {
        wxLogError(_("Animation file is not of type %ld."), type);
        return false;
    }
    else
        return M_ANIMDATA->Load(stream);
}



void wxAnimation::AddHandler( wxAnimationDecoder *handler )
{
        if (FindHandler( handler->GetType() ) == 0)
    {
        sm_handlers.Append( handler );
    }
    else
    {
                                
        wxLogDebug( wxT("Adding duplicate animation handler for '%d' type"),
                    handler->GetType() );
        delete handler;
    }
}

void wxAnimation::InsertHandler( wxAnimationDecoder *handler )
{
        if (FindHandler( handler->GetType() ) == 0)
    {
        sm_handlers.Insert( handler );
    }
    else
    {
                wxLogDebug( wxT("Inserting duplicate animation handler for '%d' type"),
                    handler->GetType() );
        delete handler;
    }
}

const wxAnimationDecoder *wxAnimation::FindHandler( wxAnimationType animType )
{
    wxAnimationDecoderList::compatibility_iterator node = sm_handlers.GetFirst();
    while (node)
    {
        const wxAnimationDecoder *handler = (const wxAnimationDecoder *)node->GetData();
        if (handler->GetType() == animType) return handler;
        node = node->GetNext();
    }
    return 0;
}

void wxAnimation::InitStandardHandlers()
{
#if wxUSE_GIF
    AddHandler(new wxGIFDecoder);
#endif #if wxUSE_ICO_CUR
    AddHandler(new wxANIDecoder);
#endif }

void wxAnimation::CleanUpHandlers()
{
    wxAnimationDecoderList::compatibility_iterator node = sm_handlers.GetFirst();
    while (node)
    {
        wxAnimationDecoder *handler = (wxAnimationDecoder *)node->GetData();
        wxAnimationDecoderList::compatibility_iterator next = node->GetNext();
        delete handler;
        node = next;
    }

    sm_handlers.Clear();
}



class wxAnimationModule: public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxAnimationModule);
public:
    wxAnimationModule() {}
    bool OnInit() wxOVERRIDE { wxAnimation::InitStandardHandlers(); return true; }
    void OnExit() wxOVERRIDE { wxAnimation::CleanUpHandlers(); }
};

wxIMPLEMENT_DYNAMIC_CLASS(wxAnimationModule, wxModule);



wxIMPLEMENT_CLASS(wxAnimationCtrl, wxAnimationCtrlBase);
wxBEGIN_EVENT_TABLE(wxAnimationCtrl, wxAnimationCtrlBase)
    EVT_PAINT(wxAnimationCtrl::OnPaint)
    EVT_SIZE(wxAnimationCtrl::OnSize)
    EVT_TIMER(wxID_ANY, wxAnimationCtrl::OnTimer)
wxEND_EVENT_TABLE()

void wxAnimationCtrl::Init()
{
    m_currentFrame = 0;
    m_looped = false;
    m_isPlaying = false;

            m_useWinBackgroundColour = true;
}

bool wxAnimationCtrl::Create(wxWindow *parent, wxWindowID id,
            const wxAnimation& animation, const wxPoint& pos,
            const wxSize& size, long style, const wxString& name)
{
    m_timer.SetOwner(this);

    if (!base_type::Create(parent, id, pos, size, style, wxDefaultValidator, name))
        return false;

        SetBackgroundColour(parent->GetBackgroundColour());

    SetAnimation(animation);

    return true;
}

wxAnimationCtrl::~wxAnimationCtrl()
{
    Stop();
}

bool wxAnimationCtrl::LoadFile(const wxString& filename, wxAnimationType type)
{
    wxFileInputStream fis(filename);
    if (!fis.IsOk())
        return false;
    return Load(fis, type);
}

bool wxAnimationCtrl::Load(wxInputStream& stream, wxAnimationType type)
{
    wxAnimation anim;
    if ( !anim.Load(stream, type) || !anim.IsOk() )
        return false;

    SetAnimation(anim);
    return true;
}

wxSize wxAnimationCtrl::DoGetBestSize() const
{
    if (m_animation.IsOk() && !this->HasFlag(wxAC_NO_AUTORESIZE))
        return m_animation.GetSize();

    return wxSize(100, 100);
}

void wxAnimationCtrl::SetAnimation(const wxAnimation& animation)
{
    if (IsPlaying())
        Stop();

        m_animation = animation;
    if (!m_animation.IsOk())
    {
        DisplayStaticImage();
        return;
    }

    if (m_animation.GetBackgroundColour() == wxNullColour)
        SetUseWindowBackgroundColour();
    if (!this->HasFlag(wxAC_NO_AUTORESIZE))
        FitToAnimation();

    DisplayStaticImage();
}

void wxAnimationCtrl::SetInactiveBitmap(const wxBitmap &bmp)
{
                        if ( bmp.IsOk() && bmp.GetMask() != NULL && GetParent() != NULL )
        SetBackgroundColour(GetParent()->GetBackgroundColour());

    wxAnimationCtrlBase::SetInactiveBitmap(bmp);
}

void wxAnimationCtrl::FitToAnimation()
{
    SetSize(m_animation.GetSize());
}

bool wxAnimationCtrl::SetBackgroundColour(const wxColour& colour)
{
    if ( !wxWindow::SetBackgroundColour(colour) )
        return false;

            if ( !IsPlaying() )
        DisplayStaticImage();

    return true;
}



void wxAnimationCtrl::Stop()
{
    m_timer.Stop();
    m_isPlaying = false;

        m_currentFrame = 0;

    DisplayStaticImage();
}

bool wxAnimationCtrl::Play(bool looped)
{
    if (!m_animation.IsOk())
        return false;

    m_looped = looped;
    m_currentFrame = 0;

    if (!RebuildBackingStoreUpToFrame(0))
        return false;

    m_isPlaying = true;

            ClearBackground();

        wxClientDC clientDC(this);
    DrawCurrentFrame(clientDC);

        int delay = m_animation.GetDelay(0);
    if (delay == 0)
        delay = 1;          m_timer.Start(delay, true);

    return true;
}




bool wxAnimationCtrl::RebuildBackingStoreUpToFrame(unsigned int frame)
{
            wxSize sz = m_animation.GetSize(),
           winsz = GetClientSize();
    int w = wxMin(sz.GetWidth(), winsz.GetWidth());
    int h = wxMin(sz.GetHeight(), winsz.GetHeight());

    if ( !m_backingStore.IsOk() ||
            m_backingStore.GetWidth() < w || m_backingStore.GetHeight() < h )
    {
        if (!m_backingStore.Create(w, h))
            return false;
    }

    wxMemoryDC dc;
    dc.SelectObject(m_backingStore);

        DisposeToBackground(dc);

        for (unsigned int i = 0; i < frame; i++)
    {
        if (m_animation.GetDisposalMethod(i) == wxANIM_DONOTREMOVE ||
            m_animation.GetDisposalMethod(i) == wxANIM_UNSPECIFIED)
        {
            DrawFrame(dc, i);
        }
        else if (m_animation.GetDisposalMethod(i) == wxANIM_TOBACKGROUND)
            DisposeToBackground(dc, m_animation.GetFramePosition(i),
                                    m_animation.GetFrameSize(i));
    }

        DrawFrame(dc, frame);
    dc.SelectObject(wxNullBitmap);

    return true;
}

void wxAnimationCtrl::IncrementalUpdateBackingStore()
{
    wxMemoryDC dc;
    dc.SelectObject(m_backingStore);

                    
    if (m_currentFrame == 0)
    {
                DisposeToBackground(dc);
    }
    else
    {
        switch (m_animation.GetDisposalMethod(m_currentFrame-1))
        {
        case wxANIM_TOBACKGROUND:
            DisposeToBackground(dc, m_animation.GetFramePosition(m_currentFrame-1),
                                    m_animation.GetFrameSize(m_currentFrame-1));
            break;

        case wxANIM_TOPREVIOUS:
                                                            if (m_currentFrame == 1)
            {
                                                DisposeToBackground(dc);
            }
            else
                if (!RebuildBackingStoreUpToFrame(m_currentFrame-2))
                    Stop();
            break;

        case wxANIM_DONOTREMOVE:
        case wxANIM_UNSPECIFIED:
            break;
        }
    }

        DrawFrame(dc, m_currentFrame);
    dc.SelectObject(wxNullBitmap);
}

void wxAnimationCtrl::DisplayStaticImage()
{
    wxASSERT(!IsPlaying());

        UpdateStaticImage();

    if (m_bmpStaticReal.IsOk())
    {
                        if ( m_bmpStaticReal.GetMask() )
        {
            wxMemoryDC temp;
            temp.SelectObject(m_backingStore);
            DisposeToBackground(temp);
            temp.DrawBitmap(m_bmpStaticReal, 0, 0, true );
        }
        else
            m_backingStore = m_bmpStaticReal;
    }
    else
    {
                if (!m_animation.IsOk() ||
            !RebuildBackingStoreUpToFrame(0))
        {
            m_animation = wxNullAnimation;
            DisposeToBackground();
        }
    }

    Refresh();
}

void wxAnimationCtrl::DrawFrame(wxDC &dc, unsigned int frame)
{
                            wxBitmap bmp(m_animation.GetFrame(frame));
    dc.DrawBitmap(bmp, m_animation.GetFramePosition(frame),
                  true );
}

void wxAnimationCtrl::DrawCurrentFrame(wxDC& dc)
{
    wxASSERT( m_backingStore.IsOk() );

        dc.DrawBitmap(m_backingStore, 0, 0, true );
}

void wxAnimationCtrl::DisposeToBackground()
{
        wxMemoryDC dc;
    dc.SelectObject(m_backingStore);
    if ( dc.IsOk() )
        DisposeToBackground(dc);
}

void wxAnimationCtrl::DisposeToBackground(wxDC& dc)
{
    wxColour col = IsUsingWindowBackgroundColour()
                    ? GetBackgroundColour()
                    : m_animation.GetBackgroundColour();

    wxBrush brush(col);
    dc.SetBackground(brush);
    dc.Clear();
}

void wxAnimationCtrl::DisposeToBackground(wxDC& dc, const wxPoint &pos, const wxSize &sz)
{
    wxColour col = IsUsingWindowBackgroundColour()
                    ? GetBackgroundColour()
                    : m_animation.GetBackgroundColour();
    wxBrush brush(col);
    dc.SetBrush(brush);             dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(pos, sz);
}


void wxAnimationCtrl::OnPaint(wxPaintEvent& WXUNUSED(event))
{
        wxPaintDC dc(this);

    if ( m_backingStore.IsOk() )
    {
                                dc.DrawBitmap(m_backingStore, 0, 0, false );
    }
    else
    {
                        DisposeToBackground(dc);
    }
}

void wxAnimationCtrl::OnTimer(wxTimerEvent &WXUNUSED(event))
{
    m_currentFrame++;
    if (m_currentFrame == m_animation.GetFrameCount())
    {
                if (!m_looped)
        {
            Stop();
            return;
        }
        else
            m_currentFrame = 0;         }

    IncrementalUpdateBackingStore();

    wxClientDC dc(this);
    DrawCurrentFrame(dc);

#ifdef __WXMAC__
        Refresh();
#endif 
        int delay = m_animation.GetDelay(m_currentFrame);
    if (delay == 0)
        delay = 1;          m_timer.Start(delay, true);
}

void wxAnimationCtrl::OnSize(wxSizeEvent &WXUNUSED(event))
{
                        if (m_animation.IsOk())
    {
                                        if (IsPlaying())
        {
            if (!RebuildBackingStoreUpToFrame(m_currentFrame))
                Stop();             }
    }
}

#endif 
