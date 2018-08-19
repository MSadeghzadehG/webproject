

#include "BGM_Stream.h"

#include "BGM_Types.h"
#include "BGM_Utils.h"
#include "BGM_Device.h"
#include "BGM_PlugIn.h"

#include "CADebugMacros.h"
#include "CAException.h"
#include "CAPropertyAddress.h"
#include "CADispatchQueue.h"


#pragma clang assume_nonnull begin

BGM_Stream::BGM_Stream(AudioObjectID inObjectID,
                       AudioDeviceID inOwnerDeviceID,
                       bool inIsInput,
                       Float64 inSampleRate,
                       UInt32 inStartingChannel)
:
    BGM_Object(inObjectID, kAudioStreamClassID, kAudioObjectClassID, inOwnerDeviceID),
    mStateMutex(inIsInput ? "Input Stream State" : "Output Stream State"),
    mIsInput(inIsInput),
    mIsStreamActive(false),
    mSampleRate(inSampleRate),
    mStartingChannel(inStartingChannel)
{
}

BGM_Stream::~BGM_Stream()
{
}

bool    BGM_Stream::HasProperty(AudioObjectID inObjectID,
                                pid_t inClientPID,
                                const AudioObjectPropertyAddress& inAddress) const
{
            
    bool theAnswer = false;

    switch(inAddress.mSelector)
    {
        case kAudioStreamPropertyIsActive:
        case kAudioStreamPropertyDirection:
        case kAudioStreamPropertyTerminalType:
        case kAudioStreamPropertyStartingChannel:
        case kAudioStreamPropertyLatency:
        case kAudioStreamPropertyVirtualFormat:
        case kAudioStreamPropertyPhysicalFormat:
        case kAudioStreamPropertyAvailableVirtualFormats:
        case kAudioStreamPropertyAvailablePhysicalFormats:
            theAnswer = true;
            break;

        default:
            theAnswer = BGM_Object::HasProperty(inObjectID, inClientPID, inAddress);
            break;
    };

    return theAnswer;
}

bool    BGM_Stream::IsPropertySettable(AudioObjectID inObjectID,
                                       pid_t inClientPID,
                                       const AudioObjectPropertyAddress& inAddress) const
{
            
    bool theAnswer = false;

    switch(inAddress.mSelector)
    {
        case kAudioStreamPropertyDirection:
        case kAudioStreamPropertyTerminalType:
        case kAudioStreamPropertyStartingChannel:
        case kAudioStreamPropertyLatency:
        case kAudioStreamPropertyAvailableVirtualFormats:
        case kAudioStreamPropertyAvailablePhysicalFormats:
            theAnswer = false;
            break;

        case kAudioStreamPropertyIsActive:
        case kAudioStreamPropertyVirtualFormat:
        case kAudioStreamPropertyPhysicalFormat:
            theAnswer = true;
            break;

        default:
            theAnswer = BGM_Object::IsPropertySettable(inObjectID, inClientPID, inAddress);
            break;
    };

    return theAnswer;
}

UInt32    BGM_Stream::GetPropertyDataSize(AudioObjectID inObjectID,
                                          pid_t inClientPID,
                                          const AudioObjectPropertyAddress& inAddress,
                                          UInt32 inQualifierDataSize,
                                          const void* __nullable inQualifierData) const
{
            
    UInt32 theAnswer = 0;

    switch(inAddress.mSelector)
    {
        case kAudioStreamPropertyIsActive:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioStreamPropertyDirection:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioStreamPropertyTerminalType:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioStreamPropertyStartingChannel:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioStreamPropertyLatency:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioStreamPropertyVirtualFormat:
        case kAudioStreamPropertyPhysicalFormat:
            theAnswer = sizeof(AudioStreamBasicDescription);
            break;
            
        case kAudioStreamPropertyAvailableVirtualFormats:
        case kAudioStreamPropertyAvailablePhysicalFormats:
            theAnswer = 1 * sizeof(AudioStreamRangedDescription);
            break;
            
        default:
            theAnswer = BGM_Object::GetPropertyDataSize(inObjectID,
                                                        inClientPID,
                                                        inAddress,
                                                        inQualifierDataSize,
                                                        inQualifierData);
            break;
    };

    return theAnswer;
}

