


#include "wx/wxprec.h"

#if defined(__BORLANDC__)
    #pragma hdrstop
#endif

#if wxUSE_SOUND

#include "wx/sound.h"
#include "wx/msw/private.h"

#include <mmsystem.h>


class wxSoundData
{
public:
    wxSoundData() { }

        virtual bool IsOk() const = 0;

        virtual DWORD GetSoundFlag() const = 0;

        virtual LPCTSTR GetSoundData() const = 0;

    virtual ~wxSoundData() { }
};

class wxSoundDataMemory : public wxSoundData
{
public:
        wxSoundDataMemory(size_t size, const void* buf);

    void *GetPtr() const { return m_waveDataPtr; }

    virtual bool IsOk() const { return GetPtr() != NULL; }
    virtual DWORD GetSoundFlag() const { return SND_MEMORY; }
    virtual LPCTSTR GetSoundData() const { return (LPCTSTR)GetPtr(); }

private:
    GlobalPtr m_waveData;
    GlobalPtrLock m_waveDataPtr;

    wxDECLARE_NO_COPY_CLASS(wxSoundDataMemory);
};

class wxSoundDataFile : public wxSoundData
{
public:
    wxSoundDataFile(const wxString& filename, bool isResource);

    virtual bool IsOk() const { return !m_name.empty(); }
    virtual DWORD GetSoundFlag() const
    {
        return m_isResource ? SND_RESOURCE : SND_FILENAME;
    }
    virtual LPCTSTR GetSoundData() const { return m_name.c_str(); }

private:
    const wxString m_name;
    const bool m_isResource;

    wxDECLARE_NO_COPY_CLASS(wxSoundDataFile);
};



wxSoundDataMemory::wxSoundDataMemory(size_t size, const void* buf)
                 : m_waveData(size),
                   m_waveDataPtr(m_waveData)
{
    if ( IsOk() )
        ::CopyMemory(m_waveDataPtr, buf, size);
}

wxSoundDataFile::wxSoundDataFile(const wxString& filename, bool isResource)
               : m_name(filename),
                 m_isResource(isResource)
{
    }


wxSound::wxSound()
{
    Init();
}

wxSound::wxSound(const wxString& filename, bool isResource)
{
    Init();
    Create(filename, isResource);
}

wxSound::wxSound(size_t size, const void* data)
{
    Init();
    Create(size, data);
}

wxSound::~wxSound()
{
    Free();
}

void wxSound::Free()
{
    wxDELETE(m_data);
}

bool wxSound::CheckCreatedOk()
{
    if ( m_data && !m_data->IsOk() )
        Free();

    return m_data != NULL;
}

bool wxSound::Create(const wxString& filename, bool isResource)
{
    Free();

    m_data = new wxSoundDataFile(filename, isResource);

    return CheckCreatedOk();
}

bool wxSound::Create(size_t size, const void* data)
{
    Free();

    m_data = new wxSoundDataMemory(size, data);

    return CheckCreatedOk();
}

bool wxSound::DoPlay(unsigned flags) const
{
    if ( !IsOk() || !m_data->IsOk() )
        return false;

    DWORD flagsMSW = m_data->GetSoundFlag();
    HMODULE hmod = flagsMSW == SND_RESOURCE ? wxGetInstance() : NULL;

        flagsMSW |= SND_NODEFAULT;

        flagsMSW |= (flags & wxSOUND_ASYNC) ? SND_ASYNC : SND_SYNC;
    if ( flags & wxSOUND_LOOP )
    {
                flagsMSW |= SND_LOOP | SND_ASYNC;
    }

    return ::PlaySound(m_data->GetSoundData(), hmod, flagsMSW) != FALSE;
}


void wxSound::Stop()
{
    ::PlaySound(NULL, NULL, 0);
}

#endif 
