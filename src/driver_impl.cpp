/* -*-c++-*--------------------------------------------------------------------
 * 2020 Bernd Pfrommer bernd.pfrommer@gmail.com
 */

#include "driver_impl.h"

namespace flir_spinnaker_common {

DriverImpl::DriverImpl() {
  system_ = Spinnaker::System::GetInstance();
  if (!system_) {
    std::cerr << "cannot instantiate spinnaker driver!" << std::endl;
    throw std::runtime_error("failed to get spinnaker driver!");
  }
}

std::string DriverImpl::getLibraryVersion() const {
  const Spinnaker::LibraryVersion lv = system_->GetLibraryVersion();
  char buf[256];
  snprintf(buf, sizeof(buf), "%d.%d.%d.%d", lv.major, lv.minor, lv.type,
           lv.build);
  return (std::string(buf));
}
}  // namespace flir_spinnaker_common
