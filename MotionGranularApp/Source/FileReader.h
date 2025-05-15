/*
  ==============================================================================

    FileReader.h
    Created: 21 Apr 2025 10:00:46pm
    Author:  Tshiknn

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class FileReaderComponent : public juce::AudioAppComponent,
    public juce::ChangeListener,
    public juce::Timer {

public:

    FileReaderComponent();
    ~FileReaderComponent() override;

    struct FileReaderComponent::GrainParameters {
        GrainParameters() { }

        int noteLength = 1000;

        float attack = 0.2f;
        float decay = 0.1f;
        float sustain = 0.5f;
        float release = 1.f;

        double position = 0.0;
    };

    void setFileSource(juce::AudioFormatReader* incomingReader);

    void openButtonClicked();
    void playButtonClicked();

    void spawn();
    void stop();
    void setGrainParameters(const GrainParameters& parameters);

    // from AudioAppComponent
    void prepareToPlay(int, double) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void resized() override;

    // from ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    // from Timer
    void timerCallback() override;

private:

    // widget components
    juce::TextButton openButton_;
    juce::TextButton playButton_;
    std::unique_ptr<juce::FileChooser> chooser_;
    /*juce::AudioThumbnail thumbnail_;*/

    // playback components
    juce::AudioTransportSource transportSource_;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource_;
    std::unique_ptr<juce::AudioFormatReader> globalReader_;
    juce::AudioFormatManager formatManager_;

    juce::ADSR env_;

    // parameters
    GrainParameters params_;

};