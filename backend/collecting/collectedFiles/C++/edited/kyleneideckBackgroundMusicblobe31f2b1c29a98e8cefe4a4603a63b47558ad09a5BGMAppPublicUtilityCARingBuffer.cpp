
#include "CARingBuffer.h"
#include "CABitOperations.h"
#include "CAAutoDisposer.h"
#include "CAAtomic.h"

#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <libkern/OSAtomic.h>

CARingBuffer::CARingBuffer() :
	mBuffers(NULL), mNumberChannels(0), mCapacityFrames(0), mCapacityBytes(0)
{

}

CARingBuffer::~CARingBuffer()
{
	Deallocate();
}


void	CARingBuffer::Allocate(int nChannels, UInt32 bytesPerFrame, UInt32 capacityFrames)
{
	Deallocate();
	
	capacityFrames = NextPowerOfTwo(capacityFrames);
	
	mNumberChannels = nChannels;
	mBytesPerFrame = bytesPerFrame;
	mCapacityFrames = capacityFrames;
	mCapacityFramesMask = capacityFrames - 1;
	mCapacityBytes = bytesPerFrame * capacityFrames;

		UInt32 allocSize = (mCapacityBytes + sizeof(Byte *)) * nChannels;
	Byte *p = (Byte *)CA_malloc(allocSize);
	memset(p, 0, allocSize);
	mBuffers = (Byte **)p;
	p += nChannels * sizeof(Byte *);
	for (int i = 0; i < nChannels; ++i) {
		mBuffers[i] = p;
		p += mCapacityBytes;
	}
	
	for (UInt32 i = 0; i<kGeneralRingTimeBoundsQueueSize; ++i)
	{
		mTimeBoundsQueue[i].mStartTime = 0;
		mTimeBoundsQueue[i].mEndTime = 0;
		mTimeBoundsQueue[i].mUpdateCounter = 0;
	}
	mTimeBoundsQueuePtr = 0;
}

void	CARingBuffer::Deallocate()
{
	if (mBuffers) {
		free(mBuffers);
		mBuffers = NULL;
	}
	mNumberChannels = 0;
	mCapacityBytes = 0;
	mCapacityFrames = 0;
}

inline void ZeroRange(Byte **buffers, int nchannels, int offset, int nbytes)
{
	while (--nchannels >= 0) {
		memset(*buffers + offset, 0, nbytes);
		++buffers;
	}
}

inline void StoreABL(Byte **buffers, int destOffset, const AudioBufferList *abl, int srcOffset, int nbytes)
{
	int nchannels = abl->mNumberBuffers;
	const AudioBuffer *src = abl->mBuffers;
	while (--nchannels >= 0) {
		if (srcOffset > (int)src->mDataByteSize) continue;
		memcpy(*buffers + destOffset, (Byte *)src->mData + srcOffset, std::min(nbytes, (int)src->mDataByteSize - srcOffset));
		++buffers;
		++src;
	}
}

inline void FetchABL(AudioBufferList *abl, int destOffset, Byte **buffers, int srcOffset, int nbytes)
{
	int nchannels = abl->mNumberBuffers;
	AudioBuffer *dest = abl->mBuffers;
	while (--nchannels >= 0) {
		if (destOffset > (int)dest->mDataByteSize) continue;
		memcpy((Byte *)dest->mData + destOffset, *buffers + srcOffset, std::min(nbytes, (int)dest->mDataByteSize - destOffset));
		++buffers;
		++dest;
	}
}

inline void ZeroABL(AudioBufferList *abl, int destOffset, int nbytes)
{
	int nBuffers = abl->mNumberBuffers;
	AudioBuffer *dest = abl->mBuffers;
	while (--nBuffers >= 0) {
		if (destOffset > (int)dest->mDataByteSize) continue;
		memset((Byte *)dest->mData + destOffset, 0, std::min(nbytes, (int)dest->mDataByteSize - destOffset));
		++dest;
	}
}


CARingBufferError	CARingBuffer::Store(const AudioBufferList *abl, UInt32 framesToWrite, SampleTime startWrite)
{
	if (framesToWrite == 0)
		return kCARingBufferError_OK;
	
	if (framesToWrite > mCapacityFrames)
		return kCARingBufferError_TooMuch;		
	SampleTime endWrite = startWrite + framesToWrite;
	
	if (startWrite < EndTime()) {
				SetTimeBounds(startWrite, startWrite);
	} else if (endWrite - StartTime() <= mCapacityFrames) {
			} else {
				SampleTime newStart = endWrite - mCapacityFrames;			SampleTime newEnd = std::max(newStart, EndTime());
		SetTimeBounds(newStart, newEnd);
	}
	
		Byte **buffers = mBuffers;
	int nchannels = mNumberChannels;
	int offset0, offset1, nbytes;
	SampleTime curEnd = EndTime();
	
	if (startWrite > curEnd) {
				offset0 = FrameOffset(curEnd);
		offset1 = FrameOffset(startWrite);
		if (offset0 < offset1)
			ZeroRange(buffers, nchannels, offset0, offset1 - offset0);
		else {
			ZeroRange(buffers, nchannels, offset0, mCapacityBytes - offset0);
			ZeroRange(buffers, nchannels, 0, offset1);
		}
		offset0 = offset1;
	} else {
		offset0 = FrameOffset(startWrite);
	}

	offset1 = FrameOffset(endWrite);
	if (offset0 < offset1)
		StoreABL(buffers, offset0, abl, 0, offset1 - offset0);
	else {
		nbytes = mCapacityBytes - offset0;
		StoreABL(buffers, offset0, abl, 0, nbytes);
		StoreABL(buffers, 0, abl, nbytes, offset1);
	}
	
		SetTimeBounds(StartTime(), endWrite);
	
	return kCARingBufferError_OK;	}

