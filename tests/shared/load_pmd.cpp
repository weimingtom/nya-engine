//https://code.google.com/p/nya-engine/

#include "load_pmd.h"
#include "scene/mesh.h"
#include "memory/memory_reader.h"
#include "string_encoding.h"

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
    res.materials.resize(mat_count);

    nya_scene::shader sh;
    nya_scene::shared_shader sh_;
    sh_.shdr.set_sampler("base",0);
    sh_.samplers_count=1;
    sh_.samplers["diffuse"]=0;

    sh_.vertex="uniform vec3 bones_pos[255]; uniform vec4 bones_rot[255];"
    "vec3 tr(vec3 pos,int idx) { vec4 q=bones_rot[idx];"
    "return bones_pos[idx]+pos+cross(q.xyz,cross(q.xyz,pos)+pos*q.w)*2.0; }"
    "varying vec2 tc;"
    "void main()"
    "{  int bone0=int(gl_MultiTexCoord1.x); int bone1=int(gl_MultiTexCoord1.y);"
    "vec3 pos=mix(tr(gl_MultiTexCoord2.xyz,bone1),tr(gl_Vertex.xyz,bone0),gl_MultiTexCoord1.z);"
    "tc=gl_MultiTexCoord0.xy; gl_Position=gl_ModelViewProjectionMatrix*vec4(pos,1.0); }";

    sh_.pixel="varying vec2 tc; uniform sampler2D base; void main() { vec4 tm=texture2D(base,tc.xy);"
              "if(tm.a<0.001) discard;\n"
              " gl_FragColor=tm; }";

    sh_.shdr.add_program(nya_render::shader::vertex,sh_.vertex.c_str());
    sh_.shdr.add_program(nya_render::shader::pixel,sh_.pixel.c_str());
    sh_.predefines.resize(2);
    sh_.predefines[0].type=nya_scene::shared_shader::bones_pos;
    sh_.predefines[0].location=sh_.shdr.get_handler("bones_pos");
    sh_.predefines[1].type=nya_scene::shared_shader::bones_rot;
    sh_.predefines[1].location=sh_.shdr.get_handler("bones_rot");
    sh.create(sh_);
    
    nya_scene::shared_shader she_;
    nya_scene::shader she;
    
    she_.vertex="uniform vec3 bones_pos[255]; uniform vec4 bones_rot[255];"
    "vec3 tr(vec3 pos,int idx) { vec4 q=bones_rot[idx];"
    "return bones_pos[idx]+pos+cross(q.xyz,cross(q.xyz,pos)+pos*q.w)*2.0; }"
    "void main()"
    "{  int bone0=int(gl_MultiTexCoord1.x); int bone1=int(gl_MultiTexCoord1.y);"
    "vec3 pos=mix(tr(gl_MultiTexCoord2.xyz,bone1),tr(gl_Vertex.xyz,bone0),gl_MultiTexCoord1.z);"
    "pos.xyz+=gl_Normal*0.01;"
    "gl_Position=gl_ModelViewProjectionMatrix*vec4(pos,1.0); }";
    she_.pixel="void main() { gl_FragColor=vec4(0.0,0.0,0.0,1.0); }\n";
    
    she_.shdr.add_program(nya_render::shader::vertex,she_.vertex.c_str());
    she_.shdr.add_program(nya_render::shader::pixel,she_.pixel.c_str());
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
        reader.read<mmd_material_params>();

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

        m.set_shader(sh);
        m.set_blend(true,nya_render::blend::src_alpha,nya_render::blend::inv_src_alpha);
        m.set_cull_face(true,nya_render::cull_face::cw);

        if(!edge_flag)
            continue;

        res.groups.resize(res.groups.size()+1);
        nya_scene::shared_mesh::group &ge=res.groups.back();
        ge=res.groups[i];
        ge.material_idx=int(res.materials.size());
        res.materials.resize(res.materials.size()+1);
        nya_scene::material &me = res.materials.back();

        me.set_shader(she);
        //me.set_blend(true,nya_render::blend::src_alpha,nya_render::blend::inv_src_alpha);
        me.set_cull_face(true,nya_render::cull_face::ccw);
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

        if(res.skeleton.add_bone(name.c_str(),pos,parent,true)!=i)
        {
            nya_log::get_log()<<"pmd load error: invalid bone\n";
            return false;
        }
    }

    for(size_t i=0;i<vertices.size();i+=14)
    {
        const nya_math::vec3 bp0=res.skeleton.get_bone_pos(int(vertices[i+8]));

        vertices[i]-=bp0.x;
        vertices[i+1]-=bp0.y;
        vertices[i+2]-=bp0.z;

        const nya_math::vec3 bp1=res.skeleton.get_bone_pos(int(vertices[i+9]));

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

            if(strcmp(name,"\xe5\xb7\xa6\xe3\x81\xb2\xe3\x81\x96")==0 || strcmp(name,"\xe5\x8f\xb3\xe3\x81\xb2\xe3\x81\x96")==0)
                res.skeleton.add_ik_link(ik,link,0.001f,nya_math::constants::pi);
            else
                res.skeleton.add_ik_link(ik,link);
        }
    }

    const ushort morphs_count=reader.read<ushort>();
    for(ushort i=0;i<morphs_count;++i)
    {
        reader.skip(20);//name
        const uint size=reader.read<uint>();
        reader.read<uchar>();//type
        reader.skip(size*(sizeof(uint)+sizeof(float)*3));
    }

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

    const uint rigid_bodys_count=reader.read<uint>();

    return true;
}
