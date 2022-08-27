/*
  ==============================================================================

    4_stroke_engine.h
    Created: 28 Apr 2022 4:36:27pm
    Author:  ridle

  ==============================================================================
*/

#pragma once
#include <vector>               // used for std::vector<T>
#include <JuceHeader.h>
using std::vector;
using std::shared_ptr;
using juceDelay = juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>;

/** A model of a single cylinder within a 4 stroke Engine of a car
*/
class Cylinder
{
public:

    /** Constructor
    * @param sr - sample rate, Hz
    * @param pShift - phase shift (0-1)
    */
    Cylinder (float sr, float delayInSeconds, float pShift) : delayTimeInSeconds (delayInSeconds), phaseShift (pShift) {}

    /** Returns the next sample value for the cylinder
    * @param driveIn - current sample value for driving phasor
    * @param delayA - reference to delay line A
    * @param delayB - reference to delay line B
    * @param speed - engine speed control value (0-1)
    */
    float process (float driveIn, juceDelay& delayA, juceDelay& delayB, float speed);

private:
    float delaySizeInSeconds{ 0.03 };               // size of delay buffers in seconds
    float pulseWidth{};                             // pulse width, calculated as a function of the speed value in
    float delayTimeInSeconds;                       // delay time in seconds, to be initiated at construction
    float phaseShift;                               // phase shift amount to stagger cylinder from other instances
    bool initialised{ false };                      // returns true once delay buffer size has been initialised to prevent delay buffer size from being set multiple times
};

/** A model of a 4 Stroke Car Engine with 4 cylinders. Call init() before use, then call process() each sample for output.
*/
class FourStrokeEngine
{
public:

    /** Initialises the FourStrokeEngine
    * @param sr - sample rate, Hz
    */
    void init (float sr);
    
    /** Sets the cylinder mix
    * @param mix - cylinder mix (0-1)
    */
    void setCylinderMix (float mix) 
    { 
        if (mix >= 1) cylinderMix = 1;
        if (mix <= 0) cylinderMix = 0;
        else          cylinderMix = mix; 
    }

    /** Returns the next sample value for the Four Stroke Engine
    * @param speedIn - engine speed control in (0-1)
    * @param driveIn - current sample value of driving phasor
    */
    float process (float speedIn, float driveIn);

private:
    float sampleRate{};                           // sample rate, Hz
    vector<shared_ptr<Cylinder>> cylinders;       // vector of pointers to the 4 cylinders
    juce::Random random;                          // random number generator for noise
    juceDelay delayA;                             // delay buffer A, containing low frequency noise with very small amplitude
    juceDelay delayB;                             // delay buffer B, containing low frequency noise with large amplitude
    juce::dsp::ProcessSpec delaySpecs;            // specs for initialising delay
    juce::IIRFilter lpf1;                         // low pass filter 1
    juce::IIRFilter lpf2;                         // low pass filter 2
    juce::IIRFilter hpf;                          // high pass filter
    float cylinderMix{};                          // output level of cylinders (0-1)
    bool initialised{ false };                    // bool returns true when the component has been initialised
    size_t numCylinders{ 4 };                     // number of cylinders
};
