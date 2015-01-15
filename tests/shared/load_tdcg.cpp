//https://code.google.com/p/nya-engine/

#include "load_tdcg.h"
#include "scene/mesh.h"
#include "memory/memory_reader.h"
#include <zlib.h>

inline unsigned int swap_byte_order(unsigned int ui) { return (ui >> 24) | ((ui<<8) & 0x00FF0000) | ((ui>>8) & 0x0000FF00) | (ui << 24); }

bool tdcg_loader::load_hardsave(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name)
{
    nya_memory::memory_reader reader(data.get_data(),data.get_size());

    const char *png_header="\x89PNG\x0d\x0a\x1a\x0a";
    if(!reader.test(png_header, sizeof(png_header)))
        return false;

    typedef unsigned int uint;
    while(reader.get_remained())
    {
        const uint size=swap_byte_order(reader.read<uint>());
        const uint type=reader.read<uint>();
        if(memcmp(&type,"taOb",4)==0)
        {
            const size_t offset=reader.get_offset();

            const uint type=reader.read<uint>();
            reader.skip(8);
            const uint dst_len=reader.read<uint>();
            const uint src_len=reader.read<uint>();

            if(size<src_len || reader.get_remained()<src_len)
                return false;

            if(memcmp(&type,"FTSO",4)==0)
            {
                nya_memory::tmp_buffer_scoped buf(dst_len);
                uLong extracted_len=dst_len;
                if(uncompress((Byte *)buf.get_data(),&extracted_len,(Byte *)reader.get_data(),src_len)!=Z_OK)
                    return false;

                nya_memory::memory_reader reader(buf.get_data(),buf.get_size());
                if(!reader.test("TSO1", 4))
                    return false;

                //ToDo
            }

            reader.seek(offset);
        }
        else if(memcmp(&type,"IEND",4)==0)
            break;

        if(!reader.skip(size))
            return false;

        reader.skip(4); //CRC
    }

    //ToDo

    return true;
}
