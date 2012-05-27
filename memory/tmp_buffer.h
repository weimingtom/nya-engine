//https://code.google.com/p/nya-engine/

#ifndef tmp_buffer_h
#define tmp_buffer_h

#include <cstddef>

//Note: many buffers are not supposed to be opened at the same time

namespace nya_memory
{

class tmp_buffer;

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
    tmp_buffer_ref(): m_buf(0) {}

private:
    tmp_buffer* m_buf;
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
    tmp_buffer *m_buf;
};

}
#endif

