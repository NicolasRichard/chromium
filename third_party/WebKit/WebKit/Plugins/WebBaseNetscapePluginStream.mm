/*
 * Copyright (C) 2005, 2006, 2007 Apple Inc. All rights reserved.
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

#import "WebBaseNetscapePluginStream.h"

#import "WebBaseNetscapePluginView.h"
#import "WebKitErrorsPrivate.h"
#import "WebKitLogging.h"
#import "WebNSObjectExtras.h"
#import "WebNSURLExtras.h"
#import "WebNetscapePluginPackage.h"
#import <Foundation/NSURLResponse.h>
#import <WebCore/WebCoreObjCExtras.h>
#import <WebKitSystemInterface.h>
#import <wtf/HashMap.h>

#define WEB_REASON_NONE -1

static char *CarbonPathFromPOSIXPath(const char *posixPath);

typedef HashMap<NPStream*, NPP> StreamMap;
static StreamMap& streams()
{
    static StreamMap staticStreams;
    return staticStreams;
}

@implementation WebBaseNetscapePluginStream

#ifndef BUILDING_ON_TIGER
+ (void)initialize
{
    WebCoreObjCFinalizeOnMainThread(self);
}
#endif

+ (NPP)ownerForStream:(NPStream *)stream
{
    return streams().get(stream);
}

+ (NPReason)reasonForError:(NSError *)error
{
    if (error == nil) {
        return NPRES_DONE;
    }
    if ([[error domain] isEqualToString:NSURLErrorDomain] && [error code] == NSURLErrorCancelled) {
        return NPRES_USER_BREAK;
    }
    return NPRES_NETWORK_ERR;
}

- (NSError *)_pluginCancelledConnectionError
{
    return [[[NSError alloc] _initWithPluginErrorCode:WebKitErrorPlugInCancelledConnection
                                           contentURL:responseURL != nil ? responseURL : requestURL
                                        pluginPageURL:nil
                                           pluginName:[[pluginView pluginPackage] name]
                                             MIMEType:MIMEType] autorelease];
}

- (NSError *)errorForReason:(NPReason)theReason
{
    if (theReason == NPRES_DONE) {
        return nil;
    }
    if (theReason == NPRES_USER_BREAK) {
        return [NSError _webKitErrorWithDomain:NSURLErrorDomain
                                          code:NSURLErrorCancelled 
                                           URL:responseURL != nil ? responseURL : requestURL];
    }
    return [self _pluginCancelledConnectionError];
}

- (id)initWithRequestURL:(NSURL *)theRequestURL
                  plugin:(NPP)thePlugin
              notifyData:(void *)theNotifyData
        sendNotification:(BOOL)flag
{
    [super init];
 
    // Temporarily set isTerminated to YES to avoid assertion failure in dealloc in case we are released in this method.
    isTerminated = YES;

    if (theRequestURL == nil || thePlugin == NULL) {
        [self release];
        return nil;
    }
    
    [self setRequestURL:theRequestURL];
    [self setPlugin:thePlugin];
    notifyData = theNotifyData;
    sendNotification = flag;

    streams().add(&stream, thePlugin);
    
    isTerminated = NO;
    
    return self;
}

- (void)dealloc
{
    ASSERT(!plugin);
    ASSERT(isTerminated);
    ASSERT(stream.ndata == nil);

    // The stream file should have been deleted, and the path freed, in -_destroyStream
    ASSERT(!path);

    [requestURL release];
    [responseURL release];
    [MIMEType release];
    [pluginView release];
    [deliveryData release];
    
    free((void *)stream.url);
    free(path);

    streams().remove(&stream);

    [super dealloc];
}

- (void)finalize
{
    ASSERT_MAIN_THREAD();
    ASSERT(isTerminated);
    ASSERT(stream.ndata == nil);

    // The stream file should have been deleted, and the path freed, in -_destroyStream
    ASSERT(!path);

    free((void *)stream.url);
    free(path);

    streams().remove(&stream);

    [super finalize];
}

- (uint16)transferMode
{
    return transferMode;
}

- (NPP)plugin
{
    return plugin;
}

- (void)setRequestURL:(NSURL *)theRequestURL
{
    [theRequestURL retain];
    [requestURL release];
    requestURL = theRequestURL;
}

- (void)setResponseURL:(NSURL *)theResponseURL
{
    [theResponseURL retain];
    [responseURL release];
    responseURL = theResponseURL;
}

- (void)setPlugin:(NPP)thePlugin
{
    if (thePlugin) {
        plugin = thePlugin;
        pluginView = [(WebBaseNetscapePluginView *)plugin->ndata retain];
        WebNetscapePluginPackage *pluginPackage = [pluginView pluginPackage];
        NPP_NewStream = [pluginPackage NPP_NewStream];
        NPP_WriteReady = [pluginPackage NPP_WriteReady];
        NPP_Write = [pluginPackage NPP_Write];
        NPP_StreamAsFile = [pluginPackage NPP_StreamAsFile];
        NPP_DestroyStream = [pluginPackage NPP_DestroyStream];
        NPP_URLNotify = [pluginPackage NPP_URLNotify];
    } else {
        WebBaseNetscapePluginView *view = pluginView;

        plugin = NULL;
        NPP_NewStream = NULL;
        NPP_WriteReady = NULL;
        NPP_Write = NULL;
        NPP_StreamAsFile = NULL;
        NPP_DestroyStream = NULL;
        NPP_URLNotify = NULL;
        pluginView = nil;

        [view disconnectStream:self];
        [view release];
    }
}

- (void)setMIMEType:(NSString *)theMIMEType
{
    [theMIMEType retain];
    [MIMEType release];
    MIMEType = theMIMEType;
}

- (void)startStreamResponseURL:(NSURL *)URL
         expectedContentLength:(long long)expectedContentLength
              lastModifiedDate:(NSDate *)lastModifiedDate
                      MIMEType:(NSString *)theMIMEType
{
    ASSERT(!isTerminated);
    
    [self setResponseURL:URL];
    [self setMIMEType:theMIMEType];
    
    free((void *)stream.url);
    stream.url = strdup([responseURL _web_URLCString]);

    stream.ndata = self;
    stream.end = expectedContentLength > 0 ? (uint32)expectedContentLength : 0;
    stream.lastmodified = (uint32)[lastModifiedDate timeIntervalSince1970];
    stream.notifyData = notifyData;
    
    transferMode = NP_NORMAL;
    offset = 0;
    reason = WEB_REASON_NONE;

    // FIXME: Need a way to check if stream is seekable

    WebBaseNetscapePluginView *pv = pluginView;
    [pv willCallPlugInFunction];
    NPError npErr = NPP_NewStream(plugin, (char *)[MIMEType UTF8String], &stream, NO, &transferMode);
    [pv didCallPlugInFunction];
    LOG(Plugins, "NPP_NewStream URL=%@ MIME=%@ error=%d", responseURL, MIMEType, npErr);

    if (npErr != NPERR_NO_ERROR) {
        LOG_ERROR("NPP_NewStream failed with error: %d responseURL: %@", npErr, responseURL);
        // Calling cancelLoadWithError: cancels the load, but doesn't call NPP_DestroyStream.
        [self cancelLoadWithError:[self _pluginCancelledConnectionError]];
        return;
    }

    switch (transferMode) {
        case NP_NORMAL:
            LOG(Plugins, "Stream type: NP_NORMAL");
            break;
        case NP_ASFILEONLY:
            LOG(Plugins, "Stream type: NP_ASFILEONLY");
            break;
        case NP_ASFILE:
            LOG(Plugins, "Stream type: NP_ASFILE");
            break;
        case NP_SEEK:
            LOG_ERROR("Stream type: NP_SEEK not yet supported");
            [self cancelLoadAndDestroyStreamWithError:[self _pluginCancelledConnectionError]];
            break;
        default:
            LOG_ERROR("unknown stream type");
    }
}

- (void)startStreamWithResponse:(NSURLResponse *)r
{
    [self startStreamResponseURL:[r URL]
           expectedContentLength:[r expectedContentLength]
                lastModifiedDate:WKGetNSURLResponseLastModifiedDate(r)
                        MIMEType:[r MIMEType]];
}

- (void)_destroyStream
{
    if (isTerminated)
        return;

    [self retain];

    ASSERT(reason != WEB_REASON_NONE);
    ASSERT([deliveryData length] == 0);
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(_deliverData) object:nil];

    if (stream.ndata != nil) {
        if (reason == NPRES_DONE && (transferMode == NP_ASFILE || transferMode == NP_ASFILEONLY)) {
            ASSERT(path != NULL);
            char *carbonPath = CarbonPathFromPOSIXPath(path);
            ASSERT(carbonPath != NULL);
            WebBaseNetscapePluginView *pv = pluginView;
            [pv willCallPlugInFunction];
            NPP_StreamAsFile(plugin, &stream, carbonPath);
            [pv didCallPlugInFunction];

            // Delete the file after calling NPP_StreamAsFile(), instead of in -dealloc/-finalize.  It should be OK
            // to delete the file here -- NPP_StreamAsFile() is always called immediately before NPP_DestroyStream()
            // (the stream destruction function), so there can be no expectation that a plugin will read the stream
            // file asynchronously after NPP_StreamAsFile() is called.
            unlink(path);
            free(path);
            path = NULL;
            LOG(Plugins, "NPP_StreamAsFile responseURL=%@ path=%s", responseURL, carbonPath);
            free(carbonPath);

            if (isTerminated)
                goto exit;
        }

        NPError npErr;
        WebBaseNetscapePluginView *pv = pluginView;
        [pv willCallPlugInFunction];
        npErr = NPP_DestroyStream(plugin, &stream, reason);
        [pv didCallPlugInFunction];
        LOG(Plugins, "NPP_DestroyStream responseURL=%@ error=%d", responseURL, npErr);

        stream.ndata = nil;

        if (isTerminated)
            goto exit;
    }

    if (sendNotification) {
        // NPP_URLNotify expects the request URL, not the response URL.
        WebBaseNetscapePluginView *pv = pluginView;
        [pv willCallPlugInFunction];
        NPP_URLNotify(plugin, [requestURL _web_URLCString], reason, notifyData);
        [pv didCallPlugInFunction];
        LOG(Plugins, "NPP_URLNotify requestURL=%@ reason=%d", requestURL, reason);
    }

    isTerminated = YES;

    [self setPlugin:NULL];

exit:
    [self release];
}

- (void)_destroyStreamWithReason:(NPReason)theReason
{
    reason = theReason;
    if (reason != NPRES_DONE) {
        // Stop any pending data from being streamed.
        [deliveryData setLength:0];
    } else if ([deliveryData length] > 0) {
        // There is more data to be streamed, don't destroy the stream now.
        return;
    }
    [self _destroyStream];
    ASSERT(stream.ndata == nil);
}

- (void)cancelLoadWithError:(NSError *)error
{
    // Overridden by subclasses.
    ASSERT_NOT_REACHED();
}

- (void)destroyStreamWithError:(NSError *)error
{
    [self _destroyStreamWithReason:[[self class] reasonForError:error]];
}

- (void)cancelLoadAndDestroyStreamWithError:(NSError *)error
{
    [self retain];
    [self cancelLoadWithError:error];
    [self destroyStreamWithError:error];
    [self setPlugin:NULL];
    [self release];
}

- (void)finishedLoadingWithData:(NSData *)data
{
    if (!stream.ndata)
        return;
    
    if ((transferMode == NP_ASFILE || transferMode == NP_ASFILEONLY) && !path) {
        path = strdup("/tmp/WebKitPlugInStreamXXXXXX");
        int fd = mkstemp(path);
        if (fd == -1) {
            // This should almost never happen.
            LOG_ERROR("can't make temporary file, almost certainly a problem with /tmp");
            // This is not a network error, but the only error codes are "network error" and "user break".
            [self _destroyStreamWithReason:NPRES_NETWORK_ERR];
            free(path);
            path = NULL;
            return;
        }
        int dataLength = [data length];
        if (dataLength > 0) {
            int byteCount = write(fd, [data bytes], dataLength);
            if (byteCount != dataLength) {
                // This happens only rarely, when we are out of disk space or have a disk I/O error.
                LOG_ERROR("error writing to temporary file, errno %d", errno);
                close(fd);
                // This is not a network error, but the only error codes are "network error" and "user break".
                [self _destroyStreamWithReason:NPRES_NETWORK_ERR];
                free(path);
                path = NULL;
                return;
            }
        }
        close(fd);
    }

    [self _destroyStreamWithReason:NPRES_DONE];
}

- (void)_deliverData
{
    if (!stream.ndata || [deliveryData length] == 0)
        return;

    [self retain];

    int32 totalBytes = [deliveryData length];
    int32 totalBytesDelivered = 0;

    while (totalBytesDelivered < totalBytes) {
        WebBaseNetscapePluginView *pv = pluginView;
        [pv willCallPlugInFunction];
        int32 deliveryBytes = NPP_WriteReady(plugin, &stream);
        [pv didCallPlugInFunction];
        LOG(Plugins, "NPP_WriteReady responseURL=%@ bytes=%d", responseURL, deliveryBytes);

        if (isTerminated)
            goto exit;

        if (deliveryBytes <= 0) {
            // Plug-in can't receive anymore data right now. Send it later.
            [self performSelector:@selector(_deliverData) withObject:nil afterDelay:0];
            break;
        } else {
            deliveryBytes = MIN(deliveryBytes, totalBytes - totalBytesDelivered);
            NSData *subdata = [deliveryData subdataWithRange:NSMakeRange(totalBytesDelivered, deliveryBytes)];
            pv = pluginView;
            [pv willCallPlugInFunction];
            deliveryBytes = NPP_Write(plugin, &stream, offset, [subdata length], (void *)[subdata bytes]);
            [pv didCallPlugInFunction];
            if (deliveryBytes < 0) {
                // Netscape documentation says that a negative result from NPP_Write means cancel the load.
                [self cancelLoadAndDestroyStreamWithError:[self _pluginCancelledConnectionError]];
                return;
            }
            deliveryBytes = MIN((unsigned)deliveryBytes, [subdata length]);
            offset += deliveryBytes;
            totalBytesDelivered += deliveryBytes;
            LOG(Plugins, "NPP_Write responseURL=%@ bytes=%d total-delivered=%d/%d", responseURL, deliveryBytes, offset, stream.end);
        }
    }

    if (totalBytesDelivered > 0) {
        if (totalBytesDelivered < totalBytes) {
            NSMutableData *newDeliveryData = [[NSMutableData alloc] initWithCapacity:totalBytes - totalBytesDelivered];
            [newDeliveryData appendBytes:(char *)[deliveryData bytes] + totalBytesDelivered length:totalBytes - totalBytesDelivered];
            [deliveryData release];
            deliveryData = newDeliveryData;
        } else {
            [deliveryData setLength:0];
            if (reason != WEB_REASON_NONE) {
                [self _destroyStream];
            }
        }
    }

exit:
    [self release];
}

- (void)receivedData:(NSData *)data
{
    ASSERT([data length] > 0);
    
    if (transferMode != NP_ASFILEONLY) {
        if (!deliveryData) {
            deliveryData = [[NSMutableData alloc] initWithCapacity:[data length]];
        }
        [deliveryData appendData:data];
        [self _deliverData];
    }
}

@end

static char *CarbonPathFromPOSIXPath(const char *posixPath)
{
    // Doesn't add a trailing colon for directories; this is a problem for paths to a volume,
    // so this function would need to be revised if we ever wanted to call it with that.

    CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *)posixPath, strlen(posixPath), false);
    if (url) {
        CFStringRef hfsPath = CFURLCopyFileSystemPath(url, kCFURLHFSPathStyle);
        CFRelease(url);
        if (hfsPath) {
            CFIndex bufSize = CFStringGetMaximumSizeOfFileSystemRepresentation(hfsPath);
            char* filename = static_cast<char*>(malloc(bufSize));
            CFStringGetFileSystemRepresentation(hfsPath, filename, bufSize);
            CFRelease(hfsPath);
            return filename;
        }
    }

    return NULL;
}
