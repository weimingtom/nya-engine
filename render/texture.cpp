//https://code.google.com/p/nya-engine/

#include "texture.h"
#include "render.h"
#include "platform_specific_gl.h"

#include "memory/tmp_buffer.h"

//ToDo: mipmaps and dxt support for cubemaps

namespace nya_render
{

namespace
{
    OPENGL_ONLY(unsigned int active_layer=0);

    const unsigned int max_layers=8;
    int current_layers[max_layers]={-1,-1,-1,-1,-1,-1,-1,-1};
    int active_layers[max_layers]={-1,-1,-1,-1,-1,-1,-1,-1};

    texture::filter default_min_filter=texture::filter_linear;
    texture::filter default_mag_filter=texture::filter_linear;
    texture::filter default_mip_filter=texture::filter_linear;

#ifndef DIRECTX11
    const unsigned int cube_faces[]={GL_TEXTURE_CUBE_MAP_POSITIVE_X,GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                                     GL_TEXTURE_CUBE_MAP_POSITIVE_Y,GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                                     GL_TEXTURE_CUBE_MAP_POSITIVE_Z,GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
  #ifndef NO_EXTENSIONS_INIT
    PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2D=0;
  #endif
#endif

int get_bpp(texture::color_format format)
{
    switch(format)
    {
        case texture::color_rgba: return 32;
        case texture::color_bgra: return 32;
        case texture::greyscale: return 8;
        case texture::color_rgb: return 24;
        case texture::depth16: return 16;
        DIRECTX11_ONLY(case texture::depth24: return 32);
        OPENGL_ONLY(case texture::depth24: return 24);
        case texture::depth32: return 32;

        case texture::dxt1: return 4;
        case texture::dxt3: return 8;
        case texture::dxt5: return 8;
    };

    return 0;
}

unsigned int get_tex_mips_count(unsigned int width,unsigned int height)
{
	int count=width && height;
	for(unsigned int w=width,h=height;!(w==1 && h==1);w>1?w=w/2:w=1,h>1?h/=2:h=1)
		++count;

	return count;
}

unsigned int get_tex_memory_size(unsigned int width,unsigned int height,texture::color_format format,int mip_count)
{
    unsigned int size=width*height*(get_bpp(format)/8);
    unsigned int full_size=0;
    if(mip_count>0)
    {
        for(int i=0;i<mip_count;++i,size/=4)
            full_size+=size;
    }
    else if(mip_count<0)
    {
		//inaccurate
        for(unsigned int w=width,h=height;w && h;w/=2,h/=2,size/=4)
            full_size+=size;
    }
    else
        full_size=size;

    return full_size;
}

void downsample(const void *from,void *to,unsigned int width,unsigned int height,unsigned int channels)
{
    const unsigned char *f=(unsigned char *)from;
    unsigned char *t=(unsigned char *)to;

    const unsigned int new_width = width / 2;
    const unsigned int new_height = height / 2;

    for( int h = 0; h < new_height*channels; h+=channels )
    {
        for( int w = 0; w < new_width*channels; w+=channels )
        {
            for(int c = 0; c < channels; ++c)
            {
                t[h*new_width+w+c] = ( f[(h*width+w)*2 + c] +
                                         f[((h*2+channels)*width+w*2) + c] +
                                         f[(h*2*width+(w*2+channels)) + c] +
                                         f[((h*2+channels)*width+(w*2+channels)) + c] ) / 4;
            }
        }
    }
}

#ifdef DIRECTX11
void dx_convert_to_format(const unsigned char *from,unsigned char *to,size_t size,texture::color_format format)
{
    if(format==texture::color_rgb)
    {
        for(unsigned int i=0;i<size;++i)
        {
            memcpy(to,from,3);
            *(to+3)=255;
            to+=4;
            from+=3;
        }
    }
    else
    {
        for(unsigned int i=0;i<size;++i)
        {
            for(int j=0;j<3;++j,++to)
                *(to)=*(from);

            *(to)=255;
            ++to;
            ++from;
        }
    }
}
#else
void gl_select_multitex_layer(int idx)
{
    if(idx==int(active_layer))
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
#endif
}

void gl_setup_filtration(int target,bool has_mips,texture::filter minif,texture::filter magnif,texture::filter mip)
{
    glTexParameteri(target,GL_TEXTURE_MAG_FILTER,minif==texture::filter_nearest?GL_NEAREST:GL_LINEAR);

    GLint filter;

    if(has_mips)
    {
        if(minif==texture::filter_nearest)
        {
            if(mip==texture::filter_nearest)
                filter=GL_NEAREST_MIPMAP_NEAREST;
            else
                filter=GL_NEAREST_MIPMAP_LINEAR;
        }
        else
        {
            if(mip==texture::filter_nearest)
                filter=GL_LINEAR_MIPMAP_NEAREST;
            else
                filter=GL_LINEAR_MIPMAP_LINEAR;
        }
    }
    else if(magnif==texture::filter_nearest)
        filter=GL_NEAREST;
    else
        filter=GL_LINEAR;

    glTexParameteri(target,GL_TEXTURE_MIN_FILTER,filter);
}

void gl_setup_texture(int target,bool clamp,bool has_mips)
{
    if(clamp || target==GL_TEXTURE_CUBE_MAP)
    {
        glTexParameteri(target,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(target,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

#ifndef OPENGL_ES
        if(target==GL_TEXTURE_CUBE_MAP)
            glTexParameteri(target,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
#endif
    }
    else
    {
        glTexParameteri(target,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(target,GL_TEXTURE_WRAP_T,GL_REPEAT);
    }

    gl_setup_filtration(target,has_mips,default_min_filter,default_mag_filter,default_mip_filter);
}

void gl_setup_pack_alignment()
{
    static bool set=false;
    if(set)
        return;

    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glPixelStorei(GL_PACK_ALIGNMENT,1);
    set=true;
}

unsigned int gl_get_max_tex_size()
{
    static unsigned int max_tex_size=0;
    if(!max_tex_size)
    {
        GLint texSize;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
        max_tex_size=texSize;
    }

    return max_tex_size;
}
#endif

void bgra_to_rgba(const unsigned char *from,unsigned char *to,size_t data_size)
{
    if(!from || !to || from==to)
        return;

    for(size_t i=0;i<data_size;i+=4)
    {
        to[i]=from[i+2];
        to[i+1]=from[i+1];
        to[i+2]=from[i+0];
        to[i+3]=from[i+3];
    }
}

}

bool texture::build_texture(const void *data,unsigned int width,unsigned int height,color_format format,int mip_count)
{
    if(width==0 || height==0)
    {
        log()<<"Unable to build texture: invalid width or height\n";
        release();
        return false;
    }

    if(format==dxt1 || format==dxt3 || format==dxt5)
    {
        if(!data || mip_count==0)
            return false;

        if(!is_dxt_supported())
            return false;

		if(mip_count<0)
			mip_count=1;
    }

    const bool pot=((width&(width-1))==0 && (height&(height-1))==0);

    if(!data)
        mip_count=0;
    else if(!pot)
        mip_count=1;

    const bool has_mipmap=(data && pot && mip_count!=1 && mip_count!=0);

#ifdef DIRECTX11
    if(!get_device())
        return false;

    if(m_tex>=0)
        release();

    D3D11_TEXTURE2D_DESC desc;
    memset(&desc,0,sizeof(desc));

	const bool need_generate_mips=(has_mipmap && mip_count<0);

    desc.Width=width;
    desc.Height=height;
    desc.MipLevels=mip_count?mip_count:1;
	if(need_generate_mips)
		desc.MipLevels=get_tex_mips_count(width,height);
    desc.ArraySize=1;

	std::vector<D3D11_SUBRESOURCE_DATA> srdata(desc.MipLevels);
	for(auto &l:srdata)
	{
		l.pSysMem=data;
		l.SysMemPitch=width*4;
		l.SysMemSlicePitch=0;
	}

	if(!need_generate_mips && mip_count>1)
	{
		const char *mem_data=(const char *)data;
		for(int i=0,w=width,h=height;i<(mip_count<=0?1:mip_count);++i,w=w>1?w/2:1,h=h>1?h/2:1)
		{
			srdata[i].pSysMem=mem_data;
			if(format==dxt1 || format==dxt3 || format==dxt5)
			{
				srdata[i].SysMemPitch=(w>4?w:4)/4 * get_bpp(format)*2;
				mem_data+=(h>4?h:4)/4 * srdata[i].SysMemPitch;
			}
			else
			{
				srdata[i].SysMemPitch=w*4;
				mem_data+=srdata[i].SysMemPitch*h;
			}
		}
	}

    m_format=format;

    switch(format)
    {
        case greyscale:
        case color_rgb:
        case color_rgba:
            m_format=color_rgba;
            desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
        break;

        case color_bgra: desc.Format=DXGI_FORMAT_B8G8R8A8_UNORM; break;

        case depth16: desc.Format=DXGI_FORMAT_D16_UNORM; break;
        case depth24: desc.Format=DXGI_FORMAT_D24_UNORM_S8_UINT; break; //ToDo if data != 0
        case depth32: desc.Format=DXGI_FORMAT_D32_FLOAT; break;

        case dxt1: desc.Format=DXGI_FORMAT_BC1_UNORM; break;
        case dxt3: desc.Format=DXGI_FORMAT_BC2_UNORM; break;
        case dxt5: desc.Format=DXGI_FORMAT_BC3_UNORM; break;

        default: return false;
    }

    desc.SampleDesc.Count=1;
    desc.Usage=D3D11_USAGE_DEFAULT;
    if(format==depth16 || format==depth24 || format==depth32)
        desc.BindFlags=D3D11_BIND_DEPTH_STENCIL;//ToDo: D3D11_BIND_SHADER_RESOURCE
    else if(format==dxt1 || format==dxt3 || format==dxt5)
        desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
    else
        desc.BindFlags=D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET;

    if(need_generate_mips)
        desc.MiscFlags=D3D11_RESOURCE_MISC_GENERATE_MIPS;

    //desc.MiscFlags=D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

    nya_memory::tmp_buffer_ref buf_rgb;
    nya_memory::tmp_buffer_ref buf_mip;

    ID3D11Texture2D *tex=0;
    if((format==color_rgb || format==greyscale) && data)
    {
        buf_rgb.allocate(srdata[0].SysMemPitch*height);
        srdata[0].pSysMem=buf_rgb.get_data();
        dx_convert_to_format((unsigned char *)data,(unsigned char *)buf_rgb.get_data(),height*width,format);
    }

    if(need_generate_mips && width!=height)
    {
        buf_mip.allocate(srdata[0].SysMemPitch*height/2);

        const void *prev_data=srdata[0].pSysMem;
        char *mem_data=(char *)buf_mip.get_data();
        for(int i=0,w=width,h=height;i<(int)srdata.size()-1;++i)
        {
            downsample(prev_data,mem_data,w,h,4);
            prev_data=srdata[i+1].pSysMem=mem_data;
            w=w>1?w/2:1,h=h>1?h/2:1;
            srdata[i+1].SysMemPitch=w*4;
            mem_data+=srdata[i+1].SysMemPitch*h;
        }
    }

    get_device()->CreateTexture2D(&desc,data?(&srdata[0]):0,&tex);

    buf_rgb.free();
    buf_mip.free();

    if(!tex)
        return false;

    ID3D11ShaderResourceView *srv=0;
    if(format<depth16 || format>depth32) //ToDo
    {
        get_device()->CreateShaderResourceView(tex,0,&srv);
        if(!srv)
            return false;

		if(need_generate_mips && width==height)
			get_context()->GenerateMips(srv);
    }

    D3D11_SAMPLER_DESC sdesc;
    memset(&sdesc,0,sizeof(sdesc));
    sdesc.Filter=D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    sdesc.AddressU=D3D11_TEXTURE_ADDRESS_WRAP;
    sdesc.AddressV=D3D11_TEXTURE_ADDRESS_WRAP;
    sdesc.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;
    sdesc.ComparisonFunc=D3D11_COMPARISON_NEVER;
    sdesc.MaxAnisotropy=0;
    sdesc.MinLOD=0;

#ifdef WINDOWS_PHONE8
    sdesc.MaxLOD=D3D11_FLOAT32_MAX;
#else
    sdesc.MaxLOD=mip_count>=1?mip_count:has_mipmap?D3D11_FLOAT32_MAX:1;
#endif

    m_tex=texture_obj::add();
    texture_obj::get(m_tex).tex=tex;
    texture_obj::get(m_tex).dx_format=desc.Format;
    texture_obj::get(m_tex).srv=srv;

    get_device()->CreateSamplerState(&sdesc,&texture_obj::get(m_tex).sampler_state);

    m_width=width;
    m_height=height;

#else
    if(width>gl_get_max_tex_size() || height>gl_get_max_tex_size())
    {
        log()<<"Unable to build texture: width or height is too high, maximum is "<<gl_get_max_tex_size()<<"\n";
        release();
        return false;
    }

    unsigned int source_format=0,gl_format=0,precision=GL_UNSIGNED_BYTE;
    switch(format)
    {
        case color_rgb: source_format=gl_format=GL_RGB; break; //in es stored internally as rgba
        case color_rgba: source_format=gl_format=GL_RGBA; break;
#ifdef __ANDROID__
        case color_bgra: source_format=GL_RGBA; gl_format=GL_RGBA; break;
#else
        case color_bgra: source_format=GL_RGBA; gl_format=GL_BGRA; break;
#endif
        case greyscale: source_format=gl_format=GL_LUMINANCE; break;
#ifdef OPENGL_ES
        case depth16: source_format=gl_format=GL_DEPTH_COMPONENT; precision=GL_UNSIGNED_SHORT; break;
        case depth24: source_format=gl_format=GL_DEPTH_COMPONENT; precision=GL_UNSIGNED_INT; break;
        case depth32: source_format=gl_format=GL_DEPTH_COMPONENT; precision=GL_UNSIGNED_INT; break;
#else
        case depth16: source_format=GL_DEPTH_COMPONENT16; gl_format=GL_DEPTH_COMPONENT; break;
        case depth24: source_format=GL_DEPTH_COMPONENT24; gl_format=GL_DEPTH_COMPONENT; break;
        case depth32: source_format=GL_DEPTH_COMPONENT32; gl_format=GL_DEPTH_COMPONENT; break;

        case dxt1: source_format=gl_format=GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
        case dxt3: source_format=gl_format=GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
        case dxt5: source_format=gl_format=GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
#endif
        default: break;
    };

    const unsigned int source_bpp=get_bpp(format);
    if(!source_format || !gl_format || !source_bpp)
    {
        log()<<"Unable to build texture: unsuppored color format\n";
        release();
        return false;
    }

    //bool create_new=(!m_tex_id || m_width!=width || m_height!=height || m_type!=texture_2d || m_format!=format);

    if(m_tex<0)
        m_tex=texture_obj::add();

    if(!texture_obj::get(m_tex).tex_id)
    {
        glGenTextures(1,&texture_obj::get(m_tex).tex_id);
    }
    else if(texture_obj::get(m_tex).gl_type!=GL_TEXTURE_2D)
    {
        glDeleteTextures(1,&texture_obj::get(m_tex).tex_id);
        glGenTextures(1,&texture_obj::get(m_tex).tex_id);
    }

    m_width=width;
    m_height=height;
    texture_obj::get(m_tex).gl_type=GL_TEXTURE_2D;
    m_format=format;

#ifdef OPENGL_ES
    if(m_format==depth24)
        m_format=depth32;

    if(m_format==color_rgb)
        m_format=color_rgba;

  #ifdef __ANDROID__
    nya_memory::tmp_buffer_ref temp_buf;
    if(m_format==color_bgra)
    {
        m_format=color_rgba;
        if(data)
        {
            temp_buf.allocate(width*height*4);
            bgra_to_rgba((const unsigned char *)data,(unsigned char *)temp_buf.get_data(),temp_buf.get_size());
            data=temp_buf.get_data();
        }
    }
  #endif
#endif

    if(active_layers[active_layer]>=0)
    {
        if(texture_obj::get(active_layers[active_layer]).gl_type!=GL_TEXTURE_2D)
            glBindTexture(texture_obj::get(active_layers[active_layer]).gl_type,0);
    }

    glBindTexture(GL_TEXTURE_2D,texture_obj::get(m_tex).tex_id);
    active_layers[active_layer]=-1;

    if(!pot) gl_setup_pack_alignment();
    gl_setup_texture(GL_TEXTURE_2D,!pot,has_mipmap);

  #ifdef GL_GENERATE_MIPMAP
    if(has_mipmap && mip_count<0) glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
  #endif
    unsigned int w=width,h=height;
    const char *data_pointer=(const char*)data;
    for(int i=0;i<(mip_count<=0?1:mip_count);++i,w=w>1?w/2:1,h=h>1?h/2:1)
    {
        unsigned int size=0;
        switch (format)
        {
            case color_rgb:
            case color_bgra:
            case color_rgba:
            case greyscale:
            case depth16:
            case depth24:
            case depth32:
                size=w*h*(source_bpp/8);
                glTexImage2D(GL_TEXTURE_2D,i,source_format,w,h,0,gl_format,precision,data_pointer);
                break;

  #ifndef OPENGL_ES
            case dxt1:
            case dxt3:
            case dxt5:
                size=(w>4?w:4)/4 * (h>4?h:4)/4 * source_bpp*2;
                glCompressedTexImage2D(GL_TEXTURE_2D,i,gl_format,w,h,0,size,data_pointer);
                break;
  #endif
            default: break;
        }
        data_pointer+=size;
    }

    //	glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width,height,gl_format,precision,data);
  #ifndef GL_GENERATE_MIPMAP
    if(has_mipmap && mip_count<0) glGenerateMipmap(GL_TEXTURE_2D);
  #endif

  #ifdef __ANDROID__
    temp_buf.free();
  #endif
    glBindTexture(GL_TEXTURE_2D,0);
#endif

    texture_obj::get(m_tex).size=get_tex_memory_size(m_width,m_height,m_format,mip_count);
    texture_obj::get(m_tex).is_cubemap=false;
    texture_obj::get(m_tex).has_mipmaps=has_mipmap;

    return true;
}

bool texture::is_dxt_supported()
{
#ifdef DIRECTX11
    return true;
#elif defined OPENGL_ES
    return true;
#else
  #ifndef NO_EXTENSIONS_INIT
    if(!glCompressedTexImage2D)
        glCompressedTexImage2D=(PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)get_extension("glCompressedTexImage2D");

    return glCompressedTexImage2D!=0;
  #endif
    return true;
#endif
}

bool texture::build_cubemap(const void *data[6],unsigned int width,unsigned int height,color_format format)
{
    if(width==0 || height==0)
    {
        log()<<"Unable to build cube texture: invalid width/height\n";
        release();
        return false;
    }

    const bool is_dxt=(format==dxt1 || format==dxt3 || format==dxt5);
    if(is_dxt)
    {
        if(!data)// || mip_count==0) //ToDo
            return false;

        if(!is_dxt_supported())
            return false;
    }

    const bool pot=((width&(width-1))==0 && (height&(height-1))==0);
    const bool has_mipmap=(data && pot);

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
    desc.ArraySize=6;

    D3D11_SUBRESOURCE_DATA srdata;
    srdata.pSysMem=data?data[0]:0;
    srdata.SysMemPitch=width*4;
    srdata.SysMemSlicePitch=0;

    switch(format)
    {
        case greyscale:
        case color_rgb:
        case color_rgba:
            desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
        break;

        case color_bgra: desc.Format=DXGI_FORMAT_B8G8R8A8_UNORM; break;

        case depth16: desc.Format=DXGI_FORMAT_D16_UNORM; break;
        case depth24: desc.Format=DXGI_FORMAT_D24_UNORM_S8_UINT; break; //ToDo if data != 0
        case depth32: desc.Format=DXGI_FORMAT_D32_FLOAT; break;

        default: return false;
    }

    desc.SampleDesc.Count=1;
    desc.Usage=D3D11_USAGE_DEFAULT;
    desc.BindFlags=D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET;
    if(data)
        desc.MiscFlags=D3D11_RESOURCE_MISC_GENERATE_MIPS;

    desc.MiscFlags|=D3D11_RESOURCE_MISC_TEXTURECUBE;

    //desc.MiscFlags=D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

    D3D11_SUBRESOURCE_DATA srdatas[6];
    for(int i=0;i<6;++i)
    {
        srdatas[i]=srdata;
        srdatas[i].pSysMem=data?data[i]:0;
    }

    ID3D11Texture2D *tex=0;
    if((format==color_rgb || format==greyscale) && data)
    {
        size_t size=srdata.SysMemPitch*height;
        nya_memory::tmp_buffer_scoped buf(size*6);
        srdata.pSysMem=buf.get_data();

        unsigned char *to=(unsigned char *)buf.get_data();
        for(int i=0;i<6;++i,to+=size)
        {
            dx_convert_to_format((const unsigned char *)data[i],to,height*width,format);
            srdatas[i].pSysMem=to;
        }

        get_device()->CreateTexture2D(&desc,srdatas,&tex);
    }
    else
        get_device()->CreateTexture2D(&desc,data?srdatas:0,&tex);

    if(!tex)
        return false;

    ID3D11ShaderResourceView *srv;
    get_device()->CreateShaderResourceView(tex,0,&srv);
    if(!srv)
        return false;

    D3D11_SAMPLER_DESC sdesc;
    memset(&sdesc,0,sizeof(sdesc));
    sdesc.Filter=D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    sdesc.AddressU=D3D11_TEXTURE_ADDRESS_WRAP;
    sdesc.AddressV=D3D11_TEXTURE_ADDRESS_WRAP;
    sdesc.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;
    sdesc.ComparisonFunc=D3D11_COMPARISON_NEVER;
    sdesc.MaxAnisotropy=0;
    sdesc.MinLOD=0;
    sdesc.MaxLOD = D3D11_FLOAT32_MAX;

    m_tex=texture_obj::add();
    texture_obj::get(m_tex).tex=tex;
    texture_obj::get(m_tex).dx_format=desc.Format;
    texture_obj::get(m_tex).srv=srv;

    get_device()->CreateSamplerState(&sdesc,&texture_obj::get(m_tex).sampler_state);

    m_width=width;
    m_height=height;

#else
    if(width>gl_get_max_tex_size() || height>gl_get_max_tex_size())
    {
        log()<<"Unable to build cube texture: width or height is too high, maximum is "<<gl_get_max_tex_size()<<"\n";
        release();
        return false;
    }

    unsigned int source_format=0;
    unsigned int gl_format=0;

    switch(format)
    {
        case color_rgb: source_format=GL_RGB; gl_format=GL_RGB; break;
        case color_rgba: source_format=GL_RGBA; gl_format=GL_RGBA; break;
#ifdef __ANDROID__
        case color_bgra: source_format=GL_RGBA; gl_format=GL_RGBA; break;
#else
        case color_bgra: source_format=GL_RGBA; gl_format=GL_BGRA; break;
#endif
        case greyscale: source_format=GL_LUMINANCE; gl_format=GL_LUMINANCE; break;

#ifndef OPENGL_ES
        case dxt1: source_format=gl_format=GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
        case dxt3: source_format=gl_format=GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
        case dxt5: source_format=gl_format=GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
#endif
        default: break;
    };

    if(!source_format || !gl_format)
    {
        log()<<"Unable to build cube texture: unsuppored color format\n";
        release();
        return false;
    }

#ifdef OPENGL_ES
    if(format==depth24)
        format=depth32;

    if(format==color_rgb)
        format=color_rgba;

  #ifdef __ANDROID__
    const bool swap_rb=(format==color_bgra);
    if(swap_rb)
        format=color_rgba;
  #endif

#endif

    if(m_format!=format)
        release();

    if(m_tex<0)
        m_tex=texture_obj::add();

    if(!texture_obj::get(m_tex).tex_id)
    {
        glGenTextures(1,&texture_obj::get(m_tex).tex_id);
    }
    else if(texture_obj::get(m_tex).gl_type!=GL_TEXTURE_CUBE_MAP)
    {
        glDeleteTextures(1,&texture_obj::get(m_tex).tex_id);
        glGenTextures(1,&texture_obj::get(m_tex).tex_id);
    }

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

    if(!pot) gl_setup_pack_alignment();
    gl_setup_texture(GL_TEXTURE_CUBE_MAP,true,has_mipmap);

  #ifdef GL_GENERATE_MIPMAP
    if(has_mipmap) glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_GENERATE_MIPMAP,GL_TRUE);
  #endif

    const unsigned int source_bpp=get_bpp(format);
    if(!source_bpp)
        return false;

    for(int i=0;i<int(sizeof(cube_faces)/sizeof(cube_faces[0]));++i)
    {
        if(is_dxt)
            glCompressedTexImage2D(cube_faces[i],0,gl_format,width,height,0,width*height*source_bpp/8,data[i]);
        else
        {
  #ifdef __ANDROID__
            nya_memory::tmp_buffer_ref temp_buf;
            if(swap_rb &&data && data[i])
            {
                temp_buf.allocate(width*height*4);
                bgra_to_rgba((const unsigned char *)data[i],(unsigned char *)temp_buf.get_data(),temp_buf.get_size());
                data[i]=temp_buf.get_data();
            }
  #endif
            glTexImage2D(cube_faces[i],0,source_format,width,height,0,gl_format,GL_UNSIGNED_BYTE,data?data[i]:0);
  #ifdef __ANDROID__
            temp_buf.free();
  #endif
        }
    }

  #ifndef GL_GENERATE_MIPMAP
    if(has_mipmap) glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
  #endif

    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
#endif
    texture_obj::get(m_tex).size=get_tex_memory_size(m_width,m_height,m_format,-1)*6;
    texture_obj::get(m_tex).is_cubemap=true;
    texture_obj::get(m_tex).has_mipmaps=has_mipmap;

    return true;
}

bool texture::build_cubemap(const void *data,unsigned int width,unsigned int height,color_format format)
{
    if(!data)
        return build_cubemap((const void **)0,width,height,format);

    unsigned int side_size=width*height*get_bpp(format)/8;
    const char *data_ptr=(const char *)data;
    const void *data_ptrs[6];
    for(int i=0;i<6;++i,data_ptr+=side_size)
        data_ptrs[i]=data_ptr;

    return build_cubemap(data_ptrs,width,height,format);
}

void texture::bind(unsigned int layer) const { if(layer>=max_layers) return; current_layers[layer]=m_tex; }

void texture::unbind(unsigned int layer) { if(layer>=max_layers) return; current_layers[layer]=-1; }

void texture::apply(bool ignore_cache)
{
#ifdef DIRECTX11
    if(!get_context())
        return;
#endif

    for(unsigned int i=0;i<max_layers;++i)
    {
        if(current_layers[i]==active_layers[i] && !ignore_cache)
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

bool texture::get_data( nya_memory::tmp_buffer_ref &data ) const
{
    if(m_tex<0)
        return false;

    const texture_obj &tex=texture_obj::get(m_tex);
    unsigned int size=m_width*m_height*get_bpp(m_format)/8;
    if(!size)
        return false;

#ifdef DIRECTX11
    if(!get_context() || !get_device())
        return false;

    if(m_format>=dxt1)
        return false;

    ID3D11Texture2D* copy_tex=0;

    D3D11_TEXTURE2D_DESC description;
    tex.tex->GetDesc( &description );
    description.BindFlags=0;
    description.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
    description.Usage=D3D11_USAGE_STAGING;

    HRESULT hr=get_device()->CreateTexture2D(&description,0,&copy_tex);
    if(FAILED(hr) || !copy_tex)
        return false;

    get_context()->CopyResource(copy_tex,tex.tex);
    D3D11_MAPPED_SUBRESOURCE resource;
    hr = get_context()->Map(copy_tex,0,D3D11_MAP_READ,0,&resource );
    if(FAILED(hr))
        return false;

    data.allocate(size);
    data.copy_to(resource.pData,size);

    get_context()->Unmap(copy_tex,0);
    copy_tex->Release();

    return true;
#else
  #ifdef OPENGL_ES
    if(m_format>=dxt1)
        return false;

    data.allocate(size);

    gl_select_multitex_layer(0);
    glBindTexture(tex.gl_type,tex.tex_id);
    active_layers[0]=-1;

    GLint prev_fbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING,&prev_fbo);

    static GLuint copy_fbo=0;
    if(!copy_fbo)
        glGenFramebuffers(1,&copy_fbo);

    glBindFramebuffer(GL_FRAMEBUFFER,copy_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,tex.tex_id,0);

    rect prev_vp=get_viewport();
    set_viewport(0,0,m_width,m_height);

    switch(m_format)
    {
        case color_rgb: glReadPixels(0,0,m_width,m_height,GL_RGB,GL_UNSIGNED_BYTE,data.get_data()); break;
        case color_rgba: glReadPixels(0,0,m_width,m_height,GL_RGBA,GL_UNSIGNED_BYTE,data.get_data()); break;
#ifndef __ANDROID__
        case color_bgra: glReadPixels(0,0,m_width,m_height,GL_BGRA,GL_UNSIGNED_BYTE,data.get_data()); break;
#endif
        case greyscale: glReadPixels(0,0,m_width,m_height,GL_LUMINANCE,GL_UNSIGNED_BYTE,data.get_data()); break;

        default:
            glBindFramebuffer(GL_FRAMEBUFFER, prev_fbo);
            set_viewport(prev_vp);
            return false;
    };

    glBindFramebuffer(GL_FRAMEBUFFER, prev_fbo);
    set_viewport(prev_vp);
  #else
    color_format format=m_format;
    if(format>=dxt1)
    {
        size=m_width*m_height*4;
        format=color_bgra;
    }

    data.allocate(size);

    gl_select_multitex_layer(0);
    glBindTexture(tex.gl_type,tex.tex_id);
    active_layers[0]=-1;

    switch(format)
    {
        case color_rgb: glGetTexImage(tex.gl_type,0,GL_RGB,GL_UNSIGNED_BYTE,data.get_data()); break;
        case color_rgba: glGetTexImage(tex.gl_type,0,GL_RGBA,GL_UNSIGNED_BYTE,data.get_data()); break;
        case color_bgra: glGetTexImage(tex.gl_type,0,GL_BGRA,GL_UNSIGNED_BYTE,data.get_data()); break;
        case greyscale: glGetTexImage(tex.gl_type,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,data.get_data()); break;

        default: return false;
            //glGetTexImage(tex.gl_type,0,GL_DEPTH_COMPONENT,GL_DEPTH_COMPONENT,data.get_data()); break;
    };
  #endif
#endif

    return true;
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
#endif
    texture_obj::remove(m_tex);

    m_tex=-1;
    m_width=0;
    m_height=0;
}

void texture::set_wrap(bool repeat_s,bool repeat_t)
{
    if(m_tex<0)
        return;

    const texture_obj &tex=texture_obj::get(m_tex);

#ifndef DIRECTX11
    const bool pot=((m_width&(m_width-1))==0 && (m_height&(m_height-1))==0);

    glBindTexture(tex.gl_type,tex.tex_id);
    glTexParameteri(tex.gl_type,GL_TEXTURE_WRAP_S,repeat_s&&pot?GL_REPEAT:GL_CLAMP_TO_EDGE);
    glTexParameteri(tex.gl_type,GL_TEXTURE_WRAP_T,repeat_t&&pot?GL_REPEAT:GL_CLAMP_TO_EDGE);
    active_layers[0]=-1;
#endif
}

void texture::set_filter(filter minification,filter magnification,filter mipmap)
{
    if(m_tex<0)
        return;

    const texture_obj &tex=texture_obj::get(m_tex);

#ifndef DIRECTX11
    glBindTexture(tex.gl_type,tex.tex_id);
    gl_setup_filtration(tex.gl_type,tex.has_mipmaps,minification,magnification,mipmap);
    active_layers[0]=-1;
#endif
}

void texture::set_default_filter(filter minification,filter magnification,filter mipmap)
{
    default_min_filter=minification;
    default_mag_filter=magnification;
    default_mip_filter=mipmap;
}

bool texture::is_cubemap() const
{
    if(m_tex<0)
        return false;

    return texture_obj::get(m_tex).is_cubemap;
}


namespace
{

struct size_counter
{
    unsigned int size;
    void apply(const texture_obj &obj) {
        size+=obj.size; }
    size_counter(): size(0) {}
};

};

unsigned int texture_obj::get_used_vmem_size()
{
    size_counter counter;
    get_texture_objs().apply_to_all(counter);

    return counter.size;
}

unsigned int texture::get_used_vmem_size()
{
    return texture_obj::get_used_vmem_size();
}

void texture_obj::release()
{
    DIRECTX11_ONLY(if(srv) srv->Release());
    DIRECTX11_ONLY(if(sampler_state) sampler_state->Release());
    DIRECTX11_ONLY(for(int i=0;i<6;++i) if(color_targets[i]) color_targets[i]->Release());
    DIRECTX11_ONLY(if(depth_target) depth_target->Release());
    DIRECTX11_ONLY(if(tex) tex->Release());
    OPENGL_ONLY(if(tex_id) glDeleteTextures(1,&tex_id));
    *this=texture_obj();
}

}
