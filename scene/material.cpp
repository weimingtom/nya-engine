//https://code.google.com/p/nya-engine/

#include "material.h"

namespace nya_scene
{

void material::set()
{
    m_shader.set();
}

void material::unset()
{
    m_shader.set();
}

bool material::set_shader(const shader &shdr) 
{
    //m_shader.unload(); //todo: ref count and unload

    m_shader=shdr;
    return true;
}

}
