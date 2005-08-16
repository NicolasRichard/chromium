/*
 * Copyright (C) 2005 Apple Computer, Inc.  All rights reserved.
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

#import <WebKit/WebAssertions.h>
#import <WebKit/WebDataSource.h>
#import <WebKit/WebDocumentInternal.h>
#import <WebKit/WebDocumentPrivate.h>
#import <WebKit/WebFrame.h>
#import <WebKit/WebFrameInternal.h>
#import <WebKit/WebFrameView.h>
#import <WebKit/WebLocalizableStrings.h>
#import <WebKit/WebNSAttributedStringExtras.h>
#import <WebKit/WebNSPasteboardExtras.h>
#import <WebKit/WebNSViewExtras.h>
#import <WebKit/WebPDFView.h>
#import <WebKit/WebUIDelegate.h>
#import <WebKit/WebView.h>
#import <WebKit/WebViewPrivate.h>
#import <WebKit/WebPreferencesPrivate.h>

#import <WebKitSystemInterface.h>
#import <PDFKit/PDFKit.h>

// QuartzPrivate.h doesn't include the PDFKit private headers, so we can't get at PDFViewPriv.h. (3957971)
// Even if that was fixed, we'd have to tweak compile options to include QuartzPrivate.h. (3957839)

// This is a class that forwards everything it gets to a target and updates the PDF viewing prefs after
// each of those messages.  We use it as a way to hook all the places that the PDF viewing attrs change.
@interface PDFPrefUpdatingProxy : NSProxy {
    WebPDFView *view;
}
- (id)initWithView:(WebPDFView *)view;
@end

@interface PDFDocument (PDFKitSecretsIKnow)
- (NSPrintOperation *)getPrintOperationForPrintInfo:(NSPrintInfo *)printInfo autoRotate:(BOOL)doRotate;
@end

NSString *_NSPathForSystemFramework(NSString *framework);

@implementation WebPDFView

+ (NSBundle *)PDFKitBundle
{
    static NSBundle *PDFKitBundle = nil;
    if (PDFKitBundle == nil) {
        NSString *PDFKitPath = [_NSPathForSystemFramework(@"Quartz.framework") stringByAppendingString:@"/Frameworks/PDFKit.framework"];
        if (PDFKitPath == nil) {
            ERROR("Couldn't find PDFKit.framework");
            return nil;
        }
        PDFKitBundle = [NSBundle bundleWithPath:PDFKitPath];
        if (![PDFKitBundle load]) {
            ERROR("Couldn't load PDFKit.framework");
        }
    }
    return PDFKitBundle;
}

+ (Class)PDFViewClass
{
    static Class PDFViewClass = nil;
    if (PDFViewClass == nil) {
        PDFViewClass = [[WebPDFView PDFKitBundle] classNamed:@"PDFView"];
        if (PDFViewClass == nil) {
            ERROR("Couldn't find PDFView class in PDFKit.framework");
        }
    }
    return PDFViewClass;
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
        PDFSubview = [[[[self class] PDFViewClass] alloc] initWithFrame:frame];
        [PDFSubview setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
        [self addSubview:PDFSubview];
        [PDFSubview setDelegate:self];
        written = NO;
        firstLayoutDone = NO;
        // Messaging this proxy is the same as messaging PDFSubview, with the side effect that the
        // PDF viewing defaults are updated afterwards
        PDFSubviewProxy = (PDFView *)[[PDFPrefUpdatingProxy alloc] initWithView:self];
    }
    return self;
}

- (void)dealloc
{
    [PDFSubview release];
    [path release];
    [PDFSubviewProxy release];
    [super dealloc];
}

- (PDFView *)PDFSubview
{
    return PDFSubview;
}

- (void)copy:(id)sender
{
    [PDFSubview copy:sender];
}

#define TEMP_PREFIX "/tmp/XXXXXX-"
#define OBJC_TEMP_PREFIX @"/tmp/XXXXXX-"

static void applicationInfoForMIMEType(NSString *type, NSString **name, NSImage **image)
{
    NSURL *appURL = nil;
    
    OSStatus error = LSCopyApplicationForMIMEType((CFStringRef)type, kLSRolesAll, (CFURLRef *)&appURL);
    if(error != noErr){
        return;
    }

    NSString *appPath = [appURL path];
    CFRelease (appURL);
    
    *image = [[NSWorkspace sharedWorkspace] iconForFile:appPath];  
    [*image setSize:NSMakeSize(16.f,16.f)];  
    
    NSString *appName = [[NSFileManager defaultManager] displayNameAtPath:appPath];
    *name = appName;
}

- (NSString *)path
{
    // Generate path once.
    if (path)
        return path;
        
    NSString *filename = [[dataSource response] suggestedFilename];
    NSFileManager *manager = [NSFileManager defaultManager];    
    
    path = [@"/tmp/" stringByAppendingPathComponent: filename];
    if ([manager fileExistsAtPath:path]) {
        path = [OBJC_TEMP_PREFIX stringByAppendingString:filename];
        char *cpath = (char *)[path fileSystemRepresentation];
        
        int fd = mkstemps (cpath, strlen(cpath) - strlen(TEMP_PREFIX) + 1);
        if (fd < 0) {
            // Couldn't create a temporary file!  Should never happen.  Do
            // we need an alert here?
            path = nil;
        }
        else {
            close (fd);
            path = [manager stringWithFileSystemRepresentation:cpath length:strlen(cpath)];
        }
    }
    
    [path retain];
    
    return path;
}

- (NSView *)hitTest:(NSPoint)point
{
    // Override hitTest so we can override menuForEvent.
    NSEvent *event = [NSApp currentEvent];
    NSEventType type = [event type];
    if (type == NSRightMouseDown || (type == NSLeftMouseDown && ([event modifierFlags] & NSControlKeyMask))) {
        return self;
    }
    return [super hitTest:point];
}

- (BOOL)_pointIsInSelection:(NSPoint)point
{
    PDFPage *page = [PDFSubview pageForPoint:point nearest:NO];
    if (page == nil) {
        return NO;
    }
    
    NSRect selectionRect = [PDFSubview convertRect:[[PDFSubview currentSelection] boundsForPage:page] fromPage:page];
    
    return NSPointInRect(point, selectionRect);
}

- (NSDictionary *)elementAtPoint:(NSPoint)point
{
    WebFrame *frame = [dataSource webFrame];
    ASSERT(frame);

    return [NSDictionary dictionaryWithObjectsAndKeys:
        frame, WebElementFrameKey, 
        [NSNumber numberWithBool:[self _pointIsInSelection:point]], WebElementIsSelectedKey,
        nil];
}

- (NSMutableArray *)_menuItemsFromPDFKitForEvent:(NSEvent *)theEvent
{
    NSMutableArray *copiedItems = [NSMutableArray array];
    NSDictionary *actionsToTags = [[NSDictionary alloc] initWithObjectsAndKeys:
        [NSNumber numberWithInt:WebMenuItemPDFActualSize], NSStringFromSelector(@selector(_setActualSize:)),
        [NSNumber numberWithInt:WebMenuItemPDFZoomIn], NSStringFromSelector(@selector(zoomIn:)),
        [NSNumber numberWithInt:WebMenuItemPDFZoomOut], NSStringFromSelector(@selector(zoomOut:)),
        [NSNumber numberWithInt:WebMenuItemPDFAutoSize], NSStringFromSelector(@selector(_setAutoSize:)),
        [NSNumber numberWithInt:WebMenuItemPDFSinglePage], NSStringFromSelector(@selector(_setSinglePage:)),
        [NSNumber numberWithInt:WebMenuItemPDFFacingPages], NSStringFromSelector(@selector(_setDoublePage:)),
        [NSNumber numberWithInt:WebMenuItemPDFContinuous], NSStringFromSelector(@selector(_toggleContinuous:)),
        [NSNumber numberWithInt:WebMenuItemPDFNextPage], NSStringFromSelector(@selector(goToNextPage:)),
        [NSNumber numberWithInt:WebMenuItemPDFPreviousPage], NSStringFromSelector(@selector(goToPreviousPage:)),
        nil];
    
    NSEnumerator *e = [[[PDFSubview menuForEvent:theEvent] itemArray] objectEnumerator];
    NSMenuItem *item;
    while ((item = [e nextObject]) != nil) {
        // Copy items since a menu item can be in only one menu at a time, and we don't
        // want to modify the original menu supplied by PDFKit.
        NSMenuItem *itemCopy = [item copy];
        [copiedItems addObject:itemCopy];
        
        if ([itemCopy isSeparatorItem]) {
            continue;
        }
        NSString *actionString = NSStringFromSelector([itemCopy action]);
        NSNumber *tagNumber = [actionsToTags objectForKey:actionString];
        
        int tag;
        if (tagNumber != nil) {
            tag = [tagNumber intValue];
        } else {
            tag = WebMenuItemTagOther;
            ERROR("no WebKit menu item tag found for PDF context menu item action \"%@\", using WebMenuItemTagOther", actionString);
        }
        if ([itemCopy tag] == 0) {
            [itemCopy setTag:tag];
            if ([itemCopy target] == PDFSubview) {
                // Note that updating the defaults is cheap because it catches redundant settings, so installing
                // the proxy for actions that don't impact the defaults is OK
                [itemCopy setTarget:PDFSubviewProxy];
            }
        } else {
            ERROR("PDF context menu item %@ came with tag %d, so no WebKit tag was applied. This could mean that the item doesn't appear in clients such as Safari.", [itemCopy title], [itemCopy tag]);
        }
    }
    
    [actionsToTags release];
    
    return copiedItems;
}

- (BOOL)_anyPDFTagsFoundInMenu:(NSMenu *)menu
{
    NSEnumerator *e = [[menu itemArray] objectEnumerator];
    NSMenuItem *item;
    while ((item = [e nextObject]) != nil) {
        switch ([item tag]) {
            case WebMenuItemTagOpenWithDefaultApplication:
            case WebMenuItemPDFActualSize:
            case WebMenuItemPDFZoomIn:
            case WebMenuItemPDFZoomOut:
            case WebMenuItemPDFAutoSize:
            case WebMenuItemPDFSinglePage:
            case WebMenuItemPDFFacingPages:
            case WebMenuItemPDFContinuous:
            case WebMenuItemPDFNextPage:
            case WebMenuItemPDFPreviousPage:
                return YES;
        }
    }
    return NO;
}

- (NSMenu *)menuForEvent:(NSEvent *)theEvent
{
    // Start with the menu items supplied by PDFKit, with WebKit tags applied
    NSMutableArray *items = [self _menuItemsFromPDFKitForEvent:theEvent];

    // Add in an "Open with <default PDF viewer>" item
    NSString *appName = nil;
    NSImage *appIcon = nil;
    
    applicationInfoForMIMEType([[dataSource response] MIMEType], &appName, &appIcon);
    if (!appName)
        appName = UI_STRING("Finder", "Default application name for Open With context menu");
    
    NSString *title = [NSString stringWithFormat:UI_STRING("Open with %@", "context menu item for PDF"), appName];
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:title action:@selector(openWithFinder:) keyEquivalent:@""];
    [item setTag:WebMenuItemTagOpenWithDefaultApplication];
    if (appIcon)
        [item setImage:appIcon];
    [items insertObject:item atIndex:0];
    [item release];
    
    [items insertObject:[NSMenuItem separatorItem] atIndex:1];
    
    // pass the items off to the WebKit context menu mechanism
    WebView *webView = [[dataSource webFrame] webView];
    ASSERT(webView);
    NSMenu *menu = [webView _menuForElement:[self elementAtPoint:[self convertPoint:[theEvent locationInWindow] fromView:nil]] defaultItems:items];
    
    // The delegate has now had the opportunity to add items to the standard PDF-related items, or to
    // remove or modify some of the PDF-related items. In 10.4, the PDF context menu did not go through 
    // the standard WebKit delegate path, and so the standard PDF-related items always appeared. For
    // clients that create their own context menu by hand-picking specific items from the default list, such as
    // Safari, none of the PDF-related items will appear until the client is rewritten to explicitly
    // include these items. So for backwards compatibility we're going to include the entire set of PDF-related
    // items if the executable was linked in 10.4 or earlier and the menu returned from the delegate mechanism
    // contains none of the PDF-related items.
    if (WKExecutableLinkedInTigerOrEarlier()) {
        if (![self _anyPDFTagsFoundInMenu:menu]) {
            [menu addItem:[NSMenuItem separatorItem]];
            NSEnumerator *e = [items objectEnumerator];
            NSMenuItem *menuItem;
            while ((menuItem = [e nextObject]) != nil) {
                // copy menuItem since a given menuItem can be in only one menu at a time, and we don't
                // want to mess with the menu returned from PDFKit.
                [menu addItem:[menuItem copy]];
            }
        }
    }
    
    return menu;
}

- (void)setDataSource:(WebDataSource *)ds
{
    dataSource = ds;
    [self setFrame:[[self superview] frame]];
}

- (void)_readPDFDefaults
{
    // Set up default viewing params
    WebPreferences *prefs = [[self _webView] preferences];
    float scaleFactor = [prefs PDFScaleFactor];
    if (scaleFactor == 0) {
        [PDFSubview setAutoScales:YES];
    } else {
        [PDFSubview setAutoScales:NO];
        [PDFSubview setScaleFactor:scaleFactor];
    }
    [PDFSubview setDisplayMode:[prefs PDFDisplayMode]];    
}

- (void)dataSourceUpdated:(WebDataSource *)dataSource
{
}

- (void)setNeedsLayout:(BOOL)flag
{
}

- (void)layout
{
    if (!firstLayoutDone) {
        firstLayoutDone = YES;
        // Be wary of moving the point where we do this restore.  PDFKit apparently has some ordering issues
        // as to when this is done.  For example, if you do it in this class' init method, it sometimes has no
        // effect.  When I tried it in setDataSource:, the window got in a weird state where no more drawing
        // ever occurred, but the app was not hung.
        [self _readPDFDefaults];
    }
}

- (void)viewWillMoveToHostWindow:(NSWindow *)hostWindow
{
}

- (void)viewDidMoveToHostWindow
{
}

- (void)openWithFinder:(id)sender
{
    NSString *opath = [self path];
    
    if (opath) {
        if (!written) {
            [[dataSource data] writeToFile:opath atomically:YES];
            written = YES;
        }
    
        if (![[NSWorkspace sharedWorkspace] openFile:opath]) {
            // NSWorkspace couldn't open file.  Do we need an alert
            // here?  We ignore the error elsewhere.
        }
    }
}

// FIXME 4182876: We can eliminate this function in favor if -isEqual: if [PDFSelection isEqual:] is overridden
// to compare contents.
static BOOL PDFSelectionsAreEqual(PDFSelection *selectionA, PDFSelection *selectionB)
{
    NSArray *aPages = [selectionA pages];
    NSArray *bPages = [selectionB pages];
    
    if (![aPages isEqual:bPages]) {
        return NO;
    }
    
    int count = [aPages count];
    int i;
    for (i = 0; i < count; ++i) {
        NSRect aBounds = [selectionA boundsForPage:[aPages objectAtIndex:i]];
        NSRect bBounds = [selectionB boundsForPage:[bPages objectAtIndex:i]];
        if (!NSEqualRects(aBounds, bBounds)) {
            return NO;
        }
    }
    
    return YES;
}

- (BOOL)searchFor:(NSString *)string direction:(BOOL)forward caseSensitive:(BOOL)caseFlag wrap:(BOOL)wrapFlag
{
    // Our search algorithm, used in WebCore also, is to search in the selection first. If the found text is the
    // entire selection, then we search again from just past the selection.

    int options = 0;
    if (!forward) {
        options |= NSBackwardsSearch;
    }
    if (!caseFlag) {
        options |= NSCaseInsensitiveSearch;
    }
    PDFDocument *document = [PDFSubview document];
    PDFSelection *oldSelection = [PDFSubview currentSelection];
    
    // Initially we want to include the selected text in the search. PDFDocument's API always searches from just
    // past the passed-in selection, so we need to pass a selection that's modified appropriately. 
    // FIXME 4182863: Ideally we'd use a zero-length selection at the edge of the current selection, but zero-length
    // selections don't work in PDFDocument. So instead we make a one-length selection just before or after the
    // current selection, which works for our purposes even when the current selection is at an edge of the
    // document.
    PDFSelection *selectionForInitialSearch = [oldSelection copy];
    int oldSelectionLength = [[oldSelection string] length];
    if (forward) {
        [selectionForInitialSearch extendSelectionAtStart:1];
        [selectionForInitialSearch extendSelectionAtEnd:-oldSelectionLength];
    } else {
        [selectionForInitialSearch extendSelectionAtEnd:1];
        [selectionForInitialSearch extendSelectionAtStart:-oldSelectionLength];
    }
    PDFSelection *foundSelection = [document findString:string fromSelection:selectionForInitialSearch withOptions:options];
    [selectionForInitialSearch release];
    
    // If we found the selection, search again from just past the selection
    if (PDFSelectionsAreEqual(foundSelection, oldSelection)) {
        foundSelection = [document findString:string fromSelection:oldSelection withOptions:options];
    }
    
    if (foundSelection == nil && wrapFlag) {
        foundSelection = [document findString:string fromSelection:nil withOptions:options];
    }
    if (foundSelection != nil) {
        [PDFSubview setCurrentSelection:foundSelection];
        [PDFSubview scrollSelectionToVisible:nil];
        return YES;
    }
    return NO;
}

- (void)takeFindStringFromSelection:(id)sender
{
    [NSPasteboard _web_setFindPasteboardString:[[PDFSubview currentSelection] string] withOwner:self];
}

- (void)jumpToSelection:(id)sender
{
    [PDFSubview scrollSelectionToVisible:nil];
}

- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)item 
{
    SEL action = [item action];    
    if (action == @selector(takeFindStringFromSelection:) || action == @selector(jumpToSelection:)) {
        return [PDFSubview currentSelection] != nil;
    }
    return YES;
}

- (BOOL)canPrintHeadersAndFooters
{
    return NO;
}

- (NSPrintOperation *)printOperationWithPrintInfo:(NSPrintInfo *)printInfo
{
    return [[PDFSubview document] getPrintOperationForPrintInfo:printInfo autoRotate:YES];
}

// Delegates implementing the following method will be called to handle clicks on URL
// links within the PDFView.  
- (void)PDFViewWillClickOnLink:(PDFView *)sender withURL:(NSURL *)URL
{
    if (URL != nil) {    
        WebFrame *frame = [[self _web_parentWebFrameView] webFrame];
        [frame _safeLoadURL:URL];
    }
}

/*** WebDocumentText protocol implementation ***/

