//https://code.google.com/p/nya-engine/

#pragma once

#ifdef _WIN32
    #include <windows.h>
		#if _WIN32_WINNT>=_WIN32_WINNT_WIN8 && defined _M_ARM
            #define WINDOWS_PHONE8
		#endif

        //#define NYA_DIRECTX11

        #if defined WINDOWS_PHONE8 || defined NYA_DIRECTX11
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
        #include <OpenGL/gl.h>
        #include <OpenGL/glext.h>
    #endif

    #define NO_EXTENSIONS_INIT
#else
    #include <GL/glx.h>
    #include <GL/gl.h>
    #include <GL/glext.h>
#endif

#ifdef OPENGL_ES
    #define ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
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

#ifndef NO_EXTENSIONS_INIT
    bool has_extension(const char *name);
    void *get_extension(const char*ext_name);
#endif

#ifdef DIRECTX11
	ID3D11Device *get_device();
	void set_device(ID3D11Device *device);

	ID3D11DeviceContext *get_context();
	void set_context(ID3D11DeviceContext *context);

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

    compiled_shaders_provider *get_compiled_shaders_provider();
    void set_compiled_shaders_provider(compiled_shaders_provider *provider);

#endif
}
