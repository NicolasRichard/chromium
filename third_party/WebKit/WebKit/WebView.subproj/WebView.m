/*	
    WebController.mm
    Copyright 2001, 2002 Apple, Inc. All rights reserved.
*/

#import <WebKit/WebController.h>

#import <WebKit/WebBackForwardList.h>
#import <WebKit/WebController.h>
#import <WebKit/WebControllerPolicyHandler.h>
#import <WebKit/WebControllerPrivate.h>
#import <WebKit/WebDataSourcePrivate.h>
#import <WebKit/WebDefaultPolicyHandler.h>
#import <WebKit/WebDocument.h>
#import <WebKit/WebDynamicScrollBarsView.h>
#import <WebKit/WebException.h>
#import <WebKit/WebFrame.h>
#import <WebKit/WebFramePrivate.h>
#import <WebKit/WebHistoryItem.h>
#import <WebKit/WebKitErrors.h>
#import <WebKit/WebKitStatisticsPrivate.h>
#import <WebKit/WebKitDebug.h>
#import <WebKit/WebPluginDatabase.h>
#import <WebKit/WebViewPrivate.h>

#import <WebFoundation/WebFoundation.h>

@implementation WebController

- init
{
    return [self initWithView: nil provisionalDataSource: nil];
}

- initWithView: (WebView *)view provisionalDataSource: (WebDataSource *)dataSource
{
    [super init];
    
    _private = [[WebControllerPrivate alloc] init];
    _private->mainFrame = [[WebFrame alloc] initWithName: @"_top" webView: view provisionalDataSource: dataSource controller: self];

    [self setUseBackForwardList: YES];
    
    ++WebControllerCount;

    return self;
}

- (void)dealloc
{
    --WebControllerCount;
    
    [_private release];
    [super dealloc];
}


- (WebFrame *)createFrameNamed: (NSString *)fname for: (WebDataSource *)childDataSource inParent: (WebDataSource *)parentDataSource allowsScrolling: (BOOL)allowsScrolling
{
    WebView *childView;
    WebFrame *newFrame;

    childView = [[WebView alloc] initWithFrame: NSMakeRect(0,0,0,0)];

    newFrame = [[WebFrame alloc] initWithName: fname webView: childView provisionalDataSource: childDataSource controller: self];

    [parentDataSource addFrame: newFrame];
    
    [newFrame release];

    [childView _setController: self];
    [childDataSource _setController: self];

    [childView setAllowsScrolling: allowsScrolling];
    
    [childView release];
        
    return newFrame;
}


- (void)setWindowContext:(id <WebWindowContext>)context
{
    [context retain];
    [_private->windowContext release];
    _private->windowContext = context;
}

- (id <WebWindowContext>)windowContext
{
    return _private->windowContext;
}

- (void)setResourceProgressHandler: (id <WebResourceProgressHandler>)handler
{
    [handler retain];
    [_private->resourceProgressHandler release];
    _private->resourceProgressHandler = handler;
}


- (id<WebResourceProgressHandler>)resourceProgressHandler
{
    return _private->resourceProgressHandler;
}


- (void)setDownloadProgressHandler: (id<WebResourceProgressHandler>)handler
{
    [handler retain];
    [_private->downloadProgressHandler release];
    _private->downloadProgressHandler = handler;
}


- (id<WebResourceProgressHandler>)downloadProgressHandler
{
    return _private->downloadProgressHandler;
}

- (void)setContextMenuHandler: (id<WebContextMenuHandler>)handler
{
    [handler retain];
    [_private->contextMenuHandler release];
    _private->contextMenuHandler = handler;
}

- (id<WebContextMenuHandler>)contextMenuHandler
{
    return _private->contextMenuHandler;
}

- (void)setPolicyHandler:(id <WebControllerPolicyHandler>)handler
{
    [handler retain];
    [_private->policyHandler release];
    _private->policyHandler = handler;
}

