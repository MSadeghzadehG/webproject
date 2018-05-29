

#include "CAHALAudioSystemObject.h"


CAHALAudioSystemObject::CAHALAudioSystemObject()
:
    CAHALAudioObject(kAudioObjectSystemObject)
{
}

CAHALAudioSystemObject::~CAHALAudioSystemObject()
{
}

AudioObjectID	CAHALAudioSystemObject::GetAudioDeviceForUID(CFStringRef inUID) const
{
    AudioObjectID id = kAudioObjectUnknown;

            for(int i = 0; i < CFStringGetLength(inUID); i++)
    {
        id += 37 * CFStringGetCharacterAtIndex(inUID, i);
    }

    return id;
}

#pragma mark Unimplemented methods

#pragma clang diagnostic ignored "-Wunused-parameter"

UInt32	CAHALAudioSystemObject::GetNumberAudioDevices() const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

void	CAHALAudioSystemObject::GetAudioDevices(UInt32& ioNumberAudioDevices, AudioObjectID* outAudioDevices) const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

AudioObjectID	CAHALAudioSystemObject::GetAudioDeviceAtIndex(UInt32 inIndex) const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

void	CAHALAudioSystemObject::LogBasicDeviceInfo()
{
    Throw(new CAException(kAudio_UnimplementedError));
}

AudioObjectID	CAHALAudioSystemObject::GetDefaultAudioDevice(bool inIsInput, bool inIsSystem) const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

void	CAHALAudioSystemObject::SetDefaultAudioDevice(bool inIsInput, bool inIsSystem, AudioObjectID inNewDefaultDevice)
{
    Throw(new CAException(kAudio_UnimplementedError));
}

AudioObjectID	CAHALAudioSystemObject::GetAudioPlugInForBundleID(CFStringRef inUID) const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

