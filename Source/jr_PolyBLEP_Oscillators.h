#pragma once
	
#include <cmath>		// used for sin() and fabs() and std::fmod()

namespace jr {
	
	/** An Oscillator that can be set to either Sine, Sawtooth, Square, or Triangle mode. 
	Oscillator starts muted so use setMuted() to unmute, and use setSampleRate() before use 
	(static member so only needs to be set once for all instances)
	* Derived from Martin Finke's Oscillator class from this tutorial: http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
	*/
	class Oscillator
	{
	public:

		enum class OscillatorMode {
			SINE = 0,
			SAW,
			SQUARE,
			TRIANGLE
		};

		//==================== Constructors/Destructos =======================//

		Oscillator() : oscMode (OscillatorMode::SINE), PI (2 * acos(0.0)), twoPI (2 * PI),
			isMuted (true), frequency (440.0), phase (0.0), phaseDelta (0.0) {}

		~Oscillator() {}

		//====================== Mutator Functions ===========================//

		/** Sets the sample rate of all instances of Oscillator
		* @param sr - sample rate, Hz
		*/
		void setSampleRate (double sr);

		/** Sets the mode of the Oscillator to either: "SINE/SQUARE/SAW/TRIANGLE"
		* @param mode - type OscillatorMode e.g. SINE
		*/
		void setMode (OscillatorMode mode);

		/**
		* Sets the frequency of the Oscillator
		* @param freq - frequency, Hz
		*/
		void setFrequency (double freq);

		/**
		* Mutes or unmutes the Oscillator
		* @param muted - true to mute oscillator, false to unmute
		*/
		inline void setMuted (bool muted) { isMuted = muted; }

		/**
		* Resets the Oscillator by setting the phase to 0
		* @return
		*/
		inline void reset() { phase = 0.0f; }

		/** Sets the phase shift amount of the oscillator, used to stagger phase of multiple oscillators
		* @param shiftAmount - phase shift amount (0-0.5)
		*/
		inline void setPhaseShift (double shiftAmount) { if (shiftAmount <= 0 && shiftAmount <= 0.5) phaseShift = shiftAmount; }

		//======================= Accessor Functions =====================//

		/**
		* Processes the Oscillator and returns the next sample value
		* @return sampleOut
		*/
		virtual float processSingleSample();

		/**
		* returns the next sample value for a naive waveform (unprotected from aliasing) according to a certain Oscillator Mode
		* @param mode - mode corresponding to waveform type i.e. SINE (or SQUARE/SAW/TRIANGLE)
		* @return value - next sample value for naive waveform
		*/
		float naiveWaveformForMode (OscillatorMode mode);

		/**
		* Processes a block of samples
		* @param buffer - buffer to read samples into
		* @param numSamples - buffer block size in samples
		*/
		void processNextBlock (float* buffer, int numSamples);

	protected:

		//============== params ===============//

		static double sampleRate;			// Hz
		OscillatorMode oscMode;				// mode determining waveform type
		double frequency;					// Hz
		double phase;
		double phaseDelta;
		bool isMuted;						// true when Oscillator is muted
		double phaseShift{};				// phase shift amount, used to stagger phase of multiple instances (0-0.5)

		//================= constants =============//

		const double PI;					// mathematical constant pi
		const double twoPI;					// two * mathematical constant pi

		//================= functions =============//

		/**
		* updates the phaseDelta value, called each time sampleRate or frequency is updated
		* @return
		*/
		void updatePhaseDelta()
		{
			phaseDelta = frequency / sampleRate;
		}
	};

	/** An Oscillator that uses the polyBLEP algorithm for anti-aliasing
	* Derived from Martin Finke's Oscillator class from this tutorial: http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
	*/
	class polyblepOscillator : public Oscillator
	{
	public:

		//======================= Accessor Functions =====================//

		/**
		* Processes the Oscillator and returns the next sample value
		* @return sampleOut
		*/
		float processSingleSample() override;

	private:
		//============= parameters ============//

		float lastOutput;		// last sample value to be output, used for triangle wave BLEP

		//============== functions ============//

		/**
		* Checks the current phase and returns a sample adjustment amount based on polyBLEP antialiasing algorithm
		* @param t - phase (0-1)
		* @return adjustment - sample adjustment amount
		*/
		double polyBLEP (double t);

	};
	
}

