//https://code.google.com/p/nya-engine/

#include "shader.h"
#include "scene.h"

namespace nya_scene
{

bool shader::load_nya_shader(shared_shader &res,size_t data_size,const void*data)
{
    const char *text=(const char*)data;

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
                break;

            default:
                get_log()<<"scene shader load warning: unsupported shader tag\n";
            break;
        }
    }

    if(res.vertex.empty())
    {
        get_log()<<"scene shader load error: empty vertex shader\n";
        return false;
    }

    if(res.pixel.empty())
    {
        get_log()<<"scene shader load error: empty pixel shader\n";
        return false;
    }

    //get_log()<<"vertex <"<<res.vertex.c_str()<<">\n";
    //get_log()<<"pixel <"<<res.pixel.c_str()<<">\n";

    res.shdr.add_program(nya_render::shader::vertex,res.vertex.c_str());
    res.shdr.add_program(nya_render::shader::pixel,res.pixel.c_str());

    return true;
}

void shader::set()
{
    if(!m_shared.is_valid())
        return;

    m_shared->shdr.bind();
}

void shader::unset()
{
    if(!m_shared.is_valid())
        return;

    m_shared->shdr.unbind();
}

}