- (BOOL)supportsTextEncoding
{
    return NO;
}

- (NSString *)string
{
    return [[PDFSubview document] string];
}

- (NSAttributedString *)attributedString
{
    // changing the selection is a hack, but the only way to get an attr string is via PDFSelection
    
    // must copy this selection object because we change the selection which seems to release it
    PDFSelection *savedSelection = [[PDFSubview currentSelection] copy];
    [PDFSubview selectAll:nil];
    NSAttributedString *result = [[PDFSubview currentSelection] attributedString];
    if (savedSelection) {
        [PDFSubview setCurrentSelection:savedSelection];
        [savedSelection release];
    } else {
        // FIXME: behavior of setCurrentSelection:nil is not documented - check 4182934 for progress
        // Otherwise, we could collapse this code with the case above.
        [PDFSubview clearSelection];
    }
    return result;
}

- (NSString *)selectedString
{
    return [[PDFSubview currentSelection] string];
}

- (NSAttributedString *)selectedAttributedString
{
    return [[PDFSubview currentSelection] attributedString];
}

- (void)selectAll
{
    [PDFSubview selectAll:nil];
}

- (void)deselectAll
{
    [PDFSubview clearSelection];
}

/*** WebDocumentViewState protocol implementation ***/

// Even though to WebKit we are the "docView", in reality a PDFView contains its own scrollview and docView.
// And it even turns out there is another PDFKit view between the docView and its enclosing ScrollView, so
// we have to be sure to do our calculations based on that view, immediately inside the ClipView.  We try
// to make as few assumptions about the PDFKit view hierarchy as possible.

