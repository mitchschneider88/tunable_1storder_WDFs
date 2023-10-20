/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class Tunable1stOrderWDFsAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Tunable1stOrderWDFsAudioProcessorEditor (Tunable1stOrderWDFsAudioProcessor&);
    ~Tunable1stOrderWDFsAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Tunable1stOrderWDFsAudioProcessor& audioProcessor;
    
    juce::Slider frequency;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> frequencyAttachment;
        
    juce::ComboBox type;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;
    
    juce::Slider gain;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Tunable1stOrderWDFsAudioProcessorEditor)
};
