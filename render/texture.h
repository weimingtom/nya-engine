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
        etc2_a1
    };

    static bool is_dxt_supported();

public:
    //mip_count=-1 means "generate mipmaps". You have to provide a single mip or a complete mipmap pyramid instead
    bool build_texture(const void *data,unsigned int width,unsigned int height,color_format format,int mip_count=-1,unsigned int mip_padding=0);

	//order: positive_x,negative_x,positive_y,negative_y,positive_z,negative_z
	bool build_cubemap(const void *data[6],unsigned int width,unsigned int height,color_format format);
	bool build_cubemap(const void *data,unsigned int width,unsigned int height,color_format format);
	//bool build_cubemap(std::nullptr_t data,unsigned int width,unsigned int height,color_format format);

public:
    void bind(unsigned int layer) const;
    static void unbind(unsigned int layer);

    static void apply(bool ignore_cache=false);

public:
    void set_wrap(bool repeat_s,bool repeat_t);

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
    unsigned int get_width() const { return m_width; }
    unsigned int get_height() const { return m_height; }
    color_format get_color_format() const { return m_format; }
    bool is_cubemap() const;

    static void get_default_filter(filter &minification,filter &magnification,filter &mipmap);
    static unsigned int get_default_aniso();

public:
    void release();

public:
    static unsigned int get_used_vmem_size();

public:
    texture(): m_tex(-1),m_width(0),m_height(0) {}

private:
    int m_tex;

    unsigned int m_width;
    unsigned int m_height;

	color_format m_format;
};

}
