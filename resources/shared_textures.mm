//https://code.google.com/p/nya-engine/

#include "shared_textures.h"
#include "resources.h"
#include "memory/tmp_buffer.h"

#import <Cocoa/Cocoa.h>

namespace nya_resources
{

bool shared_textures_manager::fill_resource(const char *name,nya_render::texture &res)
{
    if(!name)
    {
        get_log()<<"unable to load texture: invalid name\n";
        return false;
    }

    nya_resources::resource_data *file_data = nya_resources::get_resources_provider().access(name);
    if(!file_data)
    {
        get_log()<<"unable to load texture: unable to acess resource\n";
        return false;
    }

    nya_memory::tmp_buffer_scoped texture_data(file_data->get_size());
    file_data->read_all(texture_data.get_data());
    file_data->release();

    NSData * data  = [NSData dataWithBytesNoCopy:texture_data.get_data() 
                        length: file_data->get_size()freeWhenDone:FALSE];
    if (data == nil)
    {
        get_log()<<"unable to load texture: NSData error\n";
        return false;
    }

    NSBitmapImageRep *image=[NSBitmapImageRep imageRepWithData:data];
    if (image == nil)
    {
        get_log()<<"unable to load texture: invalid file\n";
        return false;
    }

    unsigned int bpp=(unsigned int)[image bitsPerPixel];

    nya_render::texture::color_format format;

    if(bpp==24)
        format=nya_render::texture::color_rgb;
    else if(bpp== 32 )
        format=nya_render::texture::color_rgba;
    else
    {
        get_log()<<"unable to load texture: unsupported format\n";
        return false;
    }

    unsigned int width =(unsigned int)[image pixelsWide];
    unsigned int height=(unsigned int)[image pixelsHigh];
    unsigned char *image_data=[image bitmapData];

    res.build_texture(image_data,width,height,format);

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
