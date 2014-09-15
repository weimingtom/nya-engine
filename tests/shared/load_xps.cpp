//https://code.google.com/p/nya-engine/

#include "load_xps.h"
#include "scene/mesh.h"
#include "memory/memory_reader.h"
#include "memory/invalid_object.h"

#include "resources/resources.h"

class text_reader
{
public:
    template<typename t> t read(const char *str="\n\r")
    {
        const std::string s=read_string(str);
        std::istringstream iss(s);
        t out;
        iss>>out;
        return out;
    }

    std::string read_string(const char *str="\n\r")
    {
        size_t i=m_offset;
        for(;i<m_size;++i)
        {
            const char c=*(m_text+i);
            if(!c || strchr(str,c))
                break;
        }

        if(i<=m_offset)
            return std::string();

        const char *text=m_text+m_offset;
        const size_t len=i-m_offset;
        m_offset+=len;

        for(;m_offset<m_size;++m_offset)
        {
            const char c=*(m_text+m_offset);
            if(c && !strchr(str,c))
                break;
        }

        return std::string(text,len);
    }

    void skip_color() { read_string(); } //ToDo: remove
    void skip_skining() { read_string(); read_string(); } //ToDo: remove
    void skip(size_t) {}

public:
    text_reader(const char *text,size_t size): m_text(size?text:0),m_size(text?size:0),m_offset(0)
    {
        if(size>3 && strncmp(m_text,"\xef\xbb\xbf",3)==0)
        {
            m_size-=3;
            m_text+=3;
        }
    }

private:
    const char *m_text;
    size_t m_size;
    size_t m_offset;
};

class binary_reader: public nya_memory::memory_reader
{
public:
    template<typename t> t read(const char *str=0) { return memory_reader::read<t>(); }

    std::string read_string(const char *str=0)
    {
        unsigned char len=read<unsigned char>();
        std::string ret((const char *)get_data(),len);
        skip(len);
        return ret;
    }

    void skip_color() { skip(4); } //ToDo: remove
    void skip_skining() { skip(4*4*2+2*4); } //ToDo: remove

    binary_reader(const void *data,size_t size): memory_reader(data,size) {}
};

struct render_group_props
{
    enum shading_mode
    {
        shading_no,
        shading_yes,
        shading_yes_no,
        shading_vertex
    };

    enum spec_mode
    {
        spec_no,
        spec_yes,
        spec_intencity
    };

