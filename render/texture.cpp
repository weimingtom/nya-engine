//https://code.google.com/p/nya-engine/

#include "texture.h"
#include "render.h"
#include "platform_specific_gl.h"

#include "memory/tmp_buffer.h"

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
    unsigned int default_aniso=0;

#ifndef DIRECTX11
    const unsigned int cube_faces[]={GL_TEXTURE_CUBE_MAP_POSITIVE_X,GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                                     GL_TEXTURE_CUBE_MAP_POSITIVE_Y,GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                                     GL_TEXTURE_CUBE_MAP_POSITIVE_Z,GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
  #ifndef NO_EXTENSIONS_INIT
    PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2D=0;
  #endif

  #ifdef OPENGL3
    #ifdef GL_LUMINANCE
        #undef GL_LUMINANCE
    #endif
    #define GL_LUMINANCE GL_RED
    #define GL_TEXTURE_SWIZZLE_RGBA 0x8E46
  #endif

  #ifdef OPENGL_ES
    #define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
    #define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
    #define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3

    #ifndef GL_IMG_texture_compression_pvrtc
      #define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0x8c00
      #define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0x8c01
      #define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8c02
      #define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0x8c03
    #endif

    #define GL_COMPRESSED_RGB8_ETC2 0x9274
    #define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
    #define GL_COMPRESSED_RGBA8_ETC2_EAC 0x9278

    #ifdef __APPLE__
        #define GL_ETC1_RGB8_OES GL_COMPRESSED_RGB8_ETC2
    #else
        #define GL_ETC1_RGB8_OES 0x8D64
    #endif
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
        case texture::color_rgb32f: return 32*3;
        case texture::color_rgba32f: return 32*4;
        case texture::depth16: return 16;
        DIRECTX11_ONLY(case texture::depth24: return 32);
        OPENGL_ONLY(case texture::depth24: return 24);
        case texture::depth32: return 32;

        case texture::dxt1: return 4;
        case texture::dxt3: return 8;
        case texture::dxt5: return 8;

        case texture::etc1: return 4;
        case texture::etc2: return 4;
        case texture::etc2_a1: return 4;
        case texture::etc2_eac: return 8;

        case texture::pvr_rgb2b: return 2;
        case texture::pvr_rgb4b: return 4;
        case texture::pvr_rgba2b: return 2;
        case texture::pvr_rgba4b: return 4;
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

    const unsigned int new_width=width/2;
    const unsigned int new_height=height/2;

    for(unsigned int h=0;h<new_height*channels;h+=channels)
    {
        for(unsigned int w=0;w<new_width*channels;w+=channels)
        {
            for(unsigned int c=0;c<channels;++c)
            {
                t[h*new_width+w+c]=(f[(h*width+w)*2+c]+
                                    f[((h*2+channels)*width+w*2)+c]+
                                    f[(h*2*width+(w*2+channels))+c]+
                                    f[((h*2+channels)*width+(w*2+channels))+c])/4;
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

D3D11_SAMPLER_DESC dx_setup_filtration()
{
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

    return sdesc;
}

#else
bool init_compressed_extension()
{
#ifndef NO_EXTENSIONS_INIT
    if(!glCompressedTexImage2D)
        glCompressedTexImage2D=(PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)get_extension("glCompressedTexImage2DARB");
    return glCompressedTexImage2D!=0;
#else
    return true;
#endif
}

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

    tex_glActiveTexture=(PFNGLACTIVETEXTUREARBPROC)get_extension("glActiveTextureARB");
    initialised=true;
    gl_select_multitex_layer(idx);
#endif
}

void gl_setup_filtration(int target,bool has_mips,texture::filter minif,texture::filter magnif,texture::filter mip)
{
    glTexParameteri(target,GL_TEXTURE_MAG_FILTER,magnif==texture::filter_nearest?GL_NEAREST:GL_LINEAR);

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
    else if(minif==texture::filter_nearest)
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
    if(default_aniso>0)
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAX_ANISOTROPY_EXT,float(default_aniso));
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

bool texture::build_texture(const void *data_a[6],bool is_cubemap,unsigned int width,unsigned int height,
                            color_format format,int mip_count)
{
    if(width==0 || height==0)
    {
        log()<<"Unable to build texture: invalid width or height\n";
        release();
        return false;
    }

    if(is_cubemap && width!=height)
        return false;

    const void *data=data_a?data_a[0]:0;

    if(format>=dxt1)
    {
        if(!data || mip_count==0)
        {
            log()<<"Unable to build texture: compressed format with invalid data\n";
            return false;
        }

        OPENGL_ONLY(if(!init_compressed_extension()) return false;);

        if(mip_count<0)
            mip_count=1;
    }

    const bool is_pvrtc=format==pvr_rgb2b || format==pvr_rgba2b || format==pvr_rgb4b || format==pvr_rgba4b;

    const bool pot=((width&(width-1))==0 && (height&(height-1))==0);

    if(is_pvrtc && width!=height && !pot)
    {
        log()<<"Unable to build texture: pvr compression supports only square pot textures\n";
        return false;
    }

#ifdef __ANDROID__
    if(format==color_bgra && mip_count>1 && data) //ToDo
        mip_count= -1;
#endif

    if(!data)
        mip_count=0;
    else if(!pot)
        mip_count=1;

    const bool has_mipmap=(data && pot && mip_count!=1 && mip_count!=0);

#ifdef DIRECTX11
    if(!get_device())
    {
        log()<<"Unable to build texture: invalid directx device, use nya_render::set_device()\n";
        return false;
    }

    if(m_tex>=0)
        release(); //ToDo: reuse

    if(m_tex<0)
        m_tex=texture_obj::add();

    texture_obj &t=texture_obj::get(m_tex);

    if((format==color_rgb || format==greyscale) && data && mip_count>1) //ToDo
        mip_count= -1;

    D3D11_TEXTURE2D_DESC desc;
    memset(&desc,0,sizeof(desc));

    const bool need_generate_mips=(has_mipmap && mip_count<0);

    desc.Width=width;
    desc.Height=height;
    desc.MipLevels=mip_count?mip_count:1;
    if(need_generate_mips)
        desc.MipLevels=get_tex_mips_count(width,height);
    desc.ArraySize=is_cubemap?6:1;

    t.format=format;

    switch(format)
    {
        case greyscale:
        case color_rgb:
        case color_rgba:
            t.format=color_rgba;
            desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
        break;

        case color_bgra: desc.Format=DXGI_FORMAT_B8G8R8A8_UNORM; break;

        case depth16: desc.Format=DXGI_FORMAT_D16_UNORM; break;
        case depth24: desc.Format=DXGI_FORMAT_D24_UNORM_S8_UINT; break; //ToDo if data != 0
        case depth32: desc.Format=DXGI_FORMAT_D32_FLOAT; break;

        case dxt1: desc.Format=DXGI_FORMAT_BC1_UNORM; break;
        case dxt3: desc.Format=DXGI_FORMAT_BC2_UNORM; break;
        case dxt5: desc.Format=DXGI_FORMAT_BC3_UNORM; break;

        default: log()<<"Unable to build texture: unsupported format\n"; return false;
    }

    std::vector<D3D11_SUBRESOURCE_DATA> srdata(desc.MipLevels*desc.ArraySize);
    for(unsigned int f=0,s=0;f<desc.ArraySize;++f)
    {
        for(unsigned int i=0;i<desc.MipLevels;++i)
        {
            auto &l=srdata[f*desc.MipLevels+i];
            l.pSysMem=data_a?data_a[f]:0;
            if(format>=dxt1)
                l.SysMemPitch=(width>4?width:4)/4 * get_bpp(t.format)*2;
            else
                l.SysMemPitch=width*get_bpp(t.format)/8;
            l.SysMemSlicePitch=0;
        }

        if(!need_generate_mips && mip_count>1)
        {
            const char *mem_data=(const char *)(data_a?data_a[f]:0);
            for(int i=0,w=width,h=height;i<(mip_count<=0?1:mip_count);++i,w=w>1?w/2:1,h=h>1?h/2:1,++s)
            {
                srdata[s].pSysMem=mem_data;
                if(format>=dxt1)
                {
                    srdata[s].SysMemPitch=(w>4?w:4)/4 * get_bpp(t.format)*2;
                    mem_data+=(h>4?h:4)/4 * srdata[s].SysMemPitch;
                }
                else
                {
                    srdata[s].SysMemPitch=w*get_bpp(t.format)/8;
                    mem_data+=srdata[s].SysMemPitch*h;
                }
            }
        }
    }

    desc.SampleDesc.Count=1;
    desc.Usage=D3D11_USAGE_DEFAULT;
    if(format==depth16 || format==depth24 || format==depth32)
        desc.BindFlags=D3D11_BIND_DEPTH_STENCIL;//ToDo: D3D11_BIND_SHADER_RESOURCE
    else if(format>=dxt1)
        desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
    else
        desc.BindFlags=D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET;

    if(need_generate_mips)
        desc.MiscFlags=D3D11_RESOURCE_MISC_GENERATE_MIPS;

    if(is_cubemap)
        desc.MiscFlags|=D3D11_RESOURCE_MISC_TEXTURECUBE;

    //desc.MiscFlags=D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

    ID3D11Texture2D *tex=0;

    nya_memory::tmp_buffer_ref buf_rgb;
    nya_memory::tmp_buffer_ref buf_mip;

    if((format==color_rgb || format==greyscale) && data) //ToDo: mipmaps
    {
        const unsigned int face_size=srdata[0].SysMemPitch*height;
        buf_rgb.allocate(face_size*desc.ArraySize);

        for(unsigned int i=0;i<desc.ArraySize;++i)
        {
            unsigned char *to_data=(unsigned char *)buf_rgb.get_data()+face_size*i;
            dx_convert_to_format((unsigned char *)(srdata[i*desc.MipLevels].pSysMem),to_data,width*height,format);
            for(unsigned int j=0;j<desc.MipLevels;++j)
                srdata[i*desc.MipLevels+j].pSysMem=to_data;
        }
    }

    if(need_generate_mips && width!=height && !is_platform_restrictions_ignored())
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
    {
        log()<<"Unable to build texture: unable to create texture\n";
        return false;
    }

    ID3D11ShaderResourceView *srv=0;
    if(format<depth16 || format>depth32) //ToDo
    {
        get_device()->CreateShaderResourceView(tex,0,&srv);
        if(!srv)
        {
            log()<<"Unable to build texture: unable to create shader resource view\n";
            return false;
        }

        if(need_generate_mips && width==height)
            get_context()->GenerateMips(srv);
    }

    t.tex=tex;
    t.dx_format=desc.Format;
    t.srv=srv;

    auto sdesc=dx_setup_filtration();

    if(default_aniso>0)
    {
        sdesc.Filter=D3D11_FILTER_ANISOTROPIC;
        sdesc.MaxAnisotropy=default_aniso;
    }

#ifdef WINDOWS_METRO
    sdesc.MaxLOD=D3D11_FLOAT32_MAX;
#else
    sdesc.MaxLOD=mip_count>=1?mip_count:has_mipmap?D3D11_FLOAT32_MAX:1;
#endif

    get_device()->CreateSamplerState(&sdesc,&t.sampler_state);
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
        case color_rgb32f: source_format=GL_RGB; gl_format=GL_RGB; precision=GL_FLOAT; break;
        case color_rgba32f: source_format=GL_RGBA; gl_format=GL_RGBA; precision=GL_FLOAT; break;

        case depth16: source_format=gl_format=GL_DEPTH_COMPONENT; precision=GL_UNSIGNED_SHORT; break;
        case depth24: source_format=gl_format=GL_DEPTH_COMPONENT; precision=GL_UNSIGNED_INT; break;
        case depth32: source_format=gl_format=GL_DEPTH_COMPONENT; precision=GL_UNSIGNED_INT; break;

        case etc1: source_format=gl_format=GL_ETC1_RGB8_OES; break;
        case etc2: source_format=gl_format=GL_COMPRESSED_RGB8_ETC2; break;
        case etc2_a1: source_format=gl_format=GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2; break;
        case etc2_eac: source_format=gl_format=GL_COMPRESSED_RGBA8_ETC2_EAC; break;

        case pvr_rgb2b: source_format=gl_format=GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG; break;
        case pvr_rgb4b: source_format=gl_format=GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG; break;
        case pvr_rgba2b: source_format=gl_format=GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG; break;
        case pvr_rgba4b: source_format=gl_format=GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG; break;
#else
  #ifdef OPENGL3
        case color_rgb32f: source_format=GL_RGB32F; gl_format=GL_RGB; precision=GL_FLOAT; break;
        case color_rgba32f: source_format=GL_RGBA32F; gl_format=GL_RGBA; precision=GL_FLOAT; break;
  #else
        case color_rgb32f: source_format=GL_RGB32F_ARB; gl_format=GL_RGB; precision=GL_FLOAT; break;
        case color_rgba32f: source_format=GL_RGBA32F_ARB; gl_format=GL_RGBA; precision=GL_FLOAT; break;
  #endif
        case depth16: source_format=GL_DEPTH_COMPONENT16; gl_format=GL_DEPTH_COMPONENT; break;
        case depth24: source_format=GL_DEPTH_COMPONENT24; gl_format=GL_DEPTH_COMPONENT; break;
        case depth32: source_format=GL_DEPTH_COMPONENT32; gl_format=GL_DEPTH_COMPONENT; break;
#endif
        case dxt1: source_format=gl_format=GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
        case dxt3: source_format=gl_format=GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
        case dxt5: source_format=gl_format=GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;

        default: log()<<"Unable to build texture: unsupported format\n"; break;
    };

    const unsigned int source_bpp=get_bpp(format);
    if(!source_format || !gl_format || !source_bpp)
    {
        log()<<"Unable to build texture: unsuppored color format\n";
        release();
        return false;
    }

    const int gl_type=is_cubemap?GL_TEXTURE_CUBE_MAP:GL_TEXTURE_2D;

    if(m_tex<0)
        m_tex=texture_obj::add();

    texture_obj &t=texture_obj::get(m_tex);

    bool need_create=t.width!=width || t.height!=height || t.format!=format;
    if(!t.tex_id)
    {
        glGenTextures(1,&t.tex_id);
        need_create=true;
    }
    else if(t.gl_type!=gl_type)
    {
        glDeleteTextures(1,&t.tex_id);
        glGenTextures(1,&t.tex_id);
        need_create=true;
    }

    t.gl_type=gl_type;

#ifdef OPENGL_ES
    if(format==depth24)
        format=depth32;

    if(format==color_rgb)
        format=color_rgba;

  #ifdef __ANDROID__
    nya_memory::tmp_buffer_ref temp_buf;
    if(format==color_bgra)
    {
        format=color_rgba;
        if(data)
        {
            const size_t size=width*height*4;
            temp_buf.allocate(size*(is_cubemap?6:1));

            unsigned char *to_data=(unsigned char *)temp_buf.get_data();
            for(int i=0;i<(is_cubemap?6:1);++i)
            {
                bgra_to_rgba((const unsigned char *)(data_a[i]),to_data,size);
                data_a[i]=to_data;
                to_data+=size;
            }
        }
    }
  #endif
#endif

    if(active_layers[active_layer]>=0)
    {
        if(texture_obj::get(active_layers[active_layer]).gl_type!=gl_type)
            glBindTexture(texture_obj::get(active_layers[active_layer]).gl_type,0);
    }

    glBindTexture(gl_type,t.tex_id);
    active_layers[active_layer]= -1;

    const bool bad_alignment=!pot && (width*source_bpp/8)%4!=0;
    if(bad_alignment)
        glPixelStorei(GL_UNPACK_ALIGNMENT,1);

    gl_setup_texture(gl_type,!pot && !is_platform_restrictions_ignored(),has_mipmap);

    if(is_pvrtc)
        gl_setup_filtration(gl_type,has_mipmap,filter_linear,filter_linear,filter_nearest);

#ifdef OPENGL3
    if(format==greyscale)
    {
        const int swizzle[]={GL_RED,GL_RED,GL_RED,GL_ONE};
        glTexParameteriv(gl_type,GL_TEXTURE_SWIZZLE_RGBA,swizzle);
    }
#endif

#ifndef OPENGL_ES
    if(mip_count>1) //is_platform_restrictions_ignored() &&
        glTexParameteri(gl_type,GL_TEXTURE_MAX_LEVEL,mip_count-1);
#endif

  #if defined GL_GENERATE_MIPMAP && !defined OPENGL3
    if(has_mipmap && mip_count<0) glTexParameteri(gl_type,GL_GENERATE_MIPMAP,GL_TRUE);
  #endif

    for(int j=0;j<(is_cubemap?6:1);++j)
    {
        const char *data_pointer=data_a?(const char*)data_a[j]:0;
        unsigned int w=width,h=height;
        for(int i=0;i<(mip_count<=0?1:mip_count);++i,w=w>1?w/2:1,h=h>1?h/2:1)
        {
            const int gl_type=is_cubemap?cube_faces[j]:GL_TEXTURE_2D;
            unsigned int size=0;
            if(format<dxt1)
            {
                size=w*h*(source_bpp/8);
                if(need_create || is_cubemap)
                    glTexImage2D(gl_type,i,source_format,w,h,0,gl_format,precision,data_pointer);
                else
                    glTexSubImage2D(gl_type,i,0,0,w,h,gl_format,precision,data_pointer);
            }
            else
            {
                if(is_pvrtc)
                {
                    if(format==pvr_rgb2b || format==pvr_rgba2b)
                        size=((w>16?w:16)*(h>8?h:8)*2 + 7)/8;
                    else
                        size=((w>8?w:8)*(h>8?h:8)*4 + 7)/8;

                    if(size<32)
                        break;
                }
                else
                    size=(w>4?w:4)/4 * (h>4?h:4)/4 * source_bpp*2;

                glCompressedTexImage2D(gl_type,i,gl_format,w,h,0,size,data_pointer);
            }

            data_pointer+=size;
        }
    }

    if(bad_alignment)
        glPixelStorei(GL_UNPACK_ALIGNMENT,4);

  #if !defined GL_GENERATE_MIPMAP || defined OPENGL3
    #ifndef NO_EXTENSIONS_INIT
      static PFNGLGENERATEMIPMAPPROC glGenerateMipmap=NULL;
      if(!glGenerateMipmap)
          glGenerateMipmap=(PFNGLGENERATEMIPMAPPROC)get_extension("glGenerateMipmap");
    #endif
    if(has_mipmap && mip_count<0) glGenerateMipmap(gl_type);
  #endif

  #ifdef __ANDROID__
    temp_buf.free();
  #endif
    glBindTexture(gl_type,0);
    t.gl_format=gl_format;
#endif
    t.width=width;
    t.height=height;
    t.format=format;

    t.size=get_tex_memory_size(t.width,t.height,t.format,mip_count)*(is_cubemap?6:1);
    t.is_cubemap=is_cubemap;
    t.has_mipmaps=has_mipmap;

    return true;
}

bool texture::build_texture(const void *data,unsigned int width,unsigned int height,color_format format,int mip_count)
{
    const void *data_a[6]={data};

    return build_texture(data_a,false,width,height,format,mip_count);
}

bool texture::build_cubemap(const void *data[6],unsigned int width,unsigned int height,color_format format,int mip_count)
{
    return build_texture(data,true,width,height,format,mip_count);
}

void texture::update_region(const void *data,unsigned int x,unsigned int y,unsigned int width,unsigned int height,unsigned int mip)
{
    if(m_tex<0)
        return;

    const texture_obj &t=texture_obj::get(m_tex);
    if(x+width>t.width || y+height>t.height)
        return;

    if(!t.has_mipmaps && mip>0)
        return;

    if(t.format>=depth16)
        return;

#ifdef DIRECTX11
    //ToDo
#else
    glBindTexture(t.gl_type,t.tex_id);
    active_layers[active_layer]=m_tex;

    const int precision=(t.format==color_rgb32f || t.format==color_rgba32f)?GL_FLOAT:GL_UNSIGNED_BYTE;
    glTexSubImage2D(t.gl_type,mip,x,y,width,height,t.gl_format,precision,data);
#endif
}

void texture::bind(unsigned int layer) const { if(layer>=max_layers) return; current_layers[layer]=m_tex; }

void texture::unbind(unsigned int layer) { if(layer>=max_layers) return; current_layers[layer]= -1; }

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
            ID3D11ShaderResourceView * srv_null[]={0};
            get_context()->PSSetShaderResources(i,1,srv_null);
            ID3D11SamplerState *ss_null[]={0};
            get_context()->PSSetSamplers(i,1,ss_null);
            active_layers[i]= -1;
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
            active_layers[i]= -1;
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
        active_layers[i]=current_layers[i];
    }
}

