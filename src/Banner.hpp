#ifndef IOS_MINICAP_BANNER_HPP
#define IOS_MINICAP_BANNER_HPP

#include <cstdio>

#define BANNER_VERSION 1
#define BANNER_SIZE 24


struct DisplayInfo {
    int width = 0;
    int height = 0;
    int orientation = 0;
};


enum {
    QUIRK_DUMB            = 1,
    QUIRK_ALWAYS_UPRIGHT  = 2,
    QUIRK_TEAR            = 4,
};


static void
putUInt32LE(char* data, int value) {
    data[0] = (value & 0x000000FF) >> 0;
    data[1] = (value & 0x0000FF00) >> 8;
    data[2] = (value & 0x00FF0000) >> 16;
    data[3] = (value & 0xFF000000) >> 24;
}


class Banner {
public:
    Banner(DisplayInfo realInfo, DisplayInfo desiredInfo);

    ~Banner();

    char * getData();
    size_t getSize();

private:
    char* mData;
    size_t mSize;
};

#endif //IOS_MINICAP_BANNER_HPP
