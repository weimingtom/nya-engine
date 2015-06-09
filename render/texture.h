//https://code.google.com/p/nya-engine/

#pragma once

namespace nya_memory { class tmp_buffer_ref; }

namespace nya_render
{

class texture
{
    friend class fbo;

public:
    enum color_format
    {
        color_rgb,
        color_rgba,
        color_bgra,
        //color_r,
        greyscale,

        color_rgb32f,
        color_rgba32f,

        depth16,
        depth24,
        depth32,

        dxt1,
        dxt3,
        dxt5,

        dxt2=dxt3,
        dxt4=dxt5,

        etc1,
        etc2,
        etc2_eac,
        etc2_a1,

        pvr_rgb2b,
        pvr_rgb4b,
        pvr_rgba2b,
        pvr_rgba4b
    };

    static bool is_dxt_supported();

public:
    //mip_count= -1 means "generate mipmaps". You have to provide a single mip or a complete mipmap pyramid instead
    bool build_texture(const void *data,unsigned int width,unsigned int height,color_format format,
                       int mip_count= -1);

	//order: positive_x,negative_x,positive_y,negative_y,positive_z,negative_z
	bool build_cubemap(const void *data[6],unsigned int width,unsigned int height,color_format format,
                       int mip_count= -1);
public:
    bool update_region(const void *data,unsigned int x,unsigned int y,unsigned int width,unsigned int height,unsigned int mip=0);

public:
    void bind(unsigned int layer) const;
    static void unbind(unsigned int layer);

    static void apply(bool ignore_cache=false);

public:
    enum wrap
    {
        wrap_clamp,
        wrap_repeat,
        wrap_repeat_mirror
    };

    void set_wrap(wrap s,wrap t);

    enum filter
    {
        filter_nearest,
        filter_linear
    };

    void set_filter(filter minification,filter magnification,filter mipmap);
    void set_aniso(unsigned int level);
    static void set_default_filter(filter minification,filter magnification,filter mipmap);
    static void set_default_aniso(unsigned int level);

public:
    bool get_data( nya_memory::tmp_buffer_ref &data ) const;
    unsigned int get_width() const;
    unsigned int get_height() const;
    color_format get_color_format() const;
    bool is_cubemap() const;

    static void get_default_filter(filter &minification,filter &magnification,filter &mipmap);
    static unsigned int get_default_aniso();

public:
    void release();

public:
    static unsigned int get_used_vmem_size();

public:
    texture(): m_tex(-1) {}

private:
    bool build_texture(const void *data[6],bool is_cubemap,unsigned int width,unsigned int height,
                       color_format format,int mip_count= -1);

private:
    int m_tex;
};

}