- (NSPoint)scrollPoint
{
    NSView *realDocView = [PDFSubview documentView];
    NSClipView *clipView = [[realDocView enclosingScrollView] contentView];
    return [clipView bounds].origin;
}

- (void)setScrollPoint:(NSPoint)p
{
    WebFrame *frame = [dataSource webFrame];
    //FIXME:  We only restore scroll state in the non-frames case because otherwise we get a crash due to
    // PDFKit calling display from within its drawRect:. See bugzilla 4164.
    if (![frame parentFrame]) {
        NSView *realDocView = [PDFSubview documentView];
        [[[realDocView enclosingScrollView] documentView] scrollPoint:p];
    }
}

- (id)viewState
{
    NSMutableArray *state = [NSMutableArray arrayWithCapacity:4];
    PDFDisplayMode mode = [PDFSubview displayMode];
    [state addObject:[NSNumber numberWithInt:mode]];
    if (mode == kPDFDisplaySinglePage || mode == kPDFDisplayTwoUp) {
        unsigned int pageIndex = [[PDFSubview document] indexForPage:[PDFSubview currentPage]];
        [state addObject:[NSNumber numberWithUnsignedInt:pageIndex]];
    }  // else in continuous modes, scroll position gets us to the right page
    BOOL autoScaleFlag = [PDFSubview autoScales];
    [state addObject:[NSNumber numberWithBool:autoScaleFlag]];
    if (!autoScaleFlag) {
        [state addObject:[NSNumber numberWithFloat:[PDFSubview scaleFactor]]];
    }
    return state;
}

