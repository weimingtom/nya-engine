//https://code.google.com/p/nya-engine/

#include "texture.h"
#include "platform_specific_gl.h"
#include "render.h"

namespace render
{

void texture::build_texture(void *data,unsigned int width,unsigned int height,color_format format)
{
	if(!data||width==0||height==0)
		return;

	unsigned int gl_format=0;

	switch(format)
	{
		case color_rgb: gl_format=GL_RGB; break;
		case color_rgba: gl_format=GL_RGBA; break;
	};

	if(!gl_format)
		return;

	if(!m_tex_id)
	    glGenTextures(1,&m_tex_id);

    glBindTexture(GL_TEXTURE_2D, m_tex_id);

    gluBuild2DMipmaps(GL_TEXTURE_2D,gl_format,width,height,gl_format,GL_UNSIGNED_BYTE,data);

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

