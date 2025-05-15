/*
  ==============================================================================

    FileReader.cpp
    Created: 21 Apr 2025 10:00:46pm
    Author:  Tshiknn

  ==============================================================================
*/

#include "FileReader.h"


FileReaderComponent::FileReaderComponent() {
    addAndMakeVisible(&openButton_);
    openButton_.setButtonText("Open...");
    openButton_.onClick = [this]() { openButtonClicked(); };

    addAndMakeVisible(&playButton_);
    playButton_.setButtonText("Play");
    playButton_.onClick = [this]() { playButtonClicked(); };
    playButton_.setEnabled(false);

    transportSource_.addChangeListener(this);
    
    setSize(200, 100);
    setAudioChannels(2, 2);

    formatManager_.registerBasicFormats();
}


FileReaderComponent::~FileReaderComponent() {

}


void FileReaderComponent::setFileSource(juce::AudioFormatReader* incomingReader) {
    stop();
    readerSource_.reset();

    readerSource_.reset(new juce::AudioFormatReaderSource(incomingReader, false));
    transportSource_.setSource(readerSource_.get());
}


void FileReaderComponent::openButtonClicked() {
    chooser_ = std::make_unique<juce::FileChooser>("Choose File You Cur", juce::File(), "*.wav");
    int browserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    chooser_->launchAsync(browserFlags, [this] (const juce::FileChooser& fc) {
        juce::File result = fc.getResult();
        
        if (result != juce::File()) {
            globalReader_.reset(formatManager_.createReaderFor(result));
            if (globalReader_.get() != nullptr) {
                setFileSource(globalReader_.get()); // obviously a prelude to the way it will work once polyphonic
            }
        }
    });
}


void FileReaderComponent::playButtonClicked() {
    spawn();
}


void FileReaderComponent::spawn() {
    env_.noteOn();
    transportSource_.setPosition(params_.position);
    transportSource_.start();
    startTimer(params_.noteLength);

    env_.setParameters(juce::ADSR::Parameters(
        params_.attack,
        params_.decay,
        params_.sustain,
        params_.release
    ));

    playButton_.setEnabled(false);
}


void FileReaderComponent::stop() {
    transportSource_.stop();

    playButton_.setEnabled(true);
}


void FileReaderComponent::setGrainParameters(const FileReaderComponent::GrainParameters& parameters) {
    params_ = parameters;
}


void FileReaderComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    transportSource_.prepareToPlay(samplesPerBlockExpected, sampleRate);
    env_.setSampleRate(sampleRate);
}


void FileReaderComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    transportSource_.getNextAudioBlock(bufferToFill);
    env_.applyEnvelopeToBuffer(*bufferToFill.buffer, bufferToFill.startSample, bufferToFill.buffer->getNumSamples());

    if (transportSource_.isPlaying() && !env_.isActive()) {
        stop();
    }
}


void FileReaderComponent::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &transportSource_) {

    }
}


void FileReaderComponent::timerCallback() {
    env_.noteOff();
    stopTimer();
}


void FileReaderComponent::releaseResources() {
    transportSource_.releaseResources();
}


void FileReaderComponent::resized() {
    openButton_.setBounds(10, 10, getWidth() - 20, 20);
    playButton_.setBounds(10, 40, getWidth() - 20, 20);
}