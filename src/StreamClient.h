#ifndef IOS_MINICAP_STREAMCLIENT_HPP
#define IOS_MINICAP_STREAMCLIENT_HPP

#include <cstdio>
#include <cstdint>

#include "FrameListener.hpp"
#include "minicap.hpp"

struct StreamClientImpl;

class StreamClient {
public:
    StreamClient();
    ~StreamClient();
    void start();
    void stop();
    void newFrame(char* data, size_t size, uint32_t width, uint32_t height);
    bool setupDevice(const char *udid);
    void setFrameListener(FrameListener *listener);

    void getFrame(Frame *frame);

private:
    StreamClientImpl *impl;
    char* mData;
    size_t mSize;
    uint32_t mWidth;
    uint32_t mHeight;
    bool mFrameAvailable;
    FrameListener *mFrameListener;
};


#endif //IOS_MINICAP_STREAMCLIENT_HPP
