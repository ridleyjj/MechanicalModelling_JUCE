#pragma once

/** A class that represents the physical model of an electric brush used in an electric DC motor that produces noise each time it makes a contact
*/
class Brush
{
public:

	/** Sets sample rate
	* @param sr - sample rate, Hz
	*/
	void setSampleRate (float sr) { sampleRate = sr; }

	/** Sets the cutoff frequency of the band-pass filter
	* @param freq - cutoff frequency, Hz
	*/
	void setFilterFrequency (float freq) { filterFreq = freq; }

	/** Sets the level of the brush by controlling the volume of the noise (0-1)
	* @param levelIn - volume level (0-1)
	*/
	void setLevel (float levelIn) { level = levelIn; }

	/** returns the next sample value for the brush
	* @return sampleOut
	*/
	float process()
	{
		float whiteNoise = 2.0 * (random.nextFloat() - 0.5);	// white noise val between -1 and 1

		bpFilter.setCoefficients (juce::IIRCoefficients::makeBandPass (sampleRate, filterFreq, 1.0));

		return bpFilter.processSingleSampleRaw (whiteNoise) * level;
	}

private:
	float sampleRate;
	juce::Random random;
	juce::IIRFilter bpFilter;
	float filterFreq{ 4000.0 };
	float level{};
};

/** A class that is a physical model of the rotor component of an electrical DC motor, consisting of a brush causing clicks, and the DC of the rotor itself, all enveloped by the 4th power of the driving phasor
* use setSampleRate() before use, and setBrushLevel() and setRotorLevel() to control the volume of the two components
*/
class Rotor
{
public:

	/** Sets the sample rate
	* @param sr - sample rate, Hz
	*/
	void setSampleRate (float sr) { brush.setSampleRate (sr); }

	/** Sets the output volume of the brush component
	* @param brushLevel - volume value (0-1)
	*/
	void setBrushLevel (float brushLevel) { brush.setLevel (brushLevel); }

	/** Sets the output volume of the rotor component's DC signal
	* @param rotorLevelIn - volume value (0-1)
	*/
	void setRotorLevel (float rotorLevelIn) { rotorLevel = rotorLevelIn; }

	/** Sets the cutoff frequency value for the Band Pass Filter in the brush component
	* @param freq - cutoff frequency, Hz
	*/
	void setBrushFreqeuncy (float freq) { brush.setFilterFrequency (freq); }

	/** Returns the next sample value of the rotor component, synched to the input value of the driving phasor signal
	* @param phasorVal - current sample value for the driving phasor signal that controls the motor system
	* @return output - next sample value out for the rotor component
	*/
	float process (float phasorVal)
	{
		float brushOut = brush.process();
		float output = brushOut + rotorLevel;

		currentEnvVal = envelopeVal (phasorVal);

		return output * currentEnvVal;
	}

	//=========== accessors =============//

	float getRotorLevel() { return rotorLevel; }

	float getCurrentEnvVal() { return currentEnvVal; }

private:
	Brush brush;			// brush component, that makes noise each time it comes into contact with material whilst the motor spins
	float rotorLevel{};		// volume level of the rotor components DC, used as a constant signal value
	float currentEnvVal{};	// current value of the envelope

private:

	/** Returns the 4th power of the phasor signal in, creating the desired envelope synced to the phasor
	* @return currentEnvelopeValue
	*/
	float envelopeVal (float phasorValIn)
	{
		return pow (phasorValIn, 4);
	}

};