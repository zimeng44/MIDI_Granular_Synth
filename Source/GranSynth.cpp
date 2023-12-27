/*
  ==============================================================================

    GranSynth.cpp
    Created: 20 Dec 2023 10:31:29pm
    Author:  Hanzhi Zhang / Zi Meng

  ==============================================================================
*/

#include "GranSynth.h"


Grain::Grain(const juce::AudioBuffer<float>& filebuffer, int startSample,
 int grainSize, float newpitchshiftfactor)
{
    size = grainSize;
    currentPosition = 0;
    pitchShiftFactor = newpitchshiftfactor;

    grainAudioData.setSize(1, size);
    grainAudioData.clear();
    
    float *sample = grainAudioData.getWritePointer(0);
    int copiedNumSamples = 0;
    
    //interpolation
    for (int i = 0; i < size; i++)
    {
        if(std::ceil(startSample + i * pitchShiftFactor) > filebuffer.getNumSamples()-1) break;
        
        //get the decimal part of pitchShiftFactor, which is the ratio
        float decimalPart = std::abs(pitchShiftFactor - std::floor(pitchShiftFactor));
        float sample1 = filebuffer.getSample(0, std::floor(startSample+ i*pitchShiftFactor));
        float sample2 = filebuffer.getSample(0, std::ceil(startSample+ i*pitchShiftFactor));

        float interpolateSample = sample1 * decimalPart + sample2 * (1-decimalPart);
        sample[i] = interpolateSample;
        copiedNumSamples++;
        
    }
    
//    std::vector<float> window = createHanningWindow(size); //window can be in the constructor
//    
//    for (int i = 0; i < size; ++i){
//        sample[i] = sample[i] * window[i];
//    }
    
    grainAudioData.applyGainRamp(0, 0, ceil(size*0.01), 0, 1);
    grainAudioData.applyGainRamp(0, floor(copiedNumSamples*0.99), floor(copiedNumSamples*0.01), 1, 0);
    
}

Grain::~Grain()
{

}

void Grain::processGrain(juce::AudioBuffer<float>& outputBuffer, int
  startSampleInOutput, float gain) //playback
{
    if (currentPosition >= size-1)
        return;
    
    auto outputIndex = outputBuffer.getWritePointer(0, startSampleInOutput); //output it directly to the system buffer
    auto grainReadPtr = grainAudioData.getReadPointer(0, currentPosition);
   
    outputIndex[0] += grainReadPtr[0];

    currentPosition++; //iterate current position
}

bool Grain::isFinished()
{
    if (currentPosition >= size-1)
    {
        return true;
    }
    else
        return false;
}




//========================================================




GranSynth::GranSynth(juce::AudioBuffer<float>& openedFileBuffer)
{
    fileBuffer.setSize(1, openedFileBuffer.getNumSamples());
    fileBuffer.copyFrom(0, 0, openedFileBuffer, 0, 0, openedFileBuffer.getNumSamples());
}

GranSynth::~GranSynth()
{
    
}

void GranSynth::processBlock(juce::AudioBuffer<float>& bufferToFill)
{
    DBG("grain size "<<grainSize<<" overlap "<< grainOverlap <<" spacing "<<grainSpacing);
    juce::AudioBuffer<float> tempOutBuffer(1, bufferToFill.getNumSamples());
    tempOutBuffer.clear();
    
    for (int i = 0; i < tempOutBuffer.getNumSamples(); i++)
    {
//        loop and delete grains
        for(int j = 0; j < grains.size(); j++)
        {
            if(grains[j].isFinished())
                grains.erase(grains.begin() + j);
        }
        //playback
        for (auto &g: grains)
        {
            if ((g.getCurrentPosition() > 0) || (outputCounter % grainSpacing == 0))
            {

                g.processGrain(tempOutBuffer, i, gain);
//                if(outputCounter % grainSpacing == 0) DBG("whole number hit "<<outputCounter<<" with filereadposition at "<< startSampleInFile<<" grain size "<<grains.size());
            }
        }
        outputCounter++;
    }
    
    auto tempOutBufferReadPtr = tempOutBuffer.getReadPointer(0);
    auto* channelData = bufferToFill.getWritePointer(0);
    
    for (int i = 0; i < bufferToFill.getNumSamples(); i++)
    {
        if (tempOutBufferReadPtr[i] > 0.99)
            channelData[i] += 0.99 * gain;
        else
            channelData[i] += tempOutBufferReadPtr[i] * gain;
    
        if (startSampleInFile > (fileBuffer.getNumSamples() - 2)){
            startSampleInFile = 0;
        }
        if ((startSampleInFile % (grainSize - grainOverlap)) == 0)
        {
            Grain newGrain(fileBuffer, startSampleInFile, grainSize, pitchShiftFactor);
            grains.push_back(newGrain);
//            DBG("new grain added at "<< startSampleInFile);
        }
        startSampleInFile++;
    }
}

void GranSynth::setGrainsParams(int newGrainSize, int newGrainOverlap, int newGrainSpacing, float newPitchShiftFactor){
    
    grainSize = newGrainSize;
    grainOverlap = newGrainOverlap;
    grainSpacing = newGrainSpacing;
    pitchShiftFactor = newPitchShiftFactor;
    
}
