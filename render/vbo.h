//https://code.google.com/p/nya-engine/

#pragma once

#include "platform_specific_gl.h"

#ifdef DIRECTX11
    #include <string>
#endif

namespace nya_render
{

class vbo
{
public:
    enum element_type
    {
        triangles,
        triangle_strip,
        points,
        lines,
        line_strip
    };

    enum element_size
    {
        index2b=2,
        index4b=4
    };

    enum usage_hint
    {
        static_draw,
        dynamic_draw,
        stream_draw
    };

    bool set_vertex_data(const void*data,unsigned int vert_stride,unsigned int vert_count,usage_hint usage=static_draw);
    bool set_index_data(const void*data,element_size size,unsigned int elements_count,usage_hint usage=static_draw);
    void set_element_type(element_type type);
    void set_vertices(unsigned int offset,unsigned int dimension);
    void set_normals(unsigned int offset);
    void set_tc(unsigned int tc_idx,unsigned int offset,unsigned int dimension);
    void set_colors(unsigned int offset,unsigned int dimension);

public:
    void bind() const { bind_verts(); bind_indices(); }

    void bind_verts() const;
    void bind_indices() const;

    static void unbind();

public:
    static void draw();
    static void draw(unsigned int count);
    static void draw(unsigned int offset,unsigned int count);

public:
    void release();

public:
    vbo(): m_verts(-1),m_indices(-1) {}

private:
    int m_verts;
    int m_indices;
};

}
