//https://code.google.com/p/nya-engine/

/*
    ToDo:
          is_valid function
          log
          gl functions in anonymous namespace
          advanced is_supported function (public)
          cache states, static unbind
*/

#include "vbo.h"
#include "render.h"
#include "texture.h"
#include "shader.h"

#ifdef DIRECTX11
    #include "shader.h"
#endif

namespace nya_render
{
#ifndef DIRECTX11
  #ifdef NO_EXTENSIONS_INIT
    #define vbo_glGenBuffers glGenBuffers
    #define vbo_glBindBuffer glBindBuffer
    #define vbo_glBufferData glBufferData
    #define vbo_glBufferSubData glBufferSubData
    #define vbo_glDeleteBuffers glDeleteBuffers
    #define vbo_glClientActiveTexture glClientActiveTexture

    #ifndef GL_ARRAY_BUFFER_ARB
        #define GL_ARRAY_BUFFER_ARB GL_ARRAY_BUFFER
    #endif

    #ifndef GL_DYNAMIC_DRAW_ARB
        #define GL_DYNAMIC_DRAW_ARB GL_DYNAMIC_DRAW
    #endif

    #ifndef GL_STATIC_DRAW_ARB
        #define GL_STATIC_DRAW_ARB GL_STATIC_DRAW
    #endif

    #ifndef GL_STREAM_DRAW_ARB
        #define GL_STREAM_DRAW_ARB GL_STREAM_DRAW
    #endif

    #ifndef GL_ELEMENT_ARRAY_BUFFER_ARB
        #define GL_ELEMENT_ARRAY_BUFFER_ARB GL_ELEMENT_ARRAY_BUFFER
    #endif
  #else
    PFNGLGENBUFFERSARBPROC vbo_glGenBuffers;
    PFNGLBINDBUFFERARBPROC vbo_glBindBuffer;
    PFNGLBUFFERDATAARBPROC vbo_glBufferData;
    PFNGLBUFFERSUBDATAARBPROC vbo_glBufferSubData;
    PFNGLDELETEBUFFERSARBPROC vbo_glDeleteBuffers;
    PFNGLCLIENTACTIVETEXTUREARBPROC vbo_glClientActiveTexture;
  #endif

bool check_init_vbo()
{
    static bool initialised=false;
    static bool failed=true;
    if(initialised)
        return !failed;

    //if(!has_extension("GL_ARB_vertex_buffer_object"))
    //    return false;

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

    vbo_glBufferSubData = (PFNGLBUFFERSUBDATAARBPROC)  get_extension("glBufferSubData");
    if(!vbo_glBufferSubData)
        return false;

    vbo_glDeleteBuffers = (PFNGLDELETEBUFFERSARBPROC)  get_extension("glDeleteBuffers");
    if(!vbo_glDeleteBuffers)
        return false;

    vbo_glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREARBPROC)  get_extension("glClientActiveTexture");
    if(!vbo_glClientActiveTexture)
        return false;
#endif

    initialised=true;
    failed=false;

    return true;
}
#endif

void vbo::bind(bool indices_bind) const
{
    bind_verts();
    bind_normals();
    bind_colors();

    for(unsigned int i=0;i<vbo_max_tex_coord;++i)
    {
        if(m_tcs[i].has)
            bind_tc(i);
    }
    if(indices_bind)
        bind_indices();
}

void vbo::bind_verts() const
{
    if(!m_vertices.has)
        return;

	if(!m_vertex_loc)
        return;

#ifdef DIRECTX11
	UINT offset = 0;
	get_context()->IASetVertexBuffers(0,1,&m_vertex_loc,&m_vertex_stride,&offset);
#else
    if(!m_vertex_bind)
    {
        vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,m_vertex_loc);
        m_vertex_bind=true;
    }

  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
    glEnableVertexAttribArray(vertex_attribute);
    glVertexAttribPointer(vertex_attribute,m_vertices.dimension,GL_FLOAT,0,m_vertex_stride,(void *)(m_vertices.offset));
  #else
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(m_vertices.dimension,GL_FLOAT,m_vertex_stride,(void *)0);
  #endif
#endif
    m_vertex_bind=true;
}

