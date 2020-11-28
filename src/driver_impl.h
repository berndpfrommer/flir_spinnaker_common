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

#ifndef DRIVER_IMPL_H_
#define DRIVER_IMPL_H_

#include <SpinGenApi/SpinnakerGenApi.h>
#include <Spinnaker.h>

#include <flir_spinnaker_common/driver.h>

#include <string>
#include <vector>
#include <thread>
#include <memory>

namespace flir_spinnaker_common {
  class DriverImpl {

public:
    DriverImpl();
    ~DriverImpl();
    std::string getLibraryVersion() const;
    std::vector < std::string > getSerialNumbers() const;
    bool startCamera(const std::string & serialNumber, const Driver::Callback & cb);
    bool stopCamera();

private:
    void run();

    // ----- variables --
    Spinnaker::SystemPtr system_;
    Spinnaker::CameraList cameraList_;
    Spinnaker::CameraPtr camera_;
    Driver::Callback callback_;
    bool keepRunning_;
    std::shared_ptr < std::thread > thread_;
  };
}  // namespace flir_spinnaker_common

#endif  // DRIVER_IMPL_H_
