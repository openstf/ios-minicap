#include "Banner.hpp"

#include <zconf.h>
#include <iostream>


Banner::Banner(DisplayInfo realInfo, DisplayInfo desiredInfo) {
    mSize = BANNER_SIZE;
    unsigned char quirks = 0;
    quirks |= QUIRK_ALWAYS_UPRIGHT;

    // Prepare banner for clients.
    mData = new char[mSize];
    mData[0] = (char) BANNER_VERSION;
    mData[1] = (char) BANNER_SIZE;
    putUInt32LE(mData + 2, getpid());
    putUInt32LE(mData + 6,  realInfo.width);
    putUInt32LE(mData + 10,  realInfo.height);
    putUInt32LE(mData + 14, desiredInfo.width);
    putUInt32LE(mData + 18, desiredInfo.height);
    mData[22] = (char) desiredInfo.orientation;
    mData[23] = quirks;

    std::cout << "== Banner ==" << std::endl;
    std::cout << "version: " << BANNER_VERSION << std::endl;
    std::cout << "size: " << BANNER_SIZE << std::endl;
    std::cout << "pid: " << getpid() << std::endl;
    std::cout << "real width: " << realInfo.width << std::endl;
    std::cout << "real height: " << realInfo.height << std::endl;
    std::cout << "desired width: " << desiredInfo.width << std::endl;
    std::cout << "desired height: " << desiredInfo.height << std::endl;
    std::cout << "orientation: " << desiredInfo.orientation << std::endl;
    std::cout << "quirks: " << (int) quirks << std::endl;

    printf("banner: ");
    for (int i = 0; i < mSize; i++) {
        printf("%x", mData[i]);
    }
    printf("\n");
}


char * Banner::getData() {
    return mData;
}

size_t Banner::getSize() {
    return mSize;
}

Banner::~Banner() {
    delete [] mData;
}
