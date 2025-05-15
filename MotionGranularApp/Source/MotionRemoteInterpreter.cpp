/*
  ==============================================================================

    MotionRemoteInterpreter.cpp
    Created: 5 May 2025 12:03:58am
    Author:  Tshiknn

  ==============================================================================
*/

#include "MotionRemoteInterpreter.h"
#include <Windows.h>
#include <string>


MotionRemoteInterpreter::MotionRemoteInterpreter() {
    addAndMakeVisible(ipEntry_);
    ipEntry_.setText("192.168.137.33");

    addAndMakeVisible(connectionButton_);
    connectionButton_.onClick = [this]() { connectionButtonPressed(); };
    connectionButton_.setButtonText("Open Connection");

    addAndMakeVisible(axSlider_);
    axSlider_.setRange(-32768.0, 32767.0, 1.0);
    axSlider_.setEnabled(false);

    addAndMakeVisible(aySlider_);
    aySlider_.setRange(-32768.0, 32767.0, 1.0);
    aySlider_.setEnabled(false);

    addAndMakeVisible(azSlider_);
    azSlider_.setRange(-32768.0, 32767.0, 1.0);
    azSlider_.setEnabled(false);

    addAndMakeVisible(gxSlider_);
    gxSlider_.setRange(-32768.0, 32767.0, 1.0);
    gxSlider_.setEnabled(false);

    addAndMakeVisible(gySlider_);
    gySlider_.setRange(-32768.0, 32767.0, 1.0);
    gySlider_.setEnabled(false);

    addAndMakeVisible(gzSlider_);
    gzSlider_.setRange(-32768.0, 32767.0, 1.0);
    gzSlider_.setEnabled(false);

    addAndMakeVisible(requestPacketButton_);
    requestPacketButton_.onClick = [this]() { timerCallback(); };
    requestPacketButton_.setButtonText("Request Packet");
}


MotionRemoteInterpreter::~MotionRemoteInterpreter() {
    if (connected_) {
        remote_->toggleFeed(false);
    }
}


void MotionRemoteInterpreter::openConnection() {
    std::string ip_str = ipEntry_.getText().toRawUTF8();
    const char* ip = ip_str.c_str();
    remote_.reset(new MotionRemote(ip, 9001));

    OutputDebugString("trying to initialize with following connection:");
    OutputDebugString(ip);
    OutputDebugString("\n");

    remote_->verify();
    MotionRemote::MotionRemoteStatus status = remote_->status();
    if (status != MotionRemote::MotionRemoteStatus::CONNECTED) {
        indicateBadStatus(status);
        OutputDebugString("bad status\n");
    }

    OutputDebugString("done\n");

    bool success = remote_->toggleFeed(true);
    if (success) {
        connected_ = true;
        startTimer(200);
        remote_->startTimer(10);
    }
}


void MotionRemoteInterpreter::closeConnection() {
    stopTimer();
    remote_->toggleFeed(false);
    remote_.reset();
    connected_ = false;
}


void MotionRemoteInterpreter::connectionButtonPressed() {
    if (!connected_) {
        openConnection();
        connectionButton_.setButtonText("Close Connection");
    }
    else {
        closeConnection();
        connectionButton_.setButtonText("Open Connection");
    }
}


void MotionRemoteInterpreter::fetchAxes() {
    /*remote_->update(&axes_, false);*/
    remote_->fetch(&axes_);
}


void MotionRemoteInterpreter::showAxes() {
    axSlider_.setValue(axes_.ax.load());
    aySlider_.setValue(axes_.ay.load());
    azSlider_.setValue(axes_.az.load());
    gxSlider_.setValue(axes_.gx.load());
    gySlider_.setValue(axes_.gy.load());
    gzSlider_.setValue(axes_.gz.load());
}


void MotionRemoteInterpreter::indicateBadStatus(MotionRemote::MotionRemoteStatus status) {

}


void MotionRemoteInterpreter::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    int x = bounds.getX();
    int y = bounds.getY();
    int width = bounds.getWidth();
    int height = bounds.getHeight();

    ipEntry_.setBounds(x + 10, y + 10, width - 20, 20);
    connectionButton_.setBounds(x + 10, y + 40, width - 20, 20);

    axSlider_.setBounds(x + 10, y + 70, width - 20, 20);
    aySlider_.setBounds(x + 10, y + 100, width - 20, 20);
    azSlider_.setBounds(x + 10, y + 130, width - 20, 20);
    gxSlider_.setBounds(x + 10, y + 160, width - 20, 20);
    gySlider_.setBounds(x + 10, y + 190, width - 20, 20);
    gzSlider_.setBounds(x + 10, y + 220, width - 20, 20);

    requestPacketButton_.setBounds(x + 10, y + 250, width - 20, 20);
}


void MotionRemoteInterpreter::timerCallback() {
    fetchAxes();

    if (remote_->status() != MotionRemote::MotionRemoteStatus::CONNECTED) {
        return;
    }

    showAxes();
}