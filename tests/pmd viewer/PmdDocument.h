//https://code.google.com/p/nya-engine/

#import <Cocoa/Cocoa.h>
#include <string>

@interface PmdDocument : NSDocument
{
@public
    std::string m_model_name;  
}

@end
