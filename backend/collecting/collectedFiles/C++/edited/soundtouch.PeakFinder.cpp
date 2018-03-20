
#include <math.h>
#include <assert.h>

#include "PeakFinder.h"

using namespace soundtouch;

#define max(x, y) (((x) > (y)) ? (x) : (y))


PeakFinder::PeakFinder()
{
    minPos = maxPos = 0;
}


int PeakFinder::findTop(const float *data, int peakpos) const
{
    int i;
    int start, end;
    float refvalue;

    refvalue = data[peakpos];

        start = peakpos - 10;
    if (start < minPos) start = minPos;
    end = peakpos + 10;
    if (end > maxPos) end = maxPos;

    for (i = start; i <= end; i ++)
    {
        if (data[i] > refvalue)
        {
            peakpos = i;
            refvalue = data[i];
        }
    }

        if ((peakpos == start) || (peakpos == end)) return 0;

    return peakpos;
}


int PeakFinder::findGround(const float *data, int peakpos, int direction) const
{
    int lowpos;
    int pos;
    int climb_count;
    float refvalue;
    float delta;

    climb_count = 0;
    refvalue = data[peakpos];
    lowpos = peakpos;

    pos = peakpos;

    while ((pos > minPos+1) && (pos < maxPos-1))
    {
        int prevpos;

        prevpos = pos;
        pos += direction;

                delta = data[pos] - data[prevpos];
        if (delta <= 0)
        {
                        if (climb_count)
            {
                climb_count --;              }

                        if (data[pos] < refvalue)
            {
                                lowpos = pos;
                refvalue = data[pos];
            }
        }
        else
        {
                        climb_count ++;
            if (climb_count > 5) break;            }
    }
    return lowpos;
}


int PeakFinder::findCrossingLevel(const float *data, float level, int peakpos, int direction) const
{
    float peaklevel;
    int pos;

    peaklevel = data[peakpos];
    assert(peaklevel >= level);
    pos = peakpos;
    while ((pos >= minPos) && (pos < maxPos))
    {
        if (data[pos + direction] < level) return pos;           pos += direction;
    }
    return -1;  }


double PeakFinder::calcMassCenter(const float *data, int firstPos, int lastPos) const
{
    int i;
    float sum;
    float wsum;

    sum = 0;
    wsum = 0;
    for (i = firstPos; i <= lastPos; i ++)
    {
        sum += (float)i * data[i];
        wsum += data[i];
    }

    if (wsum < 1e-6) return 0;
    return sum / wsum;
}



double PeakFinder::getPeakCenter(const float *data, int peakpos) const
{
    float peakLevel;                int crosspos1, crosspos2;       float cutLevel;                 float groundLevel;              int gp1, gp2;               
        gp1 = findGround(data, peakpos, -1);
    gp2 = findGround(data, peakpos, 1);

    peakLevel = data[peakpos];

    if (gp1 == gp2) 
    {
                assert(gp1 == peakpos);
        cutLevel = groundLevel = peakLevel;
    } else {
                groundLevel = 0.5f * (data[gp1] + data[gp2]);

                cutLevel = 0.70f * peakLevel + 0.30f * groundLevel;
    }

        crosspos1 = findCrossingLevel(data, cutLevel, peakpos, -1);
    crosspos2 = findCrossingLevel(data, cutLevel, peakpos, 1);

    if ((crosspos1 < 0) || (crosspos2 < 0)) return 0;   
        return calcMassCenter(data, crosspos1, crosspos2);
}



double PeakFinder::detectPeak(const float *data, int aminPos, int amaxPos) 
{

    int i;
    int peakpos;                    double highPeak, peak;

    this->minPos = aminPos;
    this->maxPos = amaxPos;

        peakpos = minPos;
    peak = data[minPos];
    for (i = minPos + 1; i < maxPos; i ++)
    {
        if (data[i] > peak) 
        {
            peak = data[i];
            peakpos = i;
        }
    }
    
        highPeak = getPeakCenter(data, peakpos);
    peak = highPeak;

            
    for (i = 3; i < 10; i ++)
    {
        double peaktmp, harmonic;
        int i1,i2;

        harmonic = (double)i * 0.5;
        peakpos = (int)(highPeak / harmonic + 0.5f);
        if (peakpos < minPos) break;
        peakpos = findTop(data, peakpos);           if (peakpos == 0) continue;         
                peaktmp = getPeakCenter(data, peakpos);

                                
        double diff = harmonic * peaktmp / highPeak;
        if ((diff < 0.96) || (diff > 1.04)) continue;   
                i1 = (int)(highPeak + 0.5);
        i2 = (int)(peaktmp + 0.5);
        if (data[i2] >= 0.4*data[i1])
        {
                                    peak = peaktmp;
        }
    }

    return peak;
}
