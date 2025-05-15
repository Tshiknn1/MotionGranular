/*
  ==============================================================================

    MotionGrain.cpp
    Created: 23 Apr 2025 1:09:35am
    Author:  Tshiknn

  ==============================================================================
*/

#include "MotionGrain.h"


MotionGrain::MotionGrain(GrainParameters* globalParams) :
        resamplingSource_(&transportSource_, false) {
    transportSource_.addChangeListener(this);
    params_ = globalParams;
}


MotionGrain::~MotionGrain() {

}


void MotionGrain::spawn() {
    transportSource_.setPosition(params_->position.load());

    resamplingSource_.setResamplingRatio(params_->speed.load());

    env_.setParameters(juce::ADSR::Parameters(
        params_->attack.load(),
        params_->decay.load(),
        params_->sustain.load(),
        params_->release.load()
    ));

    env_.noteOn();
    transportSource_.start();

    startTimer(params_->noteLength.load()); // start timer to shut off note
}


void MotionGrain::stop() {
    transportSource_.stop();
}


void MotionGrain::setFileSource(juce::AudioFormatReader* incomingReader) {
    stop();
    readerSource_.reset(new juce::AudioFormatReaderSource(incomingReader, false));
    transportSource_.setSource(readerSource_.get());
}


void MotionGrain::timerCallback() {
    env_.noteOff();
    stopTimer();
}


void MotionGrain::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    resamplingSource_.prepareToPlay(samplesPerBlockExpected, sampleRate);
    env_.setSampleRate(sampleRate);
}


void MotionGrain::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    resamplingSource_.getNextAudioBlock(bufferToFill);
    env_.applyEnvelopeToBuffer(*bufferToFill.buffer, bufferToFill.startSample, bufferToFill.buffer->getNumSamples());

    /*if (transportSource_.isPlaying() && !env_.isActive()) {
        stop();
    }*/
}


void MotionGrain::releaseResources() {
    transportSource_.releaseResources();
}


void MotionGrain::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &transportSource_) {

    }
}


void MotionGrain::clearSource() {
    transportSource_.setSource(nullptr);
    readerSource_.reset();
}