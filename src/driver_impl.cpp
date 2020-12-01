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

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "genicam_utils.h"

namespace flir_spinnaker_common
{
namespace GenApi = Spinnaker::GenApi;
namespace GenICam = Spinnaker::GenICam;

std::string get_serial(Spinnaker::CameraPtr cam)
{
  const auto & nodeMap = cam->GetTLDeviceNodeMap();
  const Spinnaker::GenApi::CStringPtr psn =
    nodeMap.GetNode("DeviceSerialNumber");
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
  deInitCamera();
  cameraList_.Clear();
  system_->ReleaseInstance();
}

std::string DriverImpl::getLibraryVersion() const
{
  const Spinnaker::LibraryVersion lv = system_->GetLibraryVersion();
  char buf[256];
  snprintf(
    buf, sizeof(buf), "%d.%d.%d.%d", lv.major, lv.minor, lv.type, lv.build);
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

bool DriverImpl::setInINodeMap(
  double f, const std::string & field, double * fret)
{
  GenApi::INodeMap & nodeMap = camera_->GetNodeMap();
  GenApi::CFloatPtr fp = nodeMap.GetNode(field.c_str());
  if (!GenApi::IsAvailable(fp)) {
    std::cerr << "cannot find field: " << field << " in INodeMap!" << std::endl;
  } else {
    if (!GenApi::IsWritable(fp)) {
      std::cerr << "cannot write field: " << field << " in INodeMap!"
                << std::endl;
    } else {
      fp->SetValue(f);
      if (!GenApi::IsReadable(fp)) {
        std::cerr << "cannot read back field: " << field << " in INodeMap!"
                  << std::endl;
      } else {
        *fret = fp->GetValue();
        return (true);
      }
    }
  }
  *fret = f;
  return (false);
}

template <class T>
static bool is_available(T ptr)
{
  return (ptr.IsValid() && GenApi::IsAvailable(ptr));
}

template <class T>
static bool is_writable(T ptr)
{
  return (ptr.IsValid() && GenApi::IsAvailable(ptr) && GenApi::IsWritable(ptr));
}

template <class T>
static bool is_readable(T ptr)
{
  return (ptr.IsValid() && GenApi::IsAvailable(ptr) && GenApi::IsReadable(ptr));
}

static bool configure_interfaces(Spinnaker::SystemPtr system)
{
  Spinnaker::InterfaceList interfaceList = system->GetInterfaces();
  try {
    Spinnaker::InterfacePtr interfacePtr = nullptr;
    const unsigned int numInterfaces = interfaceList.GetSize();

    for (unsigned int i = 0; i < numInterfaces; i++) {
      interfacePtr = interfaceList.GetByIndex(i);
      GenApi::INodeMap & nodeMapInterface = interfacePtr->GetTLNodeMap();
      interfacePtr->UpdateCameras();
      Spinnaker::CameraList camList = interfacePtr->GetCameras();
      if (camList.GetSize() >= 1) {
        // Display interface name
        GenApi::CStringPtr ptrInterfaceDisplayName =
          nodeMapInterface.GetNode("InterfaceDisplayName");
        const GenICam::gcstring interfaceDisplayName =
          ptrInterfaceDisplayName->GetValue();
        std::cout << interfaceDisplayName << std::endl;

        // Apply Action device/group settings
        GenApi::CIntegerPtr ptrGevActionDeviceKey =
          nodeMapInterface.GetNode("GevActionDeviceKey");
        if (!is_writable(ptrGevActionDeviceKey)) {
          std::cerr << "Interface " << i
                    << " Unable to set Interface Action Device Key (node "
                       "retrieval). Aborting..."
                    << std::endl;
          continue;
        }

        // Set Action Device Key to 0
        ptrGevActionDeviceKey->SetValue(0);
        std::cout << "Interface " << i << " action device key is set"
                  << std::endl;

        GenApi::CIntegerPtr ptrGevActionGroupKey =
          nodeMapInterface.GetNode("GevActionGroupKey");
        if (!is_writable(ptrGevActionGroupKey)) {
          std::cerr
            << "Interface " << i
            << " Unable to set Interface Action Group Key (node retrieval). "
               "Aborting..."
            << std::endl;
          return (false);
        }

        // Set Action Group Key to 1
        ptrGevActionGroupKey->SetValue(1);
        std::cout << "Interface " << i << " action group key is set"
                  << std::endl;

        GenApi::CIntegerPtr ptrGevActionGroupMask =
          nodeMapInterface.GetNode("GevActionGroupMask");
        if (!is_writable(ptrGevActionGroupMask)) {
          std::cerr << "Interface " << i
                    << " Unable to set Interface Action Group Mask (node "
                       "retrieval). Aborting..."
                    << std::endl;
          return (false);
        }

        // Set Action Group Mask to 1
        ptrGevActionGroupMask->SetValue(1);
        std::cout << "Interface " << i << " action group mask is set"
                  << std::endl;
      }
    }
  } catch (Spinnaker::Exception & e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return (false);
  }
  return (true);
}

static bool configure_action_control(const Spinnaker::CameraPtr & cam)
{
  GenApi::CIntegerPtr ptrActionDeviceKey =
    cam->GetNodeMap().GetNode("ActionDeviceKey");
  if (!is_writable(ptrActionDeviceKey)) {
    std::cerr << "cannot set action device key!" << std::endl;
    return (false);
  }
  ptrActionDeviceKey->SetValue(0);

  GenApi::CIntegerPtr ptrActionGroupKey =
    cam->GetNodeMap().GetNode("ActionGroupKey");
  if (!is_writable(ptrActionGroupKey)) {
    std::cerr << "cannot set action group key!" << std::endl;
    return (false);
  }
  ptrActionGroupKey->SetValue(1);

  GenApi::CIntegerPtr ptrActionGroupMask =
    cam->GetNodeMap().GetNode("ActionGroupMask");
  if (!is_writable(ptrActionGroupMask)) {
    std::cerr << "cannot set action group mask!" << std::endl;
    return (false);
  }
  // Set action group mask to 1
  ptrActionGroupMask->SetValue(1);
  return (true);
}

static bool set_frame_rate_grasshopper(
  Spinnaker::CameraPtr cam, double rate, double * retRate)
{
  *retRate = rate;
  //
  // For the grasshopper, first need to switch off automatic
  // setting of the frame rate
  GenApi::CNodePtr fra = genicam_utils::find_node(
    "AcquisitionControl/AcquisitionFrameRateAuto", cam);
  if (!is_writable(fra)) {
    // couldn't find the node or not writable, probably not a grasshopper
    return (false);
  }
  GenApi::CEnumerationPtr frameRateAuto =
    static_cast<GenApi::CEnumerationPtr>(fra);
  // find out what integer value "Off" corresponds to
  GenApi::CEnumEntryPtr frameRateOff = frameRateAuto->GetEntryByName("Off");
  if (!is_readable(frameRateOff)) {
    return (false);  // should not happen, but whatever
  }
  // switch off the auto frame rate
  frameRateAuto->SetIntValue(frameRateOff->GetValue());
  // find and set the manual frame rate
  GenApi::CNodePtr fr =
    genicam_utils::find_node("AcquisitionControl/AcquisitionFrameRate", cam);
  if (is_writable(fr)) {
    GenApi::CFloatPtr fp = static_cast<GenApi::CFloatPtr>(fr);
    fp->SetValue(rate);
    // read back the value
    *retRate = fp->GetValue(rate);
    return (true);
  }  // namespace flir_spinnaker_common
  return (false);
}

static bool set_frame_rate_blackfly_s(
  Spinnaker::CameraPtr cam, double rate, double * retRate)
{
  *retRate = rate;
  GenApi::INodeMap & nodeMap = cam->GetNodeMap();
  // enable manual frame rate setting
  GenApi::CBooleanPtr ptrFrameRateEnable =
    nodeMap.GetNode("AcquisitionFrameRateEnable");
  if (!is_writable(ptrFrameRateEnable)) {
    std::cerr << "cannot set framerate" << std::endl;
    return (false);
  }
  ptrFrameRateEnable->SetValue(true);
  // set the framerate
  GenApi::CFloatPtr ptrFrameRate =
    cam->GetNodeMap().GetNode("AcquisitionFrameRate");
  if (!is_writable(ptrFrameRate)) {
    return (false);
  }
  ptrFrameRate->SetValue(rate);
  if (is_readable(ptrFrameRate)) {
    *retRate = ptrFrameRate->GetValue();
  }
  return (true);
}

std::string DriverImpl::setFrameRate(double rate, double * retRate)
{
  try {
    *retRate = rate;
    if (set_frame_rate_grasshopper(camera_, rate, retRate)) {
      return ("OK");
    }
    if (set_frame_rate_blackfly_s(camera_, rate, retRate)) {
      return ("OK");
    }
  } catch (const Spinnaker::Exception & e) {
    return (e.what());
  }
  return ("frame rate set failed. Unknown camera?");
}

static bool set_acquisition_mode_continuous(GenApi::INodeMap & nodeMap)
{
  Spinnaker::GenApi::CEnumerationPtr ptrAcquisitionMode =
    nodeMap.GetNode("AcquisitionMode");
  if (
    GenApi::IsAvailable(ptrAcquisitionMode) &&
    GenApi::IsWritable(ptrAcquisitionMode)) {
    GenApi::CEnumEntryPtr ptrAcquisitionModeContinuous =
      ptrAcquisitionMode->GetEntryByName("Continuous");
    if (
      GenApi::IsAvailable(ptrAcquisitionModeContinuous) &&
      GenApi::IsReadable(ptrAcquisitionModeContinuous)) {
      // Retrieve integer value from entry node
      const int64_t acquisitionModeContinuous =
        ptrAcquisitionModeContinuous->GetValue();
      // Set integer value from entry node as new value of enumeration node
      ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);
      return true;
    }
  }
  return false;
}

double DriverImpl::getFrameRate() const
{
  return (avgTimeInterval_ > 0 ? (1.0 / avgTimeInterval_) : 0);
}

void DriverImpl::OnImageEvent(Spinnaker::ImagePtr imgPtr)
{
  // update frame rate
  auto now = std::chrono::high_resolution_clock::now();
  uint64_t t =
    std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch())
      .count();
  if (avgTimeInterval_ == 0) {
    if (lastTime_ != 0) {
      avgTimeInterval_ = (t - lastTime_) * 1e-9;
    }
  } else {
    const double dt = (t - lastTime_) * 1e-9;
    const double alpha = 0.01;
    avgTimeInterval_ = avgTimeInterval_ * (1.0 - alpha) + dt * alpha;
  }
  lastTime_ = t;

