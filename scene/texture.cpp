//https://code.google.com/p/nya-engine/

#include "texture.h"
#include "memory/memory_reader.h"
#include "memory/tmp_buffer.h"
#include "formats/tga.h"
#include "formats/dds.h"

namespace nya_scene
{

void rgb_to_bgr(unsigned char *data,size_t data_size)
{
    if(!data)
        return;

    unsigned char *data2=data+2;
    for(size_t i=0;i<data_size;i+=3)
    {
        unsigned char tmp=data[i];
        data[i]=data2[i];
        data2[i]=tmp;
    }
}

bool texture::load_dds(shared_texture &res,resource_data &data,const char* name)
{
    if(!data.get_size())
        return false;

    if(data.get_size()<4)
        return false;

    if(memcmp(data.get_data(),"DDS ",4)!=0)
        return false;

    nya_formats::dds dds;
    const size_t header_size=dds.decode_header(data.get_data(),data.get_size());
    if(!header_size)
        return false;

    const int mipmap_count=dds.need_generate_mipmaps?-1:dds.mipmap_count;
    nya_render::texture::color_format cf;
    switch(dds.pf)
    {
        case nya_formats::dds::dxt1: cf=nya_render::texture::dxt1; break;
        case nya_formats::dds::dxt2:
        case nya_formats::dds::dxt3: cf=nya_render::texture::dxt3; break;
        case nya_formats::dds::dxt4:
        case nya_formats::dds::dxt5: cf=nya_render::texture::dxt5; break;
        case nya_formats::dds::bgra: cf=nya_render::texture::color_bgra; break;
        case nya_formats::dds::bgr:
            rgb_to_bgr((unsigned char*)dds.data,dds.data_size);
            cf=nya_render::texture::dxt1;
            break;

        default: return false;
    }

    switch(dds.type)
    {
        case nya_formats::dds::texture_2d: res.tex.build_texture(dds.data,dds.width,dds.height,cf,mipmap_count); break;

        case nya_formats::dds::texture_cube: //ToDo: mipmap_count
        {
            const void *data[6];
            for(int i=0;i<6;++i)
                data[i]=(const char *)dds.data+i*dds.data_size/6;
            res.tex.build_cubemap(data,dds.width,dds.height,cf);
        }
        break;

        default: return false;
    }

    return true;
}

bool texture::load_tga(shared_texture &res,resource_data &data,const char* name)
{
    if(!data.get_size())
        return false;

    nya_formats::tga tga;
    const size_t header_size=tga.decode_header(data.get_data(),data.get_size());
    if(!header_size)
        return false;

    nya_render::texture::color_format color_format;
    switch(tga.channels)
    {
        case 4: color_format=nya_render::texture::color_bgra; break;
        case 3: color_format=nya_render::texture::color_rgb; break;
        case 1: color_format=nya_render::texture::greyscale; break;
        default: return false;
    }

    typedef unsigned char uchar;

    nya_memory::tmp_buffer_ref tmp_data;
    const void *color_data=tga.data;
    if(tga.rle)
    {
        tmp_data.allocate(tga.uncompressed_size);
        if(!tga.decode_rle(tmp_data.get_data()))
        {
            tmp_data.free();
            return false;
        }

        color_data=tmp_data.get_data();
    }
    else if(header_size+tga.uncompressed_size>data.get_size())
        return false;

    if(tga.channels==3 || tga.horisontal_flip || tga.vertical_flip)
    {
        if(!tmp_data.get_data())
        {
            tmp_data.allocate(tga.uncompressed_size);

            if(tga.horisontal_flip || tga.vertical_flip)
            {
                if(tga.horisontal_flip)
                    tga.flip_horisontal(color_data,tmp_data.get_data());

                if(tga.vertical_flip)
                    tga.flip_vertical(color_data,tmp_data.get_data());
            }
            else
                tmp_data.copy_to(color_data,tga.uncompressed_size);

            color_data=tmp_data.get_data();
        }
        else
        {
            if(tga.horisontal_flip)
                tga.flip_horisontal(tmp_data.get_data(),tmp_data.get_data());

            if(tga.vertical_flip)
                tga.flip_vertical(tmp_data.get_data(),tmp_data.get_data());
        }

        if(tga.channels==3)
            rgb_to_bgr((uchar*)color_data,tga.uncompressed_size);
    }

    //const uchar *original_rle=(const uchar *)tga.data;
    nya_memory::tmp_buffer_scoped test(tga.uncompressed_size*2);
    tga.data=color_data;
    tga.compressed_size=tga.encode_rle(test.get_data(),tga.uncompressed_size*2);
    nya_memory::tmp_buffer_scoped test2(tga.uncompressed_size);
    tga.data=test.get_data();
    tga.rle=true;
    tga.decode_rle(test2.get_data());
    color_data=test2.get_data();
    //const uchar *encoded_rle=(const uchar *)tga.data;

    res.tex.build_texture(color_data,tga.width,tga.height,color_format);
    tmp_data.free();
    data.free();

    return true;
}

void texture_internal::set(int slot) const
{
    if(!m_shared.is_valid())
    {
        nya_render::texture::unbind(slot);
        return;
    }

    m_last_slot=slot;
    m_shared->tex.bind(slot);
}

void texture_internal::unset() const
{
    if(!m_shared.is_valid())
        return;

    m_shared->tex.unbind(m_last_slot);
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
