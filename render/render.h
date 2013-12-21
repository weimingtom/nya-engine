//https://code.google.com/p/nya-engine/

#pragma once

#include "log/log.h"
#include "math/matrix.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace nya_render
{

void set_log(nya_log::log *l);
nya_log::log &get_log();
void log_gl_errors(const char *place=0);

struct rect
{
    int x;
    int y;
    int width;
    int height;

    rect(): x(0), y(0), width(0), height(0) {}
};

void set_viewport(int x,int y,int width,int height);
inline void set_viewport(const rect &r) { set_viewport(r.x,r.y,r.width,r.height); }
const rect &get_viewport();

struct scissor
{
    static void enable(int x,int y,int w,int h);
    static void disable();
};

void set_projection_matrix(const nya_math::mat4 &mat);
void set_modelview_matrix(const nya_math::mat4 &mat);

void set_color(float r,float g,float b,float a);

void set_clear_color(float r,float g,float b,float a);
void set_clear_depth(float value);
void clear(bool clear_color,bool clear_depth);

struct blend
{
    enum mode
    {
        zero,
        one,
        src_color,
        inv_src_color,
        src_alpha,
        inv_src_alpha,
        dst_color,
        inv_dst_color,
        dst_alpha,
        inv_dst_alpha
    };

    static void enable(mode src,mode dst);
    static void disable();
};

struct cull_face
{
    enum order
    {
        ccw,
        cw
    };

    static void enable(order o);
    static void disable();
};

struct depth_test
{
    enum comparsion
    {
        never,
        less,
        equal,
        greater,
        not_less, //greater or equal
        not_equal,
        not_greater, //less or equal
        allways
    };

    static void enable(comparsion mode);
    static void disable();
};

struct zwrite
{
    static void enable();
    static void disable();
};

struct color_write
{
    static void enable();
    static void disable();
};

struct state
{
    float color[4];

    bool blend;
    blend::mode blend_src;
    blend::mode blend_dst;

    bool cull_face;
    cull_face::order cull_order;

    bool depth_test;
    depth_test::comparsion depth_comparsion;

    bool zwrite;
    bool color_write;

    state():
        blend(false),
        blend_src(nya_render::blend::one),
        blend_dst(nya_render::blend::zero),

        cull_face(false),
        cull_order(nya_render::cull_face::ccw),

        depth_test(true),
        depth_comparsion(nya_render::depth_test::less),

        zwrite(true),
        color_write(true)
    { for(int i=0;i<4;++i) color[i]=0.0f; }
};

void set_state(const state &s);
const state &get_state();
const state &get_applied_state();

void apply_state(bool ignore_cache=false);

//dx-specific
ID3D11Device *get_device();
void set_device(ID3D11Device *device);

ID3D11DeviceContext *get_context();
void set_context(ID3D11DeviceContext *context);

}
