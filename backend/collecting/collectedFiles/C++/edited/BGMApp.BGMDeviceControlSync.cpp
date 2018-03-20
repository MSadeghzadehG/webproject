

#include "BGMDeviceControlSync.h"

#include "BGM_Types.h"
#include "BGM_Utils.h"

#include "CAPropertyAddress.h"


#pragma clang assume_nonnull begin

static const AudioObjectPropertyAddress kMutePropertyAddress =
    { kAudioDevicePropertyMute, kAudioObjectPropertyScopeOutput, kAudioObjectPropertyElementMaster };

static const AudioObjectPropertyAddress kVolumePropertyAddress =
    { kAudioDevicePropertyVolumeScalar, kAudioObjectPropertyScopeOutput, kAudioObjectPropertyElementMaster };

#pragma mark Construction/Destruction

BGMDeviceControlSync::BGMDeviceControlSync(AudioObjectID inBGMDevice,
                                           AudioObjectID inOutputDevice,
                                           CAHALAudioSystemObject inAudioSystem)
:
    mBGMDevice(inBGMDevice),
    mOutputDevice(inOutputDevice),
    mAudioSystem(inAudioSystem),
    mBGMDeviceControlsList(inBGMDevice)
{
}

BGMDeviceControlSync::~BGMDeviceControlSync()
{
    BGMLogAndSwallowExceptions("BGMDeviceControlSync::~BGMDeviceControlSync", [&] {
        CAMutex::Locker locker(mMutex);

        Deactivate();
    });
}

void    BGMDeviceControlSync::Activate()
{
    CAMutex::Locker locker(mMutex);

    ThrowIf((mBGMDevice.GetObjectID() == kAudioObjectUnknown || mOutputDevice.GetObjectID() == kAudioObjectUnknown),
            BGM_DeviceNotSetException(),
            "BGMDeviceControlSync::Activate: Both the output device and BGMDevice must be set to start synchronizing their controls");

    if(!mActive)
    {
        DebugMsg("BGMDeviceControlSync::Activate: Activating control sync");

                                                BGMLogAndSwallowExceptionsMsg("BGMDeviceControlSync::Activate", "Controls list", [&] {
            bool wasUpdated = mBGMDeviceControlsList.MatchControlsListOf(mOutputDevice);
            if(wasUpdated)
            {
                mBGMDeviceControlsList.PropagateControlListChange();
            }
        });

                mBGMDevice.CopyVolumeFrom(mOutputDevice, kAudioObjectPropertyScopeOutput);
        mBGMDevice.CopyMuteFrom(mOutputDevice, kAudioObjectPropertyScopeOutput);

                mBGMDevice.AddPropertyListener(kVolumePropertyAddress, &BGMDeviceControlSync::BGMDeviceListenerProc, this);
        
        try
        {
            mBGMDevice.AddPropertyListener(kMutePropertyAddress, &BGMDeviceControlSync::BGMDeviceListenerProc, this);
        }
        catch(CAException)
        {
            CATry
            mBGMDevice.RemovePropertyListener(kVolumePropertyAddress, &BGMDeviceControlSync::BGMDeviceListenerProc, this);
            CACatch
            
            throw;
        }
        
        mActive = true;
    }
    else
    {
        DebugMsg("BGMDeviceControlSync::Activate: Already active");
    }
}

void    BGMDeviceControlSync::Deactivate()
{
    CAMutex::Locker locker(mMutex);

    if(mActive)
    {
        DebugMsg("BGMDeviceControlSync::Deactivate: Deactivating control sync");

                if(mBGMDevice.GetObjectID() != kAudioDeviceUnknown)
        {
            BGMLogAndSwallowExceptions("BGMDeviceControlSync::Deactivate", [&] {
                mBGMDevice.RemovePropertyListener(kVolumePropertyAddress,
                                                  &BGMDeviceControlSync::BGMDeviceListenerProc,
                                                  this);
            });

            BGMLogAndSwallowExceptions("BGMDeviceControlSync::Deactivate", [&] {
                mBGMDevice.RemovePropertyListener(kMutePropertyAddress,
                                                  &BGMDeviceControlSync::BGMDeviceListenerProc,
                                                  this);
            });
        }

        mActive = false;
    }
    else
    {
        DebugMsg("BGMDeviceControlSync::Deactivate: Not active");
    }
}

#pragma mark Accessors

void    BGMDeviceControlSync::SetDevices(AudioObjectID inBGMDevice, AudioObjectID inOutputDevice)
{
    CAMutex::Locker locker(mMutex);

    bool wasActive = mActive;

    Deactivate();

    mBGMDevice = inBGMDevice;
    mBGMDeviceControlsList.SetBGMDevice(inBGMDevice);
    mOutputDevice = inOutputDevice;
    
    if(wasActive)
    {
        Activate();
    }
}

#pragma mark Listener Procs

OSStatus    BGMDeviceControlSync::BGMDeviceListenerProc(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress* inAddresses, void* __nullable inClientData)
{
        BGMDeviceControlSync* refCon = static_cast<BGMDeviceControlSync*>(inClientData);

    auto checkState = [&] {
        if(!refCon)
        {
            LogError("BGMDeviceControlSync::BGMDeviceListenerProc: !refCon");
            return false;
        }

        if(!refCon->mActive ||
           (refCon->mBGMDevice.GetObjectID() == kAudioObjectUnknown) ||
           (refCon->mOutputDevice.GetObjectID() == kAudioObjectUnknown))
        {
            return false;
        }

        if(inObjectID != refCon->mBGMDevice.GetObjectID())
        {
            LogError("BGMDeviceControlSync::BGMDeviceListenerProc: notified about audio object other than BGMDevice");
            return false;
        }
        
        return true;
    };

    for(int i = 0; i < inNumberAddresses; i++)
    {
        AudioObjectPropertyScope scope = inAddresses[i].mScope;
        
        switch(inAddresses[i].mSelector)
        {
            case kAudioDevicePropertyVolumeScalar:
                {
                    CAMutex::Locker locker(refCon->mMutex);

                                        if(checkState())
                    {
                        refCon->mOutputDevice.CopyVolumeFrom(refCon->mBGMDevice, scope);
                    }
                }
                break;
                
            case kAudioDevicePropertyMute:
                {
                    CAMutex::Locker locker(refCon->mMutex);

                                                            if(checkState())
                    {
                        refCon->mOutputDevice.CopyMuteFrom(refCon->mBGMDevice, scope);
                    }
                }
                break;
        }
    }

        return 0;
}

#pragma clang assume_nonnull end

