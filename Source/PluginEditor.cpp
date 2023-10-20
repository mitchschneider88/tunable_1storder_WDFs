/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Tunable1stOrderWDFsAudioProcessorEditor::Tunable1stOrderWDFsAudioProcessorEditor (Tunable1stOrderWDFsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    addAndMakeVisible(frequency);
    std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "fcID", frequency);
    
    addAndMakeVisible(gain);
    std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "gainID", gain);
    
    addAndMakeVisible(type);
    
    type.addItem("LPF", 1);
    type.addItem("HPF", 2);
    type.addItem("Lo Shelf", 3);
    type.addItem("Hi Shelf", 4);
}

Tunable1stOrderWDFsAudioProcessorEditor::~Tunable1stOrderWDFsAudioProcessorEditor()
{
}

//==============================================================================
void Tunable1stOrderWDFsAudioProcessorEditor::paint (juce::Graphics& g)
{

}

void Tunable1stOrderWDFsAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
