



#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_MEDIACTRL

#ifndef WX_PRECOMP
    #include "wx/hash.h"
    #include "wx/log.h"
#endif

#include "wx/mediactrl.h"



wxIMPLEMENT_CLASS(wxMediaCtrl, wxControl);
wxDEFINE_EVENT( wxEVT_MEDIA_STATECHANGED, wxMediaEvent );
wxDEFINE_EVENT( wxEVT_MEDIA_PLAY, wxMediaEvent );
wxDEFINE_EVENT( wxEVT_MEDIA_PAUSE, wxMediaEvent );
wxIMPLEMENT_CLASS(wxMediaBackend, wxObject);
wxIMPLEMENT_DYNAMIC_CLASS(wxMediaEvent, wxEvent);
wxDEFINE_EVENT( wxEVT_MEDIA_FINISHED, wxMediaEvent );
wxDEFINE_EVENT( wxEVT_MEDIA_LOADED, wxMediaEvent );
wxDEFINE_EVENT( wxEVT_MEDIA_STOP, wxMediaEvent );


wxMediaBackend::~wxMediaBackend()
{
}

bool wxMediaCtrl::Create(wxWindow* parent, wxWindowID id,
                const wxString& fileName,
                const wxPoint& pos,
                const wxSize& size,
                long style,
                const wxString& szBackend,
                const wxValidator& validator,
                const wxString& name)
{
    if(!szBackend.empty())
    {
        wxClassInfo* pClassInfo = wxClassInfo::FindClass(szBackend);

        if(!pClassInfo || !DoCreate(pClassInfo, parent, id,
                                    pos, size, style, validator, name))
        {
            m_imp = NULL;
            return false;
        }

        if (!fileName.empty())
        {
            if (!Load(fileName))
            {
                wxDELETE(m_imp);
                return false;
            }
        }

        SetInitialSize(size);
        return true;
    }
    else
    {
        wxClassInfo::const_iterator it = wxClassInfo::begin_classinfo();

        const wxClassInfo* classInfo;

        while((classInfo = NextBackend(&it)) != NULL)
        {
            if(!DoCreate(classInfo, parent, id,
                         pos, size, style, validator, name))
                continue;

            if (!fileName.empty())
            {
                if (Load(fileName))
                {
                    SetInitialSize(size);
                    return true;
                }
                else
                    delete m_imp;
            }
            else
            {
                SetInitialSize(size);
                return true;
            }
        }

        m_imp = NULL;
        return false;
    }
}

bool wxMediaCtrl::Create(wxWindow* parent, wxWindowID id,
                         const wxURI& location,
                         const wxPoint& pos,
                         const wxSize& size,
                         long style,
                         const wxString& szBackend,
                         const wxValidator& validator,
                         const wxString& name)
{
    if(!szBackend.empty())
    {
        wxClassInfo* pClassInfo = wxClassInfo::FindClass(szBackend);
        if(!pClassInfo || !DoCreate(pClassInfo, parent, id,
                                    pos, size, style, validator, name))
        {
            m_imp = NULL;
            return false;
        }

        if (!Load(location))
        {
            wxDELETE(m_imp);
            return false;
        }

        SetInitialSize(size);
        return true;
    }
    else
    {
        wxClassInfo::const_iterator it  = wxClassInfo::begin_classinfo();

        const wxClassInfo* classInfo;

        while((classInfo = NextBackend(&it)) != NULL)
        {
            if(!DoCreate(classInfo, parent, id,
                         pos, size, style, validator, name))
                continue;

            if (Load(location))
            {
                SetInitialSize(size);
                return true;
            }
            else
                delete m_imp;
        }

        m_imp = NULL;
        return false;
    }
}

bool wxMediaCtrl::DoCreate(const wxClassInfo* classInfo,
                            wxWindow* parent, wxWindowID id,
                            const wxPoint& pos,
                            const wxSize& size,
                            long style,
                            const wxValidator& validator,
                            const wxString& name)
{
    m_imp = (wxMediaBackend*)classInfo->CreateObject();

    if( m_imp->CreateControl(this, parent, id, pos, size,
                             style, validator, name) )
    {
        return true;
    }

    delete m_imp;
    return false;
}

const wxClassInfo* wxMediaCtrl::NextBackend(wxClassInfo::const_iterator* it)
{
    for ( wxClassInfo::const_iterator end = wxClassInfo::end_classinfo();
          *it != end; ++(*it) )
    {
        const wxClassInfo* classInfo = **it;
        if ( classInfo->IsKindOf(wxCLASSINFO(wxMediaBackend))  &&
             classInfo != wxCLASSINFO(wxMediaBackend) )
        {
            ++(*it);
            return classInfo;
        }
    }

                return NULL;
}


wxMediaCtrl::~wxMediaCtrl()
{
    if (m_imp)
        delete m_imp;
}

bool wxMediaCtrl::Load(const wxString& fileName)
{
    if(m_imp)
        return (m_bLoaded = m_imp->Load(fileName));
    return false;
}

bool wxMediaCtrl::Load(const wxURI& location)
{
    if(m_imp)
        return (m_bLoaded = m_imp->Load(location));
    return false;
}

bool wxMediaCtrl::Load(const wxURI& location, const wxURI& proxy)
{
    if(m_imp)
        return (m_bLoaded = m_imp->Load(location, proxy));
    return false;
}

