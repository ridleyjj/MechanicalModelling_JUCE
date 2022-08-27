/*
  ==============================================================================

    OvertoneGenerator.h
    Created: 29 Apr 2022 10:34:14am
    Author:  ridle

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
using juceDelay = juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>;

/** A class that models the generation of 3 separate overtones, each to be fed into the circular waveguide of an Engine model
*/
class OvertoneGenerator
{
public:

    /** Constructor
    */
    OvertoneGenerator();

    /** Sets the parameter values for a specified overtone
    * @param overtoneNum - index of overtone to set parameters for (0, 1 or 2)
    * @param del - transmission delay (0-100), ms
    * @param p - phase shift (0-1)
    * @param freq - frequency control (0-1)
    * @param a - amplitude control (0-1)
    */
    void setOvertoneParams (size_t overtoneNum, float del, float p, float freq, float a);

    /** Sets the sample rate
    * @param sr - sample rate, Hz
    */
    void setSampleRate (float sr);
    
    /** Processes the generator, updating the values for the 3 overtones
    * @param driveIn - current sample value for driving phasor
    */
    void process (float driveIn);

    /** Returns the current sample value of a specified overtone
    * @param overtoneNum - index of desired overtone (0, 1, 2)
    */
    float getOvertoneVal (size_t overtoneNum) { return overtoneSampleVals[overtoneNum]; }

private:

    /** Returns the corresponding sample value for an overtone generated from input phasor and parameters
    * @param driveIn - current sample value for driving phasor
    * @param pShiftIn - phase shift amount (0-1)
    * @param freqIn - frequency control value (0-1)
    * @param ampIn - amplitude control value (0-1)
    */
    float generateOvertone (float driveIn, float pShiftIn, float freqIn, float ampIn);

private:
    float sampleRate{};                 // sample rate, Hz
    juceDelay delay;                    // delay buffer holding driving phasor output
    juce::dsp::ProcessSpec delaySpecs;  // specifications to initialise the delay line
    float transmissionDelayVals[3]{};   // array of transmission delay values coresponding to the 3 overtones (each 0-100ms)
    float phaseShiftVals[3]{};          // array of phase shift values coresponding to the 3 overtones (each 0-1)
    float freqVals[3]{};                // array of frequency control values coresponding to the 3 overtones (each 0-1)
    float ampVals[3]{};                 // array of amplitude control values coresponding to the 3 overtones (each 0-1)
    float overtoneSampleVals[3]{};      // array of current sample values corresponding to the 3 overtones
    float modVals[3]{ 16.0f, 4.0f, 8.0f }; // array of frequency modifiers corresponding to each overtone
    bool initialised{ false };          // bool returns true once delay buffer has been initiated to stop to size being set multiple times
};