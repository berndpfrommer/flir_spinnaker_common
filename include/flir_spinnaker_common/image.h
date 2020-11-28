// -*-c++-*--------------------------------------------------------------------
// Copyright 2020 Bernd Pfrommer <bernd.pfrommer@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef FLIR_SPINNAKER_COMMON__IMAGE_H_
#define FLIR_SPINNAKER_COMMON__IMAGE_H_

#include <memory>

namespace flir_spinnaker_common
{
  class Image
  {
public:
    typedef enum {INVALID = 0, BayerRG8} PixelFormat;
    Image(
      uint64_t t, uint64_t imgT, size_t imageSize, int status,
      const void * data, size_t w, size_t h,
      size_t stride, size_t bitsPerPixel, size_t numChan, uint64_t frameId,
      int pixFmtIntType, int pixFmt)
      : time_(t), imageTime_(imgT), imageSize_(imageSize), imageStatus_(status),
      data_(data), width_(w),
      height_(h), stride_(stride), bitsPerPixel_(bitsPerPixel),
      numChan_(numChan), frameId_(frameId),
      pixFmtIntType_(pixFmtIntType) {
      switch (pixFmt) {
        case 4: // PixelFormat_BayerRG8
          pixFmt_ = BayerRG8;
          break;
        default:
          pixFmt_ = INVALID;
      }
    }

    // ----- variables --
    uint64_t time_;
    uint64_t imageTime_;
    size_t imageSize_;
    int imageStatus_;
    const void * data_;
    size_t width_;
    size_t height_;
    size_t stride_; // in bytes
    size_t bitsPerPixel_;
    size_t numChan_;
    uint64_t frameId_;
    int pixFmtIntType_;
    PixelFormat pixFmt_;

private:
  };
  typedef std::shared_ptr < Image > ImagePtr;
  typedef std::shared_ptr < const Image > ImageConstPtr;
}  // namespace flir_spinnaker_common
#endif  // FLIR_SPINNAKER_COMMON__IMAGE_H_
