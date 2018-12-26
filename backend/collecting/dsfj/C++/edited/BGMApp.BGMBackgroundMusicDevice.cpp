

#include "BGMBackgroundMusicDevice.h"

#include "BGM_Types.h"
#include "BGM_Utils.h"

#include "CADebugMacros.h"
#include "CAHALAudioSystemObject.h"
#include "CACFArray.h"
#include "CACFDictionary.h"

#include <map>


#pragma clang assume_nonnull begin

#pragma mark Construction/Destruction

BGMBackgroundMusicDevice::BGMBackgroundMusicDevice()
:
    BGMAudioDevice(CFSTR(kBGMDeviceUID)),
    mUISoundsBGMDevice(CFSTR(kBGMDeviceUID_UISounds))
{
    if((GetObjectID() == kAudioObjectUnknown) || (mUISoundsBGMDevice == kAudioObjectUnknown))
    {
        LogError("BGMBackgroundMusicDevice::BGMBackgroundMusicDevice: Error getting BGMDevice ID");
        Throw(CAException(kAudioHardwareIllegalOperationError));
    }
};

BGMBackgroundMusicDevice::~BGMBackgroundMusicDevice()
{
}

#pragma mark Systemwide Default Device

void BGMBackgroundMusicDevice::SetAsOSDefault()
{
    DebugMsg("BGMBackgroundMusicDevice::SetAsOSDefault: Setting the system's default audio device "
             "to BGMDevice");

    CAHALAudioSystemObject audioSystem;

    AudioDeviceID defaultDevice = audioSystem.GetDefaultAudioDevice(false, false);
    AudioDeviceID systemDefaultDevice = audioSystem.GetDefaultAudioDevice(false, true);

    if(systemDefaultDevice == defaultDevice)
    {
                                                audioSystem.SetDefaultAudioDevice(false, true, mUISoundsBGMDevice);
    }

    audioSystem.SetDefaultAudioDevice(false, false, GetObjectID());
}

void BGMBackgroundMusicDevice::UnsetAsOSDefault(AudioDeviceID inOutputDeviceID)
{
    CAHALAudioSystemObject audioSystem;

        bool bgmDeviceIsDefault =
            (audioSystem.GetDefaultAudioDevice(false, false) == GetObjectID());

    if(bgmDeviceIsDefault)
    {
        DebugMsg("BGMBackgroundMusicDevice::UnsetAsOSDefault: Setting the system's default output "
                 "device back to device %d", inOutputDeviceID);

        audioSystem.SetDefaultAudioDevice(false, false, inOutputDeviceID);
    }

        bool bgmDeviceIsSystemDefault =
            (audioSystem.GetDefaultAudioDevice(false, true) == mUISoundsBGMDevice);

            if(bgmDeviceIsSystemDefault)
    {
        DebugMsg("BGMBackgroundMusicDevice::UnsetAsOSDefault: Setting the system's default system "
                 "output device back to device %d", inOutputDeviceID);

        audioSystem.SetDefaultAudioDevice(false, true, inOutputDeviceID);
    }
}

#pragma mark App Volumes

CFArrayRef BGMBackgroundMusicDevice::GetAppVolumes() const
{
    CFTypeRef appVolumes = GetPropertyData_CFType(kBGMAppVolumesAddress);

    ThrowIfNULL(appVolumes,
                CAException(kAudioHardwareIllegalOperationError),
                "BGMBackgroundMusicDevice::GetAppVolumes: !appVolumes");
    ThrowIf(CFGetTypeID(appVolumes) != CFArrayGetTypeID(),
            CAException(kAudioHardwareIllegalOperationError),
            "BGMBackgroundMusicDevice::GetAppVolumes: Expected CFArray value");

    return static_cast<CFArrayRef>(appVolumes);
}

void BGMBackgroundMusicDevice::SetAppVolume(SInt32 inVolume,
                                            pid_t inAppProcessID,
                                            CFStringRef __nullable inAppBundleID)
{
    BGMAssert((kAppRelativeVolumeMinRawValue <= inVolume) &&
                      (inVolume <= kAppRelativeVolumeMaxRawValue),
              "BGMBackgroundMusicDevice::SetAppVolume: Volume out of bounds");

        inVolume = std::max(kAppRelativeVolumeMinRawValue, inVolume);
    inVolume = std::min(kAppRelativeVolumeMaxRawValue, inVolume);

    SendAppVolumeOrPanToBGMDevice(inVolume,
                                  CFSTR(kBGMAppVolumesKey_RelativeVolume),
                                  inAppProcessID,
                                  inAppBundleID);
}

void BGMBackgroundMusicDevice::SetAppPanPosition(SInt32 inPanPosition,
                                                 pid_t inAppProcessID,
                                                 CFStringRef __nullable inAppBundleID)
{
    BGMAssert((kAppPanLeftRawValue <= inPanPosition) && (inPanPosition <= kAppPanRightRawValue),
              "BGMBackgroundMusicDevice::SetAppPanPosition: Pan position out of bounds");

        inPanPosition = std::max(kAppPanLeftRawValue, inPanPosition);
    inPanPosition = std::min(kAppPanRightRawValue, inPanPosition);

    SendAppVolumeOrPanToBGMDevice(inPanPosition,
                                  CFSTR(kBGMAppVolumesKey_PanPosition),
                                  inAppProcessID,
                                  inAppBundleID);
}

