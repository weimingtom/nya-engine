//https://code.google.com/p/nya-engine/

#include "shared_textures.h"
#include "resources.h"

#import <Cocoa/Cocoa.h>

namespace nya_resources
{

bool shared_textures_manager::fill_resource(const char *name,nya_render::texture &res)
{
    //printf(" loading tex %s",name);
    
    NSString *tex_name = [NSString stringWithCString:name encoding:NSUTF8StringEncoding];
    
    NSData * data  = [NSData dataWithContentsOfFile: tex_name];
    if ( data == nil )
    {
        //[[[[self window] windowController] document] get_log ]
        //<< "unable to load texture: "<<fileName.UTF8String<<"\n";
        get_log()<<"\ntex_load_error1";
        return false;
    }
    
    NSBitmapImageRep * image = [NSBitmapImageRep imageRepWithData: data];
    if ( image == nil )
    {
        //[[[[self window] windowController] document] get_log ]
        //<< "unable to load texture: "<<fileName.UTF8String<<"\n";

        get_log()<<"\ntex_load_error2";
        return false;
    }
    
    unsigned int    bitsPerPixel = (unsigned int)[image bitsPerPixel];
    
    nya_render::texture::color_format format;
    
    if (bitsPerPixel==24)
        format=nya_render::texture::color_rgb;
    else if (bitsPerPixel== 32 )
        format=nya_render::texture::color_rgba;
    else
        return false;
    
    unsigned int width  = (unsigned int)[image pixelsWide];
    unsigned int height = (unsigned int)[image pixelsHigh];
    unsigned char * imageData = [image bitmapData];
    
    res.build_texture(imageData,width,height,format);
    
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
