

#include "CAHALAudioObject.h"

#include "BGM_Types.h"

#include <CoreAudio/AudioHardware.h>


#pragma clang diagnostic ignored "-Wunused-parameter"

static CFStringRef playerBundleID = CFSTR("");

CAHALAudioObject::CAHALAudioObject(AudioObjectID inObjectID)
:
    mObjectID(inObjectID)
{
}

CAHALAudioObject::~CAHALAudioObject()
{
}

AudioObjectID	CAHALAudioObject::GetObjectID() const
{
    return mObjectID;
}

void	CAHALAudioObject::GetPropertyData(const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32& ioDataSize, void* outData) const
{
    if(inAddress.mSelector == kAudioDeviceCustomPropertyMusicPlayerBundleID)
    {
        *reinterpret_cast<CFStringRef*>(outData) = playerBundleID;
    }
}

void	CAHALAudioObject::SetPropertyData(const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
    if(inAddress.mSelector == kAudioDeviceCustomPropertyMusicPlayerBundleID)
    {
        playerBundleID = *reinterpret_cast<const CFStringRef*>(inData);
    }
}

#pragma mark Unimplemented methods

void	CAHALAudioObject::SetObjectID(AudioObjectID inObjectID)
{
    Throw(new CAException(kAudio_UnimplementedError));
}

AudioClassID	CAHALAudioObject::GetClassID() const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

AudioObjectID	CAHALAudioObject::GetOwnerObjectID() const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

CFStringRef	CAHALAudioObject::CopyOwningPlugInBundleID() const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

CFStringRef	CAHALAudioObject::CopyName() const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

CFStringRef	CAHALAudioObject::CopyManufacturer() const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

CFStringRef	CAHALAudioObject::CopyNameForElement(AudioObjectPropertyScope inScope, AudioObjectPropertyElement inElement) const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

CFStringRef	CAHALAudioObject::CopyCategoryNameForElement(AudioObjectPropertyScope inScope, AudioObjectPropertyElement inElement) const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

CFStringRef	CAHALAudioObject::CopyNumberNameForElement(AudioObjectPropertyScope inScope, AudioObjectPropertyElement inElement) const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

bool	CAHALAudioObject::ObjectExists(AudioObjectID inObjectID)
{
    Throw(new CAException(kAudio_UnimplementedError));
}

UInt32	CAHALAudioObject::GetNumberOwnedObjects(AudioClassID inClass) const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

void	CAHALAudioObject::GetAllOwnedObjects(AudioClassID inClass, UInt32& ioNumberObjects, AudioObjectID* ioObjectIDs) const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

AudioObjectID	CAHALAudioObject::GetOwnedObjectByIndex(AudioClassID inClass, UInt32 inIndex)
{
    Throw(new CAException(kAudio_UnimplementedError));
}

bool	CAHALAudioObject::HasProperty(const AudioObjectPropertyAddress& inAddress) const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

bool	CAHALAudioObject::IsPropertySettable(const AudioObjectPropertyAddress& inAddress) const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

UInt32	CAHALAudioObject::GetPropertyDataSize(const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData) const
{
    Throw(new CAException(kAudio_UnimplementedError));
}

void	CAHALAudioObject::AddPropertyListener(const AudioObjectPropertyAddress& inAddress, AudioObjectPropertyListenerProc inListenerProc, void* inClientData)
{
    Throw(new CAException(kAudio_UnimplementedError));
}

void	CAHALAudioObject::RemovePropertyListener(const AudioObjectPropertyAddress& inAddress, AudioObjectPropertyListenerProc inListenerProc, void* inClientData)
{
    Throw(new CAException(kAudio_UnimplementedError));
}

