//https://code.google.com/p/nya-engine/

#include "load_pmd.h"
#include "scene/mesh.h"
#include "memory/memory_reader.h"
#include "string_encoding.h"

namespace
{

struct add_data: public pmd_loader::additional_data, nya_scene::shared_mesh::additional_data
{
    const char *type() { return "pmd"; }
};

}

bool pmd_loader::load(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name)
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

    std::vector<vert> vertices(vert_count);
    for(size_t i=0;i<vertices.size();++i)
    {
        vertices[i].pos.x=reader.read<float>();
        vertices[i].pos.y=reader.read<float>();
        vertices[i].pos.z=-reader.read<float>();

        vertices[i].normal[0]=reader.read<float>();
        vertices[i].normal[1]=reader.read<float>();
        vertices[i].normal[2]=reader.read<float>();

        vertices[i].tc[0]=reader.read<float>();
        vertices[i].tc[1]=1.0f-reader.read<float>();

        vertices[i].bone_idx[0]=reader.read<ushort>();
        vertices[i].bone_idx[1]=reader.read<ushort>();
        vertices[i].bone_weight=reader.read<uchar>()/100.0f;

        reader.skip(1);
    }
    
    const uint ind_count=reader.read<uint>();
    const size_t inds_size=sizeof(ushort)*ind_count;
    if(!reader.check_remained(inds_size))
        return false;

    res.vbo.set_index_data(data.get_data(reader.get_offset()),nya_render::vbo::index2b,ind_count);
    reader.skip(inds_size);

    const uint mat_count=reader.read<uint>();
    if(!reader.check_remained(mat_count*(sizeof(pmd_material_params)+2+sizeof(uint)+20)))
        return false;

    res.groups.resize(mat_count);
    res.materials.resize(mat_count);

    nya_scene::shader sh;
    nya_scene::shared_shader sh_;
    sh_.shdr.set_sampler("base",0);
    sh_.samplers_count=1;
    sh_.samplers["diffuse"]=0;

    const char *vertex_code="uniform vec3 bones_pos[255]; uniform vec4 bones_rot[255];"
    "vec3 tr(vec3 pos,int idx) { vec4 q=bones_rot[idx];"
    "return bones_pos[idx]+pos+cross(q.xyz,cross(q.xyz,pos)+pos*q.w)*2.0; }"
    "varying vec2 tc;"
    "void main()"
    "{  int bone0=int(gl_MultiTexCoord1.x); int bone1=int(gl_MultiTexCoord1.y);"
    "vec3 pos=mix(tr(gl_MultiTexCoord2.xyz,bone1),tr(gl_Vertex.xyz,bone0),gl_MultiTexCoord1.z);"
    "tc=gl_MultiTexCoord0.xy; gl_Position=gl_ModelViewProjectionMatrix*vec4(pos,1.0); }";

    const char *pixel_code="varying vec2 tc; uniform sampler2D base; void main() { vec4 tm=texture2D(base,tc.xy);"
              "if(tm.a<0.001) discard;\n"
              " gl_FragColor=tm; }";

    sh_.shdr.add_program(nya_render::shader::vertex,vertex_code);
    sh_.shdr.add_program(nya_render::shader::pixel,pixel_code);
    sh_.predefines.resize(2);
    sh_.predefines[0].type=nya_scene::shared_shader::bones_pos;
    sh_.predefines[0].location=sh_.shdr.get_handler("bones_pos");
    sh_.predefines[1].type=nya_scene::shared_shader::bones_rot;
    sh_.predefines[1].location=sh_.shdr.get_handler("bones_rot");
    sh.create(sh_);
    
    nya_scene::shared_shader she_;
    nya_scene::shader she;
    
    const char *vertex_code2="uniform vec3 bones_pos[255]; uniform vec4 bones_rot[255];"
    "vec3 tr(vec3 pos,int idx) { vec4 q=bones_rot[idx];"
    "return bones_pos[idx]+pos+cross(q.xyz,cross(q.xyz,pos)+pos*q.w)*2.0; }"
    "void main()"
    "{  int bone0=int(gl_MultiTexCoord1.x); int bone1=int(gl_MultiTexCoord1.y);"
    "vec3 pos=mix(tr(gl_MultiTexCoord2.xyz,bone1),tr(gl_Vertex.xyz,bone0),gl_MultiTexCoord1.z);"
    "pos.xyz+=gl_Normal*0.01;"
    "gl_Position=gl_ModelViewProjectionMatrix*vec4(pos,1.0); }";
    const char *pixel_code2="void main() { gl_FragColor=vec4(0.0,0.0,0.0,1.0); }\n";
    
    she_.shdr.add_program(nya_render::shader::vertex,vertex_code2);
    she_.shdr.add_program(nya_render::shader::pixel,pixel_code2);
    she_.predefines.resize(2);
    she_.predefines[0].type=nya_scene::shared_shader::bones_pos;
    she_.predefines[0].location=she_.shdr.get_handler("bones_pos");
    she_.predefines[1].type=nya_scene::shared_shader::bones_rot;
    she_.predefines[1].location=she_.shdr.get_handler("bones_rot");
    she.create(she_);
    
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
        nya_scene::shared_mesh::group &g=res.groups[i];

        //const mmd_material_params params=
        reader.read<pmd_material_params>();

        reader.read<char>();//toon idx
        const char edge_flag=reader.read<char>();//edge flag

        g.offset=ind_offset;
        g.count=reader.read<uint>();
        g.material_idx=i;
        ind_offset+=g.count;

        const std::string tex_name((const char*)data.get_data(reader.get_offset()),20);
        reader.skip(20);

        std::string base_tex=tex_name;
        size_t pos=base_tex.find('*');
        if(pos!=std::string::npos)
            base_tex.resize(pos);

        nya_scene::texture tex;
        if(!base_tex.empty() && base_tex[0])
            tex.load((path+base_tex).c_str());

        nya_scene::material &m = res.materials[i];
        m.set_texture("diffuse",tex);

        nya_scene::material::pass &p=m.get_pass(m.add_pass(nya_scene::material::default_pass));

        p.set_shader(sh);
        p.get_state().set_blend(true,nya_render::blend::src_alpha,nya_render::blend::inv_src_alpha);
        p.get_state().set_cull_face(true,nya_render::cull_face::cw);

        if(!edge_flag)
            continue;

        res.groups.resize(res.groups.size()+1);
        nya_scene::shared_mesh::group &ge=res.groups.back();
        ge=res.groups[i];
        ge.material_idx=int(res.materials.size());
        res.materials.resize(res.materials.size()+1);
        nya_scene::material &me = res.materials.back();

        nya_scene::material::pass &pe=me.get_pass(me.add_pass(nya_scene::material::default_pass));
        pe.set_shader(she);
        //pe.get_state().set_blend(true,nya_render::blend::src_alpha,nya_render::blend::inv_src_alpha);
        pe.get_state().set_cull_face(true,nya_render::cull_face::ccw);
    }

    const ushort bones_count=reader.read<ushort>();
    if(!reader.check_remained(bones_count*(20+sizeof(short)+5+sizeof(nya_math::vec3))))
        return false;

    for(ushort i=0;i<bones_count;++i)
    {
        std::string name=utf8_from_shiftjis(data.get_data(reader.get_offset()),20);
        reader.skip(20);
        const short parent=reader.read<short>();
        reader.skip(5); //child,kind,ik target

        nya_math::vec3 pos;
        pos.x=reader.read<float>();
        pos.y=reader.read<float>();
        pos.z=-reader.read<float>();

        if(res.skeleton.add_bone(name.c_str(),pos,nya_math::quat(),parent,true)!=i)
        {
            nya_log::log()<<"pmd load error: invalid bone\n";
            return false;
        }
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
            const char *name=res.skeleton.get_bone_name(link);
            if(!name)
            {
                nya_log::log()<<"pmd load warning: invalid ik link\n";
                continue;
            }

            if(strcmp(name,"\xe5\xb7\xa6\xe3\x81\xb2\xe3\x81\x96")==0 || strcmp(name,"\xe5\x8f\xb3\xe3\x81\xb2\xe3\x81\x96")==0)
                res.skeleton.add_ik_link(ik,link,0.001f,nya_math::constants::pi);
            else
                res.skeleton.add_ik_link(ik,link);
        }
    }



    add_data *ad=new add_data;
    res.add_data=ad;

    const ushort morphs_count=reader.read<ushort>();
    if(morphs_count>1)
        ad->morphs.resize(morphs_count-1);

    pmd_morph_data::morph base_morph;
    for(ushort i=0;i<ushort(ad->morphs.size());)
    {
        const std::string name=utf8_from_shiftjis(data.get_data(reader.get_offset()),20);
        reader.skip(20);
        const uint size=reader.read<uint>();
        const uchar type=reader.read<uchar>();

        pmd_morph_data::morph &m=type?ad->morphs[i]:base_morph;
        if(type)
            ++i;

        //printf("name %s %d\n",name.c_str(),type);

        m.name=name;
        m.verts.resize(size);
        for(uint j=0;j<size;++j)
        {
            pmd_morph_data::morph_vertex &v=m.verts[j];

            const uint idx=reader.read<uint>();
            v.idx=type?base_morph.verts[idx].idx:idx;
            v.pos.x=reader.read<float>();
            v.pos.y=reader.read<float>();
            v.pos.z=-reader.read<float>();
        }
    }
