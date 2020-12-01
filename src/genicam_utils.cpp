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

namespace flir_spinnaker_common
{
namespace genicam_utils
{
template <class T>
static bool is_available(T ptr)
{
  return (GenApi::IsAvailable(ptr));
}

template <class T>
static bool is_writable(T ptr)
{
  return (GenApi::IsAvailable(ptr) && GenApi::IsWritable(ptr));
}

template <class T>
static bool is_readable(T ptr)
{
  return (GenApi::IsAvailable(ptr) && GenApi::IsReadable(ptr));
}

static void indent(std::stringstream & ss, int n)
{
  for (int i = 0; i < n; i++) {
    ss << " ";
  }
}

// ----- some necessary forward declarations
static void get_node_value_as_string(
  std::stringstream & ss, CNodePtr node, unsigned int level);

static void get_enumeration_selector_as_string(
  std::stringstream & ss, CNodePtr node, unsigned int level)
{
  FeatureList_t selectedFeatures;
  node->GetSelectedFeatures(selectedFeatures);
  CEnumerationPtr ptrSelectorNode = static_cast<CEnumerationPtr>(node);
  StringList_t entries;
  ptrSelectorNode->GetSymbolics(entries);
  CEnumEntryPtr ptrCurrentEntry = ptrSelectorNode->GetCurrentEntry();
  const gcstring displayName = ptrSelectorNode->GetDisplayName();
  const gcstring name = ptrSelectorNode->GetName();
  const gcstring currentEntrySymbolic = ptrSelectorNode->ToString();
  indent(ss, level);
  ss << displayName << "(" << name << "): " << currentEntrySymbolic
     << std::endl;

  for (size_t i = 0; i < entries.size(); i++) {
    CEnumEntryPtr selectorEntry = ptrSelectorNode->GetEntryByName(entries[i]);
    FeatureList_t::const_iterator it;
    if (is_writable(ptrSelectorNode)) {
      if (is_readable(selectorEntry)) {
        ptrSelectorNode->SetIntValue(selectorEntry->GetValue());
        indent(ss, level + 1);
        ss << displayName << "(" << name << "): " << ptrSelectorNode->ToString()
           << std::endl;
      }
    }

    for (it = selectedFeatures.begin(); it != selectedFeatures.end(); ++it) {
      CNodePtr ptrFeatureNode = *it;
      if (is_readable(ptrFeatureNode)) {
        get_node_value_as_string(ss, ptrFeatureNode, level + 2);
      }
    }
  }
  if (is_writable(ptrSelectorNode)) {
    ptrSelectorNode->SetIntValue(ptrCurrentEntry->GetValue());
  }
}

static void get_node_value_as_string(
  std::stringstream & ss, CNodePtr node, unsigned int level)
{
  if (
    node->IsSelector() &&
    (node->GetPrincipalInterfaceType() == intfIEnumeration)) {
    get_enumeration_selector_as_string(ss, node, level);
  } else {
    CValuePtr ptrValueNode = static_cast<CValuePtr>(node);
    const gcstring displayName = ptrValueNode->GetDisplayName();
    const gcstring name = ptrValueNode->GetName();
    indent(ss, level);
    ss << displayName << "(" << name << "): " << ptrValueNode->ToString()
       << std::endl;
  }
}

void get_node_as_string(
  std::stringstream & ss, CNodePtr node, unsigned int level)
{
  indent(ss, level);
  CCategoryPtr ptrCategoryNode = static_cast<CCategoryPtr>(node);
  gcstring displayName = ptrCategoryNode->GetDisplayName();
  gcstring name = ptrCategoryNode->GetName();
  ss << displayName << "icat(" << name << ")";
  FeatureList_t features;
  ptrCategoryNode->GetFeatures(features);
  for (auto it = features.begin(); it != features.end(); ++it) {
    CNodePtr ptrFeatureNode = *it;
    if (is_readable(ptrFeatureNode)) {
      if (ptrFeatureNode->GetPrincipalInterfaceType() == intfICategory) {
        get_node_as_string(ss, ptrFeatureNode, level + 1);
      } else {
        get_node_value_as_string(ss, ptrFeatureNode, level + 1);
      }
    }
  }
  ss << std::endl;
}

void get_nodemap_as_string(std::stringstream & ss, Spinnaker::CameraPtr cam)
{
  INodeMap & appLayerNodeMap = cam->GetNodeMap();
  const int level = 0;
  get_node_as_string(ss, appLayerNodeMap.GetNode("Root"), level);
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
  //std::cout << "parsing: " << name << " with features: " << features.size()
  //<< std::endl;
  for (auto it = features.begin(); it != features.end(); ++it) {
    CNodePtr childNode = *it;
    //std::cout << "checking child: " << childNode->GetName() << " vs " << token
    //<< std::endl;
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
