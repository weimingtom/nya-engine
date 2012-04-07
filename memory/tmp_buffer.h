//https://code.google.com/p/nya-engine/

#ifndef tmp_buffer_h
#define tmp_buffer_h

#include <cstddef>

namespace nya_memory
{

class tmp_buffer_ref
{
public:
    void add_data(void*data,size_t size);
    void copy_data(void*data,size_t size,size_t offset);

    void *get_data(size_t offset=0);

public:
    tmp_buffer_ref(size_t size);
    ~tmp_buffer_ref();

private:
    const unsigned int m_buf_idx;
};

}
#endif

