/* -*-c++-*--------------------------------------------------------------------
 * 2020 Bernd Pfrommer bernd.pfrommer@gmail.com
 */

#include <flir_spinnaker_common/driver.h>

#include "driver_impl.h"

namespace flir_spinnaker_common {

Driver::Driver() { driverImpl_.reset(new DriverImpl()); }

std::string Driver::getLibraryVersion() const {
  return (driverImpl_->getLibraryVersion());
}
}  // namespace flir_spinnaker_common
