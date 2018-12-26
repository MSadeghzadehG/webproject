

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/defs.h"
#endif

#if wxUSE_MIMETYPE

#include "wx/osx/mimetype.h"
#include "wx/osx/private.h"



wxArrayString ReadStringListFromCFDict( CFDictionaryRef dictionary, CFStringRef key )
{
        wxArrayString results;

        CFTypeRef valueData = CFDictionaryGetValue( dictionary, key );

    if( valueData )
    {
                if( CFGetTypeID( valueData ) == CFArrayGetTypeID() )
        {
            CFArrayRef valueList = reinterpret_cast< CFArrayRef >( valueData );

            CFTypeRef itemData;
            wxCFStringRef item;

                        for( CFIndex i = 0, n = CFArrayGetCount( valueList ); i < n; i++ )
            {
                itemData = CFArrayGetValueAtIndex( valueList, i );

                                if( CFGetTypeID( itemData ) == CFStringGetTypeID() )
                {
                                        item = reinterpret_cast< CFStringRef >( itemData );
                    wxCFRetain( item.get() );

                                        results.Add( item.AsString() );
                }
            }
        }

                else if( CFGetTypeID( valueData ) == CFStringGetTypeID() )
        {
                        wxCFStringRef value = reinterpret_cast< CFStringRef >( valueData );
            wxCFRetain( value.get() );

                        results.Add( value.AsString() );
        }
    }

            return results;
}


bool CheckDocTypeMatchesExt( CFDictionaryRef docType, CFStringRef requiredExt )
{
    const static wxCFStringRef extKey( "CFBundleTypeExtensions" );

    CFTypeRef extData = CFDictionaryGetValue( docType, extKey );

    if( !extData )
        return false;

    if( CFGetTypeID( extData ) == CFArrayGetTypeID() )
    {
        CFArrayRef extList = reinterpret_cast< CFArrayRef >( extData );
        CFTypeRef extItem;

        for( CFIndex i = 0, n = CFArrayGetCount( extList ); i < n; i++ )
        {
            extItem = CFArrayGetValueAtIndex( extList, i );

            if( CFGetTypeID( extItem ) == CFStringGetTypeID() )
            {
                CFStringRef ext = reinterpret_cast< CFStringRef >( extItem );

                if( CFStringCompare( ext, requiredExt, kCFCompareCaseInsensitive ) == kCFCompareEqualTo )
                    return true;
            }
        }
    }

    if( CFGetTypeID( extData ) == CFStringGetTypeID() )
    {
        CFStringRef ext = reinterpret_cast< CFStringRef >( extData );

        if( CFStringCompare( ext, requiredExt, kCFCompareCaseInsensitive ) == kCFCompareEqualTo )
            return true;
    }

    return false;
}


CFDictionaryRef GetDocTypeForExt( CFTypeRef docTypeData, CFStringRef requiredExt )
{
    CFDictionaryRef docType;
    CFArrayRef docTypes;
    CFTypeRef item;

    if( !docTypeData )
        return NULL;

    if( CFGetTypeID( docTypeData ) == CFArrayGetTypeID() )
    {
        docTypes = reinterpret_cast< CFArrayRef >( docTypeData );

        for( CFIndex i = 0, n = CFArrayGetCount( docTypes ); i < n; i++ )
        {
            item = CFArrayGetValueAtIndex( docTypes, i );

            if( CFGetTypeID( item ) == CFDictionaryGetTypeID() )
            {
                docType = reinterpret_cast< CFDictionaryRef >( item );

                if( CheckDocTypeMatchesExt( docType, requiredExt ) )
                    return docType;
            }
        }
    }

    if( CFGetTypeID( docTypeData ) == CFDictionaryGetTypeID() )
    {
        CFDictionaryRef docType = reinterpret_cast< CFDictionaryRef >( docTypeData );

        if( CheckDocTypeMatchesExt( docType, requiredExt ) )
            return docType;
    }

    return NULL;
}


