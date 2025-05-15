/*
  ==============================================================================

    MotionRemoteInterpreter.h
    Created: 5 May 2025 12:03:58am
    Author:  Tshiknn

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <stdint.h>

#include "MotionRemote.h"

class MotionRemoteInterpreter : public juce::Component,
    public juce::Timer {

public:

    MotionRemoteInterpreter();
    ~MotionRemoteInterpreter();

    int16_t ax() { return axes_.ax.load(); }
    int16_t ay() { return axes_.ay.load(); }
    int16_t az() { return axes_.az.load(); }
    int16_t gx() { return axes_.gx.load(); }
    int16_t gy() { return axes_.gy.load(); }
    int16_t gz() { return axes_.gz.load(); }

    void timerCallback() override;

    // communication functions
    void openConnection();
    void closeConnection();
    void fetchAxes();

    // interface functions
    void indicateBadStatus(MotionRemote::MotionRemoteStatus status);
    void showAxes();
    void connectionButtonPressed();

    // derived functions
    void resized() override;

private:

    std::unique_ptr<MotionRemote> remote_;
    MotionRemote::MotionAxes axes_;
    bool connected_ = false;

    juce::TextEditor ipEntry_;
    juce::TextButton connectionButton_;
    juce::TextButton requestPacketButton_;

    juce::Slider axSlider_;
    juce::Slider aySlider_;
    juce::Slider azSlider_;
    juce::Slider gxSlider_;
    juce::Slider gySlider_;
    juce::Slider gzSlider_;

    std::mutex noThreadClobber_;
    int updateViewCounter_;
};