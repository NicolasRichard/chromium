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
#include "core/html/HTMLImport.h"

#include "core/dom/Document.h"
#include "core/html/HTMLImportStateResolver.h"

namespace WebCore {

Frame* HTMLImport::frame()
{
    return master()->frame();
}

Document* HTMLImport::master()
{
    return root()->document();
}

HTMLImportsController* HTMLImport::controller()
{
    return root()->toController();
}

void HTMLImport::appendChild(HTMLImport* child)
{
    TreeNode<HTMLImport>::appendChild(child);

    // This prevents HTML parser from going beyond the
    // blockage line before the precise state is computed by recalcState().
    if (child->isCreatedByParser())
        forceBlock();

    stateWillChange();
}

void HTMLImport::stateDidChange()
{
    if (!isStateBlockedFromRunningScript()) {
        if (Document* document = this->document())
            document->didLoadAllImports();
    }
}

void HTMLImport::recalcState()
{
    ASSERT(!isStateCacheValid());
    m_cachedState = HTMLImportStateResolver(this).resolve();
}

void HTMLImport::forceBlock()
{
    ASSERT(m_cachedState != Ready);
    m_cachedState = BlockedFromCreatingDocument;
}

void HTMLImport::stateWillChange()
{
    root()->scheduleRecalcState();
}

void HTMLImport::recalcTreeState(HTMLImport* root)
{
    ASSERT(root == root->root());

    HashMap<HTMLImport*, State> snapshot;
    Vector<HTMLImport*> updated;

    for (HTMLImport* i = root; i; i = traverseNext(i)) {
        snapshot.add(i, i->state());
        i->invalidateCachedState();
    }

    // The post-visit DFS order matters here because
    // HTMLImportStateResolver in recalcState() Depends on
    // |m_cachedState| of its children and precedents of ancestors.
    // Accidental cycle dependency of state computation is prevented
    // by invalidateCachedState() and isStateCacheValid() check.
    for (HTMLImport* i = traverseFirstPostOrder(root); i; i = traverseNextPostOrder(i)) {
        i->recalcState();

        State newState = i->state();
        State oldState = snapshot.get(i);
        // Once the state reaches Ready, it shouldn't go back.
        ASSERT(oldState != Ready || oldState <= newState);
        if (newState != oldState)
            updated.append(i);
    }

    for (size_t i = 0; i < updated.size(); ++i)
        updated[i]->stateDidChange();
}

bool HTMLImport::isMaster(Document* document)
{
    if (!document->import())
        return true;
    return (document->import()->master() == document);
}

#if !defined(NDEBUG)
void HTMLImport::show()
{
    root()->showTree(this, 0);
}

void HTMLImport::showTree(HTMLImport* highlight, unsigned depth)
{
    for (unsigned i = 0; i < depth*4; ++i)
        fprintf(stderr, " ");

    fprintf(stderr, "%s", this == highlight ? "*" : " ");
    showThis();
    fprintf(stderr, "\n");
    for (HTMLImport* child = firstChild(); child; child = child->next())
        child->showTree(highlight, depth + 1);
}

void HTMLImport::showThis()
{
    fprintf(stderr, "%p state=%d", this, m_cachedState);
}
#endif

} // namespace WebCore
