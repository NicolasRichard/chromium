//
//  WebDynamicScrollBarsView.h
//  WebKit
//
//  Created by John Sullivan on Tue Jan 22 2002.
//  Copyright (c) 2001, 2002 Apple Computer, Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import <WebCore/WebCoreFrameView.h>

// FIXME 2980779: This has grown to be more than just a dynamic scroll bar view,
// and it no longer works outside of WebKit.

@interface WebDynamicScrollBarsView : NSScrollView <WebCoreFrameView>
{
    BOOL disallowsScrolling;
    BOOL suppressLayout;
}

- (void)setAllowsScrolling:(BOOL)flag;
- (BOOL)allowsScrolling;
- (void)updateScrollers;
- (void)setSuppressLayout: (BOOL)flag;
@end
