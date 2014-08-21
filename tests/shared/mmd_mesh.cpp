//https://code.google.com/p/nya-engine/

#include "mmd_mesh.h"

bool mmd_mesh::load(const char *name)
{
    unload();

    nya_scene::mesh::register_load_function(pmx_loader::load,false);
    nya_scene::mesh::register_load_function(pmd_loader::load,false);

    if(!m_mesh.load(name))
        return false;

    const nya_render::vbo &vbo=m_mesh.internal().get_shared_data()->vbo;
    nya_memory::tmp_buffer_ref buf;
    if(!vbo.get_vertex_data(buf))
        return false;

    m_vertex_data.resize(vbo.get_vert_stride()*vbo.get_verts_count()/4);
    buf.copy_from(&m_vertex_data[0],m_vertex_data.size()*4);
    buf.free();

    if(m_mesh.get_skeleton().get_bones_count()>pmd_loader::gpu_skining_bones_limit)
        m_original_vertex_data=m_vertex_data;

    m_morph_data=pmd_loader::get_additional_data(m_mesh);
    if(!m_morph_data)
    {
        if((m_morph_data=pmx_loader::get_additional_data(m_mesh)))
           m_pos_count=4;
    }
    else
        m_pos_count=2;

    if(m_morph_data)
        m_morphs.resize(m_morph_data->morphs.size());

    m_vbo.set_vertex_data(&m_vertex_data[0],vbo.get_vert_stride(),vbo.get_verts_count());
    m_vbo.set_vertices(vbo.get_vert_offset(),vbo.get_vert_dimension());
    m_vbo.set_normals(vbo.get_normals_offset());
    for(int i=0;i<nya_render::vbo::max_tex_coord;++i)
    {
        if(vbo.get_tc_dimension(i)>0)
            m_vbo.set_tc(i,vbo.get_tc_offset(i),vbo.get_tc_dimension(i));
    }

    update(0);

    return true;
}

void mmd_mesh::unload()
{
    m_mesh.unload();
    m_vertex_data.clear();
    m_original_vertex_data.clear();
    m_morph_data=0;
    m_vbo.release();
    m_morphs.clear();
}

void mmd_mesh::set_anim(const nya_scene::animation &anim,int layer)
{
    m_mesh.set_anim(anim,layer);
    int idx=-1;
    for(int i=0;i<int(m_anims.size());++i)
    {
        if(m_anims[i].layer==layer)
        {
            idx=i;
            break;
        }
    }

    if(idx<0)
    {
        idx=int(m_anims.size());
        m_anims.resize(idx+1);
    }

    m_anims[idx].curves_map.clear();
    m_anims[idx].curves_map.resize(m_morphs.size());
    if(!anim.get_shared_data().is_valid())
        return;

    if(m_morph_data)
    {
        for(int i=0;i<int(m_morphs.size());++i)
            m_anims[idx].curves_map[i]=anim.get_shared_data()->anim.get_curve_idx(m_morph_data->morphs[i].name.c_str());
    }
}

