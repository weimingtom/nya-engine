//https://code.google.com/p/nya-engine/

#ifndef vbo_h
#define vbo_h

#define VBO_MAX_TEX_COORD 16

namespace render
{

class vbo
{
public:
	enum element_type
	{
		triangles,
		triangles_strip,
		triangles_fan,
		quads
	};

	enum element_size
	{
		index2b=2,
		index4b=4
	};

	void gen_vertex_data(const void*data,unsigned int vert_stride,unsigned int vert_count,bool dynamic=false);
	void gen_index_data(const void*data,element_type type,element_size size,unsigned int faces_count,bool dynamic=false);
	void set_normals(unsigned int offset);
	void set_tc(unsigned int tc_idx,unsigned int offset,unsigned int dimension=2);
	void set_colors(unsigned int offset,unsigned int dimension=3);

public:
	void bind(bool indices_bind=true);
	void unbind();

public:    
	void draw();
	void draw(unsigned int faces_count);
	void draw(unsigned int offset,unsigned int faces_count);

public:
	void bind_verts();
	void bind_normals();
	void bind_colors();
	void bind_tc(unsigned int tc);
	void bind_indices();

public:
	void release();

public:
	vbo(): m_element_count(0), m_vertex_id(0), m_index_id(0), m_verts_count(0), m_vertex_bind(false), m_index_bind(false) {}

private:
	element_type m_element_type;
	element_size m_element_size;
	unsigned int m_element_count;

	unsigned int m_vertex_id;
	unsigned int m_index_id;
	unsigned int m_verts_count;

	unsigned int m_vertex_stride;

	bool m_vertex_bind;
	bool m_index_bind;

	struct attribute
	{
		bool has;
		bool bind;
		short dimension;
		unsigned int offset;

		attribute(): has(false), bind(false) {}
	};

	attribute m_colors;
	attribute m_normals;
	attribute m_tcs[VBO_MAX_TEX_COORD];
};

}

#endif

