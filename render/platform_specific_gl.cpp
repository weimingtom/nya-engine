//https://code.google.com/p/nya-engine/

#include "platform_specific_gl.h"

#ifndef NO_EXTENSIONS_INIT

#include "render.h"
#include <string>

namespace nya_render
{

void *get_exact_extension(const char*ext_name)
{
	#ifdef _WIN32
		return (void*)wglGetProcAddress(ext_name);
	#else
		return (void*)glXGetProcAddressARB((const GLubyte *)ext_name);	
	#endif
}

void *get_extension(const char*ext_name)
{
	if(!ext_name)
	{
		get_log()<<nya_log::error_internal<<"invalid extension name\n";
		return 0;
	}

	void *ext = get_exact_extension(ext_name);
	if(ext)
		return ext;

	const static std::string arb("ARB");
	const std::string ext_name_arb = std::string(ext_name)+arb;
	ext = get_exact_extension(ext_name_arb.c_str());
	if(ext)
		return ext;

	const static std::string ext("EXT");
	const std::string ext_name_ext = std::string(ext_name)+ext;
	ext = get_exact_extension(ext_name_ext.c_str());
	if(!ext)
		get_log()<<nya_log::error<<"unable to initialise extension "<<ext_name<<"\n";
		
	return ext;
}

bool has_extension(const char *name)
{
	const char *exts=glGetString(GL_EXTENSIONS);
	if(!exts)
		return false;

	if(std::string(exts).find(name)!=std::string::endpos)
		return true;
}

}

#endif

