

#include "BGM_PlugIn.h"

#include "BGM_Device.h"
#include "BGM_NullDevice.h"

#include "CAException.h"
#include "CADebugMacros.h"
#include "CAPropertyAddress.h"
#include "CADispatchQueue.h"


#pragma mark Construction/Destruction

pthread_once_t				BGM_PlugIn::sStaticInitializer = PTHREAD_ONCE_INIT;
BGM_PlugIn*					BGM_PlugIn::sInstance = NULL;
AudioServerPlugInHostRef	BGM_PlugIn::sHost = NULL;

BGM_PlugIn& BGM_PlugIn::GetInstance()
{
    pthread_once(&sStaticInitializer, StaticInitializer);
    return *sInstance;
}

void	BGM_PlugIn::StaticInitializer()
{
    try
    {
        sInstance = new BGM_PlugIn;
        sInstance->Activate();
    }
    catch(...)
    {
        DebugMsg("BGM_PlugIn::StaticInitializer: failed to create the plug-in");
        delete sInstance;
        sInstance = NULL;
    }
}

BGM_PlugIn::BGM_PlugIn()
:
	BGM_Object(kAudioObjectPlugInObject, kAudioPlugInClassID, kAudioObjectClassID, 0),
	mMutex("BGM_PlugIn")
{
}

BGM_PlugIn::~BGM_PlugIn()
{
}

void	BGM_PlugIn::Deactivate()
{
	CAMutex::Locker theLocker(mMutex);
	BGM_Object::Deactivate();
    	}

#pragma mark Property Operations

