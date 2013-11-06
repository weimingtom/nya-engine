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
#include "platform_specific_gl.h"

#include "memory/tmp_buffer.h"

#ifdef DIRECTX11
    #include "shader.h"
#endif

namespace
{
    int current_verts=-1;
    int active_verts=-1;
    int current_inds=-1;
    int active_inds=-1;
}

namespace nya_render
{
    struct vbo_obj
    {
        vbo::element_type element_type;
        vbo::index_size element_size;
        unsigned int element_count;
        vbo::usage_hint elements_usage;
        unsigned int allocated_elements_count;

        unsigned int verts_count;
        unsigned int allocated_verts_count;

        unsigned int vertex_stride;
        vbo::usage_hint vertex_usage;

        struct attribute
        {
            bool has;
            short dimension;
            unsigned int offset;
            
            attribute(): has(false) {}
        };

        attribute vertices;
        attribute colors;
        attribute normals;
        attribute tcs[vbo::max_tex_coord];

#ifdef DIRECTX11
        ID3D11Buffer *vertex_loc;
        ID3D11Buffer *index_loc;
#else
        unsigned int vertex_loc;
        unsigned int index_loc;
#endif
        vbo_obj(): vertex_loc(0), index_loc(0),element_type(vbo::triangles),element_count(0),allocated_elements_count(0),
                   verts_count(0),allocated_verts_count(0) {}

    public:
        static int add() { return get_vbo_objs().add(); }
        static vbo_obj &get(int idx) { return get_vbo_objs().get(idx); }
        static void remove(int idx) { return get_vbo_objs().remove(idx); }

    private:
        typedef render_objects<vbo_obj> vbo_objs;
        static vbo_objs &get_vbo_objs()
        {
            static vbo_objs objs;
            return objs;
        }
    };

#ifndef DIRECTX11
  #ifdef NO_EXTENSIONS_INIT
    #define vbo_glGenBuffers glGenBuffers
    #define vbo_glBindBuffer glBindBuffer
    #define vbo_glBufferData glBufferData
    #define vbo_glBufferSubData glBufferSubData
    #define vbo_glDeleteBuffers glDeleteBuffers
    #define vbo_glClientActiveTexture glClientActiveTexture

    #ifndef OPENGL_ES
        #define vbo_glGetBufferSubData glGetBufferSubData
    #endif

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
    PFNGLGETBUFFERSUBDATAARBPROC vbo_glGetBufferSubData;
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

    vbo_glGetBufferSubData = (PFNGLGETBUFFERSUBDATAARBPROC)  get_extension("glGetBufferSubData");
    if(!vbo_glGetBufferSubData)
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

void vbo::bind_verts() const { current_verts=m_verts; }
void vbo::bind_indices() const { current_inds=m_indices; }
void vbo::unbind() { current_verts=current_inds=-1; }

void vbo::draw()
{
    if(current_verts<0)
        return;

    if(current_inds>=0)
        draw(vbo_obj::get(current_inds).element_count);
    else
        draw(vbo_obj::get(current_verts).verts_count);
}

void vbo::draw(unsigned int count) { draw(0,count); }

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

bool set_dx_topology(vbo::element_type type)
{
    switch(type)
    {
        case vbo::triangles: get_context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); return true;
        case vbo::triangle_strip: get_context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); return true;
        case vbo::points: get_context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST); return true;
        case vbo::lines: get_context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST); return true;
        case vbo::line_strip: get_context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP); return true;
        default: return false;
    }

    return false;
}

#else
int get_gl_element_type(vbo::element_type type)
{
    switch(type)
    {
        case vbo::triangles: return GL_TRIANGLES;;
        case vbo::triangle_strip: return GL_TRIANGLE_STRIP;
        case vbo::points: return GL_POINTS;
        case vbo::lines: return GL_LINES;
        case vbo::line_strip: return GL_LINE_STRIP;
        default: return -1;
    }

    return -1;
}
#endif

