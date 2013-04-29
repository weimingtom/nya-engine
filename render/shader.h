//https://code.google.com/p/nya-engine/

#pragma once

#include <string>
#include <vector>

namespace nya_render
{

class shader
{
public:
    enum program_type
    {
        vertex,
        pixel,
        //geometry,
        //tesselation,
        program_types_count
    };

    bool add_program(program_type type,const char*code);

public:
    void bind() const;
    static void unbind();

    static void apply();

public:
    void set_sampler(const char*name,unsigned int layer);

public:
    int get_handler(const char*name) const;
    void set_uniform(unsigned int i,float f0,float f1=0.0f,float f2=0.0f,float f3=0.0f) const;
    void set_uniform3_array(unsigned int i,const float *f,unsigned int count) const;
    void set_uniform4_array(unsigned int i,const float *f,unsigned int count) const;
    void set_uniform16_array(unsigned int i,const float *f,unsigned int count,bool transpose=false) const;

public:
    void release();

public:
    shader(): m_shdr(-1) {}

private:
    int m_shdr;

    struct sampler
    {
        std::string name;
        unsigned int layer;

        sampler():layer(0){}
        sampler(const char *name,unsigned int layer):
                            name(name),layer(layer){}
    };

    std::vector<sampler> m_samplers;
};

class compiled_shader
{
public:
    void *get_data() { if(m_data.empty()) return 0; return &m_data[0]; }
    const void *get_data() const { if(m_data.empty()) return 0; return &m_data[0]; }
    size_t get_size() const { return m_data.size(); }

public:
    compiled_shader() {}
    compiled_shader(size_t size) { m_data.resize(size); }

private:
    std::vector<char> m_data;
};

class compiled_shaders_provider
{
public:
    virtual bool get(const char *text,compiled_shader &shader) { return 0; }
    virtual bool set(const char *text,const compiled_shader &shader) { return false; }
};

void set_compiled_shaders_provider(compiled_shaders_provider *provider);

}
