//https://code.google.com/p/nya-engine/

#include "shared_textures.h"
#include "resources.h"

#include	<IL/il.h>
#include	<IL/ilu.h>

/*
    libdevil1c2 1.7.8.2
    libdevil-dev 1.7.8-2

    ToDo: read files throught resource provider
 
    ILboolean ilLoadL     ( ILenum type, ILvoid * data, ILuint size );
 
    IL_TYPE_UNKNOWN
*/

namespace
{

int init_devil()
{
    static bool initialised=false;
    if(initialised)
        return true;

    if(ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
    {
        nya_resources::get_log()<<"Incorrect devil so/dll version. IL_VERSION="<<IL_VERSION<<"\n";
        return false;
    }

    if(iluGetInteger(ILU_VERSION_NUM) < ILU_VERSION)
    {
        nya_resources::get_log()<<"Incorrect ilu so/dll version. IL_VERSION="<<IL_VERSION<<"\n";
        return false;
    }

    ilInit();
    iluInit();
    ilEnable(IL_KEEP_DXTC_DATA);
    ilSetInteger(IL_KEEP_DXTC_DATA,IL_TRUE);

    initialised=true;

    return true;
}

}

namespace nya_resources
{

bool shared_textures_manager::fill_resource(const char *name,nya_render::texture &res)
{
    if(!name)
        return false;

    if(!init_devil())
        return false;

    ILuint texId=ilGenImage();
    ilBindImage(texId);

    if(!ilLoadImage(name))
	{
        get_log()<<"tex_load_error1\n";
        return false;
	}

    int  width  = ilGetInteger ( IL_IMAGE_WIDTH      );
    int  height = ilGetInteger ( IL_IMAGE_HEIGHT     );
    /*
    int  depth  = ilGetInteger ( IL_IMAGE_DEPTH      );
    int  type   = ilGetInteger ( IL_IMAGE_TYPE       );
    int  fmt    = ilGetInteger ( IL_IMAGE_FORMAT     );
    int  bpp    = ilGetInteger ( IL_IMAGE_BPP     );
    int  dxtc   = ilGetInteger ( IL_DXTC_DATA_FORMAT );
    bool comp   = (dxtc == IL_DXT1) || (dxtc == IL_DXT2) || (dxtc == IL_DXT3) || (dxtc == IL_DXT4) || (dxtc == IL_DXT5);
    */

    ILubyte *data=ilGetData ();

    nya_render::texture::color_format format;
    
    int bpp=ilGetInteger(IL_IMAGE_BPP)*8;
    
    if(bpp==24)
        format=nya_render::texture::color_rgb;
    else if(bpp==32)
        format=nya_render::texture::color_rgba;
    else
        return false;

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
