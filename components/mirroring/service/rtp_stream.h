// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_MIRRORING_SERVICE_RTP_STREAM_H_
#define COMPONENTS_MIRRORING_SERVICE_RTP_STREAM_H_

#include <memory>
#include <string>
#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/timer/timer.h"
#include "media/base/audio_bus.h"
#include "media/cast/cast_config.h"
#include "media/cast/constants.h"
#include "media/video/video_encode_accelerator.h"

namespace media {

class VideoFrame;

namespace cast {
class VideoSender;
class AudioSender;
}  // namespace cast

}  // namespace media

namespace mirroring {

class RtpStreamClient {
 public:
  virtual ~RtpStreamClient() {}

  // Called when error happened during streaming.
  virtual void OnError(const std::string& message) = 0;

  // Request a fresh video frame from the capturer.
  virtual void RequestRefreshFrame() = 0;

  // The following are for hardware video encoding.

  // Query the supported hardware encoding profiles.
  virtual media::VideoEncodeAccelerator::SupportedProfiles
  GetSupportedVideoEncodeAcceleratorProfiles() = 0;

  virtual void CreateVideoEncodeAccelerator(
      const media::cast::ReceiveVideoEncodeAcceleratorCallback& callback) = 0;

  // TODO(xjz): Remove this interface. Instead, create the shared memory in
  // external video encoder through mojo::ScopedSharedBufferHandle.
  virtual void CreateVideoEncodeMemory(
      size_t size,
      const media::cast::ReceiveVideoEncodeMemoryCallback& callback) = 0;
};

// Receives video frames and submits the data to media::cast::VideoSender. It
// also includes a timer to request refresh frames when the source halts (e.g.,
// a screen capturer stops delivering frames because the screen is not being
// updated). When a halt is detected, refresh frames will be requested at
// regular intervals for a short period of time. This provides the video
// encoder, downstream, several copies of the last frame so that it may clear up
// lossy encoding artifacts.
class VideoRtpStream {
 public:
  VideoRtpStream(std::unique_ptr<media::cast::VideoSender> video_sender,
                 base::WeakPtr<RtpStreamClient> client);
  ~VideoRtpStream();

  static std::vector<media::cast::FrameSenderConfig> GetSupportedConfigs(
      RtpStreamClient* client);

  // Called by VideoCaptureClient when a video frame is received.
  // |video_frame| is required to provide REFERENCE_TIME in the metadata.
  void InsertVideoFrame(scoped_refptr<media::VideoFrame> video_frame);

 private:
  void OnRefreshTimerFired();

  const std::unique_ptr<media::cast::VideoSender> video_sender_;
  const base::WeakPtr<RtpStreamClient> client_;

  // Requests refresh frames at a constant rate while the source is paused, up
  // to a consecutive maximum.
  base::RepeatingTimer refresh_timer_;

  // Counter for the number of consecutive "refresh frames" requested.
  int consecutive_refresh_count_;

  // Set to true when a request for a refresh frame has been made.  This is
  // cleared once the next frame is received.
  bool expecting_a_refresh_frame_;

  base::WeakPtrFactory<VideoRtpStream> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(VideoRtpStream);
};

// Receives audio data and submits the data to media::cast::AudioSender.
// TODO(xjz): Complete implementation after Audio Service mirroring refactoring
// is landed.
class AudioRtpStream {
 public:
  AudioRtpStream(std::unique_ptr<media::cast::AudioSender> audio_sender,
                 base::WeakPtr<RtpStreamClient> client);
  ~AudioRtpStream();

  static std::vector<media::cast::FrameSenderConfig> GetSupportedConfigs();

  // Called by AudioCaptureClient when new audio data is available.
  void InsertAudio(std::unique_ptr<media::AudioBus> audio_bus,
                   base::TimeTicks estimated_capture_time);

 private:
  const std::unique_ptr<media::cast::AudioSender> audio_sender_;
  const base::WeakPtr<RtpStreamClient> client_;

  DISALLOW_COPY_AND_ASSIGN(AudioRtpStream);
};

}  // namespace mirroring

#endif  // COMPONENTS_MIRRORING_SERVICE_RTP_STREAM_H_
