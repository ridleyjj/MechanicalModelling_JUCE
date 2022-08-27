/*
  ==============================================================================

    CircularWaveguide.cpp
    Created: 29 Apr 2022 12:48:51pm
    Author:  ridle

  ==============================================================================
*/

#include "CircularWaveguide.h"
#include <JuceHeader.h>

//=========================== constructors =================================//

CircularWaveguide::CircularWaveguide()
{
    setSampleRate (44100); 
}

//========================= mutator functions ==============================//

void CircularWaveguide::setSampleRate (float sr)
{
    if (sampleRate == sr)
        return;

    sampleRate = sr;
    

    //========== initialise delay lines ===========//

    delaySpecs.maximumBlockSize = 512;
    delaySpecs.sampleRate = sampleRate;
    delaySpecs.numChannels = 1;

    delay1.prepare (delaySpecs);
    delay2.prepare (delaySpecs);
    delay3.prepare (delaySpecs);
    delay4.prepare (delaySpecs);
    delayedDrive.prepare (delaySpecs);

    float sizeInSamples = 0.12f * sampleRate;
    delay1.setMaximumDelayInSamples (sizeInSamples);
    delay2.setMaximumDelayInSamples (sizeInSamples);
    delay3.setMaximumDelayInSamples (sizeInSamples);
    delay4.setMaximumDelayInSamples (sizeInSamples);
    delayedDrive.setMaximumDelayInSamples (sizeInSamples * 3.0f);

    //=========== initialise filters ===========//

    hpf1.setCoefficients (juce::IIRCoefficients::makeHighPass(sampleRate, 30.0, 0.01));   
}

void CircularWaveguide::setDimensions (float w1, float w2, float l1, float l2)
{
    width1 = w1;
    width2 = w2;
    length1 = l1;
    length2 = l2;
}

void CircularWaveguide::setParams (float parabDelay, float parabMix, float wDelay, float warpAmt)
{
    parabolicDelay = parabDelay;
    parabolicMix = parabMix;
    warpDelay = wDelay;
    waveguideWarp = warpAmt;
}

void CircularWaveguide::updateParams (float speedIn, float driveIn)
{
    delayedDrive.pushSample (0, driveIn);

    // update 'a'
    a = delayedDrive.popSample (0, ((parabolicDelay / 1000.0f) * sampleRate));
    // parabola transform
    a -= 0.5f;
    a = 0.5f * ((-4.0f * pow (a, 2)) + 1.0f);
    a *= (parabolicMix * 2.0f);
    
    // update 'fm1'
    float cosineCurve = cos (juce::MathConstants<float>().twoPi * delayedDrive.popSample (0, (warpDelay / 1000.0f)*sampleRate));
    
    float warpAmount = speedIn * waveguideWarp;

    fm1 = 0.5f + ((1.0f - cosineCurve) * warpAmount);

    fm2 = 0.5f + (cosineCurve * warpAmount);
}

//======================== accessor functions =============================//

float CircularWaveguide::process (float speedIn, float driveIn, float b, float c, float d)
{
    updateParams (speedIn, driveIn);

    float output{};
    
    delay1.pushSample (0, (hpf1.processSingleSampleRaw(a) + (feedbackAmt * fbSignal2)));

    float delayOut = delay1.popSample (0, (sampleRate * ((width2 * fm2) / 1000.0f)));

    float outputSubMix = delayOut + b;

    output += outputSubMix;

    delay2.pushSample (0, outputSubMix);

    fbSignal1 = delay2.popSample (0, (sampleRate * ((length1 * fm1) / 1000.0f)));
    output += fbSignal1;

    delay3.pushSample (0, fbSignal1 + c);

    outputSubMix = delay3.popSample (0, (sampleRate * ((width1 * fm1) / 1000.0f))) + d;
    output += outputSubMix;

    delay4.pushSample (0, outputSubMix);

    fbSignal2 = delay4.popSample (0, (sampleRate * ((length2 * fm2) / 1000.0f)));

    output += fbSignal2;

    return output;
}