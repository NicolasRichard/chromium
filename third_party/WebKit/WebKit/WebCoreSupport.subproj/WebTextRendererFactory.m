/*	
    WebTextRendererFactory.m
    Copyright 2002, Apple, Inc. All rights reserved.
*/

#import <WebKit/WebKitLogging.h>
#import <WebKit/WebTextRendererFactory.h>
#import <WebKit/WebTextRenderer.h>

#import <WebFoundation/WebAssertions.h>
#import <WebFoundation/WebSystemBits.h>

#import <CoreGraphics/CoreGraphicsPrivate.h>
#import <CoreGraphics/CGFontLCDSupport.h>

#import <mach-o/dyld.h>

@interface NSFont (WebAppKitSecretAPI)
- (BOOL)_isFakeFixedPitch;
@end

@interface WebFontCacheKey : NSObject
{
    NSString *family;
    NSFontTraitMask traits;
    float size;
}

- initWithFamily:(NSString *)f traits:(NSFontTraitMask)t size:(float)s;

@end

@implementation WebFontCacheKey

- initWithFamily:(NSString *)f traits:(NSFontTraitMask)t size:(float)s;
{
    [super init];
    family = [f copy];
    traits = t;
    size = s;
    return self;
}

- (void)dealloc
{
    [family release];
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
    return [self retain];
}

- (unsigned)hash
{
    return [family hash] ^ traits ^ (int)size;
}

- (BOOL)isEqual:(id)o
{
    WebFontCacheKey *other = o;
    return [self class] == [other class]
        && [family isEqualToString:other->family]
        && traits == other->traits
        && size == other->size;
}

@end

@implementation WebTextRendererFactory

- (BOOL)coalesceTextDrawing
{
    return [viewStack objectAtIndex: [viewStack count]-1] == [NSView focusView] ? YES : NO;
}

- (void)startCoalesceTextDrawing
{
    if (!viewStack)
        viewStack = [[NSMutableArray alloc] init];
    if (!viewBuffers)
        viewBuffers = [[NSMutableDictionary alloc] init];
    [viewStack addObject: [NSView focusView]];
}

- (void)endCoalesceTextDrawing
{
    ASSERT([self coalesceTextDrawing]);
    
    NSView *targetView = [viewStack objectAtIndex: [viewStack count]-1];
    [viewStack removeLastObject];
    NSValue *viewKey = [NSValue valueWithNonretainedObject: targetView];
    NSMutableSet *glyphBuffers = [viewBuffers objectForKey:viewKey];

    [glyphBuffers makeObjectsPerformSelector: @selector(drawInView:) withObject: targetView];
    [glyphBuffers makeObjectsPerformSelector: @selector(reset)];
    [viewBuffers removeObjectForKey: viewKey];
}

- (WebGlyphBuffer *)glyphBufferForFont: (NSFont *)font andColor: (NSColor *)color
{
    ASSERT([self coalesceTextDrawing]);

    NSMutableSet *glyphBuffers;
    WebGlyphBuffer *glyphBuffer = nil;
    NSValue *viewKey = [NSValue valueWithNonretainedObject: [NSView focusView]];
    
    glyphBuffers = [viewBuffers objectForKey:viewKey];
    if (glyphBuffers == nil){
        glyphBuffers = [[NSMutableSet alloc] init];
        [viewBuffers setObject: glyphBuffers forKey: viewKey];
        [glyphBuffers release];
    }
    
    NSEnumerator *enumerator = [glyphBuffers objectEnumerator];
    id value;
    
    // Could use a dictionary w/ font/color key for faster lookup.
    while ((value = [enumerator nextObject])) {
        if ([value font] == font && [[value color] isEqual: color])
            glyphBuffer = value;
    }
    if (glyphBuffer == nil){
        glyphBuffer = [[WebGlyphBuffer alloc] initWithFont: font color: color];
        [glyphBuffers addObject: glyphBuffer];
        [glyphBuffer release];
    }
        
    return glyphBuffer;
}

