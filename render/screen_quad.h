//https://code.google.com/p/nya-engine/

#pragma once

#include "render/vbo.h"

namespace nya_render
{

class screen_quad
{
public:
    void init()
    {
        struct { float x,y,s,t; } verts[4];
        for(int i=0;i<4;++i)
        {
            verts[i].x=i>1?-1.0:1.0,verts[i].y=i%2?1.0:-1.0;
            verts[i].s=i>1? 0.0:1.0,verts[i].t=i%2?1.0:0.0;
        }

        m_mesh.set_vertex_data(verts,sizeof(verts[0]),4);
        m_mesh.set_vertices(0,2);
        m_mesh.set_tc(0,2*4,2);
        m_mesh.set_element_type(nya_render::vbo::triangle_strip);
    }

    void draw() { m_mesh.bind(); m_mesh.draw(); m_mesh.unbind(); }
    void release() { m_mesh.release(); }

private:
    nya_render::vbo m_mesh;
};

}