wxString GetPathForIconFile( CFBundleRef bundle, CFStringRef iconFile )
{
        if( !bundle || !iconFile )
        return wxEmptyString;

        CFRange wholeString;
    wholeString.location = 0;
    wholeString.length = CFStringGetLength( iconFile );

        UniCharCount periodIndex;

            {
                UniChar* buffer = new UniChar[ wholeString.length ];
        CFStringGetCharacters( iconFile, wholeString, buffer );

                OSStatus status = LSGetExtensionInfo( wholeString.length, buffer, &periodIndex );

                delete [] buffer;

                if( status != noErr || periodIndex == kLSInvalidExtensionIndex )
            return wxEmptyString;
    }

        CFRange iconNameRange;
    iconNameRange.location = 0;
    iconNameRange.length = periodIndex - 1;

        CFRange iconExtRange;
    iconExtRange.location = periodIndex;
    iconExtRange.length = wholeString.length - periodIndex;

        wxCFStringRef iconName = CFStringCreateWithSubstring( kCFAllocatorDefault, iconFile, iconNameRange );
    wxCFStringRef iconExt = CFStringCreateWithSubstring( kCFAllocatorDefault, iconFile, iconExtRange );

        wxCFRef< CFURLRef > iconUrl = wxCFRef< CFURLRef >( CFBundleCopyResourceURL( bundle, iconName, iconExt, NULL ) );

    if( !iconUrl.get() )
        return wxEmptyString;

        return wxCFStringRef( CFURLCopyFileSystemPath( iconUrl, kCFURLPOSIXPathStyle ) ).AsString();
}


wxMimeTypesManagerImpl::wxMimeTypesManagerImpl()
{
}

wxMimeTypesManagerImpl::~wxMimeTypesManagerImpl()
{
}




void wxMimeTypesManagerImpl::Initialize(int WXUNUSED(mailcapStyles), const wxString& WXUNUSED(extraDir))
{
    }

void wxMimeTypesManagerImpl::ClearData()
{
    }



wxFileType *wxMimeTypesManagerImpl::GetFileTypeFromExtension(const wxString& ext)
{
    wxString uti;

    const TagMap::const_iterator extItr = m_extMap.find( ext );

    if( extItr == m_extMap.end() )
    {
        wxCFStringRef utiRef = UTTypeCreatePreferredIdentifierForTag( kUTTagClassFilenameExtension, wxCFStringRef( ext ), NULL );
        m_extMap[ ext ] = uti = utiRef.AsString();
    }
    else
        uti = extItr->second;

    return GetFileTypeFromUti( uti );
}

wxFileType *wxMimeTypesManagerImpl::GetFileTypeFromMimeType(const wxString& mimeType)
{
    wxString uti;

    const TagMap::const_iterator mimeItr = m_mimeMap.find( mimeType );

    if( mimeItr == m_mimeMap.end() )
    {
        wxCFStringRef utiRef = UTTypeCreatePreferredIdentifierForTag( kUTTagClassFilenameExtension, wxCFStringRef( mimeType ), NULL );
        m_mimeMap[ mimeType ] = uti = utiRef.AsString();
    }
    else
        uti = mimeItr->second;

    return GetFileTypeFromUti( uti );
}

wxFileType *wxMimeTypesManagerImpl::GetFileTypeFromUti(const wxString& uti)
{
    UtiMap::const_iterator utiItr = m_utiMap.find( uti );

    if( utiItr == m_utiMap.end() )
    {
        LoadTypeDataForUti( uti );
        LoadDisplayDataForUti( uti );
    }

    wxFileType* const ft = new wxFileType;
    ft->m_impl->m_uti = uti;
    ft->m_impl->m_manager = this;

    return ft;
}




