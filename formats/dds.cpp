//https://code.google.com/p/nya-engine/

#include "dds.h"
#include "memory/memory_reader.h"
#include "memory/tmp_buffer.h"
#include "resources/resources.h"

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

static void flip_raw(int width,int height,int channels,const void *from_data,void *to_data)
{
    const int line_size=width*channels;
    const int top=line_size*(height-1);

    typedef unsigned char uchar;

    uchar *to=(uchar*)to_data;

    const int size=line_size*height;

    const uchar *from=(const uchar*)from_data;
    for(size_t offset=0;offset<size;offset+=line_size)
    {
        const uchar *ha=from+offset;
        uchar *hb=to+top-offset;
        memcpy(hb,ha,line_size);
    }
}

static void flip_dxt1_block_full(unsigned char *data)
{
    std::swap(data[4],data[7]);
    std::swap(data[5],data[6]);
}

static void flip_dxt3_block_full(unsigned char *data)
{
    std::swap(data[0],data[6]);
    std::swap(data[1],data[7]);
    std::swap(data[2],data[4]);
    std::swap(data[3],data[5]);

    flip_dxt1_block_full(data+8);
}

static void flip_dxt5_block_full(unsigned char *data)
{
    unsigned int line_0_1 = data[2] + 256 * (data[3] + 256 * data[4]);
    unsigned int line_2_3 = data[5] + 256 * (data[6] + 256 * data[7]);
    // swap lines 0 and 1 in line_0_1.
    unsigned int line_1_0 = ((line_0_1 & 0x000fff) << 12) |
    ((line_0_1 & 0xfff000) >> 12);
    // swap lines 2 and 3 in line_2_3.
    unsigned int line_3_2 = ((line_2_3 & 0x000fff) << 12) |
    ((line_2_3 & 0xfff000) >> 12);
    data[2] = line_3_2 & 0xff;
    data[3] = (line_3_2 & 0xff00) >> 8;
    data[4] = (line_3_2 & 0xff0000) >> 16;
    data[5] = line_1_0 & 0xff;
    data[6] = (line_1_0 & 0xff00) >> 8;
    data[7] = (line_1_0 & 0xff0000) >> 16;

    flip_dxt1_block_full(data+8);
}

static void flip_dxt(int width,int height,dds::pixel_format format,const void *from_data,void *to_data)
{
    if(from_data==to_data) //ToDo ?
        return;

    const unsigned int line_size=((width+3)/4)*(format==dds::dxt1?8:16);
    const unsigned char *s=(unsigned char *)from_data;
    unsigned char *d=(unsigned char *)to_data+((height+3)/4-1)*line_size;

    if(height==1)
    {
        memcpy(d,s,line_size);
        return;
    }

    for(int i=0;i<(height+3)/4;++i)
    {
        memcpy(d,s,line_size);

        switch(format)
        {
            case dds::dxt1:
                if(height==2)
                {
                    for(unsigned int k=0;k<line_size;k+=8)
                        std::swap((d+k)[4],(d+k)[5]);
                }
                else
                {
                    for(unsigned int k=0;k<line_size;k+=8)
                        flip_dxt1_block_full(d+k);
                }
                break;

            case dds::dxt2:
            case dds::dxt3:
                for(unsigned int k=0;k<line_size;k+=16)
                    flip_dxt3_block_full(d+k);
                break;

            case dds::dxt4:
            case dds::dxt5:
                for(unsigned int k=0;k<line_size;k+=16)
                    flip_dxt5_block_full(d+k);
                break;

            default: return;
        }

        s+=line_size;
        d-=line_size;
    }
}

void dds::flip_vertical(const void *from_data,void *to_data)
{
    if(!from_data || !to_data)
        return;

    if(type==texture_cube) //ToDo
        return;

    size_t offset=0;
    for(unsigned int i=0,w=width,h=height;i<mipmap_count;++i,w>1?w=w/2:w=1,h>1?h/=2:h=1)
    {
        const void *f=(const char *)from_data+offset;
        void *t=(char *)to_data+offset;

        switch(pf)
        {
            case bgr:
            case bgra:
            {
                flip_raw(w,h,pf==bgr?3:4,f,t);
                offset+=w*h*(pf==bgr?3:4);
            }
            break;

            case dxt1:
            {
                unsigned int s=(w>4?w:4)/4 * (h>4?h:4)/4 * 8;
                flip_dxt(w,h,pf,f,t);
                offset+=s;
            }
            break;

            case dxt2:
            case dxt3:
            case dxt4:
            case dxt5:
            {
                unsigned int s=(w>4?w:4)/4 * (h>4?h:4)/4 * 16;
                flip_dxt(w,h,pf,f,t);
                offset+=s;
            }
            break;
        }
    }
}

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
    if(!mipmap_count)
    {
        need_generate_mipmaps=true;
        mipmap_count=1;
    }

    reader.skip(44);

    const uint dds_fourcc=0x00000004;
    const uint dds_rgb=0x00000040;
    const uint dds_alpha=0x00000001;
    const uint dds_cubemap=0x00000200;

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
            uint s=(w>4?w:4)/4 * (h>4?h:4)/4 * (this->pf==dxt1?8:16);
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

        this->mip0_data_size=width*height*(pf.bpp/8);
        for(uint i=0,w=width,h=height;i<mipmap_count;++i,w>1?w=w/2:w=1,h>1?h/=2:h=1)
            this->data_size+=w*h*(pf.bpp/8);
    }
    else
        return 0;

    unsigned int caps1=reader.read<unsigned int>();
    unsigned int caps2=reader.read<unsigned int>();

    type=texture_2d;
    if(caps2 & dds_cubemap)
    {
        type=texture_cube;
        this->mip0_data_size*=6;
        this->data_size*=6;
    }

    reader.seek(128);
    if(!reader.check_remained(this->data_size))
    {
        this->mipmap_count=-1;
        this->data_size=this->mip0_data_size;
        //probably broken, try load at least first mipmap
        if(!reader.check_remained(this->data_size))
            return 0;
    }

    this->data=reader.get_data();

    return reader.get_offset();
}

}
