//
//  WebPluginController.m
//  WebKit
//
//  Created by Chris Blumenberg on Wed Oct 23 2002.
//  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
//

#import <WebKit/WebController.h>
#import <WebKit/WebDataSource.h>
#import <WebKit/WebFrame.h>
#import <WebKit/WebPlugin.h>
#import <WebKit/WebPluginController.h>
#import <WebKit/WebWindowOperationsDelegate.h>
#import <WebKit/WebView.h>

#import <WebFoundation/WebAssertions.h>
#import <WebFoundation/WebResourceRequest.h>

@implementation WebPluginController

- initWithWebFrame:(WebFrame *)theFrame
{
    [super init];
    
    // Not retained because the frame retains this plug-in controller.
    frame = theFrame;
    
    views = [[NSMutableArray array] retain];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(windowWillClose)
                                                 name:NSWindowWillCloseNotification
                                               object:nil];
    
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    ASSERT([views count] == 0);
    [views release];
    [super dealloc];
}

- (void)addPluginView:(NSView <WebPlugin> *)view
{
    [views addObject:view];
    [view pluginInitialize];
}

- (void)didAddPluginView:(NSView <WebPlugin> *)view
{
    [view pluginStart];
}

- (void)startAllPlugins
{
    [views makeObjectsPerformSelector:@selector(pluginStart)];
}

- (void)stopAllPlugins
{
    [views makeObjectsPerformSelector:@selector(pluginStop)];
}

- (void)destroyAllPlugins
{
    [self stopAllPlugins];
    [views makeObjectsPerformSelector:@selector(pluginDestroy)];
    [views removeAllObjects];
}

- (void)windowWillClose:(NSNotification *)notification
{
    if([notification object] == [[frame webView] window]){
        [self destroyAllPlugins];
    }
}

- (void)showURL:(NSURL *)URL inFrame:(NSString *)target
{
    if(!URL || !target){
        return;
    }

    WebFrame *otherFrame = [frame frameNamed:target];
    if(!otherFrame){
        // FIXME: Open new window instead of return.
        return;
    }

    WebDataSource *dataSource = [[WebDataSource alloc] initWithRequest:[WebResourceRequest requestWithURL:URL]];
    if(!dataSource){
        return;
    }

    if([otherFrame setProvisionalDataSource:dataSource]){
        [otherFrame startLoading];
    }
}

- (void)showStatus:(NSString *)message
{
    if(!message){
        return;
    }
    
    [[[frame controller] windowOperationsDelegate] setStatusText:message];
}

@end
