//https://code.google.com/p/nya-engine/

#ifndef tmp_buffer_h
#define tmp_buffer_h

#include <cstddef>

//Note: many buffers are not supposed to be opened at the same time

namespace nya_memory
{

class tmp_buffer_ref
{
public:
    bool copy_from(void*data,size_t size,size_t offset=0) const;
    bool copy_to(const void*data,size_t size,size_t offset=0);

public:
    void *get_data(size_t offset=0);
    size_t get_size() const;

public:
    void allocate(size_t size);
    void free();

public:
    tmp_buffer_ref(): m_buf_idx(-1) {}

private:
    int m_buf_idx;
};

class tmp_buffer_scoped
{
public:
    bool copy_from(void*data,size_t size,size_t offset=0) const;
    bool copy_to(const void*data,size_t size,size_t offset=0);

public:
    void *get_data(size_t offset=0);

public:
    tmp_buffer_scoped(size_t size);
    ~tmp_buffer_scoped();

private:
    const unsigned int m_buf_idx;
};

}
#endif

