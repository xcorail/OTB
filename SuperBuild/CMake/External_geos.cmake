INCLUDE_ONCE_MACRO(GEOS)

SETUP_SUPERBUILD(GEOS)

if(MSVC)
ExternalProject_Add(GEOS
  PREFIX GEOS
  GIT_REPOSITORY "https://github.com/libgeos/libgeos"
  GIT_TAG 714127ef49087634e7f6687424fdf041d18c3d0f
  SOURCE_DIR ${GEOS_SB_SRC}
  BINARY_DIR ${GEOS_SB_BUILD_DIR}
  INSTALL_DIR ${SB_INSTALL_PREFIX}
  DOWNLOAD_DIR ${DOWNLOAD_LOCATION}
  CMAKE_CACHE_ARGS
  ${SB_CMAKE_CACHE_ARGS}
  -DBUILD_TESTING:BOOL=OFF
  -DGEOS_ENABLE_TESTS:BOOL=OFF
  -DProject_WC_REVISION:STRING=99999
  -DGEOS_BUILD_PACKAGED:BOOL=OFF
  -DGEOS_ENABLE_MACOSX_FRAMEWORK:BOOL=OFF
  CMAKE_COMMAND ${SB_CMAKE_COMMAND} )

else()
ExternalProject_Add(GEOS
  PREFIX GEOS
  URL "http://download.osgeo.org/geos/geos-3.4.2.tar.bz2"
  URL_MD5 fc5df2d926eb7e67f988a43a92683bae
  SOURCE_DIR ${GEOS_SB_SRC}
  BINARY_DIR ${GEOS_SB_BUILD_DIR}
  INSTALL_DIR ${SB_INSTALL_PREFIX}
  DOWNLOAD_DIR ${DOWNLOAD_LOCATION}
  CMAKE_CACHE_ARGS
  ${SB_CMAKE_CACHE_ARGS}
  -DBUILD_TESTING:BOOL=OFF
  -DGEOS_ENABLE_TESTS:BOOL=OFF
  CMAKE_COMMAND ${SB_CMAKE_COMMAND} )
endif()

SUPERBUILD_UPDATE_CMAKE_VARIABLES(GEOS FALSE)