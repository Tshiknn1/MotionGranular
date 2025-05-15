/*
  ==============================================================================

    MotionGrain.h
    Created: 23 Apr 2025 1:09:35am
    Author:  Tshiknn

  ==============================================================================
*/

#pragma once

/*
  ==============================================================================

    FileReader.h
    Created: 21 Apr 2025 10:00:46pm
    Author:  Tshiknn

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class MotionGrain : public juce::AudioSource,
    public juce::ChangeListener,
    public juce::Timer {

public:

    struct GrainParameters {
        GrainParameters() {}

        std::atomic<int> noteLength = 1000;

        std::atomic<float> attack = 0.2f;
        std::atomic<float> decay = 0.1f;
        std::atomic<float> sustain = 0.5f;
        std::atomic<float> release = 1.f;

        std::atomic<double> position = 0.0;
        std::atomic<double> speed = 1.f;

        std::atomic<float> pan = 0.5f;
        std::atomic<float> gain = 1.f;
    };

    MotionGrain(GrainParameters* globalParams);
    ~MotionGrain() override;

    void setFileSource(juce::AudioFormatReader* incomingReader);
    void clearSource();

    void spawn();
    void stop();

    // from AudioAppComponent
    void prepareToPlay(int, double) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // from ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    // from Timer
    void timerCallback() override;

private:

    // playback components
    juce::AudioTransportSource transportSource_;
    juce::ResamplingAudioSource resamplingSource_;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource_;
    juce::ADSR env_;

    // parameters
    GrainParameters* params_;
};