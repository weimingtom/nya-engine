//https://code.google.com/p/nya-engine/

#include "mesh.h"
#include "scene.h"
#include "render/render.h"
#include "memory/tmp_buffer.h"
#include "memory/memory_reader.h"
#include "math/constants.h"

namespace
{
    struct mmd_material_params
    {
        float diffuse[4];
        float shininess;        
        float specular[3];
        float ambient[3];
    };
}

namespace nya_scene
{

bool mesh::load_pmd(shared_mesh &res,resource_data &data,const char* name)
{
    if(!data.get_size())
        return false;

    nya_memory::memory_reader reader(data.get_data(),data.get_size());
    if(!reader.test("Pmd",3))
        return false;

    if(reader.read<float>()!=1.0f)
        return false;

    reader.skip(20+256); //name and comment

    typedef unsigned int uint;
    typedef unsigned short ushort;
    typedef unsigned char uchar;

    const uint vert_count=reader.read<uint>();
    const size_t pmd_vert_size=sizeof(float)*8+sizeof(ushort)*2+sizeof(uchar)*2;
    if(!reader.check_remained(pmd_vert_size*vert_count))
        return false;

    std::vector<float> vertices(vert_count*11);
    for(size_t i=0;i<vertices.size();i+=11)
    {
        vertices[i]=reader.read<float>();
        vertices[i+1]=reader.read<float>();
        vertices[i+2]=-reader.read<float>();

        for(int j=3;j<7;++j)
            vertices[i+j]=reader.read<float>();

        vertices[i+7]=1.0f-reader.read<float>();

        vertices[i+8]=reader.read<ushort>();
        vertices[i+9]=reader.read<ushort>();
        vertices[i+10]=reader.read<uchar>()/255.0f;
        reader.skip(1);
    }

    res.vbo.set_vertex_data(&vertices[0],sizeof(float)*11,vert_count);
    res.vbo.set_normals(3*sizeof(float));
    res.vbo.set_tc(0,6*sizeof(float),2);
    res.vbo.set_tc(1,8*sizeof(float),3); //skin info

    vertices.clear();

    const uint ind_count=reader.read<uint>();
    const size_t inds_size=sizeof(ushort)*ind_count;
    if(!reader.check_remained(inds_size))
        return false;

    res.vbo.set_index_data(data.get_data(reader.get_offset()),nya_render::vbo::index2b,ind_count);
    reader.skip(inds_size);

    const uint mat_count=reader.read<uint>();
    if(!reader.check_remained(mat_count*(sizeof(mmd_material_params)+2+sizeof(uint)+20)))
        return false;

    res.groups.resize(mat_count);

    shader sh;
    shared_shader sh_;
    sh_.shdr.set_sampler("base",0);
    sh_.samplers_count=1;
    sh_.samplers["diffuse"]=0;
    sh_.vertex="varying vec2 tc; void main() { tc=gl_MultiTexCoord0.xy; gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex; }";
    sh_.pixel="varying vec2 tc; uniform sampler2D base; void main() { gl_FragColor=texture2D(base,tc.xy); }";
    sh_.shdr.add_program(nya_render::shader::vertex,sh_.vertex.c_str());
    sh_.shdr.add_program(nya_render::shader::pixel,sh_.pixel.c_str());
    sh.create(sh_);

    std::string path(name);
    size_t p=path.rfind("/");
    if(p==std::string::npos)
        p=path.rfind("\\");
    if(p==std::string::npos)
        path.clear();
    else
        path.resize(p+1);

    for(uint i=0,ind_offset=0;i<mat_count;++i)
    {
        shared_mesh::group &g=res.groups[i];

        //const mmd_material_params params=
            reader.read<mmd_material_params>();

        reader.skip(2); //toon_idx and edge_flag

        g.offset=ind_offset;
        g.count=reader.read<uint>();
        ind_offset+=g.count;

        const std::string tex_name((const char*)data.get_data(reader.get_offset()),20);
        reader.skip(20);

        std::string base_tex=tex_name;
        size_t pos=base_tex.find('*');
        if(pos!=std::string::npos)
            base_tex.resize(pos);

        texture tex;
        if(!base_tex.empty() && base_tex[0])
            tex.load((path+base_tex).c_str());

        g.mat.set_texture("diffuse",tex);

        g.mat.set_shader(sh);
        //g.mat.set_cull_face(true,nya_render::cull_face::cw);
    }

    const ushort bones_count=reader.read<ushort>();
    if(!reader.check_remained(bones_count*(20+sizeof(short)+5+sizeof(nya_math::vec3))))
        return false;

    for(ushort i=0;i<bones_count;++i)
    {
        const std::string name((const char*)data.get_data(reader.get_offset()),20);
        reader.skip(20);
        const short parent=reader.read<short>();
        reader.skip(5); //child,kind,ik target

        const nya_math::vec3 pos=reader.read<nya_math::vec3>();

        if(res.skeleton.add_bone(name.c_str(),pos,parent)!=i)
            nya_log::get_log()<<"pmd load warning: invalid bone\n";
    }

    const ushort iks_count=reader.read<ushort>();
    //if(!reader.check_remained(iks_count*()))
    //    return false;

    for(ushort i=0;i<iks_count;++i)
    {
        const short target=reader.read<short>();
        const short eff=reader.read<short>();
        const uchar link_count=reader.read<uchar>();
        const ushort count=reader.read<ushort>();
        const float k=reader.read<float>();

        const int ik=res.skeleton.add_ik(target,eff,count,k*nya_math::constants::pi);
        for(int j=0;j<link_count;++j)
        {
            const ushort link=reader.read<ushort>();
            bool limit=false;
            const char *name=res.skeleton.get_bone_name(link);
            if(!name)
            {
                nya_log::get_log()<<"pmd load warning: invalid ik link\n";
                continue;
            }

            if(strcmp(name,"\x8d\xb6\x82\xd0\x82\xb4")==0)
                limit=true;
            else if(strcmp(name,"\x89\x45\x82\xd0\x82\xb4")==0)
                limit=true;

            res.skeleton.add_ik_link(ik,link,limit);
        }
    }

    return true;
}

void mesh::unload()
{
    scene_shared::unload();

    for(size_t i=0;i<m_replaced_materials.size();++i)
        m_replaced_materials[i].release();

    m_replaced_materials.clear();
    m_replaced_materials_idx.clear();
}

void mesh::draw(int idx) const
{
    if(!m_shared.is_valid() || idx>=(int)m_shared->groups.size())
        return;

    nya_scene_internal::transform::set(m_transform);

    m_shared->vbo.bind();

    if(m_replaced_materials_idx.empty())
    {
        if(idx>=0)
        {
            const shared_mesh::group &g=m_shared->groups[idx];
            g.mat.set();
            m_shared->vbo.draw(g.offset,g.count);
            g.mat.unset();
        }
        else
        {
            for(size_t i=0;i<m_shared->groups.size();++i)
            {
                const shared_mesh::group &g=m_shared->groups[i];
                g.mat.set();
                m_shared->vbo.draw(g.offset,g.count);
                g.mat.unset();
            }
        }
    }
    else
    {
        if(idx>=0)
        {
            const shared_mesh::group &g=m_shared->groups[idx];
            const int rep_idx=m_replaced_materials_idx[idx];
            if(rep_idx>=0)
            {
                m_replaced_materials[rep_idx].set();
                m_shared->vbo.draw(g.offset,g.count);
                m_replaced_materials[rep_idx].unset();
            }
            else
            {
                g.mat.set();
                m_shared->vbo.draw(g.offset,g.count);
                g.mat.unset();
            }
        }
        else
        {
            for(size_t i=0;i<m_shared->groups.size();++i)
            {
                const shared_mesh::group &g=m_shared->groups[i];
                const int rep_idx=m_replaced_materials_idx[i];
                if(rep_idx>=0)
                {
                    m_replaced_materials[rep_idx].set();
                    m_shared->vbo.draw(g.offset,g.count);
                    m_replaced_materials[rep_idx].unset();
                }
                else
                {
                    g.mat.set();
                    m_shared->vbo.draw(g.offset,g.count);
                    g.mat.unset();
                }
            }
        }
    }

    m_shared->vbo.unbind();
}

int mesh::get_materials_count() const
{
    if(!m_shared.is_valid())
        return 0;

    return (int)m_shared->groups.size();
}

const material &mesh::get_material(int idx) const
{
    if(!m_shared.is_valid() || idx<0 || idx>=(int)m_shared->groups.size())
    {
        const static material invalid;
        return invalid;
    }

    if(!m_replaced_materials.empty())
    {
        const int replace_idx=m_replaced_materials_idx[idx];
        if(replace_idx>=0)
            return m_replaced_materials[replace_idx];
    }

    return m_shared->groups[idx].mat;
}

material &mesh::modify_material(int idx)
{
    if(!m_shared.is_valid() || idx<0 || idx>=(int)m_shared->groups.size())
    {
        static material invalid;
        invalid=material();
        return invalid;
    }

    if(m_replaced_materials.empty())
        m_replaced_materials_idx.resize(m_shared->groups.size(),-1);

    int &replace_idx=m_replaced_materials_idx[idx];
    if(replace_idx<0)
    {
        replace_idx=(int)m_replaced_materials.size();
        m_replaced_materials.resize(m_replaced_materials.size()+1);
        m_replaced_materials.back()=m_shared->groups[idx].mat;
    }

    return m_replaced_materials[replace_idx];
}

void mesh::set_material(int idx,const material &mat)
{
    if(!m_shared.is_valid() || idx<0 || idx>=(int)m_shared->groups.size())
        return;

    if(m_replaced_materials_idx.empty())
        m_replaced_materials_idx.resize(m_shared->groups.size(),-1);

    if(m_replaced_materials_idx[idx]>=0)
    {
        m_replaced_materials[m_replaced_materials_idx[idx]]=mat;
        return;
    }

    m_replaced_materials_idx[idx]=(int)m_replaced_materials.size();
    m_replaced_materials.resize(m_replaced_materials.size()+1);
    m_replaced_materials.back()=mat;
}

const nya_render::skeleton &mesh::get_skeleton() const
{
    if(!m_shared.is_valid())
    {
        static nya_render::skeleton invalid;
        return invalid;
    }

    return m_shared->skeleton;
}

}
