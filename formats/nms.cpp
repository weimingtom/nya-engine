//https://code.google.com/p/nya-engine/

#include "nms.h"
#include "memory/memory_reader.h"
#include "memory/memory_writer.h"
#include <stdio.h>
#include <stdint.h>

namespace { const char nms_sign[]={'n','y','a',' ','m','e','s','h'}; }

namespace nya_formats
{

static std::string read_string(nya_memory::memory_reader &reader)
{
    unsigned short size=reader.read<unsigned short>();
    const char *str=(const char *)reader.get_data();
    if(!size || !str || !reader.check_remained(size))
    {
        reader.skip(size);
        return "";
    }

    reader.skip(size);

    return std::string(str,size);
}

bool nms::read_chunks_info(const void *data,size_t size)
{
    *this=nms();

    if(!data || !size)
        return false;

    const char *cdata=(const char *)data;
    const char *const data_end=cdata+size;
    header h;
    cdata+=read_header(h,data,size);
    if(cdata==data)
        return false;

    version=h.version;
    chunks.resize(h.chunks_count);
    for(size_t i=0;i<chunks.size();++i)
    {
        cdata+=read_chunk_info(chunks[i],cdata,data_end-cdata);
        cdata+=chunks[i].size;
        if(cdata>data_end)
        {
            *this=nms();
            return false;
        }
    }

    return true;
}

size_t nms::read_header(header &out_header,const void *data,size_t size)
{
    out_header.version=out_header.chunks_count=0;
    if(size<nms_header_size)
        return 0;

    nya_memory::memory_reader reader(data,size);
    if(!reader.test(nms_sign,sizeof(nms_sign)))
        return 0;

    out_header.version=reader.read<uint32_t>();
    if(!out_header.version)
        return 0;

    out_header.chunks_count=reader.read<uint32_t>();

    return reader.get_offset();
}

size_t nms::read_chunk_info(chunk_info &out_chunk_info,const void *data,size_t size)
{
    if(size<sizeof(uint32_t)*2)
        return 0;

    out_chunk_info.type=out_chunk_info.size=0;
    nya_memory::memory_reader reader(data,size);
    out_chunk_info.type=reader.read<uint32_t>();
    out_chunk_info.size=reader.read<uint32_t>();
    out_chunk_info.data=reader.get_data();

    return reader.get_offset();
}

size_t nms::get_nms_size()
{
    size_t size=nms_header_size;
    for(size_t i=0;i<chunks.size();++i)
        size+=get_chunk_write_size(chunks[i].size);

    return size;
}

size_t nms::write_to_buf(void *data,size_t size)
{
    if(!data)
        return 0;

    char *cdata=(char *)data;
    const char *const data_end=cdata+size;

    header h;
    h.version=version;
    h.chunks_count=(unsigned int)chunks.size();
    cdata+=write_header_to_buf(h,data,size);
    if(data==cdata)
        return 0;

    for(size_t i=0;i<chunks.size();++i)
        cdata+=write_chunk_to_buf(chunks[i],cdata,data_end-cdata);

    return cdata-(char *)data;
}

size_t nms::write_header_to_buf(const header &h,void *to_data,size_t to_size)
{
    if(!to_data || to_size<nms_header_size)
        return 0;

    nya_memory::memory_writer writer(to_data,to_size);
    writer.write(nms_sign,sizeof(nms_sign));
    writer.write_uint(h.version);
    writer.write_uint(h.chunks_count);

    return writer.get_offset();
}

size_t nms::get_chunk_write_size(size_t chunk_data_size) { return chunk_data_size+sizeof(uint32_t)*2; }

size_t nms::write_chunk_to_buf(const chunk_info &chunk,void *to_data,size_t to_size)
{
    if(!to_data || to_size<get_chunk_write_size(chunk.size))
        return 0;

    nya_memory::memory_writer writer(to_data,to_size);
    writer.write_uint(chunk.type);
    writer.write_uint(chunk.size);
    if(!chunk.size)
        return writer.get_offset();

    if(!chunk.data)
        return 0;

    writer.write(chunk.data,chunk.size);
    return writer.get_offset();
}

size_t nms_mesh_chunk::read_header(const void *data, size_t size, int version)
{
    *this=nms_mesh_chunk();

    if(!data || !size)
        return 0;

    typedef uint32_t uint;
    typedef uint16_t ushort;
    typedef uint8_t uchar;

    nya_memory::memory_reader reader(data,size);

    aabb_min=reader.read<nya_math::vec3>();
    aabb_max=reader.read<nya_math::vec3>();

    elements.resize(reader.read<uchar>());
    for(size_t i=0;i<elements.size();++i)
    {
        element &e=elements[i];

        e.offset=vertex_stride;
        e.type=reader.read<uchar>();
        e.dimension=reader.read<uchar>();
        uchar data_type=float32;
        if(version>1)
        {
            data_type=reader.read<uchar>();
            if(data_type!=float16 && data_type!=float32)
            {
                *this=nms_mesh_chunk();
                return 0;
            }
        }

        e.data_type=(vertex_atrib_type)data_type;
        read_string(reader); //semantics

        vertex_stride+=e.dimension*sizeof(float);
    }

    if(!vertex_stride)
    {
        *this=nms_mesh_chunk();
        return 0;
    }

    verts_count=reader.read<uint>();
    if(!reader.check_remained(verts_count*vertex_stride))
    {
        *this=nms_mesh_chunk();
        return 0;
    }

    vertices_data=reader.get_data();

    if(!reader.skip(verts_count*vertex_stride))
    {
        *this=nms_mesh_chunk();
        return 0;
    }

    index_size=reader.read<uchar>();
    if(index_size)
    {
        indices_count=reader.read<uint>();
        if(!reader.check_remained(indices_count*index_size))
        {
            *this=nms_mesh_chunk();
            return 0;
        }
        indices_data=reader.get_data();

        if(!reader.skip(index_size*indices_count))
        {
            *this=nms_mesh_chunk();
            return 0;
        }
    }

    lods.resize(reader.read<ushort>());
    for(size_t i=0;i<lods.size();++i)
    {
        lod &l=lods[i];
        l.groups.resize(reader.read<ushort>());
        for(size_t j=0;j<l.groups.size();++j)
        {
            group &g=l.groups[j];
            g.name=read_string(reader);

            g.aabb_min=reader.read<nya_math::vec3>();
            g.aabb_max=reader.read<nya_math::vec3>();

            g.material_idx=reader.read<ushort>();
            g.offset=reader.read<uint>();
            g.count=reader.read<uint>();
            g.element_type=version>1?draw_element_type(reader.read<uchar>()):triangles;
        }
    }

    return reader.get_offset();
}

bool nms_material_chunk::read(const void *data,size_t size,int version)
{
    *this=nms_material_chunk();

    if(!data || !size)
        return false;

    nya_memory::memory_reader reader(data,size);

    materials.resize(reader.read<uint16_t>());
    for(size_t i=0;i<materials.size();++i)
    {
        material_info &m=materials[i];
        m.name=read_string(reader);

        m.textures.resize(reader.read<uint16_t>());
        for(size_t j=0;j<m.textures.size();++j)
        {
            m.textures[j].semantics=read_string(reader);
            m.textures[j].filename=read_string(reader);
        }

        m.strings.resize(reader.read<uint16_t>());
        for(size_t j=0;j<m.strings.size();++j)
        {
            m.strings[j].name=read_string(reader);
            m.strings[j].value=read_string(reader);
        }

        m.vectors.resize(reader.read<uint16_t>());
        for(size_t j=0;j<m.vectors.size();++j)
        {
            m.vectors[j].name=read_string(reader);
            m.vectors[j].value.x=reader.read<float>();
            m.vectors[j].value.y=reader.read<float>();
            m.vectors[j].value.z=reader.read<float>();
            m.vectors[j].value.w=reader.read<float>();
        }

        m.ints.resize(reader.read<uint16_t>());
        for(size_t j=0;j<m.ints.size();++j)
        {
            m.ints[j].name=read_string(reader);
            m.ints[j].value=reader.read<int32_t>();
        }
    }

    return true;
}

size_t nms_material_chunk::write_to_buf(void *to_data,size_t to_size)
{
    nya_memory::memory_writer writer(to_data,to_size);
    writer.write_ushort((unsigned short)materials.size());
    for(size_t i=0;i<materials.size();++i)
    {
        material_info &m=materials[i];
        writer.write_string(m.name);

        writer.write_ushort((unsigned short)m.textures.size());
        for(size_t j=0;j<m.textures.size();++j)
        {
            writer.write_string(m.textures[j].semantics);
            writer.write_string(m.textures[j].filename);
        }

        writer.write_ushort((unsigned short)m.strings.size());
        for(size_t j=0;j<m.strings.size();++j)
        {
            writer.write_string(m.strings[j].name);
            writer.write_string(m.strings[j].value);
        }

        writer.write_ushort((unsigned short)m.vectors.size());
        for(size_t j=0;j<m.vectors.size();++j)
        {
            writer.write_string(m.vectors[j].name);
            writer.write_float(m.vectors[j].value.x);
            writer.write_float(m.vectors[j].value.y);
            writer.write_float(m.vectors[j].value.z);
            writer.write_float(m.vectors[j].value.w);
        }

        writer.write_ushort((unsigned short)m.ints.size());
        for(size_t j=0;j<m.ints.size();++j)
        {
            writer.write_string(m.ints[j].name);
            writer.write_int(m.ints[j].value);
        }
    }

    return writer.get_offset();
}

bool nms_skeleton_chunk::read(const void *data,size_t size,int version)
{
    *this=nms_skeleton_chunk();

    if(!data || !size)
        return false;

    nya_memory::memory_reader reader(data,size);

    bones.resize(reader.read<int32_t>());
    for(size_t i=0;i<bones.size();++i)
    {
        bone &b=bones[i];
        b.name=read_string(reader);
        b.rot=reader.read<nya_math::quat>();
        b.pos=reader.read<nya_math::vec3>();
        b.parent=reader.read<int32_t>();
    }

    return true;
}

}
