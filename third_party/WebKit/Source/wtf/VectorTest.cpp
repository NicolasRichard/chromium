/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "wtf/OwnPtr.h"
#include "wtf/PassOwnPtr.h"
#include "wtf/Vector.h"
#include <gtest/gtest.h>

namespace {

TEST(WTF_Vector, Basic)
{
    Vector<int> intVector;
    EXPECT_TRUE(intVector.isEmpty());
    EXPECT_EQ(0ul, intVector.size());
    EXPECT_EQ(0ul, intVector.capacity());
}

TEST(WTF_Vector, Reverse)
{
    Vector<int> intVector;
    intVector.append(10);
    intVector.append(11);
    intVector.append(12);
    intVector.append(13);
    intVector.reverse();

    EXPECT_EQ(13, intVector[0]);
    EXPECT_EQ(12, intVector[1]);
    EXPECT_EQ(11, intVector[2]);
    EXPECT_EQ(10, intVector[3]);

    intVector.append(9);
    intVector.reverse();

    EXPECT_EQ(9, intVector[0]);
    EXPECT_EQ(10, intVector[1]);
    EXPECT_EQ(11, intVector[2]);
    EXPECT_EQ(12, intVector[3]);
    EXPECT_EQ(13, intVector[4]);
}

TEST(WTF_Vector, Iterator)
{
    Vector<int> intVector;
    intVector.append(10);
    intVector.append(11);
    intVector.append(12);
    intVector.append(13);

    Vector<int>::iterator it = intVector.begin();
    Vector<int>::iterator end = intVector.end();
    EXPECT_TRUE(end != it);

    EXPECT_EQ(10, *it);
    ++it;
    EXPECT_EQ(11, *it);
    ++it;
    EXPECT_EQ(12, *it);
    ++it;
    EXPECT_EQ(13, *it);
    ++it;

    EXPECT_TRUE(end == it);
}

TEST(WTF_Vector, ReverseIterator)
{
    Vector<int> intVector;
    intVector.append(10);
    intVector.append(11);
    intVector.append(12);
    intVector.append(13);

    Vector<int>::reverse_iterator it = intVector.rbegin();
    Vector<int>::reverse_iterator end = intVector.rend();
    EXPECT_TRUE(end != it);

    EXPECT_EQ(13, *it);
    ++it;
    EXPECT_EQ(12, *it);
    ++it;
    EXPECT_EQ(11, *it);
    ++it;
    EXPECT_EQ(10, *it);
    ++it;

    EXPECT_TRUE(end == it);
}

class DestructCounter {
public:
    explicit DestructCounter(int i, int* destructNumber)
        : m_i(i)
        , m_destructNumber(destructNumber)
    { }

    ~DestructCounter() { ++(*m_destructNumber); }
    int get() const { return m_i; }

private:
    int m_i;
    int* m_destructNumber;
};

typedef WTF::Vector<OwnPtr<DestructCounter> > OwnPtrVector;

TEST(WTF_Vector, OwnPtr)
{
    int destructNumber = 0;
    OwnPtrVector vector;
    vector.append(adoptPtr(new DestructCounter(0, &destructNumber)));
    vector.append(adoptPtr(new DestructCounter(1, &destructNumber)));
    ASSERT_EQ(2, vector.size());

    OwnPtr<DestructCounter>& counter0 = vector.first();
    ASSERT_EQ(0, counter0->get());
    OwnPtr<DestructCounter>& counter1 = vector.last();
    ASSERT_EQ(1, counter1->get());
    ASSERT_EQ(0, destructNumber);

    size_t index = 0;
    for (OwnPtrVector::iterator iter = vector.begin(); iter != vector.end(); ++iter) {
        OwnPtr<DestructCounter>* refCounter = iter;
        ASSERT_EQ(index, refCounter->get()->get());
        ASSERT_EQ(index, (*refCounter)->get());
        index++;
    }
    ASSERT_EQ(0, destructNumber);

    for (index = 0; index < vector.size(); index++) {
        OwnPtr<DestructCounter>& refCounter = vector[index];
        ASSERT_EQ(index, refCounter->get());
        index++;
    }
    ASSERT_EQ(0, destructNumber);

    ASSERT_EQ(0, vector[0]->get());
    ASSERT_EQ(1, vector[1]->get());
    vector.remove(0);
    ASSERT_EQ(1, vector[0]->get());
    ASSERT_EQ(1, vector.size());
    ASSERT_EQ(1, destructNumber);

    OwnPtr<DestructCounter> ownCounter1 = vector[0].release();
    vector.remove(0);
    ASSERT_EQ(counter1.get(), ownCounter1.get());
    ASSERT_EQ(0, vector.size());
    ASSERT_EQ(1, destructNumber);

    ownCounter1.clear();
    ASSERT_EQ(2, destructNumber);

    int count = 1025;
    destructNumber = 0;
    for (int i = 0; i < count; i++)
        vector.prepend(adoptPtr(new DestructCounter(i, &destructNumber)));

    // Vector relocation must not destruct OwnPtr element.
    ASSERT_EQ(0, destructNumber);
    ASSERT_EQ(count, vector.size());

    OwnPtrVector copyVector;
    vector.swap(copyVector);
    ASSERT_EQ(0, destructNumber);
    ASSERT_EQ(count, copyVector.size());
    ASSERT_EQ(0, vector.size());

    copyVector.clear();
    ASSERT_EQ(count, destructNumber);
}

} // namespace
