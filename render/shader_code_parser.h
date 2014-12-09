//https://code.google.com/p/nya-engine/

#pragma once

#include <string>
#include <vector>

namespace nya_render
{

class shader_code_parser
{
public:
    const char *get_code() const { return m_code.c_str(); }

public:
    bool convert_to_hlsl();
    bool convert_to_modern_glsl();

public:
    enum variable_type
    {
        type_invalid,
        type_float,
        type_vec2,
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

        union
        {
            unsigned int array_size;
            unsigned int idx;
        };

        variable():type(type_invalid),array_size(0){}
        variable(variable_type type,const char *name,unsigned int array_size):
                 type(type),name(name?name:""),array_size(array_size) {}
    };

    int get_uniforms_count();
    variable get_uniform(int idx) const;

    int get_attributes_count() const;
    variable get_attribute(int idx) const;

public:
    shader_code_parser(const char *text,const char *replace_prefix_str="nya"):
                       m_code(text?text:""),m_replace_str(replace_prefix_str?replace_prefix_str:"") { remove_comments(); }
private:
    void remove_comments();

    bool parse_uniforms(bool remove);
    bool parse_predefined_uniforms(const char *replace_prefix_str);
    bool parse_attributes(const char *replace_prefix_str);
    bool parse_varying(bool remove);

    bool replace_main_function_header(const char *replace_str);
    bool replace_hlsl_types();
    bool replace_string(const char *from,const char *to);

private:
    std::string m_code;
    std::string m_replace_str;
    std::vector<variable> m_uniforms;
    std::vector<variable> m_attributes;
    std::vector<variable> m_varying;
};

}
