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

    std::string read_string(nya_memory::memory_reader &reader)
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
}

namespace nya_scene
{

bool mesh::load_nms_mesh_section(shared_mesh &res,const void *data,size_t size)
{
    nya_memory::memory_reader reader(data,size);

    typedef unsigned int uint;
    typedef unsigned short ushort;
    typedef unsigned char uchar;

    const nya_math::vec3 aabb_min(reader.read<float>(),reader.read<float>(),reader.read<float>());
    const nya_math::vec3 aabb_max(reader.read<float>(),reader.read<float>(),reader.read<float>());
    res.aabb.delta=(aabb_max-aabb_min)*0.5f;
    res.aabb.origin=aabb_min+res.aabb.delta;

    uint vertex_stride=0;
    const uchar el_count=reader.read<uchar>();
    for(uchar i=0;i<el_count;++i)
    {
        enum el_type
        {
            pos,
            normal,
            color,
            tc0=100
        };

        const uchar type=reader.read<uchar>();
        const uchar dimension=reader.read<uchar>();
        read_string(reader); //semantics

        switch(type)
        {
            case pos: res.vbo.set_vertices(vertex_stride,dimension); break;
            case normal: res.vbo.set_normals(vertex_stride); break;
            case color: res.vbo.set_colors(vertex_stride,dimension); break;
            default: res.vbo.set_tc(type-tc0,vertex_stride,dimension); break;
        };

        vertex_stride+=dimension*sizeof(float);
    }

    if(!vertex_stride)
        return false;

    const uint verts_count=reader.read<uint>();
    if(!reader.check_remained(verts_count*vertex_stride))
        return false;

    res.vbo.set_vertex_data(reader.get_data(),vertex_stride,verts_count);
    reader.skip(verts_count*vertex_stride);

    const uchar index_size=reader.read<uchar>();
    if(index_size)
    {
        uint inds_count=reader.read<uint>();
        if(!reader.check_remained(inds_count*sizeof(ushort)))
            return false;

        switch(index_size)
        {
            case 2: res.vbo.set_index_data(reader.get_data(),nya_render::vbo::index2b,inds_count); break;
            case 4: res.vbo.set_index_data(reader.get_data(),nya_render::vbo::index4b,inds_count); break;
            default: return false;
        }

        reader.skip(index_size*inds_count);
    }

    const ushort lods_count=reader.read<ushort>();
    for(ushort i=0;i<lods_count;++i)
    {
        const ushort groups_count=reader.read<ushort>();
        res.groups.resize(groups_count);
        for(ushort j=0;j<groups_count;++j)
        {
            shared_mesh::group &g=res.groups[j];
            g.name=read_string(reader);

            const nya_math::vec3 aabb_min=reader.read<nya_math::vec3>(); //ToDo
            const nya_math::vec3 aabb_max=reader.read<nya_math::vec3>();

            g.material_idx=reader.read<ushort>();
            g.offset=reader.read<uint>();
            g.count=reader.read<uint>();
        }

        break; //ToDo: load all lods
    }

    return true;
}

bool mesh::load_nms_skeleton_section(shared_mesh &res,const void *data,size_t size)
{
    nya_memory::memory_reader reader(data,size);

    const int bones_count=reader.read<int>();
    for(int i=0;i<bones_count;++i)
    {
        const std::string name=read_string(reader);
        const nya_math::quat rot=reader.read<nya_math::quat>(); //ToDo
        const nya_math::vec3 pos=reader.read<nya_math::vec3>();
        const int parent=reader.read<int>();

        res.skeleton.add_bone(name.c_str(),pos,parent);
    }

    return true;
}

bool mesh::load_nms_material_section(shared_mesh &res,const void *data,size_t size)
{
    nya_memory::memory_reader reader(data,size);

    typedef unsigned short ushort;

    const ushort materials_count=reader.read<ushort>();
    res.materials.resize(materials_count);
    for(ushort i=0;i<materials_count;++i)
    {
        material &m=res.materials[i];
        std::string name=read_string(reader);
        m.set_name(name.c_str());

        const ushort textures_count=reader.read<ushort>();
        for(ushort j=0;j<textures_count;++j)
        {
            std::string semantics=read_string(reader);
            std::string name=read_string(reader);

            texture tex;
            tex.load(name.c_str());
            m.set_texture(semantics.c_str(),tex);
        }

        const ushort strings_count=reader.read<ushort>();
        for(ushort j=0;j<strings_count;++j)
        {
            std::string name=read_string(reader);
            std::string value=read_string(reader);

            if(name=="nya_shader")
            {
                shader sh;
                sh.load(value.c_str());
                m.set_shader(sh);
            }
            else if(name=="nya_blend")
                m.set_blend(true,nya_render::blend::src_alpha,nya_render::blend::inv_src_alpha); //ToDo
        }

        const ushort vec4_count=reader.read<ushort>();
        for(ushort j=0;j<vec4_count;++j)
        {
            read_string(reader);
            reader.read<float>();
            reader.read<float>();
            reader.read<float>();
            reader.read<float>();
        }

        const ushort ints_count=reader.read<ushort>();
        for(ushort j=0;j<ints_count;++j)
        {
            read_string(reader);
            reader.read<int>();
        }
    }

    return true;
}

bool mesh::load_nms(shared_mesh &res,resource_data &data,const char* name)
{
    nya_memory::memory_reader reader(data.get_data(),data.get_size());
    if(!reader.test("nya mesh",8))
        return false;

    typedef unsigned int uint;

    if(reader.read<uint>()!=1)
        return false;

    enum section_type
    {
        mesh_data,
        skeleton,
        materials
    };

    const uint num_sections=reader.read<uint>();
    for(uint i=0;i<num_sections;++i)
    {
        const uint type=reader.read<uint>();
        const uint size=reader.read<uint>();

        if(!reader.check_remained(size))
        {
            nya_log::get_log()<<"nms load error: chunk size is bigger than remained data size\n";
            return false;
        }

        switch(type)
        {
            case mesh_data: load_nms_mesh_section(res,reader.get_data(),size); break;
            case skeleton: load_nms_skeleton_section(res,reader.get_data(),size); break;
            case materials: load_nms_material_section(res,reader.get_data(),size); break;
            default: nya_log::get_log()<<"nms load warning: unknown chunk type\n";
        };

        reader.skip(size);
    }

    data.free();

    return true;
}

bool mesh::load(const char *name)
{
    if(!scene_shared::load(name))
        return false;

    if(!m_shared.is_valid())
        return false;

    for(size_t i=0;i<m_replaced_materials.size();++i)
        m_replaced_materials[i].release();

    m_replaced_materials.clear();
    m_replaced_materials_idx.clear();
    m_anims.clear();

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

void mesh::draw_group(int idx) const
{
    const shared_mesh::group &g=m_shared->groups[idx];
    if(g.material_idx>=m_shared->materials.size())
        return;

    const int rep_idx=(m_replaced_materials_idx.empty()?
                       -1:m_replaced_materials_idx[g.material_idx]);
    if(rep_idx>=0)
    {
        m_replaced_materials[rep_idx].set();
        m_shared->vbo.draw(g.offset,g.count);
        m_replaced_materials[rep_idx].unset();
    }
    else
    {
        const material &mat=m_shared->materials[g.material_idx];
        mat.set();
        m_shared->vbo.draw(g.offset,g.count);
        mat.unset();
    }
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

    if(idx>=0)
    {
        if(idx<(int)m_shared->groups.size())
            draw_group(idx);
    }
    else
    {
        for(int i=0;i<(int)m_shared->groups.size();++i)
            draw_group(i);
    }

    m_shared->vbo.unbind();
}

int mesh::get_groups_count() const
{
    if(!m_shared.is_valid())
        return 0;

    return (int)m_shared->groups.size();
}

const char *mesh::get_group_name(int group_idx) const
{
    if(!m_shared.is_valid())
        return 0;

    if(group_idx<0 || group_idx>=(int)m_shared->groups.size())
        return 0;

    return m_shared->groups[group_idx].name.c_str();
}

int mesh::get_material_idx(int group_idx) const
{
    if(!m_shared.is_valid())
        return 0;

    if(group_idx<0 || group_idx>=(int)m_shared->groups.size())
        return 0;

    return m_shared->groups[group_idx].material_idx;
}

int mesh::get_materials_count() const
{
    if(!m_shared.is_valid())
        return 0;

    return (int)m_shared->materials.size();
}

const material &mesh::get_material(int idx) const
{
    if(!m_shared.is_valid() || idx<0 || idx>=(int)m_shared->materials.size())
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

    return m_shared->materials[idx];
}

material &mesh::modify_material(int idx)
{
    if(!m_shared.is_valid() || idx<0 || idx>=(int)m_shared->materials.size())
    {
        static material invalid;
        invalid=material();
        return invalid;
    }

    if(m_replaced_materials.empty())
        m_replaced_materials_idx.resize(m_shared->materials.size(),-1);

    int &replace_idx=m_replaced_materials_idx[idx];
    if(replace_idx<0)
    {
        replace_idx=(int)m_replaced_materials.size();
        m_replaced_materials.resize(m_replaced_materials.size()+1);
        m_replaced_materials.back()=m_shared->materials[idx];
    }

    return m_replaced_materials[replace_idx];
}

void mesh::set_material(int idx,const material &mat)
{
    if(!m_shared.is_valid() || idx<0 || idx>=(int)m_shared->materials.size())
        return;

    if(m_replaced_materials_idx.empty())
        m_replaced_materials_idx.resize(m_shared->materials.size(),-1);

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

void mesh::set_anim(const animation_proxy & anim,int layer)
{
    if(!m_shared.is_valid() || layer<0)
        return;

    int idx=-1;
    for(int i=0;i<(int)m_anims.size();++i)
    {
        if(m_anims[i].layer==layer)
        {
            idx=i;
            break;
        }
    }

    if(!anim.is_valid())
    {
        if(idx>=0)
            m_anims.erase(m_anims.begin()+idx);

        return;
    }

    if(idx<0)
    {
        idx=(int)m_anims.size();
        m_anims.resize(m_anims.size()+1);
    }

    applied_anim &a=m_anims[idx];
    a.layer=layer;
    a.anim=anim;
    a.time=0;
    a.version=0;
}

void mesh::anim_set_time(applied_anim &a,float t)
{
    if(!a.anim.is_valid())
    {
        a.time=0.0f;
        return;
    }

    const unsigned int anim_len=a.anim->m_range_to-a.anim->m_range_from;
    if(!anim_len)
    {
        a.time=0.0f;
        return;
    }

    a.time=t;

    const float anim_len_f=float(anim_len);
    if(!a.anim->get_loop())
    {
        if(a.time>anim_len_f)
            a.time=anim_len_f;

        if(a.time<0.0f)
            a.time=0.0f;

        return;
    }

    while(a.time>anim_len_f)
        a.time-=anim_len_f;

    while(a.time<0.0f)
        a.time+=anim_len_f;
}

void mesh::anim_update_mapping(applied_anim &a)
{
    if(!a.anim->m_shared.is_valid())
        return;

    const nya_render::animation &ra=a.anim->m_shared->anim;
    a.bones_map.resize(m_skeleton.get_bones_count(),-1);

    for(int j=0;j<(int)ra.get_bones_count();++j)
    {
        int idx=m_skeleton.get_bone_idx(ra.get_bone_name(j));
        if(idx<0)
            continue;

        a.bones_map[idx]=j;
    }
}

const animation_proxy & mesh::get_anim(int layer) const
{
    for(int i=0;i<(int)m_anims.size();++i)
    {
        if(m_anims[i].layer==layer)
            return m_anims[i].anim;
    }

    static const animation_proxy invalid;
    return invalid;
}

void mesh::set_anim_time(unsigned int time,int layer)
{
    for(int i=0;i<(int)m_anims.size();++i)
    {
        if(m_anims[i].layer==layer)
        {
            anim_set_time(m_anims[i],float(time));
            return;
        }
    }
}

unsigned int mesh::get_anim_time(int layer) const
{
    for(int i=0;i<(int)m_anims.size();++i)
    {
        if(m_anims[i].layer==layer)
            return (unsigned int)m_anims[i].time;
    }

    return 0;
}

bool mesh::is_anim_finished(int layer) const
{
    for(int i=0;i<(int)m_anims.size();++i)
    {
        if(m_anims[i].layer==layer)
        {
            if(!m_anims[i].anim.is_valid())
                return true;

            if(m_anims[i].anim->get_loop())
                return false;

            const float eps = 0.001f;
            if(m_anims[i].anim->m_speed>=0.0f)
                return fabs(m_anims[i].anim->m_range_to-m_anims[i].anim->m_range_from-m_anims[i].time)<eps;
            else
                return fabs(m_anims[i].time)<eps;
        }
    }

    return true;
}

void mesh::set_bone_pos(int bone_idx,const nya_math::vec3 &pos,bool additive)
{
    if(bone_idx<0 || bone_idx>=m_skeleton.get_bones_count())
        return;

    bone_control &b=m_bone_controls[bone_idx];
    b.pos=pos;
    b.pos_ctrl=additive?bone_additive:bone_override;
}

void mesh::set_bone_rot(int bone_idx,const nya_math::quat &rot,bool additive)
{
    if(bone_idx<0 || bone_idx>=m_skeleton.get_bones_count())
        return;

    bone_control &b=m_bone_controls[bone_idx];
    b.rot=rot;
    b.rot_ctrl=additive?bone_additive:bone_override;
}

void mesh::update(unsigned int dt)
{
    if(m_anims.empty())
        return;

    for(int i=0;i<(int)m_anims.size();++i)
    {
        applied_anim &a=m_anims[i];
        anim_set_time(a,a.time+dt*a.anim->m_speed);
        if(a.version!=a.anim->m_version)
        {
            anim_update_mapping(a);
            a.version=a.anim->m_version;
        }

        const float eps=0.0001f;
        a.full_weight=(fabsf(1.0f-a.anim->m_weight)<eps);
    }

    for(int i=0;i<m_skeleton.get_bones_count();++i)
    {
        nya_math::vec3 pos;
        nya_math::quat rot;

        const applied_anim &a=m_anims[0];
        if(i<(int)a.bones_map.size() && a.bones_map[i]>=0)
        {
            const unsigned int time=(unsigned int)a.time+a.anim->m_range_from;
            const nya_render::animation::bone &b=
                a.anim->m_shared->anim.get_bone(a.bones_map[i],time,a.anim->get_loop());

            if(a.full_weight)
            {
                pos=b.pos;
                rot=b.rot;
            }
            else
            {
                pos=b.pos*a.anim->m_weight;
                rot=b.rot;
                rot.apply_weight(a.anim->m_weight);
            }
        }

        for(int j=1;j<(int)m_anims.size();++j)
        {
            const applied_anim &a=m_anims[j];
            if(i<(int)a.bones_map.size() && a.bones_map[i]>=0)
            {
                const unsigned int time=(unsigned int)a.time+a.anim->m_range_from;
                const nya_render::animation::bone &b=
                    a.anim->m_shared->anim.get_bone(a.bones_map[i],time,a.anim->get_loop());

                if(a.full_weight)
                {
                    pos+=b.pos;
                    rot=rot*b.rot;
                }
                else
                {
                    pos+=b.pos*a.anim->m_weight;
                    nya_math::quat tmp=b.rot;
                    tmp.apply_weight(a.anim->m_weight);
                    rot=rot*tmp;
                }
            }
        }

        bone_control_map::const_iterator it=m_bone_controls.find(i);
        if(it!=m_bone_controls.end())
        {
            const bone_control &b=it->second;

            switch(b.pos_ctrl)
            {
                case bone_override: pos=b.pos; break;
                case bone_additive: pos+=b.pos; break;
                case bone_free: break;
            }

            switch(b.rot_ctrl)
            {
                case bone_override: rot=b.rot; break;
                case bone_additive: rot=rot*b.rot; break;
                case bone_free: break;
            }
        }

        m_skeleton.set_bone_transform(i,pos,rot);
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
        m_aabb=m_transform.transform_aabb(m_shared->aabb);
    }

    return m_aabb;
}

}
