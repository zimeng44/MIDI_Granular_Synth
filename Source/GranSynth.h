/*
  ==============================================================================

    GranSynth.h
    Created: 19 Dec 2023 1:30:08am
    Author:  Zi Meng

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class Grain {
    
public:
    Grain(const juce::AudioBuffer<float>& sourceBuffer, int startSample,
     int grainSize, float pitchShiftFactor);
    
    ~Grain(){  }
    
    void processGrain(juce::AudioBuffer<float>& outputBuffer, int
      startSampleInOutput);
    
    int getCurrentPosition(){ return currentPosition; }
    
    bool isFinished() const;
    
    juce::AudioBuffer<float>& getData(){ return grainAudioData;}

private:
    
    juce::AudioBuffer<float> grainAudioData;
    int currentPosition;
    int size;
    float pitchShiftFactor;
    
    inline std::vector<float> createHanningWindow(int size) {
        std::vector<float> window(size);
        for (int i = 0; i < size; ++i) {
            window[i] = 0.5f * (1 - std::cos(2 * 3.141592654 * i / (size - 1)));
        }
        return window;
    }
   // Add any other members if warranted
};

//===================================================================

class GranSynth {
    
public:
    
    GranSynth(juce::AudioBuffer<float>& audioFileBuffer, double freq, float vel);
    ~GranSynth();
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void releaseResources();
    void processBlock(juce::AudioBuffer<float>& buffer);
    void setGrainParameters(int size, int overlap, int spacing);
    double pitchShiftDetection(const float* input, int length);
    
    double getFreq() const { return currentFreq; }
    
    void setReleased(){ isReleased = true; }
    void updateTote();
    bool shouldBeRemoved() const;
    
private:

    std::vector<Grain> grains; // Vector to manage grains
    juce::AudioBuffer<float> internalBuffer;
    int internalReadPosition = 0;
    int startSampleInFile = 0;
    int startSampleInOutput = 0;
    int grainSize = 1000;
    int grainOverlap = 0;
    int grainSpacing = grainSize;
    double currentFreq;
    float currentVelocity;
    double currentSampleRate;       // Current sample rate
    int currentSamplesPerBlock;     // Current samples per block
    bool isReleased = false;
    double ATTACK_FACTOR = 1.05, DECAY_FACTOR = 0.995;
    double gain = 0.001;
    
    // Reference pitch for pitch modulation
    static constexpr double referencePitch = 440.0;  // A4 in Hertz
    // Additional private members and methods as needed
};
