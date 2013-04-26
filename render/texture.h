//https://code.google.com/p/nya-engine/

#pragma once

#include "platform_specific_gl.h"

namespace nya_render
{

class texture
{
    friend class fbo;

public:
    enum color_format
    {
        color_rgb,
        //color_bgr,
        color_rgba,
        color_bgra,
        color_r,

        depth16,
        depth24,
        depth32
    };

    bool build_texture(const void *data,unsigned int width,unsigned int height,color_format format);

	//order: positive_x,negative_x,positive_y,negative_y,positive_z,negative_z
	bool build_cubemap(const void *data[6],unsigned int width,unsigned int height,color_format format);

public:
    void bind() const;
    void unbind() const;
    static void unbind_all();

public:
    static void select_multitex_slot(unsigned int idx);

public:
    void release();

    unsigned int get_width() const { return m_width; }
    unsigned int get_height() const { return m_height; }

#ifdef DIRECTX11
public:
    texture(): m_tex(0),m_sampler_state(0),m_width(0),m_height(0),m_max_tex_size(0) {}

private:
    ID3D11ShaderResourceView* m_tex;
    ID3D11SamplerState* m_sampler_state;
#else
public:
    texture(): m_tex_id(0),m_width(0),m_height(0),m_max_tex_size(0),m_gl_type(0) {}

private:
    unsigned int m_tex_id;
	unsigned int m_gl_type;
#endif
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_max_tex_size;

	color_format m_format;
};

}
