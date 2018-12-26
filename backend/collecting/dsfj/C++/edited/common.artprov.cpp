

#include "wx/wxprec.h"

#if defined(__BORLANDC__)
    #pragma hdrstop
#endif

#include "wx/artprov.h"

#ifndef WX_PRECOMP
    #include "wx/list.h"
    #include "wx/log.h"
    #include "wx/hashmap.h"
    #include "wx/image.h"
    #include "wx/module.h"
#endif


#include "wx/listimpl.cpp"
WX_DECLARE_LIST(wxArtProvider, wxArtProvidersList);
WX_DEFINE_LIST(wxArtProvidersList)


WX_DECLARE_EXPORTED_STRING_HASH_MAP(wxBitmap, wxArtProviderBitmapsHash);
WX_DECLARE_EXPORTED_STRING_HASH_MAP(wxIconBundle, wxArtProviderIconBundlesHash);

class WXDLLEXPORT wxArtProviderCache
{
public:
    bool GetBitmap(const wxString& full_id, wxBitmap* bmp);
    void PutBitmap(const wxString& full_id, const wxBitmap& bmp)
        { m_bitmapsHash[full_id] = bmp; }

    bool GetIconBundle(const wxString& full_id, wxIconBundle* bmp);
    void PutIconBundle(const wxString& full_id, const wxIconBundle& iconbundle)
        { m_iconBundlesHash[full_id] = iconbundle; }

    void Clear();

    static wxString ConstructHashID(const wxArtID& id,
                                    const wxArtClient& client,
                                    const wxSize& size);

    static wxString ConstructHashID(const wxArtID& id,
                                    const wxArtClient& client);

private:
    wxArtProviderBitmapsHash m_bitmapsHash;             wxArtProviderIconBundlesHash m_iconBundlesHash; };

bool wxArtProviderCache::GetBitmap(const wxString& full_id, wxBitmap* bmp)
{
    wxArtProviderBitmapsHash::iterator entry = m_bitmapsHash.find(full_id);
    if ( entry == m_bitmapsHash.end() )
    {
        return false;
    }
    else
    {
        *bmp = entry->second;
        return true;
    }
}

bool wxArtProviderCache::GetIconBundle(const wxString& full_id, wxIconBundle* bmp)
{
    wxArtProviderIconBundlesHash::iterator entry = m_iconBundlesHash.find(full_id);
    if ( entry == m_iconBundlesHash.end() )
    {
        return false;
    }
    else
    {
        *bmp = entry->second;
        return true;
    }
}

void wxArtProviderCache::Clear()
{
    m_bitmapsHash.clear();
    m_iconBundlesHash.clear();
}

 wxString
wxArtProviderCache::ConstructHashID(const wxArtID& id,
                                    const wxArtClient& client)
{
    return id + wxT('-') + client;
}


 wxString
wxArtProviderCache::ConstructHashID(const wxArtID& id,
                                    const wxArtClient& client,
                                    const wxSize& size)
{
    return ConstructHashID(id, client) + wxT('-') +
            wxString::Format(wxT("%d-%d"), size.x, size.y);
}


wxIMPLEMENT_ABSTRACT_CLASS(wxArtProvider, wxObject);

wxArtProvidersList *wxArtProvider::sm_providers = NULL;
wxArtProviderCache *wxArtProvider::sm_cache = NULL;


wxArtProvider::~wxArtProvider()
{
    Remove(this);
}


 void wxArtProvider::CommonAddingProvider()
{
    if ( !sm_providers )
    {
        sm_providers = new wxArtProvidersList;
        sm_cache = new wxArtProviderCache;
    }

    sm_cache->Clear();
}

 void wxArtProvider::Push(wxArtProvider *provider)
{
    CommonAddingProvider();
    sm_providers->Insert(provider);
}

 void wxArtProvider::PushBack(wxArtProvider *provider)
{
    CommonAddingProvider();
    sm_providers->Append(provider);
}

 bool wxArtProvider::Pop()
{
    wxCHECK_MSG( sm_providers, false, wxT("no wxArtProvider exists") );
    wxCHECK_MSG( !sm_providers->empty(), false, wxT("wxArtProviders stack is empty") );

    delete sm_providers->GetFirst()->GetData();
    sm_cache->Clear();
    return true;
}

 bool wxArtProvider::Remove(wxArtProvider *provider)
{
    wxCHECK_MSG( sm_providers, false, wxT("no wxArtProvider exists") );

    if ( sm_providers->DeleteObject(provider) )
    {
        sm_cache->Clear();
        return true;
    }

    return false;
}

 bool wxArtProvider::Delete(wxArtProvider *provider)
{
        delete provider;

    return true;
}

 void wxArtProvider::CleanUpProviders()
{
    if ( sm_providers )
    {
        while ( !sm_providers->empty() )
            delete *sm_providers->begin();

        wxDELETE(sm_providers);
        wxDELETE(sm_cache);
    }
}


void wxArtProvider::RescaleBitmap(wxBitmap& bmp, const wxSize& sizeNeeded)
{
    wxCHECK_RET( sizeNeeded.IsFullySpecified(), wxS("New size must be given") );

#if wxUSE_IMAGE
    wxImage img = bmp.ConvertToImage();
    img.Rescale(sizeNeeded.x, sizeNeeded.y);
    bmp = wxBitmap(img);
#else         wxBitmap newBmp(sizeNeeded, bmp.GetDepth());
#if defined(__WXMSW__) || defined(__WXOSX__)
        newBmp.UseAlpha(bmp.HasAlpha());
#endif     {
        wxMemoryDC dc(newBmp);
        double scX = (double)sizeNeeded.GetWidth() / bmp.GetWidth();
        double scY = (double)sizeNeeded.GetHeight() / bmp.GetHeight();
        dc.SetUserScale(scX, scY);
        dc.DrawBitmap(bmp, 0, 0);
    }
    bmp = newBmp;