    shading_mode shading;
    bool alpha;

public:
    render_group_props(): shading(shading_no),alpha(false) {}
    render_group_props(const std::string &s) { *this=get(s); }
    render_group_props(shading_mode shading,bool alpha,spec_mode spec_hl,bool bump_rep1,
                       bool bump_rep2,const char *tex,const char *tex2=0,const char *tex3=0,
                       const char *tex4=0,const char *tex5=0,const char *tex6=0,const char *tex7=0)
    {
        this->shading=shading;
        this->alpha=alpha;
        //ToDo
    }

public:
    static const render_group_props &get(const std::string &s)
    {
        const static render_group_props xnl_props[]=
        {
        //1
            render_group_props(shading_yes,false,spec_yes,true,true,"diffuse","lightmap","bump","mask","bump1","bump2"),
            render_group_props(shading_yes,false,spec_yes,false,false,"diffuse","lightmap","bump"),
            render_group_props(shading_yes,false,spec_no,false,false,"diffuse","lightmap"),
            render_group_props(shading_yes,false,spec_yes,false,false,"diffuse","bump"),
            render_group_props(shading_yes,false,spec_no,false,false,"diffuse"),
        //6
            render_group_props(shading_yes,true,spec_yes,false,false,"diffuse","bump"),
            render_group_props(shading_yes,true,spec_no,false,false,"diffuse"),
            render_group_props(shading_yes,true,spec_yes,false,false,"diffuse","lightmap","bump"),
            render_group_props(shading_yes,true,spec_no,false,false,"diffuse","lightmap"),
            render_group_props(shading_no,false,spec_no,false,false,"diffuse"),
        //11
            render_group_props(shading_vertex,false,spec_yes,false,false,"diffuse","bump"),
            render_group_props(shading_vertex,true,spec_yes,false,false,"diffuse","bump"),
            render_group_props(shading_no,false,spec_no,false,false,"diffuse"),
            render_group_props(shading_no,false,spec_yes,false,false,"diffuse","bump"),
            render_group_props(shading_no,true,spec_yes,false,false,"diffuse","bump"),
        //16
            render_group_props(shading_yes,false,spec_no,false,false,"diffuse"),
            render_group_props(shading_yes,false,spec_no,false,false,"diffuse","lightmap"),
            render_group_props(shading_yes,false,spec_no,false,false,"diffuse"),
            render_group_props(shading_yes,false,spec_no,false,false,"diffuse","lightmap"),
            render_group_props(shading_yes,false,spec_yes,true,true,"diffuse","lightmap","bump","mask","bump1","bump2"),
        //21
            render_group_props(shading_no,true,spec_no,false,false,"diffuse"),
            render_group_props(shading_yes,false,spec_yes,true,true,"diffuse","lightmap","bump","mask","bump1","bump2","spec"),
            render_group_props(shading_yes,true,spec_yes,true,true,"diffuse","lightmap","bump","mask","bump1","bump2","spec"),
            render_group_props(shading_yes,false,spec_yes,false,false,"diffuse","lightmap","bump","spec"),
            render_group_props(shading_yes,true,spec_yes,false,false,"diffuse","lightmap","bump","spec"),
        //26
            render_group_props(shading_yes_no,false,spec_intencity,false,false,"diffuse","bump","env"),
            render_group_props(shading_yes_no,true,spec_intencity,false,false,"diffuse","bump","env"),
            render_group_props(shading_yes_no,false,spec_intencity,true,true,"diffuse","bump","mask","bump1","bump2","env"),
            render_group_props(shading_yes_no,true,spec_intencity,true,true,"diffuse","bump","mask","bump1","bump2","env"),
            render_group_props(shading_yes_no,false,spec_intencity,false,false,"diffuse","bump","emission"),
        //31
            render_group_props(shading_yes_no,true,spec_intencity,false,false,"diffuse","bump","emission"),
            render_group_props(shading_yes,false,spec_yes,false,false,"diffuse"),
            render_group_props(shading_yes,true,spec_yes,false,false,"diffuse"),
            render_group_props(),
            render_group_props(),
        //36
            render_group_props(shading_yes_no,false,spec_intencity,true,false,"diffuse","bump","emission"),
            render_group_props(shading_yes_no,true,spec_intencity,true,false,"diffuse","bump","emission"),
            render_group_props(shading_yes_no,false,spec_intencity,true,false,"diffuse","bump","spec","emission"),
            render_group_props(shading_yes_no,true,spec_intencity,true,false,"diffuse","bump","spec","emission"),
        };

        if(s.empty())
            return nya_memory::get_invalid_object<render_group_props>();

        const unsigned int ridx=atoi(s.c_str())-1;
        if(ridx>=sizeof(xnl_props)/sizeof(xnl_props[0]))
            return nya_memory::get_invalid_object<render_group_props>();

        return xnl_props[ridx];
    }

};

