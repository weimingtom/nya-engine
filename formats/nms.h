//https://code.google.com/p/nya-engine/

#pragma once

#include <vector>
#include <string>
#include <stddef.h>
#include "math/vector.h"
#include "math/quaternion.h"

namespace nya_formats
{

struct nms
{
    unsigned int version;

    struct chunk_info
    {
        unsigned int type;
        unsigned int size;
        const void *data;
    };

    enum section_type
    {
        mesh_data,
        skeleton,
        materials
    };

    std::vector<chunk_info> chunks;

public:
    bool read_chunks_info(const void *data,size_t size);

public:
    size_t get_nms_size();
    size_t encode_nms(void *to_data,size_t to_size);
};

struct nms_mesh_chunk
{
public:
    size_t read_header(const void *data,size_t size,int version); //0 if invalid
};

struct nms_material_chunk
{
    struct texture_info
    {
        std::string semantics;
        std::string filename;
    };

    struct string_param
    {
        std::string name;
        std::string value;
    };

    struct vector_param
    {
        std::string name;
        nya_math::vec4 value;
    };

    struct int_param
    {
        std::string name;
        int value;
    };

    struct material_info
    {
        std::string name;
        std::vector<texture_info> textures;
        std::vector<string_param> strings;
        std::vector<vector_param> vectors;
        std::vector<int_param> ints;
    };

    std::vector<material_info> materials;

public:
    bool read(const void *data,size_t size,int version);
};

struct nms_skeleton_chunk
{
    struct bone
    {
        std::string name;
        nya_math::quat rot;
        nya_math::vec3 pos;
        int parent;
    };

    std::vector<bone> bones;

public:
    bool read(const void *data,size_t size,int version);
};

}
