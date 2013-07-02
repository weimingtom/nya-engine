//https://code.google.com/p/nya-engine/

#include "texture.h"
#include "render.h"
#include "platform_specific_gl.h"

#ifdef DIRECTX11
    #include "memory/tmp_buffer.h"
#endif

namespace
{
    unsigned int current_layer=0;
    unsigned int active_layer=0;
    const unsigned int max_layers=16;
    int current_layers[max_layers]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
    int active_layers[max_layers]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
}

namespace nya_render
{

#ifndef DIRECTX11
void gl_select_multitex_layer(int idx)
{
    if(idx==active_layer)
        return;

    active_layer=idx;

#if defined(OPENGL_ES)
    glActiveTexture(GL_TEXTURE0+idx);
#elif defined(NO_EXTENSIONS_INIT)
    glActiveTexture(GL_TEXTURE0+idx);
#else
    static PFNGLACTIVETEXTUREARBPROC tex_glActiveTexture=0;

    static bool initialised=false;
    if(tex_glActiveTexture!=0)
    {
        tex_glActiveTexture(GL_TEXTURE0_ARB+idx);
        return;
    }

    if(initialised)
        return;

    tex_glActiveTexture=(PFNGLACTIVETEXTUREARBPROC)get_extension("glActiveTexture");
    initialised=true;

    texture::select_multitex_slot(idx);
#endif
}
#endif

bool texture::build_texture(const void *data,unsigned int width,unsigned int height,color_format format)
{
    if(width==0 || height==0)
    {
        get_log()<<"Unable to build texture: invalid width or height\n";
	    release();
        return false;
    }

#ifdef DIRECTX11
    if(!get_device())
        return false;

    if(m_tex>=0)
        release();

    D3D11_TEXTURE2D_DESC desc;
    memset(&desc,0,sizeof(desc));

    desc.Width=width;
    desc.Height=height;
    desc.MipLevels=1;
    desc.ArraySize=1;

    D3D11_SUBRESOURCE_DATA srdata;
    srdata.pSysMem=data;

    switch(format)
    {
        case color_rgb:
        case color_rgba:
            desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
            srdata.SysMemPitch=width*4;
        break;

        case color_bgra:
            desc.Format=DXGI_FORMAT_B8G8R8A8_UNORM;
            srdata.SysMemPitch=width*4;
        break;

        case color_r:
            desc.Format=DXGI_FORMAT_R8_UNORM;
            srdata.SysMemPitch=width;
        break;

        case depth16: desc.Format=DXGI_FORMAT_D16_UNORM; break;
        case depth24: desc.Format=DXGI_FORMAT_D24_UNORM_S8_UINT; break; //ToDo if data != 0
        case depth32: desc.Format=DXGI_FORMAT_D32_FLOAT; break;

        default: return false;
    }

    srdata.SysMemSlicePitch=srdata.SysMemPitch*height;

    desc.SampleDesc.Count=1;
    desc.Usage=D3D11_USAGE_DEFAULT;
    desc.BindFlags=D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET;
    if(data)
        desc.MiscFlags=D3D11_RESOURCE_MISC_GENERATE_MIPS;

    //desc.MiscFlags=D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

    ID3D11Texture2D *tex=0;
    if(format==color_rgb && data)
    {
        nya_memory::tmp_buffer_scoped buf(srdata.SysMemSlicePitch);
        srdata.pSysMem=buf.get_data();

        typedef unsigned char uchar;
        const uchar *from=(const uchar *)data;
        uchar *to=(uchar *)buf.get_data();
        for(unsigned int h=0;h<height;++h)
        {
            for(unsigned int w=0;w<width;++w)
            {
                memcpy(to,from,3);
                *(to+3)=255;
                to+=4;
                from+=3;
            }
        }

        get_device()->CreateTexture2D(&desc,&srdata,&tex);
    }
    else
        get_device()->CreateTexture2D(&desc,data?(&srdata):0,&tex);

    if(!tex)
        return false;

    ID3D11ShaderResourceView *srv;
    get_device()->CreateShaderResourceView(tex,0,&srv);
    tex->Release();
    if(!srv)
        return false;

    D3D11_SAMPLER_DESC sdesc;
    memset(&sdesc,0,sizeof(sdesc));
	sdesc.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sdesc.AddressU=D3D11_TEXTURE_ADDRESS_WRAP;
	sdesc.AddressV=D3D11_TEXTURE_ADDRESS_WRAP;
    sdesc.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;
    sdesc.ComparisonFunc=D3D11_COMPARISON_NEVER;
    sdesc.MaxAnisotropy=0;
    sdesc.MinLOD=0;
    sdesc.MaxLOD = D3D11_FLOAT32_MAX;

    m_tex=texture_obj::add();
    texture_obj::get(m_tex).srv=srv;

    get_device()->CreateSamplerState(&sdesc,&texture_obj::get(m_tex).sampler_state);

#else
    if(!m_max_tex_size)
    {
        GLint texSize;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
        m_max_tex_size=texSize;
    }

    if(width>m_max_tex_size || height>m_max_tex_size)
    {
        get_log()<<"Unable to build texture: width or height is too high, maximum is "<<m_max_tex_size<<"\n";
	    release();
        return false;
    }

    unsigned int source_format=0;
    unsigned int gl_format=0;
    unsigned int precision=GL_UNSIGNED_BYTE;

    switch(format)
    {
        case color_rgb: source_format=GL_RGB; gl_format=GL_RGB; break;
        //case color_bgr: source_format=GL_RGB; gl_format=GL_BGR; break;
        case color_rgba: source_format=GL_RGBA; gl_format=GL_RGBA; break;
        case color_bgra: source_format=GL_RGBA; gl_format=GL_BGRA; break;
        case color_r: source_format=GL_LUMINANCE; gl_format=GL_LUMINANCE; break;
#ifdef OPENGL_ES
        case depth16: source_format=GL_DEPTH_COMPONENT; gl_format=GL_DEPTH_COMPONENT; precision=GL_UNSIGNED_SHORT; break;
        case depth24: source_format=GL_DEPTH_COMPONENT; gl_format=GL_DEPTH_COMPONENT; precision=GL_UNSIGNED_INT; break;
        case depth32: source_format=GL_DEPTH_COMPONENT; gl_format=GL_DEPTH_COMPONENT; precision=GL_UNSIGNED_INT; break;
#else
        case depth16: source_format=GL_DEPTH_COMPONENT16; gl_format=GL_DEPTH_COMPONENT; break;
        case depth24: source_format=GL_DEPTH_COMPONENT24; gl_format=GL_DEPTH_COMPONENT; break;
        case depth32: source_format=GL_DEPTH_COMPONENT32; gl_format=GL_DEPTH_COMPONENT; break;
#endif
    };

    if(!source_format || !gl_format)
    {
        get_log()<<"Unable to build texture: unsuppored color format\n";
	    release();
        return false;
    }

	//bool create_new=(!m_tex_id || m_width!=width || m_height!=height || m_type!=texture_2d || m_format!=format);

    if(m_tex<0)
        m_tex=texture_obj::add();

    if(!texture_obj::get(m_tex).tex_id || texture_obj::get(m_tex).gl_type!=GL_TEXTURE_2D)
        glGenTextures(1,&texture_obj::get(m_tex).tex_id);

    m_width=width;
    m_height=height;
	texture_obj::get(m_tex).gl_type=GL_TEXTURE_2D;
	m_format=format;
    
    if(active_layers[active_layer]>=0)
    {
        if(texture_obj::get(active_layers[active_layer]).gl_type!=GL_TEXTURE_2D)
            glBindTexture(texture_obj::get(active_layers[active_layer]).gl_type,0);
    }

    glBindTexture(GL_TEXTURE_2D,texture_obj::get(m_tex).tex_id);
    active_layers[active_layer]=-1;

    /*
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    */

    const bool pot=((width&(width-1))==0 && (height&(height-1))==0);
    const bool has_mipmap=(data && pot);

#ifdef GL_GENERATE_MIPMAP
    if(has_mipmap)
        glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
#endif
	//if(create_new)
	    glTexImage2D(GL_TEXTURE_2D,0,source_format,width,height,0,gl_format,precision,data);
	//else
	//	glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width,height,gl_format,precision,data);
#ifndef GL_GENERATE_MIPMAP
    if(has_mipmap)
        glGenerateMipmap(GL_TEXTURE_2D);
#endif

    if(pot)
    {
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    }

    //if(format<depth16)
    {
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

        if(has_mipmap)
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        else
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    }/*
    else
    {
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    }*/

    glBindTexture(GL_TEXTURE_2D,0);
#endif

    return true;
}

bool texture::build_cubemap(const void *data[6],unsigned int width,unsigned int height,color_format format)
{
    if(!data || width==0 || height==0)
    {
        get_log()<<"Unable to build cube texture: invalid data/width/height\n";
	    release();
        return false;
    }

#ifdef DIRECTX11
#else
    if(!m_max_tex_size)
    {
        GLint texSize;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
        m_max_tex_size=texSize;
    }

    if(width>m_max_tex_size || height>m_max_tex_size)
    {
        get_log()<<"Unable to build cube texture: width or height is too high, maximum is "<<m_max_tex_size<<"\n";
	    release();
        return false;
    }

    unsigned int source_format=0;
    unsigned int gl_format=0;

    switch(format)
    {
        case color_rgb: source_format=GL_RGB; gl_format=GL_RGB; break;
        //case color_bgr: source_format=GL_RGB; gl_format=GL_BGR; break;
        case color_rgba: source_format=GL_RGBA; gl_format=GL_RGBA; break;
        case color_bgra: source_format=GL_RGBA; gl_format=GL_BGRA; break;
        case color_r: source_format=GL_LUMINANCE; gl_format=GL_LUMINANCE; break;
        default: break;
    };

    if(!source_format || !gl_format)
    {
        get_log()<<"Unable to build cube texture: unsuppored color format\n";
	    release();
        return false;
    }

	if(m_format!=format)
		release();

    if(m_tex<0)
        m_tex=texture_obj::add();

    if(!texture_obj::get(m_tex).tex_id || texture_obj::get(m_tex).gl_type!=GL_TEXTURE_CUBE_MAP)
        glGenTextures(1,&texture_obj::get(m_tex).tex_id);

    m_width=width;
    m_height=height;
	texture_obj::get(m_tex).gl_type=GL_TEXTURE_CUBE_MAP;
	m_format=format;

    if(active_layers[active_layer]>=0)
    {
        if(texture_obj::get(active_layers[active_layer]).gl_type!=GL_TEXTURE_CUBE_MAP)
            glBindTexture(texture_obj::get(active_layers[active_layer]).gl_type,0);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP,texture_obj::get(m_tex).tex_id);
    active_layers[active_layer]=-1;

#ifdef GL_GENERATE_MIPMAP
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_GENERATE_MIPMAP,GL_TRUE);
#endif

