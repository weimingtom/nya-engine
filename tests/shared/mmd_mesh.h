//https://code.google.com/p/nya-engine/

#include "scene/mesh.h"

class mmd_mesh
{
public:
    bool load(const char *name);
    void unload() { m_mesh.unload(); }

    void set_anim(const nya_scene::animation &anim,int layer=0) { m_mesh.set_anim(anim,layer); }
    void set_anim(const nya_scene::animation_proxy & anim,int layer=0) { m_mesh.set_anim(anim,layer); }
    const nya_scene::animation_proxy & get_anim(int layer=0) const { return m_mesh.get_anim(layer); }

    void update(unsigned int dt);
    void draw();

    void set_pos(const nya_math::vec3 &pos) { m_mesh.set_pos(pos); }
    void set_rot(float yaw,float pitch,float roll) { m_mesh.set_rot(yaw,pitch,roll); }
    void set_rot(const nya_math::quat &rot) { m_mesh.set_rot(rot); }
    void set_scale(float scale) { m_mesh.set_scale(scale); }

private:
    nya_scene::mesh m_mesh;
    //nya_render::vbo m_vbo;
};