void wxMimeTypesManagerImpl::LoadTypeDataForUti(const wxString& uti)
{
        const static wxCFStringRef tagsKey( "UTTypeTagSpecification" );
    const static wxCFStringRef extKey( "public.filename-extension" );
    const static wxCFStringRef mimeKey( "public.mime-type" );

        wxCFStringRef utiRef( uti );

        wxCFRef< CFDictionaryRef > utiDecl;
    utiDecl = wxCFRef< CFDictionaryRef >( UTTypeCopyDeclaration( utiRef ) );

    if( !utiDecl )
        return;

        CFTypeRef tagsData = CFDictionaryGetValue( utiDecl, tagsKey );

    if( CFGetTypeID( tagsData ) != CFDictionaryGetTypeID() )
        return;

    CFDictionaryRef tags = reinterpret_cast< CFDictionaryRef >( tagsData );

        m_utiMap[ uti ].extensions = ReadStringListFromCFDict( tags, extKey );
    m_utiMap[ uti ].mimeTypes = ReadStringListFromCFDict( tags, mimeKey );
}


void wxMimeTypesManagerImpl::LoadDisplayDataForUti(const wxString& uti)
{
        const static wxCFStringRef docTypesKey( "CFBundleDocumentTypes" );
    const static wxCFStringRef descKey( "CFBundleTypeName" );
    const static wxCFStringRef iconKey( "CFBundleTypeIconFile" );

                wxCFStringRef ext = UTTypeCopyPreferredTagWithClass( wxCFStringRef( uti ), kUTTagClassFilenameExtension );

        CFURLRef appUrl;
    OSStatus status = LSGetApplicationForInfo( kLSUnknownType, kLSUnknownCreator, ext, kLSRolesAll, NULL, &appUrl );

    if( status != noErr )
        return;

        wxCFRef< CFBundleRef > bundle;
    bundle = wxCFRef< CFBundleRef >( CFBundleCreate( kCFAllocatorDefault, appUrl ) );

    if( !bundle )
        return;

        wxCFStringRef cfsAppPath(CFURLCopyFileSystemPath(appUrl, kCFURLPOSIXPathStyle));
    m_utiMap[ uti ].application = cfsAppPath.AsString();

        CFTypeRef docTypeData;
    docTypeData = CFBundleGetValueForInfoDictionaryKey( bundle, docTypesKey );

    if( !docTypeData )
        return;

        CFDictionaryRef docType;
    docType = GetDocTypeForExt( docTypeData, ext );

    if( !docType )
        return;

        wxCFStringRef description = reinterpret_cast< CFStringRef >( CFDictionaryGetValue( docType, descKey ) );
    wxCFRetain( description.get() );
    m_utiMap[ uti ].description = description.AsString();

        CFStringRef iconFile = reinterpret_cast< CFStringRef > ( CFDictionaryGetValue( docType, iconKey ) );
    m_utiMap[ uti ].iconLoc.SetFileName( GetPathForIconFile( bundle, iconFile ) );
}





size_t wxMimeTypesManagerImpl::EnumAllFileTypes(wxArrayString& WXUNUSED(mimetypes))
{
    return 0;
}

wxFileType *wxMimeTypesManagerImpl::Associate(const wxFileTypeInfo& WXUNUSED(ftInfo))
{
    return 0;
}

bool wxMimeTypesManagerImpl::Unassociate(wxFileType *WXUNUSED(ft))
{
    return false;
}





bool wxMimeTypesManagerImpl::GetExtensions(const wxString& uti, wxArrayString& extensions)
{
    const UtiMap::const_iterator itr = m_utiMap.find( uti );

    if( itr == m_utiMap.end() || itr->second.extensions.GetCount() < 1 )
    {
        extensions.Clear();
        return false;
    }

    extensions = itr->second.extensions;
    return true;
}

bool wxMimeTypesManagerImpl::GetMimeType(const wxString& uti, wxString *mimeType)
{
    const UtiMap::const_iterator itr = m_utiMap.find( uti );

    if( itr == m_utiMap.end() || itr->second.mimeTypes.GetCount() < 1 )
    {
        *mimeType = wxEmptyString;
        return false;
    }

    *mimeType = itr->second.mimeTypes[ 0 ];
    return true;
}

