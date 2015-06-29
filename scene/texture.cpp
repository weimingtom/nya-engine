//https://code.google.com/p/nya-engine/

#include "texture.h"
#include "memory/memory_reader.h"
#include "memory/tmp_buffer.h"
#include "formats/tga.h"
#include "formats/dds.h"
#include "formats/ktx.h"

namespace nya_scene
{

inline void bgr_to_rgb(unsigned char *data,size_t data_size,int channels)
{
    if(!data)
        return;

    unsigned char *data2=data+2;
    for(size_t i=0;i<data_size;i+=channels)
    {
        unsigned char tmp=data[i];
        data[i]=data2[i];
        data2[i]=tmp;
    }
}

int texture::m_load_ktx_mip_offset=0;

bool texture::load_ktx(shared_texture &res,resource_data &data,const char* name)
{
    if(!data.get_size())
        return false;

    if(data.get_size()<12)
        return false;

    if(memcmp((const char *)data.get_data()+1,"KTX ",4)!=0)
        return false;

    nya_formats::ktx ktx;
    const size_t header_size=ktx.decode_header(data.get_data(),data.get_size());
    if(!header_size)
    {
        nya_log::log()<<"unable to load ktx: invalid or unsupported ktx header in file "<<name<<"\n";
        return false;
    }

    nya_render::texture::color_format cf;

    switch(ktx.pf)
    {
        case nya_formats::ktx::rgb: cf=nya_render::texture::color_rgb; break;
        case nya_formats::ktx::rgba: cf=nya_render::texture::color_rgba; break;
        case nya_formats::ktx::bgra: cf=nya_render::texture::color_bgra; break;

        case nya_formats::ktx::etc1: cf=nya_render::texture::etc1; break;
        case nya_formats::ktx::etc2: cf=nya_render::texture::etc2; break;
        case nya_formats::ktx::etc2_eac: cf=nya_render::texture::etc2_eac; break;
        case nya_formats::ktx::etc2_a1: cf=nya_render::texture::etc2_a1; break;

        case nya_formats::ktx::pvr_rgb2b: cf=nya_render::texture::pvr_rgb2b; break;
        case nya_formats::ktx::pvr_rgb4b: cf=nya_render::texture::pvr_rgb4b; break;
        case nya_formats::ktx::pvr_rgba2b: cf=nya_render::texture::pvr_rgba2b; break;
        case nya_formats::ktx::pvr_rgba4b: cf=nya_render::texture::pvr_rgba4b; break;

        default: nya_log::log()<<"unable to load ktx: unsupported color format in file "<<name<<"\n"; return false;
    }

    const int mip_off=m_load_ktx_mip_offset>=ktx.mipmap_count?0:m_load_ktx_mip_offset;
    char *d=(char *)ktx.data;
    nya_memory::memory_reader r(ktx.data,ktx.data_size);
    for(unsigned int i=0;i<ktx.mipmap_count;++i)
    {
        const unsigned int size=r.read<unsigned int>();
        if(i>=mip_off)
        {
            memmove(d,r.get_data(),size);
            d+=size;
        }
        r.skip(size);
    }

    const int width=ktx.width>>mip_off;
    const int height=ktx.height>>mip_off;
    return res.tex.build_texture(ktx.data,width>0?width:1,height>0?height:1,cf,ktx.mipmap_count-mip_off);
}

bool texture::m_load_dds_flip=false;
int texture::m_load_dds_mip_offset=0;

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
    {
        nya_log::log()<<"unable to load dds: invalid or unsupported dds header in file "<<name<<"\n";
        return false;
    }

    if(dds.pf!=nya_formats::dds::palette8_rgba && dds.pf!=nya_formats::dds::palette4_rgba) //ToDo
    {
        for(int i=0;i<m_load_dds_mip_offset && dds.mipmap_count > 1;++i)
        {
            dds.data=(char *)dds.data+dds.get_mip_size(0);
            if(dds.width>1)
                dds.width/=2;
            if(dds.height>1)
                dds.height/=2;
            --dds.mipmap_count;
        }
    }

    nya_memory::tmp_buffer_ref tmp_buf;

