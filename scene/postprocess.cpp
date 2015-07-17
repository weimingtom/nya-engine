//https://code.google.com/p/nya-engine/

#include "postprocess.h"
#include "formats/text_parser.h"
#include "formats/string_convert.h"
#include "formats/math_expr_parser.h"
#include "memory/invalid_object.h"
#include "scene.h"
#include <string.h>

#ifdef min
    #undef min
    #undef max
#endif

namespace nya_scene
{

bool postprocess::load(const char *name)
{
    unload();
    if(!scene_shared::load(name))
        return false;

    m_quad=nya_memory::shared_ptr<nya_render::screen_quad>(nya_render::screen_quad());
    m_quad->init();
    update();
    return true;
}

void postprocess::resize(unsigned int width,unsigned int height)
{
    m_width=width,m_height=height;
    update();
}

void postprocess::draw(int dt)
{
    nya_render::rect prev_rect=nya_render::get_viewport();

    nya_render::state state;
    std::vector<size_t> textures_set;
    for(size_t i=0;i<m_op.size();++i)
    {
        const size_t idx=m_op[i].idx;
        switch(m_op[i].type)
        {
            case type_set_shader:
                m_op_set_shader[idx].sh.internal().set();
                for(int j=0;j<(int)m_op_set_shader[idx].params_map.size();++j)
                {
                    const int param_idx=m_op_set_shader[idx].params_map[j];
                    if(param_idx<0)
                        continue;

                    const nya_math::vec4 &v=m_shader_params[param_idx].second;
                    m_op_set_shader[idx].sh.internal().set_uniform_value(j,v.x,v.y,v.z,v.w);
                }
                break;

            case type_set_target:
                {
                    op_target &t=m_targets[idx];
                    nya_render::set_viewport(t.rect);

                    if(t.color_idx>=0)
                    {
                        nya_scene::texture_proxy &tp=m_textures[t.color_idx].second.tex;
                        if(tp.is_valid() && tp->internal().get_shared_data().is_valid())
                            t.fbo->set_color_target(tp->internal().get_shared_data()->tex,0,t.samples);
                    }

                    if(t.depth_idx>=0)
                    {
                        nya_scene::texture_proxy &tp=m_textures[t.depth_idx].second.tex;
                        if(tp.is_valid() && tp->internal().get_shared_data().is_valid())
                            t.fbo->set_depth_target(tp->internal().get_shared_data()->tex);
                    }

                    t.fbo->bind();
                }
                break;

            case type_set_texture:
                {
                    const size_t tex_idx=m_op_set_texture[idx].tex_idx;
                    textures_set.push_back(tex_idx);
                    const texture_proxy &t=m_textures[tex_idx].second.tex;
                    if(t.is_valid())
                        t->internal().set(m_op_set_texture[idx].layer);
                }
                break;

            case type_clear:
                nya_render::clear(true,true); //ToDo: clear separately
                break;

            case type_draw_scene:
                draw_scene(m_op_draw_scene[idx].pass.c_str(),m_op_draw_scene[idx].tags.c_str());
                break;

            case type_draw_quad:
                nya_render::set_state(state);
                m_quad->draw();
                break;
        }
    }

    nya_render::set_viewport(prev_rect);
    shader_internal::unset();
    nya_render::fbo::unbind();
    for(size_t i=0;i<textures_set.size();++i)
    {
        const texture_proxy &t=m_textures[textures_set[i]].second.tex;
        if(t.is_valid())
            t->internal().unset();
    }
}

bool postprocess::load_text(shared_postprocess &res,resource_data &data,const char* name)
{
    nya_formats::text_parser parser;
    if(!parser.load_from_data((char *)data.get_data(),data.get_size()))
        return false;

    res.lines.resize(parser.get_sections_count());
    for(int i=0;i<parser.get_sections_count();++i)
    {
        shared_postprocess::line &l=res.lines[i];
        l.type.assign(parser.get_section_type(i)+1);
        const char *name=parser.get_section_name(i);
        l.name.assign(name?name:"");

        l.values.resize(parser.get_subsections_count(i));
        for(int j=0;j<parser.get_subsections_count(i);++j)
        {
            l.values[j].first=parser.get_subsection_type(i,j);
            const char *value=parser.get_subsection_value(i,j);
            l.values[j].second=value?value:"";
        }
    }

    return true;
}

template<typename t> bool set_value(std::vector<std::pair<std::string, t> > &vec,const char *name,const t &value)
{
    if(!name)
        return false;

    for(int i=0;i<(int)vec.size();++i)
    {
        if(vec[i].first!=name)
            continue;

        vec[i].second=value;
        return true;
    }

    vec.push_back(std::make_pair(name,value));
    return true;
}

template<typename t> int get_idx(const std::vector<std::pair<std::string, t> > &vec,const char *name)
{
    if(!name)
        return -1;

    for(int i=0;i<(int)vec.size();++i)
    {
        if(vec[i].first==name)
            return i;
    }

    return -1;
}

void postprocess::set_condition(const char *condition,bool value) { if(set_value(m_conditions,condition,value)) update(); }
bool postprocess::get_condition(const char *condition) const
{
    const int i=get_idx(m_conditions,condition);
    return i<0?false:m_conditions[i].second;
}

void postprocess::set_variable(const char *name,float value) { if(set_value(m_variables,name,value)) update(); }
float postprocess::get_variable(const char *name) const
{
    const int i=get_idx(m_variables,name);
    return i<0?0.0f:m_variables[i].second;
}

void postprocess::set_texture(const char *name,const texture_proxy &tex)
{
    set_value(m_textures,name,tex_holder(true,tex));
}

const texture_proxy &postprocess::get_texture(const char *name) const
{
    const int i=get_idx(m_textures,name);
    return i<0?nya_memory::get_invalid_object<texture_proxy>():m_textures[i].second.tex;
}

void postprocess::set_shader_param(const char *name,const nya_math::vec4 &value)
{
    if(!name)
        return;

    const int i=get_idx(m_shader_params,name);
    if(i>=0)
    {
        m_shader_params[i].second=value;
        update_shader_param(i);
        return;
    }

    const int idx=(int)m_shader_params.size();
    m_shader_params.push_back(std::make_pair(name,value));
    update_shader_param(idx);
}

void postprocess::update_shader_param(int param_idx)
{
    if(param_idx<0 || param_idx>=(int)m_shader_params.size())
        return;

    for(size_t i=0;i<m_op_set_shader.size();++i)
    {
        int idx= -1;
        for(int j=0;j<m_op_set_shader[i].sh.internal().get_uniforms_count();++j)
        {
            if(m_op_set_shader[i].sh.internal().get_uniform(j).name!=m_shader_params[param_idx].first)
                continue;

            idx=j;
            break;
        }

        if(idx<0)
            continue;

        m_op_set_shader[i].params_map.resize(m_op_set_shader[i].sh.internal().get_uniforms_count(),-1);
        m_op_set_shader[i].params_map[idx]=param_idx;
    }
}

const nya_math::vec4 &postprocess::get_shader_param(const char *name) const
{
    const int i=get_idx(m_shader_params,name);
    return i<0?nya_memory::get_invalid_object<nya_math::vec4>():m_shader_params[i].second;
}

template <typename op_t,typename t,typename op_e> t &add_op(op_t &ops,std::vector<t> &spec_ops,op_e type)
{
    ops.resize(ops.size()+1);
    ops.back().type=type;
    ops.back().idx=spec_ops.size();
    spec_ops.resize(spec_ops.size()+1);
    return spec_ops.back();
}

void postprocess::update()
{
    clear_ops();

    if(!m_shared.is_valid())
        return;

    std::vector<std::pair<bool,bool> > ifs;
    m_targets.resize(1);
    m_targets.back().rect.width=m_width,m_targets.back().rect.height=m_height;
    m_targets.back().fbo=nya_memory::shared_ptr<nya_render::fbo>(nya_render::fbo());
    typedef std::map<std::string,size_t> string_map;
    string_map targets;
    string_map current_tex;

    for(int i=(int)m_textures.size()-1;i>=0;--i)
    {
        if(m_textures[i].second.user_set)
            continue;

        m_textures.erase(m_textures.begin()+i);
    }

    //ToDo: don't create unused targets

    for(int i=0;i<(int)m_shared->lines.size();++i)
    {
        const shared_postprocess::line &l=m_shared->lines[i];

        if(l.type=="if")
        {
            ifs.resize(ifs.size()+1);
            ifs.back().first=ifs.back().second=get_condition(l.name.c_str());
            continue;
        }
        else if(l.type=="else")
        {
            if(ifs.empty())
            {
                log()<<"warning: postprocess: syntax error - else without if in file "<<m_shared.get_name()<<"\n";
                return;
            }

            ifs.back().second=!ifs.back().first;
            continue;
        }
        else if(l.type=="elif" || l.type=="else_if")
        {
            if(ifs.empty())
            {
                log()<<"warning: postprocess: syntax error - else without if in file "<<m_shared.get_name()<<"\n";
                return;
            }

            if(ifs.back().first)
            {
                ifs.back().second=false;
                continue;
            }

            ifs.back().second=get_condition(l.name.c_str());
            if(ifs.back().second)
                ifs.back().first=true;
            continue;
        }
        else if(l.type=="end")
        {
            if(ifs.empty())
            {
                log()<<"warning: postprocess: syntax error - end without if in file "<<m_shared.get_name()<<"\n";
                return;
            }
            else
                ifs.pop_back();
            continue;
        }

        bool should_contiue=false;
        for(int j=0;j<(int)ifs.size();++j)
        {
            if(!ifs[j].second)
            {
                should_contiue=true;
                break;
            }
        }

        if(should_contiue)
            continue;

        if(l.type=="target")
        {
            const char *color=l.get_value("color"),*depth=l.get_value("depth");
            const char *width=l.get_value("width"),*height=l.get_value("height");
            const char *samples=l.get_value("samples");

            nya_formats::math_expr_parser p;
            p.set_var("screen_width",float(m_width));
            p.set_var("screen_height",float(m_height));
            for(size_t i=0;i<m_variables.size();++i)
                p.set_var(m_variables[i].first.c_str(),m_variables[i].second);

            const unsigned int w=p.parse(width)?(unsigned int)p.calculate():m_width;
            const unsigned int h=p.parse(height)?(unsigned int)p.calculate():m_height;
            const unsigned int s=p.parse(samples)?(unsigned int)p.calculate():1;

            if(targets.find(l.name)==targets.end())
            {
                targets[l.name]=m_targets.size();
                m_targets.resize(m_targets.size()+1);
                m_targets.back().rect.width=w,m_targets.back().rect.height=h;
                m_targets.back().fbo=nya_memory::shared_ptr<nya_render::fbo>(nya_render::fbo());
            }
            else
            {
                log()<<"warning: postprocess: target "<<l.name<<" redifinition in file "<<m_shared.get_name()<<"\n";
                continue;
            }

            if(!w || !h)
                continue;

            if(color)
            {
                nya_render::texture::color_format f=nya_render::texture::color_rgba;
                const char *format=l.get_value("color_format");
                if(format)
                {
                    if(strcmp(format,"rgba")==0 || strcmp(format,"rgba8")==0)
                        f=nya_render::texture::color_rgba;
                    else if(strcmp(format,"rgb")==0 || strcmp(format,"rgb8")==0)
                        f=nya_render::texture::color_rgb;
                    else if(strcmp(format,"rgba32f")==0)
                        f=nya_render::texture::color_rgba32f;
                    else if(strcmp(format,"rgb32f")==0)
                        f=nya_render::texture::color_rgb32f;
                    else
                    {
                        log()<<"warning: postprocess: invalid texture "<<color<<" format"<<format<<"in file "<<m_shared.get_name()<<"\n";
                        log()<<"available formats: rgba, rgb, rgba32f, rgb32f\n";
                    }
                }

                texture_proxy t=get_texture(color);
                if(t.is_valid())
                {
                    if(t->get_width()!=w || t->get_height()!=h)
                        log()<<"warning: postprocess: texture "<<color<<" with different size in file "<<m_shared.get_name()<<"\n";
                    if(t->get_format()!=f)
                        log()<<"warning: postprocess: texture "<<color<<" with different format in file "<<m_shared.get_name()<<"\n";
                }
                else
                {
                    const char *init_color=l.get_value("init_color");
                    nya_memory::tmp_buffer_ref color_buf;
                    if(init_color)
                    {
                        typedef unsigned char uchar;
                        nya_math::vec4 cf=nya_formats::vec4_from_string(init_color);
                        nya_math::vec4 cfc=cf.min(cf.max(cf, nya_math::vec4(0.0,0.0,0.0,0.0)), nya_math::vec4(1.0,1.0,1.0,1.0))*255.0;
                        uchar c[4]={uchar(cfc.x),uchar(cfc.y),uchar(cfc.z),uchar(cfc.w)};
                        switch(f)
                        {
                            case nya_render::texture::color_rgba:
                            case nya_render::texture::color_rgb:
                            {
                                unsigned int bpp=f==nya_render::texture::color_rgba?4:3;
                                color_buf.allocate(w*h*bpp);
                                for(unsigned int i=0;i<w*h*bpp;i+=bpp)
                                    memcpy(color_buf.get_data(i),c,bpp);
                            }
                            break;

                            case nya_render::texture::color_rgba32f:
                            case nya_render::texture::color_rgb32f:
                            {
                                unsigned int bpp=f==nya_render::texture::color_rgba32f?4*4:3*4;
                                color_buf.allocate(w*h*bpp);
                                for(unsigned int i=0;i<w*h*bpp;i+=bpp)
                                    memcpy(color_buf.get_data(i),&cf,bpp);
                                break;
                            }

                            default:
                                log()<<"warning: postprocess: texture "<<color<<" invalid color format initialisation "<<m_shared.get_name()<<"\n";
                        }
                    }

                    t.create();
                    t->build(color_buf.get_data(),w,h,f);
                    color_buf.free();

                    m_textures.push_back(std::make_pair(color,tex_holder(false,t)));
                }

                m_targets.back().color_idx=get_idx(m_textures, color);
                m_targets.back().samples=s;
            }

            if(depth)
            {
                nya_render::texture::color_format f=nya_render::texture::depth16;
                const char *format=l.get_value("depth_format");
                if(format)
                {
                    if(strcmp(format,"depth16")==0)
                        f=nya_render::texture::depth16;
                    else if(strcmp(format,"depth24")==0)
                        f=nya_render::texture::depth24;
                    else if(strcmp(format,"depth32")==0)
                        f=nya_render::texture::depth32;
                    else
                    {
                        log()<<"warning: postprocess: invalid texture "<<depth<<" format"<<format<<"in file "<<m_shared.get_name()<<"\n";
                        log()<<"available formats: depth16, depth24, depth32\n";
                    }
                }

                texture_proxy t=get_texture(depth);
                if(t.is_valid())
                {
                    if(t->get_width()!=w || t->get_height()!=h)
                        log()<<"warning: postprocess: texture "<<depth<<" with different size in file "<<m_shared.get_name()<<"\n";
                    if(t->get_format()!=f)
                        log()<<"warning: postprocess: texture "<<depth<<" with different format in file "<<m_shared.get_name()<<"\n";
                }
                else
                {
                    t.create();
                    t->build(0,w,h,f);
                    m_textures.push_back(std::make_pair(depth,tex_holder(false,t)));
                }

                m_targets.back().depth_idx=get_idx(m_textures, depth);
            }
        }
        else if(l.type=="set_shader")
        {
            op_set_shader &o=add_op(m_op,m_op_set_shader,type_set_shader);
            o.sh.load(l.name.c_str());
        }
        else if(l.type=="set_target")
        {
            m_op.resize(m_op.size()+1);
            m_op.back().type=type_set_target;
            string_map::const_iterator it=targets.find(l.name);
            m_op.back().idx=it==targets.end()?0:it->second;
        }
        else if(l.type=="set_texture")
        {
            if(l.values.empty())
                continue;

            int idx=get_idx(m_textures,l.values.front().first.c_str());
            if(idx<0)
            {
                idx=(int)m_textures.size();
                m_textures.push_back(std::make_pair(l.values.front().first,tex_holder(false,texture_proxy())));
            }

            current_tex[l.name]=idx;
        }
        else if(l.type=="clear")
        {
            m_op.resize(m_op.size()+1);
            m_op.back().type=type_clear;
        }
        else if(l.type=="draw_scene")
        {
            op_draw_scene &o=add_op(m_op,m_op_draw_scene,type_draw_scene);
            o.pass=l.name;
            if(l.values.empty())
                continue;

            o.tags=l.values.front().first;
        }
        else if(l.type=="draw_quad")
        {
            if(m_op_set_shader.empty())
                continue;

            for(string_map::const_iterator it=current_tex.begin();it!=current_tex.end();++it)
            {
                const int idx=m_op_set_shader.back().sh.internal().get_texture_slot(it->first.c_str());
                if(idx<0)
                    continue;

                op_set_texture &o=add_op(m_op,m_op_set_texture,type_set_texture);
                o.tex_idx=it->second;
                o.layer=idx;
            }

            m_op.resize(m_op.size()+1);
            m_op.back().type=type_draw_quad;
        }
        else
            log()<<"warning: postprocess: unknown operation "<<l.type<<" in file "<<m_shared.get_name()<<"\n";
    }

    for(int i=0;i<(int)m_shader_params.size();++i)
        update_shader_param(i);
}

void postprocess::unload()
{
    if(m_quad.get_ref_count()==1)
        m_quad->release();
    m_quad.free();
    scene_shared::unload();
    m_textures.clear();
    clear_ops();
}

void postprocess::clear_ops()
{
    m_op.clear();
    m_op_draw_scene.clear();
    m_op_set_shader.clear();

    for(size_t i=0;i<m_targets.size();++i)
    {
        if(m_targets[i].fbo.get_ref_count()==1)
            m_targets[i].fbo->release();
    }

    m_targets.clear();
}

const char *shared_postprocess::line::get_value(const char *name) const
{
    const int i=get_idx(values,name);
    return i<0?0:values[i].second.c_str();
}

}
