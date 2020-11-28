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

#include <flir_spinnaker_common/driver.h>

#include <string>

#include "./driver_impl.h"

namespace flir_spinnaker_common
{

Driver::Driver() {driverImpl_.reset(new DriverImpl());}

std::string Driver::getLibraryVersion() const
{
  return driverImpl_->getLibraryVersion();
}
std::vector<std::string> Driver::getSerialNumbers() const
{
  return driverImpl_->getSerialNumbers();
}

bool Driver::startCamera(const std::string & serialNumber, const Callback & cb)
{
  return driverImpl_->startCamera(serialNumber, cb);
}
bool Driver::stopCamera()
{
  return driverImpl_->stopCamera();
}

}  // namespace flir_spinnaker_common
