


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/gdicmn.h"
    #include "wx/window.h"
    #include "wx/module.h"
#endif 
#include "wx/display.h"
#include "wx/display_impl.h"

#if wxUSE_DISPLAY

#include "wx/arrimpl.cpp"
WX_DEFINE_OBJARRAY(wxArrayVideoModes)

const wxVideoMode wxDefaultVideoMode;

#endif 

static wxDisplayFactory *gs_factory = NULL;


class WXDLLEXPORT wxDisplayImplSingle : public wxDisplayImpl
{
public:
    wxDisplayImplSingle() : wxDisplayImpl(0) { }

    virtual wxRect GetGeometry() const wxOVERRIDE
    {
        wxRect r;
        wxDisplaySize(&r.width, &r.height);
        return r;
    }

    virtual wxRect GetClientArea() const wxOVERRIDE { return wxGetClientDisplayRect(); }

    virtual wxString GetName() const wxOVERRIDE { return wxString(); }

#if wxUSE_DISPLAY
    
    virtual wxArrayVideoModes GetModes(const wxVideoMode& WXUNUSED(mode)) const wxOVERRIDE
    {
        return wxArrayVideoModes();
    }

    virtual wxVideoMode GetCurrentMode() const wxOVERRIDE { return wxVideoMode(); }

    virtual bool ChangeMode(const wxVideoMode& WXUNUSED(mode)) wxOVERRIDE { return false; }
#endif 

    wxDECLARE_NO_COPY_CLASS(wxDisplayImplSingle);
};


class wxDisplayModule : public wxModule
{
public:
    virtual bool OnInit() wxOVERRIDE { return true; }
    virtual void OnExit() wxOVERRIDE
    {
        wxDELETE(gs_factory);
    }

    wxDECLARE_DYNAMIC_CLASS(wxDisplayModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxDisplayModule, wxModule);



wxDisplay::wxDisplay(unsigned n)
{
    wxASSERT_MSG( n < GetCount(),
                    wxT("An invalid index was passed to wxDisplay") );

    m_impl = Factory().CreateDisplay(n);
}

wxDisplay::~wxDisplay()
{
    delete m_impl;
}


 unsigned wxDisplay::GetCount()
{
    return Factory().GetCount();
}

 int wxDisplay::GetFromPoint(const wxPoint& pt)
{
    return Factory().GetFromPoint(pt);
}

 int wxDisplay::GetFromWindow(const wxWindow *window)
{
    wxCHECK_MSG( window, wxNOT_FOUND, wxT("invalid window") );

    return Factory().GetFromWindow(window);
}


wxRect wxDisplay::GetGeometry() const
{
    wxCHECK_MSG( IsOk(), wxRect(), wxT("invalid wxDisplay object") );

    return m_impl->GetGeometry();
}

wxRect wxDisplay::GetClientArea() const
{
    wxCHECK_MSG( IsOk(), wxRect(), wxT("invalid wxDisplay object") );

    return m_impl->GetClientArea();
}

wxString wxDisplay::GetName() const
{
    wxCHECK_MSG( IsOk(), wxString(), wxT("invalid wxDisplay object") );

    return m_impl->GetName();
}

bool wxDisplay::IsPrimary() const
{
    return m_impl && m_impl->GetIndex() == 0;
}

#if wxUSE_DISPLAY

wxArrayVideoModes wxDisplay::GetModes(const wxVideoMode& mode) const
{
    wxCHECK_MSG( IsOk(), wxArrayVideoModes(), wxT("invalid wxDisplay object") );

    return m_impl->GetModes(mode);
}

wxVideoMode wxDisplay::GetCurrentMode() const
{
    wxCHECK_MSG( IsOk(), wxVideoMode(), wxT("invalid wxDisplay object") );

    return m_impl->GetCurrentMode();
}

bool wxDisplay::ChangeMode(const wxVideoMode& mode)
{
    wxCHECK_MSG( IsOk(), false, wxT("invalid wxDisplay object") );

    return m_impl->ChangeMode(mode);
}

#endif 

#if !wxUSE_DISPLAY

 wxDisplayFactory *wxDisplay::CreateFactory()
{
    return new wxDisplayFactorySingle;
}

#endif 
 wxDisplayFactory& wxDisplay::Factory()
{
    if ( !gs_factory )
    {
        gs_factory = CreateFactory();
    }

    return *gs_factory;
}


int wxDisplayFactory::GetFromWindow(const wxWindow *window)
{
        const wxRect r(window->GetScreenRect());
    return GetFromPoint(wxPoint(r.x + r.width/2, r.y + r.height/2));
}



wxDisplayImpl *wxDisplayFactorySingle::CreateDisplay(unsigned n)
{
        return n != 0 ? NULL : new wxDisplayImplSingle;
}

int wxDisplayFactorySingle::GetFromPoint(const wxPoint& pt)
{
    if ( pt.x >= 0 && pt.y >= 0 )
    {
        int w, h;
        wxDisplaySize(&w, &h);

        if ( pt.x < w && pt.y < h )
            return 0;
    }

        return wxNOT_FOUND;
}