static bool
getAppDefaultValue(CFStringRef key, int *v)
{
    CFPropertyListRef value;

    value = CFPreferencesCopyValue(key, kCFPreferencesCurrentApplication,
                                   kCFPreferencesAnyUser,
                                   kCFPreferencesAnyHost);
    if (value == NULL) {
        value = CFPreferencesCopyValue(key, kCFPreferencesCurrentApplication,
                                       kCFPreferencesCurrentUser,
                                       kCFPreferencesAnyHost);
        if (value == NULL)
            return false;
    }

    if (CFGetTypeID(value) == CFNumberGetTypeID()) {
        if (v != NULL)
            CFNumberGetValue(value, kCFNumberIntType, v);
    } else if (CFGetTypeID(value) == CFStringGetTypeID()) {
        if (v != NULL)
            *v = CFStringGetIntValue(value);
    } else {
        CFRelease(value);
        return false;
    }

    CFRelease(value);
    return true;
}

static bool
getUserDefaultValue(CFStringRef key, int *v)
{
    CFPropertyListRef value;

    value = CFPreferencesCopyValue(key, kCFPreferencesAnyApplication,
                                   kCFPreferencesCurrentUser,
                                   kCFPreferencesCurrentHost);
    if (value == NULL)
        return false;

    if (CFGetTypeID(value) == CFNumberGetTypeID()) {
        if (v != NULL)
            CFNumberGetValue(value, kCFNumberIntType, v);
    } else if (CFGetTypeID(value) == CFStringGetTypeID()) {
        if (v != NULL)
            *v = CFStringGetIntValue(value);
    } else {
        CFRelease(value);
        return false;
    }

    CFRelease(value);
    return true;
}

static int getLCDScaleParameters(void)
{
    int mode;
    CFStringRef key;

    key = CFSTR("AppleFontSmoothing");
    if (!getAppDefaultValue(key, &mode)) {
        if (!getUserDefaultValue(key, &mode))
            return 1;
    }

    switch (mode) {
        case kCGFontSmoothingLCDLight:
        case kCGFontSmoothingLCDMedium:
        case kCGFontSmoothingLCDStrong:
            return 4;
        default:
            return 1;
    }

}

#define MINIMUM_GLYPH_CACHE_SIZE 1536 * 1024

+ (void)createSharedFactory;
{
    if (![self sharedFactory]) {
        [[[self alloc] init] release];

        // Turn off auto expiration of glyphs in CG's cache
        // and increase the cache size.
        NSSymbol symbol1 = NULL, symbol2 = NULL;
        if (NSIsSymbolNameDefined ("_CGFontCacheSetShouldAutoExpire")){
            symbol1 = NSLookupAndBindSymbol("_CGFontCacheSetShouldAutoExpire");
            symbol2 = NSLookupAndBindSymbol("_CGFontCacheSetMaxSize");
            if (symbol1 != NULL && symbol2 != NULL) {
                void (*functionPtr1)(CGFontCache *,bool) = NSAddressOfSymbol(symbol1);
                void (*functionPtr2)(CGFontCache *,size_t) = NSAddressOfSymbol(symbol2);
        
                CGFontCache *fontCache;
                fontCache = CGFontCacheCreate();
                functionPtr1 (fontCache, false);

                size_t s;
                if (WebSystemMainMemory() > 128 * 1024 * 1024)
                    s = MINIMUM_GLYPH_CACHE_SIZE*getLCDScaleParameters();
                else
                    s = MINIMUM_GLYPH_CACHE_SIZE;
#ifndef NDEBUG
                LOG (CacheSizes, "Glyph cache size set to %d bytes.", s);
#endif
                functionPtr2 (fontCache, s);
                CGFontCacheRelease(fontCache);
            }
        }

        if (symbol1 == NULL || symbol2 == NULL)
            NSLog(@"CoreGraphics is missing call to disable glyph auto expiration. Pages will load more slowly.");
    }
    ASSERT([[self sharedFactory] isKindOfClass:self]);
}

+ (WebTextRendererFactory *)sharedFactory;
{
    return (WebTextRendererFactory *)[super sharedFactory];
}

- (BOOL)isFontFixedPitch: (NSFont *)font
{
    return [font isFixedPitch] || [font _isFakeFixedPitch];
}

- init
{
    [super init];
    
    cache = [[NSMutableDictionary alloc] init];
    
    return self;
}

- (void)dealloc
{
    [cache release];
    [viewBuffers release];
    [viewStack release];
    
    [super dealloc];
}

- (WebTextRenderer *)rendererWithFont:(NSFont *)font
{
    WebTextRenderer *renderer = [cache objectForKey:font];
    if (renderer == nil) {
        renderer = [[WebTextRenderer alloc] initWithFont:font];
        [cache setObject:renderer forKey:font];
        [renderer release];
    }
    return renderer;
}

