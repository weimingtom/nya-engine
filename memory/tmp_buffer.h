//https://code.google.com/p/nya-engine/

#ifndef tmp_buffer_h
#define tmp_buffer_h

#include <cstddef>

namespace nya_memory
{

class tmp_buffer_ref
{
public:
    template<typename t_data>void add_data(const t_data &data)
    {
        add_data((void*)&data,sizeof(data));
    }

    void *get_data(size_t offset=0);

    template<typename t_data>void copy_data(t_data &data,size_t offset=0)
    {
        copy_data((void*)&data,sizeof(data),offset);
    }

public:
    tmp_buffer_ref(size_t size);
    ~tmp_buffer_ref();

private:
    void add_data(void*data,size_t size);
    void copy_data(void*data,size_t size,size_t offset);

private:
    const unsigned int m_buf_idx;
};

}
#endif

