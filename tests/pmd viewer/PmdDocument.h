//https://code.google.com/p/nya-engine/

#import <Cocoa/Cocoa.h>
#import "PmdView.h"
#include <string>

@interface PmdDocument : NSDocument//<NSValidatedUserInterfaceItem>
{
@public
    std::string m_model_name;
    std::string m_animation_name;

    mmd_mesh *m_mesh;
    PmdView *m_view;

@private
    MorphsWindow *m_morphs_window;
}

-(IBAction)loadAnimation:(id)sender;
-(BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)anItem;

@end
