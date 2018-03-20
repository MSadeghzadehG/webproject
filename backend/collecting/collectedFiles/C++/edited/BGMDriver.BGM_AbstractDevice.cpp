

#include "BGM_AbstractDevice.h"

#include "BGM_Utils.h"

#include "CAException.h"
#include "CADebugMacros.h"


#pragma clang assume_nonnull begin

BGM_AbstractDevice::BGM_AbstractDevice(AudioObjectID inObjectID, AudioObjectID inOwnerObjectID)
:
    BGM_Object(inObjectID, kAudioDeviceClassID, kAudioObjectClassID, inOwnerObjectID)
{
}

BGM_AbstractDevice::~BGM_AbstractDevice()
{
}

#pragma mark Property Operations

bool    BGM_AbstractDevice::HasProperty(AudioObjectID inObjectID,
                                        pid_t inClientPID,
                                        const AudioObjectPropertyAddress& inAddress) const
{
    bool theAnswer = false;
    
    switch(inAddress.mSelector)
    {
        case kAudioObjectPropertyName:
        case kAudioObjectPropertyManufacturer:
        case kAudioDevicePropertyDeviceUID:
        case kAudioDevicePropertyModelUID:
        case kAudioDevicePropertyTransportType:
        case kAudioDevicePropertyRelatedDevices:
        case kAudioDevicePropertyClockDomain:
        case kAudioDevicePropertyDeviceIsAlive:
        case kAudioDevicePropertyDeviceIsRunning:
        case kAudioDevicePropertyDeviceCanBeDefaultDevice:
        case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
        case kAudioDevicePropertyLatency:
        case kAudioDevicePropertyStreams:
        case kAudioObjectPropertyControlList:
        case kAudioDevicePropertySafetyOffset:
        case kAudioDevicePropertyNominalSampleRate:
        case kAudioDevicePropertyAvailableNominalSampleRates:
        case kAudioDevicePropertyIsHidden:
        case kAudioDevicePropertyZeroTimeStampPeriod:
            theAnswer = true;
            break;
            
        default:
            theAnswer = BGM_Object::HasProperty(inObjectID, inClientPID, inAddress);
            break;
    };

    return theAnswer;
}

bool    BGM_AbstractDevice::IsPropertySettable(AudioObjectID inObjectID,
                                               pid_t inClientPID,
                                               const AudioObjectPropertyAddress& inAddress) const
{
    bool theAnswer = false;

    switch(inAddress.mSelector)
    {
        case kAudioObjectPropertyName:
        case kAudioObjectPropertyManufacturer:
        case kAudioDevicePropertyDeviceUID:
        case kAudioDevicePropertyModelUID:
        case kAudioDevicePropertyTransportType:
        case kAudioDevicePropertyRelatedDevices:
        case kAudioDevicePropertyClockDomain:
        case kAudioDevicePropertyDeviceIsAlive:
        case kAudioDevicePropertyDeviceIsRunning:
        case kAudioDevicePropertyDeviceCanBeDefaultDevice:
        case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
        case kAudioDevicePropertyLatency:
        case kAudioDevicePropertyStreams:
        case kAudioObjectPropertyControlList:
        case kAudioDevicePropertySafetyOffset:
        case kAudioDevicePropertyNominalSampleRate:
        case kAudioDevicePropertyAvailableNominalSampleRates:
        case kAudioDevicePropertyIsHidden:
        case kAudioDevicePropertyZeroTimeStampPeriod:
            theAnswer = false;
            break;
            
        default:
            theAnswer = BGM_Object::IsPropertySettable(inObjectID, inClientPID, inAddress);
            break;
    };

    return theAnswer;
}

