#
# Bernd Pfrommer 2020
#

#
# sets the following variables:
#
# SPINNAKER_INCLUDE_DIRS
# SPINNAKER_LIBRARIES
# SPINNAKER_FOUND
#
# or for the more modern cmake usage, sets the target Spinnaker::Spinnaker
#
#
# searches first in "SPINNAKER_ROOT_DIR" for location of spinnaker SDK
#

if( EXISTS "$ENV{SPINNAKER_ROOT_DIR}" )
  file( TO_CMAKE_PATH "$ENV{SPINNAKER_ROOT_DIR}" SPINNAKER_ROOT_DIR )
  set( SPINNAKER_ROOT_DIR "${SPINNAKER_ROOT_DIR}" CACHE PATH "Prefix for Spinnaker installation." )
endif()

find_path(SPINNAKER_INCLUDE_DIR
  NAMES Spinnaker.h
  HINTS
  ${SPINNAKER_ROOT_DIR}/include
  /opt/spinnaker/include
  /usr/include/spinnaker
  /usr/local/include/spinnaker
  )
  
find_library(SPINNAKER_LIBRARY
  NAMES Spinnaker
  HINTS
  ${SPINNAKER_ROOT_DIR}/lib
  /opt/spinnaker/lib
  /usr/lib/
  /usr/local/lib
  PATH_SUFFIXES Release Debug
  )

set(SPINNAKER_INCLUDE_DIRS ${SPINNAKER_INCLUDE_DIR})
set(SPINNAKER_LIBRARIES ${SPINNAKER_LIBRARY})

if (SPINNAKER_INCLUDE_DIR AND SPINNAKER_LIBRARY)
  set(SPINNAKER_FOUND 1)
endif()

if( SPINNAKER_FOUND AND NOT TARGET Spinnaker::Spinnaker)
    add_library(Spinnaker::Spinnaker      UNKNOWN IMPORTED )
    set_target_properties(Spinnaker::Spinnaker PROPERTIES
      IMPORTED_LOCATION                 "${SPINNAKER_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES     "${SPINNAKER_INCLUDE_DIRS}"
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX")
endif()
    

  
