

#include "BGMAudioDevice.h"

#include "BGM_Types.h"

#include <AudioToolbox/AudioServices.h>


#pragma mark Construction/Destruction

BGMAudioDevice::BGMAudioDevice(AudioObjectID inAudioDevice)
:
    CAHALAudioDevice(inAudioDevice)
{
}

BGMAudioDevice::BGMAudioDevice(CFStringRef inUID)
:
    CAHALAudioDevice(inUID)
{
}

BGMAudioDevice::BGMAudioDevice(const CAHALAudioDevice& inDevice)
:
    BGMAudioDevice(inDevice.GetObjectID())
{
};

BGMAudioDevice::~BGMAudioDevice()
{
}

bool    BGMAudioDevice::CanBeOutputDeviceInBGMApp() const
{
    CFStringRef uid = CopyDeviceUID();
    bool isNullDevice = CFEqual(uid, CFSTR(kBGMNullDeviceUID));
    CFRelease(uid);

    bool hasOutputChannels = GetTotalNumberChannels( false) > 0;
    bool canBeDefault = CanBeDefaultDevice( false,  false);

    return !IsBGMDeviceInstance() &&
            !isNullDevice &&
            !IsHidden() &&
            hasOutputChannels &&
            canBeDefault;
}

#pragma mark Available Controls

bool    BGMAudioDevice::HasSettableMasterVolume(AudioObjectPropertyScope inScope) const
{
    return HasVolumeControl(inScope, kMasterChannel) &&
        VolumeControlIsSettable(inScope, kMasterChannel);
}

bool    BGMAudioDevice::HasSettableVirtualMasterVolume(AudioObjectPropertyScope inScope) const
{
    AudioObjectPropertyAddress virtualMasterVolumeAddress = {
        kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
        inScope,
        kAudioObjectPropertyElementMaster
    };

    #pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    Boolean virtualMasterVolumeIsSettable;
    OSStatus err = AudioHardwareServiceIsPropertySettable(GetObjectID(),
                                                          &virtualMasterVolumeAddress,
                                                          &virtualMasterVolumeIsSettable);
    virtualMasterVolumeIsSettable &= (err == kAudioServicesNoError);

    bool hasVirtualMasterVolume =
        AudioHardwareServiceHasProperty(GetObjectID(), &virtualMasterVolumeAddress);
#pragma clang diagnostic pop

    return hasVirtualMasterVolume && virtualMasterVolumeIsSettable;
}

bool    BGMAudioDevice::HasSettableMasterMute(AudioObjectPropertyScope inScope) const
{
    return HasMuteControl(inScope, kMasterChannel) &&
        MuteControlIsSettable(inScope, kMasterChannel);
}

#pragma mark Control Values Accessors

void    BGMAudioDevice::CopyMuteFrom(const BGMAudioDevice inDevice,
                                     AudioObjectPropertyScope inScope)
{
        if(HasSettableMasterMute(inScope) && inDevice.HasMuteControl(inScope, kMasterChannel))
    {
        SetMuteControlValue(inScope,
                            kMasterChannel,
                            inDevice.GetMuteControlValue(inScope, kMasterChannel));
    }
}

void    BGMAudioDevice::CopyVolumeFrom(const BGMAudioDevice inDevice,
                                       AudioObjectPropertyScope inScope)
{
        bool didGetVolume = false;
    Float32 volume = FLT_MIN;

    if(inDevice.HasVolumeControl(inScope, kMasterChannel))
    {
        volume = inDevice.GetVolumeControlScalarValue(inScope, kMasterChannel);
        didGetVolume = true;
    }

        if(!didGetVolume)
    {
        UInt32 numChannels =
            inDevice.GetTotalNumberChannels(inScope == kAudioObjectPropertyScopeInput);
        volume = 0;

        for(UInt32 channel = 1; channel <= numChannels; channel++)
        {
            if(inDevice.HasVolumeControl(inScope, channel))
            {
                volume += inDevice.GetVolumeControlScalarValue(inScope, channel);
                didGetVolume = true;
            }
        }

        if(numChannels > 0)          {
            volume /= numChannels;
        }
    }

        if(didGetVolume && volume != FLT_MIN)
    {
        bool didSetVolume = false;

        try
        {
            didSetVolume = SetMasterVolumeScalar(inScope, volume);
        }
        catch(CAException e)
        {
            OSStatus err = e.GetError();
            char err4CC[5] = CA4CCToCString(err);
            CFStringRef uid = CopyDeviceUID();
            LogWarning("BGMAudioDevice::CopyVolumeFrom: CAException '%s' trying to set master "
                       "volume of %s",
                       err4CC,
                       CFStringGetCStringPtr(uid, kCFStringEncodingUTF8));
            CFRelease(uid);
        }

        if(!didSetVolume)
        {
                        Float32 virtualMasterVolume;
            bool success = inDevice.GetVirtualMasterVolumeScalar(inScope, virtualMasterVolume);
            if(success)
            {
                didSetVolume = SetVirtualMasterVolumeScalar(inScope, virtualMasterVolume);
            }
        }

        if(!didSetVolume)
        {
                                    UInt32 numChannels = GetTotalNumberChannels(inScope == kAudioObjectPropertyScopeInput);
            for(UInt32 channel = 1; channel <= numChannels; channel++)
            {
                if(HasVolumeControl(inScope, channel) && VolumeControlIsSettable(inScope, channel))
                {
                    SetVolumeControlScalarValue(inScope, channel, volume);
                }
            }
        }
    }
}

