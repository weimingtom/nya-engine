//https://code.google.com/p/nya-engine/

#pragma once

#include <vector>

namespace nya_formats
{

struct dds
{
    unsigned int width;
    unsigned int height;

    unsigned int mipmap_count;

    enum pixel_format
    {
        dxt1,
        dxt2,
        dxt3,
        dxt4,
        dxt5,
        bgra,
        bgr
    };

    pixel_format pf;

    const void *data;
    size_t data_size;
    size_t mip0_data_size;

    dds(): width(0),height(0),data(0),mip0_data_size(0),data_size(0),mipmap_count(0) {}

public:
    size_t decode_header(const void *data,size_t size); //0 if invalid
};

}
