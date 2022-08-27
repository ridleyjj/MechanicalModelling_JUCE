/*
  ==============================================================================

    ElectricMotorDC.h
    Created: 20 Apr 2022 1:43:16pm
    Author:  ridle

  ==============================================================================
*/

#pragma once
#include "Motor_Envelope.h"                 // used for Motor Envelope
#include "Rotor.h"                          // used for Rotor / Brush
#include "Stator.h"                         // used for Stator
#include "FM_Resonator.h"                   // used for FM resonance
#include "jr_PolyBLEP_Oscillators.h"        // used for driving phasor (Oscillator set to SAW mode)

class ElectricMotorDC
{
public:

    ElectricMotorDC() 
    { 
        phasor.setMuted (false);
        phasor.setMode (jr::Oscillator::OscillatorMode::SAW);
    }

    //======================== mutators ===========================//

    /** Sets the sample rate
    * @param sr - sample rate, Hz
    */
    void setSampleRate (float sr)
    {
        phasor.setSampleRate (sr);
        rotor.setSampleRate (sr);
        stator.setSampleRate (sr);
        resonator.setSampleRate (sr);
        envelope.setSampleRate (sr);
        smoothedGain.reset (sr, 0.01);
    }

    /** Sets the parameters of the motor via a simlpified set of parameters that the others are mapped to
    * @param powerUpTime - power up time in seconds 
    * @param powerDownTime - power down time in seconds 
    * @param accelRate - acceleration rate of motor
    * @param gainIn - gain (0-1)
    * @param maxSpeedIn - max speed (Hz)
    * @param casingSizeIn - motor casing size (0-1)
    * @param rotorIn - rotor level (0-1)
    * @param sparksIn - sparks level (0-1)
    * @param humIn - true if rotor DC signal is sent to resonator pre-envelope
    */
    void setMappedParams (float powerUpTimeIn,float powerDownTimeIn, float accelRate, float gainIn, float maxSpeedIn, float casingSizeIn, float rotorIn, float sparksIn, bool humIn)
    {
        float statorVal = 0.4 + (0.6 * casingSizeIn);

        setParams (gainIn, powerUpTimeIn, powerDownTimeIn, accelRate, statorVal, sparksIn, rotorIn, casingSizeIn, maxSpeedIn, 1.0f, 2800.0f, humIn);
    }

    void setParams (float gainIn, float powerUpTime, float powerDownTime, float accRate, float statorLevel, float brushLevel, float rotorLevel, float resAmount, float maxSpeedIn, float jitterAmount, float brushFreq=4000, int resModeIn=0)
    {
        smoothedGain.setTargetValue (gainIn);
        envelope.setPowerUpTime (powerUpTime);
        envelope.setPowerDownTime (powerDownTime);
        envelope.setAccelRate (accRate);
        rotor.setBrushLevel (brushLevel);
        rotor.setRotorLevel (rotorLevel);
        rotor.setBrushFreqeuncy (brushFreq);
        stator.setStatorLevel (statorLevel);
        resonator.setResonanceAmount (resAmount);
        setMaxSpeed (maxSpeedIn);
        setPhasorJitter (jitterAmount);
        setResMode (resModeIn);
    }

    /** Set the amount of jitter applied to the driving phasor's frequency (0-1)
    * @param jitterAmount - volume of jitter noise (0-1)
    */
    void setPhasorJitter (float jitterAmount) { phasorJitterAmount = jitterAmount; }

    /** Sets the maximum speed of the motor, reflected in the maximum frequency the motor will reach
    * @param speed - maximum frequency of the motor, Hz
    */
    void setMaxSpeed (float speed) { maxSpeed = speed; }

    /** Sets the resonation mode via index
    * @param mode - index dictating mode, 0 for resonating after the rotor envelope applied, 1 for resonating the unenveloped rotor signal
    */
    void setResMode (int mode) { if (mode == 0 || mode == 1)  resMode = mode; }

    //=============================================================================//

    /** Turns motor ON
    * @return
    */
    void powerOn() { envelope.powerOn(); }

    /** Turns motor OFF
    * @return
    */
    void powerOff() { envelope.powerOff(); }

    /** Returns the next sample value for the motor
    * @return sampleOut - next sample value
    */
    float process()
    {
        float envelopeVal = envelope.process();
        float jitter = (2.0f * (random.nextFloat() - 1.0f)) * phasorJitterAmount;
        currentFreq = (envelopeVal * maxSpeed) + jitter;
        phasor.setFrequency (currentFreq);
        float phasorOut = (phasor.processSingleSample() + 1.0f) / 2.0f;
        float statorOut = stator.process (currentFreq);
        float rotorOut = rotor.process (phasorOut);
        float resonatorOut{};
        switch (resMode)
        {
        default:
            resonatorOut = resonator.process (rotor.getRotorLevel() * rotor.getCurrentEnvVal(), phasorOut);
            break;
        case 1:
            resonatorOut = resonator.process (rotor.getRotorLevel(), phasorOut);
            break;
        }

        float sampleOut = (statorOut + rotorOut + resonatorOut) * envelopeVal;

        gainVal = smoothedGain.getNextValue();
        return sampleOut * gainVal;
    }

    float getEnvelope() { return envelope.getCurrentValue(); }

    float getCurrentSpeed() { return currentFreq; }

private:
    float gainVal{};                           // master gain for motor (0-1)
    juce::SmoothedValue<float> smoothedGain;   // smoothed gain
    Rotor rotor;
    Stator stator;
    MotorFMResonator resonator;
    MotorEnvelope envelope;
    jr::polyblepOscillator phasor;

    juce::Random random;                    // random number generator used to generate white noise
    float phasorJitterAmount{};             // amount of jitter to add to the frequency of the driving phasor (0-1)
    int resMode{};                          // 0 or 1 value indicating whether rotor signal is sent to resonator before or after its volume envelope

    float maxSpeed{ 80.0f };                // max speed of the motor, controls the maximum frequency the motor will spin at
    float currentFreq{};                    // stores the current frequency value of the driving phasor
};