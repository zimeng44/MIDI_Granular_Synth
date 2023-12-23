/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "GranSynth.h"

//==============================================================================
GranSynthZiAudioProcessor::GranSynthZiAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
}

GranSynthZiAudioProcessor::~GranSynthZiAudioProcessor()
{

}

//==============================================================================
const juce::String GranSynthZiAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GranSynthZiAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GranSynthZiAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GranSynthZiAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GranSynthZiAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GranSynthZiAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GranSynthZiAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GranSynthZiAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GranSynthZiAudioProcessor::getProgramName (int index)
{
    return {};
}

void GranSynthZiAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void GranSynthZiAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    midiMessageCollector.reset (sampleRate);
}

void GranSynthZiAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GranSynthZiAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void GranSynthZiAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    grainSize = *apvts.getRawParameterValue("grainSize") / 1000 * getSampleRate();
    grainOverlap = *apvts.getRawParameterValue("grainOverlap") / 1000 * getSampleRate();
    grainSpacing = *apvts.getRawParameterValue("grainSpacing") / 1000 * getSampleRate();
    
    midiMessageCollector.removeNextBlockOfMessages (midiMessages, buffer.getNumSamples());

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    for(const auto metaData: midiMessages){
        auto message = metaData.getMessage();
        
        if (message.isNoteOn()){
            
            double newFrequency = (double) message.getMidiNoteInHertz(message.getNoteNumber());
            float newVel = message.getFloatVelocity();
            noteOn(grainSize, grainOverlap, grainSpacing, newFrequency, newVel);
            
        } else if (message.isNoteOff()){
            
            double frequency = (double) message.getMidiNoteInHertz(message.getNoteNumber());
            noteOff(frequency);
            
        }
    }
    
    auto it = granSynthBank.begin();
    
    for(auto& granSynthTone : granSynthBank){
                    
        if (granSynthTone.shouldBeRemoved()) {
            granSynthBank.erase(it);
            break;
        }
        
        it++;

    }
    
    for(auto& granSynthTone : granSynthBank){
        granSynthTone.processBlock(buffer);
//        granSynthTone.updateTote();
    }

}

//==============================================================================
bool GranSynthZiAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GranSynthZiAudioProcessor::createEditor()
{
    return new GranSynthZiAudioProcessorEditor (*this);
}

//==============================================================================
void GranSynthZiAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    auto state = apvts.copyState();
        std::unique_ptr<juce::XmlElement> xml(state.createXml());
        copyXmlToBinary(*xml, destData);
    
}

void GranSynthZiAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
        
        if (xmlState.get() != nullptr) {
            if (xmlState->hasTagName(apvts.state.getType()))
                apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    
}

void GranSynthZiAudioProcessor::noteOn(int grainSize, int grainOverlap, int grainSpacing, double frequency, float velocity)
{
    
    if (granSynthBank.size() >= 5) {
            return;
    }
    
    for(auto& note : granSynthBank){
        if (note.getFreq() == frequency) {
            return;
        }
    }
    
    GranSynth granSynthTone(audioBuffer, frequency, velocity);
    granSynthTone.setGrainParameters(grainSize, grainOverlap, grainSpacing);
    
    granSynthTone.prepareToPlay(getSampleRate(), getBlockSize());
    granSynthBank.push_back(granSynthTone);
    
    
}

void  GranSynthZiAudioProcessor::noteOff(double frequency){
    for(auto& granSynth : granSynthBank){
        if (granSynth.getFreq() == frequency) {
            granSynth.setReleased();
        }
    }
}

void GranSynthZiAudioProcessor::setGrainSize(int newGrainSize) { 
    grainSize = newGrainSize;
    for (auto& gst: granSynthBank) 
        gst.setGrainParameters(grainSize, grainOverlap, grainSpacing);
}

void GranSynthZiAudioProcessor::setGrainOverlap(int newGrainOverlap) {
    grainOverlap = newGrainOverlap;
    for (auto& gst: granSynthBank) 
        gst.setGrainParameters(grainSize, grainOverlap, grainSpacing);
}

void GranSynthZiAudioProcessor::setGrainSpacing(int newGrainSpacing) {
    grainSpacing = newGrainSpacing;
    for (auto& gst: granSynthBank)
        gst.setGrainParameters(grainSize, grainOverlap, grainSpacing);
}

void GranSynthZiAudioProcessor::loadFile(const juce::File& file)
{
    // Load the audio file directly into the member audioBuffer
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

    if (reader != nullptr)
    {
        // Assuming a mono audio file for simplicity
        const int numChannels = 1;
        const int numSamples = (int)reader->lengthInSamples;

        audioBuffer.setSize(numChannels, numSamples);
        reader->read(&audioBuffer, 0, numSamples, 0, true, true);

//        // Normalize the audio buffer
//        audioBuffer.applyGain(1.0 / audioBuffer.getRMSLevel(0));

        // Reset the position to the beginning
//        audioPosition = 0;
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout GranSynthZiAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Add delay time parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(grainSizeId, "Grain Size",
                                                           juce::NormalisableRange<float>(5.0, 100.0, 1.0),
                                                           20.0f));

    // Add feedback parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(grainOverlapId, "Grain Overlap",
                                                           juce::NormalisableRange<float>(0.0, 99.0, 1.0),
                                                           0.0f));

    // Add wet/dry ratio parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(grainSpacingId, "Grain Spacing",
                                                           juce::NormalisableRange<float>(1.0, 200.0, 1.0),
                                                           20.0f));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GranSynthZiAudioProcessor();
}
