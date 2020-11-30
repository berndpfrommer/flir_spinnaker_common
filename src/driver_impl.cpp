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

#include "driver_impl.h"

#include <string>
#include <iostream>
#include <vector>
#include <chrono>

namespace flir_spinnaker_common
{
namespace GenApi = Spinnaker::GenApi;

std::string get_serial(Spinnaker::CameraPtr cam)
{
  const auto & nodeMap = cam->GetTLDeviceNodeMap();
  const Spinnaker::GenApi::CStringPtr psn = nodeMap.GetNode("DeviceSerialNumber");
  return IsReadable(psn) ? std::string(psn->GetValue()) : "";
}

DriverImpl::DriverImpl()
{
  system_ = Spinnaker::System::GetInstance();
  if (!system_) {
    std::cerr << "cannot instantiate spinnaker driver!" << std::endl;
    throw std::runtime_error("failed to get spinnaker driver!");
  }
  cameraList_ = system_->GetCameras();
  for (size_t cam_idx = 0; cam_idx < cameraList_.GetSize(); cam_idx++) {
    const auto cam = cameraList_[cam_idx];
  }

}

DriverImpl::~DriverImpl()
{
  stopCamera();
  cameraList_.Clear();
  system_->ReleaseInstance();
}

std::string DriverImpl::getLibraryVersion() const
{
  const Spinnaker::LibraryVersion lv = system_->GetLibraryVersion();
  char buf[256];
  snprintf(
    buf, sizeof(buf), "%d.%d.%d.%d", lv.major, lv.minor, lv.type,
    lv.build);
  return std::string(buf);
}

std::vector<std::string> DriverImpl::getSerialNumbers() const
{
  std::vector<std::string> sn;
  for (size_t cam_idx = 0; cam_idx < cameraList_.GetSize(); cam_idx++) {
    const auto cam = cameraList_.GetByIndex(cam_idx);
    sn.push_back(get_serial(cam));
  }
  return sn;
}

static bool set_acquisition_mode_continuous(GenApi::INodeMap & nodeMap)
{
  Spinnaker::GenApi::CEnumerationPtr ptrAcquisitionMode =
    nodeMap.GetNode("AcquisitionMode");
  if (GenApi::IsAvailable(ptrAcquisitionMode) &&
    GenApi::IsWritable(ptrAcquisitionMode))
  {
    GenApi::CEnumEntryPtr ptrAcquisitionModeContinuous =
      ptrAcquisitionMode->GetEntryByName("Continuous");
    if (GenApi::IsAvailable(ptrAcquisitionModeContinuous) &&
      GenApi::IsReadable(ptrAcquisitionModeContinuous))
    {
      // Retrieve integer value from entry node
      const int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();
      // Set integer value from entry node as new value of enumeration node
      ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);
      return true;
    }
  }
  return false;
}

void DriverImpl::run()
{
  while (keepRunning_) {
    Spinnaker::ImagePtr imgPtr = camera_->GetNextImage(1000);
    auto now = std::chrono::high_resolution_clock::now();
    uint64_t t = std::chrono::duration_cast<std::chrono::nanoseconds>(
                   now.time_since_epoch())
                   .count();
    if (imgPtr->IsIncomplete()) {
      // Retrieve and print the image status description
      std::cout << "Image incomplete: "
                << Spinnaker::Image::GetImageStatusDescription(
                     imgPtr->GetImageStatus())
                << std::endl;
    } else {
      const size_t width = imgPtr->GetWidth();
      const size_t height = imgPtr->GetHeight();
      //auto cpa = imgPtr->GetColorProcessing();
#if 0      
      std::cout << "got image: " << width << "x" << height << " stride: " << imgPtr->GetStride() <<
        " bpp: " << imgPtr->GetBitsPerPixel() << " chan: " << imgPtr->GetNumChannels() <<
        " tl payload type: " << imgPtr->GetTLPayloadType() << " tl pix fmt: " <<
        imgPtr->GetTLPixelFormat() <<
        " payload type: " << imgPtr->GetPayloadType() << " pixfmt enum: " <<
        imgPtr->GetPixelFormat() <<
        " fmt: " << imgPtr->GetPixelFormatName() << " int type: " <<
        imgPtr->GetPixelFormatIntType() << " frame id: " << imgPtr->GetFrameID() << " img id: " <<
        imgPtr->GetID() << std::endl;
#endif
      // Note: GetPixelFormat() did not work for the grasshopper, so ignoring
      // pixel format
      ImagePtr img(new Image(
        t, imgPtr->GetTimeStamp(), imgPtr->GetImageSize(),
        imgPtr->GetImageStatus(), imgPtr->GetData(), imgPtr->GetWidth(),
        imgPtr->GetHeight(), imgPtr->GetStride(), imgPtr->GetBitsPerPixel(),
        imgPtr->GetNumChannels(), imgPtr->GetFrameID(), pixelFormat_));
      callback_(img);
    }
  }
}

bool DriverImpl::startCamera(
  const std::string & serialNumber, const Driver::Callback & cb)
{
  if (camera_) {
    return false;
  }
  callback_ = cb;
  for (size_t cam_idx = 0; cam_idx < cameraList_.GetSize(); cam_idx++) {
    auto cam = cameraList_.GetByIndex(cam_idx);
    const std::string sn = get_serial(cam);
    if (sn == serialNumber) {
      camera_ = cam;
      camera_->Init();
      // get GenICam nodemap
      GenApi::INodeMap & nodeMap = camera_->GetNodeMap();
      if (set_acquisition_mode_continuous(nodeMap)) {
        camera_->BeginAcquisition();
        keepRunning_ = true;
        thread_ = std::make_shared<std::thread>(&DriverImpl::run, this);
      } else {
        std::cerr << "continuous acquisition failed for " << serialNumber
                  << std::endl;
        camera_->DeInit();
        camera_ = 0;
        return false;
      }
      GenApi::CEnumerationPtr ptrPixelFormat = nodeMap.GetNode("PixelFormat");
      if (GenApi::IsAvailable(ptrPixelFormat)) {
        setPixelFormat(
          ptrPixelFormat->GetCurrentEntry()->GetSymbolic().c_str());
        if (GenApi::IsWritable(ptrPixelFormat)) {
          std::cout << "is writeable!" << std::endl;
        }
      } else {
        setPixelFormat("BayerRG8");
        std::cerr << "WARNING: driver could not read pixel format!"
                  << std::endl;
      }
    }
  }
  return camera_ != 0;
}

bool DriverImpl::stopCamera()
{
  if (thread_) {
    keepRunning_ = false;
    thread_->join();
    thread_.reset();
  }
  if (camera_) {
    camera_->EndAcquisition();
    camera_->DeInit();
    camera_ = 0;
  } else {
    return false;
  }
  return true;
}

void DriverImpl::setPixelFormat(const std::string & pixFmt)
{
  pixelFormat_ = pixel_format::from_nodemap_string(pixFmt);
}

std::string DriverImpl::getPixelFormat() const
{
  return (pixel_format::to_string(pixelFormat_));
}

}  // namespace flir_spinnaker_common
