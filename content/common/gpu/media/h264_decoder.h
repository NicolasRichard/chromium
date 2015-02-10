// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_GPU_MEDIA_H264_DECODER_H_
#define CONTENT_COMMON_GPU_MEDIA_H264_DECODER_H_

#include <vector>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop_proxy.h"
#include "content/common/content_export.h"
#include "content/common/gpu/media/accelerated_video_decoder.h"
#include "content/common/gpu/media/h264_dpb.h"
#include "media/base/limits.h"
#include "media/filters/h264_parser.h"
#include "ui/gfx/geometry/size.h"

namespace content {

// Clients of this class are expected to pass H264 Annex-B byte stream
// and are expected to provide an implementation of H264Accelerator for
// offloading final steps of the decoding process.
//
// This class must be created, called and destroyed on a single thread, and
// does nothing internally on any other thread.
class CONTENT_EXPORT H264Decoder : public AcceleratedVideoDecoder {
 public:
  class CONTENT_EXPORT H264Accelerator {
   public:
    H264Accelerator();
    virtual ~H264Accelerator();

    // Create a new H264Picture that the decoder client can use for decoding
    // and pass back to this accelerator for decoding or reference.
    // When the picture is no longer needed by decoder, it will just drop
    // its reference to it, and it may do so at any time.
    // Note that this may return nullptr if accelerator is not able to provide
    // any new pictures at given time. The decoder is expected to handle
    // this situation as normal and return from Decode() with kRanOutOfSurfaces.
    virtual scoped_refptr<H264Picture> CreateH264Picture() = 0;

    // Submit metadata for the current frame, providing the current |sps| and
    // |pps| for it, |dpb| has to contain all the pictures in DPB for current
    // frame, and |ref_pic_p0/b0/b1| as specified in the H264 spec. Note that
    // depending on the frame type, either p0, or b0 and b1 are used. |pic|
    // contains information about the picture for the current frame.
    // Note that this does not run decode in the accelerator and the decoder
    // is expected to follow this call with one or more SubmitSlice() calls
    // before calling SubmitDecode().
    // Return true if successful.
    virtual bool SubmitFrameMetadata(const media::H264SPS* sps,
                                     const media::H264PPS* pps,
                                     const H264DPB& dpb,
                                     const H264Picture::Vector& ref_pic_listp0,
                                     const H264Picture::Vector& ref_pic_listb0,
                                     const H264Picture::Vector& ref_pic_listb1,
                                     const scoped_refptr<H264Picture>& pic) = 0;

    // Submit one slice for the current frame, passing the current |pps| and
    // |pic| (same as in SubmitFrameMetadata()), the parsed header for the
    // current slice in |slice_hdr|, and the reordered |ref_pic_listX|,
    // as per H264 spec.
    // |data| pointing to the full slice (including the unparsed header| of
    // |size| in bytes.
    // This must be called one or more times per frame, before SubmitDecode().
    // Note that |data| does not have to remain valid after this call returns.
    // Return true if successful.
    virtual bool SubmitSlice(const media::H264PPS* pps,
                             const media::H264SliceHeader* slice_hdr,
                             const H264Picture::Vector& ref_pic_list0,
                             const H264Picture::Vector& ref_pic_list1,
                             const scoped_refptr<H264Picture>& pic,
                             const uint8_t* data,
                             size_t size) = 0;

    // Execute the decode in hardware for |pic|, using all the slices and
    // metadata submitted via SubmitFrameMetadata() and SubmitSlice() since
    // the previous call to SubmitDecode().
    // Return true if successful.
    virtual bool SubmitDecode(const scoped_refptr<H264Picture>& pic) = 0;

    // Schedule output (display) of |pic|. Note that returning from this
    // method does not mean that |pic| has already been outputted (displayed),
    // but guarantees that all pictures will be outputted in the same order
    // as this method was called for them. Decoder may drop its reference
    // to |pic| after calling this method.
    // Return true if successful.
    virtual bool OutputPicture(const scoped_refptr<H264Picture>& pic) = 0;

    // Reset any current state that may be cached in the accelerator, dropping
    // any cached parameters/slices that have not been committed yet.
    virtual void Reset() = 0;

   private:
    DISALLOW_COPY_AND_ASSIGN(H264Accelerator);
  };

  H264Decoder(H264Accelerator* accelerator);
  ~H264Decoder() override;

  // content::AcceleratedVideoDecoder implementation.
  bool Flush() override WARN_UNUSED_RESULT;
  void Reset() override;
  void SetStream(const uint8_t* ptr, size_t size) override;
  DecodeResult Decode() override WARN_UNUSED_RESULT;
  gfx::Size GetPicSize() const override { return pic_size_; }
  size_t GetRequiredNumOfPictures() const override;

 private:
  // We need to keep at most kDPBMaxSize pictures in DPB for
  // reference/to display later and an additional one for the one currently
  // being decoded. We also ask for some additional ones since VDA needs
  // to accumulate a few ready-to-output pictures before it actually starts
  // displaying and giving them back. +2 instead of +1 because of subjective
  // smoothness improvement during testing.
  enum {
    kPicsInPipeline = media::limits::kMaxVideoFrames + 2,
    kMaxNumReqPictures = H264DPB::kDPBMaxSize + kPicsInPipeline,
  };