void vbo::draw(unsigned int offset,unsigned int count)
{
    if(current_verts<0)
        return;

    shader::apply();
    texture::apply();
    apply_state();

    vbo_obj &vobj=vbo_obj::get(current_verts);

#ifdef DIRECTX11
    UINT zero_offset = 0;
    get_context()->IASetVertexBuffers(0,1,&vobj.vertex_loc,&vobj.vertex_stride,&zero_offset);

    ID3D11InputLayout *layout=get_layout(current_verts);
    if(!layout)
    {
        std::vector<D3D11_INPUT_ELEMENT_DESC> desc;

        if(vobj.vertices.has)
        {
            D3D11_INPUT_ELEMENT_DESC d;
            d.SemanticName="POSITION";
            d.SemanticIndex=0;
            d.Format=get_dx_format(vobj.vertices.dimension);
            d.InputSlot=0;
            d.AlignedByteOffset=vobj.vertices.offset;
            d.InputSlotClass=D3D11_INPUT_PER_VERTEX_DATA;
            d.InstanceDataStepRate=0;
            desc.push_back(d);
        }

        if(vobj.normals.has)
        {
            D3D11_INPUT_ELEMENT_DESC d;
            d.SemanticName="NORMAL";
            d.SemanticIndex=0;
            d.Format=get_dx_format(3);
            d.InputSlot=0;
            d.AlignedByteOffset=vobj.normals.offset;
            d.InputSlotClass=D3D11_INPUT_PER_VERTEX_DATA;
            d.InstanceDataStepRate=0;
            desc.push_back(d);
        }

        if(vobj.colors.has)
        {
            D3D11_INPUT_ELEMENT_DESC d;
            d.SemanticName="COLOR";
            d.SemanticIndex=0;
            d.Format=get_dx_format(vobj.colors.dimension);
            d.InputSlot=0;
            d.AlignedByteOffset=vobj.colors.offset;
            d.InputSlotClass=D3D11_INPUT_PER_VERTEX_DATA;
            d.InstanceDataStepRate=0;
            desc.push_back(d);
        }

        for(int i=0;i<max_tex_coord;++i)
        {
            const vbo_obj::attribute &tc=vobj.tcs[i];
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

        layout=add_layout(current_verts,&desc[0],desc.size());

        if(!layout)
            return;
    }
	get_context()->IASetInputLayout(layout);

    if(current_inds>=0)
    {
        vbo_obj &iobj=vbo_obj::get(current_inds);
        if(offset+count>iobj.element_count)
            return;

        set_dx_topology(iobj.element_type);

        switch(iobj.element_size)
        {
            case index2b: get_context()->IASetIndexBuffer(iobj.index_loc,DXGI_FORMAT_R16_UINT,0); break;
            case index4b: get_context()->IASetIndexBuffer(iobj.index_loc,DXGI_FORMAT_R32_UINT,0); break;
        }

		get_context()->DrawIndexed(count,offset,0);
	}
	else
	{
        if(offset+count>vobj.verts_count)
            return;

        set_dx_topology(vobj.element_type);

		get_context()->Draw(count,offset);
	}
#else
    if(current_verts!=active_verts)
    {
        if(!vobj.vertices.has)
            return;

        vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,vobj.vertex_loc);

#ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
        glEnableVertexAttribArray(vertex_attribute);
        glVertexAttribPointer(vertex_attribute,vobj.vertices.dimension,GL_FLOAT,0,vobj.vertex_stride,(void *)(vobj.vertices.offset));
#else
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(vobj.vertices.dimension,GL_FLOAT,vobj.vertex_stride,(void *)0);
#endif
        for(int i=0;i<max_tex_coord;++i)
        {
            const vbo_obj::attribute &tc=vobj.tcs[i];
            if(tc.has)
            {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
                glEnableVertexAttribArray(tc0_attribute+i);
                glVertexAttribPointer(tc0_attribute+i,tc.dimension,GL_FLOAT,0,vobj.vertex_stride,(void *)(tc.offset));
  #else
                vbo_glClientActiveTexture(GL_TEXTURE0_ARB+i);
                glTexCoordPointer(tc.dimension,GL_FLOAT,vobj.vertex_stride,(void *)(tc.offset));
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  #endif
            }
            else if(active_verts>=0 && vbo_obj::get(active_verts).tcs[i].has)
            {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
                glDisableVertexAttribArray(tc0_attribute+i);
  #else
                vbo_glClientActiveTexture(GL_TEXTURE0_ARB+i);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  #endif
            }
        }

        if(vobj.normals.has)
        {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
            glEnableVertexAttribArray(normal_attribute);
            glVertexAttribPointer(normal_attribute,3,GL_FLOAT,1,vobj.vertex_stride,(void *)(vobj.normals.offset));
  #else
            glEnableClientState(GL_NORMAL_ARRAY);
            glNormalPointer(GL_FLOAT,vobj.vertex_stride,(void *)(vobj.normals.offset));
  #endif
        }
        else if(active_verts>=0 && vbo_obj::get(active_verts).normals.has)
        {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
            glDisableVertexAttribArray(normal_attribute);
  #else
            glDisableClientState(GL_NORMAL_ARRAY);
  #endif
        }

        if(vobj.colors.has)
        {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
            glEnableVertexAttribArray(color_attribute);
            glVertexAttribPointer(color_attribute,vobj.colors.dimension,GL_FLOAT,0,vobj.vertex_stride,(void *)(vobj.colors.offset));
  #else
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(vobj.colors.dimension,GL_FLOAT,vobj.vertex_stride,(void *)(vobj.colors.offset));
  #endif
        }
        else if(active_verts>=0 && vbo_obj::get(active_verts).colors.has)
        {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
            glDisableVertexAttribArray(color_attribute);
  #else
            glDisableClientState(GL_COLOR_ARRAY);
  #endif
        }

        active_verts=current_verts;
    }

    if(current_inds>=0)
    {
        vbo_obj &iobj=vbo_obj::get(current_inds);
        if(offset+count>iobj.element_count)
            return;

        if(current_inds!=active_inds)
        {
            if(current_inds>=0)
                vbo_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,iobj.index_loc);
            else
                vbo_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,0);

            active_inds=current_inds;
        }

        const int gl_elem=get_gl_element_type(iobj.element_type);
        const unsigned int gl_elem_type=(iobj.element_size==index4b?GL_UNSIGNED_INT:GL_UNSIGNED_SHORT);
        glDrawElements(gl_elem,count,gl_elem_type,(void*)(offset*iobj.element_size));
    }
    else
    {
        if(offset+count>vobj.verts_count)
            return;

        const int gl_elem=get_gl_element_type(vobj.element_type);
        glDrawArrays(gl_elem,offset,count);
    }
