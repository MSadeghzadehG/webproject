

#include <CoreAudio/AudioServerPlugIn.h>

#include "CADebugMacros.h"
#include "CAException.h"

#include "BGM_Types.h"
#include "BGM_Object.h"
#include "BGM_PlugIn.h"
#include "BGM_Device.h"
#include "BGM_NullDevice.h"


#pragma mark COM Prototypes

extern "C" void*	BGM_Create(CFAllocatorRef inAllocator, CFUUIDRef inRequestedTypeUUID);
static HRESULT		BGM_QueryInterface(void* inDriver, REFIID inUUID, LPVOID* outInterface);
static ULONG		BGM_AddRef(void* inDriver);
static ULONG		BGM_Release(void* inDriver);
static OSStatus		BGM_Initialize(AudioServerPlugInDriverRef inDriver, AudioServerPlugInHostRef inHost);
static OSStatus		BGM_CreateDevice(AudioServerPlugInDriverRef inDriver, CFDictionaryRef inDescription, const AudioServerPlugInClientInfo* inClientInfo, AudioObjectID* outDeviceObjectID);
static OSStatus		BGM_DestroyDevice(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID);
static OSStatus		BGM_AddDeviceClient(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, const AudioServerPlugInClientInfo* inClientInfo);
static OSStatus		BGM_RemoveDeviceClient(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, const AudioServerPlugInClientInfo* inClientInfo);
static OSStatus		BGM_PerformDeviceConfigurationChange(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt64 inChangeAction, void* inChangeInfo);
static OSStatus		BGM_AbortDeviceConfigurationChange(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt64 inChangeAction, void* inChangeInfo);
static Boolean		BGM_HasProperty(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress* inAddress);
static OSStatus		BGM_IsPropertySettable(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress* inAddress, Boolean* outIsSettable);
static OSStatus		BGM_GetPropertyDataSize(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32* outDataSize);
static OSStatus		BGM_GetPropertyData(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, UInt32* outDataSize, void* outData);
static OSStatus		BGM_SetPropertyData(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData);
static OSStatus		BGM_StartIO(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID);
static OSStatus		BGM_StopIO(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID);
static OSStatus		BGM_GetZeroTimeStamp(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID, Float64* outSampleTime, UInt64* outHostTime, UInt64* outSeed);
static OSStatus		BGM_WillDoIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID, UInt32 inOperationID, Boolean* outWillDo, Boolean* outWillDoInPlace);
static OSStatus		BGM_BeginIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo* inIOCycleInfo);
static OSStatus		BGM_DoIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, AudioObjectID inStreamObjectID, UInt32 inClientID, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo* inIOCycleInfo, void* ioMainBuffer, void* ioSecondaryBuffer);
static OSStatus		BGM_EndIOOperation(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt32 inClientID, UInt32 inOperationID, UInt32 inIOBufferFrameSize, const AudioServerPlugInIOCycleInfo* inIOCycleInfo);

#pragma mark The COM Interface

static AudioServerPlugInDriverInterface	gAudioServerPlugInDriverInterface =
{
	NULL,
	BGM_QueryInterface,
	BGM_AddRef,
	BGM_Release,
	BGM_Initialize,
	BGM_CreateDevice,
	BGM_DestroyDevice,
	BGM_AddDeviceClient,
	BGM_RemoveDeviceClient,
	BGM_PerformDeviceConfigurationChange,
	BGM_AbortDeviceConfigurationChange,
	BGM_HasProperty,
	BGM_IsPropertySettable,
	BGM_GetPropertyDataSize,
	BGM_GetPropertyData,
	BGM_SetPropertyData,
	BGM_StartIO,
	BGM_StopIO,
	BGM_GetZeroTimeStamp,
	BGM_WillDoIOOperation,
	BGM_BeginIOOperation,
	BGM_DoIOOperation,
	BGM_EndIOOperation
};
static AudioServerPlugInDriverInterface*	gAudioServerPlugInDriverInterfacePtr	= &gAudioServerPlugInDriverInterface;
static AudioServerPlugInDriverRef			gAudioServerPlugInDriverRef				= &gAudioServerPlugInDriverInterfacePtr;
static UInt32								gAudioServerPlugInDriverRefCount		= 1;

