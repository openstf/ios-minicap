#ifndef IOS_MINICAP_JPEGENCODER_HPP
#define IOS_MINICAP_JPEGENCODER_HPP


#include <cstdio>

#include <turbojpeg.h>

class JpegEncoder {
public:
    JpegEncoder(size_t width, size_t height);
    ~JpegEncoder();

    void encode(unsigned char *data, int width, int height);
    unsigned char* getEncodedData();
    size_t getEncodedSize();
    unsigned long getBufferSize();

private:
    tjhandle mCompressor;
    int mQuality;
    TJSAMP mSubsampling;
    TJPF mFormat;

    unsigned char* mEncodedData;
    size_t mEncodedSize;
    unsigned long mBufferSize;
};


#endif //IOS_MINICAP_JPEGENCODER_HPP
