/*	
        WebBaseNetscapePluginStream.m
	Copyright (c) 2002, Apple, Inc. All rights reserved.
*/

#import <WebKit/WebBaseNetscapePluginStream.h>
#import <WebKit/WebBaseNetscapePluginView.h>
#import <WebKit/WebKitLogging.h>
#import <WebKit/WebNetscapePluginPackage.h>
#import <WebKit/WebNSURLExtras.h>

#import <Foundation/NSURLResponse.h>
#import <Foundation/NSURLResponsePrivate.h>
#import <Foundation/NSFileManager_NSURLExtras.h>
#import <Foundation/NSURL_NSURLExtras.h>

@implementation WebBaseNetscapePluginStream

- (void)dealloc
{
    ASSERT(stream.ndata == nil);

    // FIXME: It's generally considered bad style to do work, like deleting a file,
    // at dealloc time. We should change things around so that this is done at a
    // more well-defined time rather than when the last release happens.
    if (path) {
        unlink(path);
    }

    [URL release];
    free((void *)stream.URL);
    free(path);
    [plugin release];
    
    [super dealloc];
}

- (uint16)transferMode
{
    return transferMode;
}

- (void)setPluginPointer:(NPP)pluginPointer
{
    instance = pluginPointer;
    
    plugin = [[(WebBaseNetscapePluginView *)instance->ndata plugin] retain];

    NPP_NewStream = 	[plugin NPP_NewStream];
    NPP_WriteReady = 	[plugin NPP_WriteReady];
    NPP_Write = 	[plugin NPP_Write];
    NPP_StreamAsFile = 	[plugin NPP_StreamAsFile];
    NPP_DestroyStream = [plugin NPP_DestroyStream];
    NPP_URLNotify = 	[plugin NPP_URLNotify];
}

- (void)setNotifyData:(void *)theNotifyData
{
    notifyData = theNotifyData;
}

- (void)startStreamWithURL:(NSURL *)theURL 
     expectedContentLength:(long long)expectedContentLength
          lastModifiedDate:(NSDate *)lastModifiedDate
                  MIMEType:(NSString *)MIMEType
{
    if (![plugin isLoaded]) {
        return;
    }
    
    [theURL retain];
    [URL release];
    URL = theURL;

    free((void *)stream.URL);
    stream.URL = strdup([URL _web_URLCString]);

    stream.ndata = self;
    stream.end = expectedContentLength;
    stream.lastmodified = [lastModifiedDate timeIntervalSince1970];
    stream.notifyData = notifyData;
    
    transferMode = NP_NORMAL;
    offset = 0;

    // FIXME: Need a way to check if stream is seekable

    NPError npErr = NPP_NewStream(instance, (char *)[MIMEType cString], &stream, NO, &transferMode);
    LOG(Plugins, "NPP_NewStream: %d %@", npErr, URL);

    if (npErr != NPERR_NO_ERROR) {
        ERROR("NPP_NewStream failed with error: %d URLString: %s", npErr, [URL _web_URLCString]);
        stream.ndata = nil;
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
            ERROR("Stream type: NP_SEEK not yet supported");
            // FIXME: Need to properly handle this error.
            break;
        default:
            ERROR("unknown stream type");
    }
}

- (void)startStreamWithResponse:(NSURLResponse *)r
{
    [self startStreamWithURL:[r URL]
       expectedContentLength:[r expectedContentLength]
            lastModifiedDate:[r _lastModifiedDate]
                    MIMEType:[r MIMEType]];
}

- (void)receivedData:(NSData *)data
{
    if (![plugin isLoaded] || !stream.ndata || [data length] == 0) {
        return;
    }
    
    if (transferMode != NP_ASFILEONLY) {
        int32 numBytes;
        
        numBytes = NPP_WriteReady(instance, &stream);
        LOG(Plugins, "NPP_WriteReady bytes=%lu", numBytes);
        
        numBytes = NPP_Write(instance, &stream, offset, [data length], (void *)[data bytes]);
        LOG(Plugins, "NPP_Write bytes=%lu", numBytes);
        
        offset += [data length];
    }
}

- (void)destroyStreamWithReason:(NPReason)reason
{
    if (![plugin isLoaded] || !stream.ndata) {
        return;
    }
    
    NPError npErr;
    npErr = NPP_DestroyStream(instance, &stream, reason);
    LOG(Plugins, "NPP_DestroyStream: %d", npErr);
    
    stream.ndata = nil;
        
    if (notifyData) {
        NPP_URLNotify(instance, [URL _web_URLCString], reason, notifyData);
        LOG(Plugins, "NPP_URLNotify");
    }
}

- (void)receivedError:(NPError)reason
{
    [self destroyStreamWithReason:reason];
}

- (void)finishedLoadingWithData:(NSData *)data
{
    if (![plugin isLoaded] || !stream.ndata) {
        return;
    }
    
    if ((transferMode == NP_ASFILE || transferMode == NP_ASFILEONLY) && [data length] > 0) {
        if (!path) {
            path = strdup("/tmp/WebKitPlugInStreamXXXXXX");
            int fd = mkstemp(path);
            if (fd == -1) {
                // This should almost never happen.
                ERROR("can't make temporary file, almost certainly a problem with /tmp");
                // This is not a network error, but the only error codes are "network error" and "user break".
                [self destroyStreamWithReason:NPRES_NETWORK_ERR];
                free(path);
                path = NULL;
                return;
            }
            int dataLength = [data length];
            int byteCount = write(fd, [data bytes], dataLength);
            if (byteCount != dataLength) {
                // This happens only rarely, when we are out of disk space or have a disk I/O error.
                ERROR("error writing to temporary file, errno %d", errno);
                close(fd);
                // This is not a network error, but the only error codes are "network error" and "user break".
                [self destroyStreamWithReason:NPRES_NETWORK_ERR];
                free(path);
                path = NULL;
                return;
            }
            close(fd);
        }
        
        NSString *carbonPath = [[NSFileManager defaultManager] _web_carbonPathForPath:[NSString stringWithCString:path]];
        NPP_StreamAsFile(instance, &stream, [carbonPath cString]);
        LOG(Plugins, "NPP_StreamAsFile: %@", carbonPath);
    }

    [self destroyStreamWithReason:NPRES_DONE];
}

@end
