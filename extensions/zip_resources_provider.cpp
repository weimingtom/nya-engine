//https://code.google.com/p/nya-engine/

#include "zip_resources_provider.h"
#include "memory/tmp_buffer.h"
#include "memory/memory_reader.h"
#include "zlib.h"

//ToDo: log

namespace nya_resources
{

inline std::string fix_name(const char *name)
{
    if(!name)
        return std::string();

    std::string name_str(name);
    for(size_t i=0;i<name_str.size();++i)
    {
        if(name_str[i]=='\\')
            name_str[i]='/';
    }

    std::string out;
    char prev=0;
    for(size_t i=0;i<name_str.size();++i)
    {
        if(prev=='/' && name_str[i]=='/')
            continue;
        
        prev=name_str[i];
        out.push_back(prev);
    }
    
    return out;
}

bool zip_resources_provider::open_archive(const char *archive_name)
{
    if(!archive_name)
        return false;

    return open_archive(nya_resources::get_resources_provider().access(archive_name));
}

bool zip_resources_provider::open_archive(nya_resources::resource_data *data)
{
    if(!data)
        return false;

    typedef unsigned int uint;
    typedef unsigned short ushort;

    const size_t data_size=data->get_size();
    if(data_size<22)
        return false;

    size_t sign_offset=data_size-22;

    uint sign;
    if(!data->read_chunk(&sign,4,sign_offset))
        return false;

    if(sign!=0x06054b50)
        return false; //ToDo: comment ahead, search for the sign

    struct { uint size,offset; } dir_size_offset;

    if(!data->read_chunk(&dir_size_offset,sizeof(dir_size_offset),sign_offset+12))
        return false;

    nya_memory::tmp_buffer_scoped dir_buf(dir_size_offset.size);

    if(!data->read_chunk(dir_buf.get_data(),dir_buf.get_size(),dir_size_offset.offset))
        return false;

    nya_memory::memory_reader reader(dir_buf.get_data(),dir_buf.get_size());
    while(reader.get_remained())
    {
        const uint pk_sign=0x02014b50;
        if(!reader.test(&pk_sign,sizeof(pk_sign)))
            break;

        zip_entry entry;

        reader.skip(6);
        entry.compression=reader.read<ushort>();
        reader.skip(8);
        entry.packed_size=reader.read<uint>();
        entry.unpacked_size=reader.read<uint>();

        const ushort file_name_len=reader.read<ushort>();
        const ushort extra_len=reader.read<ushort>();
        const ushort comment_len=reader.read<ushort>();

        reader.skip(8);
        entry.offset=reader.read<uint>();

        entry.name=std::string((const char *)reader.get_data(),file_name_len);
        reader.skip(file_name_len+extra_len+comment_len);

        if(entry.unpacked_size==0)
        {
            if(entry.name.empty())
                continue;

            if(entry.name[entry.name.size()-1]=='/')
                continue;
        }

        m_entries.push_back(entry);
    }

    m_res=data;
    return true;
}

namespace
{

class zip_resource: public resource_data
{
public:
    size_t get_size() { return m_unpacked_size; }

    bool read_all(void*data)
    {
        if(m_compression==0)
            return m_res?m_res->read_all(data):false;

        if(m_data.get_size()>0)
            return m_data.copy_to(data,m_data.get_size());

        return unpack_to(data);
    }

    bool read_chunk(void *data,size_t size,size_t offset)
    {
        if(m_compression==0)
            return m_res?m_res->read_chunk(data,size,offset):false;

        m_data.allocate(m_unpacked_size);
        if(!unpack_to(m_data.get_data()))
        {
            m_data.free();
            return false;
        }

        return m_data.copy_to(data,size,offset);
    }

    void release() { m_data.free(); delete this; }

public:
    zip_resource(nya_resources::resource_data *res,unsigned int compression,unsigned int offset,unsigned int packed_size,unsigned int unpacked_size):
                 m_res(res),m_compression(compression),m_offset(offset),m_packed_size(packed_size),m_unpacked_size(unpacked_size) {}
private:
    bool unpack_to(void *data)
    {
        if(!data || !m_res)
            return false;

        if(m_compression!=8)
            return false;

        size_t offset=m_offset;

        unsigned int sign;
        if(!m_res->read_chunk(&sign,sizeof(sign),offset))
            return false;

        if(sign!=0x04034b50)
            return false;

        offset+=26;

        struct { unsigned short file_name_len,extra_field_len; } header;

        if(!m_res->read_chunk(&header,sizeof(header),offset))
            return false;

        offset+=sizeof(header);

        std::string name;
        name.resize(header.file_name_len);

        if(!m_res->read_chunk(&name[0],header.file_name_len,offset))
            return false;

        offset+=header.file_name_len+header.extra_field_len;

        nya_memory::tmp_buffer_scoped packed_buf(m_packed_size);
        m_res->read_chunk(packed_buf.get_data(),m_packed_size,offset);

        z_stream infstream;
        infstream.zalloc=Z_NULL;
        infstream.zfree=Z_NULL;
        infstream.opaque=Z_NULL;
        infstream.avail_in=(uInt)packed_buf.get_size();
        infstream.next_in=(Bytef *)packed_buf.get_data();
        infstream.avail_out=(uInt)m_unpacked_size;
        infstream.next_out=(Bytef *)data;

        inflateInit2(&infstream,-MAX_WBITS);
        if(inflate(&infstream,Z_NO_FLUSH)!=Z_OK)
            return false;

        inflateEnd(&infstream);
        return true;
    }

private:
    nya_resources::resource_data *m_res;
    nya_memory::tmp_buffer_ref m_data;
    unsigned int m_compression,m_offset,m_packed_size,m_unpacked_size;
};

}

resource_data *zip_resources_provider::access(const char *resource_name)
{
    std::string name=fix_name(resource_name);
    if(name.empty())
        return 0;

    for(int i=0;i<(int)m_entries.size();++i)
    {
        const zip_entry &e=m_entries[i];
        if(e.name==name)
            return new zip_resource(m_res,e.compression,e.offset,e.packed_size,e.unpacked_size);
    }

    return 0;
}

bool zip_resources_provider::has(const char *resource_name)
{
    std::string name=fix_name(resource_name);
    if(name.empty())
        return false;

    for(int i=0;i<(int)m_entries.size();++i)
    {
        if(m_entries[i].name==name)
            return true;
    }

    return false;
}

int zip_resources_provider::get_resources_count() { return (int)m_entries.size(); }

const char *zip_resources_provider::get_resource_name(int idx)
{
    if(idx<0 || idx>=(int)m_entries.size())
        return 0;

    return m_entries[idx].name.c_str();
}

}
