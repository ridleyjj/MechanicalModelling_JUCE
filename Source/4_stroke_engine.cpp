/*
  ==============================================================================

    4_stroke_engine.cpp
    Created: 28 Apr 2022 4:36:41pm
    Author:  ridle

  ==============================================================================
*/

#include "4_stroke_engine.h"
#include <memory>               // used for shared_ptr<T>
using std::vector;
using std::shared_ptr;

//=================================== cylinder =========================================//  

float Cylinder::process (float driveIn, juceDelay& delayA, juceDelay& delayB, float speed)
{
    // calculate pulsewidth
    pulseWidth = 2.0f + 3.0f * (1.0f - speed);

    float delayAOut = delayA.popSample(0, delayTimeInSeconds, false);
    float delayBOut = delayB.popSample(0, delayTimeInSeconds, false);

    float sampleOut = cos (juce::MathConstants<float>().twoPi * (driveIn + delayAOut - phaseShift));
    
    sampleOut *= (pulseWidth + delayBOut);
    
    // gaussian transform
    sampleOut = 1.0f / ((sampleOut * sampleOut) + 1.0f);

    return sampleOut;
}

//================================= Four Stroke Engine =====================================//

void FourStrokeEngine::init (float sr)
{
    if (sampleRate == sr)
        return;

    sampleRate = sr;

    delaySpecs.maximumBlockSize = 512;
    delaySpecs.numChannels = 1;
    delaySpecs.sampleRate = sampleRate;

    delayA.prepare (delaySpecs);
    delayB.prepare (delaySpecs);

    delayA.setMaximumDelayInSamples (0.03 * sampleRate);
    delayB.setMaximumDelayInSamples (0.03 * sampleRate);

    lpf1.setCoefficients (juce::IIRCoefficients::makeLowPass(sampleRate, 20.0, 0.01));
    lpf2.setCoefficients (juce::IIRCoefficients::makeLowPass(sampleRate, 20.0, 0.01));
    hpf.setCoefficients (juce::IIRCoefficients::makeHighPass(sampleRate, 2.0, 0.01));

    // if cylinders already exist
    if (cylinders.size() > 0)
        return;
    else
    {
        for (size_t i = 1; i < (numCylinders + 1); i++)
        {
            cylinders.push_back (std::make_shared<Cylinder> (sampleRate, (0.005 * i), (1.0f - (0.25 * i))));
        }
        initialised = true;
    }
}

float FourStrokeEngine::process (float speedIn, float driveIn)
{
    if (initialised == false)
        return 0.0f;

    // generate filtered noise
    float rawNoise = 2.0f * (random.nextFloat() - 0.5f);
    float filteredNoise = lpf1.processSingleSampleRaw (rawNoise);
    filteredNoise = lpf2.processSingleSampleRaw (filteredNoise);

    delayA.pushSample (0, filteredNoise * 0.5f);
    delayB.pushSample (0, filteredNoise * 10.0f);

    // process cylinders, tally output
    float sampleOut{};
    for (size_t i = 0; i < numCylinders; i++)
    {
        sampleOut += cylinders.at (i)->process (driveIn, delayA, delayB, speedIn);
    }

    // updates the readPos of the delay lines
    delayA.popSample (0, -1, true);
    delayB.popSample (0, -1, true);

    // scale output and return
    sampleOut *= (cylinderMix * 2.0f);

    return hpf.processSingleSampleRaw (sampleOut);
    
}