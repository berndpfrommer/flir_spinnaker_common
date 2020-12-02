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

#ifndef FLIR_SPINNAKER_COMMON__DRIVER_H_
#define FLIR_SPINNAKER_COMMON__DRIVER_H_

#include <flir_spinnaker_common/image.h>

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace flir_spinnaker_common
{
  class DriverImpl;
  class Driver
  {
  public:
    typedef std::function<void(const ImageConstPtr & img)> Callback;
    Driver();
    std::string getLibraryVersion() const;
    std::vector<std::string> getSerialNumbers() const;

    bool initCamera(const std::string & serialNumber);
    bool deInitCamera();
    bool startCamera(const Driver::Callback & cb);
    bool stopCamera();

    std::string getPixelFormat() const;
    double getFrameRate() const;
    std::string setFrameRate(double rate, double * retRate);
    std::string getNodeMapAsString();
    std::string setExposureTime(double t, double * retT);
    std::string setAutoExposure(bool a, bool * retA);

  private:
    // ----- variables --
    std::shared_ptr<DriverImpl> driverImpl_;
  };
  }     // namespace flir_spinnaker_common
#endif  // FLIR_SPINNAKER_COMMON__DRIVER_H_