static BGM_Object& BGM_LookUpOwnerObject(AudioObjectID inObjectID)
{
    switch(inObjectID)
    {
        case kObjectID_PlugIn:
            return BGM_PlugIn::GetInstance();
            
        case kObjectID_Device:
        case kObjectID_Stream_Input:
        case kObjectID_Stream_Output:
        case kObjectID_Volume_Output_Master:
        case kObjectID_Mute_Output_Master:
            return BGM_Device::GetInstance();

        case kObjectID_Device_UI_Sounds:
        case kObjectID_Stream_Input_UI_Sounds:
        case kObjectID_Stream_Output_UI_Sounds:
        case kObjectID_Volume_Output_Master_UI_Sounds:
            return BGM_Device::GetUISoundsInstance();

        case kObjectID_Device_Null:
        case kObjectID_Stream_Null:
            return BGM_NullDevice::GetInstance();
    }
    
    DebugMsg("BGM_LookUpOwnerObject: unknown object");
    Throw(CAException(kAudioHardwareBadObjectError));
}

static BGM_AbstractDevice& BGM_LookUpDevice(AudioObjectID inObjectID)
{
    switch(inObjectID)
    {
        case kObjectID_Device:
            return BGM_Device::GetInstance();

        case kObjectID_Device_UI_Sounds:
            return BGM_Device::GetUISoundsInstance();

        case kObjectID_Device_Null:
            return BGM_NullDevice::GetInstance();
    }

    DebugMsg("BGM_LookUpDevice: unknown device");
    Throw(CAException(kAudioHardwareBadDeviceError));
}

#pragma mark Factory

extern "C"
void*	BGM_Create(CFAllocatorRef inAllocator, CFUUIDRef inRequestedTypeUUID)
{
									
	#pragma unused(inAllocator)
    
    void* theAnswer = NULL;
    if(CFEqual(inRequestedTypeUUID, kAudioServerPlugInTypeUUID))
    {
		theAnswer = gAudioServerPlugInDriverRef;
        
        BGM_PlugIn::GetInstance();
    }
    return theAnswer;
}

#pragma mark Inheritence

static HRESULT	BGM_QueryInterface(void* inDriver, REFIID inUUID, LPVOID* outInterface)
{
					
	HRESULT theAnswer = 0;
	CFUUIDRef theRequestedUUID = NULL;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "BGM_QueryInterface: bad driver reference");
		ThrowIfNULL(outInterface, CAException(kAudioHardwareIllegalOperationError), "BGM_QueryInterface: no place to store the returned interface");

    	    	theRequestedUUID = CFUUIDCreateFromUUIDBytes(NULL, inUUID);
    	ThrowIf(theRequestedUUID == NULL, CAException(kAudioHardwareIllegalOperationError), "BGM_QueryInterface: failed to create the CFUUIDRef");

								ThrowIf(!CFEqual(theRequestedUUID, IUnknownUUID) && !CFEqual(theRequestedUUID, kAudioServerPlugInDriverInterfaceUUID), CAException(E_NOINTERFACE), "BGM_QueryInterface: requested interface is unsupported");
		ThrowIf(gAudioServerPlugInDriverRefCount == UINT32_MAX, CAException(E_NOINTERFACE), "BGM_QueryInterface: the ref count is maxxed out");
		
				++gAudioServerPlugInDriverRefCount;
		*outInterface = gAudioServerPlugInDriverRef;
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
	}
	catch(...)
	{
		theAnswer = kAudioHardwareUnspecifiedError;
	}
    
    if(theRequestedUUID != NULL)
    {
    	CFRelease(theRequestedUUID);
    }
		
	return theAnswer;
}

static ULONG	BGM_AddRef(void* inDriver)
{
		
	ULONG theAnswer = 0;
	
		FailIf(inDriver != gAudioServerPlugInDriverRef, Done, "BGM_AddRef: bad driver reference");
	FailIf(gAudioServerPlugInDriverRefCount == UINT32_MAX, Done, "BGM_AddRef: out of references");

		++gAudioServerPlugInDriverRefCount;
	theAnswer = gAudioServerPlugInDriverRefCount;

Done:
	return theAnswer;
}

