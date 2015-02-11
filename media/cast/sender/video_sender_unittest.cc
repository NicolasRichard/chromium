// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <vector>

#include "base/bind.h"
#include "base/memory/scoped_ptr.h"
#include "base/test/simple_test_tick_clock.h"
#include "media/base/video_frame.h"
#include "media/cast/cast_environment.h"
#include "media/cast/logging/simple_event_subscriber.h"
#include "media/cast/net/cast_transport_config.h"
#include "media/cast/net/cast_transport_sender_impl.h"
#include "media/cast/net/pacing/paced_sender.h"
#include "media/cast/sender/fake_video_encode_accelerator_factory.h"
#include "media/cast/sender/video_frame_factory.h"
#include "media/cast/sender/video_sender.h"
#include "media/cast/test/fake_single_thread_task_runner.h"
#include "media/cast/test/utility/default_config.h"
#include "media/cast/test/utility/video_utility.h"
#include "media/video/fake_video_encode_accelerator.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace media {
namespace cast {

namespace {
static const uint8 kPixelValue = 123;
static const int kWidth = 320;
static const int kHeight = 240;

using testing::_;
using testing::AtLeast;


void SaveOperationalStatus(OperationalStatus* out_status,
                           OperationalStatus in_status) {
  DVLOG(1) << "OperationalStatus transitioning from " << *out_status << " to "
           << in_status;
  *out_status = in_status;
}

class TestPacketSender : public PacketSender {
 public:
  TestPacketSender()
      : number_of_rtp_packets_(0),
        number_of_rtcp_packets_(0),
        paused_(false) {}

  // A singular packet implies a RTCP packet.
  bool SendPacket(PacketRef packet, const base::Closure& cb) override {
    if (paused_) {
      stored_packet_ = packet;
      callback_ = cb;
      return false;
    }
    if (Rtcp::IsRtcpPacket(&packet->data[0], packet->data.size())) {
      ++number_of_rtcp_packets_;
    } else {
      // Check that at least one RTCP packet was sent before the first RTP
      // packet.  This confirms that the receiver will have the necessary lip
      // sync info before it has to calculate the playout time of the first
      // frame.
      if (number_of_rtp_packets_ == 0)
        EXPECT_LE(1, number_of_rtcp_packets_);
      ++number_of_rtp_packets_;
    }
    return true;
  }

  int64 GetBytesSent() override { return 0; }

  int number_of_rtp_packets() const { return number_of_rtp_packets_; }

  int number_of_rtcp_packets() const { return number_of_rtcp_packets_; }

  void SetPause(bool paused) {
    paused_ = paused;
    if (!paused && stored_packet_.get()) {
      SendPacket(stored_packet_, callback_);
      callback_.Run();
    }
  }

 private:
  int number_of_rtp_packets_;
  int number_of_rtcp_packets_;
  bool paused_;
  base::Closure callback_;
  PacketRef stored_packet_;

  DISALLOW_COPY_AND_ASSIGN(TestPacketSender);
};

void IgnorePlayoutDelayChanges(base::TimeDelta unused_playout_delay) {
}

class PeerVideoSender : public VideoSender {
 public:
  PeerVideoSender(
      scoped_refptr<CastEnvironment> cast_environment,
      const VideoSenderConfig& video_config,
      const StatusChangeCallback& status_change_cb,
      const CreateVideoEncodeAcceleratorCallback& create_vea_cb,
      const CreateVideoEncodeMemoryCallback& create_video_encode_mem_cb,
      CastTransportSender* const transport_sender)
      : VideoSender(cast_environment,
                    video_config,
                    status_change_cb,
                    create_vea_cb,
                    create_video_encode_mem_cb,
                    transport_sender,
                    base::Bind(&IgnorePlayoutDelayChanges)) {}
  using VideoSender::OnReceivedCastFeedback;
};
}  // namespace

class VideoSenderTest : public ::testing::Test {
 protected:
  VideoSenderTest()
      : testing_clock_(new base::SimpleTestTickClock()),
        task_runner_(new test::FakeSingleThreadTaskRunner(testing_clock_)),
        cast_environment_(new CastEnvironment(
            scoped_ptr<base::TickClock>(testing_clock_).Pass(),
            task_runner_,
            task_runner_,
            task_runner_)),
        operational_status_(STATUS_UNINITIALIZED),
        vea_factory_(task_runner_) {
    testing_clock_->Advance(base::TimeTicks::Now() - base::TimeTicks());
    vea_factory_.SetAutoRespond(true);
    last_pixel_value_ = kPixelValue;
    net::IPEndPoint dummy_endpoint;
    transport_sender_.reset(new CastTransportSenderImpl(
        NULL,
        testing_clock_,
        dummy_endpoint,
        dummy_endpoint,
        make_scoped_ptr(new base::DictionaryValue),
        base::Bind(&UpdateCastTransportStatus),
        BulkRawEventsCallback(),
        base::TimeDelta(),
        task_runner_,
        PacketReceiverCallback(),
        &transport_));
  }