#endif }

 wxBitmap wxArtProvider::GetBitmap(const wxArtID& id,
                                             const wxArtClient& client,
                                             const wxSize& size)
{
        wxASSERT_MSG( client.Last() == wxT('C'), wxT("invalid 'client' parameter") );

    wxCHECK_MSG( sm_providers, wxNullBitmap, wxT("no wxArtProvider exists") );

    wxString hashId = wxArtProviderCache::ConstructHashID(id, client, size);

    wxBitmap bmp;
    if ( !sm_cache->GetBitmap(hashId, &bmp) )
    {
        for (wxArtProvidersList::compatibility_iterator node = sm_providers->GetFirst();
             node; node = node->GetNext())
        {
            bmp = node->GetData()->CreateBitmap(id, client, size);
            if ( bmp.IsOk() )
                break;
        }

        wxSize sizeNeeded = size;
        if ( !bmp.IsOk() )
        {
                                    wxIconBundle iconBundle = DoGetIconBundle(id, client);
            if ( iconBundle.IsOk() )
            {
                if ( sizeNeeded == wxDefaultSize )
                    sizeNeeded = GetNativeSizeHint(client);

                wxIcon icon(iconBundle.GetIcon(sizeNeeded));
                if ( icon.IsOk() )
                {
                                                            bmp.CopyFromIcon(icon);
                }
            }
        }

                if ( bmp.IsOk() && sizeNeeded != wxDefaultSize )
        {
            if ( bmp.GetSize() != sizeNeeded )
            {
                RescaleBitmap(bmp, sizeNeeded);
            }
        }

        sm_cache->PutBitmap(hashId, bmp);
    }

    return bmp;
}


wxIconBundle wxArtProvider::GetIconBundle(const wxArtID& id, const wxArtClient& client)
{
    wxIconBundle iconbundle(DoGetIconBundle(id, client));

    if ( iconbundle.IsOk() )
    {
        return iconbundle;
    }
    else
    {
                return wxIconBundle(GetIcon(id, client));
    }
}


wxIconBundle wxArtProvider::DoGetIconBundle(const wxArtID& id, const wxArtClient& client)
{
        wxASSERT_MSG( client.Last() == wxT('C'), wxT("invalid 'client' parameter") );

    wxCHECK_MSG( sm_providers, wxNullIconBundle, wxT("no wxArtProvider exists") );

    wxString hashId = wxArtProviderCache::ConstructHashID(id, client);

    wxIconBundle iconbundle;
    if ( !sm_cache->GetIconBundle(hashId, &iconbundle) )
    {
        for (wxArtProvidersList::compatibility_iterator node = sm_providers->GetFirst();
             node; node = node->GetNext())
        {
            iconbundle = node->GetData()->CreateIconBundle(id, client);
            if ( iconbundle.IsOk() )
                break;
        }

        sm_cache->PutIconBundle(hashId, iconbundle);
    }

    return iconbundle;
}

 wxIcon wxArtProvider::GetIcon(const wxArtID& id,
                                         const wxArtClient& client,
                                         const wxSize& size)
{
    wxBitmap bmp = GetBitmap(id, client, size);

    if ( !bmp.IsOk() )
        return wxNullIcon;

    wxIcon icon;
    icon.CopyFromBitmap(bmp);
    return icon;
}


wxArtID wxArtProvider::GetMessageBoxIconId(int flags)
{
    switch ( flags & wxICON_MASK )
    {
        default:
            wxFAIL_MSG(wxT("incorrect message box icon flags"));
            wxFALLTHROUGH;

        case wxICON_ERROR:
            return wxART_ERROR;

        case wxICON_INFORMATION:
            return wxART_INFORMATION;

        case wxICON_WARNING:
            return wxART_WARNING;

        case wxICON_QUESTION:
            return wxART_QUESTION;
    }
}

 wxSize wxArtProvider::GetSizeHint(const wxArtClient& client,
                                         bool platform_dependent)
{
    if (!platform_dependent)
    {
        wxArtProvidersList::compatibility_iterator node = sm_providers->GetFirst();
        if (node)
            return node->GetData()->DoGetSizeHint(client);
    }

    return GetNativeSizeHint(client);
}

#ifndef wxHAS_NATIVE_ART_PROVIDER_IMPL

wxSize wxArtProvider::GetNativeSizeHint(const wxArtClient& WXUNUSED(client))
{
            return wxDefaultSize;
}


void wxArtProvider::InitNativeProvider()
{
}
#endif 


bool wxArtProvider::HasNativeProvider()
{
#ifdef __WXGTK20__
    return true;
#else
    return false;
#endif
}


#if WXWIN_COMPATIBILITY_2_8
 void wxArtProvider::Insert(wxArtProvider *provider)
{
    PushBack(provider);
}
#endif 

class wxArtProviderModule: public wxModule
{
public:
    bool OnInit() wxOVERRIDE
    {
                                wxArtProvider::InitNativeProvider();
#if wxUSE_ARTPROVIDER_TANGO
        wxArtProvider::InitTangoProvider();
#endif #if wxUSE_ARTPROVIDER_STD
        wxArtProvider::InitStdProvider();
#endif         return true;
    }
    void OnExit() wxOVERRIDE
    {
        wxArtProvider::CleanUpProviders();
    }

    wxDECLARE_DYNAMIC_CLASS(wxArtProviderModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxArtProviderModule, wxModule);
