

#include "BGM_Control.h"

#include "CADebugMacros.h"
#include "CAException.h"

#include <CoreAudio/AudioHardwareBase.h>


#pragma clang assume_nonnull begin

BGM_Control::BGM_Control(AudioObjectID inObjectID,
                         AudioClassID inClassID,
                         AudioClassID inBaseClassID,
                         AudioObjectID inOwnerObjectID,
                         AudioObjectPropertyScope inScope,
                         AudioObjectPropertyElement inElement)
:
    BGM_Object(inObjectID, inClassID, inBaseClassID, inOwnerObjectID),
    mScope(inScope),
    mElement(inElement)
{
}

bool    BGM_Control::HasProperty(AudioObjectID inObjectID,
                                 pid_t inClientPID,
                                 const AudioObjectPropertyAddress& inAddress) const
{
    CheckObjectID(inObjectID);

    bool theAnswer = false;

    switch(inAddress.mSelector)
    {
        case kAudioControlPropertyScope:
        case kAudioControlPropertyElement:
            theAnswer = true;
            break;

        default:
            theAnswer = BGM_Object::HasProperty(inObjectID, inClientPID, inAddress);
            break;
    };

    return theAnswer;
}

bool    BGM_Control::IsPropertySettable(AudioObjectID inObjectID,
                                        pid_t inClientPID,
                                        const AudioObjectPropertyAddress& inAddress) const
{
    CheckObjectID(inObjectID);

    bool theAnswer = false;

    switch(inAddress.mSelector)
    {
        case kAudioControlPropertyScope:
        case kAudioControlPropertyElement:
            theAnswer = false;
            break;

        default:
            theAnswer = BGM_Object::IsPropertySettable(inObjectID, inClientPID, inAddress);
            break;
    };

    return theAnswer;
}

UInt32  BGM_Control::GetPropertyDataSize(AudioObjectID inObjectID,
                                         pid_t inClientPID,
                                         const AudioObjectPropertyAddress& inAddress,
                                         UInt32 inQualifierDataSize,
                                         const void* inQualifierData) const
{
    CheckObjectID(inObjectID);

    UInt32 theAnswer = 0;

    switch(inAddress.mSelector)
    {
        case kAudioControlPropertyScope:
            theAnswer = sizeof(AudioObjectPropertyScope);
            break;

        case kAudioControlPropertyElement:
            theAnswer = sizeof(AudioObjectPropertyElement);
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

void    BGM_Control::GetPropertyData(AudioObjectID inObjectID,
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
        case kAudioControlPropertyScope:
                        ThrowIf(inDataSize < sizeof(AudioObjectPropertyScope),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_Control::GetPropertyData: not enough space for the return value of "
                    "kAudioControlPropertyScope for the control");
            *reinterpret_cast<AudioObjectPropertyScope*>(outData) = mScope;
            outDataSize = sizeof(AudioObjectPropertyScope);
            break;

        case kAudioControlPropertyElement:
                        ThrowIf(inDataSize < sizeof(AudioObjectPropertyElement),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_Control::GetPropertyData: not enough space for the return value of "
                    "kAudioControlPropertyElement for the control");
            *reinterpret_cast<AudioObjectPropertyElement*>(outData) = mElement;
            outDataSize = sizeof(AudioObjectPropertyElement);
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

void    BGM_Control::CheckObjectID(AudioObjectID inObjectID) const
{
    ThrowIf(inObjectID == kAudioObjectUnknown || inObjectID != GetObjectID(),
            CAException(kAudioHardwareBadObjectError),
            "BGM_Control::CheckObjectID: wrong audio object ID for the control");
}

#pragma clang assume_nonnull end

