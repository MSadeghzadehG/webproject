


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxOSX_USE_COCOA_OR_CARBON

#include "wx/osx/core/hid.h"

#ifndef WX_PRECOMP
    #include "wx/dynarray.h"
    #include "wx/string.h"
    #include "wx/log.h"
    #include "wx/utils.h"
    #include "wx/module.h"
#endif

#include "wx/osx/private.h"



bool wxHIDDevice::Create (int nClass, int nType, int nDev)
{
        if(IOMasterPort(bootstrap_port, &m_pPort) != kIOReturnSuccess)
    {
        wxLogSysError(wxT("Could not create mach port"));
        return false;
    }

                                CFMutableDictionaryRef pDictionary = IOServiceMatching(kIOHIDDeviceKey);
    if(pDictionary == NULL)
    {
        wxLogSysError( wxT("IOServiceMatching(kIOHIDDeviceKey) failed") );
        return false;
    }

        if (nType != -1)
    {
        CFNumberRef pType = CFNumberCreate(kCFAllocatorDefault,
                                    kCFNumberIntType, &nType);
        CFDictionarySetValue(pDictionary, CFSTR(kIOHIDPrimaryUsageKey), pType);
        CFRelease(pType);
    }
    if (nClass != -1)
    {
        CFNumberRef pClass = CFNumberCreate(kCFAllocatorDefault,
                                    kCFNumberIntType, &nClass);
        CFDictionarySetValue(pDictionary, CFSTR(kIOHIDPrimaryUsagePageKey), pClass);
        CFRelease(pClass);
    }

        io_iterator_t pIterator;
    if( IOServiceGetMatchingServices(m_pPort,
                        pDictionary, &pIterator) != kIOReturnSuccess )
    {
        wxLogSysError(wxT("No Matching HID Services"));
        return false;
    }

        if(pIterator == 0)
        return false; 
        io_object_t pObject;
    while ( (pObject = IOIteratorNext(pIterator)) != 0)
    {
        if(--nDev != 0)
        {
            IOObjectRelease(pObject);
            continue;
        }

        if ( IORegistryEntryCreateCFProperties
             (
                pObject,
                &pDictionary,
                kCFAllocatorDefault,
                kNilOptions
             ) != KERN_SUCCESS )
        {
            wxLogDebug(wxT("IORegistryEntryCreateCFProperties failed"));
        }

                        
                CFStringRef cfsProduct = (CFStringRef)
            CFDictionaryGetValue(pDictionary, CFSTR(kIOHIDProductKey));
        m_szProductName =
            wxCFStringRef( wxCFRetain(cfsProduct)
                               ).AsString();

                CFNumberRef cfnProductId = (CFNumberRef)
            CFDictionaryGetValue(pDictionary, CFSTR(kIOHIDProductIDKey));
        if (cfnProductId)
        {
            CFNumberGetValue(cfnProductId, kCFNumberIntType, &m_nProductId);
        }

                CFNumberRef cfnVendorId = (CFNumberRef)
            CFDictionaryGetValue(pDictionary, CFSTR(kIOHIDVendorIDKey));
        if (cfnVendorId)
        {
            CFNumberGetValue(cfnVendorId, kCFNumberIntType, &m_nManufacturerId);
        }

                        
                SInt32 nScore;
        IOCFPlugInInterface** ppPlugin;
        if(IOCreatePlugInInterfaceForService(pObject,
                                             kIOHIDDeviceUserClientTypeID,
                                             kIOCFPlugInInterfaceID, &ppPlugin,
                                             &nScore) !=  kIOReturnSuccess)
        {
            wxLogSysError(wxT("Could not create HID Interface for product"));
            return false;
        }

                        
                if((*ppPlugin)->QueryInterface(ppPlugin,
                               CFUUIDGetUUIDBytes(kIOHIDDeviceInterfaceID),
                               (void**) &m_ppDevice) != S_OK)
        {
            wxLogSysError(wxT("Could not get device interface from HID interface"));
            return false;
        }

                (*ppPlugin)->Release(ppPlugin);

                if ( (*m_ppDevice)->open(m_ppDevice, 0) != S_OK )
        {
            wxLogDebug(wxT("HID device: open failed"));
        }

                                CFArrayRef cfaCookies = (CFArrayRef)CFDictionaryGetValue(pDictionary,
                                 CFSTR(kIOHIDElementKey));
        BuildCookies(cfaCookies);

                CFRelease(pDictionary);
        IOObjectRelease(pObject);

                IOObjectRelease(pIterator);

        return true;
    }

        IOObjectRelease(pIterator);

    return false; }