- (void)setViewState:(id)statePList
{
    ASSERT([statePList isKindOfClass:[NSArray class]]);
    NSArray *state = statePList;
    int i = 0;
    PDFDisplayMode mode = [[state objectAtIndex:i++] intValue];
    [PDFSubview setDisplayMode:mode];
    if (mode == kPDFDisplaySinglePage || mode == kPDFDisplayTwoUp) {
        unsigned int pageIndex = [[state objectAtIndex:i++] unsignedIntValue];
        [PDFSubview goToPage:[[PDFSubview document] pageAtIndex:pageIndex]];
    }  // else in continuous modes, scroll position gets us to the right page
    BOOL autoScaleFlag = [[state objectAtIndex:i++] boolValue];
    [PDFSubview setAutoScales:autoScaleFlag];
    if (!autoScaleFlag) {
        [PDFSubview setScaleFactor:[[state objectAtIndex:i++] floatValue]];
    }
}

/*** _WebDocumentTextSizing protocol implementation ***/

- (IBAction)_makeTextSmaller:(id)sender
{
    [PDFSubviewProxy zoomOut:sender];
}

- (IBAction)_makeTextLarger:(id)sender
{
    [PDFSubviewProxy zoomIn:sender];
}

- (IBAction)_makeTextStandardSize:(id)sender
{
    [PDFSubviewProxy setScaleFactor:1.0];
}

