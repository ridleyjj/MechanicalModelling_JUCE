/*
  ==============================================================================

    Stator.h
    Created: 20 Apr 2022 9:30:03am
    Author:  ridle

  ==============================================================================
*/

#pragma once
#include "jr_PolyBLEP_Oscillators.h"        // used for jr::Oscillator

/** A physical model of the stator that surrounds an electric DC motor and resonates with the spinning motor
*/
class Stator
{
public:

    Stator() { phasor.setMode (jr::Oscillator::OscillatorMode::SAW); }

    /** Sets the sample rate
    * @param sr - sample rate, Hz
    */
    void setSampleRate (float sr) 
    { 
        phasor.setSampleRate (sr);
        phasor.setMuted (false);
    }
    
    /** Sets the stator level
    * @param level - volume level of the stator component (0-1)
    */
    void setStatorLevel (float level) { statorLevel = level; }

    /** returns the next sample out value for the stator
    * @param freq - current frequency of the driving phasor
    */
    float process (float freq)
    {
        phasor.setFrequency (freq / 4.0);

        float output{};

        output = phasor.processSingleSample() + 1.0;

        if (output >= 1)
            output -= 1;

        output = (1.0 / (pow (cos (output), 2) + 1.0)) - 0.5;

        return output * statorLevel;
    }

private:
    float statorLevel{};        // volume level out of stator (0-1)
    jr::Oscillator phasor;      // phasor that controls the resonating, set to 1/4 frequency of driving phasor of the motor
};