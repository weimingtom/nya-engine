//https://code.google.com/p/nya-engine/

#include "texture.h"
#include "platform_specific_gl.h"
#include "render.h"

namespace nya_render
{

void texture::build_texture(const void *data,unsigned int width,unsigned int height,color_format format)
{
    release();

    if(!data || width==0 || height==0)
    {
        get_log()<<"Unable to build texture: invalid data/width/height\n";
        return;
    }

    if(!m_max_tex_size)
    {
        GLint texSize;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
        m_max_tex_size=texSize;
    }

    if(width>m_max_tex_size || height>m_max_tex_size)
    {
        get_log()<<"Unable to build texture: width or height is too high, maximum is "<<m_max_tex_size<<"\n";
        return;
    }

    unsigned int source_format=0;
    unsigned int gl_format=0;

    switch(format)
    {
        case color_rgb: source_format=GL_RGB; gl_format=GL_RGB; break;
        //case color_bgr: source_format=GL_RGB; gl_format=GL_BGR; break;
        case color_rgba: source_format=GL_RGBA; gl_format=GL_RGBA; break;
        case color_bgra: source_format=GL_RGBA; gl_format=GL_BGRA; break;
    };

    if(!source_format || !gl_format)
    {
        get_log()<<"Unable to build texture: unsuppored color format\n";
        return;
    }

    m_width=width;
    m_height=height;

    if(!m_tex_id)
        glGenTextures(1,&m_tex_id);

    glBindTexture(GL_TEXTURE_2D,m_tex_id);

    /*
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    */

#ifdef GL_GENERATE_MIPMAP
    glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
#endif

    glTexImage2D(GL_TEXTURE_2D,0,source_format,width,height,0,gl_format,GL_UNSIGNED_BYTE,data);

#ifndef GL_GENERATE_MIPMAP
    glGenerateMipmap(GL_TEXTURE_2D);
#endif

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
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

    m_tex_id=0;
    m_width=0;
    m_height=0;
}

}

