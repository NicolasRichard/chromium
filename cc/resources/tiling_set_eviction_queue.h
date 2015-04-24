// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_RESOURCES_TILING_SET_EVICTION_QUEUE_H_
#define CC_RESOURCES_TILING_SET_EVICTION_QUEUE_H_

#include <vector>

#include "cc/base/cc_export.h"
#include "cc/resources/picture_layer_tiling_set.h"

namespace cc {

// This eviction queue returned tiles from all tilings in a tiling set in
// the order in which the tiles should be evicted. It can be thought of as the
// following:
//  for all phases:
//    for all ordered tilings:
//      yield the next tile for the given phase from the given tiling
//
// Phases are the following (in order in which they are processed):
// EVENTUALLY_RECT - Tiles in the eventually region of the tiling.
// SOON_BORDER_RECT - Tiles in the prepainting skirt of the tiling.
// SKEWPORT_RECT - Tiles in the skewport of the tiling.
// PENDING_VISIBLE_RECT - Tiles that will be visible upon activation, not
//     required for activation.
// PENDING_VISIBLE_RECT_REQUIRED_FOR_ACTIVATION - Tiles that will be visible
//     upon activation, required for activation.
// VISIBLE_RECT_OCCLUDED - Occluded, not required for activation, visible tiles.
// VISIBLE_RECT_UNOCCLUDED - Unoccluded, not required for activation, visible
//     tiles.
// VISIBLE_RECT_REQUIRED_FOR_ACTIVATION_OCCLUDED - Occluded, but required for
//     activation, visible tiles. This can happen when an active tree tile is
//     occluded, but is not occluded on the pending tree (and is required for
//     activation).
// VISIBLE_RECT_REQUIRED_FOR_ACTIVATION_UNOCCLUDED - Unoccluded, required for
//     activation, tiles.
//
// The tilings are ordered as follows. Suppose we have tilings with the scales
// below:
// 2.0   1.5   1.0(HR)   0.8   0.5   0.25(LR)   0.2   0.1
// With HR referring to high res tiling and LR referring to low res tiling,
// then tilings are processed in this order:
// 2.0   1.5   0.1   0.2   0.5   0.8   0.25(LR)   1.0(HR).
//
// To put it differently:
//  1. Process the highest scale tiling down to, but not including, high res
//     tiling.
//  2. Process the lowest scale tiling up to, but not including, the low res
//     tiling. In cases without a low res tiling, this is an empty set.
//  3. Process low res tiling up to high res tiling, including neither high
//     nor low res tilings. In cases without a low res tiling, this set
//     includes all tilings with a lower scale than the high res tiling.
//  4. Process the low res tiling.
//  5. Process the high res tiling.
//
// Additional notes:
// Since eventually the tiles are considered to have the priority which is the
// higher of the two trees, we might visit a tile that should actually be
// returned by its twin. In those situations, the tiles are not returned. That
// is, since the twin has higher priority, it should return it when it gets to
// it. This ensures that we don't block raster because we've returned a tile
// with low priority on one tree, but high combined priority.
class CC_EXPORT TilingSetEvictionQueue {
 public:
  TilingSetEvictionQueue(PictureLayerTilingSet* tiling_set,
                         bool skip_shared_out_of_order_tiles);
  ~TilingSetEvictionQueue();

  Tile* Top();
  const Tile* Top() const;
  void Pop();
  bool IsEmpty() const;

 private:
  enum Phase {
    EVENTUALLY_RECT,
    SOON_BORDER_RECT,
    SKEWPORT_RECT,
    PENDING_VISIBLE_RECT,
    PENDING_VISIBLE_RECT_REQUIRED_FOR_ACTIVATION,
    VISIBLE_RECT_OCCLUDED,
    VISIBLE_RECT_UNOCCLUDED,
    VISIBLE_RECT_REQUIRED_FOR_ACTIVATION_OCCLUDED,
    VISIBLE_RECT_REQUIRED_FOR_ACTIVATION_UNOCCLUDED
  };

  void GenerateTilingOrder(PictureLayerTilingSet* tiling_set);

