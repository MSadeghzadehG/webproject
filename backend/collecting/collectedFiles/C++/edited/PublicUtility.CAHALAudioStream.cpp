

#include "CAHALAudioStream.h"

#include "CAAutoDisposer.h"
#include "CADebugMacros.h"
#include "CAException.h"
#include "CAPropertyAddress.h"


CAHALAudioStream::CAHALAudioStream(AudioObjectID inAudioStream)
:
	CAHALAudioObject(inAudioStream)
{
}

CAHALAudioStream::~CAHALAudioStream()
{
}

UInt32	CAHALAudioStream::GetDirection() const
{
	CAPropertyAddress theAddress(kAudioStreamPropertyDirection);
	return GetPropertyData_UInt32(theAddress, 0, NULL);
}

UInt32	CAHALAudioStream::GetTerminalType() const
{
	CAPropertyAddress theAddress(kAudioStreamPropertyTerminalType);
	return GetPropertyData_UInt32(theAddress, 0, NULL);
}

UInt32	CAHALAudioStream::GetStartingChannel() const
{
	CAPropertyAddress theAddress(kAudioStreamPropertyStartingChannel);
	return GetPropertyData_UInt32(theAddress, 0, NULL);
}

UInt32	CAHALAudioStream::GetLatency() const
{
	CAPropertyAddress theAddress(kAudioStreamPropertyLatency);
	return GetPropertyData_UInt32(theAddress, 0, NULL);
}

void	CAHALAudioStream::GetCurrentVirtualFormat(AudioStreamBasicDescription& outFormat) const
{
	CAPropertyAddress theAddress(kAudioStreamPropertyVirtualFormat);
	UInt32 theSize = sizeof(AudioStreamBasicDescription);
	GetPropertyData(theAddress, 0, NULL, theSize, &outFormat);
}

void	CAHALAudioStream::SetCurrentVirtualFormat(const AudioStreamBasicDescription& inFormat)
{
	CAPropertyAddress theAddress(kAudioStreamPropertyVirtualFormat);
	SetPropertyData(theAddress, 0, NULL, sizeof(AudioStreamBasicDescription), &inFormat);
}

UInt32	CAHALAudioStream::GetNumberAvailableVirtualFormats() const
{
	CAPropertyAddress theAddress(kAudioStreamPropertyAvailableVirtualFormats);
	UInt32 theAnswer = GetPropertyDataSize(theAddress, 0, NULL);
	theAnswer /= SizeOf32(AudioStreamRangedDescription);
	return theAnswer;
}

void	CAHALAudioStream::GetAvailableVirtualFormats(UInt32& ioNumberFormats, AudioStreamRangedDescription* outFormats) const
{
	CAPropertyAddress theAddress(kAudioStreamPropertyAvailableVirtualFormats);
	UInt32 theSize = ioNumberFormats * SizeOf32(AudioStreamRangedDescription);
	GetPropertyData(theAddress, 0, NULL, theSize, outFormats);
	ioNumberFormats = theSize / SizeOf32(AudioStreamRangedDescription);
}

void	CAHALAudioStream::GetAvailableVirtualFormatByIndex(UInt32 inIndex, AudioStreamRangedDescription& outFormat) const
{
	UInt32 theNumberFormats = GetNumberAvailableVirtualFormats();
	if((theNumberFormats > 0) && (inIndex < theNumberFormats))
	{
		CAAutoArrayDelete<AudioStreamRangedDescription> theFormats(theNumberFormats);
		GetAvailableVirtualFormats(theNumberFormats, theFormats);
		if((theNumberFormats > 0) && (inIndex < theNumberFormats))
		{
			outFormat = theFormats[inIndex];
		}
	}
}

void	CAHALAudioStream::GetCurrentPhysicalFormat(AudioStreamBasicDescription& outFormat) const
{
	CAPropertyAddress theAddress(kAudioStreamPropertyPhysicalFormat);
	UInt32 theSize = sizeof(AudioStreamBasicDescription);
	GetPropertyData(theAddress, 0, NULL, theSize, &outFormat);
}

void	CAHALAudioStream::SetCurrentPhysicalFormat(const AudioStreamBasicDescription& inFormat)
{
	CAPropertyAddress theAddress(kAudioStreamPropertyPhysicalFormat);
	SetPropertyData(theAddress, 0, NULL, sizeof(AudioStreamBasicDescription), &inFormat);
}

UInt32	CAHALAudioStream::GetNumberAvailablePhysicalFormats() const
{
	CAPropertyAddress theAddress(kAudioStreamPropertyAvailablePhysicalFormats);
	UInt32 theAnswer = GetPropertyDataSize(theAddress, 0, NULL);
	theAnswer /= SizeOf32(AudioStreamRangedDescription);
	return theAnswer;
}

void	CAHALAudioStream::GetAvailablePhysicalFormats(UInt32& ioNumberFormats, AudioStreamRangedDescription* outFormats) const
{
	CAPropertyAddress theAddress(kAudioStreamPropertyAvailablePhysicalFormats);
	UInt32 theSize = ioNumberFormats * SizeOf32(AudioStreamRangedDescription);
	GetPropertyData(theAddress, 0, NULL, theSize, outFormats);
	ioNumberFormats = theSize / SizeOf32(AudioStreamRangedDescription);
}

void	CAHALAudioStream::GetAvailablePhysicalFormatByIndex(UInt32 inIndex, AudioStreamRangedDescription& outFormat) const
{
	UInt32 theNumberFormats = GetNumberAvailablePhysicalFormats();
	if((theNumberFormats > 0) && (inIndex < theNumberFormats))
	{
		CAAutoArrayDelete<AudioStreamRangedDescription> theFormats(theNumberFormats);
		GetAvailablePhysicalFormats(theNumberFormats, theFormats);
		if((theNumberFormats > 0) && (inIndex < theNumberFormats))
		{
			outFormat = theFormats[inIndex];
		}
	}
}
