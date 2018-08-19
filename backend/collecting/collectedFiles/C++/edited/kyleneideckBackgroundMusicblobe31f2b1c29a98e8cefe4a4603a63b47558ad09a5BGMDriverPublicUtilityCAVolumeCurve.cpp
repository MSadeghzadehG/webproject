

#include "CAVolumeCurve.h"
#include "CADebugMacros.h"
#include <math.h>


CAVolumeCurve::CAVolumeCurve()
:
	mTag(0),
	mCurveMap(),
	mIsApplyingTransferFunction(true),
	mTransferFunction(kPow2Over1Curve),
	mRawToScalarExponentNumerator(2.0f),
	mRawToScalarExponentDenominator(1.0f)
{
}

CAVolumeCurve::~CAVolumeCurve()
{
}

SInt32	CAVolumeCurve::GetMinimumRaw() const
{
	SInt32 theAnswer = 0;
	
	if(!mCurveMap.empty())
	{
		CurveMap::const_iterator theIterator = mCurveMap.begin();
		theAnswer = theIterator->first.mMinimum;
	}
	
	return theAnswer;
}

SInt32	CAVolumeCurve::GetMaximumRaw() const
{
	SInt32 theAnswer = 0;
	
	if(!mCurveMap.empty())
	{
		CurveMap::const_iterator theIterator = mCurveMap.begin();
		std::advance(theIterator, static_cast<int>(mCurveMap.size() - 1));
		theAnswer = theIterator->first.mMaximum;
	}
	
	return theAnswer;
}

Float32	CAVolumeCurve::GetMinimumDB() const
{
	Float32 theAnswer = 0;
	
	if(!mCurveMap.empty())
	{
		CurveMap::const_iterator theIterator = mCurveMap.begin();
		theAnswer = theIterator->second.mMinimum;
	}
	
	return theAnswer;
}

Float32	CAVolumeCurve::GetMaximumDB() const
{
	Float32 theAnswer = 0;
	
	if(!mCurveMap.empty())
	{
		CurveMap::const_iterator theIterator = mCurveMap.begin();
		std::advance(theIterator, static_cast<int>(mCurveMap.size() - 1));
		theAnswer = theIterator->second.mMaximum;
	}
	
	return theAnswer;
}

void	CAVolumeCurve::SetTransferFunction(UInt32 inTransferFunction)
{
	mTransferFunction = inTransferFunction;
	
		switch(inTransferFunction)
	{
		case kLinearCurve:
			mIsApplyingTransferFunction = false;
			mRawToScalarExponentNumerator = 1.0f;
			mRawToScalarExponentDenominator = 1.0f;
			break;
			
		case kPow1Over3Curve:
			mIsApplyingTransferFunction = true;
			mRawToScalarExponentNumerator = 1.0f;
			mRawToScalarExponentDenominator = 3.0f;
			break;
			
		case kPow1Over2Curve:
			mIsApplyingTransferFunction = true;
			mRawToScalarExponentNumerator = 1.0f;
			mRawToScalarExponentDenominator = 2.0f;
			break;
			
		case kPow3Over4Curve:
			mIsApplyingTransferFunction = true;
			mRawToScalarExponentNumerator = 3.0f;
			mRawToScalarExponentDenominator = 4.0f;
			break;
			
		case kPow3Over2Curve:
			mIsApplyingTransferFunction = true;
			mRawToScalarExponentNumerator = 3.0f;
			mRawToScalarExponentDenominator = 2.0f;
			break;
			
		case kPow2Over1Curve:
			mIsApplyingTransferFunction = true;
			mRawToScalarExponentNumerator = 2.0f;
			mRawToScalarExponentDenominator = 1.0f;
			break;
			
		case kPow3Over1Curve:
			mIsApplyingTransferFunction = true;
			mRawToScalarExponentNumerator = 3.0f;
			mRawToScalarExponentDenominator = 1.0f;
			break;
		
		case kPow4Over1Curve:
			mIsApplyingTransferFunction = true;
			mRawToScalarExponentNumerator = 4.0f;
			mRawToScalarExponentDenominator = 1.0f;
			break;
		
		case kPow5Over1Curve:
			mIsApplyingTransferFunction = true;
			mRawToScalarExponentNumerator = 5.0f;
			mRawToScalarExponentDenominator = 1.0f;
			break;
		
		case kPow6Over1Curve:
			mIsApplyingTransferFunction = true;
			mRawToScalarExponentNumerator = 6.0f;
			mRawToScalarExponentDenominator = 1.0f;
			break;
		
		case kPow7Over1Curve:
			mIsApplyingTransferFunction = true;
			mRawToScalarExponentNumerator = 7.0f;
			mRawToScalarExponentDenominator = 1.0f;
			break;
		
		case kPow8Over1Curve:
			mIsApplyingTransferFunction = true;
			mRawToScalarExponentNumerator = 8.0f;
			mRawToScalarExponentDenominator = 1.0f;
			break;
		
		case kPow9Over1Curve:
			mIsApplyingTransferFunction = true;
			mRawToScalarExponentNumerator = 9.0f;
			mRawToScalarExponentDenominator = 1.0f;
			break;
		
		case kPow10Over1Curve:
			mIsApplyingTransferFunction = true;
			mRawToScalarExponentNumerator = 10.0f;
			mRawToScalarExponentDenominator = 1.0f;
			break;
		
		case kPow11Over1Curve:
			mIsApplyingTransferFunction = true;
			mRawToScalarExponentNumerator = 11.0f;
			mRawToScalarExponentDenominator = 1.0f;
			break;
		
		case kPow12Over1Curve:
			mIsApplyingTransferFunction = true;
			mRawToScalarExponentNumerator = 12.0f;
			mRawToScalarExponentDenominator = 1.0f;
			break;
		
		default:
			mIsApplyingTransferFunction = true;
			mRawToScalarExponentNumerator = 2.0f;
			mRawToScalarExponentDenominator = 1.0f;
			break;
	};
}

