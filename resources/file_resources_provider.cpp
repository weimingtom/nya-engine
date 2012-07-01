//https://code.google.com/p/nya-engine/

#include "file_resources_provider.h"
#include <stdio.h>
#include "memory/pool.h"
#include <dirent.h>
#include <string>

#ifdef WIN32
    #include <sys/stat.h>
#endif

namespace nya_resources
{

class file_resource: public resource_data
{
public:
    bool is_valid() const { return m_file!=0; }
    size_t get_size() const { return m_size; }

    bool read_all(void*data) const;
    bool read_chunk(void *data,size_t size,size_t offset) const;

public:
    bool open(const char*filename);
    void release();

    file_resource(): m_file(0), m_size(0) {}
    //~file_resource() { release(); }

private:
    FILE *m_file;
    size_t m_size;
};

struct file_resource_info: public resource_info
{
public:
    std::string name;
    file_resource_info *next;

public:
    file_resource_info(): next(0) {}

private:
    resource_data *access();
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
    nya_memory::pool<nya_resources::file_resource,8> file_resources;
    nya_memory::pool<nya_resources::file_resource_info,32> entries;
}

namespace nya_resources
{

resource_data *file_resource_info::access()
{
    file_resource *file = file_resources.allocate();

    if(!file->open(name.c_str()))
    {
        get_log()<<"unable to acess file "<<name.c_str()<<"\n";
        file_resources.free(file);
        return 0;
    }

    return file;
}

resource_data *file_resources_provider::access(const char *resource_name)
{
    if(!resource_name)
    {
        get_log()<<"unable to access file: invalid name\n";
        return 0;
    }

    file_resource *file = file_resources.allocate();

    if(!file->open(resource_name))
    {
        get_log()<<"unable to access file: "<<resource_name<<"\n";
        file_resources.free(file);
        return 0;
    }

    return file;
}

bool file_resources_provider::set_folder(const char*name)
{
    clear_entries();

    return chdir(name);
}

void file_resources_provider::clear_entries()
{
    file_resource_info *entry=m_entries;
    while(entry)
    {
        file_resource_info *next=entry->next;
        entries.free(entry);
        entry=next;
    }

    m_entries=0;
}

void enumerate_folder(const char*folder_name,file_resource_info **last)
{
    if(!folder_name || !last)
        return;

    const std::string folder_name_str(folder_name);

    DIR *dirp=opendir(folder_name);
    dirent *dp;
    while((dp=readdir(dirp))!=0)
    {
#ifdef WIN32
        struct stat stat_buf;
        stat((folder_name_str+"/"+dp->d_name).c_str(),&stat_buf);
        if((stat_buf.st_mode&S_IFDIR)==S_IFDIR)
#else
        if(dp->d_type==DT_DIR)
#endif
        {
            std::string dir_name(dp->d_name);
            if(dir_name=="."||dir_name=="..")
                continue;

            enumerate_folder((folder_name_str+"/"+
                            dir_name).c_str(),last);
            continue;
        }

        file_resource_info *entry=entries.allocate();
        entry->name=folder_name_str;
        entry->name.push_back('/');
        entry->name.append(dp->d_name);
        if(entry->name.compare("./")>0)
            entry->name=entry->name.substr(2);
        entry->next=*last;
        *last=entry;
    }
    closedir(dirp);
}

resource_info *file_resources_provider::first_res_info()
{
    if(m_entries)
        return m_entries;

    file_resource_info *last=0;
    enumerate_folder(".",&last);
    m_entries=last;

    return m_entries;
}

bool file_resource::read_all(void*data) const
{
    if(!data||!m_file)
    {
        get_log()<<"unable to read file data\n";
        return false;
    }

    if(fseek(m_file,0,SEEK_SET)!=0)
    {
        get_log()<<"unable to read file data: seek_set failed\n";
        return false;
    }

    if(fread(data,1,m_size,m_file)!=m_size)
    {
        get_log()<<"unable to read file data: unexpected size of readen data\n";
        return false;
    }

    return true;
}

bool file_resource::read_chunk(void *data,size_t size,size_t offset) const
{
    if(!data||!m_file)
    {
        get_log()<<"unable to read file data chunk\n";
        return false;
    }

    if(offset+size>m_size||!size)
    {
        get_log()<<"unable to read file data chunk: invalid size\n";
        return false;
    }

    if(fseek(m_file,offset,SEEK_SET)!=0)
    {
        get_log()<<"unable to read file data chunk: seek_set failed\n";
        return false;
    }

    if(fread(data,1,size,m_file)!=size)
    {
        get_log()<<"unable to read file data chunk: unexpected size of readen data\n";
        return false;
    }

    return true;
}

bool file_resource::open(const char*filename)
{
    if(m_file)
        fclose(m_file);

    m_size=0;
    m_file=fopen(filename,"rb");
    if(!m_file)
        return false;

    if(fseek(m_file,0,SEEK_END)!=0)
        return false;

    m_size=ftell(m_file);

    return true;
}

void file_resource::release()
{
    if(m_file)
        fclose(m_file);

    file_resources.free(this);
}

}
