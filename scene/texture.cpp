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

void flip_horisontal(unsigned char *data,int width,int height,int bpp)
{
    const int line_size=width*bpp;
    const int half=line_size/2;
    const int size=line_size*height;

    unsigned char tmp[4];

    for(int offset=0;offset<size;offset+=line_size)
    {
        unsigned char *ha=data+offset;
        unsigned char *hb=ha+line_size-bpp;

        for(int w=0;w<half;w+=bpp)
        {
            unsigned char *a=ha+w;
            unsigned char *b=hb-w;
            memcpy(tmp,a,bpp);
            memcpy(a,b,bpp);
            memcpy(b,tmp,bpp);
        }
    }
}

void flip_vertical(unsigned char *data,int width,int height,int bpp)
{
    const int line_size=width*bpp;
    const int top=line_size*(height-1);
    const int half=line_size*height/2;

    unsigned char tmp[4];

    for(int offset=0;offset<half;offset+=line_size)
    {
        unsigned char *ha=data+offset;
        unsigned char *hb=data+top-offset;

        for(int w=0;w<line_size;w+=bpp)
        {
            unsigned char *a=ha+w;
            unsigned char *b=hb+w;
            memcpy(tmp,a,bpp);
            memcpy(a,b,bpp);
            memcpy(b,tmp,bpp);
        }
    }
}

bool texture::load_tga(shared_texture &res,resource_data &data,const char* name)
{
    if(!data.get_size())
        return false;

    nya_memory::memory_reader reader(data.get_data(),data.get_size());

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

    //const short x_origin=
        reader.read<short>();
    //const short y_origin=
        reader.read<short>();
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
        color_format=nya_render::texture::greyscale;
    else
        return false;

    if(rle)
    {
        nya_memory::tmp_buffer_scoped color_data(color_data_size);

        const unsigned char *cur=(unsigned char *)reader.get_data();
        const unsigned char *const last=cur+reader.get_remained();

        for(size_t i=0;i<color_data_size;)
        {
            if(cur>=last)
                return false;

            if(*cur & 0x80)
            {
                unsigned char len= *cur -127;
                ++cur;

                if(cur>=last+channels)
                    return false;

                for(unsigned char j=0;j<len;++j,i+=channels)
                    memcpy(color_data.get_data(i),cur,channels);

                cur+=channels;
            }
            else // raw
            {
                unsigned char len= *cur +1;
                ++cur;

                if(cur>=last+len*channels)
                    return false;

                for(unsigned char j=0;j<len;++j,i+=channels,cur+=channels)
                    memcpy(color_data.get_data(i),cur,channels);
            }
        }

        if(color_format==nya_render::texture::color_rgb)
            rgb_to_bgr((unsigned char*)color_data.get_data(),color_data_size);

        if(imagedescriptor & 0x10)
            flip_horisontal((unsigned char*)color_data.get_data(),width,height,channels);

        if(imagedescriptor & 0x20)
            flip_vertical((unsigned char*)color_data.get_data(),width,height,channels);

        res.tex.build_texture(color_data.get_data(),width,height,color_format);
    }
    else
    {
        if(!reader.check_remained(color_data_size))
            return false;

        if(color_format==nya_render::texture::color_rgb)
            rgb_to_bgr((unsigned char*)reader.get_data(),color_data_size);

        if(imagedescriptor & 0x10)
            flip_horisontal((unsigned char*)reader.get_data(),width,height,channels);

        if(imagedescriptor & 0x20)
            flip_vertical((unsigned char*)reader.get_data(),width,height,channels);

        res.tex.build_texture(reader.get_data(),width,height,color_format);
    }

    data.free();

    return true;
}

void texture_internal::set(int slot) const
{
    if(!m_shared.is_valid())
    {
        nya_render::texture::select_multitex_slot(slot);
        nya_render::texture::unbind();
        return;
    }

    m_last_slot=slot;
    nya_render::texture::select_multitex_slot(slot);
    m_shared->tex.bind();
}

void texture_internal::unset() const
{
    if(!m_shared.is_valid())
        return;

    nya_render::texture::select_multitex_slot(m_last_slot);
    m_shared->tex.unbind();
}

unsigned int texture::get_width() const
{
    if( !internal().get_shared_data().is_valid() )
        return 0;

    return internal().get_shared_data()->tex.get_width();
}

unsigned int texture::get_height() const
{
    if(!internal().get_shared_data().is_valid())
        return 0;

    return internal().get_shared_data()->tex.get_height();
}

void texture::build(const void *data,unsigned int width,unsigned int height,color_format format)
{
    if(!data)
	{
		m_internal.unload();
        return;
	}

    texture_internal::shared_resources::shared_resource_mutable_ref ref=m_internal.get_shared_resources().create();
    if(!ref.is_valid())
	{
		m_internal.unload();
        return;
	}

    ref->tex.build_texture(data,width,height,format);

    m_internal.m_shared=ref;
}

}
