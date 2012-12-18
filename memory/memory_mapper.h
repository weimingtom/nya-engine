//https://code.google.com/p/nya-engine/

#pragma once

#include <cstddef>

namespace nya_memory
{

template<typename type>
class array
{
    friend class memory_mapper;

public:
    type &get(int idx) const
    {
        return m_data[idx];
    }

    unsigned long get_count() const
    {
        return m_count;
    }

    array(): m_count(0), m_data(0) {}

private:
    unsigned long m_count;
    type *m_data;
};

class memory_mapper
{
public:
	template<typename t>
    t *read()
	{
		char *tmp=m_data;
		tmp+=m_offset;

		m_offset+=sizeof(t);
		if(m_offset>=m_size)
			return 0;

        return (t*)tmp;
	}

	template<typename type,typename count_type>
	array<type> read_array()
	{
	    array<type> a;

	    count_type *c=read<count_type>();
	    if(!c)
            return a;

        count_type count=*c;

        if(count==0)
            return a;

		char *tmp=m_data;
		tmp+=m_offset;

		m_offset+=sizeof(sizeof(type)*count);
		if(m_offset>=m_size)
			return a;

        a.m_count=count;
        a.m_data=(type*)tmp;

	    return a;
	}
/*
	template<typename t>
	bool map(t*&m)
	{
		char *tmp=m_data;
		tmp+=m_offset;

		m_offset+=sizeof(t);
		if(m_offset>=m_size)
		{
			m=0;
			return false;
		}

		m=(t*)tmp;
		return true;
	}
*/
	unsigned int read_uint()
	{
		char *tmp=m_data;
		tmp+=m_offset;

		m_offset+=sizeof(unsigned int);
		if(m_offset>=m_size)
			return 0;

		return *(unsigned int*)tmp;
	}

	memory_mapper(void *data,size_t size)
	{
		m_offset=0;
		if(data && size)
		{
			m_data=(char*)data;
			m_size=size;
		}
		else
		{
			m_data=0;
			m_size=0;
		}
	}

private:
	char *m_data;
	size_t m_offset;
	size_t m_size;
};

}