bool wxMimeTypesManagerImpl::GetMimeTypes(const wxString& uti, wxArrayString& mimeTypes)
{
    const UtiMap::const_iterator itr = m_utiMap.find( uti );

    if( itr == m_utiMap.end() || itr->second.mimeTypes.GetCount() < 1 )
    {
        mimeTypes.Clear();
        return false;
    }

    mimeTypes = itr->second.mimeTypes;
    return true;
}

bool wxMimeTypesManagerImpl::GetIcon(const wxString& uti, wxIconLocation *iconLoc)
{
    const UtiMap::const_iterator itr = m_utiMap.find( uti );

    if( itr == m_utiMap.end() || !itr->second.iconLoc.IsOk() )
    {
        *iconLoc = wxIconLocation();
        return false;
    }

    *iconLoc = itr->second.iconLoc;
    return true;
}

bool wxMimeTypesManagerImpl::GetDescription(const wxString& uti, wxString *desc)
{
    const UtiMap::const_iterator itr = m_utiMap.find( uti );

    if( itr == m_utiMap.end() || itr->second.description.empty() )
    {
        *desc = wxEmptyString;
        return false;
    }

    *desc = itr->second.description;
    return true;
}

bool wxMimeTypesManagerImpl::GetApplication(const wxString& uti, wxString *command)
{
    const UtiMap::const_iterator itr = m_utiMap.find( uti );

    if( itr == m_utiMap.end() )
    {
        command->clear();
        return false;
    }

    *command = itr->second.application;
    return true;
}


wxFileTypeImpl::wxFileTypeImpl()
{
}

wxFileTypeImpl::~wxFileTypeImpl()
{
}

bool wxFileTypeImpl::GetExtensions(wxArrayString& extensions) const
{
    return m_manager->GetExtensions( m_uti, extensions );
}

bool wxFileTypeImpl::GetMimeType(wxString *mimeType) const
{
    return m_manager->GetMimeType( m_uti, mimeType );
}

bool wxFileTypeImpl::GetMimeTypes(wxArrayString& mimeTypes) const
{
    return m_manager->GetMimeTypes( m_uti, mimeTypes );
}

bool wxFileTypeImpl::GetIcon(wxIconLocation *iconLoc) const
{
    return m_manager->GetIcon( m_uti, iconLoc );
}

bool wxFileTypeImpl::GetDescription(wxString *desc) const
{
    return m_manager->GetDescription( m_uti, desc );
}

namespace
{

wxString QuoteIfNecessary(const wxString& path)
{
    wxString result(path);

    if ( path.find(' ') != wxString::npos )
    {
        result.insert(0, "'");
        result.append("'");
    }

    return result;
}

} 
bool wxFileTypeImpl::GetOpenCommand(wxString *openCmd, const wxFileType::MessageParameters& params) const
{
    wxString application;
    if ( !m_manager->GetApplication(m_uti, &application) )
        return false;

    *openCmd << QuoteIfNecessary(application)
             << ' ' << QuoteIfNecessary(params.GetFileName());

    return true;
}

bool wxFileTypeImpl::GetPrintCommand(wxString *WXUNUSED(printCmd), const wxFileType::MessageParameters& WXUNUSED(params)) const
{
    return false;
}

size_t wxFileTypeImpl::GetAllCommands(wxArrayString *WXUNUSED(verbs), wxArrayString *WXUNUSED(commands), const wxFileType::MessageParameters& WXUNUSED(params)) const
{
    return false;
}

bool wxFileTypeImpl::SetCommand(const wxString& WXUNUSED(cmd), const wxString& WXUNUSED(verb), bool WXUNUSED(overwriteprompt))
{
    return false;
}

bool wxFileTypeImpl::SetDefaultIcon(const wxString& WXUNUSED(strIcon), int WXUNUSED(index))
{
    return false;
}

bool wxFileTypeImpl::Unassociate(wxFileType *WXUNUSED(ft))
{
    return false;
}

wxString
wxFileTypeImpl::GetExpandedCommand(const wxString& WXUNUSED(verb),
                                   const wxFileType::MessageParameters& WXUNUSED(params)) const
{
    return wxString();
}

#endif 

