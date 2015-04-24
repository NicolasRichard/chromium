// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/test/fake_picture_layer_impl.h"

#include <vector>
#include "cc/resources/tile.h"
#include "cc/trees/layer_tree_impl.h"

namespace cc {

FakePictureLayerImpl::FakePictureLayerImpl(
    LayerTreeImpl* tree_impl,
    int id,
    scoped_refptr<RasterSource> raster_source,
    bool is_mask)
    : PictureLayerImpl(tree_impl,
                       id,
                       is_mask,
                       new LayerImpl::SyncedScrollOffset),
      append_quads_count_(0),
      did_become_active_call_count_(0),
      has_valid_tile_priorities_(false),
      use_set_valid_tile_priorities_flag_(false),
      release_resources_count_(0) {
  SetBounds(raster_source->GetSize());
  SetContentBounds(raster_source->GetSize());
  SetRasterSourceOnPending(raster_source, Region());
}

FakePictureLayerImpl::FakePictureLayerImpl(
    LayerTreeImpl* tree_impl,
    int id,
    scoped_refptr<RasterSource> raster_source,
    bool is_mask,
    const gfx::Size& layer_bounds)
    : PictureLayerImpl(tree_impl,
                       id,
                       is_mask,
                       new LayerImpl::SyncedScrollOffset),
      append_quads_count_(0),
      did_become_active_call_count_(0),
      has_valid_tile_priorities_(false),
      use_set_valid_tile_priorities_flag_(false),
      release_resources_count_(0) {
  SetBounds(layer_bounds);
  SetContentBounds(layer_bounds);
  SetRasterSourceOnPending(raster_source, Region());
}

FakePictureLayerImpl::FakePictureLayerImpl(LayerTreeImpl* tree_impl,
                                           int id,
                                           bool is_mask)
    : FakePictureLayerImpl(tree_impl,
                           id,
                           is_mask,
                           new LayerImpl::SyncedScrollOffset) {
}

FakePictureLayerImpl::FakePictureLayerImpl(
    LayerTreeImpl* tree_impl,
    int id,
    bool is_mask,
    scoped_refptr<LayerImpl::SyncedScrollOffset> synced_scroll_offset)
    : PictureLayerImpl(tree_impl, id, is_mask, synced_scroll_offset),
      append_quads_count_(0),
      did_become_active_call_count_(0),
      has_valid_tile_priorities_(false),
      use_set_valid_tile_priorities_flag_(false),
      release_resources_count_(0) {
}

scoped_ptr<LayerImpl> FakePictureLayerImpl::CreateLayerImpl(
    LayerTreeImpl* tree_impl) {
  return make_scoped_ptr(new FakePictureLayerImpl(tree_impl, id(), is_mask_,
                                                  synced_scroll_offset()));
}

void FakePictureLayerImpl::PushPropertiesTo(LayerImpl* layer_impl) {
  FakePictureLayerImpl* picture_layer_impl =
      static_cast<FakePictureLayerImpl*>(layer_impl);
  picture_layer_impl->fixed_tile_size_ = fixed_tile_size_;
  PictureLayerImpl::PushPropertiesTo(layer_impl);
}

void FakePictureLayerImpl::AppendQuads(
    RenderPass* render_pass,
    AppendQuadsData* append_quads_data) {
  PictureLayerImpl::AppendQuads(render_pass, append_quads_data);
  ++append_quads_count_;
}

gfx::Size FakePictureLayerImpl::CalculateTileSize(
    const gfx::Size& content_bounds) const {
  if (fixed_tile_size_.IsEmpty()) {
    return PictureLayerImpl::CalculateTileSize(content_bounds);
  }

  return fixed_tile_size_;
}

PictureLayerTiling* FakePictureLayerImpl::HighResTiling() const {
  PictureLayerTiling* result = NULL;
  for (size_t i = 0; i < tilings_->num_tilings(); ++i) {
    PictureLayerTiling* tiling = tilings_->tiling_at(i);
    if (tiling->resolution() == HIGH_RESOLUTION) {
      // There should be only one high res tiling.
      CHECK(!result);
      result = tiling;
    }
  }
  return result;
}

PictureLayerTiling* FakePictureLayerImpl::LowResTiling() const {
  PictureLayerTiling* result = NULL;
  for (size_t i = 0; i < tilings_->num_tilings(); ++i) {
    PictureLayerTiling* tiling = tilings_->tiling_at(i);
    if (tiling->resolution() == LOW_RESOLUTION) {
      // There should be only one low res tiling.
      CHECK(!result);
      result = tiling;
    }
  }
  return result;
}

void FakePictureLayerImpl::SetRasterSourceOnPending(
    scoped_refptr<RasterSource> raster_source,
    const Region& invalidation) {
  DCHECK(layer_tree_impl()->IsPendingTree());
  Region invalidation_temp = invalidation;
  const PictureLayerTilingSet* pending_set = nullptr;
  set_gpu_raster_max_texture_size(layer_tree_impl()->device_viewport_size());
  UpdateRasterSource(raster_source, &invalidation_temp, pending_set);
}

void FakePictureLayerImpl::SetIsDrawnRenderSurfaceLayerListMember(bool is) {
  draw_properties().last_drawn_render_surface_layer_list_id =
      is ? layer_tree_impl()->current_render_surface_list_id()
         : layer_tree_impl()->current_render_surface_list_id() - 1;
}

void FakePictureLayerImpl::CreateAllTiles() {
  for (size_t i = 0; i < num_tilings(); ++i)
    tilings_->tiling_at(i)->CreateAllTilesForTesting();
}

void FakePictureLayerImpl::SetAllTilesVisible() {
  for (size_t tiling_idx = 0; tiling_idx < tilings_->num_tilings();
       ++tiling_idx) {
    PictureLayerTiling* tiling = tilings_->tiling_at(tiling_idx);
    std::vector<Tile*> tiles = tiling->AllTilesForTesting();
    for (size_t tile_idx = 0; tile_idx < tiles.size(); ++tile_idx) {
      Tile* tile = tiles[tile_idx];
      TilePriority priority;
      priority.resolution = HIGH_RESOLUTION;
      priority.priority_bin = TilePriority::NOW;
      priority.distance_to_visible = 0.f;
      tile->set_priority(priority);
    }
  }
}

void FakePictureLayerImpl::ResetAllTilesPriorities() {
  for (size_t tiling_idx = 0; tiling_idx < tilings_->num_tilings();
       ++tiling_idx) {
    PictureLayerTiling* tiling = tilings_->tiling_at(tiling_idx);
    std::vector<Tile*> tiles = tiling->AllTilesForTesting();
    for (size_t tile_idx = 0; tile_idx < tiles.size(); ++tile_idx) {
      Tile* tile = tiles[tile_idx];
      tile->set_priority(TilePriority());
    }
  }
}

void FakePictureLayerImpl::SetAllTilesReady() {
  for (size_t tiling_idx = 0; tiling_idx < tilings_->num_tilings();
       ++tiling_idx) {
    PictureLayerTiling* tiling = tilings_->tiling_at(tiling_idx);
    SetAllTilesReadyInTiling(tiling);
  }
}

void FakePictureLayerImpl::SetAllTilesReadyInTiling(
    PictureLayerTiling* tiling) {
  std::vector<Tile*> tiles = tiling->AllTilesForTesting();
  for (size_t tile_idx = 0; tile_idx < tiles.size(); ++tile_idx) {
    Tile* tile = tiles[tile_idx];
    SetTileReady(tile);
  }
}

void FakePictureLayerImpl::SetTileReady(Tile* tile) {
  TileDrawInfo& draw_info = tile->draw_info();
  draw_info.SetSolidColorForTesting(true);
  DCHECK(tile->IsReadyToDraw());
}

void FakePictureLayerImpl::DidBecomeActive() {
  PictureLayerImpl::DidBecomeActive();
  ++did_become_active_call_count_;
}

bool FakePictureLayerImpl::HasValidTilePriorities() const {
  return use_set_valid_tile_priorities_flag_
             ? has_valid_tile_priorities_
             : PictureLayerImpl::HasValidTilePriorities();
}

size_t FakePictureLayerImpl::CountTilesRequired(
    TileRequirementCheck is_tile_required_callback) const {
  if (!HasValidTilePriorities())
    return 0;

  if (!tilings_)
    return 0;

  if (visible_rect_for_tile_priority_.IsEmpty())
    return 0;

  gfx::Rect rect = viewport_rect_for_tile_priority_in_content_space_;
  rect.Intersect(visible_rect_for_tile_priority_);

  size_t count = 0;

  for (size_t i = 0; i < tilings_->num_tilings(); ++i) {
    PictureLayerTiling* tiling = tilings_->tiling_at(i);
    if (tiling->resolution() != HIGH_RESOLUTION &&
        tiling->resolution() != LOW_RESOLUTION)
      continue;

    for (PictureLayerTiling::CoverageIterator iter(tiling, 1.f, rect); iter;
         ++iter) {
      const Tile* tile = *iter;
      // A null tile (i.e. missing recording) can just be skipped.
      // TODO(vmpstr): Verify this is true if we create tiles in raster
      // iterators.
      if (!tile)
        continue;

      // We can't check tile->required_for_activation, because that value might
      // be out of date. It is updated in the raster/eviction iterators.
      // TODO(vmpstr): Remove the comment once you can't access this information
      // from the tile.
      if ((tiling->*is_tile_required_callback)(tile))
        ++count;
    }
  }

  return count;
}

size_t FakePictureLayerImpl::CountTilesRequiredForActivation() const {
  if (!layer_tree_impl()->IsPendingTree())
    return 0;

  return CountTilesRequired(&PictureLayerTiling::IsTileRequiredForActivation);
}

size_t FakePictureLayerImpl::CountTilesRequiredForDraw() const {
  if (!layer_tree_impl()->IsActiveTree())
    return 0;

  return CountTilesRequired(&PictureLayerTiling::IsTileRequiredForDraw);
}

void FakePictureLayerImpl::ReleaseResources() {
  PictureLayerImpl::ReleaseResources();
  ++release_resources_count_;
}

}  // namespace cc
