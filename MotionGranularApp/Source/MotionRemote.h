/*
  ==============================================================================

    MotionRemote.h
    Created: 30 Apr 2025 12:27:12am
    Author:  Tshiknn

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include <stdint.h>
#include <atomic>

#if defined(_WIN32)
    #include <WinSock2.h>
#else
    #include <sys/socket.h>
#endif

#define MOTION_REMOTE_BUF_SIZE 1024

// must be high resolution because regular timer doesn't start thread
class MotionRemote : public juce::HighResolutionTimer {
    
public:

    struct MotionAxes {
        std::atomic<int16_t> ax, ay, az;
        std::atomic<int16_t> gx, gy, gz;
    };

    /*
    consider taking arguments for connection info (ip, port, etc.)
    */
    MotionRemote(const char* ipAddress, int port);
    ~MotionRemote();


    enum MotionRemoteStatus {
        IDLE,
        CONNECTED,
        NO_CONNECTION,
        TIMEOUT,
        BAD_RESPONSE
    };


    void update(MotionAxes* r, bool requestNeeded);
    void fetch(MotionAxes* r);
    void verify();
    bool toggleFeed(bool);

    MotionRemoteStatus status() { return status_; }

    void hiResTimerCallback() override;


private:

    int request(const char* req, char* buffer, int bufSize, sockaddr_in* respaddr);

    const int DATA_BUF_SIZE = 12;
    const int ID_BUF_SIZE = 64;

    const char* DATA_REQUEST = "GYRO";
    const char* ID_REQUEST = "ID";
    const char* ID_EXPECT = "MotionGranular";
    const char* START_FEED_REQUEST = "FEED_START";
    const char* STOP_FEED_REQUEST = "FEED_STOP";
    const char* FEED_EXPECT = "OK";

    const int64_t ID_TIMEOUT = 2000;

    int sockfd_;
    sockaddr_in sockaddr_;

    MotionRemoteStatus status_;
    MotionAxes axesState_;
    std::mutex timerMutex_;
};