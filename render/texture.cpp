//https://code.google.com/p/nya-engine/

#include "texture.h"
#include "platform_specific_gl.h"
#include "render.h"

namespace nya_render
{

void texture::build_texture(void *data,unsigned int width,unsigned int height,color_format format)
{
    if(!data||width==0||height==0)
    {
        get_log()<<"Unable to build texture: invalid data/width/height";
        return;
    }

    unsigned int source_format=0;
    unsigned int gl_format=0;

    switch(format)
    {
        case color_rgb: source_format=GL_RGB8; gl_format=GL_RGB; break;
        case color_bgr: source_format=GL_RGB8; gl_format=GL_BGR; break;
        case color_rgba: source_format=GL_RGBA8; gl_format=GL_RGBA; break;
        case color_bgra: source_format=GL_RGBA8; gl_format=GL_BGRA; break;
    };

    if(!source_format || !gl_format)
    {
        get_log()<<"Unable to build texture: unsuppored color format";
        return;
    }


    if(!m_tex_id)
        glGenTextures(1,&m_tex_id);

    glBindTexture(GL_TEXTURE_2D,m_tex_id);

    gluBuild2DMipmaps(GL_TEXTURE_2D,source_format,width,height,gl_format,GL_UNSIGNED_BYTE,data);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
}

void texture::bind()
{
    glBindTexture(GL_TEXTURE_2D,m_tex_id);
}

void texture::unbind()
{
    glBindTexture(GL_TEXTURE_2D,0);
}

void texture::release()
{
    if(!m_tex_id)
        return;

    glDeleteTextures(1,&m_tex_id);
}

}