	const unsigned int cube_faces[]={GL_TEXTURE_CUBE_MAP_POSITIVE_X,GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
									 GL_TEXTURE_CUBE_MAP_POSITIVE_Y,GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
									 GL_TEXTURE_CUBE_MAP_POSITIVE_Z,GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};

	for(int i=0;i<sizeof(cube_faces)/sizeof(cube_faces[0]);++i)
		glTexImage2D(cube_faces[i],0,source_format,width,height,0,gl_format,GL_UNSIGNED_BYTE,data[i]);

#ifndef GL_GENERATE_MIPMAP
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
#endif

    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);

    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
#endif

    return true;
}

void texture::bind() const { current_layers[current_layer]=m_tex; }

void texture::unbind() { current_layers[current_layer]=-1; }

void texture::select_multitex_slot(unsigned int idx)
{
    if(idx>=max_layers)
        return;

    current_layer=idx;
}

void texture::apply()
{
#ifdef DIRECTX11
    if(!get_context())
        return;
#endif

    for(unsigned int i=0;i<max_layers;++i)
    {
        if(current_layers[i]==active_layers[i])
            continue;

#ifdef DIRECTX11
        if(current_layers[i]<0)
        {
            get_context()->PSSetShaderResources(i,0,0);
            get_context()->PSSetSamplers(i,0,0);
            continue;
        }

        const texture_obj &tex=texture_obj::get(current_layers[i]);
        get_context()->PSSetShaderResources(i,1,&tex.srv);
        get_context()->PSSetSamplers(i,1,&tex.sampler_state);
#else
        gl_select_multitex_layer(i);

        if(current_layers[i]<0)
        {
            if(active_layers[i]<0)
                continue;

            glBindTexture(texture_obj::get(active_layers[i]).gl_type,0);
            continue;
        }

        const texture_obj &tex=texture_obj::get(current_layers[i]);
        if(active_layers[i]>=0)
        {
            const texture_obj &atex=texture_obj::get(active_layers[i]);
            if(tex.gl_type!=atex.gl_type)
                glBindTexture(atex.gl_type,0);
        }

        glBindTexture(tex.gl_type,tex.tex_id);
#endif
    }
}

