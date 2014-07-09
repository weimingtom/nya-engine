//https://code.google.com/p/nya-engine/

#pragma once

//write_to_buf return used size or 0 if failed
//to_data size should be allocated with enough size

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
    struct header
    {
        unsigned int version;
        unsigned int chunks_count;
    };

    static size_t read_header(header &out_header,const void *data,size_t size=nms_header_size);
    static size_t read_chunk_info(chunk_info &out_chunk_info,const void *data,size_t size);

public:
    size_t get_nms_size();
    size_t write_to_buf(void *to_data,size_t to_size); //to_size=get_nms_size()

public:
    static size_t write_header_to_buf(unsigned int chunks_count,void *to_data,size_t to_size=nms_header_size)
    {
        header h; h.version=latest_version,h.chunks_count=chunks_count;
        return write_header_to_buf(h,to_data,to_size);
    }

    static size_t write_header_to_buf(const header &h,void *to_data,size_t to_size=nms_header_size);

    static size_t get_chunk_write_size(size_t chunk_data_size);
    static size_t write_chunk_to_buf(const chunk_info &chunk,void *to_data,size_t to_size); //to_size=get_chunk_size()

public:
    const static size_t nms_header_size=16;
    const static unsigned int latest_version=2;
};

struct nms_mesh_chunk
{
    enum el_type
    {
        pos,
        normal,
        color,
        tc0=100
    };

    enum vertex_atrib_type
    {
        float16,
        float32
    };

    struct element
    {
        unsigned int type;
        unsigned int dimension;
        unsigned int offset;
        vertex_atrib_type data_type;
        std::string semantics;
    };

    enum draw_element_type
    {
        triangles,
        triangle_strip,
        points,
        lines,
        line_strip
    };

    struct group
    {
        nya_math::vec3 aabb_min;
        nya_math::vec3 aabb_max;

        std::string name;
        unsigned int material_idx;
        unsigned int offset;
        unsigned int count;
        draw_element_type element_type;
    };

    struct lod { std::vector<group> groups; };

    nya_math::vec3 aabb_min;
    nya_math::vec3 aabb_max;

    std::vector<element> elements;
    unsigned int vcount;
    unsigned int stride;
    std::vector<char> vbuf;

    unsigned int index_size;
    unsigned int icount;
    std::vector<char> ibuf;

    std::vector<lod> lods;

public:
    size_t read_header(const void *data,size_t size,int version); //0 if invalid

public:
    //size_t get_chunk_size();
    size_t write_to_buf(void *to_data,size_t to_size);
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

public:
    //size_t get_chunk_size();
    size_t write_to_buf(void *to_data,size_t to_size);
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

public:
    //size_t get_chunk_size();
    size_t write_to_buf(void *to_data,size_t to_size);
};

}
