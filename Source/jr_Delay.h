/*
  ==============================================================================

    jr_Delay.h

  ==============================================================================
*/

#pragma once

#include <cmath>        // included for floor()
#include <JuceHeader.h>

/**
A delay class using a Fractional Delay Line for smoother delay time variation. Use setSampleRate(), setSize() and setDelayTime() before use - the call process() each sample for output
*/
class FractionalDelay
{
public:
    /**
    * Destructor, releases buffer memory on instance destruction
    * 
    * @return
    */
    ~FractionalDelay()
    {
        delete[] buffer;
    }

    /**
    * sets the sample rate
    * 
    * @param sr - sample rate, Hz
    */
    void setSampleRate (float sr)
    {
        sampleRate = sr;
    }

    /**
    * sets the size of the delay buffer, which is effectively the maximum possible delay time
    *
    * @param maxDelayTime - maximum delay time/length, seconds
    */
    void setSize (float maxDelayTime)
    {
        if (maxDelayTime < 0.01) size = 0.01 * sampleRate;
        else                     size = maxDelayTime * sampleRate;
        buffer = new float[size];

        clearBuffer();
    }

    /**
    * sets the delay time/delay length
    * 
    * @param delayTime - delay time, seconds
    */
    void setDelayTime (float delayTime)
    {
        delayTimeInSamples = delayTime * sampleRate;

        readPos = writePos - delayTimeInSamples;
        while (readPos < 0)
            readPos += size;
    }

    /**
    * sets the delay time/delay length directly in samples
    *
    * @param delayTimeInSamplesIn - delay time, samples
    */
    void setDelayTimeInSamples (float delayTimeInSamplesIn)
    {
        delayTimeInSamples = delayTimeInSamplesIn;

        readPos = writePos - delayTimeInSamples;
        if (readPos < 0)
            readPos += size;
    }

    /**
    * sets the feedback amount
    *
    * @param feedback - feedback amount, (0 - 1)
    */
    void setFeedback (float feedback)
    {
        if (feedback > 1) feedbackAmt = 1;
        if (feedback < 0) feedbackAmt = 0;
        else              feedbackAmt = feedback;
    }

    /**
    * Sets the wet mix of the delay (0 = dry signal only, 1 = wet signal only)
    * @param mix - wet/dry mix (0-1)
    */
    void setWetMix (float mix)
    {
        if (mix > 1) wetMix = 1;
        if (mix < 0) wetMix = 0;
        else         wetMix = mix;
    }


    /**
    * writes the input dry signal into the delay buffer, and returns next sample value of the wet signal mixed with the input dry signal
    * 
    * @param drySignal - current sample value for the incoming dry signal
    * @return nextSample - next sample value of wet/dry mixed signal
    */
    float process (float drySignal)
    {
        float output = readVal();

        writeVal ((output * feedbackAmt) + drySignal);

        return (output * wetMix) + ((1 - wetMix) * drySignal);
    }

    /**
    * returns the sample value that correlates with current readPos in delay buffer, and increments readPos
    * 
    * @return readV - sample value for readPos
    */
    float readVal()
    {
        float readV = linearInterpolation (readPos);

        readPos++;

        if (readPos >= size)
        {
            readPos -= size;
        }

        return readV;
    }

    /**
    * writes a sample value into the delay buffer at the current writePos and then increments the writePos
    * 
    * @param sampleIn - sample value to be written into delay buffer
    */
    void writeVal (float sampleIn)
    {
        buffer[writePos] = sampleIn;

        writePos++;

        if (writePos >= size)
            writePos = 0;
    }

    /**
    * sets all buffer values to 0.0f
    */
    void clearBuffer()
    {
        for (int i = 0; i < size; i++)
        {
            buffer[i] = 0.0f;
        }
    }

    /**
    * returns the interpolated sample value for an index value that lies between two discreet index values in an array
    *
    * @param readPosIn - index value to be evaluated
    * @return interpolatedSample - the interpolated sample value that correlates to _readPos
    */
    float linearInterpolation (float readPosIn)
    {
        int indexA = floor (readPosIn);
        int indexB = indexA + 1;

        if (indexB >= size)
            indexB -= size;

        jassert(indexA >= 0 && indexA <= size);
        jassert(indexB >= 0 && indexB <= size);

        float remainder = readPosIn - indexA;

        float interpolatedSample = (remainder * buffer[indexB]) + ((1 - remainder) * buffer[indexA]);

        return interpolatedSample;
    }

private:
    float sampleRate;            // sample rate, Hz
    float* buffer;               // delay buffer, array of floats
    int size;                    // size of delay buffer in samples (maximum delay length in samples)
    float delayTimeInSamples;    // current delay time/length in samples
    float feedbackAmt{ 0.0f };   // feedback amount (0 - 1), amount of wet signal fed back through the delay line
    float readPos{ 0.0f };       // index of delay buffer array where output is currently being output from
    int writePos{ 0 };           // index of delay buffer array where delayed signal is currently being written to
    float wetMix{ 0.33f };       // dry/wet mix of wet signal vs. dry signal, 0 = only dry, 1 = only wet
};