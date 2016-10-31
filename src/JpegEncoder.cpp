#include <iostream>
#include "JpegEncoder.hpp"


JpegEncoder::JpegEncoder(size_t width, size_t height) {
    mCompressor = tjInitCompress();
    mQuality = 80;
    mSubsampling = TJSAMP_420;
    mFormat = TJPF_BGRA;
    mBufferSize = tjBufSize(
            width,
            height,
            mSubsampling
    );

    std::cout << "Allocating " << mBufferSize << " bytes for JPEG encoder" << std::endl;

    mEncodedData = tjAlloc(mBufferSize);
    mEncodedSize = 0;
}

JpegEncoder::~JpegEncoder() {
    tjDestroy(mCompressor);
    tjFree(mEncodedData);
}


void JpegEncoder::encode(unsigned char *data, int width, int height) {
    tjCompress2(mCompressor, data, width, 0, height, mFormat,
                &mEncodedData, &mEncodedSize, mSubsampling, mQuality,
                TJFLAG_FASTDCT | TJFLAG_NOREALLOC);
}

unsigned char *JpegEncoder::getEncodedData() {
    return mEncodedData;
}

size_t JpegEncoder::getEncodedSize() {
    return mEncodedSize;
}

unsigned long JpegEncoder::getBufferSize() {
    return mBufferSize;
}
