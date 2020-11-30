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

#include <flir_spinnaker_common/pixel_format.h>
namespace flir_spinnaker_common
{
namespace pixel_format
{
std::string to_string(PixelFormat f)
{
  switch (f) {
    case BayerRG8:
      return ("BayerRG8");
      break;
    default:
      return ("INVALID");
      break;
  }
  return ("INVALID");
}

PixelFormat from_nodemap_string(const std::string pixFmt)
{
  if (pixFmt == "BayerRG8") {
    return (BayerRG8);
  } else {
    return (INVALID);
  }
  return (INVALID);
}

}  // namespace pixel_format
}  // namespace flir_spinnaker_common
