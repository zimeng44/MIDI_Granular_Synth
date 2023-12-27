#include "MainComponent.h"
//#include "GranSynth.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    juce::AudioBuffer<float> temp(1, 512);
    granSynth.reset(new GranSynth(temp));

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
    
    setSize (650, 200);
    
    // Setup for buttons
    addAndMakeVisible(openButton);
    openButton.setButtonText("Open");
    openButton.addListener(this);
    openButton.setEnabled(true);
    
    addAndMakeVisible(playButton);
    playButton.setButtonText("Play");
    playButton.addListener(this);
    playButton.setEnabled(false);
    
    addAndMakeVisible(stopButton);
    stopButton.setButtonText("Stop");
    stopButton.addListener(this);
    stopButton.setEnabled(false);
    
    addAndMakeVisible(grainSizeSlider);
    grainSizeSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    grainSizeSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, 1, 50, 20);
    grainSizeSlider.setNormalisableRange(juce::NormalisableRange<double>(10.0, 1000.0, 1.0));
    grainSizeSlider.setValue(100.0);
    grainSizeSlider.setDoubleClickReturnValue(true, 100);
    grainSizeSlider.addListener(this);
    
    addAndMakeVisible(grainOverlapSlider);
    grainOverlapSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    grainOverlapSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, 1, 50, 20);
    grainOverlapSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0, 0.99, 0.01));
    grainOverlapSlider.setValue(0.0);
    grainOverlapSlider.setDoubleClickReturnValue(true, 0.0);
    grainOverlapSlider.addListener(this);
    
    addAndMakeVisible(grainSpacingSlider);
    grainSpacingSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    grainSpacingSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, 1, 50, 20);
    grainSpacingSlider.setNormalisableRange(juce::NormalisableRange<double>(0.1, 250.0, 0.1));
    grainSpacingSlider.setValue(25);
    grainSpacingSlider.setDoubleClickReturnValue(true, 25);
    grainSpacingSlider.addListener(this);
    
    addAndMakeVisible(pitchShiftSlider);
    pitchShiftSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    pitchShiftSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, 1, 50, 20);
    pitchShiftSlider.setNormalisableRange(juce::NormalisableRange<double>(0.1, 2.0, 0.1));
    pitchShiftSlider.setValue(1);
    pitchShiftSlider.setDoubleClickReturnValue(true, 1);
    pitchShiftSlider.addListener(this);
    
    initializeLabel(grainSizeLabel, "Grain Size (ms)", grainSizeSlider);
    initializeLabel(grainOverlapLabel, "Overlap (%)", grainOverlapSlider);
    initializeLabel(grainSpacingLabel, "Density (Hz)", grainSpacingSlider);
    initializeLabel(pitchShiftLabel, "Pitch Shift", pitchShiftSlider);

}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    
    currentSampleRate = sampleRate;
    samplesPerBlock = samplesPerBlockExpected;
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();
    
    // a barrier that only pass when the play button is clicked
    if (!currentlyPlaying)
        return;
    
    for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); channel++)
    {
        if (channel > 0)
        {
            bufferToFill.buffer->copyFrom(channel, 0, *bufferToFill.buffer, 0, 0, bufferToFill.buffer->getNumSamples());
            continue;
        }
        granSynth->processBlock(*bufferToFill.buffer);
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    
    auto area = getLocalBounds();

    auto leftArea1 = area.removeFromLeft(area.getWidth() / 3);
    auto leftArea = area.removeFromLeft(area.getWidth() / 4);
    auto rightArea = area;

    auto buttonHeight = leftArea.getHeight() / 3;
    openButton.setBounds(leftArea1.removeFromTop(buttonHeight));
    playButton.setBounds(leftArea1.removeFromTop(buttonHeight));
    stopButton.setBounds(leftArea1.removeFromTop(buttonHeight));

    auto sliderHeight = rightArea.getHeight() / 4;
    grainSizeSlider.setBounds(rightArea.removeFromTop(sliderHeight));
    grainOverlapSlider.setBounds(rightArea.removeFromTop(sliderHeight));
    grainSpacingSlider.setBounds(rightArea.removeFromTop(sliderHeight));
    pitchShiftSlider.setBounds(rightArea.removeFromTop(sliderHeight));
    
}

void MainComponent::initializeLabel(juce::Label& label, const juce::String& text, juce::Slider& slider) {
    addAndMakeVisible(label);
    label.setText(text, juce::dontSendNotification);
    label.attachToComponent(&slider, true);
}

void MainComponent::openFile()
{
    chooser = std::make_unique<juce::FileChooser>("Select a Wave file...",
                                                  juce::File{},
                                                  "*.wav");
    auto chooserFlags = juce::FileBrowserComponent::openMode
                      | juce::FileBrowserComponent::canSelectFiles;
    
    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)       // [8]
            {
                loadAudioFile(fc.getResult());
            });
}

bool MainComponent::loadAudioFile(const juce::File& file)
{
    
    //load and prepare the file for playing
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> formatread (formatManager.createReaderFor(file));
    
    if (formatread == nullptr)
    {
        return false;
    }

    fileBuffer.clear();
    fileBuffer.setSize(1, (int)formatread->lengthInSamples);
    formatread->read(&fileBuffer, 0, (int)formatread->lengthInSamples, 0, true, true);
    
    if (!fileBuffer.hasBeenCleared())
    {
        granSynth.reset(new GranSynth(fileBuffer));
        
        granSynth->setGrainsParams((int)(grainSizeSlider.getValue() / 1000 * currentSampleRate),
                                      (int)(grainOverlapSlider.getValue() * grainSizeSlider.getValue() / 1000 * currentSampleRate),
                                      (int)( currentSampleRate/grainSpacingSlider.getValue() ),
                                      (float)pitchShiftSlider.getValue());
        openButton.setEnabled(false);
        stopButton.setEnabled(false);
        playButton.setEnabled(true);
    }
    
    return true;
}

void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &openButton)
    {
        openFile();
    }
    else if (button == &playButton)
    {
        currentlyPlaying = true;
        openButton.setEnabled(false);
        playButton.setEnabled(false);
        stopButton.setEnabled(true);
    }
    else if (button == &stopButton)
    {
        currentlyPlaying = false;
        openButton.setEnabled(true);
        playButton.setEnabled(true);
        stopButton.setEnabled(false);
    }
}

void MainComponent::sliderValueChanged(juce::Slider *slider)
{
//    grainOverlapSlider.setNormalisableRange(juce::NormalisableRange<double>(0, grainSizeSlider.getValue()-1, 1.0));
    
    granSynth->setGrainsParams((int)(grainSizeSlider.getValue() / 1000 * currentSampleRate),
                                  (int)(grainOverlapSlider.getValue() * grainSizeSlider.getValue() / 1000 * currentSampleRate),
                                  (int)(currentSampleRate/grainSpacingSlider.getValue()),
                                  (float)pitchShiftSlider.getValue());
}
