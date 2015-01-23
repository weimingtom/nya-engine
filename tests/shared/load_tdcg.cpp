//https://code.google.com/p/nya-engine/

#include "load_tdcg.h"
#include "scene/mesh.h"
#include "memory/memory_reader.h"
#include <zlib.h>

inline unsigned int swap_byte_order(unsigned int ui) { return (ui >> 24) | ((ui<<8) & 0x00FF0000) | ((ui>>8) & 0x0000FF00) | (ui << 24); }

std::string read_string(nya_memory::memory_reader &reader)
{
    std::string out;
    while(reader.get_remained())
    {
        const char c=reader.read<char>();
        if(!c)
            break;

        out.push_back(c);
    }

    return out;
}

bool tdcg_loader::load_hardsave(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name)
{
    nya_memory::memory_reader reader(data.get_data(),data.get_size());

    const char *png_header="\x89PNG\x0d\x0a\x1a\x0a";
    if(!reader.test(png_header, sizeof(png_header)))
        return false;

    std::vector<vert> verts;

    typedef unsigned int uint;
    while(reader.get_remained())
    {
        const uint size=swap_byte_order(reader.read<uint>());
        const uint type=reader.read<uint>();
        if(memcmp(&type,"taOb",4)==0)
        {
            const size_t offset=reader.get_offset();

            const uint type=reader.read<uint>();
            reader.skip(8);
            const uint dst_len=reader.read<uint>();
            const uint src_len=reader.read<uint>();

            if(size<src_len || reader.get_remained()<src_len)
                return false;

            if(memcmp(&type,"FTSO",4)==0)
            {
                nya_memory::tmp_buffer_scoped buf(dst_len);
                uLong extracted_len=dst_len;
                if(uncompress((Byte *)buf.get_data(),&extracted_len,(Byte *)reader.get_data(),src_len)!=Z_OK)
                    return false;

                nya_memory::memory_reader reader(buf.get_data(),buf.get_size());
                if(!reader.test("TSO1", 4))
                    return false;

                const uint bones_count=reader.read<uint>();
                for(uint i=0;i<bones_count;++i)
                    read_string(reader); //ToDo

                const uint bone_mat_count=reader.read<uint>();
                if(bones_count!=bone_mat_count)
                    return false;

                for(uint i=0;i<bone_mat_count;++i)
                    reader.read<nya_math::mat4>(); //ToDo

                const uint tex_count=reader.read<uint>();
                for(uint i=0;i<tex_count;++i)
                {
                    const std::string name=read_string(reader);
                    const std::string file_name=read_string(reader);
                    const uint width=reader.read<uint>();
                    const uint height=reader.read<uint>();
                    const uint channels=reader.read<uint>();
                    reader.skip(width*height*channels);
                }

                const uint shaders_count=reader.read<uint>();
                for(uint i=0;i<shaders_count;++i)
                {
                    const std::string name=read_string(reader);
                    const uint lines_count=reader.read<uint>();
                    for(uint j=0;j<lines_count;++j)
                        read_string(reader);
                }

                const uint shader_params_count=reader.read<uint>();
                for(uint i=0;i<shader_params_count;++i)
                {
                    const std::string name=read_string(reader);
                    const std::string file_name=read_string(reader);
                    const uint lines_count=reader.read<uint>();
                    for(uint j=0;j<lines_count;++j)
                        read_string(reader);
                }

                const uint mesh_count=reader.read<uint>();
                for(uint i=0;i<mesh_count;++i)
                {
                    const std::string name=read_string(reader);
                    const nya_math::mat4 mat=reader.read<nya_math::mat4>();
                    reader.skip(4);
                    const uint groups_count=reader.read<uint>();
                    for(uint j=0;j<groups_count;++j)
                    {
                        nya_scene::shared_mesh::group g;
                        g.name=name;
                        g.offset=(uint)verts.size();
                        g.elem_type=nya_render::vbo::triangle_strip;

                        const uint shader_params_idx=reader.read<uint>();
                        const uint bone_indices_count=reader.read<uint>();
                        for(uint k=0;k<bone_indices_count;++k)
                            reader.read<uint>();

                        g.count=reader.read<uint>();
                        verts.resize(g.offset+g.count);
                        for(uint k=0;k<g.count;++k)
                        {
                            vert &v=verts[g.offset+k];
                            v.pos=reader.read<nya_math::vec3>();
                            v.normal=reader.read<nya_math::vec3>();
                            v.tc=reader.read<nya_math::vec2>();
                            const uint skin_count=reader.read<uint>();
                            if(skin_count>4)
                                return false;

                            memset(v.bone_weight,0,sizeof(v.bone_weight));
                            for(uint l=0;l<skin_count;++l)
                            {
                                v.bone_idx[l]=reader.read<uint>();
                                v.bone_weight[l]=reader.read<float>();
                            }
                        }

                        res.groups.push_back(g);
                    }
                }
            }

            reader.seek(offset);
        }
        else if(memcmp(&type,"IEND",4)==0)
            break;

        if(!reader.skip(size))
            return false;

        reader.skip(4); //CRC
    }

    if(verts.empty())
        return false;

    res.vbo.set_vertex_data(&verts[0],(uint)sizeof(vert),(uint)verts.size());

#define off(st, m) uint((size_t)(&((st *)0)->m))
    res.vbo.set_vertices(off(vert,pos),3);
    res.vbo.set_normals(off(vert,normal));
    res.vbo.set_tc(0,off(vert,tc),2);
    res.vbo.set_tc(1,off(vert,bone_idx),4);
    res.vbo.set_tc(2,off(vert,bone_weight),4);

    //ToDo

    res.materials.resize(1);
    nya_scene::material &m=res.materials.back();
    nya_scene::material::pass &p=m.get_pass(m.add_pass("opaque"));//nya_scene::material::default_pass));
    p.set_shader(nya_scene::shader("tdcg.nsh"));

    return true;
}
