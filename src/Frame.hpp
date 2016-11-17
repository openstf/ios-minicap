//
// Created by pavlov on 16/11/16.
//

#ifndef IOS_MINICAP_FRAME_HPP
#define IOS_MINICAP_FRAME_HPP

#include <cstdlib>


enum Format {
    FORMAT_BGRA_8888     = 0x01,
    FORMAT_UNKNOWN       = 0x00,
};


struct Frame {
    void const* data;
    Format format;
    uint32_t width;
    uint32_t height;
    uint32_t bytesPerRow;
    size_t size;
};

#endif //IOS_MINICAP_FRAME_HPP