size_t wxHIDDevice::GetCount (int nClass, int nType)
{
        mach_port_t             pPort;
    if(IOMasterPort(bootstrap_port, &pPort) != kIOReturnSuccess)
    {
        wxLogSysError(wxT("Could not create mach port"));
        return false;
    }

                CFMutableDictionaryRef pDictionary = IOServiceMatching(kIOHIDDeviceKey);
    if(pDictionary == NULL)
    {
        wxLogSysError( wxT("IOServiceMatching(kIOHIDDeviceKey) failed") );
        return false;
    }

        if (nType != -1)
    {
        CFNumberRef pType = CFNumberCreate(kCFAllocatorDefault,
                                    kCFNumberIntType, &nType);
        CFDictionarySetValue(pDictionary, CFSTR(kIOHIDPrimaryUsageKey), pType);
        CFRelease(pType);
    }
    if (nClass != -1)
    {
        CFNumberRef pClass = CFNumberCreate(kCFAllocatorDefault,
                                    kCFNumberIntType, &nClass);
        CFDictionarySetValue(pDictionary, CFSTR(kIOHIDPrimaryUsagePageKey), pClass);
        CFRelease(pClass);
    }

        io_iterator_t pIterator;
    if( IOServiceGetMatchingServices(pPort,
                                     pDictionary, &pIterator) != kIOReturnSuccess )
    {
        wxLogSysError(wxT("No Matching HID Services"));
        return false;
    }

        if ( !pIterator )
        return 0;

        size_t nCount = 0;
    io_object_t pObject;
    while ( (pObject = IOIteratorNext(pIterator)) != 0)
    {
        ++nCount;
        IOObjectRelease(pObject);
    }

        IOObjectRelease(pIterator);
    mach_port_deallocate(mach_task_self(), pPort);

    return nCount;
}
void wxHIDDevice::AddCookie(CFTypeRef Data, int i)
{
    CFNumberGetValue(
                (CFNumberRef) CFDictionaryGetValue    ( (CFDictionaryRef) Data
                                        , CFSTR(kIOHIDElementCookieKey)
                                        ),
                kCFNumberIntType,
                &m_pCookies[i]
                );
}

void wxHIDDevice::AddCookieInQueue(CFTypeRef Data, int i)
{
        AddCookie(Data, i);
    if ( (*m_ppQueue)->addElement(m_ppQueue, m_pCookies[i], 0) != S_OK )
    {
        wxLogDebug(wxT("HID device: adding element failed"));
    }
}

void wxHIDDevice::InitCookies(size_t dwSize, bool bQueue)
{
    m_pCookies = new IOHIDElementCookie[dwSize];
    if (bQueue)
    {
        wxASSERT( m_ppQueue == NULL);
        m_ppQueue = (*m_ppDevice)->allocQueue(m_ppDevice);
        if ( !m_ppQueue )
        {
            wxLogDebug(wxT("HID device: allocQueue failed"));
            return;
        }

                if ( (*m_ppQueue)->create(m_ppQueue, 0, 512) != S_OK )
        {
            wxLogDebug(wxT("HID device: create failed"));
        }
    }

        memset(m_pCookies, 0, sizeof(*m_pCookies) * dwSize);
}

bool wxHIDDevice::IsActive(int nIndex)
{
    if(!HasElement(nIndex))
    {
                                return false;
    }

    IOHIDEventStruct Event;
    (*m_ppDevice)->getElementValue(m_ppDevice, m_pCookies[nIndex], &Event);
    return !!Event.value;
}

bool wxHIDDevice::HasElement(int nIndex)
{
    return m_pCookies[nIndex] != 0;
}