static ULONG	BGM_Release(void* inDriver)
{
	
	ULONG theAnswer = 0;
	
		FailIf(inDriver != gAudioServerPlugInDriverRef, Done, "BGM_Release: bad driver reference");
	FailIf(gAudioServerPlugInDriverRefCount == UINT32_MAX, Done, "BGM_Release: out of references");

					--gAudioServerPlugInDriverRefCount;
	theAnswer = gAudioServerPlugInDriverRefCount;

Done:
	return theAnswer;
}

#pragma mark Basic Operations

static OSStatus	BGM_Initialize(AudioServerPlugInDriverRef inDriver, AudioServerPlugInHostRef inHost)
{
						
	OSStatus theAnswer = 0;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef,
                CAException(kAudioHardwareBadObjectError),
                "BGM_Initialize: bad driver reference");
		
				BGM_PlugIn::GetInstance().SetHost(inHost);
        
                BGM_Device::GetInstance();
        BGM_Device::GetUISoundsInstance();
        BGM_NullDevice::GetInstance();
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
	}
	catch(...)
	{
		theAnswer = kAudioHardwareUnspecifiedError;
	}

	return theAnswer;
}

static OSStatus	BGM_CreateDevice(AudioServerPlugInDriverRef inDriver, CFDictionaryRef inDescription, const AudioServerPlugInClientInfo* inClientInfo, AudioObjectID* outDeviceObjectID)
{
				
	#pragma unused(inDriver, inDescription, inClientInfo, outDeviceObjectID)
	
	return kAudioHardwareUnsupportedOperationError;
}

static OSStatus	BGM_DestroyDevice(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID)
{
				
	#pragma unused(inDriver, inDeviceObjectID)
	
	return kAudioHardwareUnsupportedOperationError;
}

static OSStatus	BGM_AddDeviceClient(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, const AudioServerPlugInClientInfo* inClientInfo)
{
			
	OSStatus theAnswer = 0;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef,
                CAException(kAudioHardwareBadObjectError),
                "BGM_AddDeviceClient: bad driver reference");
		ThrowIf(inDeviceObjectID != kObjectID_Device && inDeviceObjectID != kObjectID_Device_UI_Sounds && inDeviceObjectID != kObjectID_Device_Null,
                CAException(kAudioHardwareBadObjectError),
                "BGM_AddDeviceClient: unknown device");
		
		        BGM_LookUpDevice(inDeviceObjectID).AddClient(inClientInfo);
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
	}
    catch(BGM_InvalidClientException)
    {
        theAnswer = kAudioHardwareIllegalOperationError;
    }
	catch(...)
	{
		theAnswer = kAudioHardwareUnspecifiedError;
	}
	
	return theAnswer;
}

static OSStatus	BGM_RemoveDeviceClient(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, const AudioServerPlugInClientInfo* inClientInfo)
{
			
	OSStatus theAnswer = 0;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef,
                CAException(kAudioHardwareBadObjectError),
                "BGM_RemoveDeviceClient: bad driver reference");
		ThrowIf(inDeviceObjectID != kObjectID_Device && inDeviceObjectID != kObjectID_Device_UI_Sounds && inDeviceObjectID != kObjectID_Device_Null,
                CAException(kAudioHardwareBadObjectError),
                "BGM_RemoveDeviceClient: unknown device");
		
                BGM_LookUpDevice(inDeviceObjectID).RemoveClient(inClientInfo);
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
    }
    catch(BGM_InvalidClientException)
    {
        theAnswer = kAudioHardwareIllegalOperationError;
    }
	catch(...)
	{
		theAnswer = kAudioHardwareUnspecifiedError;
	}
	
	return theAnswer;
}

static OSStatus	BGM_PerformDeviceConfigurationChange(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt64 inChangeAction, void* inChangeInfo)
{
									
	OSStatus theAnswer = 0;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef,
                CAException(kAudioHardwareBadObjectError),
                "BGM_PerformDeviceConfigurationChange: bad driver reference");
		ThrowIf(inDeviceObjectID != kObjectID_Device && inDeviceObjectID != kObjectID_Device_UI_Sounds && inDeviceObjectID != kObjectID_Device_Null,
                CAException(kAudioHardwareBadDeviceError),
                "BGM_PerformDeviceConfigurationChange: unknown device");
		
				BGM_LookUpDevice(inDeviceObjectID).PerformConfigChange(inChangeAction, inChangeInfo);
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
	}
	catch(...)
	{
		theAnswer = kAudioHardwareUnspecifiedError;
	}
	
	return theAnswer;
}

