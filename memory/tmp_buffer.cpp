//https://code.google.com/p/nya-engine/

#include "tmp_buffer.h"
#include <memory.h>
#include <vector>

namespace nya_memory
{

class tmp_buffer
{
public:
    bool is_used() const
    {
        return m_ref_count > 0;
    }

    void allocate(size_t size)
    {
        if(size>m_data.size())
            m_data.resize(size);

        m_size = size;

        ++m_ref_count;
    }

    size_t get_size() const
    {
        return m_size;
    }

    size_t get_actual_size() const
    {
        return m_data.size();
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

    tmp_buffer(): m_ref_count(0), m_size(0) {}

private:
    std::vector<char> m_data;
    unsigned int m_ref_count;
    size_t m_size;
};

class tmp_buffer_allocator
{
public:
    unsigned int allocate(size_t size)
    {
        int first_free = -1;
        for(unsigned int i=0;i<m_buffers.size();++i)
        {
            tmp_buffer & buffer = m_buffers[i];
            if(!buffer.is_used())
            {
                if(buffer.get_actual_size()<=size)
                {
                    buffer.allocate(size);
                    return i;
                }

                if(first_free<0)
                    first_free = i;
            }
        }

        if(first_free>=0)
        {
            m_buffers[first_free].allocate(size);
            return first_free;
        }

        m_buffers.push_back(tmp_buffer());
        m_buffers.back().allocate(size);

        return (unsigned int)(m_buffers.size() - 1);
    }

    tmp_buffer &get_buffer(unsigned int i)
    {
        return m_buffers[i];
    }

private:
    std::vector<tmp_buffer> m_buffers;
};

}

namespace
{

nya_memory::tmp_buffer_allocator &get_allocator()
{
    static nya_memory::tmp_buffer_allocator allocator;
    return allocator;
}

nya_memory::tmp_buffer &get_buffer(unsigned int buf_idx)
{
    return get_allocator().get_buffer(buf_idx);
}

}

namespace nya_memory
{

void *tmp_buffer_ref::get_data(size_t offset)
{
    if(m_buf_idx<0)
        return 0;

    return get_buffer(m_buf_idx).get_data(offset);
}

size_t tmp_buffer_ref::get_size() const
{
    if(m_buf_idx<0)
        return 0;

    return get_buffer(m_buf_idx).get_size();
}

bool tmp_buffer_ref::copy_from(void*data,size_t size,size_t offset) const
{
    if(m_buf_idx<0)
        return false;

    return get_buffer(m_buf_idx).copy_from(data,size,offset);
}

bool tmp_buffer_ref::copy_to(const void*data,size_t size,size_t offset)
{
    if(m_buf_idx<0)
        return false;

    return get_buffer(m_buf_idx).copy_to(data,size,offset);
}

void tmp_buffer_ref::allocate(size_t size)
{
    m_buf_idx=get_allocator().allocate(size);
}

void tmp_buffer_ref::free()
{
    if(m_buf_idx<0)
        return;

    get_buffer(m_buf_idx).free();
    m_buf_idx=-1;
}


void *tmp_buffer_scoped::get_data(size_t offset)
{
    return get_buffer(m_buf_idx).get_data(offset);
}

bool tmp_buffer_scoped::copy_from(void*data,size_t size,size_t offset) const
{
    return get_buffer(m_buf_idx).copy_from(data,size,offset);
}

bool tmp_buffer_scoped::copy_to(const void*data,size_t size,size_t offset)
{
    return get_buffer(m_buf_idx).copy_to(data,size,offset);
}

tmp_buffer_scoped::tmp_buffer_scoped(size_t size): m_buf_idx(get_allocator().allocate(size)) {}

tmp_buffer_scoped::~tmp_buffer_scoped()
{
    get_buffer(m_buf_idx).free();
}

}