wxHIDDevice::~wxHIDDevice()
{
    if (m_ppDevice != NULL)
    {
        if (m_ppQueue != NULL)
        {
            (*m_ppQueue)->stop(m_ppQueue);
            (*m_ppQueue)->dispose(m_ppQueue);
            (*m_ppQueue)->Release(m_ppQueue);
        }
        (*m_ppDevice)->close(m_ppDevice);
        (*m_ppDevice)->Release(m_ppDevice);
        mach_port_deallocate(mach_task_self(), m_pPort);
    }

    if (m_pCookies != NULL)
    {
        delete [] m_pCookies;
    }
}


enum
{
    WXK_RSHIFT = 400,
    WXK_RALT,
    WXK_RCONTROL,
    WXK_RAW_RCONTROL,
};

int wxHIDKeyboard::GetCount()
{
    return wxHIDDevice::GetCount(kHIDPage_GenericDesktop,
                               kHIDUsage_GD_Keyboard);
}

bool wxHIDKeyboard::Create(int nDev )
{
    return wxHIDDevice::Create(kHIDPage_GenericDesktop,
                               kHIDUsage_GD_Keyboard,
                               nDev);
}

void wxHIDKeyboard::AddCookie(CFTypeRef Data, int i)
{
    if(!HasElement(i))
        wxHIDDevice::AddCookie(Data, i);
}

void wxHIDKeyboard::BuildCookies(CFArrayRef Array)
{
        InitCookies(500);

        DoBuildCookies(Array);
}