#endif
}

void vbo::release()
{
#ifdef DIRECTX11
    if(m_verts>=0)
    {
        vbo_obj &obj=vbo_obj::get(m_verts);

        if(active_verts==m_verts)
            active_verts=-1;

        if(current_verts==m_verts)
            current_verts=-1;

        if(obj.vertex_loc)
            obj.vertex_loc->Release();

        remove_layout(m_verts);
    }

    if(m_indices>=0)
    {
        vbo_obj &obj=vbo_obj::get(m_indices);

        if(active_inds==m_indices)
            active_inds=-1;

        if(current_inds==m_verts)
            current_inds=-1;

        if(obj.index_loc)
            obj.index_loc->Release();
    }
#else
    if(m_verts>=0)
    {
        vbo_obj &obj=vbo_obj::get(m_verts);

        if(active_verts==m_verts)
        {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
            glDisableVertexAttribArray(vertex_attribute);
  #else
            glDisableClientState(GL_VERTEX_ARRAY);
  #endif
            for(int i=0;i<max_tex_coord;++i)
            {
                const vbo_obj::attribute &tc=obj.tcs[i];
                if(tc.has)
                {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
                    glDisableVertexAttribArray(tc0_attribute+i);
  #else
                    vbo_glClientActiveTexture(GL_TEXTURE0_ARB+i);
                    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  #endif
                }
            }

            if(obj.normals.has)
            {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
                glDisableVertexAttribArray(normal_attribute);
  #else
                glDisableClientState(GL_NORMAL_ARRAY);
  #endif
            }

            if(obj.colors.has)
            {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
                glDisableVertexAttribArray(color_attribute);
  #else
                glDisableClientState(GL_COLOR_ARRAY);
  #endif
            }

            active_verts=-1;
        }

        if(current_verts==m_verts)
            current_verts=-1;

        if(obj.vertex_loc)
            vbo_glDeleteBuffers(1,&obj.vertex_loc);
    }

    if(m_indices>=0)
    {
        vbo_obj &obj=vbo_obj::get(m_indices);

        if(active_inds==m_indices)
        {
            vbo_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
            active_inds=-1;
        }

        if(current_inds==m_verts)
            current_inds=-1;

        if(obj.index_loc)
            vbo_glDeleteBuffers(1,&obj.index_loc);
    }
#endif

    if(m_verts>=0)
        vbo_obj::remove(m_verts);

    m_indices=m_verts=-1;
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
    if(m_verts<0)
        m_verts=vbo_obj::add();

    vbo_obj &obj=vbo_obj::get(m_verts);

    const unsigned int size=vert_count*vert_stride;
    if(size==0 || !data)
    {
        obj.verts_count=0;
        get_log()<<"Unable to set vertices: invalid data\n";
        return false;
    }

#ifdef DIRECTX11
	if(!get_device())
	{
		get_log()<<"Unable to set vertices: invalid directx device, use nya_render::set_device()\n";
		return false;
	}

	if(obj.vertex_loc)
	{
		//ToDo: release or refill
	}

	D3D11_SUBRESOURCE_DATA vertex_buffer_data={0};
	vertex_buffer_data.pSysMem=data;
	vertex_buffer_data.SysMemPitch=0;
	vertex_buffer_data.SysMemSlicePitch=0;
	CD3D11_BUFFER_DESC vertex_buffer_desc(size,D3D11_BIND_VERTEX_BUFFER);

	if(get_device()->CreateBuffer(&vertex_buffer_desc,&vertex_buffer_data,&obj.vertex_loc)<0)
	{
		get_log()<<"Unable to set vertices: unable to create buffer\n";
		obj.vertex_loc=0;
		return false;
	}

	obj.vertex_usage=usage;
    obj.vertex_stride=vert_stride;
#else
    if(!obj.vertex_loc)
    {
        if(!check_init_vbo())
        {
            get_log()<<"Unable to gen vertex data: vbo unsupported\n";
            return false;
        }

        vbo_glGenBuffers(1,&obj.vertex_loc);
        vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,obj.vertex_loc);
        vbo_glBufferData(GL_ARRAY_BUFFER_ARB,size,data,gl_usage(usage));
        obj.allocated_verts_count=vert_count;
        obj.vertex_usage=usage;
        obj.vertex_stride=vert_stride;
    }
    else
    {
        vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,obj.vertex_loc);

        if(vert_count>obj.allocated_verts_count || obj.vertex_stride!=vert_stride || obj.vertex_usage!=usage)
        {
            vbo_glBufferData(GL_ARRAY_BUFFER_ARB,size,data,gl_usage(usage));
            obj.allocated_verts_count=vert_count;
            obj.vertex_usage=usage;
            obj.vertex_stride=vert_stride;
        }
        else
        {
            vbo_glBufferData(GL_ARRAY_BUFFER_ARB,obj.allocated_verts_count*obj.vertex_stride,0,gl_usage(obj.vertex_usage));
            vbo_glBufferSubData(GL_ARRAY_BUFFER_ARB,0,size,data);
        }
    }

    active_verts=-1;
