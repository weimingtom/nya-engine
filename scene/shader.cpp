//https://code.google.com/p/nya-engine/

#include "shader.h"

namespace nya_scene
{

bool shader::load_nya_shader(shared_shader &res,size_t data_size,const void*data)
{
    const char *vs = 
    "varying vec4 colorVarying;"
    "void main()"
    "{ "
	"colorVarying = vec4(gl_Normal.xyz*0.5+vec3(0.5),1.0);"// gl_MultiTexCoord0; gl_MultiTexCoord5; gl_MultiTexCoord14; gl_Normal;"
	"gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;}";

    res.shdr.add_program(nya_render::shader::vertex,vs);

    const char *ps = 
#ifdef OPENGL_ES
    "precision mediump float;\n"
#endif
    "varying vec4 colorVarying;"
    "void main() { gl_FragColor = colorVarying; }";

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
