// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/mojo/services/mojo_media_client.h"

namespace media {

namespace internal {
extern scoped_ptr<PlatformMojoMediaClient> CreatePlatformMojoMediaClient();
}  // namespace internal

static base::LazyInstance<MojoMediaClient>::Leaky g_mojo_media_client =
    LAZY_INSTANCE_INITIALIZER;

// static
MojoMediaClient* MojoMediaClient::Get() {
  return g_mojo_media_client.Pointer();
}

scoped_ptr<RendererFactory> MojoMediaClient::GetRendererFactory(
    const scoped_refptr<MediaLog>& media_log) {
  return mojo_media_client_->GetRendererFactory(media_log);
}

ScopedVector<AudioDecoder> MojoMediaClient::GetAudioDecoders(
    const scoped_refptr<base::SingleThreadTaskRunner>& media_task_runner,
    const scoped_refptr<MediaLog>& media_log) {
  return mojo_media_client_->GetAudioDecoders(media_task_runner, media_log);
}

ScopedVector<VideoDecoder> MojoMediaClient::GetVideoDecoders(
    const scoped_refptr<base::SingleThreadTaskRunner>& media_task_runner,
    const scoped_refptr<MediaLog>& media_log) {
  return mojo_media_client_->GetVideoDecoders(media_task_runner, media_log);
}

scoped_refptr<AudioRendererSink> MojoMediaClient::GetAudioRendererSink() {
  return mojo_media_client_->GetAudioRendererSink();
}

scoped_ptr<VideoRendererSink> MojoMediaClient::GetVideoRendererSink(
    const scoped_refptr<base::SingleThreadTaskRunner>& task_runner) {
  return mojo_media_client_->GetVideoRendererSink(task_runner);
}

const AudioHardwareConfig& MojoMediaClient::GetAudioHardwareConfig() {
  return mojo_media_client_->GetAudioHardwareConfig();
}

scoped_ptr<CdmFactory> MojoMediaClient::GetCdmFactory() {
  return mojo_media_client_->GetCdmFactory();
}

MojoMediaClient::MojoMediaClient()
    : mojo_media_client_(internal::CreatePlatformMojoMediaClient().Pass()) {
}

MojoMediaClient::~MojoMediaClient() {
}

}  // namespace media
