#pragma once

#include <JuceHeader.h>
#include "GranSynth.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent, public juce::Slider::Listener, public juce::Button::Listener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    //==============================================================================
    void buttonClicked(juce::Button* button) override;
    void openFile();
    bool loadAudioFile(const juce::File& file);
    void sliderValueChanged(juce::Slider *slider) override; //@ need define
    void initializeLabel(juce::Label& label, const juce::String& text, juce::Slider& slider);
    

private:
    //==============================================================================
    // Your private member variables go here...
    
    int grainSize, grainOverlap, grainSpacing;
    float pitchShiftFactor;
    bool currentlyPlaying = false; //the flag used to check if the audio is currently
    juce::AudioBuffer<float> fileBuffer;
    int currentSampleRate;
    int samplesPerBlock;
    
    std::unique_ptr<GranSynth> granSynth;
    std::unique_ptr<juce::FileChooser> chooser;
    
    //GUI Components
    juce::Slider grainSizeSlider, grainOverlapSlider, grainSpacingSlider, pitchShiftSlider;
    juce::TextButton openButton, playButton, stopButton;
    juce::Label grainSizeLabel, grainOverlapLabel, grainSpacingLabel, pitchShiftLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
