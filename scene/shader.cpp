//https://code.google.com/p/nya-engine/

#include "shader.h"

namespace nya_scene
{

bool shader::load_nya_shader(shared_shader &res,size_t data_size,const void*data)
{
    const char *vs = 
    "varying vec4 colorVarying;"

    "uniform vec4 translate;"

    "void main()"
    "{ gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;"// gl_Position.y += sin(translate.x) / 2.0; gl_Position.xyz*=0.3;"
    "  colorVarying = gl_Normal*0.5+vec4(0.5); gl_MultiTexCoord0; gl_MultiTexCoord5; gl_MultiTexCoord14; gl_Normal; }";

    //const char *vs = "void main() { gl_Position=vec4(0.0,0.0,0.0,1.0); }";

    res.shdr.add_program(nya_render::shader::vertex,vs);

    const char *ps = 
    "varying lowp vec4 colorVarying;"
    "void main() { gl_FragColor = colorVarying*1.4; }";
    
    //        const char *ps = "void main() { gl_FragColor=vec4(1.0,0.0,0.0,1.0); }";

    res.shdr.add_program(nya_render::shader::pixel,ps);

    printf("shader loaded\n");

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
