

#include "CAHALAudioSystemObject.h"

#include "CAAutoDisposer.h"
#include "CACFString.h"
#include "CAHALAudioDevice.h"
#include "CAPropertyAddress.h"


CAHALAudioSystemObject::CAHALAudioSystemObject()
:
	CAHALAudioObject(kAudioObjectSystemObject)
{
}

CAHALAudioSystemObject::~CAHALAudioSystemObject()
{
}

UInt32	CAHALAudioSystemObject::GetNumberAudioDevices() const
{
	CAPropertyAddress theAddress(kAudioHardwarePropertyDevices);
	UInt32 theAnswer = GetPropertyDataSize(theAddress, 0, NULL);
	theAnswer /= SizeOf32(AudioObjectID);
	return theAnswer;
}

void	CAHALAudioSystemObject::GetAudioDevices(UInt32& ioNumberAudioDevices, AudioObjectID* outAudioDevices) const
{
	CAPropertyAddress theAddress(kAudioHardwarePropertyDevices);
	UInt32 theSize = ioNumberAudioDevices * SizeOf32(AudioObjectID);
	GetPropertyData(theAddress, 0, NULL, theSize, outAudioDevices);
	ioNumberAudioDevices = theSize / SizeOf32(AudioObjectID);
}

AudioObjectID	CAHALAudioSystemObject::GetAudioDeviceAtIndex(UInt32 inIndex) const
{
	AudioObjectID theAnswer = kAudioObjectUnknown;
	UInt32 theNumberDevices = GetNumberAudioDevices();
	if((theNumberDevices > 0) && (inIndex < theNumberDevices))
	{
		CAAutoArrayDelete<AudioObjectID> theDeviceList(theNumberDevices);
		GetAudioDevices(theNumberDevices, theDeviceList);
		if((theNumberDevices > 0) && (inIndex < theNumberDevices))
		{
			theAnswer = theDeviceList[inIndex];
		}
	}
	return theAnswer;
}

AudioObjectID	CAHALAudioSystemObject::GetAudioDeviceForUID(CFStringRef inUID) const
{
	AudioObjectID theAnswer = kAudioObjectUnknown;
	AudioValueTranslation theValue = { &inUID, sizeof(CFStringRef), &theAnswer, sizeof(AudioObjectID) };
	CAPropertyAddress theAddress(kAudioHardwarePropertyDeviceForUID);
	UInt32 theSize = sizeof(AudioValueTranslation);
	GetPropertyData(theAddress, 0, NULL, theSize, &theValue);
	return theAnswer;
}

void	CAHALAudioSystemObject::LogBasicDeviceInfo()
{
	UInt32 theNumberDevices = GetNumberAudioDevices();
	CAAutoArrayDelete<AudioObjectID> theDeviceList(theNumberDevices);
	GetAudioDevices(theNumberDevices, theDeviceList);
	DebugMessageN1("CAHALAudioSystemObject::LogBasicDeviceInfo: %d devices", (int)theNumberDevices);
	for(UInt32 theDeviceIndex = 0; theDeviceIndex < theNumberDevices; ++theDeviceIndex)
	{
		char theCString[256];
		UInt32 theCStringSize = sizeof(theCString);
		DebugMessageN1("CAHALAudioSystemObject::LogBasicDeviceInfo: Device %d", (int)theDeviceIndex);
		
		CAHALAudioDevice theDevice(theDeviceList[theDeviceIndex]);
		DebugMessageN1("CAHALAudioSystemObject::LogBasicDeviceInfo:   Object ID: %d", (int)theDeviceList[theDeviceIndex]);
		
		CACFString theDeviceName(theDevice.CopyName());
		theCStringSize = sizeof(theCString);
		theDeviceName.GetCString(theCString, theCStringSize);
		DebugMessageN1("CAHALAudioSystemObject::LogBasicDeviceInfo:   Name:      %s", theCString);
		
		CACFString theDeviceUID(theDevice.CopyDeviceUID());
		theCStringSize = sizeof(theCString);
		theDeviceUID.GetCString(theCString, theCStringSize);
		DebugMessageN1("CAHALAudioSystemObject::LogBasicDeviceInfo:   UID:       %s", theCString);
	}
}

static inline AudioObjectPropertySelector	CAHALAudioSystemObject_CalculateDefaultDeviceProperySelector(bool inIsInput, bool inIsSystem)
{
	AudioObjectPropertySelector theAnswer = kAudioHardwarePropertyDefaultOutputDevice;
	if(inIsInput)
	{
		theAnswer = kAudioHardwarePropertyDefaultInputDevice;
	}
	else if(inIsSystem)
	{
		theAnswer = kAudioHardwarePropertyDefaultSystemOutputDevice;
	}
	return theAnswer;
}

AudioObjectID	CAHALAudioSystemObject::GetDefaultAudioDevice(bool inIsInput, bool inIsSystem) const
{
	AudioObjectID theAnswer = kAudioObjectUnknown;
	CAPropertyAddress theAddress(CAHALAudioSystemObject_CalculateDefaultDeviceProperySelector(inIsInput, inIsSystem));
	UInt32 theSize = sizeof(AudioObjectID);
	GetPropertyData(theAddress, 0, NULL, theSize, &theAnswer);
	return theAnswer;
}

void	CAHALAudioSystemObject::SetDefaultAudioDevice(bool inIsInput, bool inIsSystem, AudioObjectID inNewDefaultDevice)
{
	CAPropertyAddress theAddress(CAHALAudioSystemObject_CalculateDefaultDeviceProperySelector(inIsInput, inIsSystem));
	UInt32 theSize = sizeof(AudioObjectID);
	SetPropertyData(theAddress, 0, NULL, theSize, &inNewDefaultDevice);
}

AudioObjectID	CAHALAudioSystemObject::GetAudioPlugInForBundleID(CFStringRef inUID) const
{
	AudioObjectID theAnswer = kAudioObjectUnknown;
	AudioValueTranslation theValue = { &inUID, sizeof(CFStringRef), &theAnswer, sizeof(AudioObjectID) };
	CAPropertyAddress theAddress(kAudioHardwarePropertyPlugInForBundleID);
	UInt32 theSize = sizeof(AudioValueTranslation);
	GetPropertyData(theAddress, 0, NULL, theSize, &theValue);
	return theAnswer;
}
