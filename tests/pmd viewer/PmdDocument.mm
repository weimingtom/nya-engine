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

@end
