if (STATIC_BUILD OR NOT USE_SYSTEM_GOOGLE_LOG)
  SET(GOOGLE_LOG_SOURCES_DIR ${CMAKE_BINARY_DIR}/glog-0.3.2)
  DownloadPackage(
    "897fbff90d91ea2b6d6e78c8cea641cc"
    "http://www.montefiore.ulg.ac.be/~jodogne/Orthanc/ThirdPartyDownloads/glog-0.3.2.tar.gz"
    "${GOOGLE_LOG_SOURCES_DIR}")


  # Glog 0.3.3 fails to build with old versions of MinGW, such as the
  # one installed on our Continuous Integration Server that runs
  # Debian Squeeze. We thus stick to Glog 0.3.2 for the time being.

  #SET(GOOGLE_LOG_SOURCES_DIR ${CMAKE_BINARY_DIR}/glog-0.3.3)
  #DownloadPackage(
  #  "a6fd2c22f8996846e34c763422717c18"
  #  "http://www.montefiore.ulg.ac.be/~jodogne/Orthanc/ThirdPartyDownloads/glog-0.3.3.tar.gz"
  #  "${GOOGLE_LOG_SOURCES_DIR}")


  set(GOOGLE_LOG_HEADERS
    ${GOOGLE_LOG_SOURCES_DIR}/src/glog/logging.h
    ${GOOGLE_LOG_SOURCES_DIR}/src/glog/raw_logging.h
    ${GOOGLE_LOG_SOURCES_DIR}/src/glog/stl_logging.h
    ${GOOGLE_LOG_SOURCES_DIR}/src/glog/vlog_is_on.h
    )

  set(ac_google_namespace google)
  set(ac_google_start_namespace "namespace google {")
  set(ac_google_end_namespace "}")

  if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(ac_cv_have_unistd_h 1)
    set(ac_cv_have_stdint_h 1)
    set(ac_cv_have_systypes_h 0)
    set(ac_cv_have_inttypes_h 0)
    set(ac_cv_have_libgflags 0)
    set(ac_cv_have_uint16_t 1)
    set(ac_cv_have_u_int16_t 0)
    set(ac_cv_have___uint16 0)
    set(ac_cv_cxx_using_operator 1)
    set(ac_cv_have___builtin_expect 1)
  else()
    set(ac_cv_have_unistd_h 0)
    set(ac_cv_have_stdint_h 0)
    set(ac_cv_have_systypes_h 0)
    set(ac_cv_have_inttypes_h 0)
    set(ac_cv_have_libgflags 0)
    set(ac_cv_have_uint16_t 0)
    set(ac_cv_have_u_int16_t 0)
    set(ac_cv_have___uint16 1)
    set(ac_cv_cxx_using_operator 1)
    set(ac_cv_have___builtin_expect 0)
  endif()

  foreach (f ${GOOGLE_LOG_HEADERS})
    configure_file(${f}.in ${f})
  endforeach()

  include_directories(
    ${GOOGLE_LOG_SOURCES_DIR}/src
    )

  if (CMAKE_COMPILER_IS_GNUCXX)
    if ("${CMAKE_SYSTEM_VERSION}" STREQUAL "LinuxStandardBase")
      execute_process(
        COMMAND patch utilities.cc ${CMAKE_SOURCE_DIR}/Resources/Patches/glog-utilities-lsb.diff
        WORKING_DIRECTORY ${GOOGLE_LOG_SOURCES_DIR}/src
        )
    else()
      execute_process(
        COMMAND patch utilities.cc ${CMAKE_SOURCE_DIR}/Resources/Patches/glog-utilities.diff
        WORKING_DIRECTORY ${GOOGLE_LOG_SOURCES_DIR}/src
        )
    endif()

    execute_process(
      COMMAND patch port.h ${CMAKE_SOURCE_DIR}/Resources/Patches/glog-port-h.diff 
      WORKING_DIRECTORY ${GOOGLE_LOG_SOURCES_DIR}/src/windows
      )
    execute_process(
      COMMAND patch port.cc ${CMAKE_SOURCE_DIR}/Resources/Patches/glog-port-cc.diff 
      WORKING_DIRECTORY ${GOOGLE_LOG_SOURCES_DIR}/src/windows
      )
  endif()

  if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    if ("${CMAKE_SYSTEM_VERSION}" STREQUAL "LinuxStandardBase")
      # Install the specific configuration for LSB SDK
      configure_file(
        ${CMAKE_SOURCE_DIR}/Resources/CMake/GoogleLogConfigurationLSB.h
        ${GOOGLE_LOG_SOURCES_DIR}/src/config.h
        COPYONLY)
    else()
      configure_file(
        ${CMAKE_SOURCE_DIR}/Resources/CMake/GoogleLogConfiguration.h
        ${GOOGLE_LOG_SOURCES_DIR}/src/config.h
        COPYONLY)
    endif()

    set(GOOGLE_LOG_SOURCES
      ${GOOGLE_LOG_SOURCES_DIR}/src/demangle.cc
      ${GOOGLE_LOG_SOURCES_DIR}/src/logging.cc
      ${GOOGLE_LOG_SOURCES_DIR}/src/raw_logging.cc
      ${GOOGLE_LOG_SOURCES_DIR}/src/signalhandler.cc
      ${GOOGLE_LOG_SOURCES_DIR}/src/symbolize.cc
      ${GOOGLE_LOG_SOURCES_DIR}/src/utilities.cc
      ${GOOGLE_LOG_SOURCES_DIR}/src/vlog_is_on.cc
      )

  elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    include_directories(
      ${GOOGLE_LOG_SOURCES_DIR}/src/windows
      )

    set(GOOGLE_LOG_SOURCES
      ${GOOGLE_LOG_SOURCES_DIR}/src/windows/port.cc
      ${GOOGLE_LOG_SOURCES_DIR}/src/logging.cc
      ${GOOGLE_LOG_SOURCES_DIR}/src/raw_logging.cc
      ${GOOGLE_LOG_SOURCES_DIR}/src/utilities.cc
      ${GOOGLE_LOG_SOURCES_DIR}/src/vlog_is_on.cc
      )

    add_definitions(
      -DGLOG_NO_ABBREVIATED_SEVERITIES=1
      -DNO_FRAME_POINTER=1
      -DGOOGLE_GLOG_DLL_DECL=
      )

    if (${CMAKE_COMPILER_IS_GNUCXX})
      # This is a patch for MinGW64
      add_definitions(-D_TIME_H__S=1)
    endif()

  endif()
 


  add_library(GoogleLog STATIC ${GOOGLE_LOG_SOURCES})
  link_libraries(GoogleLog)

else()
  CHECK_INCLUDE_FILE_CXX(glog/logging.h HAVE_GOOGLE_LOG_H)
  if (NOT HAVE_GOOGLE_LOG_H)
    message(FATAL_ERROR "Please install the libgoogle-glog-dev package")
  endif()

  link_libraries(glog)
endif()
