//
// Created by pavlov on 28/10/16.
//

#ifndef IOS_MINICAP_MINICAP_HPP
#define IOS_MINICAP_MINICAP_HPP

#include <cstdlib>


enum Format {
    FORMAT_NONE          = 0x01,
    FORMAT_CUSTOM        = 0x02,
    FORMAT_TRANSLUCENT   = 0x03,
    FORMAT_TRANSPARENT   = 0x04,
    FORMAT_OPAQUE        = 0x05,
    FORMAT_RGBA_8888     = 0x06,
    FORMAT_RGBX_8888     = 0x07,
    FORMAT_RGB_888       = 0x08,
    FORMAT_RGB_565       = 0x09,
    FORMAT_BGRA_8888     = 0x0a,
    FORMAT_RGBA_5551     = 0x0b,
    FORMAT_RGBA_4444     = 0x0c,
    FORMAT_UNKNOWN       = 0x00,
};


struct Frame {
    void const* data;
    Format format;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t bpp;
    size_t size;
};


#endif //IOS_MINICAP_MINICAP_HPP
