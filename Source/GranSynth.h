/*
  ==============================================================================

    GranSynth.h
    Created: 22 Dec 2023 6:01:40pm
    Author:  Hanzhi Zhang / Zi Meng

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#define PI 3.141592654


class Grain
{
public:
    
    Grain(const juce::AudioBuffer<float>& fileBuffer, int startsample,
          int grainSize, float pitchShiftFactor);
    ~Grain();
    void processGrain(juce::AudioBuffer<float>& systemBuffer,
                      int startSampleInOutput, float gain);
    int getCurrentPosition(){return currentPosition;}
    bool isFinished();
    
    inline std::vector<float> createHanningWindow(int size)
    {
        std::vector<float> window(size);
        for (int i = 0; i < size; i++)
        {
            window[i] = 0.5f * (1 - std::cos(2*PI*i/(size - 1)));
        }
        return window;
    }
    
private:
    juce::AudioBuffer<float> grainAudioData;
    int size;
    float pitchShiftFactor;
    int currentPosition = 0; //the position inside grain size
    // Add any other members if warranted
};



//===============================================================



class GranSynth
{
public:
    
    GranSynth(juce::AudioBuffer<float>& audioFileBuffer);
    ~GranSynth();
    
    void releaseResources();
    void processBlock(juce::AudioBuffer<float>& outputBuffer);
    void setGrainsParams(int newGrainSize, int newGrainOverlap, int newGrainSpacing, float newPitchShiftFactor);
    
    void saveOutput(juce::AudioBuffer<float>& buffer)
    {
        juce::File outputFile(juce::String("/Users/zimeng/test/analysis" + juce::String(fileVar) + ".wav"));
        fileVar += 1;
        if(outputFile.exists()) outputFile.deleteFile();
        outputFile.create();
        auto fileStream = std::make_unique<juce::FileOutputStream>(outputFile);
        
        if(fileStream == nullptr) return;
        // in real code you must check whether the stream is null before continuing
        auto wavFormat = std::make_unique<juce::WavAudioFormat>();

        std::unique_ptr<juce::AudioFormatWriter> writer (wavFormat->createWriterFor(fileStream.get(), 44100, buffer.getNumChannels(), 16, {}, 0));
        if(writer == nullptr) return;
        // again, you need add a check for a nullptr here
        writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
        fileStream->flush();
        writer.reset();
        writer.release();
        fileStream.release();
    }
    
private:
    int fileVar = 0;
    juce::AudioBuffer<float> fileBuffer;
    std::vector<Grain> grains; // Vector to manage grains
    int grainSize, grainOverlap, grainSpacing;
    float pitchShiftFactor;
    int startSampleInFile = 0;
    int outputCounter = 0;
    float gain = 1;
    
};
