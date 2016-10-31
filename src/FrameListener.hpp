#ifndef IOS_MINICAP_FRAMELISTENER_HPP
#define IOS_MINICAP_FRAMELISTENER_HPP

#include <mutex>
#include <condition_variable>
#include <chrono>


class FrameListener {
public:
    FrameListener();
    void stop();
    bool isRunning();
    void onFrameAvailable();
    int waitForFrame();

private:
    bool mRunning;
    std::mutex mMutex;
    std::condition_variable mCondition;
    std::chrono::milliseconds mTimeout;
    int mPendingFrames;
};


#endif //IOS_MINICAP_FRAMELISTENER_HPP
