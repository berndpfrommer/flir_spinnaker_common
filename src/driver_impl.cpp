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

namespace flir_spinnaker_common
{

DriverImpl::DriverImpl()
{
  system_ = Spinnaker::System::GetInstance();
  if (!system_) {
    std::cerr << "cannot instantiate spinnaker driver!" << std::endl;
    throw std::runtime_error("failed to get spinnaker driver!");
  }
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
}  // namespace flir_spinnaker_common