void vbo::bind_normals() const
{
    if(!m_normals.has)
        return;

#ifdef DIRECTX11
#else
    if(!m_vertex_bind)
    {
        if(!m_vertex_loc)
            return;

        vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,m_vertex_loc);
        m_vertex_bind=true;
    }

  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
    glEnableVertexAttribArray(normal_attribute);
    glVertexAttribPointer(normal_attribute,3,GL_FLOAT,1,m_vertex_stride,(void *)(m_normals.offset));
  #else
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT,m_vertex_stride,(void *)(m_normals.offset));
  #endif
#endif

    m_normals.bind=true;
}

void vbo::bind_colors() const
{
    if(!m_colors.has)
        return;

#ifdef DIRECTX11
#else
    if(!m_vertex_bind)
    {
        if(!m_vertex_loc)
            return;

        vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,m_vertex_loc);
        m_vertex_bind=true;
    }

  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
    glEnableVertexAttribArray(color_attribute);
    glVertexAttribPointer(color_attribute,m_colors.dimension,GL_FLOAT,0,m_vertex_stride,(void *)(m_colors.offset));
  #else
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(m_colors.dimension,GL_FLOAT,m_vertex_stride,(void *)(m_colors.offset));
  #endif
#endif
    m_colors.bind=true;
}

void vbo::bind_tc(unsigned int tc_idx) const
{
    if(tc_idx>=vbo_max_tex_coord)
        return;

    const attribute &tc=m_tcs[tc_idx];

    if(!tc.has)
        return;

#ifdef DIRECTX11
#else
    if(!m_vertex_bind)
    {
        if(!m_vertex_loc)
            return;

        vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,m_vertex_loc);
        m_vertex_bind=true;
    }

  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
    glEnableVertexAttribArray(tc0_attribute+tc_idx);
    glVertexAttribPointer(tc0_attribute+tc_idx,tc.dimension,GL_FLOAT,0,m_vertex_stride,(void *)(tc.offset));
  #else
    vbo_glClientActiveTexture(GL_TEXTURE0_ARB+tc_idx);
    glTexCoordPointer(tc.dimension,GL_FLOAT,m_vertex_stride,(void *)(tc.offset));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  #endif
#endif
    tc.bind=true;
}

void vbo::bind_indices() const
{
    if(!m_index_loc)
        return;

    m_index_bind=true;

#ifdef DIRECTX11
	switch(m_element_size)
	{
		case index2b: get_context()->IASetIndexBuffer(m_index_loc,DXGI_FORMAT_R16_UINT,0); break;
		case index4b: get_context()->IASetIndexBuffer(m_index_loc,DXGI_FORMAT_R32_UINT,0); break;
	}
#else
    vbo_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,m_index_loc);
#endif
}

void vbo::unbind() const
{
#ifdef DIRECTX11
#else
    if(m_vertex_bind)
    {

#ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
        glDisableVertexAttribArray(vertex_attribute);
#else
        glDisableClientState(GL_VERTEX_ARRAY);
        vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,0);
#endif
        m_vertex_bind=false;
    }

    if(m_index_bind)
    {
        vbo_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
        m_index_bind=false;
    }

    if(m_colors.bind)
    {

#ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
        glDisableVertexAttribArray(color_attribute);
#else
        glDisableClientState(GL_COLOR_ARRAY);
#endif
        m_colors.bind=false;
    }

    if(m_normals.bind)
    {

#ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
        glDisableVertexAttribArray(normal_attribute);
#else
        glDisableClientState(GL_NORMAL_ARRAY);
#endif
        m_normals.bind=false;
    }

#ifndef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
    bool has_unbinds=false;
#endif

    for(unsigned int i=0;i<vbo_max_tex_coord;++i)
    {
        const attribute &tc=m_tcs[i];
        if(!tc.bind)
            continue;

#ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
        glDisableVertexAttribArray(tc0_attribute+i);
#else
        vbo_glClientActiveTexture(GL_TEXTURE0_ARB+i);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        has_unbinds=true;
#endif
        tc.bind=false;
    }

#ifndef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
    if(has_unbinds)
        vbo_glClientActiveTexture(GL_TEXTURE0_ARB);
#endif
#endif
}

void vbo::draw() const
{
    if(m_index_bind)
        draw(m_element_count);
    else
        draw(m_verts_count);
}

