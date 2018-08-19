
#include "wx/wxprec.h"

#if wxUSE_SOUND

#include "wx/sound.h"

#ifndef WX_PRECOMP
    #include "wx/object.h"
    #include "wx/string.h"
    #include "wx/intl.h"
    #include "wx/log.h"
#endif

#include "wx/file.h"

#include "wx/vector.h"

wxVector<wxSoundData*> s_soundsPlaying;

wxSoundData::wxSoundData()
{
    m_markedForDeletion = false;
}

wxSoundData::~wxSoundData()
{
}

void wxSoundData::MarkForDeletion()
{
    m_markedForDeletion = true;
}

void wxSoundData::Stop()
{
    DoStop();
}

wxSound::wxSound()
{
    Init();
}

wxSound::wxSound(const wxString& sFileName, bool isResource)
{
    Init();
    Create(sFileName, isResource);
}

wxSound::wxSound(size_t size, const void* data)
{
    Init();
    Create( size, data );
}

wxSound::~wxSound()
{
                bool isPlaying = false;
    for ( wxVector<wxSoundData*>::reverse_iterator s = s_soundsPlaying.rbegin();
         s != s_soundsPlaying.rend(); ++s )
    {
        if (*s == m_data)
        {
            isPlaying = true;
            break;
        }
    }

    if (isPlaying)
        m_data->MarkForDeletion();
    else
        delete m_data;
}

void wxSound::Init()
{
    m_data = NULL;
}

bool wxSound::DoPlay(unsigned flags) const
{
    if ( m_data )
    {
        s_soundsPlaying.push_back(m_data);
        if ( !m_data->Play(flags) )
            s_soundsPlaying.pop_back();
    }

    return false;
}

bool wxSound::IsPlaying()
{
    return s_soundsPlaying.size() > 0;
}

void wxSound::Stop()
{
    for ( wxVector<wxSoundData*>::reverse_iterator s = s_soundsPlaying.rbegin();
         s != s_soundsPlaying.rend(); ++s )
    {
        (*s)->Stop();
    }
}

void wxSound::SoundStopped(const wxSoundData* data)
{
    for ( wxVector<wxSoundData*>::iterator s = s_soundsPlaying.begin();
         s != s_soundsPlaying.end(); ++s )
    {
        if ( (*s) == data )
        {
            s_soundsPlaying.erase(s);
            break;
        }
    }
}

#endif 