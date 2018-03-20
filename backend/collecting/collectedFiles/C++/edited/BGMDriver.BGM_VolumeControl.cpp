

#include "BGM_VolumeControl.h"

#include "BGM_PlugIn.h"

#include "CAException.h"
#include "CADebugMacros.h"
#include "CADispatchQueue.h"
#include "BGM_Utils.h"

#include <algorithm>

#include <CoreAudio/AudioHardwareBase.h>
#include <Accelerate/Accelerate.h>


#pragma clang assume_nonnull begin

#pragma mark Construction/Destruction

BGM_VolumeControl::BGM_VolumeControl(AudioObjectID inObjectID,
                                     AudioObjectID inOwnerObjectID,
                                     AudioObjectPropertyScope inScope,
                                     AudioObjectPropertyElement inElement)
:
    BGM_Control(inObjectID,
                kAudioVolumeControlClassID,
                kAudioLevelControlClassID,
                inOwnerObjectID,
                inScope,
                inElement),
    mMutex("Volume Control"),
    mVolumeRaw(kDefaultMinRawVolume),
    mAmplitudeGain(0.0f),
    mMinVolumeRaw(kDefaultMinRawVolume),
    mMaxVolumeRaw(kDefaultMaxRawVolume),
    mMinVolumeDb(kDefaultMinDbVolume),
    mMaxVolumeDb(kDefaultMaxDbVolume),
    mWillApplyVolumeToAudio(false)
{
        mVolumeCurve.AddRange(mMinVolumeRaw, mMaxVolumeRaw, mMinVolumeDb, mMaxVolumeDb);
}

#pragma mark Property Operations

bool    BGM_VolumeControl::HasProperty(AudioObjectID inObjectID,
                                       pid_t inClientPID,
                                       const AudioObjectPropertyAddress& inAddress) const
{
    CheckObjectID(inObjectID);

    bool theAnswer = false;

    switch(inAddress.mSelector)
    {
        case kAudioLevelControlPropertyScalarValue:
        case kAudioLevelControlPropertyDecibelValue:
        case kAudioLevelControlPropertyDecibelRange:
        case kAudioLevelControlPropertyConvertScalarToDecibels:
        case kAudioLevelControlPropertyConvertDecibelsToScalar:
            theAnswer = true;
            break;

        default:
            theAnswer = BGM_Control::HasProperty(inObjectID, inClientPID, inAddress);
            break;
    };

    return theAnswer;
}

bool    BGM_VolumeControl::IsPropertySettable(AudioObjectID inObjectID,
                                              pid_t inClientPID,
                                              const AudioObjectPropertyAddress& inAddress) const
{
    CheckObjectID(inObjectID);

    bool theAnswer = false;

    switch(inAddress.mSelector)
    {
        case kAudioLevelControlPropertyDecibelRange:
        case kAudioLevelControlPropertyConvertScalarToDecibels:
        case kAudioLevelControlPropertyConvertDecibelsToScalar:
            theAnswer = false;
            break;

        case kAudioLevelControlPropertyScalarValue:
        case kAudioLevelControlPropertyDecibelValue:
            theAnswer = true;
            break;

        default:
            theAnswer = BGM_Control::IsPropertySettable(inObjectID, inClientPID, inAddress);
            break;
    };

    return theAnswer;
}

UInt32  BGM_VolumeControl::GetPropertyDataSize(AudioObjectID inObjectID,
                                               pid_t inClientPID,
                                               const AudioObjectPropertyAddress& inAddress,
                                               UInt32 inQualifierDataSize,
                                               const void* inQualifierData) const
{
    CheckObjectID(inObjectID);

    UInt32 theAnswer = 0;

    switch(inAddress.mSelector)
    {
        case kAudioLevelControlPropertyScalarValue:
            theAnswer = sizeof(Float32);
            break;

        case kAudioLevelControlPropertyDecibelValue:
            theAnswer = sizeof(Float32);
            break;

        case kAudioLevelControlPropertyDecibelRange:
            theAnswer = sizeof(AudioValueRange);
            break;

        case kAudioLevelControlPropertyConvertScalarToDecibels:
            theAnswer = sizeof(Float32);
            break;

        case kAudioLevelControlPropertyConvertDecibelsToScalar:
            theAnswer = sizeof(Float32);
            break;

        default:
            theAnswer = BGM_Control::GetPropertyDataSize(inObjectID,
                                                         inClientPID,
                                                         inAddress,
                                                         inQualifierDataSize,
                                                         inQualifierData);
            break;
    };

    return theAnswer;
}

