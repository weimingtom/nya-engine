//https://code.google.com/p/nya-engine/

#include "file_resources_provider.h"
#include "memory/pool.h"

#include <stdio.h>

#ifdef WIN32
	#include <io.h>
#else
	#include <dirent.h>
#endif

#include <sys/stat.h>

#ifndef S_ISDIR
	#define	S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#endif

namespace nya_resources
{

class file_ref
{
public:
    void init(const char *name)
    {
        if(name)
            m_name.assign(name);
        else
            m_name.erase();
    }

    FILE *access() { return get_lru().access(*this); }

    void free() { get_lru().free(*this); }

    class lru
    {
    public:
        FILE *access(file_ref &ref)
        {
            if(ref.m_id>=0)
            {
                entry *relink=&m_entries[ref.m_id];
                if(relink==m_last)
                {
                    m_first->prev=relink;
                    relink->next=m_first;
                    m_first=relink;
                    m_last=relink->prev;
                    m_last->next=0;
                    relink->prev=0;
                }
                else if(relink!=m_first)
                {
                    relink->next->prev=relink->prev;
                    relink->prev->next=relink->next;
                    relink->next=m_first;
                    m_first->prev=relink;
                    m_first=relink;
                    relink->prev=0;
                }

                return relink->descriptor;
            }

              //ToDo: file descriptor share ?
            FILE *f=fopen(ref.m_name.c_str(),"rb");
            if(!f)
                return 0;

            entry *relink=m_last;

            if(relink->descriptor)
            {
                fclose(relink->descriptor);
                relink->descriptor=0;
            }

            if(relink->ref)
                relink->ref->m_id=-1;

            relink->descriptor=f;
            relink->ref=&ref;
            ref.m_id=relink->id;

            m_first->prev=relink;
            relink->next=m_first;
            m_first=relink;
            m_last=relink->prev;
            m_last->next=0;
            relink->prev=0;

            return f;
        }

        void free(file_ref &ref)
        {
            if(ref.m_id<0)
                return;

            entry *relink=&m_entries[ref.m_id];
            ref.m_id=-1;

            if(relink->descriptor)
            {
                fclose(relink->descriptor);
                relink->descriptor=0;
            }

            if(relink==m_last)
                return;

            if(relink==m_first)
            {
                m_first=relink->next;
                m_first->prev=0;
                m_last->next=relink;
                relink->prev=m_last;
                m_last=relink;
                relink->next=0;
            }
            else
            {
                relink->next->prev=relink->prev;
                relink->prev->next=relink->next;
                relink->prev=m_last;
                m_last->next=relink;
                m_last=relink;
                relink->next=0;
            }
        }

        lru()
        {
            for(int i=0;i<max_opened_descriptors-1;++i)
            {
                m_entries[i].next=&m_entries[i+1];
                m_entries[i+1].prev=&m_entries[i];

                m_entries[i].id=i;
            }

            m_entries[max_opened_descriptors-1].id=max_opened_descriptors-1;

            m_first=&m_entries[0];
            m_last=&m_entries[max_opened_descriptors-1];
        }

    private:
        struct entry
        {
            int id;

            FILE *descriptor;
            file_ref *ref;

            entry *prev;
            entry *next;

            entry(): id(-1),descriptor(0),ref(0),
                     prev(0),next(0) {}
        };

    private:
        entry *m_first;
        entry *m_last;

        const static int max_opened_descriptors=64;
        entry m_entries[max_opened_descriptors];
    };

    static lru &get_lru()
    {
        static lru cache;
        return cache;
    }

    file_ref(): m_id(-1) {}

private:
    int m_id;
    std::string m_name;
};

class file_resource: public resource_data
{
public:
    size_t get_size() { return m_size; }

    bool read_all(void*data);
    bool read_chunk(void *data,size_t size,size_t offset);

public:
    bool open(const char*filename);
    void release();

