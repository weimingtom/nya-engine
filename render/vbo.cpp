//https://code.google.com/p/nya-engine/

/*
    ToDo:
          is_valid function
          log
          advanced is_supported function (public)
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

namespace nya_render
{

namespace
{
    int current_verts=-1;
    int active_verts=-1;
    int current_inds=-1;
    int active_inds=-1;

    struct vbo_obj_atributes
    {
        struct attribute
        {
            bool has;
            vbo::vertex_atrib_type type:CHAR_BIT;
            short dimension;
            unsigned int offset;

            bool compare(const attribute &a) const { return a.offset==offset && a.dimension==dimension && a.type==type; }

            attribute(): has(false),type(vbo::float32),dimension(0),offset(0) {}
        };

        attribute vertices;
        attribute colors;
        attribute normals;
        attribute tcs[vbo::max_tex_coord];

        unsigned int vertex_stride;

        vbo_obj_atributes(): vertex_stride(0) {}
    };

    struct vbo_obj: public vbo_obj_atributes
    {
        vbo::element_type element_type;
        vbo::index_size element_size;
        unsigned int element_count;
        vbo::usage_hint elements_usage;
        unsigned int allocated_elements_count;

        unsigned int verts_count;
        unsigned int allocated_verts_count;

        vbo::usage_hint vertex_usage;

        DIRECTX11_ONLY(ID3D11Buffer *vertex_loc,*index_loc);
        OPENGL_ONLY(unsigned int vertex_loc,index_loc);

        vbo_obj(): vertex_loc(0),index_loc(0),element_type(vbo::triangles),element_count(0),allocated_elements_count(0),
                   verts_count(0),allocated_verts_count(0) {}

    public:
        static int add() { return get_vbo_objs().add(); }
        static vbo_obj &get(int idx) { return get_vbo_objs().get(idx); }
        static void remove(int idx) { return get_vbo_objs().remove(idx); }
        static void invalidate_all() { return get_vbo_objs().invalidate_all(); }
        static void release_all() { return get_vbo_objs().release_all(); }

    public:
        void release();

    private:
        typedef render_objects<vbo_obj> vbo_objs;
        static vbo_objs &get_vbo_objs()
        {
            static vbo_objs objs;
            return objs;
        }
    };

    vbo_obj_atributes active_attributes;

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

int get_gl_element_type(vbo::vertex_atrib_type type)
{
    switch(type)
    {
        case vbo::float32: return GL_FLOAT;
#ifdef OPENGL_ES
        case vbo::float16: return GL_HALF_FLOAT_OES;
#else
        case vbo::float16: return GL_HALF_FLOAT_ARB;
#endif
    }

    return GL_FLOAT;
}

  #ifndef NO_EXTENSIONS_INIT
    PFNGLGENBUFFERSARBPROC glGenBuffers;
    PFNGLBINDBUFFERARBPROC glBindBuffer;
    PFNGLBUFFERDATAARBPROC glBufferData;
    PFNGLBUFFERSUBDATAARBPROC glBufferSubData;
    PFNGLGETBUFFERSUBDATAARBPROC glGetBufferSubData;
    PFNGLDELETEBUFFERSARBPROC glDeleteBuffers;
    PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTexture;
  #endif

  #ifndef GL_ARRAY_BUFFER
    #define GL_ARRAY_BUFFER GL_ARRAY_BUFFER_ARB
  #endif

  #ifndef GL_DYNAMIC_DRAW
    #define GL_DYNAMIC_DRAW GL_DYNAMIC_DRAW_ARB
  #endif

  #ifndef GL_STATIC_DRAW
    #define GL_STATIC_DRAW GL_STATIC_DRAW_ARB
  #endif

  #ifndef GL_STREAM_DRAW
    #define GL_STREAM_DRAW GL_STREAM_DRAW_ARB
  #endif

  #ifndef GL_ELEMENT_ARRAY_BUFFER
    #define GL_ELEMENT_ARRAY_BUFFER GL_ELEMENT_ARRAY_BUFFER_ARB
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
    if(!(glGenBuffers=(PFNGLGENBUFFERSARBPROC)get_extension("glGenBuffers"))) return false;
    if(!(glBindBuffer=(PFNGLBINDBUFFERARBPROC)get_extension("glBindBuffer"))) return false;
    if(!(glBufferData=(PFNGLBUFFERDATAARBPROC)get_extension("glBufferData"))) return false;
    if(!(glBufferSubData=(PFNGLBUFFERSUBDATAARBPROC)get_extension("glBufferSubData"))) return false;
    if(!(glGetBufferSubData=(PFNGLGETBUFFERSUBDATAARBPROC)get_extension("glGetBufferSubData"))) return false;
    if(!(glDeleteBuffers=(PFNGLDELETEBUFFERSARBPROC)get_extension("glDeleteBuffers"))) return false;
    if(!(glClientActiveTexture=(PFNGLCLIENTACTIVETEXTUREARBPROC)get_extension("glClientActiveTexture"))) return false;
#endif

    initialised=true,failed=false;
    return true;
}
#endif

}

void reset_vbo_state() { active_verts=active_inds=-1; }
void invalidate_vbos() { vbo_obj::invalidate_all(); }
void release_vbos() { vbo_obj::release_all(); reset_vbo_state(); current_verts=current_inds=-1; }

void vbo_obj::release()
{
    DIRECTX11_ONLY(if(vertex_loc) vertex_loc->Release());
    DIRECTX11_ONLY(if(index_loc) index_loc->Release());
    OPENGL_ONLY(if(vertex_loc) glDeleteBuffers(1,&vertex_loc));
    OPENGL_ONLY(if(index_loc) glDeleteBuffers(1,&index_loc));
    *this=vbo_obj();
}

void vbo::bind_verts() const { current_verts=m_verts; }
void vbo::bind_indices() const { current_inds=m_indices; }
void vbo::unbind() { current_verts=current_inds=-1; }

void vbo::draw()
{
    if(current_verts<0)
        return;

    if(current_inds>=0)
        draw(0,vbo_obj::get(current_inds).element_count,vbo_obj::get(current_inds).element_type);
    else
        draw(0,vbo_obj::get(current_verts).verts_count,vbo_obj::get(current_verts).element_type);
}

void vbo::draw(unsigned int count) { draw(0,count); }

void vbo::draw(unsigned int offset,unsigned int count)
{
    if(current_verts<0)
        return;

    if(current_inds>=0)
        draw(offset,count,vbo_obj::get(current_inds).element_type);
    else
        draw(offset,count,vbo_obj::get(current_verts).element_type);
}

void vbo::draw(unsigned int offset,unsigned int count,element_type el_type)
{
    if(current_verts<0 || !count)
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

        if(desc.empty())
            return;

        layout=add_layout(current_verts,&desc[0],desc.size());

        if(!layout)
            return;
    }
	get_context()->IASetInputLayout(layout);
    set_dx_topology(el_type);

    if(current_inds>=0)
    {
        vbo_obj &iobj=vbo_obj::get(current_inds);
        if(offset+count>iobj.element_count)
            return;

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

		get_context()->Draw(count,offset);
	}
#else
    if(current_verts!=active_verts)
    {
        if(!vobj.vertices.has)
            return;

        glBindBuffer(GL_ARRAY_BUFFER,vobj.vertex_loc);

#ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
        glEnableVertexAttribArray(vertex_attribute);
        glVertexAttribPointer(vertex_attribute,vobj.vertices.dimension,get_gl_element_type(vobj.vertices.type),0,
                              vobj.vertex_stride,(void*)(ptrdiff_t)(vobj.vertices.offset));
#else
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(vobj.vertices.dimension,get_gl_element_type(vobj.vertices.type),vobj.vertex_stride,(void*)0);
#endif
        for(unsigned int i=0;i<max_tex_coord;++i)
        {
            const vbo_obj::attribute &tc=vobj.tcs[i];
            if(tc.has)
            {
                //if(vobj.vertex_stride!=active_attributes.vertex_stride || !tc.compare(active_attributes.tcs[i]))
                {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
                    if(!active_attributes.tcs[i].has)
                        glEnableVertexAttribArray(tc0_attribute+i);
                    glVertexAttribPointer(tc0_attribute+i,tc.dimension,get_gl_element_type(tc.type),0,
                                          vobj.vertex_stride,(void*)(ptrdiff_t)(tc.offset));
  #else
                    glClientActiveTexture(GL_TEXTURE0_ARB+i);
                    if(!active_attributes.tcs[i].has)
                        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    glTexCoordPointer(tc.dimension,get_gl_element_type(tc.type),vobj.vertex_stride,(void*)(ptrdiff_t)(tc.offset));
  #endif
                }
            }
            else if(active_attributes.tcs[i].has)
            {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
                glDisableVertexAttribArray(tc0_attribute+i);
  #else
                glClientActiveTexture(GL_TEXTURE0_ARB+i);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  #endif
            }
        }

        if(vobj.normals.has)
        {
            //if(vobj.vertex_stride!=active_attributes.vertex_stride || !vobj.normals.compare(active_attributes.normals))
            {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
                if(!active_attributes.normals.has)
                    glEnableVertexAttribArray(normal_attribute);
                glVertexAttribPointer(normal_attribute,3,get_gl_element_type(vobj.normals.type),1,
                                      vobj.vertex_stride,(void*)(ptrdiff_t)(vobj.normals.offset));
  #else
                if(!active_attributes.normals.has)
                    glEnableClientState(GL_NORMAL_ARRAY);
                glNormalPointer(get_gl_element_type(vobj.normals.type),vobj.vertex_stride,(void*)(ptrdiff_t)(vobj.normals.offset));
  #endif
            }
        }
        else if(active_attributes.normals.has)
        {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
            glDisableVertexAttribArray(normal_attribute);
  #else
            glDisableClientState(GL_NORMAL_ARRAY);
  #endif
        }

        if(vobj.colors.has)
        {
            //if(vobj.vertex_stride!=active_attributes.vertex_stride || !vobj.colors.compare(active_attributes.colors))
            {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
                if(!active_attributes.colors.has)
                    glEnableVertexAttribArray(color_attribute);
                glVertexAttribPointer(color_attribute,vobj.colors.dimension,get_gl_element_type(vobj.colors.type),0,
                                      vobj.vertex_stride,(void*)(ptrdiff_t)(vobj.colors.offset));
  #else
                if(!active_attributes.colors.has)
                    glEnableClientState(GL_COLOR_ARRAY);
                glColorPointer(vobj.colors.dimension,get_gl_element_type(vobj.colors.type),
                               vobj.vertex_stride,(void*)(ptrdiff_t)(vobj.colors.offset));
  #endif
            }
        }
        else if(active_attributes.colors.has)
        {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
            glDisableVertexAttribArray(color_attribute);
  #else
            glDisableClientState(GL_COLOR_ARRAY);
  #endif
        }

        active_verts=current_verts;
        active_attributes=vobj;
    }

    const int gl_elem=get_gl_element_type(el_type);
    if(current_inds>=0)
    {
        vbo_obj &iobj=vbo_obj::get(current_inds);
        if(offset+count>iobj.element_count)
            return;

        if(current_inds!=active_inds)
        {
            if(current_inds>=0)
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,iobj.index_loc);
            else
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

            active_inds=current_inds;
        }

        const unsigned int gl_elem_type=(iobj.element_size==index4b?GL_UNSIGNED_INT:GL_UNSIGNED_SHORT);
        glDrawElements(gl_elem,count,gl_elem_type,(void*)(ptrdiff_t)(offset*iobj.element_size));
    }
    else
    {
        if(offset+count>vobj.verts_count)
            return;

        glDrawArrays(gl_elem,offset,count);
    }
#endif
}

void vbo::release()
{
#ifndef DIRECTX11
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
            for(unsigned int i=0;i<max_tex_coord;++i)
            {
                const vbo_obj::attribute &tc=obj.tcs[i];
                if(tc.has)
                {
  #ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
                    glDisableVertexAttribArray(tc0_attribute+i);
  #else
                    glClientActiveTexture(GL_TEXTURE0_ARB+i);
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
        }
    }

    if(m_indices>=0 && active_inds==m_indices)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
#endif
    if(m_verts>=0)
    {
        if(active_verts==m_verts) active_verts=-1;
        if(current_verts==m_verts) current_verts=-1;
        DIRECTX11_ONLY(remove_layout(m_verts));
        vbo_obj::remove(m_verts);
    }

    if(active_inds==m_indices) active_inds=-1;
    if(current_inds==m_indices) current_inds=-1;

    m_indices=m_verts=-1;
}

#ifndef DIRECTX11
int gl_usage(vbo::usage_hint usage)
{
    switch(usage)
    {
        case vbo::static_draw: return GL_STATIC_DRAW;
        case vbo::dynamic_draw: return GL_DYNAMIC_DRAW;
        case vbo::stream_draw: return GL_STREAM_DRAW;
    }

    return GL_DYNAMIC_DRAW;
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
        log()<<"Unable to set vertices: invalid data\n";
        return false;
    }

#ifdef DIRECTX11
	if(!get_device())
	{
		log()<<"Unable to set vertices: invalid directx device, use nya_render::set_device()\n";
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
		log()<<"Unable to set vertices: unable to create buffer\n";
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
            log()<<"Unable to gen vertex data: vbo unsupported\n";
            return false;
        }

        glGenBuffers(1,&obj.vertex_loc);
        glBindBuffer(GL_ARRAY_BUFFER,obj.vertex_loc);
        glBufferData(GL_ARRAY_BUFFER,size,data,gl_usage(usage));
        obj.allocated_verts_count=vert_count;
        obj.vertex_usage=usage;
        obj.vertex_stride=vert_stride;
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER,obj.vertex_loc);

        if(vert_count>obj.allocated_verts_count || obj.vertex_stride!=vert_stride || obj.vertex_usage!=usage)
        {
            glBufferData(GL_ARRAY_BUFFER,size,data,gl_usage(usage));
            obj.allocated_verts_count=vert_count;
            obj.vertex_usage=usage;
            obj.vertex_stride=vert_stride;
        }
        else
        {
            glBufferData(GL_ARRAY_BUFFER,obj.allocated_verts_count*obj.vertex_stride,0,gl_usage(obj.vertex_usage));
            glBufferSubData(GL_ARRAY_BUFFER,0,size,data);
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
        log()<<"Unable to set indices: invalid data\n";
        obj.element_count=0;
        return false;
    }

#ifdef DIRECTX11
	if(!get_device())
	{
		log()<<"Unable to set indices: invalid directx device, use nya_render::set_device()\n";
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
		log()<<"Unable to set indices: unable to create buffer\n";
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
            log()<<"Unable to gen vertex data: vbo unsupported\n";
            return false;
        }

        glGenBuffers(1,&obj.index_loc);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,obj.index_loc);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,buffer_size,data,gl_usage(usage));
        obj.allocated_elements_count=indices_count;
        obj.elements_usage=usage;
        obj.element_size=size;
    }
    else
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,obj.index_loc);

        if(indices_count>obj.allocated_elements_count || obj.element_size!=size || obj.elements_usage!=usage)
        {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,buffer_size,data,gl_usage(usage));
            obj.allocated_elements_count=indices_count;
            obj.elements_usage=usage;
            obj.element_size=size;
        }
        else
        {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,obj.allocated_elements_count*obj.element_size,0,gl_usage(obj.elements_usage));
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,0,buffer_size,data);
        }
    }

    active_inds=-1;
#endif

    obj.element_count=indices_count;

    return true;
}

void vbo::set_vertices(unsigned int offset,unsigned int dimension,vertex_atrib_type type)
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
    obj.vertices.type=type;

    if(m_verts==active_verts)
        active_verts=-1;
}

void vbo::set_normals(unsigned int offset,vertex_atrib_type type)
{
    if(m_verts<0)
        m_verts=vbo_obj::add();

    vbo_obj &obj=vbo_obj::get(m_verts);

#ifdef DIRECTX11
    remove_layout(m_verts);
#endif

    obj.normals.has=true;
    obj.normals.offset=offset;
    obj.normals.type=type;
}

void vbo::set_tc(unsigned int tc_idx,unsigned int offset,unsigned int dimension,vertex_atrib_type type)
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
    tc.type=type;
}

void vbo::set_colors(unsigned int offset,unsigned int dimension,vertex_atrib_type type)
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
    obj.colors.type=type;
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
    //ToDo
    return false;
#else
    data.allocate(vbo_size);
    glBindBuffer(GL_ARRAY_BUFFER,vobj.vertex_loc);
    current_verts=-1;

#ifdef OPENGL_ES
  #ifdef ANDROID
    //ToDo
    data.free();
    return false;
  #else
    const GLvoid *buf=glMapBufferOES(GL_ARRAY_BUFFER,GL_WRITE_ONLY_OES);
    if(!buf)
    {
        data.free();
        return false;
    }

    memcpy(data.get_data(),buf,vbo_size);
    if(!glUnmapBufferOES(GL_ARRAY_BUFFER))
    {
        data.free();
        return false;
    }
  #endif
#else
    glGetBufferSubData(GL_ARRAY_BUFFER,0,vbo_size,data.get_data());
#endif
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
    //ToDo
    return false;
#else
    data.allocate(ind_size);
    glBindBuffer(GL_ARRAY_BUFFER,obj.index_loc);
    current_inds=-1;

  #ifdef OPENGL_ES
  #ifdef ANDROID
    //ToDo
    data.free();
    return false;
  #else
    const GLvoid *buf=glMapBufferOES(GL_ARRAY_BUFFER,GL_WRITE_ONLY_OES);
    if(!buf)
    {
        data.free();
        return false;
    }

    memcpy(data.get_data(),buf,ind_size);
    if(!glUnmapBufferOES(GL_ARRAY_BUFFER))
    {
        data.free();
        return false;
    }
  #endif
  #else
    glGetBufferSubData(GL_ARRAY_BUFFER,0,ind_size,data.get_data());
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
