//https://code.google.com/p/nya-engine/

#include "shader.h"
#include "scene.h"

namespace nya_scene
{

nya_math::vec3 shader::predefined::camera_local_pos;

bool shader::load_nya_shader(shared_shader &res,size_t data_size,const void*data,const char* name)
{
    const char *text=(const char*)data;

    std::map<std::string,std::string> samplers;

	std::string predef_cam_local_pos;

    for(size_t i=0;i<data_size-1;++i)
    {
        if(text[i]!='@')
            continue;

        ++i;

        switch (text[i]) 
        {
            case 'a':
                if(i+3<data_size && strncmp(&text[i],"all",3)==0)
                {
                    size_t begin=i+3;
                    for(i=begin;i<data_size;++i)
                        if(text[i]=='@')
                            break;

                    res.vertex.append(&text[begin],i-begin);
                    res.pixel.append(&text[begin],i-begin);
                    --i;
                }
                break;
                
            case 's':
                if(i+8<data_size && strncmp(&text[i],"sampler",7)==0)
                {
                    size_t begin=i+8;
                    for(i=begin;i<data_size;++i)
                        if(text[i]==' ' || text[i]=='\t')
                            break;

                    std::string sampler_name(&text[begin],i-begin);

                    while(i<data_size && (text[i]==' ' || text[i]=='\t')) ++i;

                    for(begin=i;i<data_size;++i)
                        if(text[i]=='\n' || text[i]=='\r'
                           || text[i]==' ' || text[i]=='\t')
                            break;

                    std::string sampler_semantics=std::string(&text[begin],i-begin);
                    samplers[sampler_semantics]=sampler_name;
                    res.samplers[sampler_semantics]=-1;
                    --i;
                }
                break;

            case 'v':
                if(i+6<data_size && strncmp(&text[i],"vertex",6)==0)
                {
                    size_t begin=i+6;
                    for(i=begin;i<data_size;++i)
                        if(text[i]=='@')
                            break;

                    res.vertex.append(&text[begin],i-begin);
                    --i;
                }
            break;

            case 'f':
                if(i+8<data_size && strncmp(&text[i],"fragment",8)==0)
                {
                    size_t begin=i+8;
                    for(i=begin;i<data_size;++i)
                        if(text[i]=='@')
                            break;

                    res.pixel.append(&text[begin],i-begin);
                    --i;
                }
            break;

            case 'p':
                if(i+5<data_size && strncmp(&text[i],"pixel",5)==0)
                {
                    size_t begin=i+5;
                    for(i=begin;i<data_size;++i)
                        if(text[i]=='@')
                            break;

                    res.pixel.append(&text[begin],i-begin);
                    --i;
                }
                else if(i+11<data_size && strncmp(&text[i],"predefined",10)==0)
                {
                    size_t begin=i+11;
                    for(i=begin;i<data_size;++i)
                        if(text[i]==' ' || text[i]=='\t')
                            break;

                    std::string predef_name(&text[begin],i-begin);

                    while(i<data_size && (text[i]==' ' || text[i]=='\t')) ++i;

                    for(begin=i;i<data_size;++i)
                        if(text[i]=='\n' || text[i]=='\r'
                           || text[i]==' ' || text[i]=='\t')
                            break;

                    std::string predef_semantics=std::string(&text[begin],i-begin);
					if(predef_semantics=="nya_camera_local_pos")
						predef_cam_local_pos=predef_name;
                    --i;
                }
                break;

            default:
                get_log()<<"scene shader load warning: unsupported shader tag in "<<name<<"\n";
            break;
        }
    }

    if(res.vertex.empty())
    {
        get_log()<<"scene shader load error: empty vertex shader in "<<name<<"\n";
        return false;
    }

    if(res.pixel.empty())
    {
        get_log()<<"scene shader load error: empty pixel shader in "<<name<<"\n";
        return false;
    }

    //get_log()<<"vertex <"<<res.vertex.c_str()<<">\n";
    //get_log()<<"pixel <"<<res.pixel.c_str()<<">\n";
    res.samplers_count=0;
    for(shared_shader::samplers_map::iterator it=res.samplers.begin();
        it!=res.samplers.end();++it)
    {
        res.shdr.set_sampler(samplers[it->first].c_str(),res.samplers_count);
        it->second=res.samplers_count;
        ++res.samplers_count;
    }

    res.shdr.add_program(nya_render::shader::vertex,res.vertex.c_str());
    res.shdr.add_program(nya_render::shader::pixel,res.pixel.c_str());

	if(!predef_cam_local_pos.empty())
		res.predef_camera_local_pos=res.shdr.get_handler(predef_cam_local_pos.c_str());

    return true;
}

void shader::set() const
{
    if(!m_shared.is_valid())
        return;

    m_shared->shdr.bind();

	if(m_shared->predef_camera_local_pos>=0)
	{
		m_shared->shdr.set_uniform(m_shared->predef_camera_local_pos,predefined::camera_local_pos.x,
								   predefined::camera_local_pos.y,predefined::camera_local_pos.z);
	}
}

void shader::unset() const
{
    if(!m_shared.is_valid())
        return;

    m_shared->shdr.unbind();
}

int shader::get_texture_slot(const char *semantics) const
{
    if(!semantics || !m_shared.is_valid())
        return -1;

    shared_shader::samplers_map::const_iterator it=m_shared->samplers.find(semantics);
    if(it==m_shared->samplers.end())
        return -1;

    return it->second;
}

int shader::get_texture_slots_count() const
{
    if(!m_shared.is_valid())
        return 0;

    return m_shared->samplers_count;
}

}
