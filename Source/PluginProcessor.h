/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "jr_Engine.h"
#include "ElectricMotorDC.h"
#include "jr_SimpleFan.h"

//==============================================================================
/**
*/
class MechanicalModellingAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    MechanicalModellingAudioProcessor();
    ~MechanicalModellingAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    
    ElectricMotorDC motor;
    FanPropeller fan;
    Engine engine;


    bool isPlaying{ false };            // true if start trigger/switch is on
    float gainVal{};                    // current master gain value
    float motorMaxSpeedVal{};           // current motor max speed value
    juce::SmoothedValue<float> smoothedGain; // smoothed gain value
    juce::SmoothedValue<float> smoothedMaxSpeed; // smoothed motor max speed value

    juce::AudioProcessorValueTreeState parameters;

    //===================== Global Parameters ======================//

    std::atomic<float>* gainParam;
    std::atomic<float>* powerUpParam;
    std::atomic<float>* powerDownParam;
    std::atomic<float>* accelerationParam;
    std::atomic<float>* triggerParam;
    
    //===================== Motor Parameters ======================//

    std::atomic<float>* motorGainParam;
    std::atomic<float>* motorMaxSpeedParam;
    std::atomic<float>* motorCasingSizeParam;
    std::atomic<float>* motorRotorParam;
    std::atomic<float>* motorSparksParam;
    std::atomic<float>* motorHumParam;

    //===================== Fan Parameters ======================//

    std::atomic<float>* fanGainParam;
    std::atomic<float>* fanRatioParam;
    std::atomic<float>* fanToneParam;
    std::atomic<float>* fanNoiseParam;
    std::atomic<float>* fanStereoParam;
    std::atomic<float>* fanDopplerParam;

    //===================== Engine Parameters ======================//

    std::atomic<float>* engineGainParam;
    std::atomic<float>* engineRevsParam;
    //std::atomic<float>* engineAggressionParam;
    std::atomic<float>* engineWidthParam;
    std::atomic<float>* engineLengthParam;
    std::atomic<float>* engineOT1Param;
    std::atomic<float>* engineOT2Param;
    std::atomic<float>* engineOT3Param;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MechanicalModellingAudioProcessor)
};
