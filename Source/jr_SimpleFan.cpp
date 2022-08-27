/*
  ==============================================================================

    jr_SimpleFan.cpp
    Created: 26 Apr 2022 10:52:41am
    Author:  ridle

  ==============================================================================
*/

#include "jr_SimpleFan.h"

//======================= Tone Component =========================//

float FanToneComponent::process()
{
    rawSineSignal = sineOsc.processSingleSample();

    // waveshaping technique of 1/(1 + x^2) used to obtain narrow pulse wave
    rawSignal = 1.0f / (1.0f + pow(rawSineSignal * pulseWidth, 2));

    return rawSignal * level;
}

//======================= Noise Component =========================//

void FanNoiseComponent::setFilterParams (float freq, float q)
{
    if (freq > 0)
        cutoff = freq;

    if (q > 0)
        resonance = q;
}

float FanNoiseComponent::process (float rawSignalIn)
{
    switch (filterType)
    {
    default:
        filter.setCoefficients (juce::IIRCoefficients::makeBandPass(sampleRate, cutoff, resonance));
        break;
    case 1:
        filter.setCoefficients (juce::IIRCoefficients::makeLowPass(sampleRate, cutoff, resonance));
        break;
    }

    float filteredNoise = filter.processSingleSampleRaw (random.nextFloat());

    float sampleOut = filteredNoise * rawSignalIn;

    return sampleOut * level;
}

//======================= Panner Component =========================//

void FanPanner::process (float controlSignalIn)
{
    rightLevel = (((controlSignalIn + 1.0f) / 2.0f) * panWidth) + 0.5f - (panWidth / 2.0f);

    leftLevel = 1.0f - rightLevel;
}

//======================= Doppler Component =========================//

void FanDopplerComponent::setDopplerParams (float controlSignalIn, float range, float offset, float q)
{
    cutoffRange = range;
    cutoffOffset = offset;

    if (q > 0)
        dopplerRes = q;

    dopplerCutoff = offset + (((controlSignalIn + 1.0f) / 2.0f) * range);

    if (dopplerCutoff < 0)
        dopplerCutoff = 0;
}

float FanDopplerComponent::process (float rawSignalIn)
{
    if (dopplerOn)
    {
        switch (filterType)
        {
        default:
            filter.setCoefficients (juce::IIRCoefficients::makeBandPass (sampleRate, dopplerCutoff, dopplerRes));
            break;
        case 1:
            filter.setCoefficients (juce::IIRCoefficients::makeLowPass (sampleRate, dopplerCutoff, dopplerRes));
            break;
        }

        float filteredNoise = filter.processSingleSampleRaw (random.nextFloat());

        float sampleOut = filteredNoise * rawSignalIn;

        return sampleOut * level;
    }
    else
    {
        return FanNoiseComponent::process (rawSignalIn);
    }
}

//======================= Delay Component =========================//

void FanDelay::setSampleRate (float sr)
{
    sampleRate = sr;

    delayLine.setSampleRate (sampleRate);
    delayLine.setSize (0.4);
}

float FanDelay::process (float controlSignalIn, float audioSignalIn)
{
    float delayTimeInMs = 200 + (controlSignalIn * chop);

    delayLine.setDelayTime (delayTimeInMs / 1000.0f);

    return delayLine.process (audioSignalIn);
}

//======================= Fan Propeller =========================//

FanPropeller::FanPropeller()
{
    fastBladesNoiseComp.setFilterType (1);
    fastBladesToneComp.setPhaseShift (0.25);
}

void FanPropeller::setMappedParams (float gainIn, float speedIn, float toneLevelIn, float noiseLevelIn, float stereoWidthIn, bool dopplerOnIn)
{
    setParams (speedIn, gainIn, 1.0f, 1.0f, toneLevelIn, noiseLevelIn, toneLevelIn, noiseLevelIn, dopplerOnIn, 10.0f, stereoWidthIn);
}

void FanPropeller::setSampleRate (float sr)
{
    mainBladesToneComp.setSampleRate (sr);
    fastBladesToneComp.setSampleRate (sr);
    mainBladesNoiseComp.setSampleRate (sr);
    fastBladesNoiseComp.setSampleRate (sr);
    fastBladesDelayComp.setSampleRate (sr);
}

void FanPropeller::setSpeed (float speedInHz)
{
    mainBladesToneComp.setSpeed (speedInHz);
    fastBladesToneComp.setSpeed (speedInHz);
}

void FanPropeller::setPulseWidth (float pw)
{
    mainBladesToneComp.setPulseWidth (pw);
    fastBladesToneComp.setPulseWidth (pw);
}

void FanPropeller::setParams (float speedInHz, float masterVol, float mainBladesLevelIn, float fastBladesLevelIn, float mainBladesToneLevel, float mainBladesNoiseLevel, float fastBladesToneLevel, float fastBladesNoiseLevel, bool dopplerOn, float chopIn, float panWidthIn)
{
    setSpeed (speedInHz);
    setLevel (masterVol);
    setMainBladesLevel (mainBladesLevelIn);
    setFastBladesLevel (fastBladesLevelIn);
    setMainToneLevel (mainBladesToneLevel);
    setFastToneLevel (fastBladesToneLevel);
    setMainNoiseLevel (mainBladesNoiseLevel);
    setFastNoiseLevel (fastBladesNoiseLevel);
    setDopplerOn (dopplerOn);
    setChop (chopIn);
    setPanWidth (panWidthIn);
}

void FanPropeller::process()
{
    float mainBladesToneOut = mainBladesToneComp.process();
    setDopplerParams();
    float mainBladesOut = mainBladesLevel * (mainBladesToneOut + mainBladesNoiseComp.process (mainBladesToneComp.getRawSignal()));

    float fastBladesToneOut = fastBladesToneComp.process();
    float fastBladesNoiseOut = fastBladesNoiseComp.process (fastBladesToneComp.getRawSignal());
    float fastBladesOut = fastBladesLevel * (fastBladesToneOut + fastBladesDelayComp.process(fastBladesToneComp.getRawSine(), fastBladesNoiseOut));
    
    float rawOut = level * (fastBladesOut + mainBladesOut);

    pannerComp.process (mainBladesToneComp.getRawSine());

    currentLeftSample = rawOut * pannerComp.getLeft();
    currentRightSample = rawOut * pannerComp.getRight();
}