- (BOOL)_tracksCommonSizeFactor
{
    // We keep our own scale factor instead of tracking the common one in the WebView for a couple reasons.
    // First, PDFs tend to have visually smaller text because they are laid out for a printed page instead of
    // the screen.  Second, the PDFView feature of AutoScaling means our scaling factor can be quiet variable.
    return NO;
}

- (BOOL)_canMakeTextSmaller
{
    return [PDFSubview canZoomOut];
}

- (BOOL)_canMakeTextLarger
{
    return [PDFSubview canZoomIn];
}

- (BOOL)_canMakeTextStandardSize
{
    return [PDFSubview scaleFactor] != 1.0;
}


- (NSRect)selectionRect
{
    NSRect result = NSZeroRect;
    PDFSelection *selection = [PDFSubview currentSelection];
    NSEnumerator *pages = [[selection pages] objectEnumerator];
    PDFPage *page;
    while ((page = [pages nextObject]) != nil) {
        NSRect selectionOnPageInViewCoordinates = [PDFSubview convertRect:[selection boundsForPage:page] fromPage:page];
        if (NSIsEmptyRect(result)) {
            result = selectionOnPageInViewCoordinates;
        } else {
            result = NSUnionRect(result, selectionOnPageInViewCoordinates);
        }
    }
    
    return result;
}

