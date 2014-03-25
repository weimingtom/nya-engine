    //https://code.google.com/p/nya-engine/

#include "mesh.h"
#include "scene.h"
#include "camera.h"
#include "shader.h"
#include "render/render.h"
#include "memory/tmp_buffer.h"
#include "memory/memory_reader.h"
#include "math/constants.h"

namespace nya_scene
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

bool mesh::load_nms_mesh_section(shared_mesh &res,const void *data,size_t size,int version)
{
    if(!data || !size)
        return false;

    nya_memory::memory_reader reader(data,size);

    typedef unsigned int uint;
    typedef unsigned short ushort;
    typedef unsigned char uchar;

    const nya_math::vec3 aabb_min=reader.read<nya_math::vec3>();
    const nya_math::vec3 aabb_max=reader.read<nya_math::vec3>();
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
        const uchar data_type=version>1?reader.read<uchar>():nya_render::vbo::float32;

        read_string(reader); //semantics

        switch(type) //ToDo: data_type for all types
        {
            case pos: res.vbo.set_vertices(vertex_stride,dimension); break;
            case normal: res.vbo.set_normals(vertex_stride,nya_render::vbo::vertex_atrib_type(data_type)); break;
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
    if(!reader.skip(verts_count*vertex_stride))
        return false;

    const uchar index_size=reader.read<uchar>();
    if(index_size)
    {
        uint inds_count=reader.read<uint>();
        if(!reader.check_remained(inds_count*index_size))
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

            const nya_math::vec3 aabb_min=reader.read<nya_math::vec3>();
            const nya_math::vec3 aabb_max=reader.read<nya_math::vec3>();
            g.aabb.delta=(aabb_max-aabb_min)*0.5f;
            g.aabb.origin=aabb_min+g.aabb.delta;

            g.material_idx=reader.read<ushort>();
            g.offset=reader.read<uint>();
            g.count=reader.read<uint>();
            g.elem_type=version>1?nya_render::vbo::element_type(reader.read<uchar>()):nya_render::vbo::triangles;
        }

        break; //ToDo: load all lods
    }

    return true;
}

bool mesh::load_nms_skeleton_section(shared_mesh &res,const void *data,size_t size)
{
    if(!data || !size)
        return false;

    nya_memory::memory_reader reader(data,size);

    const int bones_count=reader.read<int>();
    for(int i=0;i<bones_count;++i)
    {
        const std::string name=read_string(reader);
        const nya_math::quat rot=reader.read<nya_math::quat>(); //ToDo ?
        const nya_math::vec3 pos=reader.read<nya_math::vec3>();
        const int parent=reader.read<int>();

        res.skeleton.add_bone(name.c_str(),pos,parent);
    }

    return true;
}

nya_render::blend::mode blend_mode_from_string(const std::string &s)
{
    if(s=="src_alpha") return nya_render::blend::src_alpha;
    if(s=="inv_src_alpha") return nya_render::blend::inv_src_alpha;
    if(s=="src_color") return nya_render::blend::src_color;
    if(s=="inv_src_color") return nya_render::blend::inv_src_color;
    if(s=="dst_color") return nya_render::blend::dst_color;
    if(s=="inv_dst_color") return nya_render::blend::inv_dst_color;
    if(s=="dst_alpha") return nya_render::blend::dst_alpha;
    if(s=="inv_dst_alpha") return nya_render::blend::inv_dst_alpha;
    if(s=="zero") return nya_render::blend::zero;
    if(s=="one") return nya_render::blend::one;

    return nya_render::blend::one;
}

