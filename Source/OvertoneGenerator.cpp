/*
  ==============================================================================

    OvertoneGenerator.cpp
    Created: 29 Apr 2022 10:34:22am
    Author:  ridle

  ==============================================================================
*/

#include "OvertoneGenerator.h"
#include <math.h>
#include <JuceHeader.h>

//============================== Overtone Generator ===================================//

//===================== mutator functions ===================//

OvertoneGenerator::OvertoneGenerator()
{
    setSampleRate (44100);
}

void OvertoneGenerator::setSampleRate (float sr)
{
    if (sr != sampleRate)
    {
        sampleRate = sr;

        delaySpecs.numChannels = 1;
        delaySpecs.maximumBlockSize = 512;
        delaySpecs.sampleRate = sampleRate;
        
        delay.prepare(delaySpecs);
        delay.setMaximumDelayInSamples (0.5 * sampleRate);
    }
}

void OvertoneGenerator::setOvertoneParams (size_t overtoneNum, float del, float p, float freq, float a)
{
    if (overtoneNum < 0 || overtoneNum > 2)
        return;

    transmissionDelayVals[overtoneNum] = (del / 1000.0f);
    phaseShiftVals[overtoneNum] = p;
    freqVals[overtoneNum] = freq;
    ampVals[overtoneNum] = a;
}

void OvertoneGenerator::process (float driveIn)
{

    // write new value into delay buffer
    delay.pushSample (0, driveIn);

    for (size_t i = 0; i < 3; i++)
    {
        bool updateReadPos = false;
        if (i == 2)
            updateReadPos = true;

        float drive = (delay.popSample (0, transmissionDelayVals[i] * sampleRate, updateReadPos) * modVals[i]);

        while (drive > 1)
            drive -= 1;

        overtoneSampleVals[i] = generateOvertone (drive, phaseShiftVals[i], freqVals[i], ampVals[i]);
    }
}

float OvertoneGenerator::generateOvertone (float driveIn, float pShiftIn, float freqIn, float ampIn)
{
    // ignores phasor values below pShiftIn value
    float output = (driveIn > pShiftIn ? driveIn : pShiftIn) - pShiftIn;

    // shifts range back to 0-1
    output *= (1.0f / (1.0f - pShiftIn));

    // apply frequency shift
    output *= (pShiftIn * freqIn * 12);

    // wrap output into -0.5 to 0.5 range
    while (output >  1)
        output -= 1;

    output -= 0.5f;

    // apply parabolic transform
    output *= output;
    output = (((output) * -4.0f) + 1.0f) * 0.5f;

    // use driveIn to cause a linear decay
    output *= (1.0f - driveIn);

    // apply amplitude control
    output *= (12.0f * ampIn);

    return output;
}