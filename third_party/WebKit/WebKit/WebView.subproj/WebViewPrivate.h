/*	
    WebViewPrivate.m
    Copyright 2001, Apple, Inc. All rights reserved.
*/

#import <WebKit/WebPolicyDelegate.h>
#import <WebKit/WebView.h>
#import <WebKit/WebFramePrivate.h>

@class DOMCSSStyleDeclaration;
@class DOMNode;
@class DOMRange;
@class NSError;
@class WebBackForwardList;
@class WebFrame;
@class WebPreferences;
@class WebCoreSettings;
@protocol WebFormDelegate;

#define NUM_LOCATION_CHANGE_DELEGATE_SELECTORS	10

typedef struct _WebResourceDelegateImplementationCache {
    uint delegateImplementsDidCancelAuthenticationChallenge:1;
    uint delegateImplementsDidReceiveAuthenticationChallenge:1;
    uint delegateImplementsDidReceiveResponse:1;
    uint delegateImplementsDidReceiveContentLength:1;
    uint delegateImplementsDidFinishLoadingFromDataSource:1;
    uint delegateImplementsWillSendRequest:1;
    uint delegateImplementsIdentifierForRequest:1;
} WebResourceDelegateImplementationCache;

extern NSString *_WebCanGoBackKey;
extern NSString *_WebCanGoForwardKey;
extern NSString *_WebEstimatedProgressKey;
extern NSString *_WebIsLoadingKey;
extern NSString *_WebMainFrameIconKey;
extern NSString *_WebMainFrameTitleKey;
extern NSString *_WebMainFrameURLKey;

@interface WebViewPrivate : NSObject
{
@public
    WebFrame *mainFrame;
    
    id UIDelegate;
    id UIDelegateForwarder;
    id resourceProgressDelegate;
    id resourceProgressDelegateForwarder;
    id downloadDelegate;
    id policyDelegate;
    id policyDelegateForwarder;
    id frameLoadDelegate;
    id frameLoadDelegateForwarder;
    id <WebFormDelegate> formDelegate;
    id editingDelegate;
    id editingDelegateForwarder;
    
    WebBackForwardList *backForwardList;
    BOOL useBackForwardList;
    
    float textSizeMultiplier;

    NSString *applicationNameForUserAgent;
    NSString *userAgent;
    BOOL userAgentOverridden;
    
    BOOL defersCallbacks;

    NSString *setName;

    WebPreferences *preferences;
    WebCoreSettings *settings;
        
    BOOL lastElementWasNonNil;

    NSWindow *hostWindow;

    int programmaticFocusCount;
    
    WebResourceDelegateImplementationCache resourceLoadDelegateImplementations;

    long long totalPageAndResourceBytesToLoad;
    long long totalBytesReceived;
    double progressValue;
    double lastNotifiedProgressValue;
    double lastNotifiedProgressTime;
    double progressNotificationInterval;
    double progressNotificationTimeInterval;
    BOOL finalProgressChangedSent;
    WebFrame *orginatingProgressFrame;
    
    int numProgressTrackedFrames;
    NSMutableDictionary *progressItems;
    
    void *observationInfo;
    
    NSArray *draggedTypes;
    
    BOOL drawsBackground;
}
@end

@interface WebView (WebPendingPublic)

- (void)setMainFrameURL:(NSString *)URLString;
- (NSString *)mainFrameURL;
- (BOOL)isLoading;
- (NSString *)mainFrameTitle;
- (NSImage *)mainFrameIcon;

- (void)setDrawsBackground:(BOOL)drawsBackround;
- (BOOL)drawsBackground;

@end

@interface WebView (WebPrivate)

+ (BOOL)_viewClass:(Class *)vClass andRepresentationClass:(Class *)rClass forMIMEType:(NSString *)MIMEType;

+ (NSArray *)_supportedFileExtensions;

/*!
    @method canShowFile:
    @abstract Checks if the WebKit can show the content of the file at the specified path.
    @param path The path of the file to check
    @result YES if the WebKit can show the content of the file at the specified path.
*/
+ (BOOL)canShowFile:(NSString *)path;

/*!
    @method suggestedFileExtensionForMIMEType:
    @param MIMEType The MIME type to check.
    @result The extension based on the MIME type
*/
+ (NSString *)suggestedFileExtensionForMIMEType: (NSString *)MIMEType;

/*!
    @method frameForDataSource:
    @abstract Return the frame associated with the data source.
    @discussion Traverses the frame tree to find the frame associated
    with a datasource.
    @param datasource The datasource to  match against each frame.
    @result The frame that has the associated datasource.
*/
- (WebFrame *)_frameForDataSource: (WebDataSource *)dataSource;

    /*!
            @method frameForView:
     @abstract Return the frame associated with the view.
     @discussion Traverses the frame tree to find the view.
     @param aView The view to match against each frame.
     @result The frame that has the associated view.
     */
- (WebFrame *)_frameForView: (WebFrameView *)aView;

