# JASP.cmake sets a lot of JASP parameters, e.g., version. In addition to adding
# some of the CMake variables, it also add some _global_ definition to the compiler
# based on the value of those variables, e.g., `-DJASP_DEBUG`.
#
#
# TODOs:
#   - [ ] Most of these add_definitions should turn into `set_target_definitions`
#       and link to their appropriate targets later on.

list(APPEND CMAKE_MESSAGE_CONTEXT JASP)


set(JASP_VERSION_MSIX_PATCH_POSTFIX
    ""
    CACHE STRING "Microsoft does not allow packages of same version to be uploaded to store and forces TWEAK 0, so we add a large version postfix to PATCH in the appmanifest in case we publish a broken version and wish to switch it out. Nobody sees it anyway")

set(MSIX_STORE_PUBLISHER
    "CN=044465FF-CD1D-4EC4-B82B-C244199F66F9"
    CACHE STRING "Publisher set for store msix package")

set(MSIX_SIDELOAD_PUBLISHER
    "CN=Universiteit van Amsterdam, O=Universiteit van Amsterdam, L=Amsterdam, S=Noord-Holland, C=NL, OID.2.5.4.15=Government Entity, OID.1.3.6.1.4.1.311.60.2.1.3=NL, SERIALNUMBER=34370207"
    CACHE STRING "Publisher set for sideloaded msix package")

set(MSIX_NIGHTLY_PUBLISHER
    "CN=JASP, O=JASP, L=Amsterdam, S=Noord-Holland, C=NL"
    CACHE STRING "Publisher set for nightly msix package")

set(MSIX_SIGN_CERT_PATH
  "D:\\JASPSelfSign.pfx"
  CACHE STRING "Path to selfsign cert for Nightlies")

set(MSIX_SIGN_CERT_PASSWORD
  "0000"
  CACHE STRING "Password selfsign cert for Nightlies")


# TODO:
# - [ ] Rename all JASP related variables to `JASP_*`. This way,
#       Qt Creator can categorize them nicely in its CMake configurator
option(JASP_PRINT_ENGINE_MESSAGES
       "Whether or not JASPEngine prints log messages" ON)

if(NOT R_REPOSITORY)
  set(R_REPOSITORY
      "http://cloud.r-project.org"
      CACHE STRING "The CRAN mirror used by 'renv' and 'install.packages'")
endif()

if(FLATPAK_USED AND LINUX)
  set(R_REPOSITORY "file:///app/lib64/local-cran")
endif()

message(STATUS "CRAN mirror: ${R_REPOSITORY}")

# This one is GLOBAL
# should be off for flatpak though because it is always build in debug mode (but the symbols are split off)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT FLATPAK_USED)
  add_definitions(-DJASP_DEBUG)
endif()

option(PRINT_ENGINE_MESSAGES
       "Indicates whether the log contains JASPEngine messages" ON)

option(JASP_USES_QT_HERE "Indicates whether some projects are using Qt" ON)

# add_definitions(-DJASP_RESULTS_DEBUG_TRACES)

option(JASP_TIMER_USED "Use JASP timer for profiling" OFF)
if(JASP_TIMER_USED)
  add_definitions(-DPROFILE_JASP)
endif()

option(UPDATE_JASP_SUBMODULES
       "Whether to automatically initialize and update the submodules" OFF)


# Dealing with Git submodules

if(UPDATE_JASP_SUBMODULES)
  if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    message(STATUS "Submodule update")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT
       GIT_SUBMOD_RESULT
       EQUAL
       "0")
      message(
        FATAL_ERROR
          "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules"
      )
    endif()
  endif()
endif()

list(POP_BACK CMAKE_MESSAGE_CONTEXT)
