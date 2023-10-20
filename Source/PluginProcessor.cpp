/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Tunable1stOrderWDFsAudioProcessor::Tunable1stOrderWDFsAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), treeState(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    treeState.addParameterListener("fcID", this);
    treeState.addParameterListener("gainID", this);
    treeState.addParameterListener("typeID", this);
}

Tunable1stOrderWDFsAudioProcessor::~Tunable1stOrderWDFsAudioProcessor()
{
    treeState.removeParameterListener("fcID", this);
    treeState.removeParameterListener("gainID", this);
    treeState.removeParameterListener("typeID", this);
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout Tunable1stOrderWDFsAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.reserve(3);
    
    juce::NormalisableRange<float> fcRange (20.f, 20000.f, 1.f, 0.3f, false);
    params.push_back(std::make_unique<juce::AudioParameterFloat>("fcID", "frequency", fcRange, 1000.f));
    
    juce::NormalisableRange<float> gainRange (-10, 10, 0.1, 1.f, false);
    params.push_back(std::make_unique<juce::AudioParameterFloat>("gainID", "gain", gainRange, 0.f));
    
    params.push_back(std::make_unique<juce::AudioParameterInt>("typeID", "type", 1, 4, 1));
    
    return { params.begin(), params.end() };
}

void Tunable1stOrderWDFsAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    if (parameterID == "fcID")
    {
        params.fc = newValue;
        DBG("frequency is" + std::to_string(newValue));
        
        leftLPF.setParameters(params);
        rightLPF.setParameters(params);
        
        leftLPF.reset(getSampleRate());
        rightLPF.reset(getSampleRate());
        
        leftHPF.setParameters(params);
        rightHPF.setParameters(params);
        
        leftHPF.reset(getSampleRate());
        rightHPF.reset(getSampleRate());
    }
    
}

const juce::String Tunable1stOrderWDFsAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Tunable1stOrderWDFsAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Tunable1stOrderWDFsAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Tunable1stOrderWDFsAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Tunable1stOrderWDFsAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Tunable1stOrderWDFsAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Tunable1stOrderWDFsAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Tunable1stOrderWDFsAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Tunable1stOrderWDFsAudioProcessor::getProgramName (int index)
{
    return {};
}

void Tunable1stOrderWDFsAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Tunable1stOrderWDFsAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    params.fc = treeState.getRawParameterValue("fcID")->load();
    params.frequencyWarping = true;
    
    
    leftLPF.setParameters(params);
    rightLPF.setParameters(params);
    
    leftLPF.createWDF();
    rightLPF.createWDF();
    
    leftLPF.reset(sampleRate);
    rightLPF.reset(sampleRate);
    
    leftHPF.setParameters(params);
    rightHPF.setParameters(params);
    
    leftHPF.createWDF();
    rightHPF.createWDF();
    
    leftHPF.reset(sampleRate);
    rightHPF.reset(sampleRate);
    
}

void Tunable1stOrderWDFsAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Tunable1stOrderWDFsAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void Tunable1stOrderWDFsAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    int numSamples = buffer.getNumSamples();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    for (int sample = 0; sample < numSamples; ++sample)
    {
        auto* xnL = buffer.getReadPointer(0);
        auto* ynL = buffer.getWritePointer(0);
        
        auto* xnR = buffer.getReadPointer(1);
        auto* ynR = buffer.getWritePointer(1);
        
        if (treeState.getRawParameterValue("typeID")->load() == 1)
        {
            ynL[sample] = leftLPF.processAudioSample(xnL[sample]);
            ynR[sample] = rightLPF.processAudioSample(xnR[sample]);
        }
        else if (treeState.getRawParameterValue("typeID")->load() == 2)
        {
            ynL[sample] = leftHPF.processAudioSample(xnL[sample]);
            ynR[sample] = rightHPF.processAudioSample(xnR[sample]);
        }
        
    }
}

//==============================================================================
bool Tunable1stOrderWDFsAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Tunable1stOrderWDFsAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void Tunable1stOrderWDFsAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Tunable1stOrderWDFsAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Tunable1stOrderWDFsAudioProcessor();
}
