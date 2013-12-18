/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "heap/Heap.h"

#include "heap/ThreadState.h"

#include "wtf/OwnPtr.h"

#if OS(POSIX)
#include <sys/mman.h>
#include <unistd.h>
#elif OS(WIN)
#include <windows.h>
#endif

namespace WebCore {

#if OS(WIN)
static bool IsPowerOf2(size_t power)
{
    return !((power - 1) & power);
}
#endif

static Address roundToBlinkPageBoundary(void* base)
{
    return reinterpret_cast<Address>((reinterpret_cast<uintptr_t>(base) + blinkPageOffsetMask) & blinkPageBaseMask);
}

static size_t roundToOsPageSize(size_t size)
{
    return (size + osPageSize() - 1) & ~(osPageSize() - 1);
}

size_t osPageSize()
{
#if OS(POSIX)
    static const size_t pageSize = getpagesize();
#else
    static size_t pageSize = 0;
    if (!pageSize) {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        pageSize = info.dwPageSize;
        ASSERT(IsPowerOf2(pageSize));
    }
#endif
    return pageSize;
}

class MemoryRegion {
public:
    MemoryRegion(Address base, size_t size) : m_base(base), m_size(size) { ASSERT(size > 0); }

    bool contains(Address addr) const
    {
        return m_base <= addr && addr < (m_base + m_size);
    }


    bool contains(const MemoryRegion& other) const
    {
        return contains(other.m_base) && contains(other.m_base + other.m_size - 1);
    }

    void release()
    {
#if OS(POSIX)
        int err = munmap(m_base, m_size);
        RELEASE_ASSERT(!err);
#else
        bool success = VirtualFree(m_base, 0, MEM_RELEASE);
        RELEASE_ASSERT(success);
#endif
    }

    WARN_UNUSED_RETURN bool commit()
    {
#if OS(POSIX)
        int err = mprotect(m_base, m_size, PROT_READ | PROT_WRITE);
        if (!err) {
            madvise(m_base, m_size, MADV_NORMAL);
            return true;
        }
        return false;
#else
        void* result = VirtualAlloc(m_base, m_size, MEM_COMMIT, PAGE_READWRITE);
        return !!result;
#endif
    }

    void decommit()
    {
#if OS(POSIX)
        int err = mprotect(m_base, m_size, PROT_NONE);
        RELEASE_ASSERT(!err);
        // FIXME: Consider using MADV_FREE on MacOS.
        madvise(m_base, m_size, MADV_DONTNEED);
#else
        bool success = VirtualFree(m_base, m_size, MEM_DECOMMIT);
        RELEASE_ASSERT(success);
#endif
    }

    Address base() const { return m_base; }

private:
    Address m_base;
    size_t m_size;
};

// Representation of the memory used for a Blink heap page.
//
// The representation keeps track of two memory regions:
//
// 1. The virtual memory reserved from the sytem in order to be able
//    to free all the virtual memory reserved on destruction.
//
// 2. The writable memory (a sub-region of the reserved virtual
//    memory region) that is used for the actual heap page payload.
//
// Guard pages are created before and after the writable memory.
class PageMemory {
public:
    ~PageMemory() { m_reserved.release(); }

    bool commit() WARN_UNUSED_RETURN { return m_writable.commit(); }
    void decommit() { m_writable.decommit(); }

    Address writableStart() { return m_writable.base(); }

