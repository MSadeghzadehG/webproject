
#include <math.h>
#include "InterpolateShannon.h"
#include "STTypes.h"

using namespace soundtouch;


static const double _kaiser8[8] = 
{
   0.41778693317814,
   0.64888025049173,
   0.83508562409944,
   0.93887857733412,
   0.93887857733412,
   0.83508562409944,
   0.64888025049173,
   0.41778693317814
};


InterpolateShannon::InterpolateShannon()
{
    fract = 0;
}


void InterpolateShannon::resetRegisters()
{
    fract = 0;
}


#define PI 3.1415926536
#define sinc(x) (sin(PI * (x)) / (PI * (x)))

int InterpolateShannon::transposeMono(SAMPLETYPE *pdest, 
                    const SAMPLETYPE *psrc, 
                    int &srcSamples)
{
    int i;
    int srcSampleEnd = srcSamples - 8;
    int srcCount = 0;

    i = 0;
    while (srcCount < srcSampleEnd)
    {
        double out;
        assert(fract < 1.0);

        out  = psrc[0] * sinc(-3.0 - fract) * _kaiser8[0];
        out += psrc[1] * sinc(-2.0 - fract) * _kaiser8[1];
        out += psrc[2] * sinc(-1.0 - fract) * _kaiser8[2];
        if (fract < 1e-6)
        {
            out += psrc[3] * _kaiser8[3];             }
        else
        {
            out += psrc[3] * sinc(- fract) * _kaiser8[3];
        }
        out += psrc[4] * sinc( 1.0 - fract) * _kaiser8[4];
        out += psrc[5] * sinc( 2.0 - fract) * _kaiser8[5];
        out += psrc[6] * sinc( 3.0 - fract) * _kaiser8[6];
        out += psrc[7] * sinc( 4.0 - fract) * _kaiser8[7];

        pdest[i] = (SAMPLETYPE)out;
        i ++;

                fract += rate;
                int whole = (int)fract;
        fract -= whole;
        psrc += whole;
        srcCount += whole;
    }
    srcSamples = srcCount;
    return i;
}


int InterpolateShannon::transposeStereo(SAMPLETYPE *pdest, 
                    const SAMPLETYPE *psrc, 
                    int &srcSamples)
{
    int i;
    int srcSampleEnd = srcSamples - 8;
    int srcCount = 0;

    i = 0;
    while (srcCount < srcSampleEnd)
    {
        double out0, out1, w;
        assert(fract < 1.0);

        w = sinc(-3.0 - fract) * _kaiser8[0];
        out0 = psrc[0] * w; out1 = psrc[1] * w;
        w = sinc(-2.0 - fract) * _kaiser8[1];
        out0 += psrc[2] * w; out1 += psrc[3] * w;
        w = sinc(-1.0 - fract) * _kaiser8[2];
        out0 += psrc[4] * w; out1 += psrc[5] * w;
        w = _kaiser8[3] * ((fract < 1e-5) ? 1.0 : sinc(- fract));           out0 += psrc[6] * w; out1 += psrc[7] * w;
        w = sinc( 1.0 - fract) * _kaiser8[4];
        out0 += psrc[8] * w; out1 += psrc[9] * w;
        w = sinc( 2.0 - fract) * _kaiser8[5];
        out0 += psrc[10] * w; out1 += psrc[11] * w;
        w = sinc( 3.0 - fract) * _kaiser8[6];
        out0 += psrc[12] * w; out1 += psrc[13] * w;
        w = sinc( 4.0 - fract) * _kaiser8[7];
        out0 += psrc[14] * w; out1 += psrc[15] * w;

        pdest[2*i]   = (SAMPLETYPE)out0;
        pdest[2*i+1] = (SAMPLETYPE)out1;
        i ++;

                fract += rate;
                int whole = (int)fract;
        fract -= whole;
        psrc += 2*whole;
        srcCount += whole;
    }
    srcSamples = srcCount;
    return i;
}


int InterpolateShannon::transposeMulti(SAMPLETYPE *pdest, 
                    const SAMPLETYPE *psrc, 
                    int &srcSamples)
{
        assert(false);
    return 0;
}
