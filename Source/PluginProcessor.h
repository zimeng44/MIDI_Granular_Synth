/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GranSynth.h"

//==============================================================================
/**
*/
class GranSynthZiAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    GranSynthZiAudioProcessor();
    ~GranSynthZiAudioProcessor() override;

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
    
    //==============================================================================
    
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    juce::ParameterID grainSizeId = juce::ParameterID("grainSize", 1);
    juce::ParameterID grainOverlapId = juce::ParameterID("grainOverlap", 1);
    juce::ParameterID grainSpacingId = juce::ParameterID("grainSpacing", 1);
    
    int getGrainSize() const { return grainSize; }
    void setGrainSize(int newGrainSize);
    
    int getGrainOverlap() const { return grainOverlap; }
    void setGrainOverlap(int newGrainOverlap);
    
    int getGrainSpacing() const { return grainSpacing; }
    void setGrainSpacing(int newGrainSpacing) ;
    
    int getAudioSize() const { return audioBuffer.getNumSamples(); }
    
    juce::MidiMessageCollector& getMidiMessageCollector() noexcept { return midiMessageCollector; }
    
    void loadFile(const juce::File& file);
    
    void noteOn(int grainSize, int grainOverlap, int grainSpacing, double frequency, float velocity);
    void noteOff(double frequency);

private:
    
    juce::MidiMessageCollector midiMessageCollector;
    juce::AudioBuffer<float> audioBuffer;
    std::vector<GranSynth> granSynthBank;
    int grainSize;
    int grainOverlap;
    int grainSpacing;
    int fileIndex = 0;
    
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GranSynthZiAudioProcessor)
};
