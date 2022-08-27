/*
  ==============================================================================

    jr_Engine.cpp
    Created: 29 Apr 2022 3:08:22pm
    Author:  ridle

  ==============================================================================
*/

#include "jr_Engine.h"

//=========================== Constructors ==============================//

Engine::Engine()
{
    phasor.setMode (jr::Oscillator::OscillatorMode::SAW);
    phasor.setMuted (false);
    setSampleRate (44100);
}

//========================= mutator functions ===========================//

void Engine::setMappedParams (float gainIn, float speedIn, float aggressionIn, float widthIn, float lengthIn, float ot1LevelIn, float ot2LevelIn, float ot3LevelIn)
{
    float warpVal = 0.4 + (aggressionIn * 0.32);
    float widthVal = 4 + (widthIn * 20.0f);
    float lengthVal = 4 + (lengthIn * 20.0f);
    float otL1 = 0.1 + (ot1LevelIn * 0.2);      // overtone level 1 mapped
    float otL2 = 0.1 + (ot2LevelIn * 0.2);      // overtone level 2 mapped
    float otL3 = 0.1 + (ot3LevelIn * 0.2);      // overtone level 3 mapped
    setParams (gainIn, 0.6f, 30.0f, 0.2f, 0.8f, otL1, 55.0f, 0.6f, 0.2f, otL2, 75.0f, 0.85f, 0.5f, otL3, widthVal, widthVal, lengthVal, lengthVal, 0.35f, 50.0f, 0.5f, 50.0f, warpVal, 1.0f);
    setSpeed (speedIn);
}

void Engine::setSampleRate (float sr)
{
    if (sampleRate == sr)
        return;

    sampleRate = sr;
    overtoneGenerator.setSampleRate (sampleRate);
    waveguide.setSampleRate (sampleRate);
    fourStrokeEngine.init (sampleRate);
    frequency.reset (sampleRate, smoothingTimeInSeconds);
    engineLevel.reset (sampleRate, smoothingTimeInSeconds);
    engineLevel.setCurrentAndTargetValue (0);
    smoothedGain.reset (sampleRate, 0.1);

    lpf.setCoefficients (juce::IIRCoefficients::makeLowPass(sampleRate, 8000));
}

void Engine::setSpeed (float speedIn)
{

    float noise = (randomNoise.nextFloat() - 0.5f) / 5.0f; // white noise values scaled down

    speed = speedIn + (noise * speedJitter);
    if (speed > 1)
        speed = 1;

    frequency.setTargetValue (speed * 40.0f);
}

void Engine::setParams (float gain, float cylinderMix, float transmissionDelay1, float phaseShift1, float freq1, float amp1, float transmissionDelay2, 
                        float phaseShift2, float freq2, float amp2, float transmissionDelay3, float phaseShift3, 
                        float freq3, float amp3, float width1, float width2, float length1, float length2, float feedbackAmt,
                        float parabolicDelay, float parabolicMix, float warpDelay, float waveguideWarp, float jitterAmt)
{
    smoothedGain.setTargetValue (gain);
    speedJitter = jitterAmt;
    fourStrokeEngine.setCylinderMix (cylinderMix);

    overtoneGenerator.setOvertoneParams (0, transmissionDelay1, phaseShift1, freq1, amp1);
    overtoneGenerator.setOvertoneParams (1, transmissionDelay2, phaseShift2, freq2, amp2);
    overtoneGenerator.setOvertoneParams (2, transmissionDelay3, phaseShift3, freq3, amp3);

    waveguide.setDimensions (width1, width2, length1, length2);
    waveguide.setFeedbackAmt (feedbackAmt);
    waveguide.setParams (parabolicDelay, parabolicMix, warpDelay, waveguideWarp);
}

float Engine::process()
{
    // attenuate volume with speed
    if (speed < 0.4)
    {
        float mod = 10.0f * (0.2 - (speed - 0.2));   // speed value between 0.2 and 0.4 mapped to 2 - 0
        engineLevel.setTargetValue (exp (pow (mod, 2) * -1.0f));
        if (speed < 0.2)
            engineLevel.setTargetValue (0);
    }
    else if (speed > 0.4)
        engineLevel.setTargetValue (1);

    engineLevelVal = engineLevel.getNextValue();

    float frequencyVal = frequency.getNextValue();
    phasor.setFrequency (frequencyVal);
    float drive = 0.5f * (phasor.processSingleSample() + 1.0f);     // saw osc output converted to phasor 0-1

    overtoneGenerator.process (drive);
    float waveguideOut = waveguide.process (speed, drive, overtoneGenerator.getOvertoneVal (0), overtoneGenerator.getOvertoneVal (1), overtoneGenerator.getOvertoneVal (2));
    waveguideOut = lpf.processSingleSampleRaw (waveguideOut);
    float fourStrokeEngineOut = fourStrokeEngine.process (speed, drive);
    

    float gainVal = smoothedGain.getNextValue();
    return ((0.5f * (waveguideOut + fourStrokeEngineOut)) * engineLevelVal) * gainVal;
}