bool mesh::load_nms_material_section(shared_mesh &res,const void *data,size_t size)
{
    if(!data || !size)
        return false;

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
            {
                size_t div=value.find(':');
                if(div!=std::string::npos)
                {
                    std::string src=value;
                    src.resize(div);
                    std::string dst=value.substr(div+1);
                    m.set_blend(true,blend_mode_from_string(src),blend_mode_from_string(dst));
                }
            }
            else if(name=="nya_cull")
            {
                if(value=="ccw")
                    m.set_cull_face(true,nya_render::cull_face::ccw);
                else if(value=="cw")
                    m.set_cull_face(true,nya_render::cull_face::cw);
            }
            else if(name=="nya_zwrite")
                m.set_zwrite(value=="true"||value=="1");
        }

        const ushort vec4_count=reader.read<ushort>();
        for(ushort j=0;j<vec4_count;++j)
        {
            const std::string name=read_string(reader);
            const float r=reader.read<float>();
            const float g=reader.read<float>();
            const float b=reader.read<float>();
            const float a=reader.read<float>();
            m.set_param( m.get_param_idx(name.c_str()),r,g,b,a);
        }

        const ushort ints_count=reader.read<ushort>();
        for(ushort j=0;j<ints_count;++j)
        {
            read_string(reader); //ToDo
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

    uint version=reader.read<uint>();
    if(version!=1 && version!=2)
    {
        nya_log::get_log()<<"nms load error: unsupported version: "<<version<<"\n";
        return false;
    }

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
            case mesh_data: load_nms_mesh_section(res,reader.get_data(),size,version); break;
            case skeleton: load_nms_skeleton_section(res,reader.get_data(),size); break;
            case materials: load_nms_material_section(res,reader.get_data(),size); break;
            default: nya_log::get_log()<<"nms load warning: unknown chunk type\n";
        };

        reader.skip(size);
    }

    data.free();

    return true;
}

bool mesh_internal::init_form_shared()
{
    if(!m_shared.is_valid())
        return false;
    
    for(size_t i=0;i<m_replaced_materials.size();++i)
        m_replaced_materials[i].release();

    m_replaced_materials.clear();
    m_replaced_materials_idx.clear();
    m_anims.clear();
    
    m_skeleton=m_shared->skeleton;
    m_bone_controls.clear();

    for(int i=0;i<int(m_shared->materials.size());++i)
        m_shared->materials[i].internal().skeleton_changed(&m_skeleton);

    m_recalc_aabb=true;
    m_has_aabb=m_shared->aabb.delta*m_shared->aabb.delta>0.0001;

    return true;
}

bool mesh::load(const char *name)
{
    if(!m_internal.load(name))
        return false;

    return m_internal.init_form_shared();
}

void mesh::create(const shared_mesh &res)
{
    m_internal.create(res);
    m_internal.init_form_shared();
}

void mesh::unload()
{
    m_internal.unload();

    for(size_t i=0;i<internal().m_replaced_materials.size();++i)
        m_internal.m_replaced_materials[i].release();

    m_internal.m_replaced_materials.clear();
    m_internal.m_replaced_materials_idx.clear();
    m_internal.m_anims.clear();
    m_internal.m_skeleton=nya_render::skeleton();
    m_internal.m_aabb=nya_math::aabb();
}

const material &mesh_internal::mat(int idx) const
{
    const int rep_idx=(m_replaced_materials_idx.empty()?
                       -1:m_replaced_materials_idx[idx]);
    if(rep_idx>=0)
        return m_replaced_materials[rep_idx];

    return m_shared->materials[idx];
}

void mesh_internal::draw_group(int idx) const
{
    const shared_mesh::group &g=m_shared->groups[idx];
    if(g.material_idx>=m_shared->materials.size())
        return;

    //ToDo: check aabb for groups
    //if(g.aabb.delta*g.aabb.delta>0.0001)
    //    if(get_camera().is_valid() && !get_camera()->get_frustum().test_intersect(g.aabb))
    //        return;

    const material &m=mat(g.material_idx);
    m.internal().set();
    m_shared->vbo.draw(g.offset,g.count,g.elem_type);
    m.internal().unset();
}

void mesh::draw(int idx) const
{
    if(!internal().m_shared.is_valid() || idx>=int(internal().m_shared->groups.size()))
        return;

    if(internal().m_has_aabb)
    {
        if(get_camera().is_valid() && !get_camera()->get_frustum().test_intersect(get_aabb()))
            return;
    }

    transform::set(internal().m_transform);
    shader_internal::set_skeleton(&internal().m_skeleton);

    internal().m_shared->vbo.bind();

    if(idx>=0)
    {
        if(idx<int(internal().m_shared->groups.size()))
            internal().draw_group(idx);
    }
    else
    {
        for(int i=0;i<int(internal().m_shared->groups.size());++i)
            internal().draw_group(i);
    }

    internal().m_shared->vbo.unbind();

    shader_internal::set_skeleton(0);
}

int mesh::get_groups_count() const
{
    if(!internal().m_shared.is_valid())
        return 0;

    return int(internal().m_shared->groups.size());
}

