//https://code.google.com/p/nya-engine/

#import "PmdDocument.h"
#import "PmdView.h"

#include "resources/resources.h"
#include "log/stdout_log.h"
#include "memory/tmp_buffer.h"
#include "memory/tmp_buffer.h"

@implementation PmdDocument

- (id)init
{
    self = [super init];
    if (self) 
    {
    }
    return self;
}

- (NSString *)windowNibName
{
    return @"PmdDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *)aController
{
    [super windowControllerDidLoadNib:aController];
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
    if (outError)
        *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:unimpErr userInfo:NULL];

    return nil;
}

- (nya_log::log&) get_log
{
    static nya_log::stdout_log log;
    return log;
}

- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
    nya_log::log &log = [ self get_log ];
    const char*filename=absoluteURL.path.UTF8String;
    log<<"readFromURL: "<<filename<<"\n";
    
    nya_resources::resource_data *pmd_data = nya_resources::get_resources_provider().access(filename);
    if(!pmd_data)
    {
        log<<"Error: unable to open "<<filename<<"\n";
        return NO;
    }

    char path[512];
    sprintf(path,"%s",filename);
    const int path_length = (int)strlen(path);
    for(int i=path_length-1;i>0;--i)
    {
        if(path[i]=='\\'||path[i]=='/')
        {
            path[i+1]=0;
            break;
        }
    }

    if(chdir(path)!=0)
        log<<"unable to set path: "<<path<<"\n";

    typedef unsigned int uint;
    typedef unsigned short ushort;
    typedef unsigned char uchar;

    nya_memory::tmp_buffer_scoped pmd_buffer(pmd_data->get_size());

    log<<"opened file "<<(int)pmd_data->get_size()<<"bytes\n";

    pmd_data->read_all(pmd_buffer.get_data());
    pmd_data->release();

#pragma pack(push,1)
    struct pmd_header
    {
        char signature[3];
        float version;
        char name[20];
        char comment[256];
        
        bool is_valid() const
        {
            if(signature[0]!='P'||signature[1]!='m'||signature[2]!='d')
                return false;
            if(version!=1.0f)
                return false;
            
            return true;
        }
    };
#pragma pack(pop)

    const pmd_header *header = (pmd_header *)pmd_buffer.get_data();
    if(!header->is_valid())
    {
        log<<"invalid pmd header: "<<
        header->signature<<" "<<
        header->version<<"\n";
        return NO;
    }

#pragma pack(push,1)
    struct pmd_vertex
    {
        float pos_nor_tc[8];
        ushort bone_idx[2];
        uchar weight;
        uchar edge;
        
        struct bone_info {float bone_idx[2];float weight;};
        bone_info get_bone_info()
        {
            static bone_info info;
            info.bone_idx[0]=(float)bone_idx[0];
            info.bone_idx[1]=(float)bone_idx[1];   
            info.weight = float(weight)/255.0f;
            return info;
        };
    };
#pragma pack(pop)

    uint offset = sizeof(pmd_header);

    const uint vcount=*(uint*)pmd_buffer.get_data(offset);
    offset+=4;
    const pmd_vertex *vertices=(pmd_vertex *)pmd_buffer.get_data(offset);

    if(vcount<3)
    {
        log<<"Error: invalid vertex count in model\n";
        return NO;
    }

    m_verts.resize(vcount*8);
    for(int i=0;i<vcount;++i)
    {
        for(int k=0;k<8;++k)
            m_verts[i*8+k]=vertices[i].pos_nor_tc[k];

        //m_verts[i*8+7]=1.0f-m_verts[i*8+7];
    }

    offset+=vcount*sizeof(pmd_vertex);

    const uint icount=*(uint*)pmd_buffer.get_data(offset);
    offset+=4;
    m_indices.resize(icount);
    memcpy(&m_indices[0],pmd_buffer.get_data(offset),icount*sizeof(ushort));
    offset+=icount*sizeof(ushort);

#pragma pack(push,1)
    struct pmd_material
    {
        float diffuse[4];
        float shininess;        
        float specular[3];
        float ambient[3];
        uchar toon_idx;
        uchar edge_flag;
        uint ind_count;
        
        char tex_name[20];
    };
#pragma pack(pop)

    const uint mat_count=*(uint*)pmd_buffer.get_data(offset);
    offset+=4;
    
    m_materials.resize(mat_count);
    uint ind_offset=0;
    const pmd_material *pmd_materials=(pmd_material*)pmd_buffer.get_data(offset);
    for(int i=0;i<mat_count;++i)
    {
        const pmd_material &from=pmd_materials[i];
        material &to=m_materials[i];
        
        for(int k=0;k<3;++k)
        {
            to.diffuse[k]=from.diffuse[k];
            to.specular[k]=from.specular[k];     
            to.ambient[k]=from.ambient[k];                    
        }
        to.diffuse[3]=from.diffuse[3]; 
        to.specular[3]=from.shininess;
        to.ind_offset=ind_offset;
        to.ind_count=from.ind_count;
        ind_offset+=to.ind_count;
        
        sprintf(to.tex_name,"%s",from.tex_name);
    }
    offset+=mat_count*sizeof(pmd_material);        

    m_changed=true;

    return YES;
}

- (void)close
{
    [super close];
}

-(void) dealloc
{
    m_verts.clear();
    m_indices.clear();
    m_materials.clear();
    
    [super dealloc];
}

@end