    // Allocate a virtual address space for the blink page with the
    // following layout:
    //
    //    [ guard os page | ... payload ... | guard os page ]
    //    ^---{ aligned to blink page size }
    //
    static PageMemory* allocate(size_t payloadSize)
    {
        ASSERT(payloadSize > 0);

        // Virtual memory allocation routines operate in OS page sizes.
        // Round up the requested size to nearest os page size.
        payloadSize = roundToOsPageSize(payloadSize);

        // Overallocate by blinkPageSize and 2 times OS page size to
        // ensure a chunk of memory which is blinkPageSize aligned and
        // has a system page before and after to use for guarding. We
        // unmap the excess memory before returning.
        size_t allocationSize = payloadSize + 2 * osPageSize() + blinkPageSize;

#if OS(POSIX)
        Address base = static_cast<Address>(mmap(0, allocationSize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0));
        RELEASE_ASSERT(base != MAP_FAILED);

        Address end = base + allocationSize;
        Address alignedBase = roundToBlinkPageBoundary(base);
        Address payloadBase = alignedBase + osPageSize();
        Address payloadEnd = payloadBase + payloadSize;
        Address blinkPageEnd = payloadEnd + osPageSize();

        // If the allocate memory was not blink page aligned release
        // the memory before the aligned address.
        if (alignedBase != base)
            MemoryRegion(base, alignedBase - base).release();

        // Create guard pages by decommiting an OS page before and
        // after the payload.
        MemoryRegion(alignedBase, osPageSize()).decommit();
        MemoryRegion(payloadEnd, osPageSize()).decommit();

        // Free the additional memory at the end of the page if any.
        if (blinkPageEnd < end)
            MemoryRegion(blinkPageEnd, end - blinkPageEnd).release();

        return new PageMemory(MemoryRegion(alignedBase, blinkPageEnd - alignedBase), MemoryRegion(payloadBase, payloadSize));
#else
        Address base = 0;
        Address alignedBase = 0;

        // On Windows it is impossible to partially release a region
        // of memory allocated by VirtualAlloc. To avoid wasting
        // virtual address space we attempt to release a large region
        // of memory returned as a whole and then allocate an aligned
        // region inside this larger region.
        for (int attempt = 0; attempt < 3; attempt++) {
            base = static_cast<Address>(VirtualAlloc(0, allocationSize, MEM_RESERVE, PAGE_NOACCESS));
            RELEASE_ASSERT(base);
            VirtualFree(base, 0, MEM_RELEASE);

            alignedBase = roundToBlinkPageBoundary(base);
            base = static_cast<Address>(VirtualAlloc(alignedBase, payloadSize + 2 * osPageSize(), MEM_RESERVE, PAGE_NOACCESS));
            if (base) {
                RELEASE_ASSERT(base == alignedBase);
                allocationSize = payloadSize + 2 * osPageSize();
                break;
            }
        }

        if (!base) {
            // We failed to avoid wasting virtual address space after
            // several attempts.
            base = static_cast<Address>(VirtualAlloc(0, allocationSize, MEM_RESERVE, PAGE_NOACCESS));
            RELEASE_ASSERT(base);

            // FIXME: If base is by accident blink page size aligned
            // here then we can create two pages out of reserved
            // space. Do this.
            alignedBase = roundToBlinkPageBoundary(base);
        }

        Address payloadBase = alignedBase + osPageSize();
        PageMemory* storage = new PageMemory(MemoryRegion(base, allocationSize), MemoryRegion(payloadBase, payloadSize));
        bool res = storage->commit();
        RELEASE_ASSERT(res);
        return storage;
#endif
    }

private:
    PageMemory(const MemoryRegion& reserved, const MemoryRegion& writable)
        : m_reserved(reserved)
        , m_writable(writable)
    {
        ASSERT(reserved.contains(writable));
    }

