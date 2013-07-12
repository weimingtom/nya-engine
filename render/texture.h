//https://code.google.com/p/nya-engine/

#pragma once

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
        greyscale,

        depth16,
        depth24,
        depth32
    };

    bool build_texture(const void *data,unsigned int width,unsigned int height,color_format format);

	//order: positive_x,negative_x,positive_y,negative_y,positive_z,negative_z
	bool build_cubemap(const void *data[6],unsigned int width,unsigned int height,color_format format);

public:
    void bind() const;
    static void unbind();

    static void select_multitex_slot(unsigned int idx);

    static void apply();

public:
    void release();

    unsigned int get_width() const { return m_width; }
    unsigned int get_height() const { return m_height; }

public:
    texture(): m_tex(-1),m_width(0),m_height(0),m_max_tex_size(0) {}

private:
    int m_tex;

    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_max_tex_size;

	color_format m_format;
};

}
