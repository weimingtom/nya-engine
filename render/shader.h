//https://code.google.com/p/nya-engine/

#ifndef shader_h
#define shader_h

namespace nya_render
{

class shader
{
public:
	enum program_type
	{
		vertex,
		pixel,
		geometry,
		tesselation,
		program_types_count
	};

	void add_program(program_type type,const char*code);

public:
	void bind();
	void unbind();

public:
	void set_sampler(const char*name,unsigned int layer);

public:
	int get_handler(const char*name);
	void set_uniform(unsigned int i,float f0,float f1=0.0f,float f2=0.0f,float f3=0.0f);
	void set_uniform4_array(unsigned int i,const float *f,unsigned int count);
	void set_uniform16_array(unsigned int i,const float *f,unsigned int count,bool transpose=false);

public:
	void release();

public:
	shader(): m_program(0)
	{
		for(int i = 0;i<program_types_count;++i)
			m_objects[i]=0;
	}

private:
	void* m_program;
	void* m_objects[program_types_count];
};

}

#endif