/*
    for(int i=0;i<int(base_morph.verts.size());++i)
    {
        const pmd_morph_data::morph_vertex &b=base_morph.verts[i];
        vertices[b.idx].pos=b.pos;
    }
*/
    for(size_t i=0;i<vertices.size();++i)
    {
        vertices[i].pos2=vertices[i].pos-res.skeleton.get_bone_pos(int(vertices[i].bone_idx[1]));
        vertices[i].pos-=res.skeleton.get_bone_pos(int(vertices[i].bone_idx[0]));
    }

    res.vbo.set_vertex_data(&vertices[0],sizeof(vertices[0]),vert_count);
    res.vbo.set_tc(2,3*sizeof(float),3); //pos2
    res.vbo.set_normals(6*sizeof(float));
    res.vbo.set_tc(0,9*sizeof(float),2);
    res.vbo.set_tc(1,11*sizeof(float),3); //skin info

    vertices.clear();

    reader.skip(reader.read<uchar>()*sizeof(ushort));

    const uchar bone_groups_count=reader.read<uchar>();
    reader.skip(bone_groups_count*50);

    reader.skip(reader.read<uint>()*(sizeof(ushort)+sizeof(uchar)));

    if(!reader.get_remained())
        return true;

    if(reader.read<uchar>())
    {
        reader.skip(20);
        reader.skip(256);
        reader.skip(bones_count*20);

        if(morphs_count>1)
            reader.skip((morphs_count-1)*20);

        reader.skip(bone_groups_count*50);
    }

    reader.skip(10*100);//toon

    const uint rigid_bodies_count=reader.read<uint>();
    ad->rigid_bodies.resize(rigid_bodies_count);
    for(uint i=0;i<rigid_bodies_count;++i)
    {
        pmd_phys_data::rigid_body &rb=ad->rigid_bodies[i];
        rb.name=utf8_from_shiftjis(data.get_data(reader.get_offset()),20);
        reader.skip(20);
        rb.bone=reader.read<short>();
        rb.collision_group=reader.read<uchar>();
        rb.collision_mask=reader.read<ushort>();
        rb.type=reader.read<uchar>();
        rb.size=reader.read<nya_math::vec3>();
        rb.location=reader.read<nya_math::vec3>();
        rb.rotation=reader.read<nya_math::vec3>();
        rb.mass=reader.read<float>();
        rb.vel_attenuation=reader.read<float>();
        rb.rot_attenuation=reader.read<float>();
        rb.bounce=reader.read<float>();
        rb.friction=reader.read<float>();
        rb.mode=reader.read<uchar>();
    }

    const uint joints_count=reader.read<uint>();
    ad->joints.resize(joints_count);
    for(uint i=0;i<joints_count;++i)
    {
        pmd_phys_data::joint &j=ad->joints[i];
        j.name=utf8_from_shiftjis(data.get_data(reader.get_offset()),20);
        reader.skip(20);
        j.rigid_src=reader.read<uint>();
        j.rigid_dst=reader.read<uint>();
        j.location=reader.read<nya_math::vec3>();
        j.rotation=reader.read<nya_math::vec3>();
        j.location_max=reader.read<nya_math::vec3>();
        j.location_min=reader.read<nya_math::vec3>();
        j.rotation_max=reader.read<nya_math::vec3>();
        j.rotation_min=reader.read<nya_math::vec3>();
        j.location_spring=reader.read<nya_math::vec3>();
        j.rotation_spring=reader.read<nya_math::vec3>();
    }

    return true;
}

const pmd_loader::additional_data *pmd_loader::get_additional_data(const nya_scene::mesh &m)
{
    if(!m.internal().get_shared_data().is_valid())
        return 0;

    nya_scene::shared_mesh::additional_data *d=m.internal().get_shared_data()->add_data;
    if(!d)
        return 0;

    const char *type=d->type();
    if(!type || strcmp(type,"pmd")!=0)
        return 0;

    return static_cast<pmd_loader::additional_data*>(static_cast<add_data*>(d));
}