    int mipmap_count=dds.need_generate_mipmaps?-1:dds.mipmap_count;
    nya_render::texture::color_format cf;
    switch(dds.pf)
    {
        case nya_formats::dds::dxt1: cf=nya_render::texture::dxt1; break;
        case nya_formats::dds::dxt2:
        case nya_formats::dds::dxt3: cf=nya_render::texture::dxt3; break;
        case nya_formats::dds::dxt4:
        case nya_formats::dds::dxt5: cf=nya_render::texture::dxt5; break;

        case nya_formats::dds::bgra: cf=nya_render::texture::color_bgra; break;
        case nya_formats::dds::greyscale: cf=nya_render::texture::greyscale; break;

        case nya_formats::dds::bgr:
        {
            bgr_to_rgb((unsigned char*)dds.data,dds.data_size,3);
            cf=nya_render::texture::color_rgb;
        }
        break;

        case nya_formats::dds::palette8_rgba:
        {
            if(dds.mipmap_count!=1 || dds.type!=nya_formats::dds::texture_2d) //ToDo
            {
                nya_log::log()<<"unable to load dds: uncomplete palette8_rgba support, unable to load file "<<name<<"\n";
                return false;
            }

            cf=nya_render::texture::color_rgba;
            dds.data_size=dds.width*dds.height*4;
            tmp_buf.allocate(dds.data_size);
            dds.decode_palette8_rgba(tmp_buf.get_data());
            dds.data=tmp_buf.get_data();
            dds.pf=nya_formats::dds::bgra;
        }
        break;

        default: nya_log::log()<<"unable to load dds: unsupported color format in file "<<name<<"\n"; return false;
    }

    bool result=false;

    switch(dds.type)
    {
        case nya_formats::dds::texture_2d:
        {
            if(cf>=nya_render::texture::dxt1 && !nya_render::texture::is_dxt_supported())
            {
                tmp_buf.allocate(dds.get_decoded_size());
                dds.decode_dxt(tmp_buf.get_data());
                dds.data_size=tmp_buf.get_size();
                dds.data=tmp_buf.get_data();
                cf=nya_render::texture::color_rgba;
                dds.pf=nya_formats::dds::bgra;
                if(mipmap_count>1)
                    mipmap_count= -1;
            }

            if(m_load_dds_flip)
            {
                nya_memory::tmp_buffer_scoped tmp_data(dds.data_size);
                dds.flip_vertical(dds.data,tmp_data.get_data());
                result=res.tex.build_texture(tmp_data.get_data(),dds.width,dds.height,cf,mipmap_count);
            }
            else
                result=res.tex.build_texture(dds.data,dds.width,dds.height,cf,mipmap_count);
        }
        break;

        case nya_formats::dds::texture_cube:
        {
            if(cf>=nya_render::texture::dxt1 && !nya_render::texture::is_dxt_supported())
            {
                tmp_buf.allocate(dds.get_decoded_size());
                dds.decode_dxt(tmp_buf.get_data());
                dds.data_size=tmp_buf.get_size();
                dds.data=tmp_buf.get_data();
                cf=nya_render::texture::color_rgba;
                dds.pf=nya_formats::dds::bgra;
                if(mipmap_count>1)
                    mipmap_count= -1;
            }

            const void *data[6];
            for(int i=0;i<6;++i)
                data[i]=(const char *)dds.data+i*dds.data_size/6;
            result=res.tex.build_cubemap(data,dds.width,dds.height,cf,mipmap_count);
        }
        break;

        default:
        {
            nya_log::log()<<"unable to load dds: unsupported texture type in file "<<name<<"\n";
            tmp_buf.free();
            return false;
        }
    }

    tmp_buf.free();
    return result;
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
        default: nya_log::log()<<"unable to load tga: unsupported color format in file "<<name<<"\n"; return false;
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
            nya_log::log()<<"unable to load tga: unable to decode rle in file "<<name<<"\n";
            return false;
        }

        color_data=tmp_data.get_data();
    }
    else if(header_size+tga.uncompressed_size>data.get_size())
    {
        nya_log::log()<<"unable to load tga: lack of data, probably corrupted file "<<name<<"\n";
        return false;
    }

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
                tmp_data.copy_from(color_data,tga.uncompressed_size);

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
            bgr_to_rgb((uchar*)color_data,tga.uncompressed_size,3);
    }

    const bool result=res.tex.build_texture(color_data,tga.width,tga.height,color_format);
    tmp_data.free();

    return result;
}