void	CAVolumeCurve::AddRange(SInt32 inMinRaw, SInt32 inMaxRaw, Float32 inMinDB, Float32 inMaxDB)
{
	CARawPoint theRaw(inMinRaw, inMaxRaw);
	CADBPoint theDB(inMinDB, inMaxDB);
	
	bool isOverlapped = false;
	bool isDone = false;
	CurveMap::iterator theIterator = mCurveMap.begin();
	while((theIterator != mCurveMap.end()) && !isOverlapped && !isDone)
	{
		isOverlapped = CARawPoint::Overlap(theRaw, theIterator->first);
		isDone = theRaw >= theIterator->first;
		
		if(!isOverlapped && !isDone)
		{
			std::advance(theIterator, 1);
		}
	}
	
	if(!isOverlapped)
	{
		mCurveMap.insert(CurveMap::value_type(theRaw, theDB));
	}
	else
	{
		DebugMessage("CAVolumeCurve::AddRange: new point overlaps");
	}
}

void	CAVolumeCurve::ResetRange()
{
	mCurveMap.clear();
}

bool	CAVolumeCurve::CheckForContinuity() const
{
	bool theAnswer = true;
	
	CurveMap::const_iterator theIterator = mCurveMap.begin();
	if(theIterator != mCurveMap.end())
	{
		SInt32 theRaw = theIterator->first.mMinimum;
		Float32 theDB = theIterator->second.mMinimum;
		do
		{
			SInt32 theRawMin = theIterator->first.mMinimum;
			SInt32 theRawMax = theIterator->first.mMaximum;
			SInt32 theRawRange = theRawMax - theRawMin;
			
			Float32 theDBMin = theIterator->second.mMinimum;
			Float32 theDBMax = theIterator->second.mMaximum;
			Float32 theDBRange = theDBMax - theDBMin;

			theAnswer = theRaw == theRawMin;
			theAnswer = theAnswer && (theDB == theDBMin);
			
			theRaw += theRawRange;
			theDB += theDBRange;
			
			std::advance(theIterator, 1);
		}
		while((theIterator != mCurveMap.end()) && theAnswer);
	}
	
	return theAnswer;
}

SInt32	CAVolumeCurve::ConvertDBToRaw(Float32 inDB) const
{
		Float32 theOverallDBMin = GetMinimumDB();
	Float32 theOverallDBMax = GetMaximumDB();
	
	if(inDB < theOverallDBMin) inDB = theOverallDBMin;
	if(inDB > theOverallDBMax) inDB = theOverallDBMax;
	
		CurveMap::const_iterator theIterator = mCurveMap.begin();
	
		SInt32 theAnswer = theIterator->first.mMinimum;
	
		bool isDone = false;
	while(!isDone && (theIterator != mCurveMap.end()))
	{
		SInt32 theRawMin = theIterator->first.mMinimum;
		SInt32 theRawMax = theIterator->first.mMaximum;
		SInt32 theRawRange = theRawMax - theRawMin;
		
		Float32 theDBMin = theIterator->second.mMinimum;
		Float32 theDBMax = theIterator->second.mMaximum;
		Float32 theDBRange = theDBMax - theDBMin;
		
		Float32 theDBPerRaw = theDBRange / static_cast<Float32>(theRawRange);
		
				if(inDB > theDBMax)
		{
						theAnswer += theRawRange;
		}
		else
		{
									Float32 theNumberRawSteps = inDB - theDBMin;
			theNumberRawSteps /= theDBPerRaw;
			
						theNumberRawSteps = roundf(theNumberRawSteps);
			
						theAnswer += static_cast<SInt32>(theNumberRawSteps);
			
						isDone = true;
		}
		
				std::advance(theIterator, 1);
	}
	
	return theAnswer;
}

