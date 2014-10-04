// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// C++ bridge class to send a selector to a Cocoa object when the
// bookmark model changes.  Some Cocoa objects edit the bookmark model
// and temporarily save a copy of the state (e.g. bookmark button
// editor).  As a fail-safe, these objects want an easy cancel if the
// model changes out from under them.  For example, if you have the
// bookmark button editor sheet open, then edit the bookmark in the
// bookmark manager, we'd want to simply cancel the editor.
//
// This class is conservative and may result in notifications which
// aren't strictly necessary.  For example, node removal only needs to
// cancel an edit if the removed node is a folder (editors often have
// a list of "new parents").  But, just to be sure, notification
// happens on any removal.

#ifndef CHROME_BROWSER_UI_COCOA_BOOKMARKS_BOOKMARK_MODEL_OBSERVER_FOR_COCOA_H
#define CHROME_BROWSER_UI_COCOA_BOOKMARKS_BOOKMARK_MODEL_OBSERVER_FOR_COCOA_H

#import <Cocoa/Cocoa.h>

#include <set>

#include "base/basictypes.h"
#include "base/mac/scoped_block.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/bookmarks/browser/bookmark_model_observer.h"

class BookmarkModelObserverForCocoa : public BookmarkModelObserver {
 public:
  // Callback called on a significant model change. |nodeWasDeleted| will
  // be YES if an observed node was deleted in the change.
  typedef void(^ChangeCallback)(BOOL nodeWasDeleted);

  // When a |model| changes, or an observed node within it does, call a
  // |callback|.
  BookmarkModelObserverForCocoa(BookmarkModel* model,
                                ChangeCallback callback);
  virtual ~BookmarkModelObserverForCocoa();

  // Starts and stops observing a specified |node|; the node must be contained
  // within the model.
  void StartObservingNode(const BookmarkNode* node);
  void StopObservingNode(const BookmarkNode* node);

  // BookmarkModelObserver:
  virtual void BookmarkModelBeingDeleted(BookmarkModel* model) override;
  virtual void BookmarkNodeMoved(BookmarkModel* model,
                                 const BookmarkNode* old_parent,
                                 int old_index,
                                 const BookmarkNode* new_parent,
                                 int new_index) override;
  virtual void BookmarkNodeRemoved(BookmarkModel* model,
                                   const BookmarkNode* parent,
                                   int old_index,
                                   const BookmarkNode* node,
                                   const std::set<GURL>& removed_urls) override;
  virtual void BookmarkAllUserNodesRemoved(
      BookmarkModel* model,
      const std::set<GURL>& removed_urls) override;
  virtual void BookmarkNodeChanged(BookmarkModel* model,
                                   const BookmarkNode* node) override;

  // Some notifications we don't care about, but by being pure virtual
  // in the base class we must implement them.

  virtual void BookmarkModelLoaded(BookmarkModel* model,
                                   bool ids_reassigned) override {}
  virtual void BookmarkNodeAdded(BookmarkModel* model,
                                 const BookmarkNode* parent,
                                 int index) override {}
  virtual void BookmarkNodeFaviconChanged(BookmarkModel* model,
                                          const BookmarkNode* node) override {}
  virtual void BookmarkNodeChildrenReordered(
      BookmarkModel* model,
      const BookmarkNode* node) override {}

  virtual void ExtensiveBookmarkChangesBeginning(
      BookmarkModel* model) override {}

  virtual void ExtensiveBookmarkChangesEnded(BookmarkModel* model) override {}

 private:
  BookmarkModel* model_;  // Weak; it is owned by a Profile.
  std::set<const BookmarkNode*> nodes_;  // Weak items owned by a BookmarkModel.
  base::mac::ScopedBlock<ChangeCallback> callback_;

  // Send a notification to the client; |deleted| is YES if an observed node was
  // deleted in the change.
  void Notify(BOOL deleted);

  DISALLOW_COPY_AND_ASSIGN(BookmarkModelObserverForCocoa);
};

#endif  // CHROME_BROWSER_UI_COCOA_BOOKMARKS_BOOKMARK_MODEL_OBSERVER_FOR_COCOA_H
