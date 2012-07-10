//https://code.google.com/p/nya-engine/

#include "tmb_model.h"
#include "tsb_anim.h"
#include "memory/tmp_buffer.h"

#include "string.h"
#include <vector>

namespace
{
    struct vertex
    {
        float pos[3];
        float nor[3];
        float tc[2];          //could be stored
        float bone_idx[3];    //in two 4d
        float bone_weight[3]; //vectors
    };
}

bool tmb_model::load(nya_resources::resource_data *model_res)
{
    if(!model_res)
    {
        nya_log::get_log()<<"Unable to load model: invalid data\n";
        return false;
    }

    nya_memory::tmp_buffer_scoped model_data(model_res->get_size());
    model_res->read_all(model_data.get_data());
    model_res->release();

    const char *magic=(const char*)model_data.get_data();
    if(strncmp(magic,"TMB0",4)!=0)
    {
        nya_log::get_log()<<"Warning: invalid TMB0 magic\n";
        //return false;
    }

    typedef unsigned int uint;
    typedef unsigned short ushort;
    typedef unsigned char uchar;

    uint offset=4;

    uint tex_count=*(uint*)model_data.get_data(offset);
    offset+=4;

    m_textures.resize(tex_count);
    for(int i=0;i<tex_count;++i)
    {
        struct tmb_tex_header
        {
            char name[32];
            ushort width;
            ushort height;
        };

        tmb_tex_header *header=(tmb_tex_header*)model_data.get_data(offset);
        offset+=sizeof(tmb_tex_header);
        m_textures[i].build_texture(model_data.get_data(offset),header->width,
                                    header->height,nya_render::texture::color_bgra);

        offset+=header->width*header->height*4;
    }

    uint tmb_mat_count=*(uint*)model_data.get_data(offset);
    offset+=4;

    struct tmb_material
    {
        float ambient[4];
        float diffuse[4];
        float specular[4];
        float emission[4];
        float shininess;
        uint tex_idx;
    };

    tmb_material *tmb_materials=(tmb_material*)model_data.get_data(offset);
    offset+=tmb_mat_count*sizeof(tmb_material);

    uint group_count=*(uint*)model_data.get_data(offset);
    offset+=4;

    static std::vector<vertex> vertices;

    vertices.clear();
    m_materials.clear();
    m_vbo.release();

    uint verts_offset=0;
    for(int i=0;i<group_count;++i)
    {
        struct tmb_group_header
        {
            char name[32];
            float matrix[4][4];
            uint faces_count;
            uint unknown;
            uint mat_bind_count;
        };

        tmb_group_header *header=(tmb_group_header*)model_data.get_data(offset);
        offset+=sizeof(tmb_group_header);

        uint mat_count=header->mat_bind_count;
        uint mat_offset=(uint)m_materials.size();
        m_materials.resize(mat_offset+mat_count);

        uint verts_count=header->faces_count*3;
        vertices.resize(verts_offset+verts_count);

        struct tmb_mat_bind
        {
            uint mat_idx;
            uint vert_offset;
            uint vert_count;
        };

        tmb_mat_bind *tmb_mat_binds=(tmb_mat_bind*)model_data.get_data(offset);
        offset+=header->mat_bind_count*sizeof(tmb_mat_bind);
        int mat_idx=0;
        for(int i=0;i<header->mat_bind_count;++i)
        {
            material &to=m_materials[mat_idx+mat_offset];
            tmb_mat_bind &from_bind=tmb_mat_binds[i];
            if(from_bind.mat_idx>=tmb_mat_count)
                continue;

            tmb_material &from_mat=tmb_materials[from_bind.mat_idx];

            to.vert_offset=from_bind.vert_offset+verts_offset;
            to.vert_count=from_bind.vert_count*3;
            to.tex_idx=from_mat.tex_idx;
            ++mat_idx;
        }

        m_materials.resize(mat_idx+mat_offset);

        struct tmb_vertex
        {
            float pos[3];
            float weights[3];
            uchar bone_ids[4];
            float normal[3];
            uchar color[4];
            float tc[2];
        };

        tmb_vertex *tmb_vertices=(tmb_vertex*)model_data.get_data(offset);
        offset+=verts_count*sizeof(tmb_vertex);

        for(int i=0;i<verts_count;++i)
        {
            tmb_vertex &from=tmb_vertices[i];
            vertex &to=vertices[i+verts_offset];

            for(int k=0;k<3;++k)
            {
                to.pos[k]=header->matrix[0][k]*from.pos[0]
                         +header->matrix[1][k]*from.pos[1]
                         +header->matrix[2][k]*from.pos[2]
                         +header->matrix[3][k];
                
                to.nor[k]=from.normal[k];
                to.bone_idx[k]=(float)from.bone_ids[k];
                to.bone_weight[k]=from.weights[k];
                if(from.weights[k]<0.01f)
                {
                    to.bone_idx[k]=0;
                    to.bone_weight[k]=0;
                }
            }

            to.tc[0]=from.tc[0];
            to.tc[1]=from.tc[1];
        }
        
        verts_offset+=verts_count;
    }

    m_bones_count=*(uint*)model_data.get_data(offset);
    offset+=4;

    struct tmb_bone
    {
        float mat[16];
    };

    m_bones.resize(m_bones_count);
    model_data.copy_from(&m_bones[0],m_bones_count*sizeof(tmb_bone),offset);

    m_vbo.gen_vertex_data(&vertices[0],sizeof(vertex),verts_offset);
    m_vbo.set_normals(3*sizeof(float));
    m_vbo.set_tc(0,6*sizeof(float));
    m_vbo.set_tc(1,8*sizeof(float));
    m_vbo.set_tc(2,11*sizeof(float));

    return true;
}