template<typename reader_t>bool load_mesh(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name)
{
    if(!data.get_size())
        return false;

    std::string path(name);
    size_t p=path.rfind("/");
    if(p==std::string::npos)
        p=path.rfind("\\");
    if(p==std::string::npos)
        path.clear();
    else
        path.resize(p+1);

    reader_t reader((const char *)data.get_data(),data.get_size());
    typedef unsigned int uint;
    uint bones_count=reader.template read<uint>();
    if(bones_count>1024)
        return false;

    for(uint i=0;i<bones_count;++i)
    {
        const std::string name=reader.read_string();
        const short parent=reader.template read<short>();
        nya_math::vec3 pos;
        pos.x=reader.template read<float>("\n\r ");
        pos.y=reader.template read<float>("\n\r ");
        pos.z=reader.template read<float>();

        res.skeleton.add_bone(name.c_str(),pos,nya_math::quat(),parent,true);
    }

    std::vector<xps_loader::vert> vertices;
    typedef unsigned short ushort;
    std::vector<uint> indices;

    uint groups_count=reader.template read<uint>();
    res.groups.resize(groups_count);
    res.materials.resize(groups_count);
    for(uint i=0;i<groups_count;++i)
    {
        const std::string name=reader.read_string();
        const uint uvlayers=reader.template read<uint>();
        const uint tex_count=reader.template read<uint>();

        std::vector<std::string> tex_names(tex_count);
        std::vector<uint> tex_uvlayers(tex_count);
        for(uint j=0;j<tex_count;++j)
        {
            std::string name=reader.read_string();
            size_t p=name.rfind("/");
            if(p==std::string::npos)
                p=name.rfind("\\");
            if(p!=std::string::npos)
                name=name.substr(p+1);

            tex_names[j]=path+name;
            tex_uvlayers[j]=reader.template read<uint>();
        }

        const uint voffset=(uint)vertices.size();
        const uint vcount=reader.template read<uint>();
        vertices.resize(voffset+vcount);
        for(uint j=voffset;j<voffset+vcount;++j)
        {
            xps_loader::vert &v=vertices[j];

            v.pos.x=reader.template read<float>("\n\r ");
            v.pos.y=reader.template read<float>("\n\r ");
            v.pos.z=reader.template read<float>();

            v.normal.x=reader.template read<float>("\n\r ");
            v.normal.y=reader.template read<float>("\n\r ");
            v.normal.z=reader.template read<float>();

            reader.skip_color(); //skip colors, 4 bytes

            v.tc.x=reader.template read<float>("\n\r ");
            v.tc.y=1.0f-reader.template read<float>();

            reader.skip_skining(); //skip bones 1-4 ints, bone weights 1-4 floats
        }

        const uint ioffset=(uint)indices.size();
        const uint fcount=reader.template read<uint>();
        indices.resize(ioffset+fcount*3);
        for(uint j=0;j<fcount;++j)
        {
            uint *f=&indices[ioffset+j*3];
            f[0]=voffset+reader.template read<uint>("\n\r ");
            f[2]=voffset+reader.template read<uint>("\n\r ");
            f[1]=voffset+reader.template read<uint>();
        }

        nya_scene::shared_mesh::group &g=res.groups[i];
        g.name=name;
        g.offset=ioffset;
        g.count=fcount*3;
        g.material_idx=i;
        g.elem_type=nya_render::vbo::triangles;

        nya_scene::material &m=res.materials[i];

        const render_group_props rgp(name);
        if(rgp.alpha)
            m.load("xps_a.txt");
        else
            m.load("xps.txt");

        if(!tex_names.empty())
            m.set_texture("diffuse", nya_scene::texture(tex_names[0].c_str()));
    }

    if(res.groups.empty())
        return false;

    if(!vertices.size())
        return false;

    res.vbo.set_vertex_data(&vertices[0],(uint)sizeof(vertices[0]),(uint)vertices.size());
    res.vbo.set_normals(3*4);
    res.vbo.set_tc(0,6*4,2);

    if(!indices.size())
        return false;

    if(vertices.size()<65535)
    {
        std::vector<unsigned short> indices2b(indices.size());
        for(size_t i=0;i<indices.size();++i)
            indices2b[i]=indices[i];
        res.vbo.set_index_data(&indices2b[0],nya_render::vbo::index2b,(uint)indices2b.size());
    }
    else
        res.vbo.set_index_data(&indices[0],nya_render::vbo::index4b,(uint)indices.size());

    return true;
}

bool xps_loader::load_mesh(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name)
{
    return ::load_mesh<binary_reader>(res,data,name);
}

bool xps_loader::load_mesh_ascii(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name)
{
    return ::load_mesh<text_reader>(res,data,name);
}
