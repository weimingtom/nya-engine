//https://code.google.com/p/nya-engine/

#include "load_pmx.h"
#include "scene/mesh.h"
#include "memory/memory_reader.h"

int pmx_loader::read_idx(nya_memory::memory_reader &reader,int size)
{
    switch(size)
    {
        case 1: return reader.read<unsigned char>();
        case 2: return reader.read<unsigned short>();
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
    
    std::vector<vert> verts(vert_count);
    
    for(int i=0;i<vert_count;++i)
    {
        vert &v=verts[i];
        
        v.pos[0]=reader.read<float>();
        v.pos[1]=reader.read<float>();
        v.pos[2]=-reader.read<float>();
        
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
    
    res.vbo.set_vertex_data(&verts[0],sizeof(vert),vert_count);
    res.vbo.set_vertices(0,3);
    res.vbo.set_normals(sizeof(float)*3);
    res.vbo.set_tc(0,sizeof(float)*6,2);
    res.vbo.set_tc(1,sizeof(float)*8,4);
    res.vbo.set_tc(2,sizeof(float)*12,4);
    
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
        for(int j=0;j<2;++j)
        {
            const int name_len=reader.read<int>();
            reader.skip(name_len);
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
        
        reader.read<char>();//flag
        reader.skip(sizeof(float)*4);//edge color
        reader.read<float>();//edge size
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
        
        nya_scene::material &m = res.materials[i];
        
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
        //printf("sp %d %d\n",sph_mode,sph_tex_idx);
        
        nya_scene::shader sh;
        sh_.vertex=sh_defines+"varying vec4 tc;"
        "uniform vec4 cam_pos;"
        "void main() { tc.xy=gl_MultiTexCoord0.xy;"
        "vec3 u = normalize( gl_Vertex.xyz-cam_pos.xyz );"
        "vec3 r = normalize(reflect( u, gl_Normal ));"
        //"r.z+=1.0;"
        "tc.zw = 0.5*r.xy/length(r) + 0.5;"
        "gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex; }";
        
        sh_.pixel=sh_defines+"varying vec4 tc;\n"
        "uniform sampler2D base;\n"
        "uniform sampler2D env;\n"
        "void main() {\n"
        "vec4 tm=texture2D(base,tc.xy);\n"
        
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
        sh_.predefines.resize(1);
        sh_.predefines[0].type=nya_scene::shared_shader::camera_pos;
        sh_.predefines[0].transform=nya_scene::shared_shader::local;
        sh_.predefines[0].location=sh_.shdr.get_handler("cam_pos");
        sh.create(sh_);
        
        m.set_shader(sh);
        m.set_blend(true,nya_render::blend::src_alpha,nya_render::blend::inv_src_alpha);
    }
    
    return true;
}