void vbo::draw(unsigned int count) const
{
    draw(0,count);
}

#ifdef DIRECTX11
DXGI_FORMAT get_dx_format(int dimension)
{
    switch(dimension)
    {
        case 1: return DXGI_FORMAT_R32_FLOAT;
        case 2: return DXGI_FORMAT_R32G32_FLOAT;
        case 3: return DXGI_FORMAT_R32G32B32_FLOAT;
        case 4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
    }

    return DXGI_FORMAT_UNKNOWN;
}
#endif

void vbo::draw(unsigned int offset,unsigned int count) const
{
    if(!m_vertex_bind)
        return;

    shader::apply();
    texture::apply();
    apply_state();

#ifdef DIRECTX11
    if(m_layout.empty())
    {
        char tmp[64];
        if(!m_vertices.has)
            return;

        if(m_colors.has)
        {
            sprintf(tmp,"c%d%d",m_colors.dimension,m_colors.offset);
            m_layout.append(tmp);
        }
        if(m_normals.has)
        {
            sprintf(tmp,"n%d",m_normals.offset);
            m_layout.append(tmp);
        }
        for(int i=0;i<vbo_max_tex_coord;++i)
        {
            const attribute &tc=m_tcs[i];
            if(!tc.has)
                continue;

            sprintf(tmp,"t%d_%d%d",i,tc.dimension,tc.offset);
            m_layout.append(tmp);
        }

        sprintf(tmp,"p%d%d",m_vertices.dimension,m_vertices.offset);
        m_layout.append(tmp);
    }

    ID3D11InputLayout *layout=shader::get_layout(m_layout);
    if(!layout)
    {
        std::vector<D3D11_INPUT_ELEMENT_DESC> desc;

        if(m_vertices.has)
        {
            D3D11_INPUT_ELEMENT_DESC d;
            d.SemanticName="POSITION";
            d.SemanticIndex=0;
            d.Format=get_dx_format(m_vertices.dimension);
            d.InputSlot=0;
            d.AlignedByteOffset=m_vertices.offset;
            d.InputSlotClass=D3D11_INPUT_PER_VERTEX_DATA;
            d.InstanceDataStepRate=0;
            desc.push_back(d);
        }
        
        if(m_normals.has)
        {
            D3D11_INPUT_ELEMENT_DESC d;
            d.SemanticName="NORMAL";
            d.SemanticIndex=0;
            d.Format=get_dx_format(3);
            d.InputSlot=0;
            d.AlignedByteOffset=m_normals.offset;
            d.InputSlotClass=D3D11_INPUT_PER_VERTEX_DATA;
            d.InstanceDataStepRate=0;
            desc.push_back(d);
        }

        if(m_colors.has)
        {
            D3D11_INPUT_ELEMENT_DESC d;
            d.SemanticName="COLOR";
            d.SemanticIndex=0;
            d.Format=get_dx_format(m_colors.dimension);
            d.InputSlot=0;
            d.AlignedByteOffset=m_colors.offset;
            d.InputSlotClass=D3D11_INPUT_PER_VERTEX_DATA;
            d.InstanceDataStepRate=0;
            desc.push_back(d);
        }

        for(int i=0;i<vbo_max_tex_coord;++i)
        {
            const attribute &tc=m_tcs[i];
            if(!tc.has)
                continue;

            D3D11_INPUT_ELEMENT_DESC d;
            d.SemanticName="TEXCOORD";
            d.SemanticIndex=i;
            d.Format=get_dx_format(tc.dimension);
            d.InputSlot=0;
            d.AlignedByteOffset=tc.offset;
            d.InputSlotClass=D3D11_INPUT_PER_VERTEX_DATA;
            d.InstanceDataStepRate=0;
            desc.push_back(d);
        }

        layout=shader::add_layout(m_layout,&desc[0],desc.size());

        if(!layout)
            return;
    }
	get_context()->IASetInputLayout(layout);

    switch(m_element_type)
    {
        case triangles: get_context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); break;
        case triangle_strip: get_context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); break;
        case points: get_context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST); break;
        case lines: get_context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST); break;
		case line_strip: get_context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP); break;
        default: return;
    }

	if(m_index_bind)
	{
        if(offset+count>m_element_count)
            return;

		get_context()->DrawIndexed(count,offset,0);
	}
	else
	{
        if(offset+count>m_verts_count)
            return;

		get_context()->Draw(count,offset);
	}