  ~VideoSenderTest() override {}

  void TearDown() override {
    video_sender_.reset();
    task_runner_->RunTasks();
  }

  static void UpdateCastTransportStatus(CastTransportStatus status) {
    EXPECT_EQ(TRANSPORT_VIDEO_INITIALIZED, status);
  }

  // If |external| is true then external video encoder (VEA) is used.
  // |expect_init_sucess| is true if initialization is expected to succeed.
  void InitEncoder(bool external, bool expect_init_success) {
    VideoSenderConfig video_config;
    video_config.ssrc = 1;
    video_config.receiver_ssrc = 2;
    video_config.rtp_payload_type = 127;
    video_config.use_external_encoder = external;
    video_config.max_bitrate = 5000000;
    video_config.min_bitrate = 1000000;
    video_config.start_bitrate = 1000000;
    video_config.max_qp = 56;
    video_config.min_qp = 0;
    video_config.max_frame_rate = 30;
    video_config.max_number_of_video_buffers_used = 1;
    video_config.codec = CODEC_VIDEO_VP8;

    ASSERT_EQ(operational_status_, STATUS_UNINITIALIZED);

    if (external) {
      vea_factory_.SetInitializationWillSucceed(expect_init_success);
      video_sender_.reset(new PeerVideoSender(
          cast_environment_,
          video_config,
          base::Bind(&SaveOperationalStatus, &operational_status_),
          base::Bind(
              &FakeVideoEncodeAcceleratorFactory::CreateVideoEncodeAccelerator,
              base::Unretained(&vea_factory_)),
          base::Bind(&FakeVideoEncodeAcceleratorFactory::CreateSharedMemory,
                     base::Unretained(&vea_factory_)),
          transport_sender_.get()));
    } else {
      video_sender_.reset(new PeerVideoSender(
          cast_environment_,
          video_config,
          base::Bind(&SaveOperationalStatus, &operational_status_),
          CreateDefaultVideoEncodeAcceleratorCallback(),
          CreateDefaultVideoEncodeMemoryCallback(),
          transport_sender_.get()));
    }
    task_runner_->RunTasks();
  }

  scoped_refptr<media::VideoFrame> GetNewVideoFrame() {
    if (first_frame_timestamp_.is_null())
      first_frame_timestamp_ = testing_clock_->NowTicks();
    gfx::Size size(kWidth, kHeight);
    scoped_refptr<media::VideoFrame> video_frame =
        media::VideoFrame::CreateFrame(
            VideoFrame::I420, size, gfx::Rect(size), size,
            testing_clock_->NowTicks() - first_frame_timestamp_);
    PopulateVideoFrame(video_frame.get(), last_pixel_value_++);
    return video_frame;
  }