    MemoryRegion m_reserved;
    MemoryRegion m_writable;
};

NO_SANITIZE_ADDRESS
bool HeapObjectHeader::isMarked() const
{
    checkHeader();
    return m_size & markBitMask;
}

NO_SANITIZE_ADDRESS
void HeapObjectHeader::unmark()
{
    checkHeader();
    m_size &= ~markBitMask;
}

NO_SANITIZE_ADDRESS
bool HeapObjectHeader::hasDebugMark() const
{
    checkHeader();
    return m_size & debugBitMask;
}

NO_SANITIZE_ADDRESS
void HeapObjectHeader::clearDebugMark()
{
    checkHeader();
    m_size &= ~debugBitMask;
}

NO_SANITIZE_ADDRESS
void HeapObjectHeader::setDebugMark()
{
    checkHeader();
    m_size |= debugBitMask;
}

#ifndef NDEBUG
NO_SANITIZE_ADDRESS
void HeapObjectHeader::zapMagic()
{
    m_magic = zappedMagic;
}
#endif

HeapObjectHeader* HeapObjectHeader::fromPayload(const void* payload)
{
    Address addr = reinterpret_cast<Address>(const_cast<void*>(payload));
    HeapObjectHeader* header =
        reinterpret_cast<HeapObjectHeader*>(addr - objectHeaderSize);
    return header;
}

void HeapObjectHeader::finalize(const GCInfo* gcInfo, Address object, size_t objectSize)
{
    ASSERT(gcInfo);
#if TRACE_GC_FINALIZATION
    const char* className = 0;
    if (gcInfo->m_classOf)
        className = gcInfo->m_classOf(object);
    printf("%11s %30s <: %s\n", gcInfo->hasFinalizer() ? "Finalizer" : "NoFinalizer", className ? className : "?", gcInfo->m_typeMarker);
#endif
    if (gcInfo->hasFinalizer()) {
        gcInfo->m_finalize(object);
    }
#ifndef NDEBUG
    for (size_t i = 0; i < objectSize; i++)
        object[i] = finalizedZapValue;
#endif
    // Zap the primary vTable entry (secondary vTable entries are not zapped)
    *(reinterpret_cast<uintptr_t*>(object)) = zappedVTable;
}

NO_SANITIZE_ADDRESS
void FinalizedHeapObjectHeader::finalize()
{
    HeapObjectHeader::finalize(m_gcInfo, payload(), payloadSize());
}

template<typename Header>
void LargeHeapObject<Header>::unmark()
{
    return heapObjectHeader()->unmark();
}

template<typename Header>
bool LargeHeapObject<Header>::isMarked()
{
    return heapObjectHeader()->isMarked();
}

template<typename Header>
bool LargeHeapObject<Header>::checkAndMarkPointer(Visitor* visitor, Address address)
{
    if (contains(address)) {
#if defined(TRACE_GC_MARKING) && TRACE_GC_MARKING
        visitor->setHostInfo(&address, "stack");
#endif
        mark(visitor);
        return true;
    }
    return false;
}

template<>
void LargeHeapObject<FinalizedHeapObjectHeader>::mark(Visitor* visitor)
{
    visitor->mark(heapObjectHeader(), heapObjectHeader()->traceCallback());
}

template<>
void LargeHeapObject<HeapObjectHeader>::mark(Visitor* visitor)
{
    ASSERT(gcInfo());
    visitor->mark(heapObjectHeader(), gcInfo()->m_trace);
}

template<>
void LargeHeapObject<FinalizedHeapObjectHeader>::finalize()
{
    heapObjectHeader()->finalize();
}

template<>
void LargeHeapObject<HeapObjectHeader>::finalize()
{
    ASSERT(gcInfo());
    HeapObjectHeader::finalize(gcInfo(), payload(), payloadSize());
}

FinalizedHeapObjectHeader* FinalizedHeapObjectHeader::fromPayload(const void* payload)
{
    Address addr = reinterpret_cast<Address>(const_cast<void*>(payload));
    FinalizedHeapObjectHeader* header =
        reinterpret_cast<FinalizedHeapObjectHeader*>(addr - finalizedHeaderSize);
    return header;
}

template<typename Header>
ThreadHeap<Header>::ThreadHeap(ThreadState* state)
    : m_currentAllocationPoint(0)
    , m_remainingAllocationSize(0)
    , m_firstPage(0)
    , m_firstLargeHeapObject(0)
    , m_biggestFreeListIndex(0)
    , m_inFinalizeAll(false)
    , m_threadState(state)
    , m_pagePool(0)
{
    clearFreeLists();
}

template<typename Header>
ThreadHeap<Header>::~ThreadHeap()
{
    clearFreeLists();
    // FIXME(oilpan): at the moment we can't finalize all objects owned by the main thread eagerly
    // because there are tangled destruction order dependencies there.
    if (!ThreadState::isMainThread())
        finalizeAll();
    deletePages();
}

template<typename Header>
bool ThreadHeap<Header>::allocateFromFreeList(size_t minSize)
{
    size_t bucketSize = 1 << m_biggestFreeListIndex;
    int i = m_biggestFreeListIndex;
    for (; i > 0; i--, bucketSize >>= 1) {
        if (bucketSize < minSize)
            break;
        FreeListEntry* entry = m_freeLists[i];
        if (entry) {
            m_biggestFreeListIndex = i;
            entry->unlink(&m_freeLists[i]);
            setAllocationPoint(entry->address(), entry->size());
            ASSERT(currentAllocationPoint() && remainingAllocationSize() >= minSize);
            return true;
        }
    }
    m_biggestFreeListIndex = i;
    return false;
}

template<typename Header>
void ThreadHeap<Header>::ensureCurrentAllocation(size_t minSize, const GCInfo* gcInfo)
{
    ASSERT(minSize >= allocationGranularity);
    if (remainingAllocationSize() >= minSize)
        return;

    if (remainingAllocationSize() > 0)
        addToFreeList(currentAllocationPoint(), remainingAllocationSize());
    if (allocateFromFreeList(minSize))
        return;
    addPageToHeap(gcInfo);
    bool success = allocateFromFreeList(minSize);
    RELEASE_ASSERT(success);
}

template<typename Header>
BaseHeapPage* ThreadHeap<Header>::heapPageFromAddress(Address address)
{
    for (HeapPage<Header>* page = m_firstPage; page; page = page->next()) {
        if (page->contains(address))
            return page;
    }
    return 0;
}

template<typename Header>
BaseHeapPage* ThreadHeap<Header>::largeHeapObjectFromAddress(Address address)
{
    for (LargeHeapObject<Header>* current = m_firstLargeHeapObject; current; current = current->next()) {
        if (current->contains(address))
            return current;
    }
    return 0;
}

template<typename Header>
bool ThreadHeap<Header>::checkAndMarkLargeHeapObject(Visitor* visitor, Address address)
{
    for (LargeHeapObject<Header>* current = m_firstLargeHeapObject; current; current = current->next()) {
        if (current->checkAndMarkPointer(visitor, address))
            return true;
    }
    return false;
}

template<typename Header>
void ThreadHeap<Header>::addToFreeList(Address address, size_t size)
{
    ASSERT(heapPageFromAddress(address));
    ASSERT(heapPageFromAddress(address + size - 1));
    ASSERT(size < blinkPagePayloadSize());
    // The free list entries are only pointer aligned (but when we allocate
    // from them we are 8 byte aligned due to the header size).
    ASSERT(!((reinterpret_cast<uintptr_t>(address) + sizeof(Header)) & allocationMask));
    ASSERT(!(size & allocationMask));
    ASAN_POISON_MEMORY_REGION(address, size);
    FreeListEntry* entry;
    if (size < sizeof(*entry)) {
        // Create a dummy header with only a size and freelist bit set.
        ASSERT(size >= sizeof(BasicObjectHeader));
        // Free list encode the size to mark the lost memory as freelist memory.
        new (NotNull, address) BasicObjectHeader(BasicObjectHeader::freeListEncodedSize(size));
        // This memory gets lost. Sweeping can reclaim it.
        return;
    }
    entry = new (NotNull, address) FreeListEntry(size);
#if USE_ASAN
    // For ASAN we don't add the entry to the free lists until the asanDeferMemoryReuseCount
    // reaches zero. However we always add entire pages to ensure that adding a new page will
    // increase the allocation space.
    if (HeapPage<Header>::payloadSize() != size && !entry->shouldAddToFreeList())
        return;
#endif
    int index = bucketIndexForSize(size);
    entry->link(&m_freeLists[index]);
    if (index > m_biggestFreeListIndex)
        m_biggestFreeListIndex = index;
}

template<typename Header>
Address ThreadHeap<Header>::allocateLargeObject(size_t size, const GCInfo* gcInfo)
{
    // FIXME: Implement.
    ASSERT_NOT_REACHED();
    return 0;
}

template<typename Header>
void ThreadHeap<Header>::freeLargeObject(LargeHeapObject<Header>* object, LargeHeapObject<Header>** previousNext)
{
    // FIXME: Implement.
    ASSERT_NOT_REACHED();
}

template<>
void ThreadHeap<FinalizedHeapObjectHeader>::addPageToHeap(const GCInfo* gcInfo)
{
    // When adding a page to the ThreadHeap using FinalizedHeapObjectHeaders the GCInfo on
    // the heap should be unused (ie. 0).
    allocatePage(0);
}

template<>
void ThreadHeap<HeapObjectHeader>::addPageToHeap(const GCInfo* gcInfo)
{
    // When adding a page to the ThreadHeap using HeapObjectHeaders store the GCInfo on the heap
    // since it is the same for all objects
    ASSERT(gcInfo);
    allocatePage(gcInfo);
}

template<typename Header>
void ThreadHeap<Header>::clearPagePool()
{
    while (takePageFromPool()) { }
}

template<typename Header>
PageMemory* ThreadHeap<Header>::takePageFromPool()
{
    while (PagePoolEntry* entry = m_pagePool) {
        m_pagePool = entry->next();
        PageMemory* storage = entry->storage();
        delete entry;

        if (storage->commit())
            return storage;

        // Failed to commit pooled storage. Release it.
        delete storage;
    }

    return 0;
}

template<typename Header>
void ThreadHeap<Header>::addPageToPool(HeapPage<Header>* unused)
{
    PageMemory* storage = unused->storage();
    PagePoolEntry* entry = new PagePoolEntry(storage, m_pagePool);
    m_pagePool = entry;
    storage->decommit();
}

template<typename Header>
void ThreadHeap<Header>::allocatePage(const GCInfo* gcInfo)
{
    heapContainsCache()->flush();
    PageMemory* pageMemory = takePageFromPool();
    if (!pageMemory) {
        pageMemory = PageMemory::allocate(blinkPagePayloadSize());
        RELEASE_ASSERT(pageMemory);
    }
    HeapPage<Header>* page = new (pageMemory->writableStart()) HeapPage<Header>(pageMemory, this, gcInfo);
    page->link(&m_firstPage);
    addToFreeList(page->payload(), HeapPage<Header>::payloadSize());
}

#ifndef NDEBUG
template<typename Header>
void ThreadHeap<Header>::getScannedStats(HeapStats& scannedStats)
{
    for (HeapPage<Header>* page = m_firstPage; page; page = page->next())
        page->getStats(scannedStats);
    for (LargeHeapObject<Header>* current = m_firstLargeHeapObject; current; current = current->next())
        current->getStats(scannedStats);
}
#endif

template<typename Header>
void ThreadHeap<Header>::sweep()
{
    ASSERT(isConsistentForGC());
#if USE_ASAN
    // When using ASAN do a pre-sweep where all unmarked objects are poisoned before
    // calling their finalizer methods. This can catch the cases where one objects
    // finalizer tries to modify another object as part of finalization.
    for (HeapPage<Header>* page = m_firstPage; page; page = page->next())
        page->poisonUnmarkedObjects();
#endif
    HeapPage<Header>* page = m_firstPage;
    HeapPage<Header>** previous = &m_firstPage;
    bool pagesRemoved = false;
    while (page) {
        if (page->isEmpty()) {
            HeapPage<Header>* unused = page;
            page = page->next();
            HeapPage<Header>::unlink(unused, previous);
            pagesRemoved = true;
        } else {
            page->sweep();
            previous = &page->m_next;
            page = page->next();
        }
    }
    if (pagesRemoved)
        heapContainsCache()->flush();

    LargeHeapObject<Header>** previousNext = &m_firstLargeHeapObject;
    for (LargeHeapObject<Header>* current = m_firstLargeHeapObject; current;) {
        if (current->isMarked()) {
            stats().increaseAllocatedSpace(current->size());
            stats().increaseObjectSpace(current->payloadSize());
            current->unmark();
            previousNext = &current->m_next;
            current = current->next();
        } else {
            LargeHeapObject<Header>* next = current->next();
            freeLargeObject(current, previousNext);
            current = next;
        }
    }
}

template<typename Header>
void ThreadHeap<Header>::finalizeAll(const void* except)
{
    // FIXME: Implement.
    ASSERT_NOT_REACHED();
}

template<typename Header>
bool ThreadHeap<Header>::isConsistentForGC()
{
    for (size_t i = 0; i < blinkPageSizeLog2; i++) {
        if (m_freeLists[i])
            return false;
    }
    return !ownsNonEmptyAllocationArea();
}

template<typename Header>
void ThreadHeap<Header>::makeConsistentForGC()
{
    if (ownsNonEmptyAllocationArea())
        addToFreeList(currentAllocationPoint(), remainingAllocationSize());
    setAllocationPoint(0, 0);
    clearFreeLists();
}

template<typename Header>
void ThreadHeap<Header>::clearMarks()
{
    ASSERT(isConsistentForGC());
    for (HeapPage<Header>* page = m_firstPage; page; page = page->next())
        page->clearMarks();
    for (LargeHeapObject<Header>* current = m_firstLargeHeapObject; current; current = current->next())
        current->unmark();
}

template<typename Header>
void ThreadHeap<Header>::deletePages()
{
    heapContainsCache()->flush();
    // Add all pages in the pool to the heap's list of pages before deleting
    clearPagePool();

    for (HeapPage<Header>* page = m_firstPage; page; ) {
        HeapPage<Header>* dead = page;
        page = page->next();
        PageMemory* storage = dead->storage();
        dead->~HeapPage();
        delete storage;
    }
    m_firstPage = 0;

    for (LargeHeapObject<Header>* current = m_firstLargeHeapObject; current;) {
        LargeHeapObject<Header>* dead = current;
        current = current->next();
        PageMemory* storage = dead->storage();
        dead->~LargeHeapObject();
        delete storage;
    }
    m_firstLargeHeapObject = 0;
}

template<typename Header>
void ThreadHeap<Header>::clearFreeLists()
{
    for (size_t i = 0; i < blinkPageSizeLog2; i++)
        m_freeLists[i] = 0;
}

int BaseHeap::bucketIndexForSize(size_t size)
{
    ASSERT(size > 0);
    int index = -1;
    while (size) {
        size >>= 1;
        index++;
    }
    return index;
}

template<typename Header>
HeapPage<Header>::HeapPage(PageMemory* storage, ThreadHeap<Header>* heap, const GCInfo* gcInfo)
    : BaseHeapPage(storage, gcInfo)
    , m_next(0)
    , m_heap(heap)
{
    COMPILE_ASSERT(!(sizeof(HeapPage<Header>) & allocationMask), page_header_incorrectly_aligned);
    m_objectStartBitMapComputed = false;
    ASSERT(isPageHeaderAddress(reinterpret_cast<Address>(this)));
    heap->stats().increaseAllocatedSpace(blinkPageSize);
}

template<typename Header>
void HeapPage<Header>::link(HeapPage** prevNext)
{
    m_next = *prevNext;
    *prevNext = this;
}

template<typename Header>
void HeapPage<Header>::unlink(HeapPage* unused, HeapPage** prevNext)
{
    *prevNext = unused->m_next;
    unused->heap()->addPageToPool(unused);
}

template<typename Header>
void HeapPage<Header>::getStats(HeapStats& stats)
{
    stats.increaseAllocatedSpace(blinkPageSize);
    Address headerAddress = payload();
    ASSERT(headerAddress != end());
    do {
        Header* header = reinterpret_cast<Header*>(headerAddress);
        if (!header->isFree())
            stats.increaseObjectSpace(header->payloadSize());
        ASSERT(header->size() < blinkPagePayloadSize());
        headerAddress += header->size();
        ASSERT(headerAddress <= end());
    } while (headerAddress < end());
}

template<typename Header>
bool HeapPage<Header>::isEmpty()
{
    BasicObjectHeader* header = reinterpret_cast<BasicObjectHeader*>(payload());
    return header->isFree() && (header->size() == payloadSize());
}

template<typename Header>
void HeapPage<Header>::sweep()
{
    clearObjectStartBitMap();
    heap()->stats().increaseAllocatedSpace(blinkPageSize);
    Address startOfGap = payload();
    for (Address headerAddress = startOfGap; headerAddress < end(); ) {
        BasicObjectHeader* basicHeader = reinterpret_cast<BasicObjectHeader*>(headerAddress);
        ASSERT(basicHeader->size() < blinkPagePayloadSize());

        if (basicHeader->isFree()) {
            headerAddress += basicHeader->size();
            continue;
        }
        // At this point we know this is a valid object of type Header
        Header* header = static_cast<Header*>(basicHeader);

        if (!header->isMarked()) {
            // For ASAN we unpoison the specific object when calling the finalizer and
            // poison it again when done to allow the object's own finalizer to operate
            // on the object, but not have other finalizers be allowed to access it.
            ASAN_UNPOISON_MEMORY_REGION(header->payload(), header->payloadSize());
            // FIXME: Add back perf stats.
            // PERF_STATS(ScopedTimer timer(&heap()->threadState()->perfStats().m_finalizationTime));
            finalize(header);
            ASAN_POISON_MEMORY_REGION(header->payload(), header->payloadSize());
            headerAddress += header->size();
            continue;
        }

        if (startOfGap != headerAddress)
            heap()->addToFreeList(startOfGap, headerAddress - startOfGap);
        header->unmark();
        headerAddress += header->size();
        heap()->stats().increaseObjectSpace(header->payloadSize());
        startOfGap = headerAddress;
    }
    if (startOfGap != end())
        heap()->addToFreeList(startOfGap, end() - startOfGap);
}

template<typename Header>
void HeapPage<Header>::clearMarks()
{
    for (Address headerAddress = payload(); headerAddress < end();) {
        Header* header = reinterpret_cast<Header*>(headerAddress);
        ASSERT(header->size() < blinkPagePayloadSize());
        if (!header->isFree())
            header->unmark();
        headerAddress += header->size();
    }
}

template<typename Header>
void HeapPage<Header>::populateObjectStartBitMap()
{
    memset(&m_objectStartBitMap, 0, objectStartBitMapSize);
    Address start = payload();
    for (Address headerAddress = start; headerAddress < end();) {
        Header* header = reinterpret_cast<Header*>(headerAddress);
        size_t objectOffset = headerAddress - start;
        ASSERT(!(objectOffset & allocationMask));
        size_t objectStartNumber = objectOffset / allocationGranularity;
        size_t mapIndex = objectStartNumber / 8;
        ASSERT(mapIndex < objectStartBitMapSize);
        m_objectStartBitMap[mapIndex] |= (1 << (objectStartNumber & 7));
        headerAddress += header->size();
        ASSERT(headerAddress <= end());
    }
    m_objectStartBitMapComputed = true;
}

template<typename Header>
void HeapPage<Header>::clearObjectStartBitMap()
{
    m_objectStartBitMapComputed = false;
}

static int numberOfLeadingZeroes(uint8_t byte)
{
    if (!byte)
        return 8;
    int result = 0;
    if (byte <= 0x0F) {
        result += 4;
        byte = byte << 4;
    }
    if (byte <= 0x3F) {
        result += 2;
        byte = byte << 2;
    }
    if (byte <= 0x7F)
        result++;
    return result;
}

template<typename Header>
bool HeapPage<Header>::checkAndMarkPointer(Visitor* visitor, Address addr)
{
    if (addr < payload())
        return false;
    if (!isObjectStartBitMapComputed())
        populateObjectStartBitMap();
    size_t objectOffset = addr - payload();
    size_t objectStartNumber = objectOffset / allocationGranularity;
    size_t mapIndex = objectStartNumber / 8;
    ASSERT(mapIndex < objectStartBitMapSize);
    size_t bit = objectStartNumber & 7;
    uint8_t byte = m_objectStartBitMap[mapIndex] & ((1 << (bit + 1)) - 1);
    while (!byte) {
        ASSERT(mapIndex > 0);
        byte = m_objectStartBitMap[--mapIndex];
    }
    int leadingZeroes = numberOfLeadingZeroes(byte);
    objectStartNumber = (mapIndex * 8) + 7 - leadingZeroes;
    objectOffset = objectStartNumber * allocationGranularity;
    Address objectAddress = objectOffset + payload();
    Header* header = reinterpret_cast<Header*>(objectAddress);
    if (header->isFree())
        return false;

#if defined(TRACE_GC_MARKING) && TRACE_GC_MARKING
    visitor->setHostInfo(&addr, "stack");
#endif
    visitor->mark(header, traceCallback(header));
    return true;
}

#if USE_ASAN
template<typename Header>
void HeapPage<Header>::poisonUnmarkedObjects()
{
    for (Address headerAddress = payload(); headerAddress < end(); ) {
        Header* header = reinterpret_cast<Header*>(headerAddress);
        ASSERT(header->size() < blinkPagePayloadSize());

        if (!header->isFree() && !header->isMarked())
            ASAN_POISON_MEMORY_REGION(header->payload(), header->payloadSize());
        headerAddress += header->size();
    }
}
#endif

template<>
inline void HeapPage<FinalizedHeapObjectHeader>::finalize(FinalizedHeapObjectHeader* header)
{
    header->finalize();
}

template<>
inline void HeapPage<HeapObjectHeader>::finalize(HeapObjectHeader* header)
{
    ASSERT(gcInfo());
    HeapObjectHeader::finalize(gcInfo(), header->payload(), header->payloadSize());
}

template<>
inline TraceCallback HeapPage<HeapObjectHeader>::traceCallback(HeapObjectHeader* header)
{
    ASSERT(gcInfo());
    return gcInfo()->m_trace;
}

template<>
inline TraceCallback HeapPage<FinalizedHeapObjectHeader>::traceCallback(FinalizedHeapObjectHeader* header)
{
    return header->traceCallback();
}

template<typename Header>
void LargeHeapObject<Header>::getStats(HeapStats& stats)
{
    stats.increaseAllocatedSpace(size());
    stats.increaseObjectSpace(payloadSize());
}

void HeapContainsCache::flush()
{
    for (int i = 0; i < numberOfEntries; i++)
        m_entries[i] = Entry();
}

int HeapContainsCache::hash(Address address)
{
    uintptr_t value = (reinterpret_cast<uintptr_t>(address) >> blinkPageSizeLog2);
    value ^= value >> numberOfEntriesLog2;
    value ^= value >> (numberOfEntriesLog2 * 2);
    value &= numberOfEntries - 1;
    return value & ~1; // Returns only even number.
}

bool HeapContainsCache::lookup(Address address, BaseHeapPage** page)
{
    ASSERT(page);
    int index = hash(address);
    ASSERT(!(index & 1));
    Address cachePage = roundToBlinkPageStart(address);
    if (m_entries[index].address() == cachePage) {
        *page = m_entries[index].containingPage();
        return true;
    }
    if (m_entries[index + 1].address() == cachePage) {
        *page = m_entries[index + 1].containingPage();
        return true;
    }
    *page = 0;
    return false;
}

void HeapContainsCache::addEntry(Address address, BaseHeapPage* page)
{
    int index = hash(address);
    ASSERT(!(index & 1));
    Address cachePage = roundToBlinkPageStart(address);
    m_entries[index + 1] = m_entries[index];
    m_entries[index] = Entry(cachePage, page);
}

void Heap::init(intptr_t* startOfStack)
{
    ThreadState::init(startOfStack);
}

void Heap::shutdown()
{
    ThreadState::shutdown();
}

// Force template instantiations for the types that we need.
template class HeapPage<FinalizedHeapObjectHeader>;
template class HeapPage<HeapObjectHeader>;
template class ThreadHeap<FinalizedHeapObjectHeader>;
template class ThreadHeap<HeapObjectHeader>;

}