#else
    unsigned int gl_elem;

    switch(m_element_type)
    {
        case triangles: gl_elem=GL_TRIANGLES; break;
        case triangle_strip: gl_elem=GL_TRIANGLE_STRIP; break;
        case points: gl_elem=GL_POINTS; break;
        case lines: gl_elem=GL_LINES; break;
        case line_strip: gl_elem=GL_LINE_STRIP; break;
        default: return;
    }

    if(m_index_bind)
    {
        if(offset+count>m_element_count)
            return;

        const unsigned int gl_elem_type=(m_element_size==index4b?GL_UNSIGNED_INT:GL_UNSIGNED_SHORT);

        glDrawElements(gl_elem,count,gl_elem_type,(void*)(offset*m_element_size));
    }
    else
    {
        if(offset+count>m_verts_count)
            return;

        glDrawArrays(gl_elem,offset,count);
    }
#endif
}

void vbo::release()
{
    unbind();

#ifdef DIRECTX11
    if(m_vertex_loc)
    {
        m_vertex_loc->Release();
        m_vertex_loc=0;
    }

    if(m_index_loc)
    {
        m_index_loc->Release();
        m_index_loc=0;
    }

    m_layout.clear();
#else
    if(m_vertex_loc)
        vbo_glDeleteBuffers(1,&m_vertex_loc);

    if(m_index_loc)
        vbo_glDeleteBuffers(1,&m_index_loc);

    m_vertex_loc=m_index_loc=0;
#endif
}

#ifndef DIRECTX11
int gl_usage(vbo::usage_hint usage)
{
    switch(usage)
    {
        case vbo::static_draw: return GL_STATIC_DRAW_ARB;
        case vbo::dynamic_draw: return GL_DYNAMIC_DRAW_ARB;
        case vbo::stream_draw: return GL_STREAM_DRAW_ARB;
    }

    return GL_DYNAMIC_DRAW_ARB;
}
#endif

bool vbo::set_vertex_data(const void*data,unsigned int vert_stride,unsigned int vert_count,usage_hint usage)
{
    const unsigned int size=vert_count*vert_stride;
    if(size==0 || !data)
    {
        m_verts_count=0;
        get_log()<<"Unable to set vertices: invalid data\n";
        return false;
    }

#ifdef DIRECTX11
	if(!get_device())
	{
		get_log()<<"Unable to set vertices: invalid directx device, use nya_render::set_device()\n";
		return false;
	}

	if(m_vertex_loc)
	{
		//ToDo: release or refill
	}

	D3D11_SUBRESOURCE_DATA vertex_buffer_data={0};
	vertex_buffer_data.pSysMem=data;
	vertex_buffer_data.SysMemPitch=0;
	vertex_buffer_data.SysMemSlicePitch=0;
	CD3D11_BUFFER_DESC vertex_buffer_desc(size,D3D11_BIND_VERTEX_BUFFER);

	if(get_device()->CreateBuffer(&vertex_buffer_desc,&vertex_buffer_data,&m_vertex_loc)<0)
	{
		get_log()<<"Unable to set vertices: unable to create buffer\n";
		m_vertex_loc=0;
		return false;
	}

	m_vertex_usage=usage;
    m_vertex_stride=vert_stride;
#else
    if(!m_vertex_loc)
    {
        if(!check_init_vbo())
        {
            get_log()<<"Unable to gen vertex data: vbo unsupported\n";
            return false;
        }

        vbo_glGenBuffers(1,&m_vertex_loc);
        vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,m_vertex_loc);
        vbo_glBufferData(GL_ARRAY_BUFFER_ARB,size,data,gl_usage(usage));
        m_allocated_verts_count=vert_count;
        m_vertex_usage=usage;
        m_vertex_stride=vert_stride;
    }
    else
    {
        vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,m_vertex_loc);

        if(vert_count>m_allocated_verts_count || m_vertex_stride!=vert_stride || m_vertex_usage!=usage)
        {
            vbo_glBufferData(GL_ARRAY_BUFFER_ARB,size,data,gl_usage(usage));
            m_allocated_verts_count=vert_count;
            m_vertex_usage=usage;
            m_vertex_stride=vert_stride;
        }
        else
        {
            vbo_glBufferData(GL_ARRAY_BUFFER_ARB,m_allocated_verts_count*m_vertex_stride,0,gl_usage(m_vertex_usage));
            vbo_glBufferSubData(GL_ARRAY_BUFFER_ARB,0,size,data);
        }
    }

    vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,0);
