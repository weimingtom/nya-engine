//https://code.google.com/p/nya-engine/

#import <Cocoa/Cocoa.h>
#include <string>

@interface PmdDocument : NSDocument//<NSValidatedUserInterfaceItem>
{
@public
    std::string m_model_name;
    std::string m_animation_name;
}
-(IBAction)loadAnimation:(id)sender;
-(BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)anItem;

@end