const char *mesh::get_group_name(int group_idx) const
{
    if(!internal().m_shared.is_valid())
        return 0;

    if(group_idx<0 || group_idx>=int(internal().m_shared->groups.size()))
        return 0;

    return internal().m_shared->groups[group_idx].name.c_str();
}

int mesh::get_material_idx(int group_idx) const
{
    if(!internal().m_shared.is_valid())
        return 0;

    if(group_idx<0 || group_idx>=int(internal().m_shared->groups.size()))
        return 0;

    return internal().m_shared->groups[group_idx].material_idx;
}

int mesh::get_materials_count() const
{
    if(!internal().m_shared.is_valid())
        return 0;

    return int(internal().m_shared->materials.size());
}

const material &mesh::get_material(int idx) const
{
    if(!internal().m_shared.is_valid() || idx<0 || idx>=int(internal().m_shared->materials.size()))
    {
        const static material invalid;
        return invalid;
    }

    return internal().mat(idx);
}

material &mesh::modify_material(int idx)
{
    if(!internal().m_shared.is_valid() || idx<0 || idx>=int(internal().m_shared->materials.size()))
    {
        static material invalid;
        invalid=material();
        return invalid;
    }

    if(internal().m_replaced_materials.empty())
        m_internal.m_replaced_materials_idx.resize(internal().m_shared->materials.size(),-1);

    int &replace_idx=m_internal.m_replaced_materials_idx[idx];
    if(replace_idx<0)
    {
        replace_idx=int(internal().m_replaced_materials.size());
        m_internal.m_replaced_materials.resize(internal().m_replaced_materials.size()+1);
        m_internal.m_replaced_materials.back()=internal().m_shared->materials[idx];
    }

    return m_internal.m_replaced_materials[replace_idx];
}

void mesh::set_material(int idx,const material &mat)
{
    if(!internal().m_shared.is_valid() || idx<0 || idx>=int(internal().m_shared->materials.size()))
        return;

    if(internal().m_replaced_materials_idx.empty())
        m_internal.m_replaced_materials_idx.resize(internal().m_shared->materials.size(),-1);

    if(internal().m_replaced_materials_idx[idx]>=0)
    {
        m_internal.m_replaced_materials[internal().m_replaced_materials_idx[idx]]=mat;
        return;
    }

    m_internal.m_replaced_materials_idx[idx]=int(internal().m_replaced_materials.size());
    m_internal.m_replaced_materials.resize(internal().m_replaced_materials.size()+1);
    m_internal.m_replaced_materials.back()=mat;
}

const nya_render::skeleton &mesh::get_skeleton() const
{
    return internal().m_skeleton;
}

void mesh::set_anim(const animation_proxy & anim,int layer)
{
    if(!internal().m_shared.is_valid() || layer<0)
        return;

    int idx=-1;
    for(int i=0;i<int(internal().m_anims.size());++i)
    {
        if(m_internal.m_anims[i].layer==layer)
        {
            idx=i;
            break;
        }
    }

    if(!anim.is_valid())
    {
        if(idx>=0)
            m_internal.m_anims.erase(m_internal.m_anims.begin()+idx);

        return;
    }

    if(idx<0)
    {
        idx=int(internal().m_anims.size());
        m_internal.m_anims.resize(internal().m_anims.size()+1);
    }

    mesh_internal::applied_anim &a=m_internal.m_anims[idx];
    a.layer=layer;
    a.anim=anim;
    a.time=0;
    a.version=0;
    a.bones_map.clear();
}

void mesh_internal::anim_set_time(applied_anim &a,float t)
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

void mesh_internal::anim_update_mapping(applied_anim &a)
{
    a.bones_map.clear();

    if(!a.anim.is_valid() || !a.anim->m_shared.is_valid())
        return;

    const nya_render::animation &ra=a.anim->m_shared->anim;
    a.bones_map.resize(m_skeleton.get_bones_count(),-1);

    if(a.anim->m_mask.is_valid())
    {
        for(int j=0;j<int(m_skeleton.get_bones_count());++j)
        {
            const char *bone_name=m_skeleton.get_bone_name(j);
            if(a.anim->m_mask->data.find(bone_name)!=a.anim->m_mask->data.end())
                a.bones_map[j]=ra.get_bone_idx(bone_name);
        }
    }
    else
    {
        for(int j=0;j<int(m_skeleton.get_bones_count());++j)
            a.bones_map[j]=ra.get_bone_idx(m_skeleton.get_bone_name(j));
    }
}