void wxHIDKeyboard::DoBuildCookies(CFArrayRef Array)
{
        int i;
    long nUsage;
    for (i = 0; i < CFArrayGetCount(Array); ++i)
    {
        const void* ref = CFDictionaryGetValue(
                (CFDictionaryRef)CFArrayGetValueAtIndex(Array, i),
                CFSTR(kIOHIDElementKey)
                                              );

        if (ref != NULL)
        {
            DoBuildCookies((CFArrayRef) ref);
        }
        else
    {

                                            CFNumberGetValue(
                (CFNumberRef)
                    CFDictionaryGetValue((CFDictionaryRef)
                        CFArrayGetValueAtIndex(Array, i),
                        CFSTR(kIOHIDElementUsageKey)
                                        ),
                              kCFNumberLongType,
                              &nUsage);

                                    
                                                                                                                                                        
        if (nUsage >= kHIDUsage_KeyboardA && nUsage <= kHIDUsage_KeyboardZ)
            AddCookie(CFArrayGetValueAtIndex(Array, i), 'A' + (nUsage - kHIDUsage_KeyboardA) );
        else if (nUsage >= kHIDUsage_Keyboard1 && nUsage <= kHIDUsage_Keyboard9)
            AddCookie(CFArrayGetValueAtIndex(Array, i), '1' + (nUsage - kHIDUsage_Keyboard1) );
        else if (nUsage >= kHIDUsage_KeyboardF1 && nUsage <= kHIDUsage_KeyboardF12)
            AddCookie(CFArrayGetValueAtIndex(Array, i), WXK_F1 + (nUsage - kHIDUsage_KeyboardF1) );
        else if (nUsage >= kHIDUsage_KeyboardF13 && nUsage <= kHIDUsage_KeyboardF24)
            AddCookie(CFArrayGetValueAtIndex(Array, i), WXK_F13 + (nUsage - kHIDUsage_KeyboardF13) );
        else if (nUsage >= kHIDUsage_Keypad1 && nUsage <= kHIDUsage_Keypad9)
            AddCookie(CFArrayGetValueAtIndex(Array, i), WXK_NUMPAD1 + (nUsage - kHIDUsage_Keypad1) );
        else switch (nUsage)
        {
                        case kHIDUsage_Keyboard0:
                AddCookie(CFArrayGetValueAtIndex(Array, i), '0');
                break;
            case kHIDUsage_Keypad0:
                AddCookie(CFArrayGetValueAtIndex(Array, i), WXK_NUMPAD0);
                break;

                        case kHIDUsage_KeyboardReturnOrEnter:
                AddCookie(CFArrayGetValueAtIndex(Array, i), WXK_RETURN);
                break;
            case kHIDUsage_KeyboardEscape:
                AddCookie(CFArrayGetValueAtIndex(Array, i), WXK_ESCAPE);
                break;
            case kHIDUsage_KeyboardDeleteOrBackspace:
                AddCookie(CFArrayGetValueAtIndex(Array, i), WXK_BACK);
                break;
            case kHIDUsage_KeyboardTab:
                AddCookie(CFArrayGetValueAtIndex(Array, i), WXK_TAB);
                break;
            case kHIDUsage_KeyboardSpacebar:
                AddCookie(CFArrayGetValueAtIndex(Array, i), WXK_SPACE);
                break;
            case kHIDUsage_KeyboardPageUp:
                AddCookie(CFArrayGetValueAtIndex(Array, i), WXK_PAGEUP);
                break;
            case kHIDUsage_KeyboardEnd:
                AddCookie(CFArrayGetValueAtIndex(Array, i), WXK_END);
                break;
            case kHIDUsage_KeyboardPageDown:
                AddCookie(CFArrayGetValueAtIndex(Array, i), WXK_PAGEDOWN);
                break;
            case kHIDUsage_KeyboardRightArrow:
                AddCookie(CFArrayGetValueAtIndex(Array, i), WXK_RIGHT);
                break;
            case kHIDUsage_KeyboardLeftArrow:
                AddCookie(CFArrayGetValueAtIndex(Array, i), WXK_LEFT);
                break;
            case kHIDUsage_KeyboardDownArrow:
                AddCookie(CFArrayGetValueAtIndex(Array, i), WXK_DOWN);
                break;
            case kHIDUsage_KeyboardUpArrow:
                AddCookie(CFArrayGetValueAtIndex(Array, i), WXK_UP);
                break;

                        case kHIDUsage_KeyboardCapsLock:
                AddCookie(CFArrayGetValueAtIndex(Array, i),WXK_CAPITAL);
                break;
            case kHIDUsage_KeypadNumLock:
                AddCookie(CFArrayGetValueAtIndex(Array, i),WXK_NUMLOCK);
                break;
            case kHIDUsage_KeyboardScrollLock:
                AddCookie(CFArrayGetValueAtIndex(Array, i),WXK_SCROLL);
                break;

                        case kHIDUsage_KeyboardLeftControl:
                AddCookie(CFArrayGetValueAtIndex(Array, i),WXK_RAW_CONTROL);
                break;
            case kHIDUsage_KeyboardLeftShift:
                AddCookie(CFArrayGetValueAtIndex(Array, i),WXK_SHIFT);
                break;
            case kHIDUsage_KeyboardLeftAlt:
                AddCookie(CFArrayGetValueAtIndex(Array, i),WXK_ALT);
                break;
            case kHIDUsage_KeyboardLeftGUI:
                AddCookie(CFArrayGetValueAtIndex(Array, i),WXK_CONTROL);
                break;
            case kHIDUsage_KeyboardRightControl:
                AddCookie(CFArrayGetValueAtIndex(Array, i),WXK_RAW_RCONTROL);
                break;
            case kHIDUsage_KeyboardRightShift:
                AddCookie(CFArrayGetValueAtIndex(Array, i),WXK_RSHIFT);
                break;
            case kHIDUsage_KeyboardRightAlt:
                AddCookie(CFArrayGetValueAtIndex(Array, i),WXK_RALT);
                break;
            case kHIDUsage_KeyboardRightGUI:
                AddCookie(CFArrayGetValueAtIndex(Array, i),WXK_RCONTROL);
                break;

                        default:
                        break;
            }         }     } }

class wxHIDModule : public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxHIDModule);

public:
        static wxArrayPtrVoid sm_keyboards;
        virtual bool OnInit() wxOVERRIDE
        {
            return true;
        }
        virtual void OnExit() wxOVERRIDE
        {
            for(size_t i = 0; i < sm_keyboards.GetCount(); ++i)
                delete (wxHIDKeyboard*) sm_keyboards[i];
            sm_keyboards.Clear();
        }
};

wxIMPLEMENT_DYNAMIC_CLASS(wxHIDModule, wxModule);

wxArrayPtrVoid wxHIDModule::sm_keyboards;


bool wxGetKeyState (wxKeyCode key)
{
    wxASSERT_MSG(key != WXK_LBUTTON && key != WXK_RBUTTON && key !=
        WXK_MBUTTON, wxT("can't use wxGetKeyState() for mouse buttons"));

    CGKeyCode cgcode = wxCharCodeWXToOSX((wxKeyCode)key);
    return CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState, cgcode);
}

#endif 