bool texture_internal::set(int slot) const
{
    if(!m_shared.is_valid())
    {
        nya_render::texture::unbind(slot);
        return false;
    }

    m_last_slot=slot;
    m_shared->tex.bind(slot);

    return true;
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

texture::color_format texture::get_format() const
{
    if(!internal().get_shared_data().is_valid())
        return nya_render::texture::color_rgba;

    return internal().get_shared_data()->tex.get_color_format();
}

nya_memory::tmp_buffer_ref texture::get_data() const
{
    nya_memory::tmp_buffer_ref result;
    if(internal().get_shared_data().is_valid())
        internal().get_shared_data()->tex.get_data(result);
    return result;
}

bool texture::is_cubemap() const
{
    if(!internal().get_shared_data().is_valid())
        return false;

    return internal().get_shared_data()->tex.is_cubemap();
}

bool texture::build(const void *data,unsigned int width,unsigned int height,color_format format)
{
    texture_internal::shared_resources::shared_resource_mutable_ref ref;
    if(m_internal.m_shared.get_ref_count()==1 && !m_internal.m_shared.get_name())  //was created and unique
    {
        ref=texture_internal::shared_resources::modify(m_internal.m_shared);
        return ref->tex.build_texture(data,width,height,format);
    }

    m_internal.unload();

    ref=m_internal.get_shared_resources().create();
    if(!ref.is_valid())
        return false;

    m_internal.m_shared=ref;
    return ref->tex.build_texture(data,width,height,format);
}

bool texture::update_region(const void *data,unsigned int x,unsigned int y,unsigned int width,unsigned int height,int mip)
{
    texture_internal::shared_resources::shared_resource_mutable_ref ref;
    if(m_internal.m_shared.get_ref_count()==1 && !m_internal.m_shared.get_name())  //was created and unique
    {
        ref=texture_internal::shared_resources::modify(m_internal.m_shared);
        return ref->tex.update_region(data,x,y,width,height);
    }

    if(!m_internal.m_shared.is_valid())
        return false;

    nya_render::texture::color_format f=m_internal.m_shared->tex.get_color_format();
    const unsigned int w=m_internal.m_shared->tex.get_width();
    const unsigned int h=m_internal.m_shared->tex.get_height();

    nya_memory::tmp_buffer_ref buf;
    if(!m_internal.m_shared->tex.get_data(buf))
        return false;

    nya_memory::tmp_buffer_scoped sbuf(buf);
    m_internal.unload();
    if(!build(sbuf.get_data(),w,h,f))
        return false;

    return update_region(data,x,y,width,height,mip);
}

inline void rgba_to_rgb(unsigned char *data,size_t data_size)
{
    const unsigned char *src=data;
    unsigned char *dst=data;

    data_size/=4;
    for(size_t i=0;i<data_size;++i,src+=4,dst+=3)
        memcpy(dst,src,3);
}

inline void rgb_to_rgba(const unsigned char *src,unsigned char *dst,size_t data_size)
{
    data_size/=3;
    for(size_t i=0;i<data_size;++i,src+=3,dst+=4)
    {
        memcpy(dst,src,3);
        dst[3]=255;
    }
}

bool texture::update_region(const texture_proxy &source,unsigned int x,unsigned int y,int mip)
{
    if(!source.is_valid())
        return false;

    if(x+source->get_width()>get_width() || y+source->get_height()>get_height())
        return false;

    const nya_memory::tmp_buffer_scoped buf(source->get_data());
    if(!buf.get_size())
        return false;

    if(source->get_format()!=get_format())
    {
        if(get_format()>=nya_render::texture::greyscale || source->get_format()>=nya_render::texture::greyscale)
            return false; //ToDo?

        if((get_format()==nya_render::texture::color_bgra && source->get_format()==nya_render::texture::color_rgba)
           || (get_format()==nya_render::texture::color_rgba && source->get_format()==nya_render::texture::color_bgra))
            bgr_to_rgb((unsigned char *)buf.get_data(),buf.get_size(),4);
        else if(get_format()==nya_render::texture::color_rgb)
        {
            if(source->get_format()==nya_render::texture::color_bgra)
                bgr_to_rgb((unsigned char *)buf.get_data(),buf.get_size(),4);
            rgba_to_rgb((unsigned char *)buf.get_data(),buf.get_size());
        }
        else if(source->get_format()==nya_render::texture::color_rgb)
        {
            if(get_format()==nya_render::texture::color_bgra)
                bgr_to_rgb((unsigned char *)buf.get_data(),buf.get_size(),3);

            nya_memory::tmp_buffer_scoped buf2(source->get_width()*source->get_height()*4);
            rgb_to_rgba((unsigned char *)buf.get_data(),(unsigned char *)buf2.get_data(),buf.get_size());
            return update_region(buf2.get_data(),x,y,source->get_width(),source->get_height(),mip);
        }
        else
            return false;
    }

    return update_region(buf.get_data(),x,y,source->get_width(),source->get_height(),mip);
}

bool texture::update_region(const texture &source,unsigned int x,unsigned int y,int mip)
{
    return update_region(nya_scene::texture_proxy(source),x,y,mip);
}

}