- (NSFont *)fallbackFontWithTraits:(NSFontTraitMask)traits size:(float)size 
{
    return [self cachedFontFromFamily:@"Helvetica" traits:traits size:size];
}

- (NSFont *)fontWithFamilies:(NSString **)families traits:(NSFontTraitMask)traits size:(float)size
{
    NSFont *font = nil;
    NSString *family;
    int i = 0;
    
    while (families && families[i] != 0 && font == nil){
        family = families[i++];
        if ([family length] != 0)
            font = [self cachedFontFromFamily: family traits:traits size:size];
    }
    if (font == nil)
        font = [self fallbackFontWithTraits:traits size:size];

    return font;
}

- (NSFont *)fontWithFamily:(NSString *)family traits:(NSFontTraitMask)traits size:(float)size
{
    NSFont *font;
    NSEnumerator *e;
    NSString *availableFamily;
    
    font = [[NSFontManager sharedFontManager] fontWithFamily:family traits:traits weight:5 size:size];
    if (font != nil) {
        return font;
    }
    
    // FIXME:  For now do a simple case insensitive search for a matching font.
    // The font manager requires exact name matches.  This will at least address the problem
    // of matching arial to Arial, etc.
    e = [[[NSFontManager sharedFontManager] availableFontFamilies] objectEnumerator];
    while ((availableFamily = [e nextObject])) {
        if ([family caseInsensitiveCompare:availableFamily] == NSOrderedSame) {
            NSArray *fonts = [[NSFontManager sharedFontManager] availableMembersOfFontFamily:availableFamily];
            NSArray *fontInfo;
            NSFontTraitMask fontMask;
            int fontWeight;
            unsigned i;
        
            for (i = 0; i < [fonts count]; i++){
                fontInfo = [fonts objectAtIndex: i];
                
                // Hard coded positions depend on lame AppKit API.
                fontWeight = [[fontInfo objectAtIndex: 2] intValue];
                fontMask = [[fontInfo objectAtIndex: 3] unsignedIntValue];
                
                // First look for a 'normal' weight font.  Note that the 
                // documentation indicates that the weight parameter is ignored if the 
                // trait contains the bold mask.  This is odd as one would think that other
                // traits could also indicate weight changes.  In fact, the weight parameter
                // and the trait mask together make a conflicted API.
                if (fontWeight == 5 && (fontMask & traits) == traits){
                    font = [[NSFontManager sharedFontManager] fontWithFamily:availableFamily traits:traits weight:5 size:size];
                    if (font != nil) {
                        return font;
                    }
                } 
                
                // Get a font with the correct traits but a weight we're told actually exists.
                if ((fontMask & traits) == traits){
                    font = [[NSFontManager sharedFontManager] fontWithFamily:availableFamily traits:traits weight:fontWeight size:size];
                    if (font != nil) {
                        return font;
                    }
                } 
            }
        }
    }
    
    return nil;
}

- (NSFont *)cachedFontFromFamily:(NSString *)family traits:(NSFontTraitMask)traits size:(float)size
{
    static NSMutableDictionary *fontCache = nil;
    static NSMutableSet *missingFonts = nil;
    WebFontCacheKey *fontKey;
    NSFont *font = nil;
    
    if ([family length] == 0)
        return nil;
    
    if (!fontCache) {
        fontCache = [[NSMutableDictionary alloc] init];
    }

    fontKey = [[WebFontCacheKey alloc] initWithFamily:family traits:traits size:size];
    font = [fontCache objectForKey:fontKey];
    if (!font){
        if (![missingFonts containsObject:fontKey]){
            font = [self fontWithFamily:family traits:traits size:size];
            if (font)
                [fontCache setObject:font forKey:fontKey];
            else{
                if (!missingFonts)
                    missingFonts = [[NSMutableSet alloc] init];
                [missingFonts addObject:fontKey];
            }
        }
    }
#ifdef DEBUG_MISSING_FONT
    static int unableToFindFontCount = 0;
    if (font == nil){
        unableToFindFontCount++;
        NSLog (@"unableToFindFontCount %@, traits 0x%08x, size %f, %d\n", family, traits, size, unableToFindFontCount);
    }
#endif
    [fontKey release];
    
    return font;
}

@end