#endif

    obj.verts_count=vert_count;

    if(!obj.vertices.has)
        set_vertices(0,3);

    return true;
}

bool vbo::set_index_data(const void*data,index_size size,unsigned int indices_count,usage_hint usage)
{
    if(m_verts<0)
        m_verts=vbo_obj::add();

    m_indices=m_verts;
    vbo_obj &obj=vbo_obj::get(m_indices);

    const unsigned int buffer_size=indices_count*size;
    if(buffer_size==0 || !data)
    {
        get_log()<<"Unable to set indices: invalid data\n";
        obj.element_count=0;
        return false;
    }

#ifdef DIRECTX11
	if(!get_device())
	{
		get_log()<<"Unable to set indices: invalid directx device, use nya_render::set_device()\n";
		return false;
	}

	if(obj.index_loc)
	{
		//ToDo: release or refill
	}

	D3D11_SUBRESOURCE_DATA index_buffer_data={0};
	index_buffer_data.pSysMem=data;
	index_buffer_data.SysMemPitch=0;
	index_buffer_data.SysMemSlicePitch=0;
	CD3D11_BUFFER_DESC index_buffer_desc(buffer_size,D3D11_BIND_INDEX_BUFFER);
	if(nya_render::get_device()->CreateBuffer(&index_buffer_desc,&index_buffer_data,&obj.index_loc)<0)
	{
		get_log()<<"Unable to set indices: unable to create buffer\n";
		obj.index_loc=0;
		return false;
	}

    obj.elements_usage=usage;
    obj.element_size=size;
#else
    if(!obj.index_loc)
    {
        if(!check_init_vbo())
        {
            get_log()<<"Unable to gen vertex data: vbo unsupported\n";
            return false;
        }

        vbo_glGenBuffers(1,&obj.index_loc);
        vbo_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,obj.index_loc);
        vbo_glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB,buffer_size,data,gl_usage(usage));
        obj.allocated_elements_count=indices_count;
        obj.elements_usage=usage;
        obj.element_size=size;
    }
    else
    {
        vbo_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,obj.index_loc);

        if(indices_count>obj.allocated_elements_count || obj.element_size!=size || obj.elements_usage!=usage)
        {
            vbo_glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB,buffer_size,data,gl_usage(usage));
            obj.allocated_elements_count=indices_count;
            obj.elements_usage=usage;
            obj.element_size=size;
        }
        else
        {
            vbo_glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB,obj.allocated_elements_count*obj.element_size,0,gl_usage(obj.elements_usage));
            vbo_glBufferSubData(GL_ELEMENT_ARRAY_BUFFER_ARB,0,buffer_size,data);
        }
    }

    active_inds=-1;
