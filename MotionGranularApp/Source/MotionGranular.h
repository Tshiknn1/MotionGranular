/*
  ==============================================================================

    MotionGranular.h
    Created: 20 Apr 2025 5:10:48pm
    Author:  Tshiknn

  ==============================================================================
*/


#pragma once
#include <JuceHeader.h>
#include "MotionGrain.h"
#include "MotionRemoteInterpreter.h"

const int NUM_VOICES = 16;

class MotionGranularComponent : public juce::AudioAppComponent,
    /*public juce::ChangeListener,*/
    public juce::ComponentListener,
    public juce::Timer {

public:

    MotionGranularComponent();
    ~MotionGranularComponent() override;

    void setFileSource(juce::AudioFormatReader* incomingReader);

    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();

    constexpr float normalizeToPercent(int16_t x) const { return std::clamp(x / 16384.f, -1.f, 1.f); }

    MotionGrain::GrainParameters* parameters() { return &globalParams_; }


    // from AudioAppComponent
    void prepareToPlay(int, double) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void resized() override;

    // from Timer
    void timerCallback() override;

    // setters
    void setTimerPeriod(int timerPeriod) {
        timerPeriod_ = timerPeriod;
        timerUpdateFlag_ = true;
    }

    void setPlaybackPos(double playbackPos) { globalParams_.position = playbackPos; }
    void setPlaybackSpeed(double speed) { globalParams_.speed = speed; }

    void setAttack(float attack) { globalParams_.attack = attack < 0.f ? 0.f : attack; }
    void setDecay(float decay) { globalParams_.decay = decay < 0.f ? 0.f : decay; }
    void setSustain(float sustain) { globalParams_.sustain = std::clamp(sustain, 0.f, 1.f); }
    void setRelease(float release) { globalParams_.release = release < 0.f ? 0.f : release; }

private:

    MotionGrain* nextGrain();
    void clearAllGrains();

    std::vector<std::unique_ptr<MotionGrain>> grains_;
    juce::MixerAudioSource mixer_;

    // widget components
    juce::TextButton openButton_;
    juce::TextButton playButton_;
    juce::TextButton stopButton_;
    juce::Slider metroSlider_;
    juce::Slider posSlider_;
    juce::Slider speedSlider_;
    std::unique_ptr<juce::FileChooser> chooser_;
    /*juce::AudioThumbnail thumbnail_;*/

    // playback components
    std::unique_ptr<juce::AudioFormatReader> globalReader_;
    juce::AudioFormatManager formatManager_;

    MotionRemoteInterpreter interpreter_;

    // parameters
    MotionGrain::GrainParameters globalParams_;

    int grainCounter_ = 0;
    int timerPeriod_ = 500; // TODO: create a constant for this default value
    bool timerUpdateFlag_ = false;

    double sampleLength_;

    std::mutex sourceLock_;

};