



#include "CADispatchQueue.h"

#include "CACFString.h"
#include "CADebugMacros.h"
#include "CAException.h"
#include "CAHostTimeBase.h"

#include <mach/mach.h>

#include <algorithm>


CADispatchQueue::CADispatchQueue(const char* inName)
:
	mDispatchQueue(NULL),
	mPortDeathList(),
	mMachPortReceiverList()
{
	mDispatchQueue = dispatch_queue_create(inName, NULL);
	ThrowIfNULL(mDispatchQueue, CAException('what'), "CADispatchQueue::CADispatchQueue: failed to create the dispatch queue");
}

CADispatchQueue::CADispatchQueue(CFStringRef inName)
:
	mDispatchQueue(NULL),
	mPortDeathList(),
	mMachPortReceiverList()
{
	CACFString theCFName(inName, false);
	char theName[256];
	UInt32 theSize = 256;
	theCFName.GetCString(theName, theSize);
	mDispatchQueue = dispatch_queue_create(theName, NULL);
	ThrowIfNULL(mDispatchQueue, CAException('what'), "CADispatchQueue::CADispatchQueue: failed to create the dispatch queue");
}

CADispatchQueue::CADispatchQueue(CFStringRef inPattern, CFStringRef inName)
:
	mDispatchQueue(NULL),
	mPortDeathList(),
	mMachPortReceiverList()
{
	CACFString theCFName(CFStringCreateWithFormat(NULL, NULL, inPattern, inName), true);
	char theName[256];
	UInt32 theSize = 256;
	theCFName.GetCString(theName, theSize);
	mDispatchQueue = dispatch_queue_create(theName, NULL);
	ThrowIfNULL(mDispatchQueue, CAException('what'), "CADispatchQueue::CADispatchQueue: failed to create the dispatch queue");
}

CADispatchQueue::~CADispatchQueue()
{
			mPortDeathList.clear();
	Assert(mMachPortReceiverList.size() == 0, "CADispatchQueue::~CADispatchQueue: Implicitly removing the mach port receviers. It is best to explicitly call RemoveMachPortRecevier().");
	mMachPortReceiverList.clear();
	
		dispatch_release(mDispatchQueue);
}

void	CADispatchQueue::Dispatch(bool inDoSync, dispatch_block_t inTask) const
{
	if(inDoSync)
	{
				dispatch_sync(mDispatchQueue, inTask);
	}
	else
	{
		dispatch_async(mDispatchQueue, inTask);
	}
}

void	CADispatchQueue::Dispatch(UInt64 inNanoseconds, dispatch_block_t inTask) const
{
	if(inNanoseconds == 0)
	{
		dispatch_async(mDispatchQueue, inTask);
	}
	else
	{
		dispatch_after(dispatch_time(0, static_cast<int64_t>(CAHostTimeBase::ConvertFromNanos(inNanoseconds))), mDispatchQueue, inTask);
	}
}

void	CADispatchQueue::Dispatch(bool inDoSync, void* inTaskContext, dispatch_function_t inTask) const
{
	if(inDoSync)
	{
				dispatch_sync_f(mDispatchQueue, inTaskContext, inTask);
	}
	else
	{
		dispatch_async_f(mDispatchQueue, inTaskContext, inTask);
	}
}

void	CADispatchQueue::Dispatch(UInt64 inNanoseconds, void* inTaskContext, dispatch_function_t inTask) const
{
	if(inNanoseconds == 0)
	{
		dispatch_async_f(mDispatchQueue, inTaskContext, inTask);
	}
	else
	{
		dispatch_after_f(dispatch_time(0, static_cast<int64_t>(CAHostTimeBase::ConvertFromNanos(inNanoseconds))), mDispatchQueue, inTaskContext, inTask);
	}
}

void	CADispatchQueue::Dispatch_Global(dispatch_queue_priority_t inQueuePriority, bool inDoSync, dispatch_block_t inTask)
{
	dispatch_queue_t theDispatchQueue = dispatch_get_global_queue(inQueuePriority, 0);
	if(inDoSync)
	{
				dispatch_sync(theDispatchQueue, inTask);
	}
	else
	{
		dispatch_async(theDispatchQueue, inTask);
	}
}