  scoped_refptr<media::VideoFrame> GetLargeNewVideoFrame() {
    if (first_frame_timestamp_.is_null())
      first_frame_timestamp_ = testing_clock_->NowTicks();
    gfx::Size size(kWidth, kHeight);
    scoped_refptr<media::VideoFrame> video_frame =
        media::VideoFrame::CreateFrame(
            VideoFrame::I420, size, gfx::Rect(size), size,
            testing_clock_->NowTicks() - first_frame_timestamp_);
    PopulateVideoFrameWithNoise(video_frame.get());
    return video_frame;
  }

  void RunTasks(int during_ms) {
    task_runner_->Sleep(base::TimeDelta::FromMilliseconds(during_ms));
  }

  base::SimpleTestTickClock* const testing_clock_;  // Owned by CastEnvironment.
  const scoped_refptr<test::FakeSingleThreadTaskRunner> task_runner_;
  const scoped_refptr<CastEnvironment> cast_environment_;
  OperationalStatus operational_status_;
  FakeVideoEncodeAcceleratorFactory vea_factory_;
  TestPacketSender transport_;
  scoped_ptr<CastTransportSenderImpl> transport_sender_;
  scoped_ptr<PeerVideoSender> video_sender_;
  int last_pixel_value_;
  base::TimeTicks first_frame_timestamp_;

