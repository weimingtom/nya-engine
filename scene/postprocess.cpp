//https://code.google.com/p/nya-engine/

#include "postprocess.h"

namespace nya_scene
{

void postprocess::resize(unsigned int width,unsigned int height)
{
    m_width=width,m_height=height;
    update_targets();
}

void postprocess::draw(int dt)
{
    //ToDo
}

bool postprocess::load_text(shared_postprocess &res,resource_data &data,const char* name)
{
    //ToDo

    return false;
}

void postprocess::update_targets()
{
    //ToDo
}

}
