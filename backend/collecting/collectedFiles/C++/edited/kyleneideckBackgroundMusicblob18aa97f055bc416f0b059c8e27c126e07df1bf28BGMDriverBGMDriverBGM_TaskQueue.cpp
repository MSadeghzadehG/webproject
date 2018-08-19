

#include "BGM_TaskQueue.h"

#include "BGM_Types.h"
#include "BGM_Utils.h"
#include "BGM_PlugIn.h"
#include "BGM_Clients.h"
#include "BGM_ClientMap.h"
#include "BGM_ClientTasks.h"

#include "CAException.h"
#include "CADebugMacros.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#include "CAAtomic.h"
#pragma clang diagnostic pop

#include <mach/mach_init.h>
#include <mach/mach_time.h>
#include <mach/task.h>


#pragma clang assume_nonnull begin

#pragma mark Construction/destruction

BGM_TaskQueue::BGM_TaskQueue()
:
                    mRealTimeThread(&BGM_TaskQueue::RealTimeThreadProc,
                    this,
                     0,
                    NanosToAbsoluteTime(kRealTimeThreadNominalComputationNs),
                    NanosToAbsoluteTime(kRealTimeThreadMaximumComputationNs),
                     true),
    mNonRealTimeThread(&BGM_TaskQueue::NonRealTimeThreadProc, this)
{
        auto createSemaphore = [] () {
        semaphore_t theSemaphore;
        kern_return_t theError = semaphore_create(mach_task_self(), &theSemaphore, SYNC_POLICY_FIFO, 0);
        
        BGM_Utils::ThrowIfMachError("BGM_TaskQueue::BGM_TaskQueue", "semaphore_create", theError);
        
        ThrowIf(theSemaphore == SEMAPHORE_NULL,
                CAException(kAudioHardwareUnspecifiedError),
                "BGM_TaskQueue::BGM_TaskQueue: Could not create semaphore");
        
        return theSemaphore;
    };
    
    mRealTimeThreadWorkQueuedSemaphore = createSemaphore();
    mNonRealTimeThreadWorkQueuedSemaphore = createSemaphore();
    mRealTimeThreadSyncTaskCompletedSemaphore = createSemaphore();
    mNonRealTimeThreadSyncTaskCompletedSemaphore = createSemaphore();
    
            for(UInt32 i = 0; i < kNonRealTimeThreadTaskBufferSize; i++)
    {
        BGM_Task* theTask = new BGM_Task;
        mNonRealTimeThreadTasksFreeList.push_NA(theTask);
    }
    
        mRealTimeThread.Start();
    mNonRealTimeThread.Start();
}

BGM_TaskQueue::~BGM_TaskQueue()
{
        BGMLogAndSwallowExceptionsMsg("BGM_TaskQueue::~BGM_TaskQueue", "QueueSync", ([&] {
        QueueSync(kBGMTaskStopWorkerThread,  true);
        QueueSync(kBGMTaskStopWorkerThread,  false);
    }));

        auto destroySemaphore = [] (semaphore_t inSemaphore) {
        kern_return_t theError = semaphore_destroy(mach_task_self(), inSemaphore);
        
        BGM_Utils::LogIfMachError("BGM_TaskQueue::~BGM_TaskQueue", "semaphore_destroy", theError);
    };
    
    destroySemaphore(mRealTimeThreadWorkQueuedSemaphore);
    destroySemaphore(mNonRealTimeThreadWorkQueuedSemaphore);
    destroySemaphore(mRealTimeThreadSyncTaskCompletedSemaphore);
    destroySemaphore(mNonRealTimeThreadSyncTaskCompletedSemaphore);
    
    BGM_Task* theTask;
    
        while((theTask = mNonRealTimeThreadTasksFreeList.pop_atomic()) != NULL)
    {
        delete theTask;
    }
    
        while((theTask = mNonRealTimeThreadTasks.pop_atomic()) != NULL)
    {
        if(!theTask->IsSync())
        {
            delete theTask;
        }
    }
}

UInt32  BGM_TaskQueue::NanosToAbsoluteTime(UInt32 inNanos)
{
            
    mach_timebase_info_data_t theTimebaseInfo;
    mach_timebase_info(&theTimebaseInfo);
    
    Float64 theTicksPerNs = static_cast<Float64>(theTimebaseInfo.denom) / theTimebaseInfo.numer;
    return static_cast<UInt32>(inNanos * theTicksPerNs);
}

#pragma mark Task queueing