bool texture::get_data(nya_memory::tmp_buffer_ref &data) const
{
    if(m_tex<0)
        return false;

    const texture_obj &tex=texture_obj::get(m_tex);
    unsigned int pitch=tex.width*get_bpp(tex.format)/8;
    unsigned int size=pitch*tex.height*(is_cubemap()?6:1);
    if(!size)
        return false;

#ifdef DIRECTX11
    if(!get_context() || !get_device())
        return false;

    if(tex.format>=dxt1)
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
    const char *from=(char *)resource.pData;
    char *to=(char *)data.get_data();
    for(int i=0;i<(is_cubemap()?6:1);++i)
    {
        for(unsigned int j=0;j<tex.height;++j)
        {
            memcpy(to,from,pitch);
            from+=resource.RowPitch;
            to+=pitch;
        }
    }

    get_context()->Unmap(copy_tex,0);
    copy_tex->Release();

    return true;
#else
    color_format format=tex.format;
    if(format>=dxt1)
    {
  #ifdef OPENGL_ES
        return false;
  #endif
        size=tex.width*tex.height*4*(is_cubemap()?6:1);
        format=color_bgra;
    }

    int gl_format=0;
    switch(tex.format)
    {
        case color_rgb:gl_format=GL_RGB;break;
        case color_rgba:gl_format=GL_RGBA;break;
  #ifndef __ANDROID__
        case color_bgra:gl_format=GL_BGRA;break;
  #endif
        case greyscale:gl_format=GL_LUMINANCE;break;
        default: return false;
    }

    data.allocate(size);

  #ifdef OPENGL_ES
    GLint prev_fbo=0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING,&prev_fbo);

    static GLuint copy_fbo=0;
    if(!copy_fbo)
        glGenFramebuffers(1,&copy_fbo);

    rect prev_vp=get_viewport();
    set_viewport(0,0,tex.width,tex.height);

    glBindFramebuffer(GL_FRAMEBUFFER,copy_fbo);
    if(is_cubemap())
    {
        unsigned int size=tex.width*tex.height*get_bpp(tex.format)/8;
        for(int i=0;i<6;++i)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,cube_faces[i],tex.tex_id,0);
            glReadPixels(0,0,tex.width,tex.height,gl_format,GL_UNSIGNED_BYTE,data.get_data(size*i));
        }
    }
    else
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,tex.tex_id,0);
        glReadPixels(0,0,tex.width,tex.height,gl_format,GL_UNSIGNED_BYTE,data.get_data());
    }
    glBindFramebuffer(GL_FRAMEBUFFER, prev_fbo);
    set_viewport(prev_vp);
  #else
    gl_select_multitex_layer(0);
    glBindTexture(tex.gl_type,tex.tex_id);
    active_layers[0]= -1;
    glGetTexImage(tex.gl_type,0,gl_format,GL_UNSIGNED_BYTE,data.get_data());
  #endif