  DISALLOW_COPY_AND_ASSIGN(VideoSenderTest);
};

TEST_F(VideoSenderTest, BuiltInEncoder) {
  InitEncoder(false, true);
  ASSERT_EQ(STATUS_INITIALIZED, operational_status_);

  scoped_refptr<media::VideoFrame> video_frame = GetNewVideoFrame();

  const base::TimeTicks reference_time = testing_clock_->NowTicks();
  video_sender_->InsertRawVideoFrame(video_frame, reference_time);

  task_runner_->RunTasks();
  EXPECT_LE(1, transport_.number_of_rtp_packets());
  EXPECT_LE(1, transport_.number_of_rtcp_packets());
}

TEST_F(VideoSenderTest, ExternalEncoder) {
  InitEncoder(true, true);
  ASSERT_EQ(STATUS_INITIALIZED, operational_status_);

  // The SizeAdaptableExternalVideoEncoder initally reports STATUS_INITIALIZED
  // so that frames will be sent to it.  Therefore, no encoder activity should
  // have occurred at this point.  Send a frame to spurn creation of the
  // underlying ExternalVideoEncoder instance.
  if (vea_factory_.vea_response_count() == 0) {
    video_sender_->InsertRawVideoFrame(GetNewVideoFrame(),
                                       testing_clock_->NowTicks());
    task_runner_->RunTasks();
  }
  ASSERT_EQ(STATUS_INITIALIZED, operational_status_);
  RunTasks(33);

  // VideoSender created an encoder for 1280x720 frames, in order to provide the
  // INITIALIZED status.
  EXPECT_EQ(1, vea_factory_.vea_response_count());
  EXPECT_EQ(3, vea_factory_.shm_response_count());

  scoped_refptr<media::VideoFrame> video_frame = GetNewVideoFrame();

  for (int i = 0; i < 3; ++i) {
    const base::TimeTicks reference_time = testing_clock_->NowTicks();
    video_sender_->InsertRawVideoFrame(video_frame, reference_time);
    RunTasks(33);
    // VideoSender re-created the encoder for the 320x240 frames we're
    // providing.
    EXPECT_EQ(1, vea_factory_.vea_response_count());
    EXPECT_EQ(3, vea_factory_.shm_response_count());
  }

  video_sender_.reset(NULL);
  task_runner_->RunTasks();
  EXPECT_EQ(1, vea_factory_.vea_response_count());
  EXPECT_EQ(3, vea_factory_.shm_response_count());
}

TEST_F(VideoSenderTest, ExternalEncoderInitFails) {
  InitEncoder(true, false);

  // The SizeAdaptableExternalVideoEncoder initally reports STATUS_INITIALIZED
  // so that frames will be sent to it.  Send a frame to spurn creation of the
  // underlying ExternalVideoEncoder instance, which should result in failure.
  if (operational_status_ == STATUS_INITIALIZED ||
      operational_status_ == STATUS_CODEC_REINIT_PENDING) {
    video_sender_->InsertRawVideoFrame(GetNewVideoFrame(),
                                       testing_clock_->NowTicks());
    task_runner_->RunTasks();
  }
  EXPECT_EQ(STATUS_CODEC_INIT_FAILED, operational_status_);

  video_sender_.reset(NULL);
  task_runner_->RunTasks();
}

TEST_F(VideoSenderTest, RtcpTimer) {
  InitEncoder(false, true);
  ASSERT_EQ(STATUS_INITIALIZED, operational_status_);

  scoped_refptr<media::VideoFrame> video_frame = GetNewVideoFrame();

  const base::TimeTicks reference_time = testing_clock_->NowTicks();
  video_sender_->InsertRawVideoFrame(video_frame, reference_time);

  // Make sure that we send at least one RTCP packet.
  base::TimeDelta max_rtcp_timeout =
      base::TimeDelta::FromMilliseconds(1 + kDefaultRtcpIntervalMs * 3 / 2);

  RunTasks(max_rtcp_timeout.InMilliseconds());
  EXPECT_LE(1, transport_.number_of_rtp_packets());
  EXPECT_LE(1, transport_.number_of_rtcp_packets());
  // Build Cast msg and expect RTCP packet.
  RtcpCastMessage cast_feedback(1);
  cast_feedback.media_ssrc = 2;
  cast_feedback.ack_frame_id = 0;
  video_sender_->OnReceivedCastFeedback(cast_feedback);
  RunTasks(max_rtcp_timeout.InMilliseconds());
  EXPECT_LE(1, transport_.number_of_rtcp_packets());
}

TEST_F(VideoSenderTest, ResendTimer) {
  InitEncoder(false, true);
  ASSERT_EQ(STATUS_INITIALIZED, operational_status_);

  scoped_refptr<media::VideoFrame> video_frame = GetNewVideoFrame();

  const base::TimeTicks reference_time = testing_clock_->NowTicks();
  video_sender_->InsertRawVideoFrame(video_frame, reference_time);

  // ACK the key frame.
  RtcpCastMessage cast_feedback(1);
  cast_feedback.media_ssrc = 2;
  cast_feedback.ack_frame_id = 0;
  video_sender_->OnReceivedCastFeedback(cast_feedback);

  video_frame = GetNewVideoFrame();
  video_sender_->InsertRawVideoFrame(video_frame, reference_time);

  base::TimeDelta max_resend_timeout =
      base::TimeDelta::FromMilliseconds(1 + kDefaultRtpMaxDelayMs);

  // Make sure that we do a re-send.
  RunTasks(max_resend_timeout.InMilliseconds());
  // Should have sent at least 3 packets.
  EXPECT_LE(
      3,
      transport_.number_of_rtp_packets() + transport_.number_of_rtcp_packets());
}

TEST_F(VideoSenderTest, LogAckReceivedEvent) {
  InitEncoder(false, true);
  ASSERT_EQ(STATUS_INITIALIZED, operational_status_);

  SimpleEventSubscriber event_subscriber;
  cast_environment_->Logging()->AddRawEventSubscriber(&event_subscriber);

  int num_frames = 10;
  for (int i = 0; i < num_frames; i++) {
    scoped_refptr<media::VideoFrame> video_frame = GetNewVideoFrame();

    const base::TimeTicks reference_time = testing_clock_->NowTicks();
    video_sender_->InsertRawVideoFrame(video_frame, reference_time);
    RunTasks(33);
  }

  task_runner_->RunTasks();

  RtcpCastMessage cast_feedback(1);
  cast_feedback.ack_frame_id = num_frames - 1;

  video_sender_->OnReceivedCastFeedback(cast_feedback);

  std::vector<FrameEvent> frame_events;
  event_subscriber.GetFrameEventsAndReset(&frame_events);

  ASSERT_TRUE(!frame_events.empty());
  EXPECT_EQ(FRAME_ACK_RECEIVED, frame_events.rbegin()->type);
  EXPECT_EQ(VIDEO_EVENT, frame_events.rbegin()->media_type);
  EXPECT_EQ(num_frames - 1u, frame_events.rbegin()->frame_id);

  cast_environment_->Logging()->RemoveRawEventSubscriber(&event_subscriber);
}

TEST_F(VideoSenderTest, StopSendingInTheAbsenceOfAck) {
  InitEncoder(false, true);
  ASSERT_EQ(STATUS_INITIALIZED, operational_status_);

  // Send a stream of frames and don't ACK; by default we shouldn't have more
  // than 4 frames in flight.
  scoped_refptr<media::VideoFrame> video_frame = GetNewVideoFrame();
  video_sender_->InsertRawVideoFrame(video_frame, testing_clock_->NowTicks());
  RunTasks(33);

  // Send 3 more frames and record the number of packets sent.
  for (int i = 0; i < 3; ++i) {
    scoped_refptr<media::VideoFrame> video_frame = GetNewVideoFrame();
    video_sender_->InsertRawVideoFrame(video_frame, testing_clock_->NowTicks());
    RunTasks(33);
  }
  const int number_of_packets_sent = transport_.number_of_rtp_packets();

  // Send 3 more frames - they should not be encoded, as we have not received
  // any acks.
  for (int i = 0; i < 3; ++i) {
    scoped_refptr<media::VideoFrame> video_frame = GetNewVideoFrame();
    video_sender_->InsertRawVideoFrame(video_frame, testing_clock_->NowTicks());
    RunTasks(33);
  }

  // We expect a frame to be retransmitted because of duplicated ACKs.
  // Only one packet of the frame is re-transmitted.
  EXPECT_EQ(number_of_packets_sent + 1,
            transport_.number_of_rtp_packets());

  // Start acking and make sure we're back to steady-state.
  RtcpCastMessage cast_feedback(1);
  cast_feedback.media_ssrc = 2;
  cast_feedback.ack_frame_id = 0;
  video_sender_->OnReceivedCastFeedback(cast_feedback);
  EXPECT_LE(
      4,
      transport_.number_of_rtp_packets() + transport_.number_of_rtcp_packets());

  // Empty the pipeline.
  RunTasks(100);
  // Should have sent at least 7 packets.
  EXPECT_LE(
      7,
      transport_.number_of_rtp_packets() + transport_.number_of_rtcp_packets());
}

TEST_F(VideoSenderTest, DuplicateAckRetransmit) {
  InitEncoder(false, true);
  ASSERT_EQ(STATUS_INITIALIZED, operational_status_);

  scoped_refptr<media::VideoFrame> video_frame = GetNewVideoFrame();
  video_sender_->InsertRawVideoFrame(video_frame, testing_clock_->NowTicks());
  RunTasks(33);
  RtcpCastMessage cast_feedback(1);
  cast_feedback.media_ssrc = 2;
  cast_feedback.ack_frame_id = 0;

  // Send 3 more frames but don't ACK.
  for (int i = 0; i < 3; ++i) {
    scoped_refptr<media::VideoFrame> video_frame = GetNewVideoFrame();
    video_sender_->InsertRawVideoFrame(video_frame, testing_clock_->NowTicks());
    RunTasks(33);
  }
  const int number_of_packets_sent = transport_.number_of_rtp_packets();

  // Send duplicated ACKs and mix some invalid NACKs.
  for (int i = 0; i < 10; ++i) {
    RtcpCastMessage ack_feedback(1);
    ack_feedback.media_ssrc = 2;
    ack_feedback.ack_frame_id = 0;
    RtcpCastMessage nack_feedback(1);
    nack_feedback.media_ssrc = 2;
    nack_feedback.missing_frames_and_packets[255] = PacketIdSet();
    video_sender_->OnReceivedCastFeedback(ack_feedback);
    video_sender_->OnReceivedCastFeedback(nack_feedback);
  }
  EXPECT_EQ(number_of_packets_sent, transport_.number_of_rtp_packets());

  // Re-transmit one packet because of duplicated ACKs.
  for (int i = 0; i < 3; ++i) {
    RtcpCastMessage ack_feedback(1);
    ack_feedback.media_ssrc = 2;
    ack_feedback.ack_frame_id = 0;
    video_sender_->OnReceivedCastFeedback(ack_feedback);
  }
  EXPECT_EQ(number_of_packets_sent + 1, transport_.number_of_rtp_packets());
}

TEST_F(VideoSenderTest, DuplicateAckRetransmitDoesNotCancelRetransmits) {
  InitEncoder(false, true);
  ASSERT_EQ(STATUS_INITIALIZED, operational_status_);

  scoped_refptr<media::VideoFrame> video_frame = GetNewVideoFrame();
  video_sender_->InsertRawVideoFrame(video_frame, testing_clock_->NowTicks());
  RunTasks(33);
  RtcpCastMessage cast_feedback(1);
  cast_feedback.media_ssrc = 2;
  cast_feedback.ack_frame_id = 0;

  // Send 2 more frames but don't ACK.
  for (int i = 0; i < 2; ++i) {
    scoped_refptr<media::VideoFrame> video_frame = GetNewVideoFrame();
    video_sender_->InsertRawVideoFrame(video_frame, testing_clock_->NowTicks());
    RunTasks(33);
  }
  // Pause the transport
  transport_.SetPause(true);

  // Insert one more video frame.
  video_frame = GetLargeNewVideoFrame();
  video_sender_->InsertRawVideoFrame(video_frame, testing_clock_->NowTicks());
  RunTasks(33);

  const int number_of_packets_sent = transport_.number_of_rtp_packets();

  // Send duplicated ACKs and mix some invalid NACKs.
  for (int i = 0; i < 10; ++i) {
    RtcpCastMessage ack_feedback(1);
    ack_feedback.media_ssrc = 2;
    ack_feedback.ack_frame_id = 0;
    RtcpCastMessage nack_feedback(1);
    nack_feedback.media_ssrc = 2;
    nack_feedback.missing_frames_and_packets[255] = PacketIdSet();
    video_sender_->OnReceivedCastFeedback(ack_feedback);
    video_sender_->OnReceivedCastFeedback(nack_feedback);
  }
  EXPECT_EQ(number_of_packets_sent, transport_.number_of_rtp_packets());

  // Re-transmit one packet because of duplicated ACKs.
  for (int i = 0; i < 3; ++i) {
    RtcpCastMessage ack_feedback(1);
    ack_feedback.media_ssrc = 2;
    ack_feedback.ack_frame_id = 0;
    video_sender_->OnReceivedCastFeedback(ack_feedback);
  }

  transport_.SetPause(false);
  RunTasks(100);
  EXPECT_LT(number_of_packets_sent + 1, transport_.number_of_rtp_packets());
}

TEST_F(VideoSenderTest, AcksCancelRetransmits) {
  InitEncoder(false, true);
  ASSERT_EQ(STATUS_INITIALIZED, operational_status_);

  transport_.SetPause(true);
  scoped_refptr<media::VideoFrame> video_frame = GetLargeNewVideoFrame();
  video_sender_->InsertRawVideoFrame(video_frame, testing_clock_->NowTicks());
  RunTasks(33);

  // Frame should be in buffer, waiting. Now let's ack it.
  RtcpCastMessage cast_feedback(1);
  cast_feedback.media_ssrc = 2;
  cast_feedback.ack_frame_id = 0;
  video_sender_->OnReceivedCastFeedback(cast_feedback);

  transport_.SetPause(false);
  RunTasks(33);
  EXPECT_EQ(0, transport_.number_of_rtp_packets());
}

TEST_F(VideoSenderTest, CheckVideoFrameFactoryIsNull) {
  InitEncoder(false, true);
  ASSERT_EQ(STATUS_INITIALIZED, operational_status_);

  EXPECT_EQ(nullptr, video_sender_->CreateVideoFrameFactory().get());
}

}  // namespace cast
}  // namespace media