static OSStatus	BGM_AbortDeviceConfigurationChange(AudioServerPlugInDriverRef inDriver, AudioObjectID inDeviceObjectID, UInt64 inChangeAction, void* inChangeInfo)
{
		
	OSStatus theAnswer = 0;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef,
                CAException(kAudioHardwareBadObjectError),
                "BGM_PerformDeviceConfigurationChange: bad driver reference");
		ThrowIf(inDeviceObjectID != kObjectID_Device && inDeviceObjectID != kObjectID_Device_UI_Sounds && inDeviceObjectID != kObjectID_Device_Null,
                CAException(kAudioHardwareBadDeviceError),
                "BGM_PerformDeviceConfigurationChange: unknown device");
		
				BGM_LookUpDevice(inDeviceObjectID).AbortConfigChange(inChangeAction, inChangeInfo);
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
	}
	catch(...)
	{
		theAnswer = kAudioHardwareUnspecifiedError;
	}
	
	return theAnswer;
}

#pragma mark Property Operations

static Boolean	BGM_HasProperty(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress* inAddress)
{
		
	Boolean theAnswer = false;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "BGM_HasProperty: bad driver reference");
		ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "BGM_HasProperty: no address");
		
		theAnswer = BGM_LookUpOwnerObject(inObjectID).HasProperty(inObjectID, inClientProcessID, *inAddress);
	}
	catch(const CAException& inException)
	{
		theAnswer = false;
	}
	catch(...)
	{
		LogError("BGM_PlugInInterface::BGM_HasProperty: unknown exception. (object: %u, address: %u)",
				 inObjectID,
				 inAddress ? inAddress->mSelector : 0);
		theAnswer = false;
	}

	return theAnswer;
}

static OSStatus	BGM_IsPropertySettable(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress* inAddress, Boolean* outIsSettable)
{
			
	OSStatus theAnswer = 0;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "BGM_IsPropertySettable: bad driver reference");
		ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "BGM_IsPropertySettable: no address");
		ThrowIfNULL(outIsSettable, CAException(kAudioHardwareIllegalOperationError), "BGM_IsPropertySettable: no place to put the return value");
        
        BGM_Object& theAudioObject = BGM_LookUpOwnerObject(inObjectID);
		if(theAudioObject.HasProperty(inObjectID, inClientProcessID, *inAddress))
		{
			*outIsSettable = theAudioObject.IsPropertySettable(inObjectID, inClientProcessID, *inAddress);
		}
		else
		{
			theAnswer = kAudioHardwareUnknownPropertyError;
		}
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
	}
	catch(...)
	{
		LogError("BGM_PlugInInterface::BGM_IsPropertySettable: unknown exception. (object: %u, address: %u)",
				 inObjectID,
				 inAddress ? inAddress->mSelector : 0);
		theAnswer = kAudioHardwareUnspecifiedError;
	}
	
	return theAnswer;
}

static OSStatus	BGM_GetPropertyDataSize(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32* outDataSize)
{
		
	OSStatus theAnswer = 0;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "BGM_GetPropertyDataSize: bad driver reference");
		ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "BGM_GetPropertyDataSize: no address");
		ThrowIfNULL(outDataSize, CAException(kAudioHardwareIllegalOperationError), "BGM_GetPropertyDataSize: no place to put the return value");
        
        BGM_Object& theAudioObject = BGM_LookUpOwnerObject(inObjectID);
		if(theAudioObject.HasProperty(inObjectID, inClientProcessID, *inAddress))
		{
			*outDataSize = theAudioObject.GetPropertyDataSize(inObjectID, inClientProcessID, *inAddress, inQualifierDataSize, inQualifierData);
		}
		else
		{
			theAnswer = kAudioHardwareUnknownPropertyError;
		}
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
	}
	catch(...)
	{
		LogError("BGM_PlugInInterface::BGM_GetPropertyDataSize: unknown exception. (object: %u, address: %u)",
				 inObjectID,
				 inAddress ? inAddress->mSelector : 0);
		theAnswer = kAudioHardwareUnspecifiedError;
	}

	return theAnswer;
}