void    BGM_VolumeControl::GetPropertyData(AudioObjectID inObjectID,
                                           pid_t inClientPID,
                                           const AudioObjectPropertyAddress& inAddress,
                                           UInt32 inQualifierDataSize,
                                           const void* inQualifierData,
                                           UInt32 inDataSize,
                                           UInt32& outDataSize,
                                           void* outData) const
{
    CheckObjectID(inObjectID);

    switch(inAddress.mSelector)
    {
        case kAudioLevelControlPropertyScalarValue:
                        {
                ThrowIf(inDataSize < sizeof(Float32),
                        CAException(kAudioHardwareBadPropertySizeError),
                        "BGM_VolumeControl::GetPropertyData: not enough space for the return value "
                        "of kAudioLevelControlPropertyScalarValue for the volume control");

                CAMutex::Locker theLocker(mMutex);

                *reinterpret_cast<Float32*>(outData) = mVolumeCurve.ConvertRawToScalar(mVolumeRaw);
                outDataSize = sizeof(Float32);
            }
            break;

        case kAudioLevelControlPropertyDecibelValue:
                        {
                ThrowIf(inDataSize < sizeof(Float32),
                        CAException(kAudioHardwareBadPropertySizeError),
                        "BGM_VolumeControl::GetPropertyData: not enough space for the return value "
                        "of kAudioLevelControlPropertyDecibelValue for the volume control");

                CAMutex::Locker theLocker(mMutex);

                *reinterpret_cast<Float32*>(outData) = mVolumeCurve.ConvertRawToDB(mVolumeRaw);
                outDataSize = sizeof(Float32);
            }
            break;

        case kAudioLevelControlPropertyDecibelRange:
                        ThrowIf(inDataSize < sizeof(AudioValueRange),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_VolumeControl::GetPropertyData: not enough space for the return value of "
                    "kAudioLevelControlPropertyDecibelRange for the volume control");
            reinterpret_cast<AudioValueRange*>(outData)->mMinimum = mVolumeCurve.GetMinimumDB();
            reinterpret_cast<AudioValueRange*>(outData)->mMaximum = mVolumeCurve.GetMaximumDB();
            outDataSize = sizeof(AudioValueRange);
            break;

        case kAudioLevelControlPropertyConvertScalarToDecibels:
                        {
                ThrowIf(inDataSize < sizeof(Float32),
                        CAException(kAudioHardwareBadPropertySizeError),
                        "BGM_VolumeControl::GetPropertyData: not enough space for the return value "
                        "of kAudioLevelControlPropertyConvertScalarToDecibels for the volume "
                        "control");

                                Float32 theVolumeValue = *reinterpret_cast<Float32*>(outData);
                theVolumeValue = std::min(1.0f, std::max(0.0f, theVolumeValue));

                                *reinterpret_cast<Float32*>(outData) =
                        mVolumeCurve.ConvertScalarToDB(theVolumeValue);

                                outDataSize = sizeof(Float32);
            }
            break;

        case kAudioLevelControlPropertyConvertDecibelsToScalar:
                        {
                ThrowIf(inDataSize < sizeof(Float32),
                        CAException(kAudioHardwareBadPropertySizeError),
                        "BGM_VolumeControl::GetPropertyData: not enough space for the return value "
                        "of kAudioLevelControlPropertyConvertDecibelsToScalar for the volume "
                        "control");

                                Float32 theVolumeValue = *reinterpret_cast<Float32*>(outData);
                theVolumeValue = std::min(mMaxVolumeDb, std::max(mMinVolumeDb, theVolumeValue));

                                *reinterpret_cast<Float32*>(outData) =
                        mVolumeCurve.ConvertDBToScalar(theVolumeValue);

                                outDataSize = sizeof(Float32);
            }
            break;

        default:
            BGM_Control::GetPropertyData(inObjectID,
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

void    BGM_VolumeControl::SetPropertyData(AudioObjectID inObjectID,
                                           pid_t inClientPID,
                                           const AudioObjectPropertyAddress& inAddress,
                                           UInt32 inQualifierDataSize,
                                           const void* inQualifierData,
                                           UInt32 inDataSize,
                                           const void* inData)
{
    CheckObjectID(inObjectID);

    switch(inAddress.mSelector)
    {
        case kAudioLevelControlPropertyScalarValue:
            {
                ThrowIf(inDataSize != sizeof(Float32),
                        CAException(kAudioHardwareBadPropertySizeError),
                        "BGM_VolumeControl::SetPropertyData: wrong size for the data for "
                        "kAudioLevelControlPropertyScalarValue");

                                Float32 theNewVolumeScalar = *reinterpret_cast<const Float32*>(inData);
                SetVolumeScalar(theNewVolumeScalar);
            }
            break;

        case kAudioLevelControlPropertyDecibelValue:
            {
                ThrowIf(inDataSize != sizeof(Float32),
                        CAException(kAudioHardwareBadPropertySizeError),
                        "BGM_VolumeControl::SetPropertyData: wrong size for the data for "
                        "kAudioLevelControlPropertyDecibelValue");

                                Float32 theNewVolumeDb = *reinterpret_cast<const Float32*>(inData);
                SetVolumeDb(theNewVolumeDb);
            }
            break;

        default:
            BGM_Control::SetPropertyData(inObjectID,
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

void    BGM_VolumeControl::SetVolumeScalar(Float32 inNewVolumeScalar)
{
            inNewVolumeScalar = std::min(1.0f, std::max(0.0f, inNewVolumeScalar));

        SInt32 theNewVolumeRaw = mVolumeCurve.ConvertScalarToRaw(inNewVolumeScalar);
    SetVolumeRaw(theNewVolumeRaw);
}

void    BGM_VolumeControl::SetVolumeDb(Float32 inNewVolumeDb)
{
        
        inNewVolumeDb = std::min(mMaxVolumeDb, std::max(mMinVolumeDb, inNewVolumeDb));

        SInt32 theNewVolumeRaw = mVolumeCurve.ConvertDBToRaw(inNewVolumeDb);
    SetVolumeRaw(theNewVolumeRaw);
}

void    BGM_VolumeControl::SetWillApplyVolumeToAudio(bool inWillApplyVolumeToAudio)
{
    mWillApplyVolumeToAudio = inWillApplyVolumeToAudio;
}

#pragma mark IO Operations

bool    BGM_VolumeControl::WillApplyVolumeToAudioRT() const
{
    return mWillApplyVolumeToAudio;
}

void    BGM_VolumeControl::ApplyVolumeToAudioRT(Float32* ioBuffer, UInt32 inBufferFrameSize) const
{
    ThrowIf(!mWillApplyVolumeToAudio,
            CAException(kAudioHardwareIllegalOperationError),
            "BGM_VolumeControl::ApplyVolumeToAudioRT: This control doesn't process audio data");

        if((mAmplitudeGain < 0.99f) || (mAmplitudeGain > 1.01f))
    {
                                                                                                                                        vDSP_vsmul(ioBuffer, 1, &mAmplitudeGain, ioBuffer, 1, inBufferFrameSize * 2);
    }
}

#pragma mark Implementation

void    BGM_VolumeControl::SetVolumeRaw(SInt32 inNewVolumeRaw)
{
    CAMutex::Locker theLocker(mMutex);

        inNewVolumeRaw = std::min(std::max(mMinVolumeRaw, inNewVolumeRaw), mMaxVolumeRaw);

        if(mVolumeRaw != inNewVolumeRaw)
    {
        mVolumeRaw = inNewVolumeRaw;

                                                                                                                                                                                                Float32 theSliderPosition = mVolumeCurve.ConvertRawToScalar(mVolumeRaw);

                        SInt32 theRawRange = mMaxVolumeRaw - mMinVolumeRaw;
        SInt32 theSliderPositionInRawSteps = static_cast<SInt32>(theSliderPosition * theRawRange);
        theSliderPositionInRawSteps += mMinVolumeRaw;

        mAmplitudeGain = mVolumeCurve.ConvertRawToScalar(theSliderPositionInRawSteps);

        BGMAssert((mAmplitudeGain >= 0.0f) && (mAmplitudeGain <= 1.0f), "Gain not in [0,1]");

                CADispatchQueue::GetGlobalSerialQueue().Dispatch(false, ^{
            AudioObjectPropertyAddress theChangedProperties[2];
            theChangedProperties[0] = { kAudioLevelControlPropertyScalarValue, mScope, mElement };
            theChangedProperties[1] = { kAudioLevelControlPropertyDecibelValue, mScope, mElement };

            BGM_PlugIn::Host_PropertiesChanged(GetObjectID(), 2, theChangedProperties);
        });
    }
}

#pragma clang assume_nonnull end