void mmd_mesh::update(unsigned int dt)
{
    m_mesh.update(dt);

    for(int i=0;i<m_anims.size();++i)
    {
        const nya_scene::animation_proxy &ap=get_anim(m_anims[i].layer);
        if(ap.is_valid())
        {
            if(ap->get_shared_data().is_valid())
            {
                const nya_render::animation &a=ap->get_shared_data()->anim;
                if(i==0)
                {
                    for(int j=0;j<m_anims[i].curves_map.size();++j)
                    {
                        if(m_morphs[j].overriden)
                            continue;

                        m_morphs[j].value=a.get_curve(m_anims[i].curves_map[j],m_mesh.get_anim_time(m_anims[i].layer))*ap->get_weight();
                    }
                }
                else
                {
                    for(int j=0;j<m_anims[i].curves_map.size();++j)
                    {
                        if(m_morphs[j].overriden)
                            continue;

                        m_morphs[j].value+=a.get_curve(m_anims[i].curves_map[j],m_mesh.get_anim_time(m_anims[i].layer))*ap->get_weight();
                    }
                }
            }
        }
    }

    //not optimal, just for testing

    int stride=m_vbo.get_vert_stride()/4;
    if(!stride)
        return;

    bool need_update_vbo=false;
    if(!m_original_vertex_data.empty())
    {
        need_update_vbo=true;

        const nya_render::skeleton &sk=m_mesh.get_skeleton();
        const bool is_pmx=m_pos_count>2;
        if(is_pmx)
        {
            const pmx_loader::vert *verts=(const pmx_loader::vert *)&m_original_vertex_data[0];
            for(int i=0;i<m_vbo.get_verts_count();++i)
            {
                nya_math::vec3 pos;
                for(int j=0;j<4;++j)
                {
                    if(verts[i].bone_weight[j]>0.001f)
                    {
                        const int idx=verts[i].bone_idx[j];
                        pos+=(sk.get_bone_pos(idx)+sk.get_bone_rot(idx).rotate(verts[i].pos[j]))*verts[i].bone_weight[j];
                    }
                }

                memcpy(&m_vertex_data[i*stride],&pos,sizeof(pos));
            }
        }
        else
        {
            const pmd_loader::vert *verts=(const pmd_loader::vert *)&m_original_vertex_data[0];
            for(int i=0;i<m_vbo.get_verts_count();++i)
            {
                nya_math::vec3 pos;
                for(int j=0;j<2;++j)
                {
                    const int idx=verts[i].bone_idx[j];
                    pos+=(sk.get_bone_pos(idx)+sk.get_bone_rot(idx).rotate(verts[i].pos[j]))*
                         (j==0?verts[i].bone_weight:(1.0f-verts[i].bone_weight));
                }

                memcpy(&m_vertex_data[i*stride],&pos,sizeof(pos));
            }
        }
    }

    for(int i=0;i<m_morphs.size();++i)
    {
        const float delta=m_morphs[i].value-m_morphs[i].last_value;
        if(fabsf(delta)<0.02f)
            continue;

        if(m_morph_data)
        {
            const pmd_morph_data::morph &m=m_morph_data->morphs[i];
            for(int j=0;j<int(m.verts.size());++j)
            {
                const unsigned int base=m.verts[j].idx*stride;
                for(int k=0;k<m_pos_count*3;k+=3)
                {
                    m_vertex_data[base+k]+=m.verts[j].pos.x*delta;
                    m_vertex_data[base+k+1]+=m.verts[j].pos.y*delta;
                    m_vertex_data[base+k+2]+=m.verts[j].pos.z*delta;
                }
            }
        }

        need_update_vbo=true;
        m_morphs[i].last_value=m_original_vertex_data.empty()?m_morphs[i].value:0.0f;
    }

    if(need_update_vbo)
        m_vbo.set_vertex_data(&m_vertex_data[0],m_vbo.get_vert_stride(),m_vbo.get_verts_count());
}

void mmd_mesh::draw()
{
    if(!m_mesh.internal().get_shared_data().is_valid())
        return;
    /*
    if(m_mesh.internal().m_has_aabb) //ToDo
    {
        if(get_camera().is_valid() && !get_camera()->get_frustum().test_intersect(get_aabb()))
            return;
    }
    */
    nya_scene::transform::set(m_mesh.internal().get_transform());
    nya_scene::shader_internal::set_skeleton(&m_mesh.get_skeleton());

    const nya_scene::shared_mesh &sh=*m_mesh.internal().get_shared_data().operator ->();
    sh.vbo.bind_indices();
    m_vbo.bind_verts();
    for(int i=0;i<int(sh.groups.size());++i)
    {
        const nya_scene::material &m=sh.materials[sh.groups[i].material_idx];
        m.internal().set(nya_scene::material::default_pass);
        m_vbo.draw(sh.groups[i].offset,sh.groups[i].count);
        m.internal().unset();
    }
    m_vbo.unbind();
    sh.vbo.unbind();
    nya_scene::shader_internal::set_skeleton(0);
}
