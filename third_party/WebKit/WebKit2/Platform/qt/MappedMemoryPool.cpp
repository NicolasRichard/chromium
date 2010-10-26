/*
 * Copyright (C) 2010 University of Szeged
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY UNIVERSITY OF SZEGED ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL UNIVERSITY OF SZEGED OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "MappedMemoryPool.h"

#include "StdLibExtras.h"
#include <QDir>
#include <QIODevice>
#include <QTemporaryFile>

namespace WebKit {

MappedMemoryPool* MappedMemoryPool::instance()
{
    DEFINE_STATIC_LOCAL(MappedMemoryPool, singleton, ());
    return &singleton;
}

MappedMemory* MappedMemoryPool::searchForMappedMemory(uchar* p)
{
    for (unsigned n = 0; n < m_pool.size(); ++n) {
        MappedMemory& current = m_pool.at(n);
        if (current.data() == p)
            return reinterpret_cast<MappedMemory*>(&current);
    }
    return 0;
}

MappedMemory* MappedMemoryPool::mapMemory(size_t size, QIODevice::OpenMode openMode)
{
    for (unsigned n = 0; n < m_pool.size(); ++n) {
        MappedMemory& current = m_pool.at(n);
        if (current.dataSize >= size && current.isFree()) {
            current.markUsed();
            return &current;
        }
    }
    MappedMemory newMap;
    newMap.dataSize = size;
    newMap.file = new QTemporaryFile(QDir::tempPath() + "/WebKit2UpdateChunk");
    newMap.file->open(openMode);
    newMap.file->resize(newMap.mapSize());
    newMap.mappedBytes = newMap.file->map(0, newMap.mapSize());
    newMap.file->close();
    newMap.markUsed();
    m_pool.append(newMap);
    return &m_pool.last();
}

MappedMemory* MappedMemoryPool::mapFile(QString fileName, size_t size, QIODevice::OpenMode openMode)
{
    for (unsigned n = 0; n < m_pool.size(); ++n) {
        MappedMemory& current = m_pool.at(n);
        if (current.file->fileName() == fileName) {
            ASSERT(!current.isFree());
            return &current;
        }
    }
    MappedMemory newMap;
    newMap.file = new QFile(fileName);
    if (!newMap.file->open(openMode))
        return 0;
    newMap.dataSize = size;
    newMap.mappedBytes = newMap.file->map(0, newMap.mapSize());
    ASSERT(!newMap.isFree());
    newMap.file->close();
    newMap.file->remove(); // The map stays alive even when the file is unlinked.
    m_pool.append(newMap);
    return &m_pool.last();
}

} // namespace WebKit
