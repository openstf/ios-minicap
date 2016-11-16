#ifndef IOS_MINICAP_BANNER_HPP
#define IOS_MINICAP_BANNER_HPP

#include <cstdio>
#include <cstdlib>

#define BANNER_VERSION 1
#define BANNER_SIZE 24


static void putUInt32LE(unsigned char* data, int value) {
    data[0] = (value >> 0) & 0xFF;
    data[1] = (value >> 8) & 0xFF;
    data[2] = (value >> 16) & 0xFF;
    data[3] = (value >> 24) & 0xFF;
}


struct DeviceInfo {
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t orientation = 0;
    float fps;
    float density;
    float xdpi;
    float ydpi;
    bool secure;
    float size;
};


enum {
    QUIRK_DUMB            = 1,
    QUIRK_ALWAYS_UPRIGHT  = 2,
    QUIRK_TEAR            = 4,
};


class Banner {
public:
    Banner(DeviceInfo realInfo, DeviceInfo desiredInfo);

    ~Banner();

    unsigned char * getData();
    size_t getSize();

private:
    unsigned char* mData;
    size_t mSize;
};

#endif //IOS_MINICAP_BANNER_HPP