void BGMBackgroundMusicDevice::SendAppVolumeOrPanToBGMDevice(SInt32 inNewValue,
                                                             CFStringRef inVolumeTypeKey,
                                                             pid_t inAppProcessID,
                                                             CFStringRef __nullable inAppBundleID)
{
    CACFArray appVolumeChanges(true);

    auto addVolumeChange = [&] (pid_t pid, CFStringRef bundleID)
    {
        CACFDictionary appVolumeChange(true);

        appVolumeChange.AddSInt32(CFSTR(kBGMAppVolumesKey_ProcessID), pid);
        appVolumeChange.AddString(CFSTR(kBGMAppVolumesKey_BundleID), bundleID);
        appVolumeChange.AddSInt32(inVolumeTypeKey, inNewValue);

        appVolumeChanges.AppendDictionary(appVolumeChange.GetDict());
    };

    addVolumeChange(inAppProcessID, inAppBundleID);

        for(CACFString responsibleBundleID : ResponsibleBundleIDsOf(CACFString(inAppBundleID)))
    {
                addVolumeChange(-1, responsibleBundleID.GetCFString());
    }

    CFPropertyListRef changesPList = appVolumeChanges.AsPropertyList();

        SetPropertyData_CFType(kBGMAppVolumesAddress, changesPList);

        mUISoundsBGMDevice.SetPropertyData_CFType(kBGMAppVolumesAddress, changesPList);
}

std::vector<CACFString>
BGMBackgroundMusicDevice::ResponsibleBundleIDsOf(CACFString inParentBundleID)
{
    if(!inParentBundleID.IsValid())
    {
        return {};
    }

    std::map<CACFString, std::vector<CACFString>> bundleIDMap = {
                { "com.apple.finder",
            { "com.apple.quicklook.ui.helper",
              "com.apple.quicklook.QuickLookUIService" } },
                { "com.apple.Safari", { "com.apple.WebKit.WebContent" } },
                { "org.mozilla.firefox", { "org.mozilla.plugincontainer" } },
                { "org.mozilla.nightly", { "org.mozilla.plugincontainer" } },
                { "com.vmware.fusion", { "com.vmware.vmware-vmx" } },
                { "com.parallels.desktop.console", { "com.parallels.vm" } },
                { "hu.mplayerhq.mplayerosx.extended",
                { "ch.sttz.mplayerosx.extended.binaries.officialsvn" } }
    };

            if(inParentBundleID.StartsWith(CFSTR("com.parallels.winapp.")))
    {
        return { "com.parallels.vm" };
    }

    return bundleIDMap[inParentBundleID];
}

#pragma mark Audible State

BGMDeviceAudibleState BGMBackgroundMusicDevice::GetAudibleState() const
{
    CFTypeRef propertyDataRef = GetPropertyData_CFType(kBGMAudibleStateAddress);

    ThrowIfNULL(propertyDataRef,
                CAException(kAudioHardwareIllegalOperationError),
                "BGMBackgroundMusicDevice::GetAudibleState: !propertyDataRef");

    ThrowIf(CFGetTypeID(propertyDataRef) != CFNumberGetTypeID(),
            CAException(kAudioHardwareIllegalOperationError),
            "BGMBackgroundMusicDevice::GetAudibleState: Property was not a CFNumber");

    CFNumberRef audibleStateRef = static_cast<CFNumberRef>(propertyDataRef);

    BGMDeviceAudibleState audibleState;
    Boolean success = CFNumberGetValue(audibleStateRef, kCFNumberSInt32Type, &audibleState);
    CFRelease(audibleStateRef);

    ThrowIf(!success,
            CAException(kAudioHardwareIllegalOperationError),
            "BGMBackgroundMusicDevice::GetMusicPlayerProcessID: CFNumberGetValue failed");

    return audibleState;
}

#pragma mark Music Player

pid_t BGMBackgroundMusicDevice::GetMusicPlayerProcessID() const
{
    CFTypeRef propertyDataRef = GetPropertyData_CFType(kBGMMusicPlayerProcessIDAddress);

    ThrowIfNULL(propertyDataRef,
                CAException(kAudioHardwareIllegalOperationError),
                "BGMBackgroundMusicDevice::GetMusicPlayerProcessID: !propertyDataRef");

    ThrowIf(CFGetTypeID(propertyDataRef) != CFNumberGetTypeID(),
            CAException(kAudioHardwareIllegalOperationError),
            "BGMBackgroundMusicDevice::GetMusicPlayerProcessID: Property was not a CFNumber");

    CFNumberRef pidRef = static_cast<CFNumberRef>(propertyDataRef);

    pid_t pid;
    Boolean success = CFNumberGetValue(pidRef, kCFNumberIntType, &pid);
    CFRelease(pidRef);

    ThrowIf(!success,
            CAException(kAudioHardwareIllegalOperationError),
            "BGMBackgroundMusicDevice::GetMusicPlayerProcessID: CFNumberGetValue failed");

    return pid;
}

CFStringRef BGMBackgroundMusicDevice::GetMusicPlayerBundleID() const
{
    CFStringRef bundleID = GetPropertyData_CFString(kBGMMusicPlayerBundleIDAddress);

    ThrowIfNULL(bundleID,
                CAException(kAudioHardwareIllegalOperationError),
                "BGMBackgroundMusicDevice::GetMusicPlayerBundleID: !bundleID");

    return bundleID;
}

#pragma clang assume_nonnull end

