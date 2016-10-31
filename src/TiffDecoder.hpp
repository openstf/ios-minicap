//
// Created by pavlov on 20/09/16.
//

#ifndef IOS_MINICAP_TIFFDECODER_HPP
#define IOS_MINICAP_TIFFDECODER_HPP


#include <cstdio>
#include <sstream>


class TiffDecoder {
public:
    TiffDecoder();
    ~TiffDecoder();

//    void decode(TIFF *tiff);
    void decode(unsigned char *data, size_t length);

    unsigned char* getDecodedData();
    size_t getDecodedSize();
    unsigned int getWidth();
    unsigned int getHeight();

private:
    unsigned char* mDecodedData;
    size_t mDecodedSize;
    unsigned int mWidth;
    unsigned int mHeight;
};


#endif //IOS_MINICAP_TIFFDECODER_HPP
