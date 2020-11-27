/* -*-c++-*--------------------------------------------------------------------
 * 2020 Bernd Pfrommer bernd.pfrommer@gmail.com
 */

#pragma once

#include <SpinGenApi/SpinnakerGenApi.h>
#include <Spinnaker.h>

namespace flir_spinnaker_common {
class DriverImpl {
 public:
  DriverImpl();
  std::string getLibraryVersion() const;

 private:
  // ----- variables --
  Spinnaker::SystemPtr system_;
};
}  // namespace flir_spinnaker_common
