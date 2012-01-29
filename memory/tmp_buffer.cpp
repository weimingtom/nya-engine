//https://code.google.com/p/nya-engine/

#include "tmp_buffer.h"

#include <vector>

namespace memory
{

class tmp_buffer
{
public:
    bool is_used()
    {
        return m_ref_count > 0;
    }

    void allocate(size_t size)
    {
        if(size>m_data.size())
            m_data.resize(size);
        
        m_size = size;
        
        m_offset = 0;

        ++m_ref_count;
    }

    size_t get_size()
    {
        return m_size;
    }

    void add_data(const void*data,size_t size)
    {
        const size_t new_offset = m_offset+size;
        if(m_size<new_offset)
            return;

        memcpy(&m_data[m_offset],data,size);

        m_offset=new_offset;
    }

    void free()
    {
        if(m_ref_count)
            --m_ref_count;
    }

    void* get_data(size_t offset)
    {
        if(offset>=m_size)
            return 0;
        
        return &m_data[offset];
    }

    tmp_buffer(): m_ref_count(0), m_size(0) {}

private:
    std::vector<char> m_data;
    unsigned int m_ref_count;
    size_t m_size;
    size_t m_offset;
};

class tmp_buffer_allocator
{
public:
    unsigned int allocate(size_t size)
    {
        for(int i=0;i<m_buffers.size();++i)
        {
            tmp_buffer & buffer = m_buffers[i];
            if(!buffer.is_used())
            {
                buffer.allocate(size);
                return i;
            }
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

tmp_buffer_allocator &get_allocator()
{
    static tmp_buffer_allocator allocator;
    return allocator;
}

tmp_buffer &get_buffer(unsigned int buf_idx)
{
    return get_allocator().get_buffer(buf_idx);
}
    
void tmp_buffer_ref::add_data(void*data,size_t size)
{
    get_buffer(m_buf_idx).add_data(data,size);
}

void *tmp_buffer_ref::get_data(size_t offset)
{
    return get_buffer(m_buf_idx).get_data(offset);
}

tmp_buffer_ref::tmp_buffer_ref(size_t size): m_buf_idx(get_allocator().allocate(size)) {}

tmp_buffer_ref::~tmp_buffer_ref()
{
    get_buffer(m_buf_idx).free();
}

}
