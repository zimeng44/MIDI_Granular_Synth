/*
  ==============================================================================

    GranSynth.cpp
    Created: 19 Dec 2023 1:30:19am
    Author:  Zi Meng

  ==============================================================================
*/

#include "GranSynth.h"

Grain::Grain(const juce::AudioBuffer<float>& sourceBuffer, int startSample,
             int grainSize, float newPitchShiftFactor)
{
//    extract a grain from the source buffer with the specified parameters
    size = grainSize;
    currentPosition = 0;
    pitchShiftFactor = newPitchShiftFactor;
    grainAudioData.setSize(1, size);
    grainAudioData.clear();
    
    auto sourceReader = sourceBuffer.getReadPointer(0, startSample);
    auto grainWriter =  grainAudioData.getWritePointer(0);
    auto grainReader =  grainAudioData.getReadPointer(0);
    
    for (int i = 0; i < size; ++i){
        
        float readIndex = (float)i * pitchShiftFactor;
        float a = readIndex - floor(readIndex);
        float interpolate = 0.0f;
        
        if (((int)floor(readIndex)) < (sourceBuffer.getNumSamples()-1)){
            auto sampleLow = sourceReader[(int)floor(readIndex)];
            auto sampleHigh = sourceReader[(int)floor(readIndex) + 1];
            interpolate = (1-a) * sampleLow + a * sampleHigh;
            grainWriter[i] = interpolate;

        }
        
    }
    
    auto window = createHanningWindow(size);
    
    for (int i = 0; i < size; ++i){
        grainWriter[i] = grainReader[i] * window[i];
    }
    
    
}

void Grain::processGrain(juce::AudioBuffer<float>& outputBuffer, int
                         startSampleInOutput)
{
//    put one sample of the grain into the output buffer at 'startSampleInOutput'

    if (currentPosition > size - 1) {
            return;
    }
    auto outputBufferWriter = outputBuffer.getWritePointer(0, startSampleInOutput);
    auto grainReader = grainAudioData.getReadPointer(0, currentPosition);
    outputBufferWriter[0] = grainReader[0];
    currentPosition++;
    
}

bool Grain::isFinished() const{
//    check if the grain has been played to the last sample;
    if (currentPosition >= size)
        return true;
    else
        return false;
}


//=====================================================================


GranSynth::GranSynth(juce::AudioBuffer<float>& audioFileBuffer, double freq, float velocity)
{
//    create a new instance of GranSynth and set up the parameters
    
    internalBuffer.setSize(1, audioFileBuffer.getNumSamples());
    internalBuffer.copyFrom(0, 0, audioFileBuffer, 0, 0, audioFileBuffer.getNumSamples());
    currentFreq = freq;
    currentVelocity = velocity;
}

GranSynth::~GranSynth()
{
        
}

void GranSynth::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    
    currentSampleRate = sampleRate;
    currentSamplesPerBlock = samplesPerBlock;
    
}

void GranSynth::releaseResources()
{
    
}

void GranSynth::processBlock(juce::AudioBuffer<float>& buffer)
{
//    prepare a temp output buffer
    juce::AudioBuffer<float> output(1, buffer.getNumSamples());
    output.clear();
    auto outputReader = output.getReadPointer(0);
    
    // process grains ( put each sample of the unfinished grains into each sample of the temp output buffer sequentially )
    
    for (int i = 0; i < output.getNumSamples(); i++){
        
        // delete grains that have finished playback
        for(int j = 0; j < grains.size(); j++){
            if(grains[j].isFinished()){
                grains.erase(grains.begin() + j);

            }
        }
        
        // process each unfinished grain
        for(auto& g : grains){
            
            // if current grain has started playback, finish playing back
            if (g.getCurrentPosition() > 0) {
                
                g.processGrain(output, i);


            }
            else
            {
                // if current grain hasn't started playback, see if it's at the graining spacing
                if ((internalReadPosition % grainSpacing) == 0){
                    
                    g.processGrain(output, i);
                    
                }
                
            }
        }
        
        internalReadPosition++;

    }
    
    
    
    for (int channel = 0; channel < buffer.getNumChannels(); channel++) {
        
        // copy the first channel to other channels
        if (channel > 0) {
            buffer.copyFrom(channel, 0, buffer, 0, 0, buffer.getNumSamples());
            break;
        }
        
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int i = 0; i < buffer.getNumSamples(); i++){
            
            // copy the temp output buffer to the processBlock buffer
            if (outputReader[i] > 0.99) channelData[i] += 0.99 * gain;
            else channelData[i] += outputReader[i] * gain;
            
            // create new grains
            // read from the source file repeatedly
            if (startSampleInFile > (internalBuffer.getNumSamples() - 1)){
                startSampleInFile -=  (internalBuffer.getNumSamples());
            }
            
            // create a new grain when each time 'startSampleInFile' reaches '(grainSize - grainOverlap)'
            if (startSampleInFile % (grainSize - grainOverlap) == 0){

                Grain newGrain(internalBuffer, startSampleInFile, grainSize, currentFreq/referencePitch);
                grains.push_back(newGrain);

            }

            startSampleInFile++;
            
            // update gain factor for fade-ins and fade-ins when each GranSynth start and stop playing
            updateTote();

        }

    }
    
}

void GranSynth::setGrainParameters(int size, int overlap, int spacing)
{
    grainSize = size;
    grainOverlap = overlap;
    grainSpacing = spacing;
    
}

double GranSynth::pitchShiftDetection(const float* input, int length)
{
    //Unused
    // Implement autocorrelation-based pitch detection
    // This is a basic example, and you may need to fine-tune parameters for your use case

    // Autocorrelation function
    juce::Array<double> autocorrelation(length, 0.0);

    for (int lag = 0; lag < length; ++lag)
    {
        double sum = 0.0;

        for (int i = 0; i < length - lag; ++i)
        {
            sum += static_cast<double>(input[i]) * static_cast<double>(input[i + lag]);
        }

        autocorrelation.set(lag, sum);
    }

    // Find the peak in the autocorrelation function
    int maxIndex = 0;
    double maxValue = autocorrelation[maxIndex];

    for (int i = 1; i < length; ++i)
    {
        if (autocorrelation[i] > maxValue)
        {
            maxValue = autocorrelation[i];
            maxIndex = i;
        }
    }

    // Calculate the pitch based on the index of the peak
    double fundamentalFrequency =  currentSampleRate / static_cast<double>(maxIndex);

    // Convert frequency to MIDI note number
    double midiNoteInHertz = juce::MidiMessage::getMidiNoteInHertz(static_cast<float>(fundamentalFrequency));

    // Return the pitch ratio for granular synthesis
    return midiNoteInHertz/referencePitch;
}

void GranSynth::updateTote() {
    if (!isReleased) {
        if (gain < currentVelocity) {
            gain *= ATTACK_FACTOR;
        }
    } else {
        gain *= DECAY_FACTOR;
    }
}

bool GranSynth::shouldBeRemoved() const{
    if(gain < 0.0005){
        return true;
    }else{
        return false;
    }
}
