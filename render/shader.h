//https://code.google.com/p/nya-engine/

#pragma once

#include "platform_specific_gl.h"

#ifdef OPENGL_ES
    #define GLhandleARB GLuint
#endif

#ifdef DIRECTX11
    #include <map>
#endif

#ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
    #define SUPPORT_OLD_SHADERS
#endif

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
        geometry,
        tesselation,
        program_types_count
    };

    bool add_program(program_type type,const char*code);

public:
    void bind() const;
    static void unbind();

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

#ifdef DIRECTX11
public:
    static ID3D11InputLayout *get_layout(const std::string &layout);
    static ID3D11InputLayout *add_layout(const std::string &layout,
                                         const D3D11_INPUT_ELEMENT_DESC*desc,size_t desc_size);

public:
    shader(): m_vertex(0),m_pixel(0) {}

private:
    compiled_shader m_compiled[program_types_count];
    ID3D11VertexShader *m_vertex;
    ID3D11PixelShader *m_pixel;

    typedef std::map<std::string,ID3D11InputLayout*> layouts_map;
    static layouts_map m_layouts;

    struct constants
    {
        mutable std::vector<float> buffer;
        bool mvp_matrix;
        bool mv_matrix;
        bool p_matrix;

        ID3D11Buffer *dx_buffer;

        constants():mvp_matrix(false),mv_matrix(false),p_matrix(false),dx_buffer(0){}
    };

    constants m_constants;
#else
public:
    shader(): m_program(0)
    {
        for(int i = 0;i<program_types_count;++i)
            m_objects[i]=0;

#ifdef SUPPORT_OLD_SHADERS
        m_mat_mvp=-1;
        m_mat_mv=-1;
        m_mat_p=-1;
#endif
    }

private:
    GLhandleARB m_program;
    GLhandleARB m_objects[program_types_count];
#endif

private:
    struct sampler
    {
        std::string name;
        unsigned int layer;

        sampler():layer(0){}
        sampler(const char *name,unsigned int layer):
                            name(name),layer(layer){}
    };

    std::vector<sampler> m_samplers;

#ifdef SUPPORT_OLD_SHADERS
private:
    int m_mat_mvp;
    int m_mat_mv;
    int m_mat_p;
#endif
};

}

