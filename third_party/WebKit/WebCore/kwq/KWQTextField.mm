/*
 * Copyright (C) 2003 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#import "KWQTextField.h"

#import "KWQView.h"
#import "KWQLineEdit.h"
#import "KWQKHTMLPart.h"
#import "KWQNSViewExtras.h"
#import "WebCoreFirstResponderChanges.h"
#import "WebCoreBridge.h"

// KWQTextFieldFormatter enforces a maximum length.

@interface KWQTextFieldFormatter : NSFormatter
{
    int maxLength;
}

- (void)setMaximumLength:(int)len;
- (int)maximumLength;

@end

// KWQSecureTextField has two purposes.
// One is a workaround for bug 3024443.
// The other is hook up next and previous key views to KHTML.

@interface KWQSecureTextField : NSSecureTextField <KWQWidgetHolder>
{
    QLineEdit *widget;
    BOOL inSetFrameSize;
    BOOL inNextValidKeyView;
}

- initWithQLineEdit:(QLineEdit *)widget;

@end

@implementation KWQTextField

- (void)setUpTextField:(NSTextField *)field
{
    // This is initialization that's shared by both self and the secure text field.

    [[field cell] setScrollable:YES];
    
    [field setFormatter:formatter];

    [field setDelegate:self];
    
    [field setTarget:self];
    [field setAction:@selector(action:)];
}

- initWithFrame:(NSRect)frame
{
    [super initWithFrame:frame];
    formatter = [[KWQTextFieldFormatter alloc] init];
    [self setUpTextField:self];
    return self;
}

- initWithQLineEdit:(QLineEdit *)w 
{
    widget = w;
    return [super init];
}

- (void)action:sender
{
    widget->returnPressed();
}

- (void)dealloc
{
    [secureField release];
    [formatter release];
    [super dealloc];
}

- (KWQTextFieldFormatter *)formatter
{
    return formatter;
}

- (void)updateSecureFieldFrame
{
    [secureField setFrame:[self bounds]];
}

- (void)setFrameSize:(NSSize)size
{
    [super setFrameSize:size];
    [self updateSecureFieldFrame];
}

- (void)setPasswordMode:(BOOL)flag
{
    if (!flag == ![secureField superview]) {
        return;
    }
    
    if (!flag) {
        // Don't use [self setStringValue:] because there are unwanted side effects,
        // like sending out a text changed signal.
        [super setStringValue:[secureField stringValue]];
        [secureField removeFromSuperview];
    } else {
        if (secureField == nil) {
            secureField = [[KWQSecureTextField alloc] initWithQLineEdit:widget];
            [secureField setFormatter:formatter];
            [secureField setFont:[self font]];
            [secureField setEditable:[self isEditable]];
            [self setUpTextField:secureField];
            [self updateSecureFieldFrame];
        }
        [secureField setStringValue:[super stringValue]];
        [self addSubview:secureField];
    }
}

- (void)setEditable:(BOOL)flag
{
    [secureField setEditable:flag];
    [super setEditable:flag];
}

- (void)selectText:(id)sender
{
    if ([self passwordMode]) {
        [secureField selectText:sender];
        return;
    }
    
    // Don't call the NSTextField's selectText if the field is already first responder.
    // If we do, we'll end up deactivating and then reactivating, which will send
    // unwanted onBlur events.
    NSText *editor = [self currentEditor];
    if (editor) {
        [editor setSelectedRange:NSMakeRange(0, [[editor string] length])];
        return;
    }
    
    [super selectText:sender];
}

- (BOOL)isEditable
{
    return [super isEditable];
}

- (BOOL)passwordMode
{
    return [secureField superview] != nil;
}

- (void)setMaximumLength:(int)len
{
    NSString *oldValue = [self stringValue];
    if ((int)[oldValue length] > len) {
        [self setStringValue:[oldValue substringToIndex:len]];
    }
    [formatter setMaximumLength:len];
}

- (int)maximumLength
{
    return [formatter maximumLength];
}

- (BOOL)edited
{
    return edited;
}

- (void)setEdited:(BOOL)ed
{
    edited = ed;
}

- (void)controlTextDidBeginEditing:(NSNotification *)obj
{
    WebCoreBridge *bridge = KWQKHTMLPart::bridgeForWidget(widget);
    [bridge controlTextDidBeginEditing:obj];
}

- (void)controlTextDidEndEditing:(NSNotification *)obj
{
    WebCoreBridge *bridge = KWQKHTMLPart::bridgeForWidget(widget);
    [bridge controlTextDidEndEditing:obj];
}

- (void)controlTextDidChange:(NSNotification *)obj
{
    WebCoreBridge *bridge = KWQKHTMLPart::bridgeForWidget(widget);
    [bridge controlTextDidChange:obj];
    edited = true;
    widget->textChanged();
}

- (BOOL)control:(NSControl *)control textShouldBeginEditing:(NSText *)fieldEditor
{
    WebCoreBridge *bridge = KWQKHTMLPart::bridgeForWidget(widget);
    BOOL should = [bridge control:control textShouldBeginEditing:fieldEditor];

    if (should) {
	QFocusEvent event(QEvent::FocusIn);
	(const_cast<QObject *>(widget->eventFilterObject()))->eventFilter(widget, &event);
    }

    return should;
}

- (BOOL)control:(NSControl *)control textShouldEndEditing:(NSText *)fieldEditor
{
    WebCoreBridge *bridge = KWQKHTMLPart::bridgeForWidget(widget);
    BOOL should = [bridge control:control textShouldEndEditing:fieldEditor];

    if (should) {
	QFocusEvent event(QEvent::FocusOut);
	(const_cast<QObject *>(widget->eventFilterObject()))->eventFilter(widget, &event);
    }

    return should;
}

- (BOOL)control:(NSControl *)control didFailToFormatString:(NSString *)string errorDescription:(NSString *)error
{
    WebCoreBridge *bridge = KWQKHTMLPart::bridgeForWidget(widget);
    return [bridge control:control didFailToFormatString:string errorDescription:error];
}

- (void)control:(NSControl *)control didFailToValidatePartialString:(NSString *)string errorDescription:(NSString *)error
{
    WebCoreBridge *bridge = KWQKHTMLPart::bridgeForWidget(widget);
    [bridge control:control didFailToValidatePartialString:string errorDescription:error];
}

- (BOOL)control:(NSControl *)control isValidObject:(id)obj
{
    WebCoreBridge *bridge = KWQKHTMLPart::bridgeForWidget(widget);
    return [bridge control:control isValidObject:obj];
}

- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)commandSelector
{
    WebCoreBridge *bridge = KWQKHTMLPart::bridgeForWidget(widget);
    return [bridge control:control textView:textView doCommandBySelector:commandSelector];
}

- (NSString *)stringValue
{
    if ([secureField superview]) {
        return [secureField stringValue];
    }
    return [super stringValue];
}

- (void)setStringValue:(NSString *)string
{
    int maxLength = [formatter maximumLength];
    if ((int)[string length] > maxLength) {
        string = [string substringToIndex:maxLength];
    }
    [secureField setStringValue:string];
    [super setStringValue:string];
    widget->textChanged();
}

- (void)setFont:(NSFont *)font
{
    [secureField setFont:font];
    [super setFont:font];
}

- (NSView *)nextKeyView
{
    return inNextValidKeyView
        ? KWQKHTMLPart::nextKeyViewForWidget(widget, KWQSelectingNext)
        : [super nextKeyView];
}

- (NSView *)previousKeyView
{
   return inNextValidKeyView
        ? KWQKHTMLPart::nextKeyViewForWidget(widget, KWQSelectingPrevious)
        : [super previousKeyView];
}

- (NSView *)nextValidKeyView
{
    inNextValidKeyView = YES;
    NSView *view = [super nextValidKeyView];
    inNextValidKeyView = NO;
    return view;
}

- (NSView *)previousValidKeyView
{
    inNextValidKeyView = YES;
    NSView *view = [super previousValidKeyView];
    inNextValidKeyView = NO;
    return view;
}

- (BOOL)becomeFirstResponder
{
    if ([self passwordMode]) {
        return [[self window] makeFirstResponder:secureField];
    }
    KWQKHTMLPart::setDocumentFocus(widget);
    [self _KWQ_scrollFrameToVisible];
    return [super becomeFirstResponder];
}

- (void)display
{
    // This is a workaround for Radar 2753974.
    // Also, in the web page context, it's never OK to just display.
    [self setNeedsDisplay:YES];
}

- (QWidget *)widget
{
    return widget;
}

- (void)fieldEditorDidMouseDown:(NSEvent *)event
{
    widget->sendConsumedMouseUp();
    widget->clicked();
}

@end

@implementation KWQTextFieldFormatter

- init
{
    [super init];
    maxLength = INT_MAX;
    return self;
}

- (void)setMaximumLength:(int)len
{
    maxLength = len;
}

- (int)maximumLength
{
    return maxLength;
}

- (NSString *)stringForObjectValue:(id)object
{
    return (NSString *)object;
}

- (BOOL)getObjectValue:(id *)object forString:(NSString *)string errorDescription:(NSString **)error
{
    *object = string;
    return YES;
}

- (BOOL)isPartialStringValid:(NSString *)partialString newEditingString:(NSString **)newString errorDescription:(NSString **)error
{
    if ((int)[partialString length] > maxLength) {
        *newString = nil;
        return NO;
    }

    return YES;
}

- (NSAttributedString *)attributedStringForObjectValue:(id)anObject withDefaultAttributes:(NSDictionary *)attributes
{
    return nil;
}

@end

@implementation KWQSecureTextField

- initWithQLineEdit:(QLineEdit *)w
{
    widget = w;
    return [super init];
}

- (NSView *)nextKeyView
{
    return inNextValidKeyView
        ? KWQKHTMLPart::nextKeyViewForWidget(widget, KWQSelectingNext)
        : [super nextKeyView];
}

- (NSView *)previousKeyView
{
   return inNextValidKeyView
        ? KWQKHTMLPart::nextKeyViewForWidget(widget, KWQSelectingPrevious)
        : [super previousKeyView];
}

- (NSView *)nextValidKeyView
{
    inNextValidKeyView = YES;
    NSView *view = [super nextValidKeyView];
    inNextValidKeyView = NO;
    return view;
}

- (NSView *)previousValidKeyView
{
    inNextValidKeyView = YES;
    NSView *view = [super previousValidKeyView];
    inNextValidKeyView = NO;
    return view;
}

// These next two methods are the workaround for bug 3024443.
// Basically, setFrameSize ends up calling an inappropriate selectText, so we just ignore
// calls to selectText while setFrameSize is running.

- (void)selectText:(id)sender
{
    if (sender == self && inSetFrameSize) {
        return;
    }
    
    // Don't call the NSSecureTextField's selectText if the field is already first responder.
    // If we do, we'll end up deactivating and then reactivating, which will send
    // unwanted onBlur events and wreak havoc in other ways as well by setting the focus
    // back to the window.
    NSResponder *firstResponder = [[self window] firstResponder];
    if ([firstResponder isKindOfClass:[NSTextView class]]) {
        NSTextView *textView = (NSTextView *)firstResponder;
        if ([textView delegate] == self) {
            [textView setSelectedRange:NSMakeRange(0, [[textView string] length])];
            return;
        }
    }

    [super selectText:sender];
}

- (void)setFrameSize:(NSSize)size
{
    inSetFrameSize = YES;
    [super setFrameSize:size];
    inSetFrameSize = NO;
}

- (BOOL)becomeFirstResponder
{
    KWQKHTMLPart::setDocumentFocus(widget);
    [self _KWQ_scrollFrameToVisible];
    return [super becomeFirstResponder];
}

- (void)display
{
    // This is a workaround for Radar 2753974.
    // Also, in the web page context, it's never OK to just display.
    [self setNeedsDisplay:YES];
}

- (QWidget *)widget
{
    return widget;
}

- (void)fieldEditorDidMouseDown:(NSEvent *)event
{
    widget->sendConsumedMouseUp();
    widget->clicked();
}

@end
