/*
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies)
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

#ifndef DataTransferItemQt_h
#define DataTransferItemQt_h

#if ENABLE(DATA_TRANSFER_ITEMS)

#include "DataTransferItem.h"
#include <wtf/RefPtr.h>

namespace WebCore {

class Clipboard;
class ScriptExecutionContext;

class DataTransferItemQt : public DataTransferItem {
public:
    static PassRefPtr<DataTransferItemQt> createFromPasteboard(PassRefPtr<Clipboard> owner,
                                                               ScriptExecutionContext*,
                                                               const String&);
    static PassRefPtr<DataTransferItemQt> create(PassRefPtr<Clipboard> owner,
                                                 ScriptExecutionContext*,
                                                 const String&,
                                                 const String&);

    virtual void getAsString(PassRefPtr<StringCallback>);
    virtual PassRefPtr<Blob> getAsFile();

private:
    enum DataSource {
        PasteboardSource,
        InternalSource
    };

    DataTransferItemQt(PassRefPtr<Clipboard> owner,
                       ScriptExecutionContext*,
                       DataSource,
                       const String&, const String&, const String&);

    ScriptExecutionContext* m_context;
    const DataSource m_dataSource;
    const String m_data;
};

}

#endif // ENABLE(DATA_TRANSFER_ITEMS)

#endif
