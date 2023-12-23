/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GranSynth.h"

//==============================================================================
/**
*/
class GranSynthZiAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Button::Listener, public juce::Slider::Listener
{
public:
    GranSynthZiAudioProcessorEditor (GranSynthZiAudioProcessor&);
    ~GranSynthZiAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged (juce::Slider* slider) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    GranSynthZiAudioProcessor& audioProcessor;
    juce::TextButton openButton;
    std::unique_ptr<juce::FileChooser> chooser;
    
    juce::Slider grainSizeSlider;
    juce::Label grainSizeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> grainSizeAttachment;
    
    juce::Slider grainOverlapSlider;
    juce::Label grainOverlapLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> grainOverlapAttachment;
    
    juce::Slider grainSpacingSlider;
    juce::Label grainSpacingLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> grainSpacingAttachment;
    
    juce::Label explainLabel;
    
    juce::MidiKeyboardState midiKeyboardState;
    juce::MidiKeyboardComponent midiKeyboardComponent {
        midiKeyboardState, juce::MidiKeyboardComponent::horizontalKeyboard };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GranSynthZiAudioProcessorEditor)
};