#endif

    obj.element_count=indices_count;

    return true;
}

void vbo::set_vertices(unsigned int offset,unsigned int dimension)
{
    if(m_verts<0)
        m_verts=vbo_obj::add();

    vbo_obj &obj=vbo_obj::get(m_verts);

#ifdef DIRECTX11
    remove_layout(m_verts);
#endif

    if(dimension==0 || dimension>4)
    {
        obj.vertices.has=false;
        return;
    }

    obj.vertices.has=true;
    obj.vertices.offset=offset;
    obj.vertices.dimension=dimension;

    if(m_verts==active_verts)
        active_verts=-1;
}

void vbo::set_normals(unsigned int offset)
{
    if(m_verts<0)
        m_verts=vbo_obj::add();

    vbo_obj &obj=vbo_obj::get(m_verts);

#ifdef DIRECTX11
    remove_layout(m_verts);
#endif

    obj.normals.has=true;
    obj.normals.offset=offset;
}

void vbo::set_tc(unsigned int tc_idx,unsigned int offset,unsigned int dimension)
{
    if(tc_idx>=max_tex_coord)
        return;

    if(m_verts<0)
        m_verts=vbo_obj::add();

    vbo_obj &obj=vbo_obj::get(m_verts);

#ifdef DIRECTX11
    remove_layout(m_verts);
#endif

    vbo_obj::attribute &tc=obj.tcs[tc_idx];
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
    if(m_verts<0)
        m_verts=vbo_obj::add();

    vbo_obj &obj=vbo_obj::get(m_verts);

#ifdef DIRECTX11
    remove_layout(m_verts);
#endif

    if(dimension==0 || dimension>4)
    {
        obj.colors.has=false;
        return;
    }

    obj.colors.has=true;
    obj.colors.offset=offset;
    obj.colors.dimension=dimension;
}

void vbo::set_element_type(element_type type)
{
    if(m_verts<0)
        m_verts=vbo_obj::add();

    vbo_obj::get(m_verts).element_type=type;
}