void	CADispatchQueue::Dispatch_Global(dispatch_queue_priority_t inQueuePriority, UInt64 inNanoseconds, dispatch_block_t inTask)
{
	dispatch_queue_t theDispatchQueue = dispatch_get_global_queue(inQueuePriority, 0);
	if(inNanoseconds == 0)
	{
		dispatch_async(theDispatchQueue, inTask);
	}
	else
	{
		dispatch_after(dispatch_time(0, static_cast<int64_t>(CAHostTimeBase::ConvertFromNanos(inNanoseconds))), theDispatchQueue, inTask);
	}
}

void	CADispatchQueue::Dispatch_Global(dispatch_queue_priority_t inQueuePriority, bool inDoSync, void* inTaskContext, dispatch_function_t inTask)
{
	dispatch_queue_t theDispatchQueue = dispatch_get_global_queue(inQueuePriority, 0);
	if(inDoSync)
	{
				dispatch_sync_f(theDispatchQueue, inTaskContext, inTask);
	}
	else
	{
		dispatch_async_f(theDispatchQueue, inTaskContext, inTask);
	}
}

void	CADispatchQueue::Dispatch_Global(dispatch_queue_priority_t inQueuePriority, UInt64 inNanoseconds, void* inTaskContext, dispatch_function_t inTask)
{
	dispatch_queue_t theDispatchQueue = dispatch_get_global_queue(inQueuePriority, 0);
	if(inNanoseconds == 0)
	{
		dispatch_async_f(theDispatchQueue, inTaskContext, inTask);
	}
	else
	{
		dispatch_after_f(dispatch_time(0, static_cast<int64_t>(CAHostTimeBase::ConvertFromNanos(inNanoseconds))), theDispatchQueue, inTaskContext, inTask);
	}
}

void	CADispatchQueue::Dispatch_Main(bool inDoSync, dispatch_block_t inTask)
{
	dispatch_queue_t theDispatchQueue = dispatch_get_main_queue();
	if(inDoSync)
	{
				dispatch_sync(theDispatchQueue, inTask);
	}
	else
	{
		dispatch_async(theDispatchQueue, inTask);
	}
}

void	CADispatchQueue::Dispatch_Main(UInt64 inNanoseconds, dispatch_block_t inTask)
{
	dispatch_queue_t theDispatchQueue = dispatch_get_main_queue();
	if(inNanoseconds == 0)
	{
		dispatch_async(theDispatchQueue, inTask);
	}
	else
	{
		dispatch_after(dispatch_time(0, static_cast<int64_t>(CAHostTimeBase::ConvertFromNanos(inNanoseconds))), theDispatchQueue, inTask);
	}
}

void	CADispatchQueue::Dispatch_Main(bool inDoSync, void* inTaskContext, dispatch_function_t inTask)
{
	dispatch_queue_t theDispatchQueue = dispatch_get_main_queue();
	if(inDoSync)
	{
				dispatch_sync_f(theDispatchQueue, inTaskContext, inTask);
	}
	else
	{
		dispatch_async_f(theDispatchQueue, inTaskContext, inTask);
	}
}

void	CADispatchQueue::Dispatch_Main(UInt64 inNanoseconds, void* inTaskContext, dispatch_function_t inTask)
{
	dispatch_queue_t theDispatchQueue = dispatch_get_main_queue();
	if(inNanoseconds == 0)
	{
		dispatch_async_f(theDispatchQueue, inTaskContext, inTask);
	}
	else
	{
		dispatch_after_f(dispatch_time(0, static_cast<int64_t>(CAHostTimeBase::ConvertFromNanos(inNanoseconds))), theDispatchQueue, inTaskContext, inTask);
	}
}

void	CADispatchQueue::InstallMachPortDeathNotification(mach_port_t inMachPort, dispatch_block_t inNotificationTask)
{
	ThrowIf(inMachPort == MACH_PORT_NULL, CAException('nope'), "CADispatchQueue::InstallMachPortDeathNotification: a mach port is required");
	
		bool wasFound = false;
	EventSourceList::iterator theIterator = mPortDeathList.begin();
	while(!wasFound && (theIterator != mPortDeathList.end()))
	{
		wasFound = theIterator->mMachPort == inMachPort;
		if(!wasFound)
		{
			++theIterator;
		}
	}
	
		if(!wasFound)
	{
				dispatch_source_t theDispatchSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_SEND, inMachPort, DISPATCH_MACH_SEND_DEAD, mDispatchQueue);
		ThrowIfNULL(theDispatchSource, CAException('what'), "CADispatchQueue::InstallMachPortDeathNotification: failed to create the mach port event source");
		
				dispatch_source_set_event_handler(theDispatchSource, inNotificationTask);
		
				mPortDeathList.push_back(EventSource(theDispatchSource, inMachPort));
		
				dispatch_resume(theDispatchSource);
	}
}

