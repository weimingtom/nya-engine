//https://code.google.com/p/nya-engine/

#pragma once

#include <cstddef>
#include <string.h>

namespace nya_memory
{

class memory_writer
{
public:
    template<typename t> bool write(const t&v) { return write(&v,sizeof(t)); }
    bool write_int(int v) { return write(&v,sizeof(v)); }
    bool write_uint(unsigned int v) { return write(&v,sizeof(v)); }
    bool write_ushort(unsigned short v) { return write(&v,sizeof(v)); }
    bool write_ubyte(unsigned char v) { return write(&v,sizeof(v)); }
    bool write_float(float v) { return write(&v,sizeof(v)); }
    template<typename t=unsigned short> bool write_string(const std::string &s)
    {
        t len=(t)s.length();
        return write(&len,sizeof(t)) && write(s.c_str(),len);
    }

    bool write(const void *data,size_t size)
    {
        if(!data || !size || size>m_size-m_offset)
            return false;

        memcpy(m_data+m_offset,data,size);
        m_offset+=size;
        return true;
    }

    bool seek(size_t offset)
    {
        if(offset>=m_size)
        {
            m_offset=m_size;
            return false;
        }

        m_offset=offset;
        return true;
    }

    size_t get_offset() { return m_offset; }

public:
    memory_writer(void *data,size_t size)
    {
        if(data)
        {
            m_data=(char*)data;
            m_size=size;
        }
        else
        {
            m_data=0;
            m_size=0;
        }

        m_offset=0;
    }

private:
    char *m_data;
    size_t m_size;
    size_t m_offset;
};

}
