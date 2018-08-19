
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "FIFOSampleBuffer.h"
#include "PeakFinder.h"
#include "BPMDetect.h"

using namespace soundtouch;

#define INPUT_BLOCK_SAMPLES       2048
#define DECIMATED_BLOCK_SAMPLES   256

const float avgdecay = 0.99986f;

const float avgnorm = (1 - avgdecay);





#ifdef _CREATE_BPM_DEBUG_FILE

    #define DEBUGFILE_NAME  "c:\\temp\\soundtouch-bpm-debug.txt"

    static void _SaveDebugData(const float *data, int minpos, int maxpos, double coeff)
    {
        FILE *fptr = fopen(DEBUGFILE_NAME, "wt");
        int i;

        if (fptr)
        {
            printf("\n\nWriting BPM debug data into file " DEBUGFILE_NAME "\n\n");
            for (i = minpos; i < maxpos; i ++)
            {
                fprintf(fptr, "%d\t%.1lf\t%f\n", i, coeff / (double)i, data[i]);
            }
            fclose(fptr);
        }
    }
#else
    #define _SaveDebugData(a,b,c,d)
#endif



BPMDetect::BPMDetect(int numChannels, int aSampleRate)
{
    this->sampleRate = aSampleRate;
    this->channels = numChannels;

    decimateSum = 0;
    decimateCount = 0;

    envelopeAccu = 0;

            #ifdef SOUNDTOUCH_INTEGER_SAMPLES
        RMSVolumeAccu = (1500 * 1500) / avgnorm;
#else
        RMSVolumeAccu = (0.045f * 0.045f) / avgnorm;
#endif

        decimateBy = sampleRate / 1000;
    assert(decimateBy > 0);
    assert(INPUT_BLOCK_SAMPLES < decimateBy * DECIMATED_BLOCK_SAMPLES);

        windowLen = (60 * sampleRate) / (decimateBy * MIN_BPM);
    windowStart = (60 * sampleRate) / (decimateBy * MAX_BPM);

    assert(windowLen > windowStart);

        xcorr = new float[windowLen];
    memset(xcorr, 0, windowLen * sizeof(float));

        buffer = new FIFOSampleBuffer();
        buffer->setChannels(1);
    buffer->clear();
}



BPMDetect::~BPMDetect()
{
    delete[] xcorr;
    delete buffer;
}



int BPMDetect::decimate(SAMPLETYPE *dest, const SAMPLETYPE *src, int numsamples)
{
    int count, outcount;
    LONG_SAMPLETYPE out;

    assert(channels > 0);
    assert(decimateBy > 0);
    outcount = 0;
    for (count = 0; count < numsamples; count ++) 
    {
        int j;

                for (j = 0; j < channels; j ++)
        {
            decimateSum += src[j];
        }
        src += j;

        decimateCount ++;
        if (decimateCount >= decimateBy) 
        {
                        out = (LONG_SAMPLETYPE)(decimateSum / (decimateBy * channels));
            decimateSum = 0;
            decimateCount = 0;
#ifdef SOUNDTOUCH_INTEGER_SAMPLES
                        if (out > 32767) 
            {
                out = 32767;
            } 
            else if (out < -32768) 
            {
                out = -32768;
            }
#endif             dest[outcount] = (SAMPLETYPE)out;
            outcount ++;
        }
    }
    return outcount;
}



void BPMDetect::updateXCorr(int process_samples)
{
    int offs;
    SAMPLETYPE *pBuffer;
    
    assert(buffer->numSamples() >= (uint)(process_samples + windowLen));

    pBuffer = buffer->ptrBegin();
    #pragma omp parallel for
    for (offs = windowStart; offs < windowLen; offs ++) 
    {
        LONG_SAMPLETYPE sum;
        int i;

        sum = 0;
        for (i = 0; i < process_samples; i ++) 
        {
            sum += pBuffer[i] * pBuffer[i + offs];            }
                                                                                                                                                                
        xcorr[offs] += (float)sum;
    }
}


void BPMDetect::calcEnvelope(SAMPLETYPE *samples, int numsamples) 
{
    const static double decay = 0.7f;                   const static double norm = (1 - decay);

    int i;
    LONG_SAMPLETYPE out;
    double val;

    for (i = 0; i < numsamples; i ++) 
    {
                RMSVolumeAccu *= avgdecay;
        val = (float)fabs((float)samples[i]);
        RMSVolumeAccu += val * val;

                        if (val < 0.5 * sqrt(RMSVolumeAccu * avgnorm))
        {
            val = 0;
        }

                envelopeAccu *= decay;
        envelopeAccu += val;
        out = (LONG_SAMPLETYPE)(envelopeAccu * norm);

#ifdef SOUNDTOUCH_INTEGER_SAMPLES
                if (out > 32767) out = 32767;
#endif         samples[i] = (SAMPLETYPE)out;
    }
}



void BPMDetect::inputSamples(const SAMPLETYPE *samples, int numSamples)
{
    SAMPLETYPE decimated[DECIMATED_BLOCK_SAMPLES];

        while (numSamples > 0)
    {
        int block;
        int decSamples;

        block = (numSamples > INPUT_BLOCK_SAMPLES) ? INPUT_BLOCK_SAMPLES : numSamples;

                decSamples = decimate(decimated, samples, block);
        samples += block * channels;
        numSamples -= block;

                calcEnvelope(decimated, decSamples);
        buffer->putSamples(decimated, decSamples);
    }

        if ((int)buffer->numSamples() > windowLen) 
    {
        int processLength;

                processLength = (int)buffer->numSamples() - windowLen;

                updateXCorr(processLength);
                buffer->receiveSamples(processLength);
    }
}



void BPMDetect::removeBias()
{
    int i;
    float minval = 1e12f;   
    for (i = windowStart; i < windowLen; i ++)
    {
        if (xcorr[i] < minval)
        {
            minval = xcorr[i];
        }
    }

    for (i = windowStart; i < windowLen; i ++)
    {
        xcorr[i] -= minval;
    }
}


float BPMDetect::getBpm()
{
    double peakPos;
    double coeff;
    PeakFinder peakFinder;

    coeff = 60.0 * ((double)sampleRate / (double)decimateBy);

        _SaveDebugData(xcorr, windowStart, windowLen, coeff);

        removeBias();

        peakPos = peakFinder.detectPeak(xcorr, windowStart, windowLen);

    assert(decimateBy != 0);
    if (peakPos < 1e-9) return 0.0; 
        return (float) (coeff / peakPos);
}
