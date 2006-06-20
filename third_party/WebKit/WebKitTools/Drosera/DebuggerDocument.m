/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "DebuggerDocument.h"

@implementation WebScriptCallFrame (WebScriptCallFrameScripting)
+ (BOOL)isSelectorExcludedFromWebScript:(SEL)aSelector
{
    return NO;
}

+ (BOOL)isKeyExcludedFromWebScript:(const char *)name
{
    return NO;
}
@end

@implementation DebuggerDocument
+ (BOOL)isSelectorExcludedFromWebScript:(SEL)aSelector
{
    return NO;
}

+ (BOOL)isKeyExcludedFromWebScript:(const char *)name
{
    return NO;
}

- (id)initWithServerName:(NSString *)serverName
{
    if ((self = [super init]))
        [self switchToServerNamed:serverName];
    return self;
}

- (void)windowWillClose:(NSNotification *)notification
{
    [[webView windowScriptObject] removeWebScriptKey:@"DebuggerDocument"];

    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSApplicationWillTerminateNotification object:nil];
    [[NSDistributedNotificationCenter defaultCenter] removeObserver:self name:WebScriptDebugServerQueryReplyNotification object:nil];
    [[NSDistributedNotificationCenter defaultCenter] removeObserver:self name:WebScriptDebugServerWillUnloadNotification object:nil];

    [self switchToServerNamed:nil];

    [self autorelease]; // DebuggerApplication expects us to release on close
}

- (void)dealloc
{
    [server release];
    [currentServerName release];
    [super dealloc];
}

- (BOOL)isPaused
{
    return paused;
}

- (void)pause
{
    paused = YES;
    if ([[(NSDistantObject *)server connectionForProxy] isValid])
        [server pause];
}

- (void)resume
{
    paused = NO;
    if ([[(NSDistantObject *)server connectionForProxy] isValid])
        [server resume];
}

- (void)step
{
    if ([[(NSDistantObject *)server connectionForProxy] isValid])
        [server step];
}

- (NSString *)windowNibName
{
    return @"Debugger";
}

- (void)windowDidLoad
{
    [super windowDidLoad];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationTerminating:) name:NSApplicationWillTerminateNotification object:nil];

    NSString *path = [[NSBundle bundleForClass:[self class]] pathForResource:@"debugger" ofType:@"html" inDirectory:nil];
    [[webView mainFrame] loadRequest:[[[NSURLRequest alloc] initWithURL:[NSURL fileURLWithPath:path]] autorelease]];
}

- (void)switchToServerNamed:(NSString *)name
{
    if (server) {
        [[NSNotificationCenter defaultCenter] removeObserver:self name:NSConnectionDidDieNotification object:[(NSDistantObject *)server connectionForProxy]];
        if ([[(NSDistantObject *)server connectionForProxy] isValid]) {
            [self resume];
            [server removeListener:self];
        }
    }

    id old = server;
    server = ([name length] ? [[NSConnection rootProxyForConnectionWithRegisteredName:name host:nil] retain] : nil);
    [old release];

    old = currentServerName;
    currentServerName = [name copy];
    [old release];

    if (server) {
        @try {
            [(NSDistantObject *)server setProtocolForProxy:@protocol(WebScriptDebugServer)];
            [server addListener:self];
        } @catch (NSException *exception) {
            [currentServerName release];
            currentServerName = nil;
            [server release];
            server = nil;
        }

        if (server)
            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(serverConnectionDidDie:) name:NSConnectionDidDieNotification object:[(NSDistantObject *)server connectionForProxy]];  
    }
}

- (void)applicationTerminating:(NSNotification *)notifiction
{
    if (server && [[(NSDistantObject *)server connectionForProxy] isValid]) {
        if ([[(NSDistantObject *)server connectionForProxy] isValid])
            [server removeListener:self];
        // call the runloop for a while to make sure our removeListener: is sent to the server
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
    }
}

- (void)serverConnectionDidDie:(NSNotification *)notifiction
{
    [self switchToServerNamed:nil];
}

- (void)webView:(WebView *)sender windowScriptObjectAvailable:(WebScriptObject *)windowScriptObject
{
    // note: this is the Debuggers's own WebView, not the one being debugged
    [[sender windowScriptObject] setValue:self forKey:@"DebuggerDocument"];
}

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
    // note: this is the Debuggers's own WebView, not the one being debugged
    webViewLoaded = YES;
}

- (void)webView:(WebView *)view didParseSource:(NSString *)source fromURL:(NSString *)url sourceId:(int)sid forWebFrame:(WebFrame *)webFrame
{
    if (!webViewLoaded)
        return;

    NSString *sourceCopy = [source copy];
    NSString *urlCopy = [url copy];

    NSArray *args = [NSArray arrayWithObjects:sourceCopy, (urlCopy ? urlCopy : @""), [NSNumber numberWithInt:sid], nil];
    [[webView windowScriptObject] callWebScriptMethod:@"didParseScript" withArguments:args];

    [sourceCopy release];
    [urlCopy release];
}

- (void)webView:(WebView *)view didEnterCallFrame:(WebScriptCallFrame *)frame sourceId:(int)sid line:(int)lineno forWebFrame:(WebFrame *)webFrame
{
    if (!webViewLoaded)
        return;

    NSArray *args = [NSArray arrayWithObjects:[NSNumber numberWithInt:sid], [NSNumber numberWithInt:lineno], nil];
    [[webView windowScriptObject] callWebScriptMethod:@"didEnterCallFrame" withArguments:args];
}

- (void)webView:(WebView *)view willExecuteStatement:(WebScriptCallFrame *)frame sourceId:(int)sid line:(int)lineno forWebFrame:(WebFrame *)webFrame
{
    if (!webViewLoaded)
        return;

    NSArray *args = [NSArray arrayWithObjects:[NSNumber numberWithInt:sid], [NSNumber numberWithInt:lineno], nil];
    [[webView windowScriptObject] callWebScriptMethod:@"willExecuteStatement" withArguments:args];
}

- (void)webView:(WebView *)view willLeaveCallFrame:(WebScriptCallFrame *)frame sourceId:(int)sid line:(int)lineno forWebFrame:(WebFrame *)webFrame
{
    if (!webViewLoaded)
        return;

    NSArray *args = [NSArray arrayWithObjects:[NSNumber numberWithInt:sid], [NSNumber numberWithInt:lineno], nil];
    [[webView windowScriptObject] callWebScriptMethod:@"willLeaveCallFrame" withArguments:args];
}
@end
