//https://code.google.com/p/nya-engine/

#import <Cocoa/Cocoa.h>
#include <vector>
#include "log/log.h"

@interface PmdDocument : NSDocument
{
@public
    bool m_changed;

    std::vector<float> m_verts;
    std::vector<int> m_indices;   

    struct material
    {
        float diffuse[4];
        float specular[4]; //last is shininess
        float ambient[3];
        
        uint ind_offset;
        uint ind_count;

        char tex_name[20];
        uint spmap_idx;
        uint toon_idx;
    };

    std::vector<material> m_materials;    
}

-(nya_log::log&) get_log;

@end