#endif

    m_verts_count=vert_count;

    return true;
}

bool vbo::set_index_data(const void*data,element_size size,unsigned int elements_count,usage_hint usage)
{
    const unsigned int buffer_size=elements_count*size;
    if(buffer_size==0 || !data)
    {
        get_log()<<"Unable to set indices: invalid data\n";
        m_element_count=0;
        return false;
    }

#ifdef DIRECTX11
	if(!get_device())
	{
		get_log()<<"Unable to set indices: invalid directx device, use nya_render::set_device()\n";
		return false;
	}

	if(m_index_loc)
	{
		//ToDo: release or refill
	}

	D3D11_SUBRESOURCE_DATA index_buffer_data={0};
	index_buffer_data.pSysMem=data;
	index_buffer_data.SysMemPitch=0;
	index_buffer_data.SysMemSlicePitch=0;
	CD3D11_BUFFER_DESC index_buffer_desc(buffer_size,D3D11_BIND_INDEX_BUFFER);
	if(nya_render::get_device()->CreateBuffer(&index_buffer_desc,&index_buffer_data,&m_index_loc)<0)
	{
		get_log()<<"Unable to set indices: unable to create buffer\n";
		m_index_loc=0;
		return false;
	}

    m_elements_usage=usage;
    m_element_size=size;
#else
    if(!m_index_loc)
    {
        if(!check_init_vbo())
        {
            get_log()<<"Unable to gen vertex data: vbo unsupported\n";
            return false;
        }

        vbo_glGenBuffers(1,&m_index_loc);
        vbo_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,m_index_loc);
        vbo_glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB,buffer_size,data,gl_usage(usage));
        m_allocated_elements_count=elements_count;
        m_elements_usage=usage;
        m_element_size=size;
    }
    else
    {
        vbo_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,m_index_loc);

        if(elements_count>m_allocated_elements_count || m_element_size!=size || m_elements_usage!=usage)
        {
            vbo_glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB,buffer_size,data,gl_usage(usage));
            m_allocated_elements_count=elements_count;
            m_elements_usage=usage;
            m_element_size=size;
        }
        else
        {
            vbo_glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB,m_allocated_elements_count*m_element_size,0,gl_usage(m_elements_usage));
            vbo_glBufferSubData(GL_ELEMENT_ARRAY_BUFFER_ARB,0,buffer_size,data);
        }
    }

    vbo_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
#endif

    m_element_count=elements_count;

    return true;
}

void vbo::set_vertices(unsigned int offset,unsigned int dimension)
{
#ifdef DIRECTX11
    m_layout.clear();
#endif

    if(dimension==0 || dimension>4)
    {
        m_vertices.has=false;
        return;
    }
    
    m_vertices.has=true;
    m_vertices.offset=offset;
    m_vertices.dimension=dimension;
}

void vbo::set_normals(unsigned int offset)
{
#ifdef DIRECTX11
    m_layout.clear();
#endif

    m_normals.has=true;
    m_normals.offset=offset;
}

void vbo::set_tc(unsigned int tc_idx,unsigned int offset,unsigned int dimension)
{
    if(tc_idx>=vbo_max_tex_coord)
        return;

#ifdef DIRECTX11
    m_layout.clear();
#endif

    attribute &tc=m_tcs[tc_idx];
    if(dimension==0 || dimension>4)
    {
        tc.has=false;
        return;
    }

    tc.has=true;
    tc.offset=offset;
    tc.dimension=dimension;
}

void vbo::set_colors(unsigned int offset,unsigned int dimension)
{
#ifdef DIRECTX11
    m_layout.clear();
#endif

    if(dimension==0 || dimension>4)
    {
        m_colors.has=false;
        return;
    }

    m_colors.has=true;
    m_colors.offset=offset;
    m_colors.dimension=dimension;
}

}

