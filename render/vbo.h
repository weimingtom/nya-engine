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

	void gen_vertex_data(const void*data,unsigned int vert_stride,unsigned int vert_count,bool dynamic=false);
	void gen_index_data(const void*data,element_type type,unsigned int faces_count,bool dynamic=false);
	void set_tc(unsigned int dimensions,unsigned int offset);

public:
	void bind();
	void bind_tc(unsigned int tc);
	void bind_indices();
    void draw();
	void draw(unsigned int faces_count);
	void unbind_tcs();
	void unbind();

public:
	void release();

public:
	vbo(): m_element_count(0), m_num_tc(0), m_vertex_id(0), m_index_id(0), m_num_faces(0) {}

private:
	element_type m_element_type;
	unsigned int m_element_count;
	int m_num_tc;

	unsigned int m_vertex_id;
	unsigned int m_index_id;
	unsigned int m_num_faces;

	unsigned int m_vertex_stride;

	struct tex_coord
	{
		short dimension;
		unsigned int offset;
		bool bind;

		tex_coord(): dimension(0),offset(0),bind(false) {}
	};

	tex_coord m_tcs[VBO_MAX_TEX_COORD];
};

}

#endif

