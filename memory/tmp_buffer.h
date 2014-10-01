//https://code.google.com/p/nya-engine/

#pragma once

#include <cstddef>

//Note: many buffers are not supposed to be opened at the same time

namespace nya_memory
{

class tmp_buffer;

class tmp_buffer_ref
{
public:
    bool copy_from(void*data,size_t size,size_t offset=0) const; //from buffer to data
    bool copy_to(const void*data,size_t size,size_t offset=0); //from data to buffer

public:
    void *get_data(size_t offset=0) const;
    size_t get_size() const;

public:
    void allocate(size_t size);
    void free();

public:
    tmp_buffer_ref(): m_buf(0) {}
    tmp_buffer_ref(size_t size)
    {
        m_buf=0;
        allocate(size);
    }

private:
    tmp_buffer* m_buf;
};

class tmp_buffer_scoped
{
public:
    bool copy_from(void*data,size_t size,size_t offset=0) const; //from buffer to data
    bool copy_to(const void*data,size_t size,size_t offset=0); //from data to buffer

public:
    void *get_data(size_t offset=0) const;
    size_t get_size() const;

public:
    tmp_buffer_scoped(size_t size);
    ~tmp_buffer_scoped();

    // non copyable
private:
    tmp_buffer_scoped(const tmp_buffer_scoped &);
    tmp_buffer_scoped &operator=(const tmp_buffer_scoped &);

private:
    tmp_buffer *m_buf;
};

namespace tmp_buffers
{
    void force_free();
    size_t get_total_size();
}

}