- (NSArray *)pasteboardTypesForSelection
{
    return [NSArray arrayWithObjects:NSRTFDPboardType, NSRTFPboardType, NSStringPboardType, nil];
}

- (void)writeSelectionWithPasteboardTypes:(NSArray *)types toPasteboard:(NSPasteboard *)pasteboard
{
    NSAttributedString *attributedString = [self selectedAttributedString];
    
    if ([types containsObject:NSRTFDPboardType]) {
        NSData *RTFDData = [attributedString RTFDFromRange:NSMakeRange(0, [attributedString length]) documentAttributes:nil];
        [pasteboard setData:RTFDData forType:NSRTFDPboardType];
    }        
    
    if ([types containsObject:NSRTFPboardType]) {
        if ([attributedString containsAttachments]) {
            attributedString = [attributedString _web_attributedStringByStrippingAttachmentCharacters];
        }
        NSData *RTFData = [attributedString RTFFromRange:NSMakeRange(0, [attributedString length]) documentAttributes:nil];
        [pasteboard setData:RTFData forType:NSRTFPboardType];
    }
    
    if ([types containsObject:NSStringPboardType]) {
        [pasteboard setString:[self selectedString] forType:NSStringPboardType];
    }
}

@end

@implementation PDFPrefUpdatingProxy

- (id)initWithView:(WebPDFView *)aView
{
    // No [super init], since we inherit from NSProxy
    view = aView;
    return self;
}

- (void)forwardInvocation:(NSInvocation *)invocation
{
    PDFView *PDFSubview = [view PDFSubview];
    [invocation invokeWithTarget:PDFSubview];

    WebPreferences *prefs = [[view _webView] preferences];
    float scaleFactor = [PDFSubview autoScales] ? 0.0 : [PDFSubview scaleFactor];
    [prefs setPDFScaleFactor:scaleFactor];
    [prefs setPDFDisplayMode:[PDFSubview displayMode]];
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)sel
{
    return [[view PDFSubview] methodSignatureForSelector:sel];
}

@end
