//https://code.google.com/p/nya-engine/

#include "nms.h"
#include "memory/memory_reader.h"
#include <stdio.h>
#include <stdint.h>

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

    nya_memory::memory_reader reader(data,size);
    if(!reader.test("nya mesh",8))
        return false;

    version=reader.read<uint32_t>();

    chunks.resize(reader.read<uint32_t>());
    for(size_t i=0;i<chunks.size();++i)
    {
        chunk_info &c=chunks[i];
        c.type=reader.read<uint32_t>();
        c.size=reader.read<uint32_t>();
        c.data=reader.get_data();

        if(!reader.check_remained(c.size))
        {
            *this=nms();
            return false;
        }

        reader.skip(c.size);
    }

    return true;
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