bool wxMediaCtrl::Play()
{
    if(m_imp && m_bLoaded)
        return m_imp->Play();
    return 0;
}

bool wxMediaCtrl::Pause()
{
    if(m_imp && m_bLoaded)
        return m_imp->Pause();
    return 0;
}

bool wxMediaCtrl::Stop()
{
    if(m_imp && m_bLoaded)
        return m_imp->Stop();
    return 0;
}

double wxMediaCtrl::GetPlaybackRate()
{
    if(m_imp && m_bLoaded)
        return m_imp->GetPlaybackRate();
    return 0;
}

bool wxMediaCtrl::SetPlaybackRate(double dRate)
{
    if(m_imp && m_bLoaded)
        return m_imp->SetPlaybackRate(dRate);
    return false;
}

wxFileOffset wxMediaCtrl::Seek(wxFileOffset where, wxSeekMode mode)
{
    wxFileOffset offset;

    switch (mode)
    {
    case wxFromStart:
        offset = where;
        break;
    case wxFromEnd:
        offset = Length() - where;
        break;
    default:
        offset = Tell() + where;
        break;
    }

    if(m_imp && m_bLoaded && m_imp->SetPosition(offset))
        return offset;
    return wxInvalidOffset;
}

wxFileOffset wxMediaCtrl::Tell()
{
    if(m_imp && m_bLoaded)
        return (wxFileOffset) m_imp->GetPosition().ToLong();
    return wxInvalidOffset;
}

wxFileOffset wxMediaCtrl::Length()
{
    if(m_imp && m_bLoaded)
        return (wxFileOffset) m_imp->GetDuration().ToLong();
    return wxInvalidOffset;
}

wxMediaState wxMediaCtrl::GetState()
{
    if(m_imp && m_bLoaded)
        return m_imp->GetState();
    return wxMEDIASTATE_STOPPED;
}

wxSize wxMediaCtrl::DoGetBestSize() const
{
    if(m_imp)
        return m_imp->GetVideoSize();
    return wxSize(0,0);
}

double wxMediaCtrl::GetVolume()
{
    if(m_imp && m_bLoaded)
        return m_imp->GetVolume();
    return 0.0;
}

bool wxMediaCtrl::SetVolume(double dVolume)
{
    if(m_imp && m_bLoaded)
        return m_imp->SetVolume(dVolume);
    return false;
}

bool wxMediaCtrl::ShowPlayerControls(wxMediaCtrlPlayerControls flags)
{
    if(m_imp)
        return m_imp->ShowPlayerControls(flags);
    return false;
}

wxFileOffset wxMediaCtrl::GetDownloadProgress()
{
    if(m_imp && m_bLoaded)
        return (wxFileOffset) m_imp->GetDownloadProgress().ToLong();
    return wxInvalidOffset;
}

wxFileOffset wxMediaCtrl::GetDownloadTotal()
{
    if(m_imp && m_bLoaded)
        return (wxFileOffset) m_imp->GetDownloadTotal().ToLong();
    return wxInvalidOffset;
}

void wxMediaCtrl::DoMoveWindow(int x, int y, int w, int h)
{
    wxControl::DoMoveWindow(x,y,w,h);

    if(m_imp)
        m_imp->Move(x, y, w, h);
}


void wxMediaBackendCommonBase::NotifyMovieSizeChanged()
{
        m_ctrl->InvalidateBestSize();
    m_ctrl->SetSize(m_ctrl->GetSize());

        wxWindow * const parent = m_ctrl->GetParent();
    if ( parent->GetSizer() )
    {
        m_ctrl->GetParent()->Layout();
        m_ctrl->GetParent()->Refresh();
        m_ctrl->GetParent()->Update();
    }
}

void wxMediaBackendCommonBase::NotifyMovieLoaded()
{
    NotifyMovieSizeChanged();

        QueueEvent(wxEVT_MEDIA_LOADED);
}

bool wxMediaBackendCommonBase::SendStopEvent()
{
    wxMediaEvent theEvent(wxEVT_MEDIA_STOP, m_ctrl->GetId());

    return !m_ctrl->GetEventHandler()->ProcessEvent(theEvent) || theEvent.IsAllowed();
}

void wxMediaBackendCommonBase::QueueEvent(wxEventType evtType)
{
    wxMediaEvent theEvent(evtType, m_ctrl->GetId());
    m_ctrl->GetEventHandler()->AddPendingEvent(theEvent);
}

void wxMediaBackendCommonBase::QueuePlayEvent()
{
    QueueEvent(wxEVT_MEDIA_STATECHANGED);
    QueueEvent(wxEVT_MEDIA_PLAY);
}

void wxMediaBackendCommonBase::QueuePauseEvent()
{
    QueueEvent(wxEVT_MEDIA_STATECHANGED);
    QueueEvent(wxEVT_MEDIA_PAUSE);
}

void wxMediaBackendCommonBase::QueueStopEvent()
{
    QueueEvent(wxEVT_MEDIA_STATECHANGED);
    QueueEvent(wxEVT_MEDIA_STOP);
}


#include "wx/html/forcelnk.h"

#ifdef __WXMSW__ FORCE_LINK(wxmediabackend_am)
FORCE_LINK(wxmediabackend_wmp10)
#else
FORCE_LINK(basewxmediabackends)
#endif

#endif 