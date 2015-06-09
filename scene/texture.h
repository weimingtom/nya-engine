//https://code.google.com/p/nya-engine/

#pragma once

#include "shared_resources.h"
#include "render/texture.h"
#include "proxy.h"

namespace nya_scene
{

struct shared_texture
{
    nya_render::texture tex;

    bool release()
    {
        tex.release();
        return true;
    }
};

class texture_internal: public scene_shared<shared_texture>
{
    friend class texture;

public:
    bool set(int slot=0) const;
    void unset() const;

public:
    texture_internal():m_last_slot(0) {}

private:
    mutable int m_last_slot;
};

class texture
{
public:
    bool load(const char *name) { return m_internal.load(name); }
    void unload() { return m_internal.unload(); }

public:
    void create(const shared_texture &res) { m_internal.create(res); }

public:
    static void set_resources_prefix(const char *prefix) { texture_internal::set_resources_prefix(prefix); }
    static void register_load_function(texture_internal::load_function function,bool clear_default=true) { texture_internal::register_load_function(function,clear_default); }

public:
    typedef nya_render::texture::color_format color_format;

    const char *get_name() const { return internal().get_name(); }
    unsigned int get_width() const;
    unsigned int get_height() const;
    color_format get_format() const;
    bool is_cubemap() const;

public:
    bool build(const void *data,unsigned int width,unsigned int height,color_format format);
    bool update_region(const void *data,unsigned int x,unsigned int y,unsigned int width,unsigned int height);

public:
    texture() { texture_internal::default_load_function(load_tga);
                texture_internal::default_load_function(load_dds);
                texture_internal::default_load_function(load_ktx); }

    texture(const char *name) { *this=texture(); load(name); }

public:
    static bool load_tga(shared_texture &res,resource_data &data,const char* name);
    static bool load_dds(shared_texture &res,resource_data &data,const char* name);
    static bool load_ktx(shared_texture &res,resource_data &data,const char* name);

    static void set_load_dds_flip(bool flip) { m_load_dds_flip=flip; }

public:
    const texture_internal &internal() const { return m_internal; }

private:
    texture_internal m_internal;
    static bool m_load_dds_flip;
};

typedef proxy<texture> texture_proxy;

}
