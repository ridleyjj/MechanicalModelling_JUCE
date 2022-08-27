/*
  ==============================================================================

    jr_Engine.h
    Created: 29 Apr 2022 3:08:22pm
    Author:  ridle

  ==============================================================================
*/

#pragma once
#include "jr_PolyBLEP_Oscillators.h"        // used for jr::Oscillator class
#include "4_stroke_engine.h"                // used for FourStrokeEngine class
#include "OvertoneGenerator.h"              // used for OvertoneGenerator class
#include "CircularWaveguide.h"              // used for CircularWaveguide class

/** Physical Model of a combustion engine based on the system laid out by Andy Farnell in 'Designing Sound' (2010), p.507-516
Use setSampleRate() before use, then setMappedParams() to set params, and call process() each sample for output
*/
class Engine
{
public:

    Engine();

    /** Sets the engine parameters via a smaller set of parameters that the others are mapped to
    * @param gainIn - engine gain (0-1)
    * @param speedIn - engine speed
    */
    void setMappedParams (float gainIn, float speedIn, float aggressionIn, float widthIn, float lengthIn, float ot1LevelIn, float ot2LevelIn, float ot3LevelIn);

    /** Sets the sample rate
    * @param sr - sample rate, Hz
    */
    void setSampleRate (float sr);

    /** sets the speed of the engine
    * @param speedIn - speed (0-1)
    */
    void setSpeed (float speedIn);

    /** Sets all parameters for engine
    * @param cylinderMix - output level of four stroke engine cylinders (0-1)
    * @param transmissionDelay1 - transmission delay for overtone 1 (0-100), ms
    * @param phaseShift1 - phase shift for overtone 1 (0-1)
    * @param freq1 - frequency control for overtone 1 (0-1)
    * @param amp1 - amplitude control for overtone 1 (0-1)
    * @param transmissionDelay2 - transmission delay for overtone 2 (0-100), ms
    * @param phaseShift2 - phase shift for overtone 2 (0-1)
    * @param freq2 - frequency control for overtone 2 (0-1)
    * @param amp2 - amplitude control for overtone 2 (0-1)
    * @param transmissionDelay3 - transmission delay for overtone 3 (0-100), ms
    * @param phaseShift3 - phase shift for overtone 3 (0-1)
    * @param freq3 - frequency control for overtone 3 (0-1)
    * @param amp3 - amplitude control for overtone 3 (0-1)
    * @param width1 - width 1 of exhaust/waveguide (0-1)
    * @param width2 - width 2 of exhaust/waveguide (0-1)
    * @param length1 - length 1 of exhaust/waveguide (0-1)
    * @param length2 - length 2 of exhaust/waveguide (0-1)
    * @param feedbackAmt - feeback amount for exhaust/waveguide (0-1)
    * @param parabolicDelay - delay in ms for driver to signal 'a' (0-100) (for waveguide)
    * @param parabolicMix - mix amount for signal 'a' (0-1) (for waveguide)
    * @param warpDelay - delay in ms for 'fm1' and 'fm2' (0-100) (for waveguide)
    * @param waveguideWarp - amount of driving signal sent to 'fm1' and 'fm2' (0-1) (for waveguide)
    */
    void setParams (float gain, float cylinderMix, float transmissionDelay1, float phaseShift1, float freq1, float amp1, float transmissionDelay2,
        float phaseShift2, float freq2, float amp2, float transmissionDelay3, float phaseShift3,
        float freq3, float amp3, float width1, float width2, float length1, float length2, float feedbackAmt,
        float parabolicDelay, float parabolicMix, float warpDelay, float waveguideWarp, float jitterAmt);

    /** Returns the next sample value for the engine
    * @return sampleOut
    */
    float process();

private:
    OvertoneGenerator overtoneGenerator;    
    CircularWaveguide waveguide;            
    FourStrokeEngine fourStrokeEngine;      

    juce::IIRFilter lpf;                    // low pass filter for filter waveguide out

    float sampleRate{};                     // sample rate, Hz
    jr::Oscillator phasor;                  // driving phasor - important to not use a polyBLEP anti-aliasing osc, as this causes inconsistencies and clicks in the produced pulse waves
    float speed{};                          // current speed value of engine (0-1)
    juce::SmoothedValue<float> frequency;   // freqeuncy of phasor, Hz
    float smoothingTimeInSeconds{ 0.55 };   // smoothing time for phasor frequency, in seconds
    float speedJitter{ 0.1 };               // speed jitter amount (0.1 - 1)
    juce::Random randomNoise;               // random number generator used for noise of speed jitter
    float engineLevelVal{ 1.0f };           // engine volume (0-1) used for fade out with speed
    float engineMasterGain{ 1.0f };         // engine master volume used for overall volume control
    juce::SmoothedValue<float> smoothedGain;// smoothed value for gain
    juce::SmoothedValue<float> engineLevel; // smoothed engine volume
    int count{};                            // count used to change speed offset every set number of samples
};