- (id<WebControllerPolicyHandler>)policyHandler
{
    if (!_private->policyHandler)
        _private->policyHandler = [[WebDefaultPolicyHandler alloc] initWithWebController: self];
    return _private->policyHandler;
}

- (void)setLocationChangeHandler:(id <WebLocationChangeHandler>)handler
{
    [_private->locationChangeHandler autorelease];
    _private->locationChangeHandler = [handler retain];
}

- (id <WebLocationChangeHandler>)locationChangeHandler
{
    return _private->locationChangeHandler;
}

- (WebFrame *)_frameForDataSource: (WebDataSource *)dataSource fromFrame: (WebFrame *)frame
{
    NSArray *frames;
    int i, count;
    WebFrame *result, *aFrame;
    
    if ([frame dataSource] == dataSource)
        return frame;
        
    if ([frame provisionalDataSource] == dataSource)
        return frame;
        
    frames = [[frame dataSource] children];
    count = [frames count];
    for (i = 0; i < count; i++){
        aFrame = [frames objectAtIndex: i];
        result = [self _frameForDataSource: dataSource fromFrame: aFrame];
        if (result)
            return result;
    }

    frames = [[frame provisionalDataSource] children];
    count = [frames count];
    for (i = 0; i < count; i++){
        aFrame = [frames objectAtIndex: i];
        result = [self _frameForDataSource: dataSource fromFrame: aFrame];
        if (result)
            return result;
    }
    
    return nil;       
}


- (WebFrame *)frameForDataSource: (WebDataSource *)dataSource
{
    WebFrame *frame = [self mainFrame];
    
    return [self _frameForDataSource: dataSource fromFrame: frame];
}


- (WebFrame *)_frameForView: (WebView *)aView fromFrame: (WebFrame *)frame
{
    NSArray *frames;
    int i, count;
    WebFrame *result, *aFrame;
    
    if ([frame webView] == aView)
        return frame;
        
    frames = [[frame dataSource] children];
    count = [frames count];
    for (i = 0; i < count; i++){
        aFrame = [frames objectAtIndex: i];
        result = [self _frameForView: aView fromFrame: aFrame];
        if (result)
            return result;
    }

    frames = [[frame provisionalDataSource] children];
    count = [frames count];
    for (i = 0; i < count; i++){
        aFrame = [frames objectAtIndex: i];
        result = [self _frameForView: aView fromFrame: aFrame];
        if (result)
            return result;
    }
    
    return nil;       
}


- (WebFrame *)frameForView: (WebView *)aView
{
    WebFrame *frame = [self mainFrame];
    
    return [self _frameForView: aView fromFrame: frame];
}


- (WebFrame *)frameNamed: (NSString *)name
{
    return [[self mainFrame] frameNamed: name];
}

- (WebFrame *)mainFrame
{
    return _private->mainFrame;
}


+ (WebURLPolicy *)defaultURLPolicyForURL: (NSURL *)URL
{
    if([WebResourceHandle canInitWithURL:URL]){
        return [WebURLPolicy webPolicyWithURLAction:WebURLPolicyUseContentPolicy];
    }else{
        return [WebURLPolicy webPolicyWithURLAction:WebURLPolicyOpenExternally];
    }
}

- (void)stopAnimatedImages
{
}

- (void)startAnimatedImages
{
}

- (void)stopAnimatedImageLooping
{
}

- (void)startAnimatedImageLooping
{
}

+ (BOOL)canShowMIMEType:(NSString *)MIMEType
{
    if([WebView _canShowMIMEType:MIMEType] && [WebDataSource _canShowMIMEType:MIMEType]){
        return YES;
    }else{
        // Have the plug-ins register views and representations
        [WebPluginDatabase installedPlugins];
        if([WebView _canShowMIMEType:MIMEType] && [WebDataSource _canShowMIMEType:MIMEType])
            return YES;
    }
    return NO;
}

