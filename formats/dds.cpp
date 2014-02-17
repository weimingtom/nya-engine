//https://code.google.com/p/nya-engine/

#include "dds.h"
#include "memory/memory_reader.h"
#include "memory/tmp_buffer.h"
#include "resources/resources.h"

//ToDo: mipmaps, cubemaps

namespace nya_formats
{

struct dds_pixel_format
{
    typedef unsigned int uint;

    uint size;
    uint flags;
    uint four_cc;
    uint bpp;
    uint bit_mask[4]; //rgba
};

size_t dds::decode_header(const void *data,size_t size)
{
    *this=dds();

    if(!data || size<128)
        return 0;

    typedef unsigned int uint;

    nya_memory::memory_reader reader(data,size);
    if(!reader.test("DDS ",4))
        return 0;

    if(reader.read<uint>()!=124)
        return 0;

    const uint dds_pixelformat=0x00001000;
    const uint dds_caps=0x00000001;

    const uint flags=reader.read<uint>();
    if(!(flags & dds_pixelformat) || !(flags & dds_caps))
        return 0;

    height=reader.read<uint>();
    width=reader.read<uint>();
    reader.skip(4); //const uint pitch=reader.read<uint>();
    const uint depth=reader.read<uint>();
    if(depth>0) //ToDo
        return 0;

    mipmap_count=reader.read<uint>();
    if(mipmap_count<1)
        return 0;

    reader.skip(44);

    const uint dds_fourcc=0x00000004;
    const uint dds_rgb=0x00000040;
    const uint dds_alpha=0x00000001;

    const dds_pixel_format pf=reader.read<dds_pixel_format>();
    if(pf.flags & dds_fourcc)
    {
        switch(pf.four_cc)
        {
            case '1TXD': this->pf=dxt1; break;
            case '2TXD': this->pf=dxt2; break;
            case '3TXD': this->pf=dxt3; break;
            case '4TXD':this->pf=dxt4; break;
            case '5TXD': this->pf=dxt5; break;
            default: return 0;
        };

        for(uint i=0,w=width,h=height;i<mipmap_count;++i,w/=2,h/=2)
        {
            uint s=(w>4?w:4)/4 * (h>4?h:4)/4 * this->pf==dxt1?8:16;
            if(i==0) this->mip0_data_size=s;
            this->data_size+=s;
        }
    }
    else if(pf.flags & dds_rgb)
    {
        if(pf.bit_mask[0]!=0xff0000 || pf.bit_mask[1]!=0xff00 || pf.bit_mask[2]!=0xff )
            return 0;

        if(pf.flags & dds_alpha)
        {
            if(pf.bpp!=32 || pf.bit_mask[3]!=0xff000000U)
                return 0;

            this->pf=bgra;
        }
        else if(pf.bpp==24)
            this->pf=bgr;
        else
            return 0;

        this->mip0_data_size=width*height*pf.bpp/4;
        for(uint i=0,s=uint(mip0_data_size);i<mipmap_count;++i,s/=4)
            this->data_size+=s;
    }
    else
        return 0;

    reader.seek(128);
    this->data=reader.get_data();

    return reader.get_offset();
}

}
