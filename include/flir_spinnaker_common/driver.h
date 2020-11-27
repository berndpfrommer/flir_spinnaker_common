/* -*-c++-*--------------------------------------------------------------------
 * 2020 Bernd Pfrommer bernd.pfrommer@gmail.com
 */

#pragma once

#include <memory>
#include <string>

namespace flir_spinnaker_common {
class DriverImpl;  // forward decl
class Driver {
 public:
  Driver();
  std::string getLibraryVersion() const;

 private:
  // ----- variables --
  std::shared_ptr<DriverImpl> driverImpl_;
};
}  // namespace flir_spinnaker_common
