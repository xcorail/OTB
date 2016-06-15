INCLUDE_ONCE_MACRO(OPENJPEG)

SETUP_SUPERBUILD(PROJECT OPENJPEG)

# declare dependencies
ADDTO_DEPENDENCIES_IF_NOT_SYSTEM(OPENJPEG ZLIB TIFF PNG)

ADD_SUPERBUILD_CMAKE_VAR(OPENJPEG TIFF_INCLUDE_DIR)
ADD_SUPERBUILD_CMAKE_VAR(OPENJPEG TIFF_LIBRARY)
ADD_SUPERBUILD_CMAKE_VAR(OPENJPEG ZLIB_INCLUDE_DIR)
ADD_SUPERBUILD_CMAKE_VAR(OPENJPEG ZLIB_LIBRARY)
ADD_SUPERBUILD_CMAKE_VAR(OPENJPEG PNG_INCLUDE_DIR)
ADD_SUPERBUILD_CMAKE_VAR(OPENJPEG PNG_LIBRARY)

if(MSVC)
  #TODO: add LCMS dependency
endif()

ExternalProject_Add(OPENJPEG
  PREFIX OPENJPEG
  URL "http://sourceforge.net/projects/openjpeg.mirror/files/2.1.0/openjpeg-2.1.0.tar.gz/download"
  URL_MD5 f6419fcc233df84f9a81eb36633c6db6
  BINARY_DIR ${OPENJPEG_SB_BUILD_DIR}
  INSTALL_DIR ${SB_INSTALL_PREFIX}
  DOWNLOAD_DIR ${DOWNLOAD_LOCATION}
  CMAKE_CACHE_ARGS
  -DBUILD_CODEC:BOOL=ON
  -DBUILD_DOC:BOOL=OFF
  -DBUILD_JPIP:BOOL=OFF
  -DBUILD_JPWL:BOOL=OFF
  -DBUILD_MJ2:BOOL=OFF
  -DBUILD_PKGCONFIG_FILES:BOOL=ON
  -DBUILD_THIRDPARTY:BOOL=OFF
  ${OPENJPEG_SB_CONFIG}
  DEPENDS ${OPENJPEG_DEPENDENCIES}
  CMAKE_COMMAND ${SB_CMAKE_COMMAND}
  )

SUPERBUILD_UPDATE_CMAKE_VARIABLES(OPENJPEG FALSE)