+ (BOOL)canShowFile:(NSString *)path
{    
    NSString *MIMEType;
    
    MIMEType = [[self class] _MIMETypeForFile:path];   
    return [[self class] canShowMIMEType:MIMEType];
}

- (WebBackForwardList *)backForwardList
{
    return _private->backForwardList;
}

- (void)setUseBackForwardList: (BOOL)flag
{
    _private->useBackForwardList = flag;
}

- (BOOL)useBackForwardList
{
    return _private->useBackForwardList;
}

- (void)_goToItem: (WebHistoryItem *)item withFrameLoadType: (WebFrameLoadType)type
{
    WebFrame *targetFrame;
    
    targetFrame = [self frameNamed: [item target]];
    if (targetFrame == nil){
        NSLog (@"Target frame not found, using main frame instead, will be fixed soon\n");
#if 0
        int pos = 1;
        WebHistoryItem *next = item;
        while (next){
            NSLog (@"frame name %@, parent %@\n", [next target], [next parent]);
            nextFrame = [self frameNamed: [next parent]];
            next = [[self backForwardList] backEntryAtIndex: pos++];
            if ([[next target] isEqual: @"_top"]){
                [[self mainFrame] _goToItem: next withFrameLoadType: WebFrameLoadTypeIntermediateBack];
                return;
            }
        }
#endif            
        targetFrame = [self mainFrame];
    }
    [targetFrame _goToItem: item withFrameLoadType: type];
}

- (BOOL)goBack
{
    WebHistoryItem *item = [[self backForwardList] backEntry];
    
    if (item){
        [self _goToItem: item withFrameLoadType: WebFrameLoadTypeBack];
        return YES;
    }
    return NO;
}

- (BOOL)goForward
{
    WebHistoryItem *item = [[self backForwardList] forwardEntry];
    
    if (item){
        [self _goToItem: item withFrameLoadType: WebFrameLoadTypeForward];
        return YES;
    }
    return NO;
}

- (void)setTextSizeMultiplier:(float)m
{
    if (_private->textSizeMultiplier == m) {
        return;
    }
    _private->textSizeMultiplier = m;
    [[self mainFrame] _textSizeMultiplierChanged];
}

- (float)textSizeMultiplier
{
    return _private->textSizeMultiplier;
}

- (void)setApplicationNameForUserAgent:(NSString *)applicationName
{
    NSString *name = [applicationName copy];
    [_private->userAgentLock lock];
    [_private->applicationNameForUserAgent release];
    _private->applicationNameForUserAgent = name;
    [_private->userAgentLock unlock];
}

// Set the user agent explicitly. Setting the user-agent string to nil means
// that WebKit should construct the best possible user-agent string for each URL
// for best results rendering web pages. Setting it to any string means
// that WebKit should use that user-agent string for all purposes until it is set
// back to nil.
- (void)setUserAgent:(NSString *)userAgentString
{
    NSString *override = [userAgentString copy];
    [_private->userAgentLock lock];
    [_private->userAgentOverride release];
    _private->userAgentOverride = override;
    [_private->userAgentLock unlock];
}

// Get the appropriate user-agent string for a particular URL.
- (NSString *)userAgentForURL:(NSURL *)URL
{
    [_private->userAgentLock lock];
    NSString *result = [[_private->userAgentOverride copy] autorelease];
    [_private->userAgentLock unlock];
    if (result) {
        return result;
    }

    // Note that we currently don't look at the URL.
    // If we find that we need different user agent strings for different web pages
    // for best results, then that logic will go here.

    // FIXME: Incorporate applicationNameForUserAgent in this string so that people
    // can tell that they are talking to Alexander. Maybe also incorporate something
    // that identifies WebKit's involvement.
    return @"Mozilla/5.0 (Macintosh; U; PPC Mac OS X; en-US; rv:1.0.0) Gecko/20020715";
}

@end
