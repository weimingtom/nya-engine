//https://code.google.com/p/nya-engine/

#pragma once

namespace nya_memory { class tmp_buffer_ref; }

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

    enum index_size
    {
        index2b=2,
        index4b=4
    };

    enum vertex_atrib_type
    {
        float16,
        float32,
        uint8
    };

    enum usage_hint
    {
        static_draw,
        dynamic_draw,
        stream_draw
    };

    const static unsigned int max_tex_coord=13;

public:
    bool set_vertex_data(const void*data,unsigned int vert_stride,unsigned int vert_count,usage_hint usage=static_draw);
    bool set_index_data(const void*data,index_size size,unsigned int indices_count,usage_hint usage=static_draw);
    void set_element_type(element_type type);
    void set_vertices(unsigned int offset,unsigned int dimension,vertex_atrib_type=float32);
    void set_normals(unsigned int offset,vertex_atrib_type=float32);
    void set_tc(unsigned int tc_idx,unsigned int offset,unsigned int dimension,vertex_atrib_type=float32);
    void set_colors(unsigned int offset,unsigned int dimension,vertex_atrib_type=float32);

public:
    bool get_vertex_data( nya_memory::tmp_buffer_ref &data ) const;
    bool get_index_data( nya_memory::tmp_buffer_ref &data ) const;
    element_type get_element_type() const;
    unsigned int get_verts_count() const;
    unsigned int get_vert_stride() const;
    unsigned int get_vert_offset() const;
    unsigned int get_vert_dimension() const;
    unsigned int get_normals_offset() const;
    unsigned int get_tc_offset(unsigned int idx) const;
    unsigned int get_tc_dimension(unsigned int idx) const;
    unsigned int get_colors_offset() const;
    unsigned int get_colors_dimension() const;
    unsigned int get_indices_count() const;
    index_size get_index_size() const;

public:
    void bind() const { bind_verts(); bind_indices(); }

    void bind_verts() const;
    void bind_indices() const;

    static void unbind();

public:
    static void draw();
    static void draw(unsigned int count);
    static void draw(unsigned int offset,unsigned int count);
    static void draw(unsigned int offset,unsigned int count,element_type type,unsigned int instances=1);

public:
    void release();

public:
    static unsigned int get_used_vmem_size();

public:
    vbo(): m_verts(-1),m_indices(-1) {}

private:
    int m_verts;
    int m_indices;
};

}
