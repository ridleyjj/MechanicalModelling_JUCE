/*
  ==============================================================================

    FM_Resonator.h
    Created: 20 Apr 2022 9:30:33am
    Author:  ridle

  ==============================================================================
*/

#pragma once
#include "jr_PolyBLEP_Oscillators.h"        // used for jr::Oscillator

/** A class to physically model the resonant casing of an electric DC motor, using FM to model the resonance similar to a tube
*/
class MotorFMResonator
{
public:

    MotorFMResonator() { carrierOsc.setMuted (false); }

    /** Sets the sample rate
    * @param sr - sample rate, Hz
    */
    void setSampleRate (float sr)
    {
        carrierOsc.setSampleRate (sr);
        carrierOsc.setFrequency (carrierFreq);

        hpf.setCoefficients (juce::IIRCoefficients::makeHighPass (sr, filterFreq));
    }

    /** Sets the resonance amount
    * @param level - volume level for the resonator (0-1)
    */
    void setResonanceAmount (float level) { resonanceAmount = level; }

    /** Returns the next sample out value for the resonator
    * @param rotorVal - current sample value for the signal to be used as the excitor for the resonator (generally the current rotor sample out value before or after enveloping)
    * @param phasorVal - current sample value for the driving phasor
    */
    float process (float rotorVal, float phasorVal)
    {
        float output{};

        output = rotorVal * carrierOsc.processSingleSample();

        output += phasorVal;

        output = cos (output);

        for (size_t i = 0; i < 2; i++)
            output = hpf.processSingleSampleRaw (output);

        return output * resonanceAmount;
    }

private:
    jr::Oscillator carrierOsc;          // carrier frequency for FM (kept fixed)
    juce::IIRFilter hpf;                // high pass filter
    float carrierFreq{ 178 };           // frequency of carrier, Hz
    float filterFreq{ 180 };            // cutoff frequency for high pass filters, Hz
    float resonanceAmount{};            // volume level of the resonance
};