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

// NOTE: much of this code is adapted from the Spinnaker examples,
// in particular NodeMapInfo.cpp
//
//

#include "genicam_utils.h"

#include <iostream>
#include <sstream>
using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

//#define DEBUG_NODE_TRAVERSAL

namespace flir_spinnaker_common
{
namespace genicam_utils
{
template <class T>
static bool is_readable(T ptr)
{
  return (GenApi::IsAvailable(ptr) && GenApi::IsReadable(ptr));
}

void get_nodemap_as_string(std::stringstream & ss, Spinnaker::CameraPtr cam)
{
  GenICam::gcstring s = cam->GetGuiXml();
  ss << s;
}

static GenApi::CNodePtr find_node(const std::string & path, CNodePtr & node)
{
  // split off first part
  auto pos = path.find("/");
  const std::string token = path.substr(0, pos);  // first part of it
  if (node->GetPrincipalInterfaceType() != intfICategory) {
    std::cerr << "no category node: " << node->GetName() << " vs " << path
              << std::endl;
    return (NULL);
  }

  CCategoryPtr catNode = static_cast<CCategoryPtr>(node);
  gcstring displayName = catNode->GetDisplayName();
  gcstring name = catNode->GetName();
  FeatureList_t features;
  catNode->GetFeatures(features);
#ifdef DEBUG_NODE_TRAVERSAL
  std::cout << "parsing: " << name << " with features: " << features.size()
            << std::endl;
#endif
  for (auto it = features.begin(); it != features.end(); ++it) {
    CNodePtr childNode = *it;
#ifdef DEBUG_NODE_TRAVERSAL
    std::cout << "checking child: " << childNode->GetName() << " vs " << token
              << std::endl;
#endif
    if (std::string(childNode->GetName().c_str()) == token) {
      if (is_readable(childNode)) {
        if (pos == std::string::npos) {  // no slash in name, found leaf node
          return (childNode);
        } else {
          const std::string rest = path.substr(pos + 1);
          return (find_node(rest, childNode));
        }
      }
    }
  }
#ifdef DEBUG_NODE_TRAVERSAL
  std::cerr << "driver: node not found: " << path << std::endl;
#endif
  return (GenApi::CNodePtr(NULL));
}

GenApi::CNodePtr find_node(const std::string & path, Spinnaker::CameraPtr cam)
{
  INodeMap & appLayerNodeMap = cam->GetNodeMap();
  CNodePtr rootNode = appLayerNodeMap.GetNode("Root");
  CNodePtr retNode = find_node(path, rootNode);
  return (retNode);
}
}  // namespace genicam_utils
}  // namespace flir_spinnaker_common
