


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/log.h"
    #include "wx/intl.h"
#endif 
#include "wx/apptrait.h"
#include "wx/renderer.h"

#include "wx/scopedptr.h"

#if wxUSE_DYNLIB_CLASS
    #include "wx/dynlib.h"
#endif 

typedef wxScopedPtr<wxRendererNative> wxRendererPtrBase;

class wxRendererPtr : public wxRendererPtrBase
{
public:
        bool IsOk()
    {
        if ( !m_initialized )
        {
                        m_initialized = true;

            DoInit();
        }

        return get() != NULL;
    }

        static wxRendererPtr& Get();

private:
    wxRendererPtr() : wxRendererPtrBase(NULL) { m_initialized = false; }

    void DoInit()
    {
        wxAppTraits *traits = wxTheApp ? wxTheApp->GetTraits() : NULL;
        if ( traits )
        {
                        reset(traits->CreateRenderer());
        }
    }

    bool m_initialized;

        friend class wxRendererPtrDummyFriend;

    wxDECLARE_NO_COPY_CLASS(wxRendererPtr);
};

 wxRendererPtr& wxRendererPtr::Get()
{
    static wxRendererPtr s_renderer;

    return s_renderer;
}

#if wxUSE_DYNLIB_CLASS


class wxRendererFromDynLib : public wxDelegateRendererNative
{
public:
                    wxRendererFromDynLib(wxDynamicLibrary& dll, wxRendererNative *renderer)
        : wxDelegateRendererNative(*renderer),
          m_renderer(renderer),
          m_dllHandle(dll.Detach())
        {
        }

    virtual ~wxRendererFromDynLib()
    {
        delete m_renderer;
        wxDynamicLibrary::Unload(m_dllHandle);
    }

private:
    wxRendererNative *m_renderer;
    wxDllType m_dllHandle;
};

#endif 

wxRendererNative::~wxRendererNative()
{
    }



wxRendererNative& wxRendererNative::Get()
{
    wxRendererPtr& renderer = wxRendererPtr::Get();

    return renderer.IsOk() ? *renderer.get() : GetDefault();
}


wxRendererNative *wxRendererNative::Set(wxRendererNative *rendererNew)
{
    wxRendererPtr& renderer = wxRendererPtr::Get();

    wxRendererNative *rendererOld = renderer.release();

    renderer.reset(rendererNew);

    return rendererOld;
}



#if wxUSE_DYNLIB_CLASS


wxRendererNative *wxRendererNative::Load(const wxString& name)
{
    wxString fullname = wxDynamicLibrary::CanonicalizePluginName(name);

    wxDynamicLibrary dll(fullname);
    if ( !dll.IsLoaded() )
        return NULL;

            typedef wxRendererNative *(*wxCreateRenderer_t)();

    wxDYNLIB_FUNCTION(wxCreateRenderer_t, wxCreateRenderer, dll);
    if ( !pfnwxCreateRenderer )
        return NULL;

        wxRendererNative *renderer = (*pfnwxCreateRenderer)();
    if ( !renderer )
        return NULL;

        wxRendererVersion ver = renderer->GetVersion();
    if ( !wxRendererVersion::IsCompatible(ver) )
    {
        wxLogError(_("Renderer \"%s\" has incompatible version %d.%d and couldn't be loaded."),
                   name.c_str(), ver.version, ver.age);
        delete renderer;

        return NULL;
    }

            return new wxRendererFromDynLib(dll, renderer);
}

#endif 