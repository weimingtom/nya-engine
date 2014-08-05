//https://code.google.com/p/nya-engine/

#include "material.h"
#include "formats/text_parser.h"
#include "formats/string_convert.h"
#include "memory/invalid_object.h"
#include "system/system.h"
#include <list>
#include <cstring>

namespace nya_scene
{

const char *material::default_pass="default";

namespace
{
    nya_scene::texture missing_texture()
    {
        static nya_scene::texture missing_red;
        /*
        static nya_scene::texture missing_white;

        static bool initialised=false;
        if(!initialised)
        {
            const unsigned char red_data[4]={255,0,0,255};
            const unsigned char white_data[4]={255,255,255,255};

            nya_scene::shared_texture red_res;
            red_res.tex.build_texture(red_data,1,1,nya_render::texture::color_rgba);
            missing_red.create(red_res);

            nya_scene::shared_texture white_res;
            white_res.tex.build_texture(white_data,1,1,nya_render::texture::color_rgba);
            missing_white.create(white_res);

            initialised=true;
        }

        if((nya_system::get_time()/200)%2>0)
            return missing_white;
        */
        return missing_red;
    }
}

void material_internal::param_holder::apply_to_shader(const nya_scene::shader &shader,int uniform_idx) const
{
    if(p.is_valid())
    {
        if(m.is_valid())
            shader.internal().set_uniform_value(uniform_idx,p->f[0]*m->f[0],p->f[1]*m->f[1],p->f[2]*m->f[2],p->f[3]*m->f[3]);
        else
            shader.internal().set_uniform_value(uniform_idx,p->f[0],p->f[1],p->f[2],p->f[3]);
    }
    else
    {
        if(a.is_valid() && a->get_count()>0)
            shader.internal().set_uniform4_array(uniform_idx,a->m_params[0].f,a->get_count());
        else
            shader.internal().set_uniform_value(uniform_idx,0,0,0,0);
    }
}

void material_internal::skeleton_changed(const nya_render::skeleton *skeleton) const
{
    for(int i=0;i<(int)m_passes.size();++i)
        m_passes[i].m_shader.internal().skeleton_changed(skeleton);
}

bool material::load_text(shared_material &res,resource_data &data,const char* name)
{
    nya_formats::text_parser parser;
    parser.load_from_data((const char *)data.get_data(),data.get_size());
    for(int section_idx=0;section_idx<parser.get_sections_count();++section_idx)
    {
        const char *section_type=parser.get_section_type(section_idx);
        if(strcmp(section_type,"@pass")==0)
        {
            int pass_idx = res.add_pass(parser.get_section_name(section_idx));
            pass &p = res.get_pass(pass_idx);

            for(int subsection_idx=0;subsection_idx<parser.get_subsections_count(section_idx);++subsection_idx)
            {
                const char *subsection_type = parser.get_subsection_type(section_idx,subsection_idx);
                const char *subsection_value = parser.get_subsection_value(section_idx,subsection_idx);
                if(strcmp(subsection_type,"shader") == 0)
                {
                    if(!p.m_shader.load(subsection_value))
                        nya_log::log()<<"can't load shader when loding material '"<<name<<"'";
                }
                else if(strcmp(subsection_type,"blend")==0)
                    p.get_state().blend=nya_formats::blend_mode_from_string(subsection_value,p.get_state().blend_src,p.get_state().blend_dst);
                else if(strcmp(subsection_type,"zwrite")==0)
                    p.get_state().zwrite=nya_formats::bool_from_string(subsection_value);
                else if(strcmp(subsection_type,"cull")==0)
                    p.get_state().cull_face=nya_formats::cull_face_from_string(subsection_value,p.get_state().cull_order);
            }
        }
        else if(strcmp(section_type,"@texture")==0)
        {
            texture_proxy tex;
            tex.create();
            if(tex->load(parser.get_section_value(section_idx)))
            {
                const int texture_idx=res.get_texture_idx(parser.get_section_name(section_idx));
                if(texture_idx<0)
                {
                    material_internal::material_texture mat;
                    mat.semantics=parser.get_section_name(section_idx);
                    mat.proxy=tex;
                    res.m_textures.push_back(mat);
                }
                else
                    res.m_textures[texture_idx].proxy=tex;
            }
            else
                nya_log::log()<<"can't load texture when loading material "<<name<<"'";
        }
        else if(strcmp(section_type,"@param")==0)
        {
            material_internal::param_holder ph;
            ph.param_name=parser.get_section_name(section_idx);
            ph.p=param_proxy(material_internal::param(parser.get_section_value_vector(section_idx)));

            const int param_idx=res.get_param_idx(parser.get_section_name(section_idx));
            if(param_idx>=0)
                res.m_params[param_idx]=ph;
            else
                res.m_params.push_back(ph);
        }
        else
            nya_log::log()<<"unknown section when loading material '"<<name<<"'";
    }

    res.m_should_rebuild_passes_maps=true;
    return true;
}

void material_internal::set(const char *pass_name) const
{
    if(!pass_name)
        return;

    if(m_last_set_pass_idx>=0)
        unset();

    m_last_set_pass_idx=get_pass_idx(pass_name);
    if(m_last_set_pass_idx<0)
        return;

    update_passes_maps();
    const pass &p=m_passes[m_last_set_pass_idx];

    p.m_shader.internal().set();
    for(int uniform_idx=0;uniform_idx<(int)p.m_uniforms_idxs_map.size();++uniform_idx)
        m_params[p.m_uniforms_idxs_map[uniform_idx]].apply_to_shader(p.m_shader,uniform_idx);

    nya_render::set_state(p.m_render_state);

    for(int slot_idx=0;slot_idx<(int)p.m_textures_slots_map.size();++slot_idx)
    {
        int texture_idx=p.m_textures_slots_map[slot_idx];
        if(texture_idx>=0)
        {
            if(m_textures[texture_idx].proxy.is_valid())
            {
                if(!m_textures[texture_idx].proxy->internal().set(slot_idx))
                {
                    nya_log::warning()<<"invalid texture for semantics '"<<p.m_shader.internal().get_texture_semantics(slot_idx)<<"' in material '"<<m_name<<"\n";

                    missing_texture().internal().set(slot_idx);
                }
            }
            else
            {
                nya_log::warning()<<"invalid texture proxy for semantics '"<<p.m_shader.internal().get_texture_semantics(slot_idx)<<"' in material '"<<m_name<<"\n";

                missing_texture().internal().set(slot_idx);
            }
        }
        else
        {
            nya_log::warning()<<"missing texture for semantics '"<<p.m_shader.internal().get_texture_semantics(slot_idx)<<"' in material '"<<m_name<<"\n";

            missing_texture().internal().set(slot_idx);
        }
    }
}

void material_internal::unset() const
{
    if(m_last_set_pass_idx<0)
        return;

    const pass &p=m_passes[m_last_set_pass_idx];
    p.m_shader.internal().unset();

    if(p.m_render_state.blend)
        nya_render::blend::disable();

    if(!p.m_render_state.zwrite)
        nya_render::zwrite::enable();

    if(!p.m_render_state.color_write)
        nya_render::color_write::enable();

    for(int slot_idx=0;slot_idx<(int)p.m_textures_slots_map.size();++slot_idx)
    {
        const int texture_idx=(int)p.m_textures_slots_map[slot_idx];
        if(texture_idx>=0 && texture_idx<(int)m_textures.size() && m_textures[texture_idx].proxy.is_valid())
            m_textures[texture_idx].proxy->internal().unset();
    }

    m_last_set_pass_idx = -1;
}
    
int material_internal::get_param_idx(const char *name) const
{
    if(!name)
        return -1;

    update_passes_maps();
    for(int i=0;i<(int)m_params.size();++i)
    {
        if(m_params[i].param_name==name)
            return i;
    }

    return -1;
}

int material_internal::get_texture_idx(const char *semantics) const
{
    if(!semantics)
        return -1;

    for(int i=0;i<(int)m_textures.size();++i)
    {
        if(m_textures[i].semantics==semantics)
            return i;
    }

    return -1;
}

material_internal::pass &material_internal::pass::operator=(const pass &p)
{
    m_name=p.m_name;
    m_render_state=p.m_render_state;
    m_shader=p.m_shader;
    m_shader_changed=true;
    m_uniforms_idxs_map.clear();
    m_textures_slots_map.clear();
    return *this;
}

void material_internal::pass::set_shader(const nya_scene::shader &shader)
{
    m_shader=shader;
    m_shader_changed=true;
    m_uniforms_idxs_map.clear();
    m_textures_slots_map.clear();
}

void material_internal::pass::update_maps(const material_internal &m) const
{
    m_uniforms_idxs_map.resize(m_shader.internal().get_uniforms_count());
    std::fill(m_uniforms_idxs_map.begin(),m_uniforms_idxs_map.end(),0); // params should exists if idxs_map was rebuild properly
    for(int uniform_idx=0;uniform_idx<m_shader.internal().get_uniforms_count();++uniform_idx)
    {
        const std::string &name=m_shader.internal().get_uniform(uniform_idx).name;
        // don't use m.get_param_idx(name.c_str()) as it calls rebuil_maps
        int param_idx=-1;
        for(int i=0;i<(int)m.m_params.size();++i)
        {
            if(m.m_params[i].param_name==name)
            {
                param_idx=i;
                break;
            }
        }

        if(param_idx>=0)
            m_uniforms_idxs_map[uniform_idx]=param_idx;
    }

    m_textures_slots_map.resize(m_shader.internal().get_texture_slots_count());
    std::fill(m_textures_slots_map.begin(),m_textures_slots_map.end(),-1);
    for(int slot_idx=0;slot_idx<m_shader.internal().get_texture_slots_count();++slot_idx)
    {
        const std::string semantics=m_shader.internal().get_texture_semantics(slot_idx);
        int texture_idx=m.get_texture_idx(semantics.c_str());
        if(texture_idx>=0)
            m_textures_slots_map[slot_idx]=texture_idx;
    }
}

int material_internal::add_pass(const char *pass_name)
{
    if(!pass_name)
        return -1;

    for(std::vector<pass>::iterator iter=m_passes.begin();iter!=m_passes.end();++iter)
    {
        if(iter->m_name==pass_name)
            return (int)(iter - m_passes.begin());
    }

    m_passes.push_back(pass());
    m_passes.back().m_name.assign(pass_name);
    return (int)m_passes.size()-1;
}

int material_internal::get_pass_idx(const char *pass_name) const
{
    if(!pass_name)
        return -1;

    for(int i=0;i<(int)m_passes.size();++i)
        if(m_passes[i].m_name == pass_name)
            return i;

    return -1;
}

material_internal::pass &material_internal::get_pass(int idx)
{
    if(idx<0 || idx>=(int)m_passes.size())
        return nya_memory::get_invalid_object<pass>();

    return m_passes[idx];
}

const material_internal::pass &material_internal::get_pass(int idx) const
{
    if(idx<0 || idx>=(int)m_passes.size())
        return nya_memory::get_invalid_object<pass>();

    return m_passes[idx];
}

void material_internal::update_passes_maps() const
{
    if(!m_should_rebuild_passes_maps)
    {
        for(std::vector<pass>::const_iterator it=m_passes.begin();it!=m_passes.end();++it)
        {
            if(it->m_shader_changed)
            {
                m_should_rebuild_passes_maps=true;
                break;
            }
        }
    }

    if(!m_should_rebuild_passes_maps)
        return;

    // step 1: build params array
    // substep 1: build boolean map indicating used parameters and map of names of parameters to be added
    std::vector<bool> used_parameters(m_params.size());
    std::fill(used_parameters.begin(),used_parameters.end(),false);
    std::list<std::pair<std::string,nya_math::vec4> > parameters_to_add;
    for(int pass_idx=0;pass_idx<(int)m_passes.size();++pass_idx)
    {
        const nya_scene::shader &sh=m_passes[pass_idx].m_shader;
        for(int uniform_idx=0;uniform_idx<sh.internal().get_uniforms_count();++uniform_idx)
        {
            const std::string name = sh.internal().get_uniform(uniform_idx).name;
            //don't use get_param_idx_for_name(name.c_str()) as it calls rebuild_passes_map
            int param_idx=-1;
            for(int i=0;i<(int)m_params.size();++i)
            {
                if(m_params[i].param_name==name)
                {
                    param_idx=i;
                    break;
                }
            }

            if(param_idx>=0)
                used_parameters[param_idx]=true;
            else
                parameters_to_add.push_back(std::make_pair(name,sh.internal().get_uniform(uniform_idx).default_value));
        }
    }

    // delete unused parameters
    int param_idx = 0;
    for(std::vector<bool>::iterator iter=used_parameters.begin();iter!=used_parameters.end();++iter)
    {
        if(!*iter)
            m_params.erase(m_params.begin()+param_idx);
        else
            ++param_idx;
    }

    // add missing parameters
    for(std::list<std::pair<std::string,nya_math::vec4> >::iterator iter=parameters_to_add.begin();iter!=parameters_to_add.end();++iter)
    {
        m_params.push_back(param_holder());
        m_params.back().param_name=iter->first;
        nya_math::vec4 &v=iter->second;
        m_params.back().p=param_proxy(param());
        m_params.back().p->set(v.x,v.y,v.z,v.w);
    }

    std::map<std::string,bool> tex_semantics;

    for(std::vector<pass>::const_iterator iter=m_passes.begin();iter!=m_passes.end();++iter)
    {
        const nya_scene::shader_internal &s=iter->get_shader().internal();
        for(int i=0;i<s.get_texture_slots_count();++i)
            tex_semantics[s.get_texture_semantics(i)]=true;
    }

    for(std::map<std::string,bool>::const_iterator iter=tex_semantics.begin();iter!=tex_semantics.end();++iter)
    {
        if(get_texture_idx(iter->first.c_str())<0)
        {
            m_textures.push_back(material_texture());
            m_textures.back().semantics.assign(iter->first.c_str());
        }
    }

    int tex_count=(int)m_textures.size();
    for(int i=0;i<tex_count;++i)
    {
        if(tex_semantics.find(m_textures[i].semantics)!=tex_semantics.end())
            continue;

        m_textures.erase(m_textures.begin()+i);
        --i; --tex_count;
    }

    for(std::vector<pass>::const_iterator iter=m_passes.begin();iter!=m_passes.end();++iter)
        iter->update_maps(*this);

    for(std::vector<pass>::const_iterator it=m_passes.begin();it!=m_passes.end();++it)
        it->m_shader_changed=false;

    m_should_rebuild_passes_maps = false;
}

bool material_internal::release()
{
    m_passes.clear();
    m_textures.clear();
    m_params.clear();
    m_name.clear();
    m_should_rebuild_passes_maps = false;
    m_last_set_pass_idx = -1;

    return true;
}

bool material::load(const char *name)
{
    if(!m_internal.load(name))
        return false;

    if(!internal().m_shared.is_valid())
        return false;

    m_internal.m_last_set_pass_idx=-1;
    m_internal.m_should_rebuild_passes_maps=true;

    m_internal.m_name=m_internal.m_shared->m_name;
    m_internal.m_passes=m_internal.m_shared->m_passes;
    m_internal.m_params=m_internal.m_shared->m_params;
    m_internal.m_textures=m_internal.m_shared->m_textures;

    return true;
}

void material::set_texture(const char *semantics,const texture &tex)
{
    set_texture(semantics,texture_proxy(tex));
}

void material::set_texture(const char *semantics,const texture_proxy &proxy)
{
    if(!semantics || !semantics[0])
        return;

    int texture_idx=m_internal.get_texture_idx(semantics);
    if(texture_idx<0)
    {
        m_internal.m_textures.push_back(material_internal::material_texture());
        m_internal.m_textures.back().semantics.assign(semantics);
        m_internal.m_textures.back().proxy=proxy;
        m_internal.m_should_rebuild_passes_maps=true;
    }
    else
        m_internal.m_textures[texture_idx].proxy=proxy;
}

int material::get_textures_count() const
{
    m_internal.update_passes_maps();
    return (int)internal().m_textures.size();
}

const char *material::get_texture_semantics(int idx) const
{
    m_internal.update_passes_maps();
    if(idx < 0 || idx>=(int)internal().m_textures.size())
        return 0;

    return internal().m_textures[idx].semantics.c_str();
}

int material::get_texture_idx(const char *semantics) const
{
    m_internal.update_passes_maps();
    return m_internal.get_texture_idx(semantics);
}

const texture_proxy &material::get_texture(int idx) const
{
    if(idx < 0 || idx>=(int)internal().m_textures.size() )
        return nya_memory::get_invalid_object<texture_proxy>();

    return internal().m_textures[idx].proxy;
}

const texture_proxy &material::get_texture(const char *semantics) const
{
    return get_texture(get_texture_idx(semantics));
}

const char *material::get_param_name(int idx) const
{
    if(idx<0 || idx>=(int)internal().m_params.size())
        return 0;

    return m_internal.m_params[idx].param_name.c_str();
}

void material::set_param(int idx,float f0,float f1,float f2,float f3)
{
    set_param(idx,param(f0,f1,f2,f3));
}

void material::set_param(int idx,const param &p)
{
    set_param(idx,param_proxy(p));
}

void material::set_param(int idx,const param_proxy &p)
{
    if(idx<0 || idx>=(int)internal().m_params.size())
        return;

    m_internal.m_params[idx].p=p;
    m_internal.m_params[idx].m.free();
    m_internal.m_params[idx].a.free();
}

void material::set_param(int idx,const param_proxy &p,const param &m)
{
    set_param(idx,p,param_proxy(m));
}

void material::set_param(int idx,const param_proxy &p,const param_proxy &m)
{
    if(idx<0 || idx>=(int)internal().m_params.size())
        return;

    m_internal.m_params[idx].p=p;
    m_internal.m_params[idx].m=m;
    m_internal.m_params[idx].a.free();
}

void material::set_param_array(int idx,const param_array &a)
{
    set_param_array(idx,param_array_proxy(a));
}

void material::set_param_array(int idx,const param_array_proxy &p)
{
    if(idx<0 || idx>=(int)internal().m_params.size())
        return;

    m_internal.m_params[idx].p.free();
    m_internal.m_params[idx].m.free();
    m_internal.m_params[idx].a=p;
}

int material::get_params_count() const
{
    m_internal.update_passes_maps();
    return (int)m_internal.m_params.size();
}

const material::param_proxy &material::get_param(int idx) const
{
    if(idx<0 || idx>=(int)internal().m_params.size())
        return nya_memory::get_invalid_object<param_proxy>();

    return m_internal.m_params[idx].p;
}

const material::param_proxy &material::get_param_multiplier(int idx) const
{
    if(idx<0 || idx>=(int)internal().m_params.size())
        return nya_memory::get_invalid_object<param_proxy>();

    return internal().m_params[idx].m;
}

const material::param_array_proxy &material::get_param_array(int idx) const
{
    if(idx<0 || idx>=(int)internal().m_params.size())
        return nya_memory::get_invalid_object<param_array_proxy>();

    return internal().m_params[idx].a;
}

}