#endif

    return true;
}

unsigned int texture::get_width() const { return m_tex<0?0:texture_obj::get(m_tex).width; }
unsigned int texture::get_height() const { return m_tex<0?0:texture_obj::get(m_tex).height; }
texture::color_format texture::get_color_format() const { return m_tex<0?color_rgb:texture_obj::get(m_tex).format; }

void texture::set_wrap(wrap s,wrap t)
{
    if(m_tex<0)
        return;

    const texture_obj &tex=texture_obj::get(m_tex);
    const bool pot=((tex.width&(tex.width-1))==0 && (tex.height&(tex.height-1))==0);
    if(!pot)
        s=t=wrap_clamp;

#ifndef DIRECTX11
    glBindTexture(tex.gl_type,tex.tex_id);
    for(int i=0;i<2;++i)
    {
        const int st=i==0?GL_TEXTURE_WRAP_S:GL_TEXTURE_WRAP_T;
        switch(s)
        {
            case wrap_clamp:glTexParameteri(tex.gl_type,st,GL_CLAMP_TO_EDGE);break;
            case wrap_repeat:glTexParameteri(tex.gl_type,st,GL_REPEAT);break;
            case wrap_repeat_mirror:glTexParameteri(tex.gl_type,st,GL_MIRRORED_REPEAT);break;
        }
    }

    active_layers[0]= -1;
#endif
}

