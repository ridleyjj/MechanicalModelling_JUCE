/*
  ==============================================================================

    CircularWaveguide.h
    Created: 29 Apr 2022 12:48:51pm
    Author:  ridle

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
using juceDelay = juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>;

/** Circular Non-Linear Warping Waveguide used to model the effect of the exhaust system in a car. 
Use setSampleRate() before use, then setParams() or setMappedParams() to set parameters, and call process() each sample for output.
*/
class CircularWaveguide
{
public:

    /** Constructor
    */
    CircularWaveguide();

    /** Sets the sample rate
    * @param sr - sample rate, Hz
    */
    void setSampleRate (float sr);

    /** Sets the dimensions of the waveguide
    * @param w1 - width 1 (0-1)
    * @param w2 - width 2 (0-1)
    * @param l1 - length 1 (0-1)
    * @param l2 - length 2 (0-1)
    */
    void setDimensions (float w1, float w2, float l1, float l2);

    /** sets the amount of feedback
    * @param fb - feedback amount (0-1)
    */
    void setFeedbackAmt (float fb) { feedbackAmt = fb; }

    /** Sets the params of the waveguide
    * @param parabDelay - delay in ms for driver to signal 'a' (0-100)
    * @param parabMix - mix amount for signal 'a' (0-1)
    * @param wDelay - delay in ms for 'fm1' and 'fm2' (0-100)
    * @param warpAmt - amount of driving signal sent to 'fm1' and 'fm2' (0-1)
    */
    void setParams (float parabDelay, float parabMix, float wDelay, float warpAmt);

    /** Returns the next sample value for the waveguide
    * @param speedIn - engine speed (0-1)
    * @param driveIn - current sample value for driving phasor
    * @param b - current sample value of the input signal 'b' (from overtone generator)
    * @param c - current sample value of the input signal 'c' (from overtone generator)
    * @param d - current sample value of the input signal 'd' (from overtone generator)
    */
    float process (float speedIn, float driveIn, float b, float c, float d);

private:

    /** Uses speed in and driving phasor to update values for signals 'a' 'fm1' and 'fm2'
    * @param speedIn - engine speed (0-1)
    * @param driveIn - current sample value for driving phasor
    */
    void updateParams (float speedIn, float driveIn);

private:
    float sampleRate{};         // sample rate, Hz

    float feedbackAmt{};        // feedback amount (0-1)
    float fbSignal1{};          // current sample value for signal to be feedback through the delays
    float fbSignal2{};          // current sample value for signal to be feedback through the delays

    juce::IIRFilter hpf1;       // high pass filter to filter signal 'a'

    float width1{};             // width 1 (0-40)
    float width2{};             // width 2 (0-40)
    float length1{};            // length 1 (0-40)
    float length2{};            // length 2 (0-40)

    juceDelay delay1;           // delay buffer using linear interpolation
    juceDelay delay2;           // delay buffer using linear interpolation
    juceDelay delay3;           // delay buffer using linear interpolation
    juceDelay delay4;           // delay buffer using linear interpolation
    juceDelay delayedDrive;     // delay buffer using linear interpolation for driving phasor

    juce::dsp::ProcessSpec delaySpecs;  // process specs used to initialise delay lines

    float parabolicDelay{};     // delay in ms for driver to signal 'a' (0 - 100)
    float parabolicMix{};       // mix amount for signal 'a' (0-1)
    float warpDelay{};          // delay in ms for 'fm1' and 'fm2' (0-100)
    float waveguideWarp{};      // amount of driving signal sent to 'fm1' and 'fm2' (0-1)

    // parameters calculated within class
    float a{};                  // current sample value for signal 'a'
    float fm1{};                // current sample value for first signal controlling fm
    float fm2{};                // current sample value for second signal controlling fm
};