Float32	CAVolumeCurve::ConvertRawToDB(SInt32 inRaw) const
{
	Float32 theAnswer = 0;
	
		SInt32 theOverallRawMin = GetMinimumRaw();
	SInt32 theOverallRawMax = GetMaximumRaw();
	
	if(inRaw < theOverallRawMin) inRaw = theOverallRawMin;
	if(inRaw > theOverallRawMax) inRaw = theOverallRawMax;
	
		SInt32 theNumberRawSteps = inRaw - theOverallRawMin;

		CurveMap::const_iterator theIterator = mCurveMap.begin();
	
		theAnswer = theIterator->second.mMinimum;
	
		while((theNumberRawSteps > 0) && (theIterator != mCurveMap.end()))
	{
				SInt32 theRawMin = theIterator->first.mMinimum;
		SInt32 theRawMax = theIterator->first.mMaximum;
		SInt32 theRawRange = theRawMax - theRawMin;
		
		Float32 theDBMin = theIterator->second.mMinimum;
		Float32 theDBMax = theIterator->second.mMaximum;
		Float32 theDBRange = theDBMax - theDBMin;
		
		Float32 theDBPerRaw = theDBRange / static_cast<Float32>(theRawRange);
		
				SInt32 theRawStepsToAdd = std::min(theRawRange, theNumberRawSteps);
		
				theAnswer += theRawStepsToAdd * theDBPerRaw;
		
				theNumberRawSteps -= theRawStepsToAdd;
		
				std::advance(theIterator, 1);
	}
	
	return theAnswer;
}

Float32	CAVolumeCurve::ConvertRawToScalar(SInt32 inRaw) const
{
		Float32	theDBMin = GetMinimumDB();
	Float32	theDBMax = GetMaximumDB();
	Float32	theDBRange = theDBMax - theDBMin;
	SInt32	theRawMin = GetMinimumRaw();
	SInt32	theRawMax = GetMaximumRaw();
	SInt32	theRawRange = theRawMax - theRawMin;
	
		if(inRaw < theRawMin) inRaw = theRawMin;
	if(inRaw > theRawMax) inRaw = theRawMax;

		Float32 theAnswer = static_cast<Float32>(inRaw - theRawMin) / static_cast<Float32>(theRawRange);

		if(mIsApplyingTransferFunction && (theDBRange > 30.0f))
	{
		theAnswer = powf(theAnswer, mRawToScalarExponentNumerator / mRawToScalarExponentDenominator);
	}

	return theAnswer;
}

Float32	CAVolumeCurve::ConvertDBToScalar(Float32 inDB) const
{
	SInt32 theRawValue = ConvertDBToRaw(inDB);
	Float32 theAnswer = ConvertRawToScalar(theRawValue);
	return theAnswer;
}

SInt32	CAVolumeCurve::ConvertScalarToRaw(Float32 inScalar) const
{
		inScalar = std::min(1.0f, std::max(0.0f, inScalar));
	
		Float32	theDBMin = GetMinimumDB();
	Float32	theDBMax = GetMaximumDB();
	Float32	theDBRange = theDBMax - theDBMin;
	SInt32	theRawMin = GetMinimumRaw();
	SInt32	theRawMax = GetMaximumRaw();
	SInt32	theRawRange = theRawMax - theRawMin;
	
		if(mIsApplyingTransferFunction && (theDBRange > 30.0f))
	{
		inScalar = powf(inScalar, mRawToScalarExponentDenominator / mRawToScalarExponentNumerator);
	}
	
		Float32 theNumberRawSteps = inScalar * static_cast<Float32>(theRawRange);
	theNumberRawSteps = roundf(theNumberRawSteps);
	
		SInt32 theAnswer = theRawMin + static_cast<SInt32>(theNumberRawSteps);
	
	return theAnswer;
}

Float32	CAVolumeCurve::ConvertScalarToDB(Float32 inScalar) const
{
	SInt32 theRawValue = ConvertScalarToRaw(inScalar);
	Float32 theAnswer = ConvertRawToDB(theRawValue);
	return theAnswer;
}