void    BGM_Stream::GetPropertyData(AudioObjectID inObjectID,
                                    pid_t inClientPID,
                                    const AudioObjectPropertyAddress& inAddress,
                                    UInt32 inQualifierDataSize,
                                    const void* __nullable inQualifierData,
                                    UInt32 inDataSize,
                                    UInt32& outDataSize,
                                    void* outData) const
{
        
    switch(inAddress.mSelector)
    {
        case kAudioObjectPropertyBaseClass:
                        ThrowIf(inDataSize < sizeof(AudioClassID),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_Stream::GetPropertyData: not enough space for the return "
                    "value of kAudioObjectPropertyBaseClass for the stream");
            *reinterpret_cast<AudioClassID*>(outData) = kAudioObjectClassID;
            outDataSize = sizeof(AudioClassID);
            break;

        case kAudioObjectPropertyClass:
                        ThrowIf(inDataSize < sizeof(AudioClassID),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_Stream::GetPropertyData: not enough space for the return "
                    "value of kAudioObjectPropertyClass for the stream");
            *reinterpret_cast<AudioClassID*>(outData) = kAudioStreamClassID;
            outDataSize = sizeof(AudioClassID);
            break;

        case kAudioObjectPropertyOwner:
                        {
                ThrowIf(inDataSize < sizeof(AudioObjectID),
                        CAException(kAudioHardwareBadPropertySizeError),
                        "BGM_Stream::GetPropertyData: not enough space for the return "
                        "value of kAudioObjectPropertyOwner for the stream");

                                                CAMutex::Locker theStateLocker(mStateMutex);

                                *reinterpret_cast<AudioObjectID*>(outData) = mOwnerObjectID;
                outDataSize = sizeof(AudioObjectID);
            }
            break;

        case kAudioStreamPropertyIsActive:
                                                {
                ThrowIf(inDataSize < sizeof(UInt32),
                        CAException(kAudioHardwareBadPropertySizeError),
                        "BGM_Stream::GetPropertyData: not enough space for the return "
                        "value of kAudioStreamPropertyIsActive for the stream");

                                CAMutex::Locker theStateLocker(mStateMutex);

                                *reinterpret_cast<UInt32*>(outData) = mIsStreamActive;
                outDataSize = sizeof(UInt32);
            }
            break;

        case kAudioStreamPropertyDirection:
                        ThrowIf(inDataSize < sizeof(UInt32),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_Stream::GetPropertyData: not enough space for the return value of "
                    "kAudioStreamPropertyDirection for the stream");
            *reinterpret_cast<UInt32*>(outData) = mIsInput ? 1 : 0;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioStreamPropertyTerminalType:
                                    ThrowIf(inDataSize < sizeof(UInt32),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_Stream::GetPropertyData: not enough space for the return value of "
                    "kAudioStreamPropertyTerminalType for the stream");
            *reinterpret_cast<UInt32*>(outData) =
                mIsInput ? kAudioStreamTerminalTypeMicrophone : kAudioStreamTerminalTypeSpeaker;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioStreamPropertyStartingChannel:
                                                            ThrowIf(inDataSize < sizeof(UInt32),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_Stream::GetPropertyData: not enough space for the return "
                    "value of kAudioStreamPropertyStartingChannel for the stream");
            *reinterpret_cast<UInt32*>(outData) = mStartingChannel;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioStreamPropertyLatency:
                        ThrowIf(inDataSize < sizeof(UInt32),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_Stream::GetPropertyData: not enough space for the return "
                    "value of kAudioStreamPropertyLatency for the stream");
            *reinterpret_cast<UInt32*>(outData) = 0;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioStreamPropertyVirtualFormat:
        case kAudioStreamPropertyPhysicalFormat:
                                                {
                ThrowIf(inDataSize < sizeof(AudioStreamBasicDescription),
                        CAException(kAudioHardwareBadPropertySizeError),
                        "BGM_Stream::GetPropertyData: not enough space for the return "
                        "value of kAudioStreamPropertyVirtualFormat for the stream");

                                AudioStreamBasicDescription* outASBD =
                    reinterpret_cast<AudioStreamBasicDescription*>(outData);

                                outASBD->mSampleRate = mSampleRate;
                outASBD->mFormatID = kAudioFormatLinearPCM;
                outASBD->mFormatFlags =
                    kAudioFormatFlagIsFloat | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
                outASBD->mBytesPerPacket = 8;
                outASBD->mFramesPerPacket = 1;
                outASBD->mBytesPerFrame = 8;
                outASBD->mChannelsPerFrame = 2;
                outASBD->mBitsPerChannel = 32;

                outDataSize = sizeof(AudioStreamBasicDescription);
            }
            break;

        case kAudioStreamPropertyAvailableVirtualFormats:
        case kAudioStreamPropertyAvailablePhysicalFormats:
                                    if((inDataSize / sizeof(AudioStreamRangedDescription)) >= 1)
            {
                AudioStreamRangedDescription* outASRD =
                    reinterpret_cast<AudioStreamRangedDescription*>(outData);

                outASRD[0].mFormat.mSampleRate = mSampleRate;
                outASRD[0].mFormat.mFormatID = kAudioFormatLinearPCM;
                outASRD[0].mFormat.mFormatFlags =
                    kAudioFormatFlagIsFloat | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
                outASRD[0].mFormat.mBytesPerPacket = 8;
                outASRD[0].mFormat.mFramesPerPacket = 1;
                outASRD[0].mFormat.mBytesPerFrame = 8;
                outASRD[0].mFormat.mChannelsPerFrame = 2;
                outASRD[0].mFormat.mBitsPerChannel = 32;
                                outASRD[0].mSampleRateRange.mMinimum = 1.0;
                outASRD[0].mSampleRateRange.mMaximum = 1000000000.0;

                                outDataSize = sizeof(AudioStreamRangedDescription);
            }
            else
            {
                outDataSize = 0;
            }
            break;

        default:
            BGM_Object::GetPropertyData(inObjectID,
                                        inClientPID,
                                        inAddress,
                                        inQualifierDataSize,
                                        inQualifierData,
                                        inDataSize,
                                        outDataSize,
                                        outData);
            break;
    };
}

