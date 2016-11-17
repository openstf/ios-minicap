#include <iostream>
#include "FrameListener.hpp"

FrameListener::FrameListener() {
    mPendingFrames = 0;
    mTimeout = std::chrono::milliseconds(100);
    mRunning = true;
}

void FrameListener::stop() {
    mRunning = false;
}

bool FrameListener::isRunning() {
    return mRunning;
}

void FrameListener::onFrameAvailable() {
    std::unique_lock<std::mutex> lock(mMutex);
    mPendingFrames += 1;
    mCondition.notify_one();
}

int FrameListener::waitForFrame() {
    std::unique_lock<std::mutex> lock(mMutex);

    while (mRunning) {
        if (mCondition.wait_for(lock, mTimeout, [this]{return mPendingFrames > 0;})) {
            return mPendingFrames--;
        }
    }

    return 0;
}

