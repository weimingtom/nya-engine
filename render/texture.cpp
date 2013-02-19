//https://code.google.com/p/nya-engine/

#include "texture.h"
#include "platform_specific_gl.h"
#include "render.h"

namespace nya_render
{

void texture::build_texture(const void *data,unsigned int width,unsigned int height,color_format format)
{
    if(width==0 || height==0)
    {
        get_log()<<"Unable to build texture: invalid width or height\n";
	    release();
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
	    release();
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
        case color_r: source_format=GL_LUMINANCE; gl_format=GL_LUMINANCE; break;
        case depth16: source_format=GL_DEPTH_COMPONENT16; gl_format=GL_DEPTH_COMPONENT; break;
#ifdef OPENGL_ES
        case depth24: source_format=GL_DEPTH_COMPONENT24_OES; gl_format=GL_DEPTH_COMPONENT; break;
        case depth32: source_format=GL_DEPTH_COMPONENT32_OES; gl_format=GL_DEPTH_COMPONENT; break;
#else
        case depth24: source_format=GL_DEPTH_COMPONENT24; gl_format=GL_DEPTH_COMPONENT; break;
        case depth32: source_format=GL_DEPTH_COMPONENT32; gl_format=GL_DEPTH_COMPONENT; break;
#endif
    };

    if(!source_format || !gl_format)
    {
        get_log()<<"Unable to build texture: unsuppored color format\n";
	    release();
        return;
    }

	//bool create_new=(!m_tex_id || m_width!=width || m_height!=height || m_type!=texture_2d || m_format!=format);

    if(!m_tex_id || m_gl_type!=GL_TEXTURE_2D)
        glGenTextures(1,&m_tex_id);

    m_width=width;
    m_height=height;
	m_gl_type=GL_TEXTURE_2D;
	m_format=format;

    glBindTexture(GL_TEXTURE_2D,m_tex_id);

    /*
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    */

#ifdef GL_GENERATE_MIPMAP
    if(data)
        glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
#endif

	//if(create_new)
	    glTexImage2D(GL_TEXTURE_2D,0,source_format,width,height,0,gl_format,GL_UNSIGNED_BYTE,data);
	//else
	//	glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width,height,gl_format,GL_UNSIGNED_BYTE,data);

#ifndef GL_GENERATE_MIPMAP
    if(data)
        glGenerateMipmap(GL_TEXTURE_2D);
#endif

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    if(data)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    else
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
}

void texture::build_cubemap(const void *data[6],unsigned int width,unsigned int height,color_format format)
{
    if(!data || width==0 || height==0)
    {
        get_log()<<"Unable to build texture: invalid data/width/height\n";
	    release();
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
	    release();
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
        case color_r: source_format=GL_LUMINANCE; gl_format=GL_LUMINANCE; break;
    };

    if(!source_format || !gl_format)
    {
        get_log()<<"Unable to build texture: unsuppored color format\n";
	    release();
        return;
    }

	if(m_format!=format)
		release();

	//bool create_new=(!m_tex_id || m_width!=width || m_height!=height || m_type!=texture_2d || m_format!=format);

    m_width=width;
    m_height=height;
	m_gl_type=GL_TEXTURE_CUBE_MAP;
	m_format=format;

    if(!m_tex_id)
        glGenTextures(1,&m_tex_id);

    glBindTexture(GL_TEXTURE_CUBE_MAP,m_tex_id);

#ifdef GL_GENERATE_MIPMAP
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_GENERATE_MIPMAP,GL_TRUE);
#endif

	const unsigned int cube_faces[]={GL_TEXTURE_CUBE_MAP_POSITIVE_X,GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
									 GL_TEXTURE_CUBE_MAP_POSITIVE_Y,GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
									 GL_TEXTURE_CUBE_MAP_POSITIVE_Z,GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};

	for(int i=0;i<sizeof(cube_faces)/sizeof(cube_faces[0]);++i)
		glTexImage2D(cube_faces[i],0,source_format,width,height,0,gl_format,GL_UNSIGNED_BYTE,data[i]);

#ifndef GL_GENERATE_MIPMAP
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
#endif

    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
}

void texture::bind() const
{
	if(!m_gl_type)
	{
		unbind_all();
		return;
	}

    glBindTexture(m_gl_type,m_tex_id);
}

void texture::unbind() const
{
	if(!m_gl_type)
	{
		unbind_all();
		return;
	}

	glBindTexture(m_gl_type,0);
}

void texture::unbind_all()
{
    glBindTexture(GL_TEXTURE_2D,0);
    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
}

void texture::select_multitex_slot(unsigned int idx)
{
#if defined(OPENGL_ES)
    glActiveTexture(GL_TEXTURE0+idx);
#elif defined(NO_EXTENSIONS_INIT)
    glActiveTexture(GL_TEXTURE0+idx);
#else
    static PFNGLACTIVETEXTUREARBPROC tex_glActiveTexture=0;

    static bool initialised=false;
    if(tex_glActiveTexture!=0)
    {
        tex_glActiveTexture(GL_TEXTURE0_ARB+idx);
        return;
    }

    if(initialised)
        return;

    tex_glActiveTexture=(PFNGLACTIVETEXTUREARBPROC)get_extension("glActiveTexture");
    initialised=true;

    select_multitex_slot(idx);
#endif
}

void texture::release()
{
    if(m_tex_id)
	    glDeleteTextures(1,&m_tex_id);

    m_tex_id=0;
    m_width=0;
    m_height=0;
}

}

