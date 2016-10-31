#include "PngDecoder.hpp"

#include <istream>
#include <sstream>

PngDecoder::PngDecoder() {
    mDecodedData = 0;
    mDecodedSize = 0;
    mWidth = 0;
    mHeight = 0;
}

PngDecoder::~PngDecoder() {
//    delete []mDecodedData;
}


void read_data_fn(png_structp png_ptr, png_bytep data, png_size_t length) {
    png_voidp io_ptr = png_get_io_ptr(png_ptr);
    if(io_ptr == NULL) {
        printf("PNG: Failed to png_get_io_ptr\n");
        return;
    }

    //Cast the pointer to std::istream* and read 'length' bytes into 'data'
    ((std::istringstream*)io_ptr)->read((char*)data, length);
}


void PngDecoder::decode(char *data, size_t length) {
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png) {
        printf("PNG: Failed to png_create_read_struct\n");
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        printf("PNG: Failed to png_create_info_struct\n");
        png_destroy_read_struct(&png, NULL, NULL);
        return;
    }

    //TODO: add error handling
//    setjmp(png_jmpbuf(png));

    std::istringstream dataStream(std::string((char*)data, length));
    png_set_read_fn(png, &dataStream, read_data_fn);
    png_read_info(png, info);

    mWidth      = png_get_image_width(png, info);
    mHeight     = png_get_image_height(png, info);
    mColorType  = png_get_color_type(png, info);
    mBitDepth   = png_get_bit_depth(png, info);
    mChannels   = png_get_channels(png, info);

    // Read any color_type into 8bit depth, RGBA format.
    // See http://www.libpng.org/pub/png/libpng-manual.txt
    if(mBitDepth == 16)
        png_set_strip_16(png);

    if(mColorType == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if(mColorType == PNG_COLOR_TYPE_GRAY && mBitDepth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if(png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    // These color_type don't have an alpha channel then fill it with 0xff.
    if(mColorType == PNG_COLOR_TYPE_RGB ||
       mColorType == PNG_COLOR_TYPE_GRAY ||
       mColorType == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if(mColorType == PNG_COLOR_TYPE_GRAY ||
       mColorType == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    png_size_t bytesPerRow = png_get_rowbytes(png,info);
    mDecodedSize = mHeight * bytesPerRow;
    mDecodedData = new unsigned char[mDecodedSize];

    mRowPointers = new png_bytep[mHeight];
    for (uint32_t i=0; i < mHeight; i++)
    {
        mRowPointers[i] = mDecodedData + i * bytesPerRow;
    }
    png_read_image(png, mRowPointers);
}

unsigned char *PngDecoder::getDecodedData() {
    return mDecodedData;
}

size_t PngDecoder::getDecodedSize() {
    return mDecodedSize;
}

unsigned int PngDecoder::getWidth() {
    return mWidth;
}

unsigned int PngDecoder::getHeight() {
    return mHeight;
}
