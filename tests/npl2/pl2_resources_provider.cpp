//https://code.google.com/p/nya-engine/

#include "pl2_resources_provider.h"
#include "memory/pool.h"
#include "memory/tmp_buffer.h"
#include <string>
#include <memory.h>

namespace
{
    typedef unsigned int uint;
    typedef unsigned short ushort;
    typedef unsigned char uchar;
    
    struct pl2_entry
    {
        char name[32];
        uint offset;
        uint packed_size;
        uint size;
        uint align_bytes;
    };
}

namespace nya_resources
{

class pl2_resource: public resource_data
{
public:
    size_t get_size() const { return m_data.get_size(); }

    bool read_all(void*data) const
    {
        return m_data.copy_from(data,m_data.get_size());
    }

    bool read_chunk(void *data,size_t size,size_t offset) const
    {
        return m_data.copy_from(data,size,offset);
    }

public:
    bool decompress(const void*data,size_t packed_size,size_t size);
    void release() { m_data.free(); }

private:
    nya_memory::tmp_buffer_ref m_data;
};

struct pl2_entry_info: public resource_info
{
public:
    std::string name;
    uint offset;
    uint packed_size;
    uint size;
    resource_data *archieve_data;

    pl2_entry_info *next;

public:
    pl2_entry_info(): offset(0), packed_size(0), size(0), archieve_data(0), next(0) {}

public:
    resource_data *access();

private:
    const char *get_name() const { return name.c_str(); };
    bool check_extension(const char *ext) const
    {
        if(!ext)
            return false;

        std::string ext_str(ext);
        return (name.size() >= ext_str.size() && 
                std::equal(name.end()-ext_str.size(),name.end(),ext_str.begin()));
    }

    resource_info *get_next() const { return next; };
};

}

namespace
{
    nya_memory::pool<nya_resources::pl2_resource,8> pl2_resources;
    nya_memory::pool<nya_resources::pl2_entry_info,32> entries;
}

namespace nya_resources
{

bool pl2_resources_provider::open_archieve(resource_data *archieve_data)
{
    if(m_archieve_data)
        close_archieve();

    if(!archieve_data)
        return false;

    const size_t data_size = archieve_data->get_size();
    const size_t header_size = sizeof(uint)*4;
    const size_t entry_size = sizeof(pl2_entry);

    if(data_size<header_size+entry_size)
        return false;

    pl2_entry first_entry;
    archieve_data->read_chunk(&first_entry,entry_size,header_size);

    m_attribute = entries.allocate();
    m_attribute->name.assign(first_entry.name);
    m_attribute->size=first_entry.size;
    m_attribute->offset=first_entry.offset;
    m_attribute->packed_size=first_entry.packed_size;
    m_attribute->archieve_data=archieve_data;

    const uint ecount=(first_entry.offset-16)/entry_size;
    pl2_entry_info *last=0;

    if(ecount>1)
    {
        const uint last_entries_size = (ecount-1)*entry_size;
        if(data_size<header_size+entry_size+last_entries_size)
            return false;

        nya_memory::tmp_buffer_scoped tmp(last_entries_size);
        archieve_data->read_chunk(tmp.get_data(),last_entries_size,header_size+entry_size);

        pl2_entry *last_entries=(pl2_entry *)tmp.get_data();
        for(int i=ecount-2;i>=0;--i)
        {
            pl2_entry &from = last_entries[i];
            pl2_entry_info *to = entries.allocate();

            to->next=last;
            to->name.assign(from.name);
            to->size=from.size;
            to->offset=from.offset;
            to->packed_size=from.packed_size;
            to->archieve_data=archieve_data;

            last=to;
        }
    }

    m_entries=last;

    return true;
}

void pl2_resources_provider::close_archieve()
{
    if(m_archieve_data)
        m_archieve_data->release();

    pl2_entry_info *entry=m_entries;
    while(entry)
    {
        pl2_entry_info *next=entry->next;
        entries.free(entry);
        entry=next;
    }

    if(m_attribute)
        entries.free(m_attribute);

    m_entries=0;
    m_attribute=0;
}

resource_info *pl2_resources_provider::first_res_info()
{
    return m_entries;
}

resource_data *pl2_resources_provider::access(const char *resource_name)
{
    if(!resource_name)
    {
        get_log()<<"unable to access archieve entry: invalid name\n";
        return 0;
    }

    pl2_entry_info *entry=m_entries;
    while(entry)
    {
        if(entry->name.compare(resource_name)==0)
            return entry->access();

        entry=entry->next;
    }

    get_log()<<"unable to access archieve entry: not found\n";

    return 0;
}

resource_data *pl2_resources_provider::access_attribute()
{
    if(!m_attribute)
        return 0;

    return m_attribute->access();
}

resource_data *pl2_entry_info::access()
{
    if(!archieve_data || !packed_size || !size)
    {
        get_log()<<"unable to access archieve entry: invalid size\n";
        return 0;
    }

    nya_memory::tmp_buffer_scoped packed(packed_size);

    if(!archieve_data->read_chunk(packed.get_data(),packed_size,offset))
    {
        get_log()<<"unable to access archieve entry: invalid data\n";
        return 0;
    }

    pl2_resource *data = pl2_resources.allocate();

    if(!data->decompress(packed.get_data(),packed_size,size))
    {
        pl2_resources.free(data);
        get_log()<<"unable to access archieve entry: decompress failed\n";
        return 0;
    }

    return data;
}

bool pl2_resource::decompress(const void*data,size_t packed_size,size_t size)
{
    if(!data || !packed_size || !size)
        return false;

    m_data.allocate(size);
    const uchar *src=(const uchar*)data;
    uchar *dst=(uchar*)m_data.get_data();
    if(!dst)
        return false;

    const size_t lzbuf_size=4096;
    uchar lzbuf[lzbuf_size]={0};

    uchar flags=src[0], mask=1;
    const uchar offset_mask=0xf0;
    const uchar length_mask=0x0f;

    size_t src_pos=0, dst_pos=0;
    while(++src_pos < packed_size)
    {
        if (flags & mask)
            dst[dst_pos++]=lzbuf[dst_pos%lzbuf_size]=src[src_pos];
        else
        {
            const uchar b[]={src[src_pos],src[++src_pos]};
            size_t offset=(((b[1] & offset_mask)<<4)|b[0])+18;
            for(size_t i=0;i<(b[1] & length_mask)+3;++i,++offset)
                dst[dst_pos++]=lzbuf[dst_pos%lzbuf_size]=lzbuf[offset%lzbuf_size];
        }

        if(!(mask += mask))
        {
            mask=1;
            if(++src_pos < packed_size)
                flags=src[src_pos];
        }
    }

    return true;
}

}
