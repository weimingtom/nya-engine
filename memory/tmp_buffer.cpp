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
        return m_used;
    }

    void allocate(size_t size)
    {
        if(size>m_data.size())
        {
            get_log()<<"tmp buf resized from "<<m_data.size()<<" to "<<size<<", ";
            m_data.resize(size);
            get_log()<<get_total_size()<<" in "<<m_buffers.size()<<" buffers total)\n";
        }

        m_size = size;

        m_used=true;
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
        m_size=0;
        m_used=false;
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
        tmp_buffer* min_suit_buf=0;
        tmp_buffer* max_buf=0;

        for(buffers_list::iterator it=m_buffers.begin();it!=m_buffers.end();++it)
        {
            tmp_buffer &buffer = *it;
            if(buffer.is_used())
                continue;

            if(min_suit_buf)
            {
                if(buffer.get_actual_size()>=size && buffer.get_actual_size()< min_suit_buf->get_actual_size())
                    min_suit_buf=&buffer;
            }
            else
                min_suit_buf=&buffer;

            if(max_buf)
            {
                if(buffer.get_actual_size() > max_buf->get_actual_size())
                    max_buf=&buffer;
            }
            else
                max_buf=&buffer;
        }

        if(min_suit_buf)
        {
            min_suit_buf->allocate(size);
            return min_suit_buf;
        }

        if(max_buf)
        {
            max_buf->allocate(size);
            return max_buf;
        }

        m_buffers.push_back(tmp_buffer());
        m_buffers.back().allocate(size);

        get_log()<<"new tmp buf allocated ("<<m_buffers.size()<<" total)\n";

        return &m_buffers.back();
    }

    static void force_free()
    {
        for(buffers_list::iterator it=m_buffers.begin();it!=m_buffers.end();++it)
        {
            tmp_buffer &buffer = *it;
            if(buffer.is_used())
                continue;

            std::vector<char>().swap(buffer.m_data);
        }
    }

    static size_t get_total_size()
    {
        size_t size=0;
        for(buffers_list::iterator it=m_buffers.begin();it!=m_buffers.end();++it)
        {
            tmp_buffer &buffer = *it;
            size+=buffer.m_data.size();
        }

        return size;
    }

    tmp_buffer(): m_used(false), m_size(0) {}

private:
    std::vector<char> m_data;
    bool m_used;
    size_t m_size;

private:
    typedef std::list<tmp_buffer> buffers_list;
    static std::list<tmp_buffer> m_buffers;
};

tmp_buffer::buffers_list tmp_buffer::m_buffers;

void tmp_buffers::force_free()
{
    tmp_buffer::force_free();
}

size_t tmp_buffers::get_total_size()
{
    return tmp_buffer::get_total_size();
}

void *tmp_buffer_ref::get_data(size_t offset) const
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

    m_buf=tmp_buffer::allocate_new(size);
}

void tmp_buffer_ref::free()
{
    if(!m_buf)
        return;

    m_buf->free();
    m_buf=0;
}

void *tmp_buffer_scoped::get_data(size_t offset) const
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

tmp_buffer_scoped::tmp_buffer_scoped(size_t size): m_buf(tmp_buffer::allocate_new(size)) {}

tmp_buffer_scoped::~tmp_buffer_scoped()
{
    m_buf->free();
}

}
