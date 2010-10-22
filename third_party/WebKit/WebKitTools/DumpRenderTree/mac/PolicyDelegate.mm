/*
 * Copyright (C) 2007, 2009 Apple Inc. All rights reserved.
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

#import "config.h"
#import "PolicyDelegate.h"

#import "DumpRenderTree.h"
#import "LayoutTestController.h"
#import <WebKit/DOMElement.h>
#import <WebKit/WebPolicyDelegate.h>
#import <WebKit/WebView.h>

@interface NSURL (DRTExtras)
- (NSString *)_drt_descriptionSuitableForTestResult;
@end

@interface DOMNode (dumpPath)
- (NSString *)dumpPath;
@end

@implementation PolicyDelegate

- (void)webView:(WebView *)webView decidePolicyForNavigationAction:(NSDictionary *)actionInformation
                                                           request:(NSURLRequest *)request
                                                             frame:(WebFrame *)frame
                                                  decisionListener:(id<WebPolicyDecisionListener>)listener
{
    WebNavigationType navType = (WebNavigationType)[[actionInformation objectForKey:WebActionNavigationTypeKey] intValue];

    const char* typeDescription;
    switch (navType) {
        case WebNavigationTypeLinkClicked:
            typeDescription = "link clicked";
            break;
        case WebNavigationTypeFormSubmitted:
            typeDescription = "form submitted";
            break;
        case WebNavigationTypeBackForward:
            typeDescription = "back/forward";
            break;
        case WebNavigationTypeReload:
            typeDescription = "reload";
            break;
        case WebNavigationTypeFormResubmitted:
            typeDescription = "form resubmitted";
            break;
        case WebNavigationTypeOther:
            typeDescription = "other";
            break;
        default:
            typeDescription = "illegal value";
    }

    NSString *message = [NSString stringWithFormat:@"Policy delegate: attempt to load %@ with navigation type '%s'", [[request URL] _drt_descriptionSuitableForTestResult], typeDescription];

    if (DOMElement *originatingNode = [[actionInformation objectForKey:WebActionElementKey] objectForKey:WebElementDOMNodeKey])
        message = [message stringByAppendingFormat:@" originating from %@", [originatingNode dumpPath]];

    printf("%s\n", [message UTF8String]);

    if (_permissiveDelegate)
        [listener use];
    else
        [listener ignore];

    if (_controllerToNotifyDone) {
        _controllerToNotifyDone->notifyDone();
        _controllerToNotifyDone = 0;
    }
}

- (void)webView:(WebView *)webView decidePolicyForMIMEType:(NSString *)type
                                                   request:(NSURLRequest *)request
                                                     frame:(WebFrame *)frame
                                          decisionListener:(id<WebPolicyDecisionListener>)listener
{
    if (!_callIgnoreInDecidePolicyForMIMETypeAfterOneSecond) {
        [listener use];
        return;
    }

    [(NSObject *)listener performSelector:@selector(ignore) withObject:nil afterDelay:1.0];
}

- (void)webView:(WebView *)webView unableToImplementPolicyWithError:(NSError *)error frame:(WebFrame *)frame
{
    NSString *message = [NSString stringWithFormat:@"Policy delegate: unable to implement policy with error domain '%@', error code %d, in frame '%@'", [error domain], [error code], [frame name]];
    printf("%s\n", [message UTF8String]);
}

- (void)setCallIgnoreInDecidePolicyForMIMETypeAfterOneSecond:(BOOL)callIgnoreInDecidePolicyForMIMETypeAfterOneSecond
{
    _callIgnoreInDecidePolicyForMIMETypeAfterOneSecond = callIgnoreInDecidePolicyForMIMETypeAfterOneSecond;
}

- (void)setPermissive:(BOOL)permissive
{
    _permissiveDelegate = permissive;
}

- (void)setControllerToNotifyDone:(LayoutTestController*)controller
{
    _controllerToNotifyDone = controller;
}

@end
