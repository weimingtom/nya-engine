//https://code.google.com/p/nya-engine/

#include <stdio.h>
#include <string.h>
#include "log/log.h"

const char *help="Usage: shader_compiler mode code\n"
                 "output begins with Error: if something goes wrong\n"
                 "modes:\n"
                 "nshvs2nsc - compiles vertex shader from nsh\n"
                 "nshps2nsc - compiles pixel shader from nsh\n"
                 "glsl2nsc - compiles glsl shader\n"
                 "glsl2hlsl - converts glsl shader to hlsl\n"
                 "hlsl2asm - compiles hlsl shader and returns text assembly\n"
                 "\n";

int main(int argc, char* argv[])
{
    if(argc<3)
    {
        printf(help);
        return 0;
    }

    nya_log::set_log(&nya_log::no_log());

    if(strcmp(argv[1],"nshvs2nsc")==0)
    {
    }
    else if(strcmp(argv[1],"nshps2nsc")==0)
    {
    }
    else if(strcmp(argv[1],"glsl2nsc")==0)
    {
    }
    else if(strcmp(argv[1],"glsl2hlsl")==0)
    {
    }
    else if(strcmp(argv[1],"hlsl2asm")==0)
    {
    }

	return 0;
}
