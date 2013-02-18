//https://code.google.com/p/nya-engine/

#pragma once

namespace nya_render
{

class texture
{
public:
    enum color_format
    {
        color_rgb,
        //color_bgr,
        color_rgba,
        color_bgra,
        color_r
    };

    void build_texture(const void *data,unsigned int width,unsigned int height,color_format format);

	//order: positive_x,negative_x,positive_y,negative_y,positive_z,negative_z
	void build_cubemap(const void *data[6],unsigned int width,unsigned int height,color_format format);

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

public:
    texture(): m_tex_id(0),m_width(0),m_height(0),m_max_tex_size(0),m_gl_type(0) {}

private:
    unsigned int m_tex_id;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_max_tex_size;

	unsigned int m_gl_type;
	color_format m_format;
};

}