void    BGM_TaskQueue::QueueSync_SwapClientShadowMaps(BGM_ClientMap* inClientMap)
{
                            QueueSync(kBGMTaskSwapClientShadowMaps,  true, reinterpret_cast<UInt64>(inClientMap));
}

void    BGM_TaskQueue::QueueAsync_SendPropertyNotification(AudioObjectPropertySelector inProperty, AudioObjectID inDeviceID)
{
    DebugMsg("BGM_TaskQueue::QueueAsync_SendPropertyNotification: Queueing property notification. inProperty=%u inDeviceID=%u",
             inProperty,
             inDeviceID);
    BGM_Task theTask(kBGMTaskSendPropertyNotification,  false, inProperty, inDeviceID);
    QueueOnNonRealtimeThread(theTask);
}

bool    BGM_TaskQueue::Queue_UpdateClientIOState(bool inSync, BGM_Clients* inClients, UInt32 inClientID, bool inDoingIO)
{
    DebugMsg("BGM_TaskQueue::Queue_UpdateClientIOState: Queueing %s %s",
             (inDoingIO ? "kBGMTaskStartClientIO" : "kBGMTaskStopClientIO"),
             (inSync ? "synchronously" : "asynchronously"));
    
    BGM_TaskID theTaskID = (inDoingIO ? kBGMTaskStartClientIO : kBGMTaskStopClientIO);
    UInt64 theClientsPtrArg = reinterpret_cast<UInt64>(inClients);
    UInt64 theClientIDTaskArg = static_cast<UInt64>(inClientID);
    
    if(inSync)
    {
        return QueueSync(theTaskID, false, theClientsPtrArg, theClientIDTaskArg);
    }
    else
    {
        BGM_Task theTask(theTaskID,  false, theClientsPtrArg, theClientIDTaskArg);
        QueueOnNonRealtimeThread(theTask);
        
                return false;
    }
}

UInt64    BGM_TaskQueue::QueueSync(BGM_TaskID inTaskID, bool inRunOnRealtimeThread, UInt64 inTaskArg1, UInt64 inTaskArg2)
{
    DebugMsg("BGM_TaskQueue::QueueSync: Queueing task synchronously to be processed on the %s thread. inTaskID=%d inTaskArg1=%llu inTaskArg2=%llu",
             (inRunOnRealtimeThread ? "realtime" : "non-realtime"),
             inTaskID,
             inTaskArg1,
             inTaskArg2);
    
        BGM_Task theTask(inTaskID,  true, inTaskArg1, inTaskArg2);
    
        TAtomicStack<BGM_Task>& theTasks = (inRunOnRealtimeThread ? mRealTimeThreadTasks : mNonRealTimeThreadTasks);
    theTasks.push_atomic(&theTask);
    
        kern_return_t theError = semaphore_signal(inRunOnRealtimeThread ? mRealTimeThreadWorkQueuedSemaphore : mNonRealTimeThreadWorkQueuedSemaphore);
    BGM_Utils::ThrowIfMachError("BGM_TaskQueue::QueueSync", "semaphore_signal", theError);
    
                    bool didLogTimeoutMessage = false;
    while(!theTask.IsComplete())
    {
        semaphore_t theTaskCompletedSemaphore =
            inRunOnRealtimeThread ? mRealTimeThreadSyncTaskCompletedSemaphore : mNonRealTimeThreadSyncTaskCompletedSemaphore;
                        theError = semaphore_timedwait(theTaskCompletedSemaphore,
                                       (mach_timespec_t){ 0, kRealTimeThreadMaximumComputationNs * 4 });
        
        if(theError == KERN_OPERATION_TIMED_OUT)
        {
            if(!didLogTimeoutMessage && inRunOnRealtimeThread)
            {
                DebugMsg("BGM_TaskQueue::QueueSync: Task %d taking longer than expected.", theTask.GetTaskID());
                didLogTimeoutMessage = true;
            }
        }
        else
        {
            BGM_Utils::ThrowIfMachError("BGM_TaskQueue::QueueSync", "semaphore_timedwait", theError);
        }
        
        CAMemoryBarrier();
    }
    
    if(didLogTimeoutMessage)
    {
        DebugMsg("BGM_TaskQueue::QueueSync: Late task %d finished.", theTask.GetTaskID());
    }
    
    if(theTask.GetReturnValue() != INT64_MAX)
    {
        DebugMsg("BGM_TaskQueue::QueueSync: Task %d returned %llu.", theTask.GetTaskID(), theTask.GetReturnValue());
    }
    
    return theTask.GetReturnValue();
}

