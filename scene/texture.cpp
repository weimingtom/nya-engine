//https://code.google.com/p/nya-engine/

#include "texture.h"
#include "memory/memory_reader.h"
#include "memory/tmp_buffer.h"

namespace nya_scene
{

void rgb_to_bgr(unsigned char *data,size_t data_size)
{
    unsigned char *data2=data+2;
    for(size_t i=0;i<data_size;i+=3)
    {
        unsigned char tmp=data[i];
        data[i]=data2[i];
        data2[i]=tmp;
    }
}

bool texture::load_tga(shared_texture &res,size_t data_size,const void*data) 
{
    if(!data)
        return false;

    nya_memory::memory_reader reader(data,data_size);

    const char id_length=reader.read<char>();
    reader.skip(id_length);

    const char colourmaptype=reader.read<char>();
    if(colourmaptype!=0)
        return false;

    const char datatypecode=reader.read<char>();
    const short colourmaporigin=reader.read<short>();
    if(colourmaporigin!=0)
        return false;

    const short colourmaplength=reader.read<short>();
    if(colourmaplength!=0)
        return false;

    const char colourmapdepth=reader.read<char>();
    if(colourmapdepth!=0)
        return false;

    const short x_origin=reader.read<short>();
    const short y_origin=reader.read<short>();
    const short width=reader.read<short>();
    const short height=reader.read<short>();
    const char bitsperpixel=reader.read<char>();
    const char imagedescriptor=reader.read<char>();

    int channels = 0;
    bool rle = false;

    if(bitsperpixel==32)
    {
        if(datatypecode==10)
        {
            channels=4;
            rle=true;
        }
        else if(datatypecode==2)
            channels = 4;
    }
    else if(bitsperpixel==24)
    {
        if(datatypecode==10)
        {
            channels=3;
            rle=true;
        }
        else if(datatypecode==2)
            channels = 3;
    }
    else if(bitsperpixel==8)
    {
        if(datatypecode==11)
        {
            channels=1;
            rle=true;
        }
        else if(datatypecode==3)
            channels=1;
    }

    if(channels==0)
        return false;

    const size_t color_data_size=width*height*channels;

    nya_render::texture::color_format color_format;
    if(channels==4)
        color_format=nya_render::texture::color_bgra;
    else if(channels==3)
        color_format=nya_render::texture::color_rgb;
    else if(channels==1)
        color_format=nya_render::texture::color_r;
    else
        return false;

    if(rle)
    {
        nya_memory::tmp_buffer_scoped color_data(color_data_size);

        const unsigned char *cur=(unsigned char *)reader.get_data();

        for(size_t i=0;i<color_data_size;) 
        {
            if(*cur & 0x80)
            {
                unsigned char len= *cur -127; 
                ++cur;

                for(unsigned char j=0;j<len;++j,i+=channels)
                    memcpy(color_data.get_data(i),cur,channels);

                cur+=channels;
            }
            else // raw
            {
                unsigned char len= *cur +1; 
                ++cur;

                for(unsigned char j=0;j<len;++j,i+=channels,cur+=channels)
                    memcpy(color_data.get_data(i),cur,channels);
            }
        }

        if(color_format==nya_render::texture::color_rgb)
            rgb_to_bgr((unsigned char*)color_data.get_data(),color_data_size);

        res.tex.build_texture(color_data.get_data(),width,height,color_format);
    }
    else
    {
        if(!reader.check_remained(color_data_size))
            return false;

        if(color_format==nya_render::texture::color_rgb)
            rgb_to_bgr((unsigned char*)reader.get_data(),color_data_size);

        res.tex.build_texture(reader.get_data(),width,height,color_format);
    }

    return true;
}

void texture::set(int slot) const
{
    m_last_slot=slot;

    if(!m_shared.is_valid())
        return;

    nya_render::texture::select_multitex_slot(slot);
    m_shared->tex.bind();
}

void texture::unset() const
{
    if(!m_shared.is_valid())
        return;

    nya_render::texture::select_multitex_slot(m_last_slot);
    m_shared->tex.unbind();
}

}
