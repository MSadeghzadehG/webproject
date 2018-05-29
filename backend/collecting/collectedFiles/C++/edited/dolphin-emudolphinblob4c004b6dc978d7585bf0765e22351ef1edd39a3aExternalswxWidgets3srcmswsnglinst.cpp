


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_SNGLINST_CHECKER

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/log.h"
#endif 
#include "wx/snglinst.h"

#include "wx/msw/private.h"


class WXDLLIMPEXP_BASE wxSingleInstanceCheckerImpl
{
public:
    wxSingleInstanceCheckerImpl()
    {
                        m_hMutex = NULL;
    }

    bool Create(const wxString& name)
    {
        m_hMutex = ::CreateMutex(NULL, FALSE, name.t_str());
        if ( !m_hMutex )
        {
            wxLogLastError(wxT("CreateMutex"));

            return false;
        }

                m_wasOpened = ::GetLastError() == ERROR_ALREADY_EXISTS;

        return true;
    }

    bool WasOpened() const
    {
        wxCHECK_MSG( m_hMutex, false,
                     wxT("can't be called if mutex creation failed") );

        return m_wasOpened;
    }

    ~wxSingleInstanceCheckerImpl()
    {
        if ( m_hMutex )
        {
            if ( !::CloseHandle(m_hMutex) )
            {
                wxLogLastError(wxT("CloseHandle(mutex)"));
            }
        }
    }

private:
        bool m_wasOpened;

        HANDLE m_hMutex;

    wxDECLARE_NO_COPY_CLASS(wxSingleInstanceCheckerImpl);
};


bool wxSingleInstanceChecker::Create(const wxString& name,
                                     const wxString& WXUNUSED(path))
{
    wxASSERT_MSG( !m_impl,
                  wxT("calling wxSingleInstanceChecker::Create() twice?") );

        wxASSERT_MSG( !name.empty(), wxT("mutex name can't be empty") );

    m_impl = new wxSingleInstanceCheckerImpl;

    return m_impl->Create(name);
}

bool wxSingleInstanceChecker::DoIsAnotherRunning() const
{
    wxCHECK_MSG( m_impl, false, wxT("must call Create() first") );

            return m_impl->WasOpened();
}

wxSingleInstanceChecker::~wxSingleInstanceChecker()
{
    delete m_impl;
}

#endif 