  // Internal state of the decoder.
  enum State {
    kNeedStreamMetadata,  // After initialization, need an SPS.
    kDecoding,            // Ready to decode from any point.
    kAfterReset,          // After Reset(), need a resume point.
    kError,               // Error in decode, can't continue.
  };

  // Process H264 stream structures.
  bool ProcessSPS(int sps_id, bool* need_new_buffers);
  bool ProcessPPS(int pps_id);
  bool PreprocessSlice(media::H264SliceHeader* slice_hdr);
  bool ProcessSlice(media::H264SliceHeader* slice_hdr);

  // Initialize the current picture according to data in |slice_hdr|.
  bool InitCurrPicture(media::H264SliceHeader* slice_hdr);

  // Calculate picture order counts for the new picture
  // on initialization of a new frame (see spec).
  bool CalculatePicOrderCounts(media::H264SliceHeader* slice_hdr);

  // Update PicNum values in pictures stored in DPB on creation of new
  // frame (see spec).
  void UpdatePicNums();

  bool UpdateMaxNumReorderFrames(const media::H264SPS* sps);

  // Prepare reference picture lists for the current frame.
  void PrepareRefPicLists(media::H264SliceHeader* slice_hdr);
  // Prepare reference picture lists for the given slice.
  bool ModifyReferencePicLists(media::H264SliceHeader* slice_hdr,
                               H264Picture::Vector* ref_pic_list0,
                               H264Picture::Vector* ref_pic_list1);

  // Construct initial reference picture lists for use in decoding of
  // P and B pictures (see 8.2.4 in spec).
  void ConstructReferencePicListsP(media::H264SliceHeader* slice_hdr);
  void ConstructReferencePicListsB(media::H264SliceHeader* slice_hdr);

  // Helper functions for reference list construction, per spec.
  int PicNumF(const scoped_refptr<H264Picture>& pic);
  int LongTermPicNumF(const scoped_refptr<H264Picture>& pic);

  // Perform the reference picture lists' modification (reordering), as
  // specified in spec (8.2.4).
  //
  // |list| indicates list number and should be either 0 or 1.
  bool ModifyReferencePicList(media::H264SliceHeader* slice_hdr,
                              int list,
                              H264Picture::Vector* ref_pic_listx);

  // Perform reference picture memory management operations (marking/unmarking
  // of reference pictures, long term picture management, discarding, etc.).
  // See 8.2.5 in spec.
  bool HandleMemoryManagementOps();
  void ReferencePictureMarking();

  // Start processing a new frame.
  bool StartNewFrame(media::H264SliceHeader* slice_hdr);

  // All data for a frame received, process it and decode.
  bool FinishPrevFrameIfPresent();

  // Called after decoding, performs all operations to be done after decoding,
  // including DPB management, reference picture marking and memory management
  // operations.
  // This will also output a picture if one is ready for output.
  bool FinishPicture();

  // Clear DPB contents and remove all surfaces in DPB from *in_use_ list.
  // Cleared pictures will be made available for decode, unless they are
  // at client waiting to be displayed.
  void ClearDPB();

  // Commits all pending data for HW decoder and starts HW decoder.
  bool DecodePicture();

  // Notifies client that a picture is ready for output.
  void OutputPic(scoped_refptr<H264Picture> pic);

  // Output all pictures in DPB that have not been outputted yet.
  bool OutputAllRemainingPics();

  // Decoder state.
  State state_;

  // Parser in use.
  media::H264Parser parser_;

  // DPB in use.
  H264DPB dpb_;

  // Picture currently being processed/decoded.
  scoped_refptr<H264Picture> curr_pic_;

  // Reference picture lists, constructed for each frame.
  H264Picture::Vector ref_pic_list_p0_;
  H264Picture::Vector ref_pic_list_b0_;
  H264Picture::Vector ref_pic_list_b1_;

  // Global state values, needed in decoding. See spec.
  int max_pic_order_cnt_lsb_;
  int max_frame_num_;
  int max_pic_num_;
  int max_long_term_frame_idx_;
  size_t max_num_reorder_frames_;

  int frame_num_;
  int prev_frame_num_;
  int prev_frame_num_offset_;
  bool prev_has_memmgmnt5_;

  // Values related to previously decoded reference picture.
  bool prev_ref_has_memmgmnt5_;
  int prev_ref_top_field_order_cnt_;
  int prev_ref_pic_order_cnt_msb_;
  int prev_ref_pic_order_cnt_lsb_;
  H264Picture::Field prev_ref_field_;

  // Currently active SPS and PPS.
  int curr_sps_id_;
  int curr_pps_id_;

  // Current NALU and slice header being processed.
  scoped_ptr<media::H264NALU> curr_nalu_;
  scoped_ptr<media::H264SliceHeader> curr_slice_hdr_;

  // Output picture size.
  gfx::Size pic_size_;

  // PicOrderCount of the previously outputted frame.
  int last_output_poc_;

  H264Accelerator* accelerator_;

  DISALLOW_COPY_AND_ASSIGN(H264Decoder);
};

}  // namespace content

#endif  // CONTENT_COMMON_GPU_MEDIA_H264_DECODER_H_