bool    BGMAudioDevice::SetMasterVolumeScalar(AudioObjectPropertyScope inScope, Float32 inVolume)
{
    if(HasSettableMasterVolume(inScope))
    {
        SetVolumeControlScalarValue(inScope, kMasterChannel, inVolume);
        return true;
    }

    return false;
}

bool    BGMAudioDevice::GetVirtualMasterVolumeScalar(AudioObjectPropertyScope inScope,
                                                     Float32& outVirtualMasterVolume) const
{
    AudioObjectPropertyAddress virtualMasterVolumeAddress = {
        kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
        inScope,
        kAudioObjectPropertyElementMaster
    };

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if(!AudioHardwareServiceHasProperty(GetObjectID(), &virtualMasterVolumeAddress))
    {
        return false;
    }
#pragma clang diagnostic pop

    UInt32 virtualMasterVolumePropertySize = sizeof(Float32);
    return kAudioServicesNoError == AHSGetPropertyData(GetObjectID(),
                                                       &virtualMasterVolumeAddress,
                                                       &virtualMasterVolumePropertySize,
                                                       &outVirtualMasterVolume);
}

bool    BGMAudioDevice::SetVirtualMasterVolumeScalar(AudioObjectPropertyScope inScope,
                                                     Float32 inVolume)
{
                                            
    bool didSetVolume = false;

    if(HasSettableVirtualMasterVolume(inScope))
    {
                        Float32 virtualMasterBalance;
        bool didGetVirtualMasterBalance = GetVirtualMasterBalance(inScope, virtualMasterBalance);

        AudioObjectPropertyAddress virtualMasterVolumeAddress = {
            kAudioHardwareServiceDeviceProperty_VirtualMasterVolume,
            inScope,
            kAudioObjectPropertyElementMaster
        };

        didSetVolume = (kAudioServicesNoError == AHSSetPropertyData(GetObjectID(),
                                                                    &virtualMasterVolumeAddress,
                                                                    sizeof(Float32),
                                                                    &inVolume));

                AudioObjectPropertyAddress virtualMasterBalanceAddress = {
            kAudioHardwareServiceDeviceProperty_VirtualMasterBalance,
            inScope,
            kAudioObjectPropertyElementMaster
        };

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        if(didSetVolume &&
           didGetVirtualMasterBalance &&
           AudioHardwareServiceHasProperty(GetObjectID(), &virtualMasterBalanceAddress))
        {
            Boolean balanceIsSettable;
            OSStatus err = AudioHardwareServiceIsPropertySettable(GetObjectID(),
                                                                  &virtualMasterBalanceAddress,
                                                                  &balanceIsSettable);
            if(err == kAudioServicesNoError && balanceIsSettable)
            {
                AHSSetPropertyData(GetObjectID(),
                                   &virtualMasterBalanceAddress,
                                   sizeof(Float32),
                                   &virtualMasterBalance);
            }
        }
#pragma clang diagnostic pop
    }

    return didSetVolume;
}

bool    BGMAudioDevice::GetVirtualMasterBalance(AudioObjectPropertyScope inScope,
                                                Float32& outVirtualMasterBalance) const
{
    AudioObjectPropertyAddress virtualMasterBalanceAddress = {
        kAudioHardwareServiceDeviceProperty_VirtualMasterBalance,
        inScope,
        kAudioObjectPropertyElementMaster
    };

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if(!AudioHardwareServiceHasProperty(GetObjectID(), &virtualMasterBalanceAddress))
    {
        return false;
    }
#pragma clang diagnostic pop

    UInt32 virtualMasterVolumePropertySize = sizeof(Float32);
    return kAudioServicesNoError == AHSGetPropertyData(GetObjectID(),
                                                       &virtualMasterBalanceAddress,
                                                       &virtualMasterVolumePropertySize,
                                                       &outVirtualMasterBalance);
}

#pragma mark Implementation

bool    BGMAudioDevice::IsBGMDevice(bool inIncludeUISoundsInstance) const
{
    bool isBGMDevice = false;

    if(GetObjectID() != kAudioObjectUnknown)
    {
                CFStringRef uid = CopyDeviceUID();

        isBGMDevice =
            CFEqual(uid, CFSTR(kBGMDeviceUID)) ||
                    (inIncludeUISoundsInstance && CFEqual(uid, CFSTR(kBGMDeviceUID_UISounds)));

        CFRelease(uid);
    }

    return isBGMDevice;
}

OSStatus    BGMAudioDevice::AHSGetPropertyData(AudioObjectID inObjectID,
                                               const AudioObjectPropertyAddress* inAddress,
                                               UInt32* ioDataSize,
                                               void* outData)
{
                #pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnonnull"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            return AudioHardwareServiceGetPropertyData(inObjectID, inAddress, 0, NULL, ioDataSize, outData);
#pragma clang diagnostic pop
}

OSStatus    BGMAudioDevice::AHSSetPropertyData(AudioObjectID inObjectID,
                                               const AudioObjectPropertyAddress* inAddress,
                                               UInt32 inDataSize,
                                               const void* inData)
{
    #pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnonnull"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    return AudioHardwareServiceSetPropertyData(inObjectID, inAddress, 0, NULL, inDataSize, inData);
#pragma clang diagnostic pop
}