void texture::set_aniso(unsigned int level)
{
    if(m_tex<0)
        return;

    texture_obj &tex=texture_obj::get(m_tex);

#ifdef DIRECTX11
    auto sdesc=dx_setup_filtration();
    if(level>0)
    {
        sdesc.Filter=D3D11_FILTER_ANISOTROPIC;
        sdesc.MaxAnisotropy=level;
    }

    get_device()->CreateSamplerState(&sdesc,&tex.sampler_state);
#else
    glBindTexture(tex.gl_type,tex.tex_id);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAX_ANISOTROPY_EXT,float(level));
    active_layers[0]= -1;
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
    active_layers[0]= -1;
#endif
}

void texture::set_default_filter(filter minification,filter magnification,filter mipmap)
{
    default_min_filter=minification;
    default_mag_filter=magnification;
    default_mip_filter=mipmap;
}

void texture::set_default_aniso(unsigned int level) { default_aniso=level; }

void texture::get_default_filter(filter &minification,filter &magnification,filter &mipmap)
{
    minification=default_min_filter;
    magnification=default_mag_filter;
    mipmap=default_mip_filter;
}

unsigned int texture::get_default_aniso() { return default_aniso; }

bool texture::is_cubemap() const
{
    if(m_tex<0)
        return false;

    return texture_obj::get(m_tex).is_cubemap;
}

