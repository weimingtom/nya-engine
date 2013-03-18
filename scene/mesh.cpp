//https://code.google.com/p/nya-engine/

#include "mesh.h"
#include "scene.h"
#include "camera.h"
#include "shader.h"
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

    std::vector<float> vertices(vert_count*14);
    for(size_t i=0;i<vertices.size();i+=14)
    {
        vertices[i+11]=vertices[i]=reader.read<float>();
        vertices[i+12]=vertices[i+1]=reader.read<float>();
        vertices[i+13]=vertices[i+2]=-reader.read<float>();

        for(int j=3;j<7;++j)
            vertices[i+j]=reader.read<float>();

        vertices[i+7]=1.0f-reader.read<float>();

        vertices[i+8]=reader.read<ushort>();
        vertices[i+9]=reader.read<ushort>();
        vertices[i+10]=reader.read<uchar>()/100.0f;

        reader.skip(1);
    }

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

    sh_.vertex="uniform vec3 bones_pos[200]; uniform vec4 bones_rot[200];"
                "vec3 tr(vec3 pos,int idx) { vec4 q=bones_rot[idx];"
                "return bones_pos[idx]+pos+cross(q.xyz,cross(q.xyz,pos)+pos*q.w)*2.0; }"
                "varying vec2 tc;"
                "void main()"
                "{  int bone0=int(gl_MultiTexCoord1.x); int bone1=int(gl_MultiTexCoord1.y);"
                "vec3 pos=mix(tr(gl_MultiTexCoord2.xyz,bone1),tr(gl_Vertex.xyz,bone0),gl_MultiTexCoord1.z);"
                "tc=gl_MultiTexCoord0.xy; gl_Position=gl_ModelViewProjectionMatrix*vec4(pos,1.0); }";

    sh_.pixel="varying vec2 tc; uniform sampler2D base; void main() { gl_FragColor=texture2D(base,tc.xy); }";

    sh_.shdr.add_program(nya_render::shader::vertex,sh_.vertex.c_str());
    sh_.shdr.add_program(nya_render::shader::pixel,sh_.pixel.c_str());
    sh_.predefines.resize(2);
    sh_.predefines[0].type=shared_shader::bones_pos;
    sh_.predefines[0].location=sh_.shdr.get_handler("bones_pos");
    sh_.predefines[1].type=shared_shader::bones_rot;
    sh_.predefines[1].location=sh_.shdr.get_handler("bones_rot");
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
        g.mat.set_blend(true,nya_render::blend::src_alpha,nya_render::blend::inv_src_alpha);
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

        nya_math::vec3 pos;
        pos.x=reader.read<float>();
        pos.y=reader.read<float>();
        pos.z=-reader.read<float>();

        if(res.skeleton.add_bone(name.c_str(),pos,parent)!=i)
            nya_log::get_log()<<"pmd load warning: invalid bone\n";
    }

    for(size_t i=0;i<vertices.size();i+=14)
    {
        const nya_math::vec3 bp0=res.skeleton.get_bone_pos(vertices[i+8]);

        vertices[i]-=bp0.x;
        vertices[i+1]-=bp0.y;
        vertices[i+2]-=bp0.z;

        const nya_math::vec3 bp1=res.skeleton.get_bone_pos(vertices[i+9]);

        vertices[i+11]-=bp1.x;
        vertices[i+12]-=bp1.y;
        vertices[i+13]-=bp1.z;
    }

    res.vbo.set_vertex_data(&vertices[0],sizeof(float)*14,vert_count);
    res.vbo.set_normals(3*sizeof(float));
    res.vbo.set_tc(0,6*sizeof(float),2);
    res.vbo.set_tc(1,8*sizeof(float),3); //skin info
    res.vbo.set_tc(2,11*sizeof(float),3); //pos2

    vertices.clear();

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
            const char *name=res.skeleton.get_bone_name(link);
            if(!name)
            {
                nya_log::get_log()<<"pmd load warning: invalid ik link\n";
                continue;
            }

            if(strcmp(name,"\x8d\xb6\x82\xd0\x82\xb4")==0 || strcmp(name,"\x89\x45\x82\xd0\x82\xb4")==0)
                res.skeleton.add_ik_link(ik,link,0.002f,nya_math::constants::pi);
            else
                res.skeleton.add_ik_link(ik,link);
        }
    }

    return true;
}

bool mesh::load(const char *name)
{
    if(!scene_shared::load(name))
        return false;

    if(!m_shared.is_valid())
        return false;

    m_skeleton=m_shared->skeleton;

    m_recalc_aabb=true;
    m_has_aabb=m_shared->aabb.delta*m_shared->aabb.delta>0.0001;

    return true;
}

void mesh::unload()
{
    scene_shared::unload();

    for(size_t i=0;i<m_replaced_materials.size();++i)
        m_replaced_materials[i].release();

    m_replaced_materials.clear();
    m_replaced_materials_idx.clear();
    m_anims.clear();
    m_skeleton=nya_render::skeleton();
    m_aabb=nya_math::aabb();
}

void mesh::draw(int idx) const
{
    if(!m_shared.is_valid() || idx>=(int)m_shared->groups.size())
        return;

    if(m_has_aabb && !get_camera().get_frustum().test_intersect(get_aabb()))
        return;

    nya_scene_internal::transform::set(m_transform);
    shader::set_skeleton(&m_skeleton);

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
    return m_skeleton;
}

int mesh::add_anim(const animation & anim,float weight,float speed)
{
    if(!m_shared.is_valid() || !anim.m_shared.is_valid())
        return -1;

    size_t i=0;
    for(i=0;i<m_anims.size();++i)
    {
        if(!m_anims[i].free)
            break;
    }

    if(i>=m_anims.size())
        m_anims.resize(i+1);

    applied_anim &a=m_anims[i];
    a.free=false;
    a.anim=anim;
    a.weight=weight;
    a.speed=speed;
    a.time=0;
    const nya_render::animation &ra=anim.m_shared->anim;
    a.bones_map.resize(ra.get_bones_count());
    for(int j=0;j<(int)a.bones_map.size();++j)
        a.bones_map[j]=m_skeleton.get_bone_idx(ra.get_bone_name(j));

    return (int)i;
}

void mesh::update(unsigned int dt)
{
    for(size_t i=0;i<m_anims.size();++i) //ToDo: animblend, speed
    {
        applied_anim &a=m_anims[i];
        if(a.free)
            continue;

        a.time+=dt;
        const nya_render::animation &ra=a.anim.m_shared->anim;

        for(int j=0;j<(int)a.bones_map.size();++j)
        {
            const nya_render::animation::bone &b=ra.get_bone(j,a.time);
            m_skeleton.set_bone_transform(a.bones_map[j],b.pos,b.rot);
        }
    }

    m_skeleton.update();
}

const nya_math::aabb &mesh::get_aabb() const
{
    if(!m_shared.is_valid())
        return m_aabb;

    if(m_recalc_aabb)
    {
        m_recalc_aabb=false;
        m_aabb.origin=m_shared->aabb.origin+m_transform.get_pos();
        m_aabb.delta=m_shared->aabb.delta; //ToDo: rotate and scale
    }

    return m_aabb;
}

}
