//https://code.google.com/p/nya-engine/

#pragma once

#include <string>
#include <vector>

namespace nya_render
{

class shader_code_parser
{
public:
    const char *get_code() { return m_code.c_str(); }

public:
    bool convert_to_hlsl();
    bool convert_to_modern_glsl();

public:
    enum variable_type
    {
        type_invalid,
        type_float,
        type_vec3,
        type_vec4,
        type_mat4,
        type_sampler2d,
        type_sampler_cube
    };

    struct variable
    {
        variable_type type;
        std::string name;
        unsigned int array_size;

        variable():type(type_invalid),array_size(0){}
    };

    int get_uniforms_count();
    variable get_uniform(int idx);

    int get_attributes_count();
    variable get_attribute(int idx);

public:
    shader_code_parser(const char *text,const char *replace_prefix_str="nya"):
                       m_code(text?text:""),m_replace_str(replace_prefix_str?replace_prefix_str:"") {}
private:
    void remove_comments();

    bool parse_uniforms(bool remove);
    bool parse_attributes(const char *replace_str);

    bool replace_main_function_header(const char *to);
    bool replace_string(const char *from,const char *to);

private:
    std::string m_code;
    std::string m_replace_str;
    std::vector<variable> m_uniforms;
    std::vector<variable> m_attributes;
};

}
