#include <iostream>
#include "JpegEncoder.hpp"


JpegEncoder::JpegEncoder(Frame *frame) {
    mCompressor = tjInitCompress();
    mQuality = 80;
    mSubsampling = TJSAMP_420;
    mFormat = TJPF_BGRA;
    mBufferSize = tjBufSize(
            frame->width,
            frame->height,
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


void JpegEncoder::encode(Frame *frame) {
    if ( tjCompress2(
             mCompressor,
             (unsigned char*)frame->data,
             frame->width,
             frame->bytesPerRow,
             frame->height,
             mFormat,
             &mEncodedData,
             &mEncodedSize,
             mSubsampling,
             mQuality,
             TJFLAG_FASTDCT | TJFLAG_NOREALLOC) < 0 ) {
        std::cout << "Compress to JPEG failed: " << tjGetErrorStr() << std::endl;

    };
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