const animation_proxy & mesh::get_anim(int layer) const
{
    for(int i=0;i<int(internal().m_anims.size());++i)
    {
        if(internal().m_anims[i].layer==layer)
            return internal().m_anims[i].anim;
    }

    static const animation_proxy invalid;
    return invalid;
}

void mesh::set_anim_time(unsigned int time,int layer)
{
    for(int i=0;i<int(internal().m_anims.size());++i)
    {
        if(internal().m_anims[i].layer==layer)
        {
            m_internal.anim_set_time(m_internal.m_anims[i],float(time));
            return;
        }
    }
}

unsigned int mesh::get_anim_time(int layer) const
{
    for(int i=0;i<int(internal().m_anims.size());++i)
    {
        if(internal().m_anims[i].layer==layer)
            return (unsigned int)(internal().m_anims[i].time);
    }

    return 0;
}

bool mesh_internal::is_anim_finished(int layer) const
{
    for(int i=0;i<int(m_anims.size());++i)
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

bool mesh::is_anim_finished(int layer) const { return internal().is_anim_finished(layer); }

nya_math::vec3 mesh::get_bone_pos(int bone_idx,bool local,bool ignore_animations)
{
    const nya_math::vec3 pos=ignore_animations?internal().m_skeleton.get_bone_original_pos(bone_idx):
                                               internal().m_skeleton.get_bone_pos(bone_idx);
    if(local)
        return pos;

    return internal().get_transform().transform_vec( pos );
}

nya_math::quat mesh::get_bone_rot(int bone_idx,bool local)
{
    const nya_math::quat rot=internal().m_skeleton.get_bone_rot(bone_idx);
    if(local)
        return rot;

    return internal().get_transform().transform_quat( rot );
}

void mesh::set_bone_pos(int bone_idx,const nya_math::vec3 &pos,bool additive)
{
    if(bone_idx<0 || bone_idx>=internal().m_skeleton.get_bones_count())
        return;

    mesh_internal::bone_control &b=m_internal.m_bone_controls[bone_idx];
    b.pos=pos;
    b.pos_ctrl=additive?mesh_internal::bone_additive:mesh_internal::bone_override;
}

void mesh::set_bone_rot(int bone_idx,const nya_math::quat &rot,bool additive)
{
    if(bone_idx<0 || bone_idx>=internal().m_skeleton.get_bones_count())
        return;

    mesh_internal::bone_control &b=m_internal.m_bone_controls[bone_idx];
    b.rot=rot;
    b.rot_ctrl=additive?mesh_internal::bone_additive:mesh_internal::bone_override;
}

void mesh::update(unsigned int dt) { m_internal.update(dt); }

void mesh_internal::update(unsigned int dt)
{
    if(!m_shared.is_valid())
        return;

    if(m_anims.empty() && m_bone_controls.empty())
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

        for(int j=0;j<(int)m_anims.size();++j)
        {
            const applied_anim &a=m_anims[j];
            if(i>=(int)a.bones_map.size() || a.bones_map[i]<0)
                continue;

            const unsigned int time=(unsigned int)a.time+a.anim->m_range_from;
            nya_math::vec3 bone_pos=a.anim->m_shared->anim.get_bone_pos(a.bones_map[i],time,a.anim->get_loop());
            nya_math::quat bone_rot=a.anim->m_shared->anim.get_bone_rot(a.bones_map[i],time,a.anim->get_loop());
            if(!a.full_weight)
                bone_pos*=a.anim->m_weight,bone_rot.apply_weight(a.anim->m_weight);

            if(j==0)
                pos=bone_pos,rot=bone_rot;
            else
                pos+=bone_pos,rot=rot*bone_rot;
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

    for(int i=0;i<int(m_shared->materials.size());++i)
        mat(i).internal().skeleton_changed(&m_skeleton);
}

const nya_math::aabb &mesh::get_aabb() const
{
    if(!internal().m_shared.is_valid())
    {
        static nya_math::aabb invalid;
        return invalid;
    }

    if(internal().m_recalc_aabb)
    {
        m_internal.m_recalc_aabb=false;
        m_internal.m_aabb=internal().m_transform.transform_aabb(internal().m_shared->aabb);
    }

    return internal().m_aabb;
}

}
