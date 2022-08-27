/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MechanicalModellingAudioProcessor::MechanicalModellingAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    parameters(*this, nullptr, "params", {
        // Global Params
        std::make_unique<juce::AudioParameterBool>("trigger", "On / Off", false),
        std::make_unique<juce::AudioParameterFloat>("masterGain", "Master Gain", 0.0f, 1.0f, 0.5f),
        std::make_unique<juce::AudioParameterFloat>("powerUpTime", "Power Up Time (s)", 0.5f, 10.0f, 3.0f),
        std::make_unique<juce::AudioParameterFloat>("powerDownTime", "Power Down Time (s)", 0.5f, 10.0f, 3.0f),
        std::make_unique<juce::AudioParameterFloat>("acceleration", "Rate of Acceleration", 0.0f, 1.0f, 0.5f),
        
        // Motor Params
        std::make_unique<juce::AudioParameterFloat>("motorGain", "Motor Level", 0.0f, 1.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("motorMaxSpeed", "Motor Max Speed (Hz)", 60.0f, 800.0f, 200.0f),
        std::make_unique<juce::AudioParameterFloat>("motorCasingSize", "Motor Casing Size", 0.0f, 1.0f, 0.75f),
        std::make_unique<juce::AudioParameterFloat>("motorRotorLevel", "Motor Rotor Level", 0.0f, 1.0f, 0.6f),
        std::make_unique<juce::AudioParameterFloat>("motorSparksLevel", "Motor Sparks Level", 0.0f, 1.0f, 0.2f),
        std::make_unique<juce::AudioParameterBool>("motorHum", "Motor Hum On / Off", false),

        // Fan Params
        std::make_unique<juce::AudioParameterFloat>("fanGain", "Fan Level", 0.0f, 1.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("fanRatio", "Fan Speed Ratio", 10.0f, 30.0f, 20.0f),
        std::make_unique<juce::AudioParameterFloat>("fanToneLevel", "Fan Tone Level", 0.0f, 1.0f, 0.75f),
        std::make_unique<juce::AudioParameterFloat>("fanNoiseLevel", "Fan Noise Level", 0.0f, 1.0f, 0.75f),
        std::make_unique<juce::AudioParameterFloat>("fanStereoWidth", "Fan Stereo Width", 0.0f, 1.0f, 0.0f),
        std::make_unique<juce::AudioParameterBool>("fanDoppler", "Fan Doppler On / Off", false),

        // Engine Params
        std::make_unique<juce::AudioParameterFloat>("engineGain", "Engine Level", 0.0f, 1.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("engineRevs", "Engine Revs (Speed)", 0.0f, 1.0f, 0.0f),
        //std::make_unique<juce::AudioParameterFloat>("engineAggression", "Engine Aggression", 0.0f, 1.0f, 0.33f),
        std::make_unique<juce::AudioParameterFloat>("engineWidth", "Engine Exhaust Width", 0.0f, 1.0f, 0.75f),
        std::make_unique<juce::AudioParameterFloat>("engineLength", "Engine Exhaust Length", 0.0f, 1.0f, 0.65f),
        std::make_unique<juce::AudioParameterFloat>("engineOT1", "Engine Overtone 1 Level", 0.0f, 1.0f, 0.5f),
        std::make_unique<juce::AudioParameterFloat>("engineOT2", "Engine Overtone 2 Level", 0.0f, 1.0f, 0.27),
        std::make_unique<juce::AudioParameterFloat>("engineOT3", "Engine Overtone 3 Level", 0.0f, 1.0f, 0.42f)
        })
{
    // Global Params
    gainParam = parameters.getRawParameterValue("masterGain");
    powerUpParam = parameters.getRawParameterValue("powerUpTime");
    powerDownParam = parameters.getRawParameterValue("powerDownTime");
    accelerationParam = parameters.getRawParameterValue("acceleration");
    triggerParam = parameters.getRawParameterValue("trigger");

    // Motor Params
    motorGainParam = parameters.getRawParameterValue("motorGain");
    motorMaxSpeedParam = parameters.getRawParameterValue("motorMaxSpeed");
    motorCasingSizeParam = parameters.getRawParameterValue("motorCasingSize");
    motorRotorParam = parameters.getRawParameterValue("motorRotorLevel");
    motorSparksParam = parameters.getRawParameterValue("motorSparksLevel");
    motorHumParam = parameters.getRawParameterValue("motorHum");

    // Fan Params
    fanGainParam = parameters.getRawParameterValue("fanGain");
    fanRatioParam = parameters.getRawParameterValue("fanRatio");
    fanToneParam = parameters.getRawParameterValue("fanToneLevel");
    fanNoiseParam = parameters.getRawParameterValue("fanNoiseLevel");
    fanStereoParam = parameters.getRawParameterValue("fanStereoWidth");
    fanDopplerParam = parameters.getRawParameterValue("fanDoppler");

    // Engine Params
    engineGainParam = parameters.getRawParameterValue("engineGain");
    engineRevsParam = parameters.getRawParameterValue("engineRevs");
    //engineAggressionParam = parameters.getRawParameterValue("engineAggression");
    engineWidthParam = parameters.getRawParameterValue("engineWidth");
    engineLengthParam = parameters.getRawParameterValue("engineLength");
    engineOT1Param = parameters.getRawParameterValue("engineOT1");
    engineOT2Param = parameters.getRawParameterValue("engineOT2");
    engineOT3Param = parameters.getRawParameterValue("engineOT3");
}

MechanicalModellingAudioProcessor::~MechanicalModellingAudioProcessor()
{
}

void MechanicalModellingAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    smoothedGain.reset(sampleRate, 0.1f);
    smoothedMaxSpeed.reset(sampleRate, 0.55f);

    engine.setSampleRate(sampleRate);
    fan.setSampleRate(sampleRate);
    motor.setSampleRate(sampleRate);
}

void MechanicalModellingAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    int numSamples = buffer.getNumSamples();

    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    // update smoothed value targets
    smoothedGain.setTargetValue(*gainParam);
    smoothedMaxSpeed.setTargetValue(*motorMaxSpeedParam);

    //=============================== DSP LOOP ===============================//
    for (int i = 0; i < numSamples; i++)
    {
        motorMaxSpeedVal = smoothedMaxSpeed.getNextValue();
        int resMode = *motorHumParam;
        motor.setMappedParams(*powerUpParam, *powerDownParam, *accelerationParam, *motorGainParam, motorMaxSpeedVal, *motorCasingSizeParam, *motorRotorParam, *motorSparksParam, *motorHumParam);

        fan.setMappedParams(*fanGainParam, motor.getCurrentSpeed() / (*fanRatioParam), * fanToneParam, * fanNoiseParam, * fanStereoParam, *fanDopplerParam);

        if (*triggerParam && !isPlaying)
        {
            isPlaying = true;
            motor.powerOn();
        }
        else if (isPlaying && !(*triggerParam))
        {
            isPlaying = false;
            motor.powerOff();
        }

        float motorOut = motor.process();
        fan.process();

        float revsVal = *engineRevsParam;
        if (*triggerParam == false)
            revsVal = 0;

        float engineSpeedVal = (0.10 + (0.25 * motor.getEnvelope())) * (1.0f + (revsVal * 1.37f));
        engine.setMappedParams(*engineGainParam, engineSpeedVal, 0.5f, *engineWidthParam, *engineLengthParam, *engineOT1Param, *engineOT2Param, *engineOT3Param);
        float engineOut = engine.process();

        gainVal = smoothedGain.getNextValue();

        leftChannel[i] = gainVal * (engineOut + motorOut + (motor.getEnvelope() * fan.getLeftSample()));
        rightChannel[i] = gainVal * (engineOut + motorOut + (motor.getEnvelope() * fan.getRightSample()));
    }
}
//==============================================================================
const juce::String MechanicalModellingAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MechanicalModellingAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MechanicalModellingAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MechanicalModellingAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MechanicalModellingAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MechanicalModellingAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MechanicalModellingAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MechanicalModellingAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MechanicalModellingAudioProcessor::getProgramName (int index)
{
    return {};
}

void MechanicalModellingAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//=============================================================================

void MechanicalModellingAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MechanicalModellingAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//==============================================================================
bool MechanicalModellingAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MechanicalModellingAudioProcessor::createEditor()
{
    //return new MechanicalModellingAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void MechanicalModellingAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // getStateInformation
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MechanicalModellingAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // setStateInformation
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MechanicalModellingAudioProcessor();
}
