/*
 * Copyright (C) 2002 Apple Computer, Inc.  All rights reserved.
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

#import "KWQResourceLoader.h"

#import "KWQAssertions.h"
#import "KWQKJobClasses.h"
#import "loader.h"

using khtml::Loader;
using KIO::TransferJob;

@implementation KWQResourceLoader

- (id)initWithLoader:(Loader *)loader job:(TransferJob *)job;
{
    [super init];

    _loader = loader;
    _job = job;

    job->setLoader(self);

    return self;
}

- (void)setHandle:(id <WebCoreResourceHandle>)handle
{
    ASSERT(_handle == nil);
    _handle = [handle retain];
}

- (void)receivedResponse:(id)response
{
    ASSERT(response);
    ASSERT(_loader);
    ASSERT(_job);
    _loader->receivedResponse(_job, response);
}

- (void)addData:(NSData *)data
{
    ASSERT(data);
    ASSERT(_loader);
    ASSERT(_job);
    _loader->slotData(_job, (const char *)[data bytes], [data length]);
}

- (void)jobWillBeDeallocated
{
    id <WebCoreResourceHandle> handle = _handle;
    _job = 0;
    _handle = nil;

    [handle cancel];
    [handle release];
}

- (void)cancel
{
    TransferJob *job = _job;
    id <WebCoreResourceHandle> handle = _handle;
    _job = 0;
    _handle = nil;

    if (job) {
        job->setError(1);
        job->setLoader(nil);
    }
    [handle release];
}

- (void)finish
{
    ASSERT(_job);
    ASSERT(_handle);

    TransferJob *job = _job;
    id <WebCoreResourceHandle> handle = _handle;
    _job = 0;
    _handle = nil;

    _loader->slotFinished(job);
    delete job;
    [handle release];
}

@end
