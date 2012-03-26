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
}

-(nya_log::log&) get_log;

@end