bool texture::is_dxt_supported()
{
#ifdef DIRECTX11
    return true;
#else
  #ifdef OPENGL_ES
    return false;
  #else
    return true;
  #endif
#endif

//ToDo
/*
#else
    static bool checked=false,supported=false;
    if(!checked) checked=true,supported=has_extension("GL_EXT_texture_compression_s3tc");
    return supported;
#endif
*/
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

unsigned int texture::get_used_vmem_size()
{
    size_counter counter;
    texture_obj::get_texture_objs().apply_to_all(counter);

    return counter.size;
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
            ID3D11ShaderResourceView * srv_null[]={0};
            get_context()->PSSetShaderResources(i,1,srv_null);
            ID3D11SamplerState *ss_null[]={0};
            get_context()->PSSetSamplers(i,1,ss_null);
            active_layers[i]= -1;
        }

        if(current_layers[i]==m_tex)
            current_layers[i]= -1;
    }
#else
    for(unsigned int i=0;i<max_layers;++i)
    {
        if(active_layers[i]==m_tex)
        {
            gl_select_multitex_layer(i);
            glBindTexture(texture_obj::get(m_tex).gl_type,0);
            active_layers[i]= -1;
        }

        if(current_layers[i]==m_tex)
            current_layers[i]= -1;
    }
#endif
    texture_obj::remove(m_tex);
    m_tex= -1;
}

}
