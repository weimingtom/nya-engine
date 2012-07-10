//https://code.google.com/p/nya-engine/

// Dependency: libdevil-dev

#include "shared_textures.h"
#include "resources.h"
#include "memory/tmp_buffer.h"

#include <IL/il.h>

namespace
{

int init_devil()
{
    static bool initialised=false;
    if(initialised)
        return true;

    const int version = ilGetInteger(IL_VERSION_NUM);
    if(version < IL_VERSION)
    {
        nya_resources::get_log()<<"Incorrect devil so/dll version. Current: "
                                <<version<<" required: "<<IL_VERSION<<"\n";
        return false;
    }

    ilInit();

    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
    //ilEnable(IL_KEEP_DXTC_DATA);
    //ilSetInteger(IL_KEEP_DXTC_DATA,IL_TRUE);

    initialised=true;

    return true;
}

}

namespace nya_resources
{

bool shared_textures_manager::fill_resource(const char *name,nya_render::texture &res)
{
    if(!name)
    {
        get_log()<<"unable to load texture: invalid name\n";
        return false;
    }

    if(!init_devil())
    {
        get_log()<<"unable to load texture: unable to initialise DevIL\n";
        return false;
    }

    ILuint texId=ilGenImage();
    ilBindImage(texId);

    nya_resources::resource_data *file_data = nya_resources::get_resources_provider().access(name);
    if(!file_data)
    {
        get_log()<<"unable to load texture: unable to acess resource\n";
        return false;
    }

    nya_memory::tmp_buffer_scoped texture_data(file_data->get_size());
    file_data->read_all(texture_data.get_data());
    file_data->release();

    if(!ilLoadL(IL_TYPE_UNKNOWN,texture_data.get_data(),(ILuint)file_data->get_size()))
	{
        get_log()<<"unable to load texture: invalid file\n";
        return false;
	}

    //int  dxtc   = ilGetInteger ( IL_DXTC_DATA_FORMAT );
    //bool comp   = (dxtc == IL_DXT1) || (dxtc == IL_DXT2) || (dxtc == IL_DXT3) || (dxtc == IL_DXT4) || (dxtc == IL_DXT5);

    const int width=ilGetInteger(IL_IMAGE_WIDTH);
    const int height=ilGetInteger(IL_IMAGE_HEIGHT);
    const ILubyte *data=ilGetData();

    nya_render::texture::color_format format;
    const int fmt=ilGetInteger(IL_IMAGE_FORMAT);   

    switch(fmt)
    {
        case IL_RGB:
            format=nya_render::texture::color_rgb;
            break;
            
        case IL_RGBA:
            format=nya_render::texture::color_rgba;
            break;

        case IL_BGR:
            format=nya_render::texture::color_bgr;
            break;
            
        case IL_BGRA:
            format=nya_render::texture::color_bgra;
            break;
            
        default:
            get_log()<<"unable to load texture: unsupported format\n";
            ilDeleteImage(texId);
            return false;
    }

    res.build_texture(data,width,height,format);

    ilDeleteImage(texId);
    return true;
}

bool shared_textures_manager::release_resource(nya_render::texture &res)
{
    res.release();
    return true;
}

shared_textures_manager &get_shared_textures()
{
    static shared_textures_manager manager;
    return manager;
}

}
