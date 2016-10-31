//
// Created by pavlov on 20/09/16.
//

#include "TiffDecoder.hpp"

#include <tiffio.h>
#include <tiffio.hxx>


TiffDecoder::TiffDecoder() {
    mDecodedData = 0;
    mDecodedSize = 0;
    mWidth = 0;
    mHeight = 0;
}

TiffDecoder::~TiffDecoder() {
    _TIFFfree(mDecodedData);
}

struct membuf: std::streambuf {
    membuf(char* base, std::ptrdiff_t n) {
        this->setg(base, base, base + n);
    }
};

void TiffDecoder::decode(unsigned char* data, size_t length) {
    std::istringstream mTiffStream(std::string((char*)data,length));
    TIFF* tiff = TIFFStreamOpen("Memory", &mTiffStream);

    uint32* raster = 0;

    TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &mWidth);
    TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &mHeight);
    mDecodedSize = mWidth * mHeight * sizeof(uint32);

    _TIFFfree(mDecodedData);
    raster = (uint32*) _TIFFmalloc(mDecodedSize);
    if (raster != NULL) {
        if (TIFFReadRGBAImageOriented(tiff, mWidth, mHeight, raster, ORIENTATION_TOPLEFT, 0)) {
            mDecodedData = (unsigned char *) raster;
        }
    }
}

unsigned char *TiffDecoder::getDecodedData() {
    return mDecodedData;
}

size_t TiffDecoder::getDecodedSize() {
    return mDecodedSize;
}

unsigned int TiffDecoder::getWidth() {
    return mWidth;
}

unsigned int TiffDecoder::getHeight() {
    return mHeight;
}


