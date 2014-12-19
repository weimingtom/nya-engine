//https://code.google.com/p/nya-engine/

#include <stdio.h>
#include <string.h>
#include "log/log.h"
#include "render/shader_code_parser.h"
#include "formats/text_parser.h"
#include <D3Dcompiler.h>
#include <io.h>
#include <fcntl.h>

#pragma comment(lib, "D3DCompiler.lib")
#pragma warning(disable: 4996)

const char *help="Usage: shader_compiler \%mode\%\n"
                 "accepts shader's code from stdin\n"
                 "stderr output begins with Error: if something goes wrong\n"
                 "outputs compiled shader/text assembly to stdout\n"
                 "modes:\n"
                 "hlsl - compiles hlsl shader\n"
                 "hlsl2asm - compiles hlsl shader and returns text assembly\n"
                 "glsl2hlsl - converts glsl shader to hlsl\n"
                 "or use: shader_compiler gencache \%src_dir\% \%dst_dir\%"
                 "\n";

                 //ToDo: allow sampler index assignment
                 //or glsl2hlsl is only for debug purpose?

bool compile_hlsl_code(const char *code,bool text_asm)
{
    if(!code)
        return false;

    const bool is_ps=strstr(code,"SV_TARGET")!=0;
    const char *profile=is_ps?"ps_4_0_level_9_3":"vs_4_0_level_9_3";
    ID3D10Blob *compiled=0, *error=0;
    D3DCompile(code,strlen(code),0,0,0,"main",profile,0,0,&compiled,&error);
    if(error)
    {
        fprintf(stderr,"Error: can`t compile shader with profile %s\n",profile);
        std::string error_text((const char *)error->GetBufferPointer(),error->GetBufferSize());
        fprintf(stderr,"%s\n",error_text.c_str());
        error->Release();
        return false;
    }

    if(!compiled)
    {
        fprintf(stderr,"Error: unknown compile error\n");
        return false;
    }

    if(text_asm)
    {
        ID3D10Blob *asm_blob;
        D3DDisassemble(compiled->GetBufferPointer(),compiled->GetBufferSize(),
                       D3D_DISASM_ENABLE_INSTRUCTION_NUMBERING,"",&asm_blob);
        if(!asm_blob)
            return false;

        fwrite(asm_blob->GetBufferPointer(),1,asm_blob->GetBufferSize(),stdout);
        asm_blob->Release();
    }
    else
        fwrite(compiled->GetBufferPointer(),1,compiled->GetBufferSize(),stdout);

    compiled->Release();
    return true;
}

int main(int argc, char* argv[])
{
    if(argc<2)
    {
        fprintf(stderr,"Error: no arguments\n");
        printf("%s",help);
        return 0;
    }

    setmode(fileno(stdout),O_BINARY);
    setmode(fileno(stdin),O_BINARY);

    nya_log::set_log(&nya_log::no_log());

    std::string shader_code;
    char buf[512];
    for(size_t size=fread(buf,1,sizeof(buf),stdin);size>0;size=fread(buf,1,sizeof(buf),stdin))
        shader_code.append(buf,size);

    if(shader_code.empty())
    {
        fprintf(stderr,"Error: empty stdin\n");
        printf("%s",help);
        return 0;
    }

    if(strcmp(argv[1],"hlsl")==0)
        return compile_hlsl_code(shader_code.c_str(),false)?0:-1;

    if(strcmp(argv[1],"hlsl2asm")==0)
        return compile_hlsl_code(shader_code.c_str(),true)?0:-1;

    if(strcmp(argv[1],"glsl2hlsl")==0)
    {
        nya_render::shader_code_parser parser(shader_code.c_str());
        if(!parser.convert_to_hlsl())
        {
            fprintf(stderr,"Error: cannot convert to hlsl\n");
            return -1;
        }

        return compile_hlsl_code(parser.get_code(),false)?0:-1;
    }

    if(strcmp(argv[1],"gencache")==0)
    {
        if(argc!=4)
        {
            fprintf(stderr,"Error: src and dst dir not specified\n");
            printf("Usage: shader_compiler gencache \%src_dir\% \%dst_dir\%\n");
            return -1;
        }

        //ToDo
    }

    fprintf(stderr,"Error: invalid compile mode: %s\n",argv[1]);
	return -1;
}
