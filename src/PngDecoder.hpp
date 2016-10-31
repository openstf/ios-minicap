#ifndef IOS_MINICAP_PNGDECODER_HPP
#define IOS_MINICAP_PNGDECODER_HPP


#include <cstdio>
#include <cstdint>

#include <png.h>


class PngDecoder {
public:
    PngDecoder();
    ~PngDecoder();

    void decode(char *data, size_t length);

    unsigned char* getDecodedData();
    size_t getDecodedSize();
    unsigned int getWidth();
    unsigned int getHeight();

private:
    unsigned char* mDecodedData;
    png_bytep* mRowPointers;
    size_t mDecodedSize;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mBitDepth;
    uint32_t mColorType;
    uint32_t mChannels;
};


#endif //IOS_MINICAP_PNGDECODER_HPP
