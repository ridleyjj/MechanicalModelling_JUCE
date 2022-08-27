/*
  ==============================================================================

    jr_PolyBLEP_Oscillators.cpp
    Created: 11 Apr 2022 2:14:20pm
    Author:  ridle

  ==============================================================================
*/

#include "jr_PolyBLEP_Oscillators.h"

namespace jr
{

	//================================ Oscillator Class ===================================//

	//initialise static member outside of class to a default value
	double Oscillator::sampleRate{ 44100 };

	//====================== Mutator Functions ===========================//

	void Oscillator::setSampleRate (double sr)
	{
		if (sr > 0)
		{
			sampleRate = sr;

			updatePhaseDelta();
		}
	}

	void Oscillator::setMode (OscillatorMode mode)
	{
		oscMode = mode;
	}

	void Oscillator::setFrequency (double freq)
	{
		if (freq > 0)
		{
			frequency = freq;

			updatePhaseDelta();
		}
	}

	//======================= Accessor Functions =====================//

	float Oscillator::processSingleSample()
	{

		float sampleOut{};

		if (!isMuted)
		{
			sampleOut = naiveWaveformForMode (oscMode);
		}

		phase += phaseDelta;

		if ((phase + phaseShift) >= 1)
			phase -= 1;

		return sampleOut;
	}

	float Oscillator::naiveWaveformForMode (OscillatorMode mode)
	{
		float value{};

		switch (mode)
		{
		default:
			// sine as default and case OscMode_Sine:
			value = sin (twoPI * (phase + phaseShift));
			break;
		case OscillatorMode::SAW:
			value = 2 * ((phase + phaseShift) - 0.5);
			break;
		case OscillatorMode::SQUARE:
			value = 1;
			if ((phase + phaseShift) > 0.5)
				value = -1;
			break;
		case OscillatorMode::TRIANGLE:
			value = 4 * fabs ((phase + phaseShift) - 0.5);
			break;
		}

		return value;
	}

	void Oscillator::processNextBlock (float* buffer, int numSamples)
	{
		for (size_t i = 0; i < numSamples; i++)
		{
			buffer[i] = processSingleSample();
		}
	}

	//============================ polyblepOscillator Class ===============================//

	//======================= Accessor Functions =====================//

	float polyblepOscillator::processSingleSample()
	{
		float sampleOut{};

		if (oscMode == OscillatorMode::SINE)
		{
			sampleOut = naiveWaveformForMode (oscMode);
		}
		else if (oscMode == OscillatorMode::SAW)
		{
			sampleOut = naiveWaveformForMode (oscMode);
			sampleOut -= polyBLEP ((phase + phaseShift));
		}
		else
		{
			// square wave
			sampleOut = naiveWaveformForMode (OscillatorMode::SQUARE);
			sampleOut += polyBLEP ((phase + phaseShift));
			sampleOut -= polyBLEP (std::fmod ((phase + phaseShift) + 0.5, 1.0));			// fmod() clamps phase between 0-1 whilst offsetting value by 0.5

			if (oscMode == OscillatorMode::TRIANGLE)
			{
				sampleOut = phaseDelta * sampleOut + (1 - phaseDelta) * lastOutput;		// leaky integrator: multiplying by (1 - phaseDelta) instead of 1 to stop output from accumulating
				lastOutput = sampleOut;
			}
		}

		phase += phaseDelta;
		if ((phase + phaseShift) >= 1)
			phase -= 1;

		return sampleOut;
	}

	double polyblepOscillator::polyBLEP (double t)
	{
		if (t < phaseDelta)
		{
			t /= phaseDelta;
			return (t + t - (t * t) - 1.0);
		}
		else if (t > (1.0 - phaseDelta))
		{
			t = (t - 1.0) / phaseDelta;
			return ((t * t) + t + t + 1.0);
		}
		else	return 0.0;
	}
};