static OSStatus	BGM_GetPropertyData(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, UInt32* outDataSize, void* outData)
{
		
	OSStatus theAnswer = 0;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "BGM_GetPropertyData: bad driver reference");
		ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "BGM_GetPropertyData: no address");
		ThrowIfNULL(outDataSize, CAException(kAudioHardwareIllegalOperationError), "BGM_GetPropertyData: no place to put the return value size");
		ThrowIfNULL(outData, CAException(kAudioHardwareIllegalOperationError), "BGM_GetPropertyData: no place to put the return value");
		
        BGM_Object& theAudioObject = BGM_LookUpOwnerObject(inObjectID);
		if(theAudioObject.HasProperty(inObjectID, inClientProcessID, *inAddress))
		{
			theAudioObject.GetPropertyData(inObjectID, inClientProcessID, *inAddress, inQualifierDataSize, inQualifierData, inDataSize, *outDataSize, outData);
		}
		else
		{
			theAnswer = kAudioHardwareUnknownPropertyError;
		}
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
	}
	catch(...)
	{
		LogError("BGM_PlugInInterface::BGM_GetPropertyData: unknown exception. (object: %u, address: %u)",
                 inObjectID,
				 inAddress ? inAddress->mSelector : 0);
		theAnswer = kAudioHardwareUnspecifiedError;
	}

	return theAnswer;
}

static OSStatus	BGM_SetPropertyData(AudioServerPlugInDriverRef inDriver, AudioObjectID inObjectID, pid_t inClientProcessID, const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize, const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
	
	OSStatus theAnswer = 0;

	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef, CAException(kAudioHardwareBadObjectError), "BGM_SetPropertyData: bad driver reference");
        ThrowIfNULL(inAddress, CAException(kAudioHardwareIllegalOperationError), "BGM_SetPropertyData: no address");
        ThrowIfNULL(inData, CAException(kAudioHardwareIllegalOperationError), "BGM_SetPropertyData: no data");
		
        BGM_Object& theAudioObject = BGM_LookUpOwnerObject(inObjectID);
		if(theAudioObject.HasProperty(inObjectID, inClientProcessID, *inAddress))
		{
			if(theAudioObject.IsPropertySettable(inObjectID, inClientProcessID, *inAddress))
			{
				theAudioObject.SetPropertyData(inObjectID, inClientProcessID, *inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
			}
			else
			{
				theAnswer = kAudioHardwareUnsupportedOperationError;
			}
		}
		else
		{
			theAnswer = kAudioHardwareUnknownPropertyError;
		}
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
	}
	catch(...)
	{
		LogError("BGM_PlugInInterface::BGM_SetPropertyData: unknown exception. (object: %u, address: %u)",
				 inObjectID,
				 inAddress ? inAddress->mSelector : 0);
		theAnswer = kAudioHardwareUnspecifiedError;
	}
	
	return theAnswer;
}

#pragma mark IO Operations

static OSStatus	BGM_StartIO(AudioServerPlugInDriverRef inDriver,
                            AudioObjectID inDeviceObjectID,
                            UInt32 inClientID)
{
						
	OSStatus theAnswer = 0;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef,
                CAException(kAudioHardwareBadObjectError),
                "BGM_StartIO: bad driver reference");
		ThrowIf(inDeviceObjectID != kObjectID_Device && inDeviceObjectID != kObjectID_Device_UI_Sounds && inDeviceObjectID != kObjectID_Device_Null,
                CAException(kAudioHardwareBadDeviceError),
                "BGM_StartIO: unknown device");
		
		        BGM_LookUpDevice(inDeviceObjectID).StartIO(inClientID);
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
	}
	catch(...)
	{
		theAnswer = kAudioHardwareUnspecifiedError;
	}
	
	return theAnswer;
}

static OSStatus	BGM_StopIO(AudioServerPlugInDriverRef inDriver,
                           AudioObjectID inDeviceObjectID,
                           UInt32 inClientID)
{
			
	OSStatus theAnswer = 0;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef,
                CAException(kAudioHardwareBadObjectError),
                "BGM_StopIO: bad driver reference");
		ThrowIf(inDeviceObjectID != kObjectID_Device && inDeviceObjectID != kObjectID_Device_UI_Sounds && inDeviceObjectID != kObjectID_Device_Null,
                CAException(kAudioHardwareBadDeviceError),
                "BGM_StopIO: unknown device");
		
				BGM_LookUpDevice(inDeviceObjectID).StopIO(inClientID);
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
	}
	catch(...)
	{
		theAnswer = kAudioHardwareUnspecifiedError;
	}
	
	return theAnswer;
}

