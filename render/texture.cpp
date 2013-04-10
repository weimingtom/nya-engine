//https://code.google.com/p/nya-engine/

#include "texture.h"
#include "platform_specific_gl.h"
#include "render.h"

namespace nya_render
{

bool texture::build_texture(const void *data,unsigned int width,unsigned int height,color_format format)
{
    if(width==0 || height==0)
    {
        get_log()<<"Unable to build texture: invalid width or height\n";
	    release();
        return false;
    }

#ifdef DIRECTX11
#else
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
        return false;
    }

    unsigned int source_format=0;
    unsigned int gl_format=0;
    unsigned int precision=GL_UNSIGNED_BYTE;

    switch(format)
    {
        case color_rgb: source_format=GL_RGB; gl_format=GL_RGB; break;
        //case color_bgr: source_format=GL_RGB; gl_format=GL_BGR; break;
        case color_rgba: source_format=GL_RGBA; gl_format=GL_RGBA; break;
        case color_bgra: source_format=GL_RGBA; gl_format=GL_BGRA; break;
        case color_r: source_format=GL_LUMINANCE; gl_format=GL_LUMINANCE; break;
#ifdef OPENGL_ES
        case depth16: source_format=GL_DEPTH_COMPONENT; gl_format=GL_DEPTH_COMPONENT; precision=GL_UNSIGNED_SHORT; break;
        case depth24: source_format=GL_DEPTH_COMPONENT; gl_format=GL_DEPTH_COMPONENT; precision=GL_UNSIGNED_INT; break;
        case depth32: source_format=GL_DEPTH_COMPONENT; gl_format=GL_DEPTH_COMPONENT; precision=GL_UNSIGNED_INT; break;
#else
        case depth16: source_format=GL_DEPTH_COMPONENT16; gl_format=GL_DEPTH_COMPONENT; break;
        case depth24: source_format=GL_DEPTH_COMPONENT24; gl_format=GL_DEPTH_COMPONENT; break;
        case depth32: source_format=GL_DEPTH_COMPONENT32; gl_format=GL_DEPTH_COMPONENT; break;
#endif
    };

    if(!source_format || !gl_format)
    {
        get_log()<<"Unable to build texture: unsuppored color format\n";
	    release();
        return false;
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

    const bool pot=((width&(width-1))==0 && (height&(height-1))==0);
    const bool has_mipmap=(data && pot);

#ifdef GL_GENERATE_MIPMAP
    if(has_mipmap)
        glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
#endif
	//if(create_new)
	    glTexImage2D(GL_TEXTURE_2D,0,source_format,width,height,0,gl_format,precision,data);
	//else
	//	glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width,height,gl_format,precision,data);
#ifndef GL_GENERATE_MIPMAP
    if(has_mipmap)
        glGenerateMipmap(GL_TEXTURE_2D);
#endif

    if(pot)
    {
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    }

    //if(format<depth16)
    {
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

        if(has_mipmap)
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        else
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    }/*
    else
    {
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    }*/
#endif

    return true;
}

bool texture::build_cubemap(const void *data[6],unsigned int width,unsigned int height,color_format format)
{
    if(!data || width==0 || height==0)
    {
        get_log()<<"Unable to build cube texture: invalid data/width/height\n";
	    release();
        return false;
    }

#ifdef DIRECTX11
#else
    if(!m_max_tex_size)
    {
        GLint texSize;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
        m_max_tex_size=texSize;
    }

    if(width>m_max_tex_size || height>m_max_tex_size)
    {
        get_log()<<"Unable to build cube texture: width or height is too high, maximum is "<<m_max_tex_size<<"\n";
	    release();
        return false;
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
        default: break;
    };

    if(!source_format || !gl_format)
    {
        get_log()<<"Unable to build cube texture: unsuppored color format\n";
	    release();
        return false;
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
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
#endif

    return true;
}

void texture::bind() const
{
	if(!m_gl_type)
	{
		unbind_all();
		return;
	}

#ifdef DIRECTX11
#else
    glBindTexture(m_gl_type,m_tex_id);
#endif
}

void texture::unbind() const
{
	if(!m_gl_type)
	{
		unbind_all();
		return;
	}

#ifdef DIRECTX11
#else
	glBindTexture(m_gl_type,0);
#endif
}

void texture::unbind_all()
{
#ifdef DIRECTX11
#else
    glBindTexture(GL_TEXTURE_2D,0);
    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
#endif
}

void texture::select_multitex_slot(unsigned int idx)
{
#ifdef DIRECTX11
#else

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

#endif
}

void texture::release()
{
#ifdef DIRECTX11
#else
    if(m_tex_id)
	    glDeleteTextures(1,&m_tex_id);

    m_tex_id=0;
    m_width=0;
    m_height=0;
#endif
}

}