void	CARingBuffer::SetTimeBounds(SampleTime startTime, SampleTime endTime)
{
	UInt32 nextPtr = mTimeBoundsQueuePtr + 1;
	UInt32 index = nextPtr & kGeneralRingTimeBoundsQueueMask;
	
	mTimeBoundsQueue[index].mStartTime = startTime;
	mTimeBoundsQueue[index].mEndTime = endTime;
	mTimeBoundsQueue[index].mUpdateCounter = nextPtr;
	CAAtomicCompareAndSwap32Barrier(mTimeBoundsQueuePtr, mTimeBoundsQueuePtr + 1, (SInt32*)&mTimeBoundsQueuePtr);
}

CARingBufferError	CARingBuffer::GetTimeBounds(SampleTime &startTime, SampleTime &endTime)
{
	for (int i=0; i<8; ++i) 	{
		UInt32 curPtr = mTimeBoundsQueuePtr;
		UInt32 index = curPtr & kGeneralRingTimeBoundsQueueMask;
		CARingBuffer::TimeBounds* bounds = mTimeBoundsQueue + index;
		
		startTime = bounds->mStartTime;
		endTime = bounds->mEndTime;
		UInt32 newPtr = bounds->mUpdateCounter;
		
		if (newPtr == curPtr) 
			return kCARingBufferError_OK;
	}
	return kCARingBufferError_CPUOverload;
}

CARingBufferError	CARingBuffer::ClipTimeBounds(SampleTime& startRead, SampleTime& endRead)
{
	SampleTime startTime, endTime;
	
	CARingBufferError err = GetTimeBounds(startTime, endTime);
	if (err) return err;
	
	if (startRead > endTime || endRead < startTime) {
		endRead = startRead;
		return kCARingBufferError_OK;
	}
	
	startRead = std::max(startRead, startTime);
	endRead = std::min(endRead, endTime);
	endRead = std::max(endRead, startRead);
		
	return kCARingBufferError_OK;	}

CARingBufferError	CARingBuffer::Fetch(AudioBufferList *abl, UInt32 nFrames, SampleTime startRead)
{
	if (nFrames == 0)
		return kCARingBufferError_OK;
		
	startRead = std::max(0LL, startRead);
	
	SampleTime endRead = startRead + nFrames;

	SampleTime startRead0 = startRead;
	SampleTime endRead0 = endRead;

	CARingBufferError err = ClipTimeBounds(startRead, endRead);
	if (err) return err;

	if (startRead == endRead) {
		ZeroABL(abl, 0, nFrames * mBytesPerFrame);
		return kCARingBufferError_OK;
	}
	
	SInt32 byteSize = (SInt32)((endRead - startRead) * mBytesPerFrame);
	
	SInt32 destStartByteOffset = std::max((SInt32)0, (SInt32)((startRead - startRead0) * mBytesPerFrame)); 
		
	if (destStartByteOffset > 0) {
		ZeroABL(abl, 0, std::min((SInt32)(nFrames * mBytesPerFrame), destStartByteOffset));
	}

	SInt32 destEndSize = std::max((SInt32)0, (SInt32)(endRead0 - endRead)); 
	if (destEndSize > 0) {
		ZeroABL(abl, destStartByteOffset + byteSize, destEndSize * mBytesPerFrame);
	}
	
	Byte **buffers = mBuffers;
	int offset0 = FrameOffset(startRead);
	int offset1 = FrameOffset(endRead);
	int nbytes;
	
	if (offset0 < offset1) {
		nbytes = offset1 - offset0;
		FetchABL(abl, destStartByteOffset, buffers, offset0, nbytes);
	} else {
		nbytes = mCapacityBytes - offset0;
		FetchABL(abl, destStartByteOffset, buffers, offset0, nbytes);
		FetchABL(abl, destStartByteOffset + nbytes, buffers, 0, offset1);
		nbytes += offset1;
	}

	int nchannels = abl->mNumberBuffers;
	AudioBuffer *dest = abl->mBuffers;
	while (--nchannels >= 0)
	{
		dest->mDataByteSize = nbytes;
		dest++;
	}

	return noErr;
}
