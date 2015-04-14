//https://code.google.com/p/nya-engine/

#pragma once

//#define NYA_DIRECTX11
//#define NYA_OPENGL3

#include "render_objects.h"
#include "texture.h"

#ifdef _WIN32
    #include <windows.h>
    #if defined(_MSC_VER) && _MSC_VER >= 1700
        #if _WIN32_WINNT >= _WIN32_WINNT_WIN8 && !_USING_V110_SDK71_
            #include "winapifamily.h"
            #if defined(WINAPI_PARTITION_DESKTOP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
            #elif defined(WINAPI_PARTITION_PHONE) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE)
                #define WINDOWS_PHONE8
                #define WINDOWS_METRO
            #elif defined(WINAPI_PARTITION_APP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
                #define WINDOWS_METRO
            #endif
        #endif
    #endif

    #if defined WINDOWS_METRO || defined NYA_DIRECTX11
        #define DIRECTX11
    #endif

    #ifdef DIRECTX11
        #include <d3d11.h>
        #include <vector>
        #define NO_EXTENSIONS_INIT
    #else
        #include <gl/gl.h>
        #include <gl/glext.h>
    #endif
#elif defined __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #import <OpenGLES/ES2/gl.h>
        #import <OpenGLES/ES2/glext.h>

        #define OPENGL_ES
    #else
        #ifdef NYA_OPENGL3
            #define OPENGL3
        #endif

        #ifdef OPENGL3
            #include <OpenGL/gl3.h>
        #else
            #include <OpenGL/gl.h>
        #endif

        #include <OpenGL/glext.h>
        #define NO_EXTENSIONS_INIT
    #endif

#elif __ANDROID__
    #include <GLES2/gl2.h>
    #define GL_GLEXT_PROTOTYPES
    #include <GLES2/gl2ext.h>
    #define OPENGL_ES
#else
    #include <GL/glx.h>
    #include <GL/gl.h>
    #include <GL/glext.h>
#endif

#if defined OPENGL_ES || defined OPENGL3
    #define ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
    #define NO_EXTENSIONS_INIT
#endif

#ifdef DIRECTX11
    #define DIRECTX11_ONLY(...) __VA_ARGS__
    #define OPENGL_ONLY(...)
#else
    #define DIRECTX11_ONLY(...)
    #define OPENGL_ONLY(...) __VA_ARGS__
#endif

namespace nya_render
{
#ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
    enum
    {
        vertex_attribute,
        normal_attribute,
        color_attribute,
        tc0_attribute,
        max_attributes = tc0_attribute+16
    };
#endif

    bool has_extension(const char *name);
#ifndef NO_EXTENSIONS_INIT
    void *get_extension(const char*ext_name);
#endif

    void set_ignore_platform_restrictions(bool ignore);
    bool is_platform_restrictions_ignored();

    struct texture_obj
    {
        unsigned int width,height;
        unsigned int size;
        bool has_mipmaps;
        bool is_cubemap;
        texture::color_format format;

#ifdef DIRECTX11
        ID3D11Texture2D *tex;
        ID3D11ShaderResourceView* srv;
        ID3D11SamplerState* sampler_state;
        ID3D11RenderTargetView *color_targets[6];
        ID3D11DepthStencilView *depth_target;
        DXGI_FORMAT dx_format;
#else
        unsigned int tex_id,gl_type;
#endif

        texture_obj(): width(0),height(0),size(0),is_cubemap(false),has_mipmaps(false)
        {
#ifdef DIRECTX11
            tex=0,srv=0,sampler_state=0,depth_target=0;
            for(int i=0;i<6;++i)
                color_targets[i]=0;
#else
            tex_id=0,gl_type=0;
#endif
        }

    public:
        static int add() { return get_texture_objs().add(); }
        static texture_obj &get(int idx) { return get_texture_objs().get(idx); }
        static void remove(int idx) { return get_texture_objs().remove(idx); }
        static unsigned int get_used_vmem_size();
        static int invalidate_all() { return get_texture_objs().invalidate_all(); }
        static int release_all() { return get_texture_objs().release_all(); }

    public:
        void release();

    public:
        typedef render_objects<texture_obj> texture_objs;
        static texture_objs &get_texture_objs()
        {
            static texture_objs objs;
            return objs;
        }
    };

#ifdef DIRECTX11
    ID3D11InputLayout *get_layout(int mesh_idx);
    ID3D11InputLayout *add_layout(int mesh_idx,
                                  const D3D11_INPUT_ELEMENT_DESC*desc,size_t desc_size);
    void remove_layout(int mesh_idx);

    struct dx_target
    {
        ID3D11RenderTargetView *color;
        ID3D11DepthStencilView *depth;

        dx_target(): color(0),depth(0) {}
    };

    dx_target get_default_target();
    dx_target get_target();

    void set_target(ID3D11RenderTargetView *color,ID3D11DepthStencilView *depth,bool default=false);
#endif
    void reset_vbo_state();

    inline int release_textures() { return texture_obj::release_all(); }
    int release_vbos();
    int release_shaders();
    int release_fbos();
    void release_states();

    int invalidate_shaders();
    int invalidate_vbos();
    int invalidate_fbos();
}