static OSStatus	BGM_GetZeroTimeStamp(AudioServerPlugInDriverRef inDriver,
                                     AudioObjectID inDeviceObjectID,
                                     UInt32 inClientID,
                                     Float64* outSampleTime,
                                     UInt64* outHostTime,
                                     UInt64* outSeed)
{
    #pragma unused(inClientID)
					
	OSStatus theAnswer = 0;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef,
                CAException(kAudioHardwareBadObjectError),
                "BGM_GetZeroTimeStamp: bad driver reference");
		ThrowIfNULL(outSampleTime,
                    CAException(kAudioHardwareIllegalOperationError),
                    "BGM_GetZeroTimeStamp: no place to put the sample time");
		ThrowIfNULL(outHostTime,
                    CAException(kAudioHardwareIllegalOperationError),
                    "BGM_GetZeroTimeStamp: no place to put the host time");
		ThrowIfNULL(outSeed,
                    CAException(kAudioHardwareIllegalOperationError),
                    "BGM_GetZeroTimeStamp: no place to put the seed");
		ThrowIf(inDeviceObjectID != kObjectID_Device && inDeviceObjectID != kObjectID_Device_UI_Sounds && inDeviceObjectID != kObjectID_Device_Null,
                CAException(kAudioHardwareBadDeviceError),
                "BGM_GetZeroTimeStamp: unknown device");
		
				BGM_LookUpDevice(inDeviceObjectID).GetZeroTimeStamp(*outSampleTime, *outHostTime, *outSeed);
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
	}
	catch(...)
	{
		theAnswer = kAudioHardwareUnspecifiedError;
	}
	
	return theAnswer;
}

static OSStatus	BGM_WillDoIOOperation(AudioServerPlugInDriverRef inDriver,
                                      AudioObjectID inDeviceObjectID,
                                      UInt32 inClientID,
                                      UInt32 inOperationID,
                                      Boolean* outWillDo,
                                      Boolean* outWillDoInPlace)
{
	#pragma unused(inClientID)
	
	OSStatus theAnswer = 0;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef,
                CAException(kAudioHardwareBadObjectError),
                "BGM_WillDoIOOperation: bad driver reference");
		ThrowIfNULL(outWillDo,
                    CAException(kAudioHardwareIllegalOperationError),
                    "BGM_WillDoIOOperation: no place to put the will-do return value");
		ThrowIfNULL(outWillDoInPlace,
                    CAException(kAudioHardwareIllegalOperationError),
                    "BGM_WillDoIOOperation: no place to put the in-place return value");
		ThrowIf(inDeviceObjectID != kObjectID_Device && inDeviceObjectID != kObjectID_Device_UI_Sounds && inDeviceObjectID != kObjectID_Device_Null,
                CAException(kAudioHardwareBadDeviceError),
                "BGM_WillDoIOOperation: unknown device");
		
				bool willDo = false;
		bool willDoInPlace = false;
		BGM_LookUpDevice(inDeviceObjectID).WillDoIOOperation(inOperationID, willDo, willDoInPlace);
		
				*outWillDo = willDo;
		*outWillDoInPlace = willDoInPlace;
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
	}
	catch(...)
	{
		theAnswer = kAudioHardwareUnspecifiedError;
	}

	return theAnswer;
}

static OSStatus	BGM_BeginIOOperation(AudioServerPlugInDriverRef inDriver,
                                     AudioObjectID inDeviceObjectID,
                                     UInt32 inClientID,
                                     UInt32 inOperationID,
                                     UInt32 inIOBufferFrameSize,
                                     const AudioServerPlugInIOCycleInfo* inIOCycleInfo)
{
		
	OSStatus theAnswer = 0;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef,
                CAException(kAudioHardwareBadObjectError),
                "BGM_BeginIOOperation: bad driver reference");
		ThrowIfNULL(inIOCycleInfo,
                    CAException(kAudioHardwareIllegalOperationError),
                    "BGM_BeginIOOperation: no cycle info");
		ThrowIf(inDeviceObjectID != kObjectID_Device && inDeviceObjectID != kObjectID_Device_UI_Sounds && inDeviceObjectID != kObjectID_Device_Null,
                CAException(kAudioHardwareBadDeviceError),
                "BGM_BeginIOOperation: unknown device");
		
				BGM_LookUpDevice(inDeviceObjectID).BeginIOOperation(inOperationID,
                                                            inIOBufferFrameSize,
                                                            *inIOCycleInfo,
                                                            inClientID);
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
	}
	catch(...)
	{
		DebugMsg("BGM_PlugInInterface::BGM_BeginIOOperation: unknown exception. (device: %s, operation: %u)",
				 (inDeviceObjectID == kObjectID_Device ? "BGMDevice" : "other"),
				 inOperationID);
		theAnswer = kAudioHardwareUnspecifiedError;
	}
	
	return theAnswer;
}