  if (imgPtr->IsIncomplete()) {
    // Retrieve and print the image status description
    std::cout << "Image incomplete: "
              << Spinnaker::Image::GetImageStatusDescription(
                   imgPtr->GetImageStatus())
              << std::endl;
  } else {
#if 0    
    std::cout << "got image: " << imgPtr->GetWidth() << "x"
              << imgPtr->GetHeight() << " stride: " << imgPtr->GetStride()
              << " bpp: " << imgPtr->GetBitsPerPixel()
              << " chan: " << imgPtr->GetNumChannels()
              << " tl payload type: " << imgPtr->GetTLPayloadType()
              << " tl pix fmt: " << imgPtr->GetTLPixelFormat()
              << " payload type: " << imgPtr->GetPayloadType()
              << " pixfmt enum: " << imgPtr->GetPixelFormat()
              << " fmt: " << imgPtr->GetPixelFormatName()
              << " int type: " << imgPtr->GetPixelFormatIntType()
              << " frame id: " << imgPtr->GetFrameID()
              << " img id: " << imgPtr->GetID() << std::endl;
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

bool DriverImpl::initCamera(const std::string & serialNumber)
{
  if (camera_) {
    return false;
  }
  for (size_t cam_idx = 0; cam_idx < cameraList_.GetSize(); cam_idx++) {
    auto cam = cameraList_.GetByIndex(cam_idx);
    const std::string sn = get_serial(cam);
    if (sn == serialNumber) {
      camera_ = cam;
      camera_->Init();
      break;
    }
  }
  return (camera_ != 0);
}

bool DriverImpl::deInitCamera()
{
  if (!camera_) {
    return (false);
  }
  camera_->DeInit();
  return (true);
}

bool DriverImpl::startCamera(const Driver::Callback & cb)
{
  if (!camera_) {
    return false;
  }
  // switch on continuous acquisition
  // and get pixel format
  GenApi::INodeMap & nodeMap = camera_->GetNodeMap();
  if (set_acquisition_mode_continuous(nodeMap)) {
    camera_->RegisterEventHandler(*this);
    camera_->BeginAcquisition();
    GenApi::CEnumerationPtr ptrPixelFormat = nodeMap.GetNode("PixelFormat");
    if (GenApi::IsAvailable(ptrPixelFormat)) {
      setPixelFormat(ptrPixelFormat->GetCurrentEntry()->GetSymbolic().c_str());
    } else {
      setPixelFormat("BayerRG8");
      std::cerr << "WARNING: driver could not read pixel format!" << std::endl;
    }
  } else {
    std::cerr << "failed to switch on continuous acquisition!" << std::endl;
    return (false);
  }
  callback_ = cb;
  return (true);
}

bool DriverImpl::stopCamera()
{
  if (camera_) {
    camera_->UnregisterEventHandler(*this);
    camera_->EndAcquisition();
    return true;
  }
  return (false);
}

void DriverImpl::setPixelFormat(const std::string & pixFmt)
{
  pixelFormat_ = pixel_format::from_nodemap_string(pixFmt);
}

std::string DriverImpl::getPixelFormat() const
{
  return (pixel_format::to_string(pixelFormat_));
}

std::string DriverImpl::getNodeMapAsString()
{
  std::stringstream ss;
  genicam_utils::get_nodemap_as_string(ss, camera_);
  return (ss.str());
}

}  // namespace flir_spinnaker_common
