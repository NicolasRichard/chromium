/*	IFImageRenderer.h
	Copyright 2002, Apple, Inc. All rights reserved.
*/

#import <Cocoa/Cocoa.h>
#import <WebCore/WebCoreImageRenderer.h>

@interface IFImageRenderer : NSImage <WebCoreImageRenderer>
{
    NSTimer *frameTimer;
    NSView *frameView;
    NSRect imageRect;
    NSRect targetRect;
    int loadStatus;
    NSColor *patternColor;
    int patternColorLoadStatus;
    BOOL animationFinished;
}

+ (void)stopAnimationsInView: (NSView *)aView;

@end
