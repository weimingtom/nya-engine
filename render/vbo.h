//https://code.google.com/p/nya-engine/

#pragma once

namespace nya_render
{

class vbo
{
public:
    enum element_type
    {
        triangles,
        triangle_strip,
        triangle_fan,
        points,
        lines,
        line_loop
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

    void set_vertex_data(const void*data,unsigned int vert_stride,unsigned int vert_count,usage_hint usage=static_draw);
    void set_index_data(const void*data,element_size size,unsigned int elements_count,usage_hint usage=static_draw);
    void set_element_type(element_type type) { m_element_type = type; }
    void set_normals(unsigned int offset);
    void set_tc(unsigned int tc_idx,unsigned int offset,unsigned int dimension=2);
    void set_colors(unsigned int offset,unsigned int dimension=3);

public:
    void bind(bool indices_bind=true) const;
    void unbind() const;

public:
    void draw() const;
    void draw(unsigned int count) const; // verts or faces (if has indices) count
    void draw(unsigned int offset,unsigned int count) const;

public:
    void bind_verts() const;
    void bind_normals() const;
    void bind_colors() const;
    void bind_tc(unsigned int tc) const;
    void bind_indices() const;

public:
    void release();

public:
    vbo(): m_element_type(triangles), m_element_count(0), m_vertex_id(0), m_index_id(0),
           m_verts_count(0), m_vertex_bind(false), m_index_bind(false) {}

private:
    element_type m_element_type;
    element_size m_element_size;
    unsigned int m_element_count;
    usage_hint m_elements_usage;

    unsigned int m_vertex_id;
    unsigned int m_index_id;
    unsigned int m_verts_count;

    unsigned int m_vertex_stride;
    usage_hint m_vertex_usage;

    mutable bool m_vertex_bind;
    mutable bool m_index_bind;

    struct attribute
    {
        bool has;
        mutable bool bind;
        short dimension;
        unsigned int offset;

        attribute(): has(false), bind(false) {}
    };

    attribute m_colors;
    attribute m_normals;
    const static unsigned int vbo_max_tex_coord=16;
    attribute m_tcs[vbo_max_tex_coord];
};

}
