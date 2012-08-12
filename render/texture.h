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
        color_bgr,
        color_rgba,
        color_bgra
    };

    void build_texture(const void *data,unsigned int width,unsigned int height,color_format format);

public:
    void bind();
    void unbind();

public:
    void release();

    unsigned int get_width() const { return m_width; }
    unsigned int get_height() const { return m_height; }

public:
    texture(): m_tex_id(0), m_width(0), m_height(0) {}

private:
    unsigned int m_tex_id;
    unsigned int m_width;
    unsigned int m_height;
};

}
