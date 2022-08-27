/*
  ==============================================================================

    Motor_Envelope.h
    Created: 20 Apr 2022 9:30:50am
    Author:  ridle

  ==============================================================================
*/

#pragma once

/** A class to simulate the behaviour of an electric DC motor as it turns on and off, by modelling an envelope of its frequency and volume
* use setSampleRate() before use, then call process() every sample, and call powerOn() and powerOff() to cause envelope to rise or fall
*/
class MotorEnvelope
{
public:

    //================== mutators ===================//

    /** Sets the sample rate
    * @param sr - sample rate, Hz
    */
    void setSampleRate (float sr) 
    { 
        sampleRate = sr;
        volDelta = 1.0f / (powerDownTimeSeconds * sampleRate);
    }

    /** Sets the time in seconds that it takes for the envelope to reach its max value from 0
    * @param time - power up time, seconds
    */
    void setPowerUpTime (float time) { powerUpTimeSeconds = time; }

    /** Sets the time in seconds that it takes for the envelope to fall from its max value to 0
    * @param time - power down time, seconds
    */
    void setPowerDownTime (float time) 
    { 
        powerDownTimeSeconds = time; 
        volDelta = 1.0f / (powerDownTimeSeconds * sampleRate);
    }

    /** Sets the acceleration rate of the envelope
    * @param rate - 0-1 value, where 0 is the minimum acceleration rate, and 1 is the max
    */
    void setAccelRate (float rate) { accelRate = rate; }

    //=============== actions ===============//

    /** Simulates motor turning on, by causing envelope to rise to its maximum value over set time
    * @return
    */
    void powerOn() 
    { 
        phase.setCurrentAndTargetValue (0.0f); 
        phase.reset (sampleRate, powerUpTimeSeconds);
        phase.setTargetValue (0.5f);
    }

    /** Simulates motor turning off, by causing envelope to fall to 0 over set time
    * @return
    */
    void powerOff()
    {
        poweringOff = true;
        phase.setCurrentAndTargetValue (0.0f);
    }

    /** processes the envelope, updating the currentEnvValue and then returning this value
    * @return currentEnvValue
    */
    float process()
    {
        if (!poweringOff)
        {
            float currentPhaseVal = phase.getNextValue() * 2.0;

            float risingVal = 1.0f - juce::jmin (1.0f, currentPhaseVal);
            risingVal = pow (risingVal, (3.0f + (accelRate * 6.0f)));

            float fallingVal = juce::jmax (1.0f, currentPhaseVal) - 1.0f;

            currentEnvValue = 1.0f + (-1.0f * (risingVal + fallingVal));
        }
        else
        {
            currentEnvValue -= volDelta;

            if (currentEnvValue <= 0)
            {
                currentEnvValue = 0;
                poweringOff = false;
            }
        }

        return currentEnvValue;
    }

    //================ accessors ================//

    float getCurrentValue() { return currentEnvValue; }

private:
    juce::SmoothedValue<float> phase;
    float powerUpTimeSeconds{ 1.5f };           // time in seconds for envelope to rise to max value
    float powerDownTimeSeconds{ 1.5f };         // time in seconds for envelope to fall from max value
    float volDelta{};                           // increment needed to linearly decrease volume from 1 to 0 over desired power down time
    float accelRate{ 0.5f };                    // rate at which the envelope rises exponentially - 0-1 value, 0 is min rate, 1 is max
    float sampleRate{};
    float currentEnvValue{};                    // current value of the envelope
    bool poweringOff{ false };
};