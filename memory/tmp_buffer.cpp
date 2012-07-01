//https://code.google.com/p/nya-engine/

#include "tmp_buffer.h"
#include "memory.h"
#include <memory.h>
#include <vector>
#include <list>

namespace nya_memory
{

class tmp_buffer
{
private:
    bool is_used() const
    {
        return m_ref_count > 0;
    }

    void allocate(size_t size)
    {
        if(size>m_data.size())
        {
            get_log()<<"tmp buf resized from "<<m_data.size()<<" to "<<size<<"\n";
            m_data.resize(size);
        }

        m_size = size;

        ++m_ref_count;
    }

    size_t get_actual_size() const
    {
        return m_data.size();
    }

public:
    size_t get_size() const
    {
        return m_size;
    }

    void free()
    {
        if(m_ref_count)
            --m_ref_count;
    }

    void *get_data(size_t offset)
    {
        if(offset>=m_size)
            return 0;

        return &m_data[offset];
    }

    const void *get_data(size_t offset) const
    {
        if(offset>=m_size)
            return 0;

        return &m_data[offset];
    }

    bool copy_from(void *data,size_t size,size_t offset) const
    {
        if(size+offset>m_size)
            return false;

        memcpy(data,&m_data[offset],size);
        return true;
    }

    bool copy_to(const void *data,size_t size,size_t offset)
    {
        if(size+offset>m_size)
            return false;

        memcpy(&m_data[offset],data,size);
        return true;
    }

    static tmp_buffer *allocate_new(size_t size)
    {
        typedef std::list<tmp_buffer> buffers_list;
        static std::list<tmp_buffer> m_buffers;

        tmp_buffer* first_free=0;
        buffers_list::iterator it=m_buffers.begin();
        while(it!=m_buffers.end())
        {
            tmp_buffer &buffer = *it;
            if(!buffer.is_used())
            {
                if(buffer.get_actual_size()<=size)
                {
                    buffer.allocate(size);
                    return &buffer;
                }

                if(!first_free)
                    first_free = &buffer;
            }
            
            ++it;
        }

        if(first_free)
        {
            first_free->allocate(size);
            return first_free;
        }

        m_buffers.push_back(tmp_buffer());
        m_buffers.back().allocate(size);

        get_log()<<"new tmp buf allocated ("<<m_buffers.size()<<" total)\n";

        return &m_buffers.back();
    }

    tmp_buffer(): m_ref_count(0), m_size(0) {}

private:
    std::vector<char> m_data;
    unsigned int m_ref_count;
    size_t m_size;
};

void *tmp_buffer_ref::get_data(size_t offset)
{
    if(!m_buf)
        return 0;

    return m_buf->get_data(offset);
}

size_t tmp_buffer_ref::get_size() const
{
    if(!m_buf)
        return 0;

    return m_buf->get_size();
}

bool tmp_buffer_ref::copy_from(void*data,size_t size,size_t offset) const
{
    if(!m_buf)
        return false;

    return m_buf->copy_from(data,size,offset);
}

bool tmp_buffer_ref::copy_to(const void*data,size_t size,size_t offset)
{
    if(!m_buf)
        return false;

    return m_buf->copy_to(data,size,offset);
}

void tmp_buffer_ref::allocate(size_t size)
{
    if(m_buf)
        m_buf->free();

    m_buf=m_buf->allocate_new(size);
}

void tmp_buffer_ref::free()
{
    if(!m_buf)
        return;

    m_buf->free();
    m_buf=0;
}

void *tmp_buffer_scoped::get_data(size_t offset)
{
    return m_buf->get_data(offset);
}

bool tmp_buffer_scoped::copy_from(void*data,size_t size,size_t offset) const
{
    return m_buf->copy_from(data,size,offset);
}

bool tmp_buffer_scoped::copy_to(const void*data,size_t size,size_t offset)
{
    return m_buf->copy_to(data,size,offset);
}

tmp_buffer_scoped::tmp_buffer_scoped(size_t size): m_buf(m_buf->allocate_new(size)) {}

tmp_buffer_scoped::~tmp_buffer_scoped()
{
    m_buf->free();
}

}