void	CADispatchQueue::RemoveMachPortDeathNotification(mach_port_t inMachPort)
{
	bool wasFound = false;
	EventSourceList::iterator theIterator = mPortDeathList.begin();
	while(!wasFound && (theIterator != mPortDeathList.end()))
	{
		wasFound = theIterator->mMachPort == inMachPort;
		if(!wasFound)
		{
			++theIterator;
		}
	}
	if(wasFound)
	{
		if(theIterator->mDispatchSource != NULL)
		{
			dispatch_source_cancel(theIterator->mDispatchSource);
		}
		mPortDeathList.erase(theIterator);
	}
}

void	CADispatchQueue::InstallMachPortReceiver(mach_port_t inMachPort, dispatch_block_t inMessageTask)
{
	ThrowIf(inMachPort == MACH_PORT_NULL, CAException('nope'), "CADispatchQueue::InstallMachPortReceiver: a mach port is required");
	
		bool wasFound = false;
	EventSourceList::iterator theIterator = mMachPortReceiverList.begin();
	while(!wasFound && (theIterator != mMachPortReceiverList.end()))
	{
		wasFound = theIterator->mMachPort == inMachPort;
		if(!wasFound)
		{
			++theIterator;
		}
	}
	
		if(!wasFound)
	{
				dispatch_source_t theDispatchSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, inMachPort, 0, mDispatchQueue);
		ThrowIfNULL(theDispatchSource, CAException('what'), "CADispatchQueue::InstallMachPortReceiver: failed to create the mach port event source");
		
				dispatch_source_set_event_handler(theDispatchSource, inMessageTask);
		
				mMachPortReceiverList.push_back(EventSource(theDispatchSource, inMachPort));
		
				dispatch_resume(theDispatchSource);
	}
}

void	CADispatchQueue::RemoveMachPortReceiver(mach_port_t inMachPort, dispatch_block_t inCompletionTask)
{
	bool wasFound = false;
	EventSourceList::iterator theIterator = mMachPortReceiverList.begin();
	while(!wasFound && (theIterator != mMachPortReceiverList.end()))
	{
		wasFound = theIterator->mMachPort == inMachPort;
		if(!wasFound)
		{
			++theIterator;
		}
	}
	if(wasFound)
	{
		if(theIterator->mDispatchSource != NULL)
		{
												if(inCompletionTask != 0)
			{
				dispatch_source_set_cancel_handler(theIterator->mDispatchSource, inCompletionTask);
			}
		
			dispatch_source_cancel(theIterator->mDispatchSource);
		}
		mMachPortReceiverList.erase(theIterator);
	}
}

void	CADispatchQueue::RemoveMachPortReceiver(mach_port_t inMachPort, bool inDestroySendRight, bool inDestroyReceiveRight)
{
	RemoveMachPortReceiver(inMachPort,	^{
											if(inDestroySendRight)
											{
												kern_return_t theError = mach_port_deallocate(mach_task_self(), inMachPort);
												AssertNoKernelError(theError, "CADispatchQueue::RemoveMachPortReceiver: deallocating the send right failed");
											}
											if(inDestroyReceiveRight)
											{
												kern_return_t theError = mach_port_mod_refs(mach_task_self(), inMachPort, MACH_PORT_RIGHT_RECEIVE, -1);
												AssertNoKernelError(theError, "CADispatchQueue::RemoveMachPortReceiver: deallocating the receive right failed");
											}
										});
}

CADispatchQueue&	CADispatchQueue::GetGlobalSerialQueue()
{
	dispatch_once_f(&sGlobalSerialQueueInitialized, NULL, InitializeGlobalSerialQueue);
	ThrowIfNULL(sGlobalSerialQueue, CAException('nope'), "CADispatchQueue::GetGlobalSerialQueue: there is no global serial queue");
	return *sGlobalSerialQueue;
}

void	CADispatchQueue::InitializeGlobalSerialQueue(void*)
{
	try
	{
		sGlobalSerialQueue = new CADispatchQueue("com.apple.audio.CADispatchQueue.SerialQueue");
	}
	catch(...)
	{
		sGlobalSerialQueue = NULL;
	}
}

CADispatchQueue*	CADispatchQueue::sGlobalSerialQueue = NULL;
dispatch_once_t		CADispatchQueue::sGlobalSerialQueueInitialized = 0;
