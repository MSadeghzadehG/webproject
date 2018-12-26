


#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/log.h"
    #include "wx/intl.h"
    #include "wx/utils.h"
    #include "wx/wxcrt.h"
#endif 
#include <CoreFoundation/CoreFoundation.h>
#ifndef __WXOSX_IPHONE__
#include <ApplicationServices/ApplicationServices.h>
#endif

#include "wx/filename.h"
#include "wx/osx/core/cfstring.h"
#include "wx/osx/core/private.h"

#define kDefaultPathStyle kCFURLPOSIXPathStyle

#if wxUSE_SOCKETS
class wxSocketManager;
extern WXDLLIMPEXP_BASE wxSocketManager *wxOSXSocketManagerCF;
wxSocketManager *wxOSXSocketManagerCF = NULL;
#endif 
#if ( !wxUSE_GUI && !wxOSX_USE_IPHONE ) || wxOSX_USE_COCOA_OR_CARBON


bool wxMacLaunch(char **argv)
{
            CFIndex cfiCount = 0;
    for(char** argvcopy = argv; *argvcopy != NULL ; ++argvcopy)
    {
        ++cfiCount;
    }

            if(cfiCount == 0)
    {
        wxLogDebug(wxT("wxMacLaunch No file to launch!"));
        return false ;
    }

        wxString path = *argv++;

            CFURLRef cfurlApp =
        CFURLCreateWithFileSystemPath(
            kCFAllocatorDefault,
            wxCFStringRef(path),
            kDefaultPathStyle,
            true); 
        if(!cfurlApp)
    {
        wxLogDebug(wxT("wxMacLaunch Can't open path: %s"), path.c_str());
        return false ;
    }

        CFBundleRef cfbApp = CFBundleCreate(kCFAllocatorDefault, cfurlApp);

                if(!cfbApp)
    {
        wxLogDebug(wxT("wxMacLaunch Bad bundle: %s"), path.c_str());
        CFRelease(cfurlApp);
        return false ;
    }

            UInt32 dwBundleType, dwBundleCreator;
    CFBundleGetPackageInfo(cfbApp, &dwBundleType, &dwBundleCreator);
    if(dwBundleType != 'APPL')
    {
        wxLogDebug(wxT("wxMacLaunch Not an APPL bundle: %s"), path.c_str());
        CFRelease(cfbApp);
        CFRelease(cfurlApp);
        return false ;
    }

            CFMutableArrayRef cfaFiles = CFArrayCreateMutable(kCFAllocatorDefault,
                                    cfiCount-1, &kCFTypeArrayCallBacks);
    if(!cfaFiles)     {
        wxLogDebug(wxT("wxMacLaunch Could not create CFMutableArray"));
        CFRelease(cfbApp);
        CFRelease(cfurlApp);
        return false ;
    }

                for( ; *argv != NULL ; ++argv)
    {
                                if (wxStrcmp(*argv, wxT("<")) == 0)
            continue;


        CFURLRef cfurlCurrentFile;            wxFileName argfn(*argv);     
        if(argfn.DirExists())
        {
                        cfurlCurrentFile = CFURLCreateWithFileSystemPath(
                                kCFAllocatorDefault,
                                wxCFStringRef(*argv),
                                kDefaultPathStyle,
                                true);         }
        else if(argfn.FileExists())
        {
                                    cfurlCurrentFile = CFURLCreateWithFileSystemPath(
                                kCFAllocatorDefault,
                                wxCFStringRef(*argv),
                                kDefaultPathStyle,
                                false);         }
        else
        {
                                                cfurlCurrentFile = CFURLCreateWithString(
                                kCFAllocatorDefault,
                                wxCFStringRef(*argv),
                                NULL);
        }

                if(!cfurlCurrentFile)
        {
            wxLogDebug(
                wxT("wxMacLaunch Could not create CFURL for argument:%s"),
                *argv);
            continue;
        }

                        CFArrayAppendValue(
            cfaFiles,
            cfurlCurrentFile
                        );
        CFRelease(cfurlCurrentFile);     }

                    LSLaunchURLSpec launchspec;
    launchspec.appURL = cfurlApp;
    launchspec.itemURLs = cfaFiles;
    launchspec.passThruParams = NULL;     launchspec.launchFlags = kLSLaunchDefaults;
    launchspec.asyncRefCon = NULL;

                OSStatus status = LSOpenFromURLSpec(&launchspec, NULL);

        CFRelease(cfbApp);
    CFRelease(cfurlApp);
    CFRelease(cfaFiles);

        if(status != noErr)
    {
        wxLogDebug(wxT("wxMacLaunch LSOpenFromURLSpec Error: %d"),
                   (int)status);
        return false ;
    }

        return true ;
}

#endif 