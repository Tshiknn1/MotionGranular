/*
  ==============================================================================

    MotionRemote.cpp
    Created: 30 Apr 2025 12:27:12am
    Author:  Tshiknn

  ==============================================================================
*/

#include "MotionRemote.h"

#if defined(_WIN32)
    #define _WIN32_WINNT 0x601
    #include <WinSock2.h>
#else
    #include <sys/socket.h>
    #include <sys/types.h>
#endif
#include <string>


MotionRemote::MotionRemote(const char* ipAddress, int port) {
#ifdef _WIN32
    WSADATA wsaData;

    // Initialize Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        OutputDebugString("WSAStartup failed: ");
        OutputDebugString(std::to_string(result).c_str());
        OutputDebugString("\n");
    }
#endif

    OutputDebugString(ipAddress);
    OutputDebugString(":");
    OutputDebugString(std::to_string(port).c_str());
    OutputDebugString("\n");
    sockfd_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_.sin_family = AF_INET;
    sockaddr_.sin_port = htons(port);
    sockaddr_.sin_addr.s_addr = inet_addr(ipAddress);

    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 2000;
    setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(timeval));

    status_ = MotionRemoteStatus::IDLE;
}


MotionRemote::~MotionRemote() {
    if (isTimerRunning()) {
        stopTimer();
    }

    closesocket(sockfd_);
#ifdef _WIN32
    WSACleanup();
#endif
}


void MotionRemote::update(MotionRemote::MotionAxes* r, bool requestNeeded) {
    uint8_t buffer[MOTION_REMOTE_BUF_SIZE];
    int n;
    sockaddr_in respaddr;

    if (requestNeeded) {
        n = request(DATA_REQUEST, (char*)buffer, MOTION_REMOTE_BUF_SIZE, &respaddr);
    }
    else {
        int respaddr_size = sizeof(respaddr);
        n = recvfrom(sockfd_, (char*)buffer, MOTION_REMOTE_BUF_SIZE, 0, (struct sockaddr*)&respaddr, &respaddr_size);
    }

    if (n != -1) {
        r->ax.store((buffer[0] << 8) | buffer[1]);
        r->ay.store((buffer[2] << 8) | buffer[3]);
        r->az.store((buffer[4] << 8) | buffer[5]);
        r->gx.store((buffer[6] << 8) | buffer[7]);
        r->gy.store((buffer[8] << 8) | buffer[9]);
        r->gz.store((buffer[10] << 8) | buffer[11]);
    }
}


void MotionRemote::fetch(MotionRemote::MotionAxes* r) {
    r->ax.store(axesState_.ax.load());
    r->ay.store(axesState_.ay.load());
    r->az.store(axesState_.az.load());
    r->gx.store(axesState_.gx.load());
    r->gy.store(axesState_.gy.load());
    r->gz.store(axesState_.gz.load());
}


void MotionRemote::verify() {
    char* buffer = new char[1024];
    int n;
    sockaddr_in respaddr;

    OutputDebugString("STARTING REQUEST\n");
    n = request(ID_REQUEST, buffer, 1024, &respaddr);
    OutputDebugString("FINISHING REQUEST\n");

    if (n < 1024 && n > 0) {
        buffer[n] = '\0';
    }

    OutputDebugString(std::to_string(n).c_str());
    OutputDebugString(buffer);
    OutputDebugString("\n");

    if (strcmp(buffer, ID_EXPECT) == 0) {
        status_ = MotionRemoteStatus::CONNECTED;
    }
    else {
        status_ = MotionRemoteStatus::BAD_RESPONSE;
    }
}


int MotionRemote::request(const char* req, char* buffer, int bufSize, sockaddr_in* respaddr) {
    sendto(sockfd_, req, strlen(req), 0, (struct sockaddr*)&sockaddr_, sizeof(sockaddr_));

    int n;
    int64_t startTime = juce::Time::currentTimeMillis();

    int respaddr_size = sizeof(*respaddr);

    do {
        n = recvfrom(sockfd_, buffer, 1024, 0, (struct sockaddr*)respaddr, &respaddr_size);

        if (juce::Time::currentTimeMillis() - startTime > ID_TIMEOUT) {
            break;
        }
    } while (n == -1);

    return n;
}


bool MotionRemote::toggleFeed(bool on) {
    if (!on && isTimerRunning()) {
        stopTimer();
    }

    char buffer[1024];
    sockaddr_in respaddr;
    int n;

    n = request(on ? START_FEED_REQUEST : STOP_FEED_REQUEST, buffer, 1024, &respaddr);

    if (n > 0 && n < 1024) {
        buffer[n] == '\0';
        if (strcmp(buffer, FEED_EXPECT) != 0) {
            return false;
        }
    } else {
        return false;
    }

    return true;
}


//void MotionRemote::timerCallback() {
//    OutputDebugString("callback time\n");
//    uint8_t* buffer = new uint8_t[1024];
//    sockaddr_in respaddr;
//    int respaddr_size = sizeof(respaddr);
//    int n;
//
//    n = recvfrom(sockfd_, (char*)buffer, 1024, 0, (struct sockaddr*)&respaddr, &respaddr_size);
//
//    if (n != -1) {
//        OutputDebugString("got data and updating ptrAxes\n");
//        ptrAxes_->ax.store((buffer[0] << 8) | buffer[1]);
//        ptrAxes_->ay.store((buffer[2] << 8) | buffer[3]);
//        ptrAxes_->az.store((buffer[4] << 8) | buffer[5]);
//        ptrAxes_->gx.store((buffer[6] << 8) | buffer[7]);
//        ptrAxes_->gy.store((buffer[8] << 8) | buffer[9]);
//        ptrAxes_->gz.store((buffer[10] << 8) | buffer[11]);
//    }
//    else {
//        OutputDebugString("no data gotten\n");
//    }
//}


void MotionRemote::hiResTimerCallback() {
    if (!timerMutex_.try_lock()) { return; }

    update(&axesState_, false);

    timerMutex_.unlock();
}