bool	BGM_PlugIn::HasProperty(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const
{
	bool theAnswer = false;
	switch(inAddress.mSelector)
	{
		case kAudioObjectPropertyManufacturer:
		case kAudioPlugInPropertyDeviceList:
		case kAudioPlugInPropertyTranslateUIDToDevice:
        case kAudioPlugInPropertyResourceBundle:
        case kAudioObjectPropertyCustomPropertyInfoList:
        case kAudioPlugInCustomPropertyNullDeviceActive:
			theAnswer = true;
			break;
		
		default:
			theAnswer = BGM_Object::HasProperty(inObjectID, inClientPID, inAddress);
	};
	return theAnswer;
}

bool	BGM_PlugIn::IsPropertySettable(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress) const
{
	bool theAnswer = false;
	switch(inAddress.mSelector)
	{
		case kAudioObjectPropertyManufacturer:
		case kAudioPlugInPropertyDeviceList:
		case kAudioPlugInPropertyTranslateUIDToDevice:
        case kAudioPlugInPropertyResourceBundle:
        case kAudioObjectPropertyCustomPropertyInfoList:
			theAnswer = false;
			break;

        case kAudioPlugInCustomPropertyNullDeviceActive:
            theAnswer = true;
            break;
		
		default:
			theAnswer = BGM_Object::IsPropertySettable(inObjectID, inClientPID, inAddress);
	};
	return theAnswer;
}

UInt32	BGM_PlugIn::GetPropertyDataSize(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData) const
{
	UInt32 theAnswer = 0;
	switch(inAddress.mSelector)
	{
		case kAudioObjectPropertyManufacturer:
			theAnswer = sizeof(CFStringRef);
			break;
			
		case kAudioObjectPropertyOwnedObjects:
		case kAudioPlugInPropertyDeviceList:
                                    theAnswer = (BGM_NullDevice::GetInstance().IsActive() ? 3 : 2) * sizeof(AudioObjectID);
			break;
			
		case kAudioPlugInPropertyTranslateUIDToDevice:
			theAnswer = sizeof(AudioObjectID);
			break;
			
		case kAudioPlugInPropertyResourceBundle:
			theAnswer = sizeof(CFStringRef);
			break;

        case kAudioObjectPropertyCustomPropertyInfoList:
            theAnswer = sizeof(AudioServerPlugInCustomPropertyInfo);
            break;

        case kAudioPlugInCustomPropertyNullDeviceActive:
            theAnswer = sizeof(CFBooleanRef);
            break;
		
		default:
			theAnswer = BGM_Object::GetPropertyDataSize(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData);
	};
	return theAnswer;
}

void	BGM_PlugIn::GetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, UInt32& outDataSize, void* outData) const
{
	switch(inAddress.mSelector)
	{
		case kAudioObjectPropertyManufacturer:
						ThrowIf(inDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "BGM_PlugIn::GetPropertyData: not enough space for the return value of kAudioObjectPropertyManufacturer");
			*reinterpret_cast<CFStringRef*>(outData) = CFSTR("Background Music contributors");
			outDataSize = sizeof(CFStringRef);
			break;
			
		case kAudioObjectPropertyOwnedObjects:
            		case kAudioPlugInPropertyDeviceList:
            {
    			AudioObjectID* theReturnedDeviceList = reinterpret_cast<AudioObjectID*>(outData);
                if(inDataSize >= 3 * sizeof(AudioObjectID))
                {
                    if(BGM_NullDevice::GetInstance().IsActive())
                    {
                        theReturnedDeviceList[0] = kObjectID_Device;
                        theReturnedDeviceList[1] = kObjectID_Device_UI_Sounds;
                        theReturnedDeviceList[2] = kObjectID_Device_Null;
                        
                                                outDataSize = 3 * sizeof(AudioObjectID);
                    }
                    else
                    {
                        theReturnedDeviceList[0] = kObjectID_Device;
                        theReturnedDeviceList[1] = kObjectID_Device_UI_Sounds;

                                                outDataSize = 2 * sizeof(AudioObjectID);
                    }
                }
                else if(inDataSize >= 2 * sizeof(AudioObjectID))
                {
                    theReturnedDeviceList[0] = kObjectID_Device;
                    theReturnedDeviceList[1] = kObjectID_Device_UI_Sounds;

                                        outDataSize = 2 * sizeof(AudioObjectID);
                }
                else if(inDataSize >= sizeof(AudioObjectID))
                {
                    theReturnedDeviceList[0] = kObjectID_Device;
                    outDataSize = sizeof(AudioObjectID);
                }
                else
                {
                    outDataSize = 0;
                }
            }
			break;
			
		case kAudioPlugInPropertyTranslateUIDToDevice:
            {
                                                                ThrowIf(inQualifierDataSize < sizeof(CFStringRef), CAException(kAudioHardwareBadPropertySizeError), "BGM_PlugIn::GetPropertyData: the qualifier size is too small for kAudioPlugInPropertyTranslateUIDToDevice");
                ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "BGM_PlugIn::GetPropertyData: not enough space for the return value of kAudioPlugInPropertyTranslateUIDToDevice");

                CFStringRef theUID = *reinterpret_cast<const CFStringRef*>(inQualifierData);
                AudioObjectID* outID = reinterpret_cast<AudioObjectID*>(outData);

                if(CFEqual(theUID, BGM_Device::GetInstance().CopyDeviceUID()))
                {
                    DebugMsg("BGM_PlugIn::GetPropertyData: Returning BGMDevice for "
                             "kAudioPlugInPropertyTranslateUIDToDevice");
                    *outID = kObjectID_Device;
                }
                else if(CFEqual(theUID, BGM_Device::GetUISoundsInstance().CopyDeviceUID()))
                {
                    DebugMsg("BGM_PlugIn::GetPropertyData: Returning BGMUISoundsDevice for "
                             "kAudioPlugInPropertyTranslateUIDToDevice");
                    *outID = kObjectID_Device_UI_Sounds;
                }
                else if(BGM_NullDevice::GetInstance().IsActive() &&
                        CFEqual(theUID, BGM_NullDevice::GetInstance().CopyDeviceUID()))
                {
                    DebugMsg("BGM_PlugIn::GetPropertyData: Returning null device for "
                             "kAudioPlugInPropertyTranslateUIDToDevice");
                    *outID = kObjectID_Device_Null;
                }
                else
                {
                    LogWarning("BGM_PlugIn::GetPropertyData: Returning kAudioObjectUnknown for "
                               "kAudioPlugInPropertyTranslateUIDToDevice");
                    *outID = kAudioObjectUnknown;
                }

                outDataSize = sizeof(AudioObjectID);
            }
			break;
			
		case kAudioPlugInPropertyResourceBundle:
												ThrowIf(inDataSize < sizeof(AudioObjectID), CAException(kAudioHardwareBadPropertySizeError), "BGM_GetPlugInPropertyData: not enough space for the return value of kAudioPlugInPropertyResourceBundle");
			*reinterpret_cast<CFStringRef*>(outData) = CFSTR("");
			outDataSize = sizeof(CFStringRef);
			break;

        case kAudioObjectPropertyCustomPropertyInfoList:
            if(inDataSize >= sizeof(AudioServerPlugInCustomPropertyInfo))
            {
                AudioServerPlugInCustomPropertyInfo* outCustomProperties =
                    reinterpret_cast<AudioServerPlugInCustomPropertyInfo*>(outData);

                outCustomProperties[0].mSelector = kAudioPlugInCustomPropertyNullDeviceActive;
                outCustomProperties[0].mPropertyDataType =
                    kAudioServerPlugInCustomPropertyDataTypeCFPropertyList;
                outCustomProperties[0].mQualifierDataType =
                    kAudioServerPlugInCustomPropertyDataTypeNone;

                outDataSize = sizeof(AudioServerPlugInCustomPropertyInfo);
            }
            else
            {
                outDataSize = 0;
            }
            break;

        case kAudioPlugInCustomPropertyNullDeviceActive:
            ThrowIf(inDataSize < sizeof(CFBooleanRef),
                    CAException(kAudioHardwareBadPropertySizeError),
                    "BGM_PlugIn::GetPropertyData: not enough space for the return value of "
                    "kAudioPlugInCustomPropertyNullDeviceActive");
            *reinterpret_cast<CFBooleanRef*>(outData) =
                BGM_NullDevice::GetInstance().IsActive() ? kCFBooleanTrue : kCFBooleanFalse;
            outDataSize = sizeof(CFBooleanRef);
            break;

		default:
			BGM_Object::GetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, outDataSize, outData);
			break;
	};
}

