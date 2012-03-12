//https://code.google.com/p/nya-engine/

/*
    ToDo: unsigned short int indices
		  bind indices with offset
		  is_valid function
		  log
*/

#include "vbo.h"
#include "platform_specific_gl.h"
#include "render.h"

namespace render
{

#ifdef NO_EXTENSIONS_INIT
	#define vbo_glGenBuffers	glGenBuffers
	#define vbo_glBindBuffer	glBindBuffer
	#define vbo_glBufferData	glBufferData
	#define vbo_glDeleteBuffers	glDeleteBuffers
	#define	vbo_glClientActiveTexture	glClientActiveTexture
#else
	PFNGLGENBUFFERSARBPROC vbo_glGenBuffers;
	PFNGLBINDBUFFERARBPROC vbo_glBindBuffer;
	PFNGLBUFFERDATAARBPROC vbo_glBufferData;
	PFNGLDELETEBUFFERSARBPROC vbo_glDeleteBuffers;
	PFNGLCLIENTACTIVETEXTUREARBPROC	vbo_glClientActiveTexture;
#endif

bool check_init_vbo()
{
	static bool initialised = false;
	if(initialised)
		return true;

#ifndef NO_EXTENSIONS_INIT
	vbo_glGenBuffers = (PFNGLGENBUFFERSARBPROC) get_extension("glGenBuffers");
	if(!vbo_glGenBuffers)
		return false;

	vbo_glBindBuffer = (PFNGLBINDBUFFERARBPROC) get_extension("glBindBuffer");
	if(!vbo_glBindBuffer)
		return false;

	vbo_glBufferData = (PFNGLBUFFERDATAARBPROC)  get_extension("glBufferData");
	if(!vbo_glBufferData)
        return false;

	vbo_glDeleteBuffers = (PFNGLDELETEBUFFERSARBPROC)  get_extension("glDeleteBuffers");
	if(!vbo_glDeleteBuffers)
		return false;

	vbo_glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREARBPROC)  get_extension("glClientActiveTexture");
	if(!vbo_glClientActiveTexture)
		return false;
#endif

	initialised=true;
	return true;
}

inline void vbo_active_tc(unsigned int tc)
{
	static unsigned int state_last_client_tc=0;
    if(tc!=state_last_client_tc)
    {
        vbo_glClientActiveTexture(GL_TEXTURE0_ARB+tc);
        state_last_client_tc=tc;
    }
}

void vbo::bind_tc(unsigned int tc_idx)
{
	if(tc_idx>=VBO_MAX_TEX_COORD)
		return;

	tex_coord &tc = m_tcs[tc_idx];
	tc.bind=true;

	vbo_active_tc(tc_idx);
	glTexCoordPointer(tc.dimension,GL_FLOAT,m_vertex_stride,(void *)(tc.offset));
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void vbo::unbind_tcs()
{
	for(int i=m_num_tc-1;i>=0;i--)
	{
		tex_coord &tc = m_tcs[i];
	    if(!tc.bind)
	    	continue;

        vbo_active_tc(i);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        tc.bind=false;
	}
	vbo_active_tc(0);
}

void vbo::bind()
{
	if(!m_vertex_id)
		return;

	glEnableClientState(GL_VERTEX_ARRAY);
	vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,m_vertex_id);

	const int element_dimensions=(m_element_type==quads)?4:3;

	/*
	static int state_vertex_pointer_i=-1;
	static int state_vertex_pointer_stride=-1;

	if(!(i==state_vertex_pointer_i&&m_vertex_stride==state_vertex_pointer_stride))
    {
		glVertexPointer(element_dimensions,GL_FLOAT,m_vertex_stride,(void *)0);
		state_vertex_pointer_i=element_dimensions;
		state_vertex_pointer_stride=m_vertex_stride;
    }
	*/

	glVertexPointer(element_dimensions,GL_FLOAT,m_vertex_stride,(void *)0);
}

void vbo::bind_indices()
{
	if(!m_index_id)
		return;

	vbo_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,m_index_id);
}

void vbo::unbind()
{
	if(!check_init_vbo())
		return;

	unbind_tcs();
	glDisableClientState(GL_VERTEX_ARRAY);
	vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,0);
	vbo_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
}
    
void vbo::draw()
{
    draw(m_element_count);
}
    
void vbo::draw(unsigned int n_faces)
{
	if(n_faces>m_element_count)
		n_faces = m_element_count;

	if(!n_faces)
		return;

	switch(m_element_type)
	{
		case triangles:
			glDrawElements(GL_TRIANGLES,n_faces*3,GL_UNSIGNED_INT,0);
		break;

		case triangles_strip:
			glDrawElements(GL_TRIANGLE_STRIP,n_faces*3,GL_UNSIGNED_INT,0);
		break;

		case triangles_fan:
			glDrawElements(GL_TRIANGLE_FAN,n_faces*3,GL_UNSIGNED_INT,0);
		break;

		case quads:
			glDrawElements(GL_QUADS,n_faces*4,GL_UNSIGNED_INT,0);
		break;
	}
}

void vbo::release()
{
	//unbind();

	if(m_vertex_id)
	    vbo_glDeleteBuffers(1,&m_vertex_id);

	if(m_index_id)
	    vbo_glDeleteBuffers(1,&m_index_id);

	m_vertex_id = m_index_id = 0;
}

void vbo::gen_vertex_data(const void*data,unsigned int vert_stride,unsigned int vert_count,bool dynamic)
{
	if(!check_init_vbo())
		return;

	const unsigned int size=vert_count*vert_stride;
	if(size==0)
		return; 

	if(m_vertex_id)
	    vbo_glDeleteBuffers(1,&m_vertex_id);

	vbo_glGenBuffers(1,&m_vertex_id);
	vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,m_vertex_id);

	if(dynamic)
		vbo_glBufferData(GL_ARRAY_BUFFER_ARB,size,data,GL_DYNAMIC_DRAW_ARB);
	else
		vbo_glBufferData(GL_ARRAY_BUFFER_ARB,size,data,GL_STATIC_DRAW_ARB);

	vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,0);

	m_vertex_stride = vert_stride;
}

void vbo::gen_index_data(const void*data,element_type type,unsigned int faces_count,bool dynamic)
{
	if(!check_init_vbo())
		return;

	const unsigned int size=(type==quads?(faces_count*4*sizeof(unsigned int)):(faces_count*3)*sizeof(unsigned int));
    if(size==0)
        return;

	if(m_index_id)
	    vbo_glDeleteBuffers(1,&m_index_id);

    m_element_count = faces_count;
    m_element_type = type;

	vbo_glGenBuffers(1,&m_index_id);
	vbo_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,m_index_id);

	if(dynamic)
		vbo_glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB,size,data,GL_DYNAMIC_DRAW_ARB);
	else
		vbo_glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB,size,data,GL_STATIC_DRAW_ARB);

	vbo_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
}

}

