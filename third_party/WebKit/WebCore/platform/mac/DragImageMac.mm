/*
 * Copyright (C) 2007 Apple Inc.  All rights reserved.
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
#import "config.h"
#import "DragImage.h"

#import "CachedImage.h"
#import "Image.h"
#import "KURL.h"
#import "PlatformString.h"
#import "ResourceResponse.h"

namespace WebCore {



IntSize dragImageSize(DragImageRef image)
{
    return (IntSize)[image.get() size];
}

void deleteDragImage(DragImageRef image)
{
    //DragImageRef is a RetainPtr, so we don't need to explicitly delete it
}

DragImageRef scaleDragImage(DragImageRef image, FloatSize scale)
{
    NSSize originalSize = [image.get() size];
    NSSize newSize = NSMakeSize((originalSize.width * scale.width()), (originalSize.height * scale.height()));
    newSize.width = roundf(newSize.width);
    newSize.height = roundf(newSize.height);
    [image.get() setScalesWhenResized:YES];
    [image.get() setSize:newSize];
    return image;
}
    
DragImageRef dissolveDragImageToFraction(DragImageRef image, float delta)
{
    RetainPtr<NSImage> dissolvedImage(AdoptNS, [[NSImage alloc] initWithSize:[image.get() size]]);
    
    NSPoint point = [image.get() isFlipped] ? NSMakePoint(0, [image.get() size].height) : NSZeroPoint;
    
    // In this case the dragging image is always correct.
    [dissolvedImage.get() setFlipped:[image.get() isFlipped]];
    
    [dissolvedImage.get() lockFocus];
    [image.get() dissolveToPoint:point fraction: delta];
    [dissolvedImage.get() unlockFocus];
    
    [image.get() lockFocus];
    [dissolvedImage.get() compositeToPoint:point operation:NSCompositeCopy];
    [image.get() unlockFocus];
    
    return image;
}
        
DragImageRef createDragImageFromImage(Image* image)
{
    DragImageRef dragImage(AdoptNS, [image->getNSImage() copy]);
    [dragImage.get() setSize:(NSSize)(image->size())];
    return dragImage;
}
    
DragImageRef createDragImageIconForCachedImage(CachedImage* image)
{
    const String& filename = image->response().suggestedFilename();
    NSString *extension = nil;
    int dotIndex = filename.reverseFind('.');
    
    if (dotIndex > 0 && dotIndex < (int)(filename.length() - 1)) // require that a . exists after the first character and before the last
        extension = filename.substring(dotIndex + 1);
    else
        //It might be worth doing a further look up to pull the extension from the mimetype
        extension = @"";
    
    return DragImageRef([[NSWorkspace sharedWorkspace] iconForFileType:extension]);
     
}
    
}
