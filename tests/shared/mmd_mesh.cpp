//https://code.google.com/p/nya-engine/

#include "mmd_mesh.h"

#include "load_pmd.h"
#include "load_pmx.h"

bool mmd_mesh::load(const char *name)
{
    nya_scene::mesh::register_load_function(pmx_loader::load);
    nya_scene::mesh::register_load_function(pmd_loader::load);
    return m_mesh.load(name);
}

void mmd_mesh::update(unsigned int dt) { m_mesh.update(dt); }
void mmd_mesh::draw() { m_mesh.draw(); }