void   BGM_TaskQueue::QueueOnNonRealtimeThread(BGM_Task inTask)
{
        BGM_Task* freeTask = mNonRealTimeThreadTasksFreeList.pop_atomic();
    
    if(freeTask == NULL)
    {
        LogWarning("BGM_TaskQueue::QueueOnNonRealtimeThread: No pre-allocated tasks left in the free list. Allocating new task.");
        freeTask = new BGM_Task;
    }
    
    *freeTask = inTask;
    
    mNonRealTimeThreadTasks.push_atomic(freeTask);
    
        kern_return_t theError = semaphore_signal(mNonRealTimeThreadWorkQueuedSemaphore);
    BGM_Utils::ThrowIfMachError("BGM_TaskQueue::QueueOnNonRealtimeThread", "semaphore_signal", theError);
}

#pragma mark Worker threads

void    BGM_TaskQueue::AssertCurrentThreadIsRTWorkerThread(const char* inCallerMethodName)
{
#if DEBUG      if(!mRealTimeThread.IsCurrentThread())
    {
        DebugMsg("%s should only be called on the realtime worker thread.", inCallerMethodName);
        __ASSERT_STOP;      }
    
    Assert(mRealTimeThread.IsTimeConstraintThread(), "mRealTimeThread should be in a time-constraint priority band.");
#else
    #pragma unused (inCallerMethodName)
#endif
}

void* __nullable    BGM_TaskQueue::RealTimeThreadProc(void* inRefCon)
{
    DebugMsg("BGM_TaskQueue::RealTimeThreadProc: The realtime worker thread has started");
    
    BGM_TaskQueue* refCon = static_cast<BGM_TaskQueue*>(inRefCon);
    refCon->WorkerThreadProc(refCon->mRealTimeThreadWorkQueuedSemaphore,
                             refCon->mRealTimeThreadSyncTaskCompletedSemaphore,
                             &refCon->mRealTimeThreadTasks,
                             NULL,
                             [&] (BGM_Task* inTask) { return refCon->ProcessRealTimeThreadTask(inTask); });
    
    return NULL;
}

void* __nullable    BGM_TaskQueue::NonRealTimeThreadProc(void* inRefCon)
{
    DebugMsg("BGM_TaskQueue::NonRealTimeThreadProc: The non-realtime worker thread has started");
    
    BGM_TaskQueue* refCon = static_cast<BGM_TaskQueue*>(inRefCon);
    refCon->WorkerThreadProc(refCon->mNonRealTimeThreadWorkQueuedSemaphore,
                             refCon->mNonRealTimeThreadSyncTaskCompletedSemaphore,
                             &refCon->mNonRealTimeThreadTasks,
                             &refCon->mNonRealTimeThreadTasksFreeList,
                             [&] (BGM_Task* inTask) { return refCon->ProcessNonRealTimeThreadTask(inTask); });
    
    return NULL;
}

void    BGM_TaskQueue::WorkerThreadProc(semaphore_t inWorkQueuedSemaphore, semaphore_t inSyncTaskCompletedSemaphore, TAtomicStack<BGM_Task>* inTasks, TAtomicStack2<BGM_Task>* __nullable inFreeList, std::function<bool(BGM_Task*)> inProcessTask)
{
    bool theThreadShouldStop = false;
    
    while(!theThreadShouldStop)
    {
                                        kern_return_t theError = semaphore_wait(inWorkQueuedSemaphore);
        BGM_Utils::ThrowIfMachError("BGM_TaskQueue::WorkerThreadProc", "semaphore_wait", theError);
        
                                        BGM_Task* theTask = inTasks->pop_all_reversed();
        
        while(theTask != NULL &&
              !theThreadShouldStop)          {
            BGM_Task* theNextTask = theTask->mNext;
            
            BGMAssert(!theTask->IsComplete(),
                      "BGM_TaskQueue::WorkerThreadProc: Cannot process already completed task (ID %d)",
                      theTask->GetTaskID());
            
            BGMAssert(theTask != theNextTask,
                      "BGM_TaskQueue::WorkerThreadProc: BGM_Task %p (ID %d) was added to %s multiple times. arg1=%llu arg2=%llu",
                      theTask,
                      theTask->GetTaskID(),
                      (inTasks == &mRealTimeThreadTasks ? "mRealTimeThreadTasks" : "mNonRealTimeThreadTasks"),
                      theTask->GetArg1(),
                      theTask->GetArg2());
            
                        theThreadShouldStop = inProcessTask(theTask);
            
                        if(theTask->IsSync())
            {
                                                CAMemoryBarrier();
                theTask->MarkCompleted();
                
                                                                                                                                theError = semaphore_signal_all(inSyncTaskCompletedSemaphore);
                BGM_Utils::ThrowIfMachError("BGM_TaskQueue::WorkerThreadProc", "semaphore_signal_all", theError);
            }
            else if(inFreeList != NULL)
            {
                                inFreeList->push_atomic(theTask);
            }
            
            theTask = theNextTask;
        }
    }
}