bool vbo::get_vertex_data( nya_memory::tmp_buffer_ref &data ) const
{
    data.free();
    if(m_verts<0)
        return false;

    const vbo_obj &vobj=vbo_obj::get(m_verts);
    if(!vobj.vertices.has || !vobj.vertex_loc )
        return false;

    size_t vbo_size=vobj.vertex_stride*vobj.verts_count;
    if(!vbo_size)
        return false;

#ifdef DIRECTX11
#else
    data.allocate(vbo_size);
    vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,vobj.vertex_loc);

#ifdef OPENGL_ES
    const GLvoid *buf=glMapBufferOES(GL_ARRAY_BUFFER,GL_WRITE_ONLY_OES);
    if(!buf)
        return false;

    memcpy(data.get_data(),buf,vbo_size);
    return glUnmapBufferOES(GL_ARRAY_BUFFER);
#else
    vbo_glGetBufferSubData(GL_ARRAY_BUFFER_ARB,0,vbo_size,data.get_data());
#endif
    current_verts=-1;
#endif

    return true;
}

bool vbo::get_index_data( nya_memory::tmp_buffer_ref &data ) const
{
    data.free();
    if(m_indices<0)
        return false;

    const vbo_obj &obj=vbo_obj::get(m_indices);
    size_t ind_size=obj.element_count*obj.element_size;
    if(!ind_size)
        return false;

#ifdef DIRECTX11
#else
    data.allocate(ind_size);
    vbo_glBindBuffer(GL_ARRAY_BUFFER_ARB,obj.index_loc);
    current_inds=-1;

  #ifdef OPENGL_ES
    const GLvoid *buf=glMapBufferOES(GL_ARRAY_BUFFER,GL_WRITE_ONLY_OES);
    if(!buf)
        return false;

    memcpy(data.get_data(),buf,ind_size);
    return glUnmapBufferOES(GL_ARRAY_BUFFER);
  #else
    vbo_glGetBufferSubData(GL_ARRAY_BUFFER_ARB,0,ind_size,data.get_data());
  #endif
#endif

    return true;
}

vbo::index_size vbo::get_index_size() const
{
    if(m_indices<0)
        return index2b;

    return vbo_obj::get(m_indices).element_size;
}

vbo::element_type vbo::get_element_type() const
{
    if(m_indices<0)
    {
        if(m_verts<0)
            return triangles;

        return vbo_obj::get(m_verts).element_type;
    }

    return vbo_obj::get(m_indices).element_type;
}

unsigned int vbo::get_vert_stride() const
{
    if(m_verts<0)
        return 0;

    return vbo_obj::get(m_verts).vertex_stride;
}

unsigned int vbo::get_vert_offset() const
{
    if(m_verts<0)
        return 0;

    return vbo_obj::get(m_verts).vertices.offset;
}

unsigned int vbo::get_vert_dimension() const
{
    if(m_verts<0)
        return 0;

    return vbo_obj::get(m_verts).vertices.dimension;
}

unsigned int vbo::get_normals_offset() const
{
    if(m_verts<0)
        return 0;

    return vbo_obj::get(m_verts).normals.offset;
}

unsigned int vbo::get_tc_offset(unsigned int idx) const
{
    if(m_verts<0 || idx>=max_tex_coord)
        return 0;

    return vbo_obj::get(m_verts).tcs[idx].offset;
}

unsigned int vbo::get_tc_dimension(unsigned int idx) const
{
    if(m_verts<0 || idx>=max_tex_coord)
        return 0;

    return vbo_obj::get(m_verts).tcs[idx].dimension;
}

unsigned int vbo::get_colors_offset() const
{
    if(m_verts<0)
        return 0;

    return vbo_obj::get(m_verts).colors.offset;
}

unsigned int vbo::get_colors_dimension() const
{
    if(m_verts<0)
        return 0;

    return vbo_obj::get(m_verts).colors.dimension;
}

unsigned int vbo::get_verts_count() const
{
    if(m_verts<0)
        return 0;

    return vbo_obj::get(m_verts).verts_count;
}

unsigned int vbo::get_indices_count() const
{
    if(m_indices<0)
        return 0;

    return vbo_obj::get(m_indices).element_count;
}

}