void	BGM_PlugIn::SetPropertyData(AudioObjectID inObjectID, pid_t inClientPID, const AudioObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
	switch(inAddress.mSelector)
	{
        case kAudioPlugInCustomPropertyNullDeviceActive:
            {
                ThrowIf(inDataSize < sizeof(CFBooleanRef),
                        CAException(kAudioHardwareBadPropertySizeError),
                        "BGM_PlugIn::SetPropertyData: wrong size for the data for "
                        "kAudioPlugInCustomPropertyNullDeviceActive");

                CFBooleanRef theIsActiveRef = *reinterpret_cast<const CFBooleanRef*>(inData);

                ThrowIfNULL(theIsActiveRef,
                            CAException(kAudioHardwareIllegalOperationError),
                            "BGM_PlugIn::SetPropertyData: null reference given for "
                            "kAudioPlugInCustomPropertyNullDeviceActive");
                ThrowIf(CFGetTypeID(theIsActiveRef) != CFBooleanGetTypeID(),
                        CAException(kAudioHardwareIllegalOperationError),
                        "BGM_PlugIn::SetPropertyData: CFType given for "
                        "kAudioPlugInCustomPropertyNullDeviceActive was not a CFBoolean");

                bool theIsActive = CFBooleanGetValue(theIsActiveRef);

                if(theIsActive != BGM_NullDevice::GetInstance().IsActive())
                {
                                                                                if(theIsActive)
                    {
                        DebugMsg("BGM_PlugIn::SetPropertyData: Activating null device");
                        BGM_NullDevice::GetInstance().Activate();
                    }
                    else
                    {
                        DebugMsg("BGM_PlugIn::SetPropertyData: Deactivating null device");
                        BGM_NullDevice::GetInstance().Deactivate();
                    }

                                        CADispatchQueue::GetGlobalSerialQueue().Dispatch(false, ^{
                        AudioObjectPropertyAddress theChangedProperties[] = {
                            CAPropertyAddress(kAudioObjectPropertyOwnedObjects),
                            CAPropertyAddress(kAudioPlugInPropertyDeviceList)
                        };

                        Host_PropertiesChanged(GetObjectID(), 2, theChangedProperties);
                    });
                }
            }
            break;
            
		default:
			BGM_Object::SetPropertyData(inObjectID, inClientPID, inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
			break;
	};
}