  // Helper base class for individual region iterators.
  class EvictionRectIterator {
   public:
    EvictionRectIterator();
    EvictionRectIterator(std::vector<PictureLayerTiling*>* tilings,
                         WhichTree tree,
                         bool skip_shared_out_of_order_tiles,
                         bool skip_pending_visible_rect);

    bool done() const { return !tile_; }
    Tile* operator*() const { return tile_; }

   protected:
    ~EvictionRectIterator() = default;

    template <typename TilingIteratorType>
    bool AdvanceToNextTile(TilingIteratorType* iterator);
    template <typename TilingIteratorType>
    bool GetFirstTileAndCheckIfValid(TilingIteratorType* iterator);

    Tile* tile_;
    std::vector<PictureLayerTiling*>* tilings_;
    WhichTree tree_;
    bool skip_shared_out_of_order_tiles_;
    bool skip_pending_visible_rect_;
    size_t tiling_index_;
  };

  class PendingVisibleTilingIterator : public EvictionRectIterator {
   public:
    PendingVisibleTilingIterator() = default;
    PendingVisibleTilingIterator(std::vector<PictureLayerTiling*>* tilings,
                                 WhichTree tree,
                                 bool skip_shared_out_of_order_tiles,
                                 bool return_required_for_activation_tiles);

    PendingVisibleTilingIterator& operator++();

   private:
    bool TileMatchesRequiredFlags(const Tile* tile) const;

    TilingData::DifferenceIterator iterator_;
    bool return_required_for_activation_tiles_;
  };

  class VisibleTilingIterator : public EvictionRectIterator {
   public:
    VisibleTilingIterator() = default;
    VisibleTilingIterator(std::vector<PictureLayerTiling*>* tilings,
                          WhichTree tree,
                          bool skip_shared_out_of_order_tiles,
                          bool return_occluded_tiles,
                          bool return_required_for_activation_tiles);

    VisibleTilingIterator& operator++();

   private:
    bool TileMatchesRequiredFlags(const Tile* tile) const;

    TilingData::Iterator iterator_;
    bool return_occluded_tiles_;
    bool return_required_for_activation_tiles_;
  };

  class SkewportTilingIterator : public EvictionRectIterator {
   public:
    SkewportTilingIterator() = default;
    SkewportTilingIterator(std::vector<PictureLayerTiling*>* tilings,
                           WhichTree tree,
                           bool skip_shared_out_of_order_tiles);

    SkewportTilingIterator& operator++();

   private:
    TilingData::ReverseSpiralDifferenceIterator iterator_;
  };

  class SoonBorderTilingIterator : public EvictionRectIterator {
   public:
    SoonBorderTilingIterator() = default;
    SoonBorderTilingIterator(std::vector<PictureLayerTiling*>* tilings,
                             WhichTree tree,
                             bool skip_shared_out_of_order_tiles);

    SoonBorderTilingIterator& operator++();

   private:
    TilingData::ReverseSpiralDifferenceIterator iterator_;
  };

  class EventuallyTilingIterator : public EvictionRectIterator {
   public:
    EventuallyTilingIterator() = default;
    EventuallyTilingIterator(std::vector<PictureLayerTiling*>* tilings,
                             WhichTree tree,
                             bool skip_shared_out_of_order_tiles);

    EventuallyTilingIterator& operator++();

   private:
    TilingData::ReverseSpiralDifferenceIterator iterator_;
  };

  void AdvancePhase();

  WhichTree tree_;
  bool skip_shared_out_of_order_tiles_;
  Phase phase_;
  Tile* current_tile_;
  std::vector<PictureLayerTiling*> tilings_;

  EventuallyTilingIterator eventually_iterator_;
  SoonBorderTilingIterator soon_iterator_;
  SkewportTilingIterator skewport_iterator_;
  PendingVisibleTilingIterator pending_visible_iterator_;
  VisibleTilingIterator visible_iterator_;
};

}  // namespace cc

#endif  // CC_RESOURCES_TILING_SET_EVICTION_QUEUE_H_
