//https://code.google.com/p/nya-engine/

#pragma once

#ifdef _WIN32
    #include "windows.h"
    #include "../gl/glew.h"
    #include "../GL/glext.h"
//    #pragma comment ( lib, "opengl32.lib" )
//    #pragma comment ( lib, "glu32.lib"  )

#elif defined __APPLE__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
    #include <OpenGL/glext.h>
    #define NO_EXTENSIONS_INIT
#else
    #include <GL/glx.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
    #include <GL/glext.h>
#endif

#ifndef NO_EXTENSIONS_INIT
namespace nya_render
{

bool has_extension(const char *name);
void *get_extension(const char*ext_name);

}
#endif
