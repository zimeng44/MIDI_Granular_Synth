/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GranSynthZiAudioProcessorEditor::GranSynthZiAudioProcessorEditor (GranSynthZiAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 200);
    
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open File");
    openButton.addListener(this);
    openButton.setEnabled(true);
    
    addAndMakeVisible(grainSizeSlider);
    grainSizeSlider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    grainSizeSlider.setTextBoxIsEditable(true);
    grainSizeSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 120, 20);
    grainSizeSlider.setNormalisableRange(juce::NormalisableRange<double>(5.0, 100.0, 1.0));
    grainSizeSlider.setValue (audioProcessor.getGrainSize());
    grainSizeSlider.addListener (this);
    grainSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.getAPVTS(), "grainSize", grainSizeSlider);
    
    addAndMakeVisible(grainOverlapSlider);
    grainOverlapSlider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    grainOverlapSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 120, 20);
    grainOverlapSlider.setNormalisableRange(juce::NormalisableRange<double>(0, grainSizeSlider.getValue()-1, 1.0));
    grainOverlapSlider.setValue (audioProcessor.getGrainOverlap());
    grainOverlapSlider.addListener (this);
    grainOverlapAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.getAPVTS(), "grainOverlap", grainOverlapSlider);
    
    addAndMakeVisible(grainSpacingSlider);
    grainSpacingSlider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    grainSpacingSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 120, 20);
    grainSpacingSlider.setNormalisableRange(juce::NormalisableRange<double>(1.0, 200.0, 1));
    grainSpacingSlider.setValue (audioProcessor.getGrainSpacing());
    grainSpacingSlider.addListener (this);
    grainSpacingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.getAPVTS(), "grainSpacing", grainSpacingSlider);
    
    addAndMakeVisible(grainSizeLabel);
    grainSizeLabel.setText("Grain Size (ms)", juce::dontSendNotification);
    grainSizeLabel.attachToComponent(&grainSizeSlider, false);
    
    addAndMakeVisible(grainOverlapLabel);
    grainOverlapLabel.setText("Grain Overlap (ms)", juce::dontSendNotification);
    grainOverlapLabel.attachToComponent(&grainOverlapSlider, false);
    
    addAndMakeVisible(grainSpacingLabel);
    grainSpacingLabel.setText("Grain Spacing (ms)", juce::dontSendNotification);
    grainSpacingLabel.attachToComponent(&grainSpacingSlider, false);
    
    addAndMakeVisible(explainLabel);
    explainLabel.setText("Original pitch is mapped to A3", juce::dontSendNotification);
    
    addAndMakeVisible (midiKeyboardComponent);
    midiKeyboardComponent.setMidiChannel (2);
    midiKeyboardComponent.setVelocity(0.6f, true);
    midiKeyboardState.addListener (&audioProcessor.getMidiMessageCollector());
}

GranSynthZiAudioProcessorEditor::~GranSynthZiAudioProcessorEditor()
{
    midiKeyboardState.removeListener (&audioProcessor.getMidiMessageCollector());
}

//==============================================================================
void GranSynthZiAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
//    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void GranSynthZiAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto area = getLocalBounds();
    int sliderWidth = 100;
    int sliderHeight = 70;
//    int labelHeight = 20;
    
    openButton.setBounds(area.getWidth()*0.025, area.getHeight()*0.2, 90, 40);

    grainSizeSlider.setBounds(area.getWidth()*0.03+140, area.getHeight()*0.15, sliderWidth, sliderHeight);
    
    grainOverlapSlider.setBounds(area.getWidth()*0.03+250, area.getHeight()*0.15, sliderWidth, sliderHeight);
    grainSpacingSlider.setBounds(area.getWidth()*0.03+360, area.getHeight()*0.15, sliderWidth, sliderHeight);
    
    explainLabel.setBounds(area.getWidth()*0.65, area.getHeight()*0.15, 250, 30);
    explainLabel.setFont(juce::Font(20));
    
    midiKeyboardComponent.setBounds (0, area.getHeight()*0.6, area.getWidth(), area.getHeight()*0.4);
}

void GranSynthZiAudioProcessorEditor::buttonClicked(juce::Button* button) {
    
    if (button == &openButton){
        // Open a file chooser dialog to select an audio file
        chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...",
                                                                 juce::File{},
                                                                     "*.wav");
        
        auto chooserFlags = juce::FileBrowserComponent::openMode
                                    | juce::FileBrowserComponent::canSelectFiles;
        
        chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)       // [8]
        {
            audioProcessor.loadFile(fc.getResult());
        });
    }
    
}

void GranSynthZiAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    if (slider == &grainSizeSlider)
    {
        audioProcessor.setGrainSize(grainSizeSlider.getValue() / 1000 * audioProcessor.getSampleRate());
        grainOverlapSlider.setNormalisableRange(juce::NormalisableRange<double>(0, grainSizeSlider.getValue()-1, 1.0));
    }
    else if(slider == &grainOverlapSlider)
    {
        audioProcessor.setGrainOverlap(grainOverlapSlider.getValue() / 1000 * audioProcessor.getSampleRate());
    }
    else if(slider == &grainSpacingSlider)
    {
        audioProcessor.setGrainSpacing(grainSpacingSlider.getValue() / 1000 * audioProcessor.getSampleRate());
    }
    
}