bool    BGM_TaskQueue::ProcessRealTimeThreadTask(BGM_Task* inTask)
{
    AssertCurrentThreadIsRTWorkerThread("BGM_TaskQueue::ProcessRealTimeThreadTask");
    
    switch(inTask->GetTaskID())
    {
        case kBGMTaskStopWorkerThread:
            DebugMsg("BGM_TaskQueue::ProcessRealTimeThreadTask: Stopping");
                        return true;
            
        case kBGMTaskSwapClientShadowMaps:
            {
                DebugMsg("BGM_TaskQueue::ProcessRealTimeThreadTask: Swapping the shadow maps in BGM_ClientMap");
                BGM_ClientMap* theClientMap = reinterpret_cast<BGM_ClientMap*>(inTask->GetArg1());
                BGM_ClientTasks::SwapInShadowMapsRT(theClientMap);
            }
            break;
            
        default:
            Assert(false, "BGM_TaskQueue::ProcessRealTimeThreadTask: Unexpected task ID");
            break;
    }
    
    return false;
}

bool    BGM_TaskQueue::ProcessNonRealTimeThreadTask(BGM_Task* inTask)
{
#if DEBUG      Assert(mNonRealTimeThread.IsCurrentThread(), "ProcessNonRealTimeThreadTask should only be called on the non-realtime worker thread.");
    Assert(mNonRealTimeThread.IsTimeShareThread(), "mNonRealTimeThread should not be in a time-constraint priority band.");
#endif
    
    switch(inTask->GetTaskID())
    {
        case kBGMTaskStopWorkerThread:
            DebugMsg("BGM_TaskQueue::ProcessNonRealTimeThreadTask: Stopping");
                        return true;
            
        case kBGMTaskStartClientIO:
            DebugMsg("BGM_TaskQueue::ProcessNonRealTimeThreadTask: Processing kBGMTaskStartClientIO");
            try
            {
                BGM_Clients* theClients = reinterpret_cast<BGM_Clients*>(inTask->GetArg1());
                bool didStartIO = BGM_ClientTasks::StartIONonRT(theClients, static_cast<UInt32>(inTask->GetArg2()));
                inTask->SetReturnValue(didStartIO);
            }
                                                                                                catch(BGM_InvalidClientException)
            {
                DebugMsg("BGM_TaskQueue::ProcessNonRealTimeThreadTask: Ignoring BGM_InvalidClientException thrown by StartIONonRT. %s",
                         "It's possible the client was removed before this task was processed.");
            }
            break;

        case kBGMTaskStopClientIO:
            DebugMsg("BGM_TaskQueue::ProcessNonRealTimeThreadTask: Processing kBGMTaskStopClientIO");
            try
            {
                BGM_Clients* theClients = reinterpret_cast<BGM_Clients*>(inTask->GetArg1());
                bool didStopIO = BGM_ClientTasks::StopIONonRT(theClients, static_cast<UInt32>(inTask->GetArg2()));
                inTask->SetReturnValue(didStopIO);
            }
            catch(BGM_InvalidClientException)
            {
                DebugMsg("BGM_TaskQueue::ProcessNonRealTimeThreadTask: Ignoring BGM_InvalidClientException thrown by StopIONonRT. %s",
                         "It's possible the client was removed before this task was processed.");
            }
            break;
            
        case kBGMTaskSendPropertyNotification:
            DebugMsg("BGM_TaskQueue::ProcessNonRealTimeThreadTask: Processing kBGMTaskSendPropertyNotification");
            {
                AudioObjectPropertyAddress thePropertyAddress[] = {
                    { static_cast<UInt32>(inTask->GetArg1()), kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster } };
                BGM_PlugIn::Host_PropertiesChanged(static_cast<AudioObjectID>(inTask->GetArg2()), 1, thePropertyAddress);
            }
            break;
            
        default:
            Assert(false, "BGM_TaskQueue::ProcessNonRealTimeThreadTask: Unexpected task ID");
            break;
    }
    
    return false;
}

#pragma clang assume_nonnull end