void texture::release()
{
    if(m_tex<0)
        return;

#ifdef DIRECTX11
    if(!get_context())
        return;

    for(unsigned int i=0;i<max_layers;++i)
    {
        if(active_layers[i]==m_tex)
        {
            get_context()->PSSetShaderResources(i,0,0);
            get_context()->PSSetSamplers(i,0,0);
            active_layers[i]=-1;
        }

        if(current_layers[i]==m_tex)
            current_layers[i]=-1;
    }

    if(texture_obj::get(m_tex).srv)
        texture_obj::get(m_tex).srv->Release();

    if(texture_obj::get(m_tex).sampler_state)
        texture_obj::get(m_tex).sampler_state->Release();
#else
    for(unsigned int i=0;i<max_layers;++i)
    {
        if(active_layers[i]==m_tex)
        {
            gl_select_multitex_layer(i);
            glBindTexture(texture_obj::get(m_tex).gl_type,0);
            active_layers[i]=-1;
        }

        if(current_layers[i]==m_tex)
            current_layers[i]=-1;
    }

    if(texture_obj::get(m_tex).tex_id)
	    glDeleteTextures(1,&texture_obj::get(m_tex).tex_id);
#endif
    texture_obj::remove(m_tex);

    m_tex=-1;
    m_width=0;
    m_height=0;
}

}