- (WebFrame *)_createFrameNamed:(NSString *)name inParent:(WebFrame *)parent allowsScrolling:(BOOL)allowsScrolling;

- (void)_finishedLoadingResourceFromDataSource:(WebDataSource *)dataSource;
- (void)_receivedError:(NSError *)error fromDataSource:(WebDataSource *)dataSource;
- (void)_mainReceivedBytesSoFar:(unsigned)bytesSoFar fromDataSource:(WebDataSource *)dataSource complete:(BOOL)isComplete;
- (void)_mainReceivedError:(NSError *)error fromDataSource:(WebDataSource *)dataSource complete:(BOOL)isComplete;
+ (NSString *)_MIMETypeForFile:(NSString *)path;
- (void)_downloadURL:(NSURL *)URL;
- (void)_downloadURL:(NSURL *)URL toDirectory:(NSString *)directoryPath;

- (BOOL)defersCallbacks;
- (void)setDefersCallbacks:(BOOL)defers;

- (void)_setTopLevelFrameName:(NSString *)name;
- (WebFrame *)_findFrameInThisWindowNamed: (NSString *)name;
- (WebFrame *)_findFrameNamed: (NSString *)name;

- (WebView *)_openNewWindowWithRequest:(NSURLRequest *)request;

- (NSMenu *)_menuForElement:(NSDictionary *)element;

- (void)_mouseDidMoveOverElement:(NSDictionary *)dictionary modifierFlags:(unsigned)modifierFlags;

/*!
Could be worth adding to the API.
    @method loadItem:
    @abstract Loads the view with the contents described by the item, including frame content
        described by child items.
    @param item   The item to load.  It is not retained, but a copy will appear in the
        BackForwardList on this WebView.
*/
- (void)_loadItem:(WebHistoryItem *)item;
/*!
Could be worth adding to the API.
    @method loadItemsFromOtherView:
    @abstract Loads the view with the contents of the other view, including its backforward list.
    @param otherView   The WebView from which to copy contents.
*/
- (void)_loadBackForwardListFromOtherView:(WebView *)otherView;

- (void)_goToItem:(WebHistoryItem *)item withLoadType:(WebFrameLoadType)type;

// May well become public
- (void)_setFormDelegate:(id<WebFormDelegate>)delegate;
- (id<WebFormDelegate>)_formDelegate;

- (WebCoreSettings *)_settings;
- (void)_updateWebCoreSettingsFromPreferences:(WebPreferences *)prefs;

- (id)_frameLoadDelegateForwarder;
- (id)_resourceLoadDelegateForwarder;
- (void)_cacheResourceLoadDelegateImplementations;
- (WebResourceDelegateImplementationCache)_resourceLoadDelegateImplementations;
- (id)_policyDelegateForwarder;
- (id)_UIDelegateForwarder;
- (id)_editingDelegateForwarder;

- (void)_closeWindow;

- (void)_registerDraggedTypes;

- (void)_close;

/*!
    @method _registerViewClass:representationClass:forURLScheme:
    @discussion Register classes that implement WebDocumentView and WebDocumentRepresentation respectively.
    @param viewClass The WebDocumentView class to use to render data for a given MIME type.
    @param representationClass The WebDocumentRepresentation class to use to represent data of the given MIME type.
    @param scheme The URL scheme to represent with an object of the given class.
*/
+ (void)_registerViewClass:(Class)viewClass representationClass:(Class)representationClass forURLScheme:(NSString *)URLScheme;

+ (void)_unregisterViewClassAndRepresentationClassForMIMEType:(NSString *)MIMEType;

+ (NSString *)_generatedMIMETypeForURLScheme:(NSString *)URLScheme;
+ (BOOL)_representationExistsForURLScheme:(NSString *)URLScheme;
/*!
     @method _canHandleRequest:
     @abstract Performs a "preflight" operation that performs some
     speculative checks to see if a request can be used to create
     a WebDocumentView and WebDocumentRepresentation.
     @discussion The result of this method is valid only as long as no
     protocols or schemes are registered or unregistered, and as long as
     the request is not mutated (if the request is mutable). Hence, clients
     should be prepared to handle failures even if they have performed request
     preflighting by caling this method.
     @param request The request to preflight.
     @result YES if it is likely that a WebDocumentView and WebDocumentRepresentation
     can be created for the request, NO otherwise.
*/
+ (BOOL)_canHandleRequest:(NSURLRequest *)request;

+ (NSString *)_decodeData:(NSData *)data;

- (void)_pushPerformingProgrammaticFocus;
- (void)_popPerformingProgrammaticFocus;
- (BOOL)_isPerformingProgrammaticFocus;

// Methods dealing with the estimated progress completion.
- (void)_progressStarted:(WebFrame *)frame;
- (void)_progressCompleted:(WebFrame *)frame;
- (void)_incrementProgressForConnectionDelegate:(id)connectionDelegate response:(NSURLResponse *)response;
- (void)_incrementProgressForConnectionDelegate:(id)connectionDelegate data:(NSData *)dataSource;
- (void)_completeProgressForConnectionDelegate:(id)connectionDelegate;

