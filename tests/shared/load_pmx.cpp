//https://code.google.com/p/nya-engine/

#include "load_pmx.h"
#include "scene/mesh.h"
#include "memory/memory_reader.h"
#include "string_encoding.h"

int pmx_loader::read_idx(nya_memory::memory_reader &reader,int size)
{
    switch(size)
    {
        case 1: return reader.read<char>();
        case 2: return reader.read<short>();
        case 4: return reader.read<int>();
    }

    return 0;
}

bool pmx_loader::load(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name)
{
    if(!data.get_size())
        return false;

    nya_memory::memory_reader reader(data.get_data(),data.get_size());
    if(!reader.test("PMX ",4))
        return false;
    
    if(reader.read<float>()!=2.0f)
        return false;
    
    const char header_size=reader.read<char>();
    if(header_size!=sizeof(pmx_header))
        return false;
    
    const pmx_header header=reader.read<pmx_header>();
    if(header.extended_uv>0)
        return false; //ToDo
    
    for(int i=0;i<4;++i)
    {
        const int size=reader.read<int>();
        reader.skip(size);
    }
    
    const int vert_count=reader.read<int>();
    if(!vert_count)
        return false;

    std::vector<vert> verts(vert_count);

    for(int i=0;i<vert_count;++i)
    {
        vert &v=verts[i];
        
        v.pos[0][0]=reader.read<float>();
        v.pos[0][1]=reader.read<float>();
        v.pos[0][2]=-reader.read<float>();

        v.normal[0]=reader.read<float>();
        v.normal[1]=reader.read<float>();
        v.normal[2]=-reader.read<float>();
        
        v.tc[0]=reader.read<float>();
        v.tc[1]=1.0-reader.read<float>();
        
        switch(reader.read<char>())
        {
            case 0:
                v.bone_idx[0]=read_idx(reader,header.bone_idx_size);
                v.bone_weight[0]=1.0f;
                for(int j=1;j<4;++j)
                    v.bone_weight[j]=v.bone_idx[j]=0.0f;
                break;
                
            case 1:
                v.bone_idx[0]=read_idx(reader,header.bone_idx_size);
                v.bone_idx[1]=read_idx(reader,header.bone_idx_size);
                
                v.bone_weight[0]=reader.read<float>();
                v.bone_weight[1]=1.0f-v.bone_weight[0];
                
                for(int j=2;j<4;++j)
                    v.bone_weight[j]=v.bone_idx[j]=0.0f;
                break;
                
            case 2:
                for(int j=0;j<4;++j)
                    v.bone_idx[j]=read_idx(reader,header.bone_idx_size);
                
                for(int j=0;j<4;++j)
                    v.bone_weight[j]=reader.read<float>();
                break;
                
            case 3:
                v.bone_idx[0]=read_idx(reader,header.bone_idx_size);
                v.bone_idx[1]=read_idx(reader,header.bone_idx_size);
                
                v.bone_weight[0]=reader.read<float>();
                v.bone_weight[1]=1.0f-v.bone_weight[0];
                
                for(int j=2;j<4;++j)
                    v.bone_weight[j]=v.bone_idx[j]=0.0f;

                reader.skip(sizeof(float)*3*3);
                break;

            default: return false;
        }

        reader.read<float>(); //edge
    }

    const int indices_count=reader.read<int>();
    if(header.index_size==2)
        res.vbo.set_index_data(reader.get_data(),nya_render::vbo::index2b,indices_count);
    else if(header.index_size==4)
        res.vbo.set_index_data(reader.get_data(),nya_render::vbo::index4b,indices_count);
    else
        return false;

    reader.skip(indices_count*header.index_size);

    const int textures_count=reader.read<int>();
    std::vector<std::string> tex_names(textures_count);
    for(int i=0;i<textures_count;++i)
    {
        const int str_len=reader.read<int>();
        if(header.text_encoding==0)
        {
            /*
             NSData* data = [[[NSData alloc] initWithBytes:reader.get_data()
             length:str_len] autorelease];
             NSString* str = [[NSString alloc] initWithData:data
             encoding:NSUTF16LittleEndianStringEncoding];
             tex_names[i].assign(str.UTF8String);
             */
            
            const char *data=(const char*)reader.get_data();
            tex_names[i].resize(str_len/2);
            for(int j=0;j<str_len/2;++j)
                tex_names[i][j]=data[j*2];
        }
        else
            tex_names[i]=std::string((const char*)reader.get_data(),str_len);
        
        reader.skip(str_len);
    }
    
    const int mat_count=reader.read<int>();
    res.groups.resize(mat_count);
    res.materials.resize(mat_count);
    
    std::string path(name);
    size_t p=path.rfind("/");
    if(p==std::string::npos)
        p=path.rfind("\\");
    if(p==std::string::npos)
        path.clear();
    else
        path.resize(p+1);

    for(int i=0,offset=0;i<mat_count;++i)
    {
        nya_scene::shared_mesh::group &g=res.groups[i];
        nya_scene::material &m = res.materials[i];

        for(int j=0;j<2;++j)
        {
            const int name_len=reader.read<int>();
            std::string name((const char*)reader.get_data(),name_len);
            reader.skip(name_len);
            m.set_name(name.c_str());
        }

        std::string sh_defines;
        
        pmx_material_params params=reader.read<pmx_material_params>();
        
        char buf[255];
        sprintf(buf,"#define k_d vec4(%f,%f,%f,%f)\n",params.diffuse[0],params.diffuse[1],
                params.diffuse[2],params.diffuse[3]);
        sh_defines.append(buf);
        
        sprintf(buf,"#define k_s vec4(%f,%f,%f,%f)\n",params.specular[0],params.specular[1],
                params.specular[2],params.shininess);
        sh_defines.append(buf);
        
        sprintf(buf,"#define k_a vec3(%f,%f,%f)\n",params.ambient[0],params.ambient[1],
                params.ambient[2]);
        sh_defines.append(buf);

        const char unsigned flag=reader.read<unsigned char>();
        pmx_edge_params edge=reader.read<pmx_edge_params>();

        const int tex_idx=read_idx(reader,header.texture_idx_size);
        const int sph_tex_idx=read_idx(reader,header.texture_idx_size);
        const int sph_mode=reader.read<char>();
        const char toon_flag=reader.read<char>();
        
        int toon_tex_idx=-1;
        if(toon_flag==0)
            toon_tex_idx=read_idx(reader,header.texture_idx_size);
        else if(toon_flag==1)
            toon_tex_idx=reader.read<char>();
        else
            return false;

        const int comment_len=reader.read<int>();
        reader.skip(comment_len);

        g.offset=offset;
        g.count=reader.read<int>();
        g.material_idx=i;
        offset+=g.count;

        nya_scene::shared_shader sh_;
        sh_.shdr.set_sampler("base",0);
        sh_.samplers_count=1;
        sh_.samplers["diffuse"]=0;

        if(tex_idx>=0 && tex_idx<(int)tex_names.size())
        {
            nya_scene::texture tex;
            tex.load((path+tex_names[tex_idx]).c_str());
            m.set_texture("diffuse",tex);
            //printf("tex %d %s ",tex_idx,tex_names[tex_idx].c_str());
        }

        if(sph_tex_idx>=0 && sph_tex_idx<(int)tex_names.size())
        {
            const std::string &name=tex_names[sph_tex_idx];
            if(sph_mode>0 && !name.empty())
            {
                sh_.shdr.set_sampler("env",1);
                sh_.samplers["env"]=1;
                ++sh_.samplers_count;
                
                nya_scene::texture tex;
                tex.load((path+name).c_str());
                m.set_texture("env",tex);
                //printf("sp %d %d %s\n",sph_mode,sph_tex_idx,tex_names[sph_tex_idx].c_str());
                
                char last=name[name.length()-1];
                if(last=='h' || last=='H')
                    sh_defines+="#define sph\n";
                else
                    sh_defines+="#define spa\n";
            }
        }
        //else
        //    printf("sp %d %d\n",sph_mode,sph_tex_idx);

        nya_scene::shader sh;
        sh_.vertex=sh_defines+"uniform vec3 bones_pos[255]; uniform vec4 bones_rot[255];"
        "vec3 tr(vec3 pos,int idx) { vec4 q=bones_rot[idx];"
        "return bones_pos[idx]+pos+cross(q.xyz,cross(q.xyz,pos)+pos*q.w)*2.0; }"
        "vec3 trn(vec3 normal,int idx) { vec4 q=bones_rot[idx];"
        "return normal+cross(q.xyz,cross(q.xyz,normal)+normal*q.w)*2.0; }"
        "varying vec4 tc; varying vec3 normal;"
        "void main() { tc.xy=gl_MultiTexCoord0.xy;"

        "vec3 pos=tr(gl_Vertex.xyz,int(gl_MultiTexCoord1.x))*gl_MultiTexCoord2.x;"
        "normal=trn(gl_Normal.xyz,int(gl_MultiTexCoord1.x))*gl_MultiTexCoord2.x;"

        "if(gl_MultiTexCoord2.y>0.0) {"
        "pos+=tr(gl_MultiTexCoord3.xyz,int(gl_MultiTexCoord1.y))*gl_MultiTexCoord2.y;"
        "normal+=trn(gl_Normal.xyz,int(gl_MultiTexCoord1.y))*gl_MultiTexCoord2.y; }"

        "if(gl_MultiTexCoord2.z>0.0) {"
        "pos+=tr(gl_MultiTexCoord4.xyz,int(gl_MultiTexCoord1.z))*gl_MultiTexCoord2.z;"
        "normal+=trn(gl_Normal.xyz,int(gl_MultiTexCoord1.z))*gl_MultiTexCoord2.z; }"
        
        "if(gl_MultiTexCoord2.w>0.0) {"
        "pos+=tr(gl_MultiTexCoord5.xyz,int(gl_MultiTexCoord1.w))*gl_MultiTexCoord2.w;"
        "normal+=trn(gl_Normal.xyz,int(gl_MultiTexCoord1.w))*gl_MultiTexCoord2.w; }"

        "vec3 r = normalize((gl_ModelViewProjectionMatrix * vec4(normal,0.0)).xyz);"
        "r.z-=1.0; tc.zw = 0.5*r.xy/length(r) + 0.5;"
        "gl_Position=gl_ModelViewProjectionMatrix*vec4(pos,gl_Vertex.w); }";

        sh_.pixel=sh_defines+"varying vec4 tc; varying vec3 normal;\n"
        "uniform sampler2D base;\n"
        "uniform sampler2D env;\n"
        "void main() {\n"
        "vec4 tm=texture2D(base,tc.xy);\n"
        "if(tm.a<0.001) discard;\n"
        "#ifdef spa\n"
        "   vec4 em=texture2D(env,tc.zw);\n"
        "   gl_FragColor=vec4(tm.rgb+em.rgb,tm.a); }\n"
        "#elif defined sph\n"
        "   vec4 em=texture2D(env,tc.zw);\n"
        "   gl_FragColor=vec4(tm.xyz*(k_a+k_d.rgb*em.rgb),tm.a); }\n"
        "#else\n"
        "   gl_FragColor=tm; }\n"
        "#endif\n";
        
        sh_.shdr.add_program(nya_render::shader::vertex,sh_.vertex.c_str());
        sh_.shdr.add_program(nya_render::shader::pixel,sh_.pixel.c_str());
        sh_.predefines.resize(2);
        sh_.predefines[0].type=nya_scene::shared_shader::bones_pos;
        sh_.predefines[0].location=sh_.shdr.get_handler("bones_pos");
        sh_.predefines[1].type=nya_scene::shared_shader::bones_rot;
        sh_.predefines[1].location=sh_.shdr.get_handler("bones_rot");
        sh.create(sh_);
        
        m.set_shader(sh);
        m.set_blend(true,nya_render::blend::src_alpha,nya_render::blend::inv_src_alpha);
        m.set_cull_face(!(flag & (1<<0)),nya_render::cull_face::cw);

        if(!(flag & (1<<4)))
            continue;

        res.groups.resize(res.groups.size()+1);
        nya_scene::shared_mesh::group &ge=res.groups.back();
        ge=res.groups[i];
        ge.material_idx=int(res.materials.size());
        res.materials.resize(res.materials.size()+1);
        nya_scene::material &me = res.materials.back();

        nya_scene::shared_shader she_;
        nya_scene::shader she;

        sprintf(buf,"%f",edge.width*0.02f);
        she_.vertex="uniform vec3 bones_pos[255]; uniform vec4 bones_rot[255];"
        "vec3 tr(vec3 pos,int idx) { vec4 q=bones_rot[idx];"
        "return bones_pos[idx]+pos+cross(q.xyz,cross(q.xyz,pos)+pos*q.w)*2.0; }"
        "void main() {"
        "vec3 offset=gl_Normal*"+std::string(buf)+";"

        "vec3 pos=tr(gl_Vertex.xyz+offset,int(gl_MultiTexCoord1.x))*gl_MultiTexCoord2.x;"
        "if(gl_MultiTexCoord2.y>0.0) pos+=tr(gl_MultiTexCoord3.xyz+offset,int(gl_MultiTexCoord1.y))*gl_MultiTexCoord2.y;"
        "if(gl_MultiTexCoord2.z>0.0) pos+=tr(gl_MultiTexCoord4.xyz+offset,int(gl_MultiTexCoord1.z))*gl_MultiTexCoord2.z;"
        "if(gl_MultiTexCoord2.w>0.0) pos+=tr(gl_MultiTexCoord5.xyz+offset,int(gl_MultiTexCoord1.w))*gl_MultiTexCoord2.w;"

        "gl_Position=gl_ModelViewProjectionMatrix*vec4(pos,gl_Vertex.w); }";
        sprintf(buf,"%f,%f,%f,%f",edge.color[0],edge.color[1],edge.color[2],edge.color[3]);
        she_.pixel="void main() { gl_FragColor=vec4("+std::string(buf)+"); }\n";

        she_.shdr.add_program(nya_render::shader::vertex,she_.vertex.c_str());
        she_.shdr.add_program(nya_render::shader::pixel,she_.pixel.c_str());
        she_.predefines.resize(2);
        she_.predefines[0].type=nya_scene::shared_shader::bones_pos;
        she_.predefines[0].location=she_.shdr.get_handler("bones_pos");
        she_.predefines[1].type=nya_scene::shared_shader::bones_rot;
        she_.predefines[1].location=she_.shdr.get_handler("bones_rot");
        she.create(she_);

        me.set_shader(she);
        me.set_blend(true,nya_render::blend::src_alpha,nya_render::blend::inv_src_alpha);
        me.set_cull_face(true,nya_render::cull_face::ccw);
    }

    typedef unsigned short ushort;

    const int bones_count=reader.read<int>();
    for(int i=0;i<bones_count;++i)
    {
        const int name_len=reader.read<int>();
        const std::string name=header.text_encoding?std::string((const char*)reader.get_data(),name_len):
                                                           utf8_from_utf16le(reader.get_data(),name_len);
        reader.skip(name_len);
        reader.skip(reader.read<int>());

        nya_math::vec3 pos;
        pos.x=reader.read<float>();
        pos.y=reader.read<float>();
        pos.z=-reader.read<float>();

        const int parent=read_idx(reader,header.bone_idx_size);
        reader.read<int>(); //ToDo: transform order

        const flag<ushort> f(reader.read<ushort>());
        if(f.c(0x0001))
            read_idx(reader,header.bone_idx_size);
        else
            reader.skip(sizeof(float)*3);

        const bool has_add_rot=f.c(0x0100);
        const bool has_add_pos=f.c(0x0200);
        if(has_add_rot || has_add_pos)
        {
            read_idx(reader,header.bone_idx_size); //ToDo
            reader.read<float>();
        }

        if(f.c(0x0400))
            reader.skip(sizeof(float)*3); //ToDo
        
        if(f.c(0x0800))
            reader.skip(sizeof(float)*3*2); //ToDo

        if(f.c(0x2000))
            reader.read<int>(); //ToDo

        if(res.skeleton.add_bone(name.c_str(),pos,parent,true)!=i)
        {
            nya_log::get_log()<<"pmx load error: invalid bone\n";
            return false;
        }

        const bool has_ik=f.c(0x0020);
        if(has_ik)
        {
            const int eff=read_idx(reader,header.bone_idx_size);
            const int count=reader.read<int>();
            const float k=reader.read<float>();

            const int ik=res.skeleton.add_ik(i,eff,count,k,true);
            const int link_count=reader.read<int>();
            for(int j=0;j<link_count;++j)
            {
                const int link=read_idx(reader,header.bone_idx_size);
                if(reader.read<char>())
                {
                    nya_math::vec3 from;
                    from.x=reader.read<float>();
                    from.y=reader.read<float>();
                    from.z=reader.read<float>();

                    nya_math::vec3 to;
                    to.x=reader.read<float>();
                    to.y=reader.read<float>();
                    to.z=reader.read<float>();

                    //res.skeleton.add_ik_link(ik,link,-to.x,-from.x,true);
                    res.skeleton.add_ik_link(ik,link,0.001f,nya_math::constants::pi,true); //ToDo
                }
                else
                    res.skeleton.add_ik_link(ik,link,true);
            }
        }
    }

    for(int i=0;i<vert_count;++i)
    {
        vert &v=verts[i];

        for(int j=3;j>=0;--j)
        if(v.bone_weight[j]>0.0f)
        {
            const nya_math::vec3 &p=res.skeleton.get_bone_pos(v.bone_idx[j]);
            v.pos[j][0]=v.pos[0][0]-p.x;
            v.pos[j][1]=v.pos[0][1]-p.y;
            v.pos[j][2]=v.pos[0][2]-p.z;
        }
    }

    res.vbo.set_vertex_data(&verts[0],sizeof(vert),vert_count);
    int offset=0;
    res.vbo.set_vertices(offset,3); offset+=sizeof(verts[0].pos[0]);
    res.vbo.set_tc(3,offset,3); offset+=sizeof(verts[0].pos[1]);
    res.vbo.set_tc(4,offset,3); offset+=sizeof(verts[0].pos[2]);
    res.vbo.set_tc(5,offset,3); offset+=sizeof(verts[0].pos[3]);
    res.vbo.set_normals(offset); offset+=sizeof(verts[0].normal);
    res.vbo.set_tc(0,offset,2); offset+=sizeof(verts[0].tc);
    res.vbo.set_tc(1,offset,4); offset+=sizeof(verts[0].bone_idx);
    res.vbo.set_tc(2,offset,4); offset+=sizeof(verts[0].bone_weight);

    return true;
}
