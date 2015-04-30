//https://code.google.com/p/nya-engine/

#include "platform_specific_gl.h"
#include "render.h"
#include <string>

#if defined OPENGL_ES && !defined __APPLE__
    #include <EGL/egl.h>
#endif

namespace nya_render
{

#ifndef DIRECTX11
bool has_extension(const char *name)
{
    const char *exts=(const char*)glGetString(GL_EXTENSIONS);
    if(!exts)
        return false;

    if(std::string(exts).find(name)==std::string::npos)
        return false;
    
    return true;
}

void *get_extension(const char*ext_name)
{
    if(!ext_name)
    {
        log()<<"invalid extension name\n";
        return 0;
    }

#if defined __APPLE__
    return 0;
#else
  #if defined OPENGL_ES
    return (void*)eglGetProcAddress(ext_name);
  #elif defined _WIN32
    return (void*)wglGetProcAddress(ext_name);
  #else
    return (void*)glXGetProcAddressARB((const GLubyte *)ext_name);
  #endif
#endif
}
#endif

namespace { bool ignore_platform_restrictions=false; }
void set_ignore_platform_restrictions(bool ignore) { ignore_platform_restrictions=ignore; }
bool is_platform_restrictions_ignored() { return ignore_platform_restrictions; }
}