- (void)_didStartProvisionalLoadForFrame:(WebFrame *)frame;
- (void)_didCommitLoadForFrame:(WebFrame *)frame;
- (void)_didFinishLoadForFrame:(WebFrame *)frame;
- (void)_didFailLoadWithError:(NSError *)error forFrame:(WebFrame *)frame;
- (void)_didFailProvisionalLoadWithError:(NSError *)error forFrame:(WebFrame *)frame;

- (void)_willChangeValueForKey:(NSString *)key;
- (void)_didChangeValueForKey:(NSString *)key;

- (void)_reloadForPluginChanges;
+ (void)_setAlwaysUseATSU:(BOOL)f;

- (NSCachedURLResponse *)_cachedResponseForURL:(NSURL *)URL;

@end

@interface WebView (WebViewPrintingPrivate)
/*!
    @method _adjustPrintingMarginsForHeaderAndFooter:
    @abstract Increase the top and bottom margins for the current print operation to
    account for the header and footer height. 
    @discussion Called by <WebDocument> implementors once when a print job begins. If the
    <WebDocument> implementor implements knowsPageRange:, this should be called from there.
    Otherwise this should be called from beginDocument. The <WebDocument> implementors need
    to also call _drawHeaderAndFooter.
*/
- (void)_adjustPrintingMarginsForHeaderAndFooter;

/*!
    @method _drawHeaderAndFooter
    @abstract Gives the WebView's UIDelegate a chance to draw a header and footer on the
    printed page. 
    @discussion This should be called by <WebDocument> implementors from an override of
    drawPageBorderWithSize:.
*/
- (void)_drawHeaderAndFooter;
@end

@interface _WebSafeForwarder : NSObject
{
    id target;	// Non-retained.  Don't retain delegates;
    id defaultTarget;
    Class templateClass;
}
- (id)initWithTarget:(id)t defaultTarget:(id)dt templateClass:(Class)aClass;
+ (id)safeForwarderWithTarget:(id)t defaultTarget:(id)dt templateClass:(Class)aClass;
@end



/* ------------------------------------------------------------------*/

typedef enum
{
	WebViewInsertActionTyped,	
	WebViewInsertActionPasted,	
	WebViewInsertActionDropped,	
} WebViewInsertAction;

extern NSString * const WebViewDidBeginEditingNotification;
extern NSString * const WebViewDidChangeNotification;
extern NSString * const WebViewDidEndEditingNotification;
extern NSString * const WebViewDidChangeTypingStyleNotification;
extern NSString * const WebViewDidChangeSelectionNotification;

@interface NSObject (WebViewEditingDelegate)
- (BOOL)webViewShouldBeginEditing:(WebView *)webView;
- (BOOL)webViewShouldEndEditing:(WebView *)webView;
- (BOOL)webView:(WebView *)webView shouldInsertNode:(DOMNode *)node replacingDOMRange:(DOMRange *)range givenAction:(WebViewInsertAction)action;
- (BOOL)webView:(WebView *)webView shouldInsertText:(NSString *)text replacingDOMRange:(DOMRange *)range givenAction:(WebViewInsertAction)action;
- (BOOL)webView:(WebView *)webView shouldDeleteDOMRange:(DOMRange *)range;
- (BOOL)webView:(WebView *)webView shouldChangeSelectedDOMRange:(DOMRange *)currentRange toDOMRange:(DOMRange *)proposedRange;
- (BOOL)webView:(WebView *)webView shouldApplyStyle:(DOMCSSStyleDeclaration *)style toElementsInDOMRange:(DOMRange *)range;
- (BOOL)webView:(WebView *)webView shouldChangeTypingStyle:(DOMCSSStyleDeclaration *)currentStyle toStyle:(DOMCSSStyleDeclaration *)proposedStyle;
- (BOOL)webView:(WebView *)webView doCommandBySelector:(SEL)selector;
- (void)webViewDidBeginEditing:(NSNotification *)notification;
- (void)webViewDidChange:(NSNotification *)notification;
- (void)webViewDidEndEditing:(NSNotification *)notification;
- (void)webViewDidChangeTypingStyle:(NSNotification *)notification;
- (void)webViewDidChangeSelection:(NSNotification *)notification;
- (NSUndoManager *)undoManagerForWebView:(WebView *)webView;
@end

@interface WebView (WebEditingExtras)
- (WebBridge *)_bridgeForCurrentSelection;
- (void)setSelectedDOMRange:(DOMRange *)range;
- (DOMRange *)selectedDOMRange;
- (void)setEditingDelegate:(id)delegate;
- (id)editingDelegate;
- (void)editingKeyDown:(NSEvent *)event;
@end


@interface WebView (WebViewUndoableEditing)
- (void)insertText:(NSString *)text replacingDOMRange:(DOMRange *)range;    
@end