    file_resource(): m_size(0) {}
    //~file_resource() { release(); }

private:
    file_ref m_file;
    size_t m_size;
};

class file_resource_info: public resource_info
{
public:
    std::string name;
    std::string path;
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
    if(!file->open((path+name).c_str()))
    {
        get_log()<<"unable to access file "<<name.c_str()
                    <<" at path "<<path.c_str()<<"\n";
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

    std::string file_name=m_path+resource_name;
    for(size_t i=m_path.size();i<file_name.size();++i)
    {
        if(file_name[i]=='\\')
            file_name[i]='/';
    }

    if(!file->open(file_name.c_str()))
    {
        get_log()<<"unable to access file: "<<file_name.c_str()+m_path.size()
                        <<" at path "<<m_path.c_str()<<"\n";
        file_resources.free(file);
        return 0;
    }

    return file;
}

bool file_resources_provider::has(const char *name)
{
    if(!name)
        return false;

    std::string file_name=m_path+name;
    for(size_t i=m_path.size();i<file_name.size();++i)
    {
        if(file_name[i]=='\\')
            file_name[i]='/';
    }

    struct stat sb;
    return stat(file_name.c_str(),&sb)==0;
}

bool file_resources_provider::set_folder(const char*name,bool recursive)
{
    clear_entries();

    m_recursive=recursive;

    if(!name)
    {
        m_path.erase();
        return false;
    }

    m_path.assign(name);
    for(size_t i=0;i<m_path.size();++i)
    {
        if(m_path[i]=='\\')
            m_path[i]='/';
    }

    while(!m_path.empty() && m_path[m_path.length()-1]=='/')
        m_path.resize(m_path.length()-1);

    if(m_path.empty())
        return true;

    struct stat sb;
    if(stat(m_path.c_str(),&sb)==-1)
    {
        get_log()<<"unable to set folder: invalid path "<<name<<"\n";
        m_path.erase();
        return false;
    }

    if(!S_ISDIR(sb.st_mode))
    {
        get_log()<<"unable to set folder: specified path is not a directory "<<name<<"\n";
        m_path.erase();
        return false;
    }

    m_path.push_back('/');

    return true;
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

void file_resources_provider::enumerate_folder(const char*folder_name,file_resource_info **last)
{
    if(!folder_name || !last)
        return;

    std::string folder_name_str(folder_name);

    while(!folder_name_str.empty() && folder_name_str[folder_name_str.length()-1]=='/')
        folder_name_str.resize(folder_name_str.length()-1);

    std::string first_dir=(m_path+folder_name_str);
    if(first_dir.empty())
        return;

    if(first_dir[0]=='.')
        return;

    while(!first_dir.empty() && first_dir[first_dir.length()-1]=='/')
        first_dir.resize(first_dir.length()-1);

#ifdef WIN32
	_finddata_t data;
	intptr_t hdl=_findfirst((first_dir + "/*").c_str(),&data);
    if(!hdl)
#else
    DIR *dirp=opendir(first_dir.c_str());
    if(!dirp)
#endif
    {
        nya_log::get_log()<<"unable to enumerate folder "<<(m_path+folder_name_str).c_str()<<"\n";
        return;
    }

#ifdef WIN32
	for(intptr_t it=hdl;it>=0;it=_findnext(hdl,&data))
	{
		const char *name=data.name;
#else
    while(dirent *dp=readdir(dirp))
    {
		const char *name=dp->d_name;
#endif
        if((name[0]=='.'&&name[1]=='\0')||(name[0]=='.'&&name[1]=='.'&&name[2]=='\0'))
            continue;

#ifdef WIN32
        std::string name_str=folder_name_str+"/"+name;
        std::string full_path_str=m_path+name_str;

        struct stat stat_buf;
        if(stat(full_path_str.c_str(),&stat_buf)<0)
        {
            nya_log::get_log()<<"unable to read "<<full_path_str.c_str()<<"\n";
            continue;
        }

        if((stat_buf.st_mode&S_IFDIR)==S_IFDIR && m_recursive)
#else
        if(dp->d_type==DT_DIR && m_recursive)
#endif
        {
            enumerate_folder((folder_name_str+"/"+name).c_str(),last);
            continue;
        }

        file_resource_info *entry=entries.allocate();

        if(!folder_name_str.empty() && folder_name_str != ".")
        {
            entry->name=folder_name_str;
            entry->name.push_back('/');
        }

        entry->name.append(name);

        entry->path=m_path;
        entry->next=*last;
        *last=entry;
    }
#ifdef WIN32
    _findclose(hdl);
#else
    closedir(dirp);
#endif
}

resource_info *file_resources_provider::first_res_info()
{
    if(m_entries)
        return m_entries;

    file_resource_info *last=0;
    enumerate_folder(m_path.empty()?".":"",&last);

    m_entries=last;

    return m_entries;
}

bool file_resource::read_all(void*data)
{
    if(!data)
    {
        get_log()<<"unable to read file data: invalid data pointer\n";
        return false;
    }

    FILE *file=m_file.access();
    if(!file)
    {
        get_log()<<"unable to read file data: no such file\n";
        return false;
    }

    if(fseek(file,0,SEEK_SET)!=0)
    {
        get_log()<<"unable to read file data: seek_set failed\n";
        return false;
    }

    if(fread(data,1,m_size,file)!=m_size)
    {
        get_log()<<"unable to read file data: unexpected size of readen data\n";
        return false;
    }

    return true;
}

bool file_resource::read_chunk(void *data,size_t size,size_t offset)
{
    if(!data)
    {
        get_log()<<"unable to read file data chunk: invalid data pointer\n";
        return false;
    }

    FILE *file=m_file.access();
    if(!file)
    {
        get_log()<<"unable to read file data: no such file\n";
        return false;
    }

    if(offset+size>m_size||!size)
    {
        get_log()<<"unable to read file data chunk: invalid size\n";
        return false;
    }

    if(fseek(file,(long)offset,SEEK_SET)!=0)
    {
        get_log()<<"unable to read file data chunk: seek_set failed\n";
        return false;
    }

    if(fread(data,1,size,file)!=size)
    {
        get_log()<<"unable to read file data chunk: unexpected size of readen data\n";
        return false;
    }

    return true;
}

bool file_resource::open(const char*filename)
{
    m_file.free();

    m_size=0;

    if(!filename)
        return false;

    m_file.init(filename);
    FILE *file=m_file.access();
    if(!file)
        return false;

    if(fseek(file,0,SEEK_END)!=0)
        return false;

    m_size=ftell(file);

    return true;
}

void file_resource::release()
{
    m_file.free();

    file_resources.free(this);
}

}
