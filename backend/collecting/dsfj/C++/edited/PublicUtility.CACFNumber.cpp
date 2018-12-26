

#include "CACFNumber.h"


Float32	CACFNumber::GetFixed32() const
{
	SInt32 theFixedValue = GetSInt32();
	
		Float32 theSign = theFixedValue < 0 ? -1.0f : 1.0f;
	theFixedValue *= (SInt32)theSign;
	Float32 theWholePart = (theFixedValue & 0x7FFF0000) >> 16;
	Float32 theFractPart = theFixedValue & 0x0000FFFF;
	theFractPart /= 65536.0f;
	
	return theSign * (theWholePart + theFractPart);
}

Float64	CACFNumber::GetFixed64() const
{
	SInt64 theFixedValue = GetSInt64();
	
		Float64 theSign = theFixedValue < 0 ? -1.0 : 1.0;
	theFixedValue *= (SInt64)theSign;
	Float64 theWholePart = (theFixedValue & 0x7FFFFFFF00000000LL) >> 32;
	Float64 theFractPart = theFixedValue & 0x00000000FFFFFFFFLL;
	theFractPart /= 4294967296.0;
	
	return theSign * (theWholePart + theFractPart);
}
