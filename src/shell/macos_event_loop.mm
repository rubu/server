#import <AppKit/AppKit.h>

#include <include/cef_application_mac.h>

@interface application : NSApplication<CefAppProtocol>
{
    BOOL handlingSendEvent_;
}

-(void)finishLaunching;
-(BOOL)isHandlingSendEvent;
-(void)setHandlingSendEvent:(BOOL)handlingSendEvent;
@end

@implementation application
{
}

-(void)finishLaunching
{
}

-(BOOL)isHandlingSendEvent
{
    return handlingSendEvent_;
}

-(void)setHandlingSendEvent:(BOOL)handlingSendEvent
{
    handlingSendEvent_ = handlingSendEvent;
}
@end

@interface application_delegate : NSObject<NSApplicationDelegate>
{
}

@end;

@implementation application_delegate
{
}

-(void)applicationDidFinishLaunching:(NSNotification *) notification
{
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return NO;
}
@end

void initialize_html_event_loop_delegate()
{
    application_delegate *app_delegate = [[application_delegate alloc] init];
    application* app = [application sharedApplication];
    app.delegate =  app_delegate;
}

void run_macos_event_loop(int argc, char** argv)
{
    NSApplicationMain(argc, const_cast<const char**>(argv));
    return;
}