static OSStatus	BGM_DoIOOperation(AudioServerPlugInDriverRef inDriver,
                                  AudioObjectID inDeviceObjectID,
                                  AudioObjectID inStreamObjectID,
                                  UInt32 inClientID,
                                  UInt32 inOperationID,
                                  UInt32 inIOBufferFrameSize,
                                  const AudioServerPlugInIOCycleInfo* inIOCycleInfo,
                                  void* ioMainBuffer,
                                  void* ioSecondaryBuffer)
{
		
	OSStatus theAnswer = 0;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef,
                CAException(kAudioHardwareBadObjectError),
                "BGM_EndIOOperation: bad driver reference");
		ThrowIfNULL(inIOCycleInfo,
                    CAException(kAudioHardwareIllegalOperationError),
                    "BGM_EndIOOperation: no cycle info");
		ThrowIf(inDeviceObjectID != kObjectID_Device && inDeviceObjectID != kObjectID_Device_UI_Sounds && inDeviceObjectID != kObjectID_Device_Null,
                CAException(kAudioHardwareBadDeviceError),
                "BGM_EndIOOperation: unknown device");
		
				BGM_LookUpDevice(inDeviceObjectID).DoIOOperation(inStreamObjectID,
                                                         inClientID,
                                                         inOperationID,
                                                         inIOBufferFrameSize,
                                                         *inIOCycleInfo,
                                                         ioMainBuffer,
                                                         ioSecondaryBuffer);
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
	}
	catch(...)
	{
		DebugMsg("BGM_PlugInInterface::BGM_DoIOOperation: unknown exception. (device: %s, operation: %u)",
				 (inDeviceObjectID == kObjectID_Device ? "BGMDevice" : "other"),
				 inOperationID);
		theAnswer = kAudioHardwareUnspecifiedError;
	}

	return theAnswer;
}

static OSStatus	BGM_EndIOOperation(AudioServerPlugInDriverRef inDriver,
                                   AudioObjectID inDeviceObjectID,
                                   UInt32 inClientID,
                                   UInt32 inOperationID,
                                   UInt32 inIOBufferFrameSize,
                                   const AudioServerPlugInIOCycleInfo* inIOCycleInfo)
{
		
	OSStatus theAnswer = 0;
	
	try
	{
				ThrowIf(inDriver != gAudioServerPlugInDriverRef,
                CAException(kAudioHardwareBadObjectError),
                "BGM_EndIOOperation: bad driver reference");
		ThrowIfNULL(inIOCycleInfo,
                    CAException(kAudioHardwareIllegalOperationError),
                    "BGM_EndIOOperation: no cycle info");
		ThrowIf(inDeviceObjectID != kObjectID_Device && inDeviceObjectID != kObjectID_Device_UI_Sounds && inDeviceObjectID != kObjectID_Device_Null,
                CAException(kAudioHardwareBadDeviceError),
                "BGM_EndIOOperation: unknown device");
		
				BGM_LookUpDevice(inDeviceObjectID).EndIOOperation(inOperationID,
                                                          inIOBufferFrameSize,
                                                          *inIOCycleInfo,
                                                          inClientID);
	}
	catch(const CAException& inException)
	{
		theAnswer = inException.GetError();
	}
	catch(...)
	{
		DebugMsg("BGM_PlugInInterface::BGM_EndIOOperation: unknown exception. (device: %s, operation: %u)",
				 (inDeviceObjectID == kObjectID_Device ? "BGMDevice" : "other"),
				 inOperationID);
		theAnswer = kAudioHardwareUnspecifiedError;
	}
	
	return theAnswer;
}

