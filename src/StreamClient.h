#ifndef IOS_MINICAP_STREAMCLIENT_HPP
#define IOS_MINICAP_STREAMCLIENT_HPP

typedef struct opaqueCMSampleBuffer *CMSampleBufferRef;

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
    void captureOutput(CMSampleBufferRef buffer);
    bool setupDevice(const char *udid);
    void setFrameListener(FrameListener *listener);

    void lockFrame(Frame *frame);
    void releaseFrame(Frame *frame);
    void getFrame(Frame *frame);

private:
    StreamClientImpl *impl;
    char* mData;
    size_t mSize;
    uint32_t mWidth;
    uint32_t mHeight;
    bool mFrameAvailable;
    FrameListener *mFrameListener;
    std::mutex mMutex;
    CMSampleBufferRef mBuffer;
    CMSampleBufferRef mLockedBuffer;
};


#endif //IOS_MINICAP_STREAMCLIENT_HPP
