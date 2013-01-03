//https://code.google.com/p/nya-engine/

#include "mesh.h"
#include "scene.h"
#include "memory/tmp_buffer.h"
#include "memory/memory_reader.h"

namespace
{

bool load_pmd(nya_scene::mesh::shared_mesh &res,size_t data_size,const void*data)
{
    if(!data)
        return false;

    nya_memory::memory_reader reader(data,data_size);
    if(!reader.test("Pmd",3))
        return false;

    if(reader.read<float>()!=1.0f)
        return false;

    reader.skip(20+256); //name and comment

    typedef unsigned int uint;
    typedef unsigned short ushort;
    typedef unsigned char uchar;

    uint vert_count=reader.read<uint>();
    const size_t pmd_vert_size=sizeof(float)*8+sizeof(ushort)*2+sizeof(uchar)*2;
    if(!reader.check_remained(pmd_vert_size*vert_count))
        return false;

    std::vector<float> vertices;
    vertices.resize(vert_count*11);
    for(size_t i=0;i<vertices.size();i+=11)
    {
        for(int j=0;j<8;++j)
            vertices[i+j]=reader.read<float>();

        vertices[i+8]=reader.read<ushort>();
        vertices[i+9]=reader.read<ushort>();
        vertices[i+10]=reader.read<uchar>()/255.0f;
        reader.skip(1);
    }

    res.vbo.gen_vertex_data(&vertices[0],sizeof(float)*11,vert_count);
    res.vbo.set_normals(3*sizeof(float));
    res.vbo.set_tc(0,6*sizeof(float));
    res.vbo.set_tc(1,8*sizeof(float),3); //skin info

    vertices.clear();

    uint ind_count=reader.read<uint>();
    if(!reader.check_remained(sizeof(ushort)*ind_count))
        return false;

    res.vbo.gen_index_data((const char*)data+reader.get_offset(),nya_render::vbo::index2b,ind_count);

    return true;
}

}

namespace nya_scene
{

bool mesh::load(const char *name)
{
    unload();

    if(!name)
        return false;

    m_shared_mesh_ref=get_shared_meshes().access(name);

    return true;
}

void mesh::draw()
{
    if(!m_shared_mesh_ref.is_valid())
        return;

    m_shared_mesh_ref->vbo.bind();
    m_shared_mesh_ref->vbo.draw();
    m_shared_mesh_ref->vbo.unbind();
}

void mesh::unload()
{
    if(m_shared_mesh_ref.is_valid())
        m_shared_mesh_ref.free();
}

bool mesh::shared_meshes_manager::fill_resource(const char *name,shared_mesh &res)
{
    if(!name)
    {
        nya_resources::get_log()<<"unable to load mesh: invalid name\n";
        return false;
    }

    nya_resources::resource_data *file_data=nya_resources::get_resources_provider().access(name);
    if(!file_data)
    {
        nya_resources::get_log()<<"unable to load mesh: unable to acess resource\n";
        return false;
    }

    const size_t data_size=file_data->get_size();
    nya_memory::tmp_buffer_scoped mesh_data(data_size);
    file_data->read_all(mesh_data.get_data());
    file_data->release();

    if(load_pmd(res,data_size,mesh_data.get_data()))
        return true;

    return false;
}

bool mesh::shared_meshes_manager::release_resource(shared_mesh &res)
{
    res.vbo.release();

    return true;
}

}
