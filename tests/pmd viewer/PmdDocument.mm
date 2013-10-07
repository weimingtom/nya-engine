//https://code.google.com/p/nya-engine/

#import "PmdDocument.h"
#import "PmdView.h"

#include "resources/resources.h"

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

- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
    const char*filename=absoluteURL.path.UTF8String;
    if(!filename)
        return FALSE;

    if(!nya_resources::get_resources_provider().has(filename))
        return FALSE;

    m_model_name.assign(filename);

    return YES;
}

- (void)close
{
    [super close];
}

-(void) dealloc
{
    [super dealloc];
}

-(IBAction)loadAnimation:(id)sender
{
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];

     NSArray *fileTypes = [NSArray arrayWithObjects:@"vmd",nil];
    [openDlg setAllowedFileTypes:fileTypes];
    [openDlg setAllowsMultipleSelection:NO];
    [openDlg setCanChooseFiles:YES];
    [openDlg setCanChooseDirectories:NO];

    if ( [openDlg runModal] == NSOKButton )
    {
        NSArray* URLs = [openDlg URLs];
        if([URLs count]<1)
            return;

        NSURL* fileName = [URLs objectAtIndex:0];
        m_animation_name.assign(fileName.path.UTF8String);
    }
}

- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)anItem
{
    SEL theAction = [anItem action];

    if (theAction == @selector(loadAnimation:))
        return YES;
    else
        return [super validateUserInterfaceItem:anItem];
}

@end
