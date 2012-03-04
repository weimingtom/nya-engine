//https://code.google.com/p/nya-engine/

#include "file_resources_provider.h"
#include "stdio.h"

namespace resources
{

class file_resource: public resource_data
{
public:
    bool is_valid() { return m_file!=0; }
	size_t get_size() { return m_size; }

	bool read_all(void*data);
	bool read_chunk(void *data,size_t size,size_t offset);

public:
	bool open(const char*filename);
	void close();

	file_resource(): m_file(0), m_size(0) {}

private:
	FILE *m_file;
	size_t m_size;
};

resource_data *file_resources_provider::access(const char *resource_name)
{
	file_resource *file = new file_resource;
	if(!file)
		return 0;

	if(!file->open(resource_name))
		return 0;

	return file;
}

void file_resources_provider::close(resource_data *res)
{
	if(!res)
		return;

	delete res;
}

bool file_resource::read_all(void*data)
{
	if(!data||m_file)
		return false;

	fseek(m_file,0,SEEK_SET);
	fread(data,1,m_size,m_file);

	return true;
}

bool file_resource::read_chunk(void *data,size_t size,size_t offset)
{
	if(!data||!m_file)
		return false;

	if(offset+size>m_size||!size)
		return false;

	fseek(m_file,offset,SEEK_SET);
	fread(data,1,size,m_file);

	return true;
}

bool file_resource::open(const char*filename)
{
	if(m_file)
	{
		fclose(m_file);
	}

	m_file=fopen(filename,"rb");
	if(!m_file)
	{
		m_size=0;
		return false;
	}

	fseek(m_file,0,SEEK_END);
	m_size=ftell(m_file);

	return true;
}

void file_resource::close()
{
	fclose(m_file);
}

}
