//https://code.google.com/p/nya-engine/

#pragma once

#ifdef _WIN32
    #include <windows.h>
    #include <gl/gl.h>
    #include <gl/glu.h>
    #include <gl/glext.h>
#elif defined __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #import <OpenGLES/ES2/gl.h>
        #import <OpenGLES/ES2/glext.h>

        #define OPENGL_ES
    #else
        #include <OpenGL/gl.h>
        #include <OpenGL/glu.h>
        #include <OpenGL/glext.h>
    #endif

    #define NO_EXTENSIONS_INIT
#else
    #include <GL/glx.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
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
}