void tmb_model::draw(bool use_materials)
{
    m_vbo.bind();
    if(!use_materials)
    {
        m_vbo.draw();
        m_vbo.unbind();
        return;
    }

    for(unsigned int i=0;i<m_materials.size();++i)
    {
        material & mat = m_materials[i];
        m_textures[mat.tex_idx].bind();

        //glColor4fv(mat.color);
        m_vbo.draw(mat.vert_offset,mat.vert_count);
    }
    m_vbo.unbind();
}

void tmb_model::release()
{
    m_vbo.release();
    for(unsigned int i=0;i<m_textures.size();++i)
        m_textures[i].release();

    m_textures.clear();
    m_materials.clear();
}

void tmb_model::apply_anim(tsb_anim *anim)
{
    m_frames_count=0;
    m_anim_bones.clear();

    if(!anim)
        return;

    m_frames_count = anim->get_frames_count();
    if(!m_frames_count)
    {
        nya_log::get_log()<<"Unable to set empty animation\n";
        return;
    }

    m_anim_bones.resize(m_frames_count*m_bones_count);

    for(unsigned int i=0;i<m_frames_count;++i)
    {
        tsb_anim::bone *anim_bones=anim->get_bones(i);
        tmb_model::bone *final_bones=&m_anim_bones[i*m_bones_count];

        for(int k=0;k<m_bones_count;++k)
        {
            tsb_anim::bone &a=anim_bones[k];
            tmb_model::bone &b=m_bones[k];
            tmb_model::bone &f=final_bones[k];

            for(int x=0;x<4;++x)
                for(int y=0;y<4;++y)
                    f.mat[y][x]=a.mat[0][x]*b.mat[y][0]+
                                a.mat[1][x]*b.mat[y][1]+
                                a.mat[2][x]*b.mat[y][2]+
                                a.mat[3][x]*b.mat[y][3];
        }
    }
}

bool shared_models_manager::fill_resource(const char *name,tmb_model &res)
{
    if(!name)
    {
        nya_resources::get_log()<<"Unable to access model: invalid name\n";
        return false;
    }
    
    nya_resources::resource_data *data = nya_resources::get_resources_provider().access(name);
    return res.load(data);
}

bool shared_models_manager::release_resource(tmb_model &res)
{
    res.release();
    return true;
}

shared_models_manager &get_shared_models()
{
    static shared_models_manager manager;
    return manager;
}