void    BGM_Stream::SetPropertyData(AudioObjectID inObjectID,
                                    pid_t inClientPID,
                                    const AudioObjectPropertyAddress& inAddress,
                                    UInt32 inQualifierDataSize,
                                    const void* __nullable inQualifierData,
                                    UInt32 inDataSize,
                                    const void* inData)
{
    
    switch(inAddress.mSelector)
    {
        case kAudioStreamPropertyIsActive:
            {
                                                ThrowIf(inDataSize != sizeof(UInt32),
                        CAException(kAudioHardwareBadPropertySizeError),
                        "BGM_Stream::SetPropertyData: wrong size for the data for "
                        "kAudioStreamPropertyIsActive");
                bool theNewIsActive = *reinterpret_cast<const UInt32*>(inData) != 0;

                CAMutex::Locker theStateLocker(mStateMutex);

                if(mIsStreamActive != theNewIsActive)
                {
                    mIsStreamActive = theNewIsActive;

                                        CADispatchQueue::GetGlobalSerialQueue().Dispatch(false,	^{
                        AudioObjectPropertyAddress theProperty[] = {
                            CAPropertyAddress(kAudioStreamPropertyIsActive)
                        };
                        BGM_PlugIn::Host_PropertiesChanged(inObjectID, 1, theProperty);
                    });
                }
            }
            break;

        case kAudioStreamPropertyVirtualFormat:
        case kAudioStreamPropertyPhysicalFormat:
            {
                                                                                                                ThrowIf(inDataSize != sizeof(AudioStreamBasicDescription),
                        CAException(kAudioHardwareBadPropertySizeError),
                        "BGM_Stream::SetPropertyData: wrong size for the data for "
                        "kAudioStreamPropertyPhysicalFormat");

                const AudioStreamBasicDescription* theNewFormat =
                    reinterpret_cast<const AudioStreamBasicDescription*>(inData);

                ThrowIf(theNewFormat->mFormatID != kAudioFormatLinearPCM,
                        CAException(kAudioDeviceUnsupportedFormatError),
                        "BGM_Stream::SetPropertyData: unsupported format ID for "
                        "kAudioStreamPropertyPhysicalFormat");
                ThrowIf(theNewFormat->mFormatFlags !=
                            (kAudioFormatFlagIsFloat |
                             kAudioFormatFlagsNativeEndian |
                             kAudioFormatFlagIsPacked),
                        CAException(kAudioDeviceUnsupportedFormatError),
                        "BGM_Stream::SetPropertyData: unsupported format flags for "
                        "kAudioStreamPropertyPhysicalFormat");
                ThrowIf(theNewFormat->mBytesPerPacket != 8,
                        CAException(kAudioDeviceUnsupportedFormatError),
                        "BGM_Stream::SetPropertyData: unsupported bytes per packet for "
                        "kAudioStreamPropertyPhysicalFormat");
                ThrowIf(theNewFormat->mFramesPerPacket != 1,
                        CAException(kAudioDeviceUnsupportedFormatError),
                        "BGM_Stream::SetPropertyData: unsupported frames per packet for "
                        "kAudioStreamPropertyPhysicalFormat");
                ThrowIf(theNewFormat->mBytesPerFrame != 8,
                        CAException(kAudioDeviceUnsupportedFormatError),
                        "BGM_Stream::SetPropertyData: unsupported bytes per frame for "
                        "kAudioStreamPropertyPhysicalFormat");
                ThrowIf(theNewFormat->mChannelsPerFrame != 2,
                        CAException(kAudioDeviceUnsupportedFormatError),
                        "BGM_Stream::SetPropertyData: unsupported channels per frame for "
                        "kAudioStreamPropertyPhysicalFormat");
                ThrowIf(theNewFormat->mBitsPerChannel != 32,
                        CAException(kAudioDeviceUnsupportedFormatError),
                        "BGM_Stream::SetPropertyData: unsupported bits per channel for "
                        "kAudioStreamPropertyPhysicalFormat");
                ThrowIf(theNewFormat->mSampleRate < 1.0,
                        CAException(kAudioDeviceUnsupportedFormatError),
                        "BGM_Stream::SetPropertyData: unsupported sample rate for "
                        "kAudioStreamPropertyPhysicalFormat");
            }
            break;

        default:
            BGM_Object::SetPropertyData(inObjectID,
                                        inClientPID,
                                        inAddress,
                                        inQualifierDataSize,
                                        inQualifierData,
                                        inDataSize,
                                        inData);
            break;
    };
}

#pragma mark Accessors

void    BGM_Stream::SetSampleRate(Float64 inSampleRate)
{
    CAMutex::Locker theStateLocker(mStateMutex);
    mSampleRate = inSampleRate;
}

#pragma clang assume_nonnull end

