/*
  ==============================================================================

    MotionGranular.cpp
    Created: 20 Apr 2025 5:10:48pm
    Author:  Tshiknn

  ==============================================================================
*/

#include "MotionGranular.h"


MotionGranularComponent::MotionGranularComponent() {
    for (int i = 0; i < NUM_VOICES; i++) {
        std::unique_ptr<MotionGrain> newGrain = std::make_unique<MotionGrain>(&globalParams_);
        mixer_.addInputSource(newGrain.get(), false);
        grains_.push_back(std::move(newGrain));
    }

    addAndMakeVisible(&interpreter_);

    addAndMakeVisible(&openButton_);
    openButton_.setButtonText("Open...");
    openButton_.onClick = [this]() { openButtonClicked(); };

    addAndMakeVisible(&playButton_);
    playButton_.setButtonText("Play");
    playButton_.onClick = [this]() { playButtonClicked(); };
    playButton_.setEnabled(false);

    addAndMakeVisible(&stopButton_);
    stopButton_.setButtonText("Stop");
    stopButton_.onClick = [this]() { stopButtonClicked(); };
    stopButton_.setEnabled(false);

    addAndMakeVisible(&metroSlider_);
    metroSlider_.setNormalisableRange(juce::NormalisableRange<double>(0.0, 2000.0, 1.0, 0.5));
    metroSlider_.onValueChange = [this]() { setTimerPeriod(metroSlider_.getValue()); };
    metroSlider_.setValue(500.0);

    addAndMakeVisible(&posSlider_);
    posSlider_.onValueChange = [this]() { setPlaybackPos(posSlider_.getValue()); };
    posSlider_.setValue(0.0);
    posSlider_.setEnabled(false);

    addAndMakeVisible(&speedSlider_);
    speedSlider_.setNormalisableRange(juce::NormalisableRange<double>(0.25, 4.0, 0.01, 0.5));
    speedSlider_.onValueChange = [this]() { setPlaybackSpeed(speedSlider_.getValue()); };
    speedSlider_.setValue(1.0);

    setSize(200, 500);
    setAudioChannels(2, 2);

    formatManager_.registerBasicFormats();
}


MotionGranularComponent::~MotionGranularComponent() {

}


void MotionGranularComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    mixer_.prepareToPlay(samplesPerBlockExpected, sampleRate);
}


// TODO: think about clipping
void MotionGranularComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    mixer_.getNextAudioBlock(bufferToFill);
}


void MotionGranularComponent::releaseResources() {
    mixer_.releaseResources();
}


void MotionGranularComponent::resized() {
    openButton_.setBounds(10, 10, getWidth() - 20, 20);
    playButton_.setBounds(10, 40, getWidth() - 20, 20);
    stopButton_.setBounds(10, 70, getWidth() - 20, 20);
    metroSlider_.setBounds(5, 100, getWidth() - 10, 20);
    posSlider_.setBounds(5, 130, getWidth() - 10, 20);
    speedSlider_.setBounds(5, 160, getWidth() - 10, 20);

    interpreter_.setBounds(0, 190, getWidth(), 280);
}


MotionGrain* MotionGranularComponent::nextGrain() {
    MotionGrain* r = grains_.at(grainCounter_).get();
    grainCounter_ = (grainCounter_ + 1) % NUM_VOICES;
    return r;
}


void MotionGranularComponent::timerCallback() {
    int16_t z = interpreter_.az();
    /*OutputDebugString("z then pos\n");
    OutputDebugString(std::to_string(z).c_str());
    OutputDebugString("\n");
    OutputDebugString(std::to_string(normalizeToPercent(z)).c_str());
    OutputDebugString("\n");*/
    double pos = ((1 - normalizeToPercent(z)) / 2) * sampleLength_ * 0.85;
    /*OutputDebugString(std::to_string(pos).c_str());
    OutputDebugString("\n");*/
    globalParams_.position.store(pos);

    int16_t y = interpreter_.ay();
    globalParams_.speed.store(1.01 + (normalizeToPercent(y)));

    int16_t gz = interpreter_.gx();
    setTimerPeriod((16384 - std::abs(gz)) / 64);

    nextGrain()->spawn();
    if (timerUpdateFlag_) {
        stopTimer();
        startTimer(timerPeriod_);
    }
}


void MotionGranularComponent::openButtonClicked() {
    chooser_ = std::make_unique<juce::FileChooser>("Choose File You Cur", juce::File(), "*.wav");
    int browserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    chooser_->launchAsync(browserFlags, [this](const juce::FileChooser& fc) {
        juce::File result = fc.getResult();

        if (result != juce::File()) {
            clearAllGrains();
            globalReader_.reset(formatManager_.createReaderFor(result));
            if (globalReader_.get() != nullptr) {
                setFileSource(globalReader_.get());
                sampleLength_ = ((double)(globalReader_->lengthInSamples)) / globalReader_->sampleRate;
                posSlider_.setRange(0.0, sampleLength_);
                posSlider_.setEnabled(true);
            }
        }

        playButton_.setEnabled(true);
        stopButton_.setEnabled(false);
    });
}


void MotionGranularComponent::playButtonClicked() {
    startTimer(timerPeriod_);
    playButton_.setEnabled(false);
    stopButton_.setEnabled(true);
}


void MotionGranularComponent::stopButtonClicked() {
    stopTimer();
    playButton_.setEnabled(true);
    stopButton_.setEnabled(false);
}


void MotionGranularComponent::setFileSource(juce::AudioFormatReader* incomingReader) {
    for (auto&& grain : grains_) {
        grain->setFileSource(incomingReader);
    }
}


void MotionGranularComponent::clearAllGrains() {
    for (auto&& grain : grains_) {
        grain->clearSource();
    }
}