UInt32    BGM_AbstractDevice::GetPropertyDataSize(AudioObjectID inObjectID,
                                                  pid_t inClientPID,
                                                  const AudioObjectPropertyAddress& inAddress,
                                                  UInt32 inQualifierDataSize,
                                                  const void* __nullable inQualifierData) const
{
    UInt32 theAnswer = 0;

    switch(inAddress.mSelector)
    {
        case kAudioObjectPropertyName:
            theAnswer = sizeof(CFStringRef);
            break;

        case kAudioObjectPropertyManufacturer:
            theAnswer = sizeof(CFStringRef);
            break;

        case kAudioDevicePropertyDeviceUID:
            theAnswer = sizeof(CFStringRef);
            break;

        case kAudioDevicePropertyModelUID:
            theAnswer = sizeof(CFStringRef);
            break;

        case kAudioDevicePropertyTransportType:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioDevicePropertyRelatedDevices:
            theAnswer = sizeof(AudioObjectID);
            break;

        case kAudioDevicePropertyClockDomain:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioDevicePropertyDeviceCanBeDefaultDevice:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioDevicePropertyDeviceIsAlive:
            theAnswer = sizeof(AudioClassID);
            break;

        case kAudioDevicePropertyDeviceIsRunning:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioDevicePropertyLatency:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioDevicePropertyStreams:
            theAnswer = 0;
            break;

        case kAudioObjectPropertyControlList:
            theAnswer = 0;
            break;

        case kAudioDevicePropertySafetyOffset:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioDevicePropertyNominalSampleRate:
            theAnswer = sizeof(Float64);
            break;

        case kAudioDevicePropertyAvailableNominalSampleRates:
            theAnswer = 0;
            break;

        case kAudioDevicePropertyIsHidden:
            theAnswer = sizeof(UInt32);
            break;

        case kAudioDevicePropertyZeroTimeStampPeriod:
            theAnswer = sizeof(UInt32);
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

void    BGM_AbstractDevice::GetPropertyData(AudioObjectID inObjectID,
                                            pid_t inClientPID,
                                            const AudioObjectPropertyAddress& inAddress,
                                            UInt32 inQualifierDataSize,
                                            const void* __nullable inQualifierData,
                                            UInt32 inDataSize,
                                            UInt32& outDataSize,
                                            void* outData) const
{
    UInt32 theNumberItemsToFetch;

    switch(inAddress.mSelector)
    {
        case kAudioObjectPropertyName:
        case kAudioObjectPropertyManufacturer:
        case kAudioDevicePropertyDeviceUID:
        case kAudioDevicePropertyModelUID:
        case kAudioDevicePropertyDeviceIsRunning:
        case kAudioDevicePropertyZeroTimeStampPeriod:
        case kAudioDevicePropertyNominalSampleRate:
        case kAudioDevicePropertyAvailableNominalSampleRates:
                                                                                    BGMAssert(false,
                      "BGM_AbstractDevice::GetPropertyData: Property %u not handled in subclass",
                      inAddress.mSelector);
                        Throw(CAException(kAudioHardwareIllegalOperationError));

        case kAudioDevicePropertyTransportType:
                                                ThrowIf(inDataSize < sizeof(UInt32),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_AbstractDevice::GetPropertyData: not enough space for the return value of "
                    "kAudioDevicePropertyTransportType for the device");
                        *reinterpret_cast<UInt32*>(outData) = kAudioDeviceTransportTypeVirtual;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioDevicePropertyRelatedDevices:
                                                                                    
                                                theNumberItemsToFetch = inDataSize / sizeof(AudioObjectID);

                        if(theNumberItemsToFetch > 1)
            {
                theNumberItemsToFetch = 1;
            }

                        if(theNumberItemsToFetch > 0)
            {
                reinterpret_cast<AudioObjectID*>(outData)[0] = GetObjectID();
            }
            
                        outDataSize = theNumberItemsToFetch * sizeof(AudioObjectID);
            break;

        case kAudioDevicePropertyClockDomain:
                                                                                                            ThrowIf(inDataSize < sizeof(UInt32),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_AbstractDevice::GetPropertyData: not enough space for the return value of "
                    "kAudioDevicePropertyClockDomain for the device");
            *reinterpret_cast<UInt32*>(outData) = 0;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioDevicePropertyDeviceIsAlive:
                        ThrowIf(inDataSize < sizeof(UInt32),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_AbstractDevice::GetPropertyData: not enough space for the return value of "
                    "kAudioDevicePropertyDeviceIsAlive for the device");
            *reinterpret_cast<UInt32*>(outData) = 1;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioDevicePropertyDeviceCanBeDefaultDevice:
                                                                                    ThrowIf(inDataSize < sizeof(UInt32),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_AbstractDevice::GetPropertyData: not enough space for the return value of "
                    "kAudioDevicePropertyDeviceCanBeDefaultDevice for the device");
            *reinterpret_cast<UInt32*>(outData) = 1;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
                                                                                    ThrowIf(inDataSize < sizeof(UInt32),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_AbstractDevice::GetPropertyData: not enough space for the return value of "
                    "kAudioDevicePropertyDeviceCanBeDefaultSystemDevice for the device");
            *reinterpret_cast<UInt32*>(outData) = 1;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioDevicePropertyLatency:
                        ThrowIf(inDataSize < sizeof(UInt32),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_AbstractDevice::GetPropertyData: not enough space for the return value of "
                    "kAudioDevicePropertyLatency for the device");
            *reinterpret_cast<UInt32*>(outData) = 0;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioDevicePropertyStreams:
                        outDataSize = 0;
            break;

        case kAudioObjectPropertyControlList:
                        outDataSize = 0;
            break;

        case kAudioDevicePropertySafetyOffset:
                        ThrowIf(inDataSize < sizeof(UInt32),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_AbstractDevice::GetPropertyData: not enough space for the return value of "
                    "kAudioDevicePropertySafetyOffset for the device");
            *reinterpret_cast<UInt32*>(outData) = 0;
            outDataSize = sizeof(UInt32);
            break;

        case kAudioDevicePropertyIsHidden:
                        ThrowIf(inDataSize < sizeof(UInt32),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_AbstractDevice::GetPropertyData: not enough space for the return value of "
                    "kAudioDevicePropertyIsHidden for the device");
            *reinterpret_cast<UInt32*>(outData) = 0;
            outDataSize = sizeof(UInt32);
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

#pragma clang assume_nonnull end

