# This will build the Common library
#
# On Linux,
#   - We need to link to the `librt.so`, and for some reason, CMake cannot find it
#     easily, so, Libraries module tries to find it and provides it to Common here.
#   - Since we are not using Conan, I link to the libjsoncpp, using the PkgConfig variables
#
#
list(APPEND CMAKE_MESSAGE_CONTEXT CommonQt)

configure_file(${CMAKE_CURRENT_LIST_DIR}/utilities/appdirs.h.in
               ${CMAKE_CURRENT_LIST_DIR}/utilities//appdirs.h)
message(STATUS "appdirs.h is successfully generated......")

file(GLOB_RECURSE HEADER_FILES "${CMAKE_CURRENT_LIST_DIR}/*.h")
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_library(CommonQt SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_compile_definitions(CommonQt PRIVATE JASP_COMMONQT_LIB)

if(LINUX)
  target_link_libraries(CommonQt PUBLIC ${_LIB_RT})
  target_link_options(CommonQt PUBLIC -lrt)
  if(LINUX_LOCAL_BUILD)
    target_compile_definitions(CommonQt PUBLIC LINUX_LOCAL_BUILD)
  endif()
  if(FLATPAK_USED)
    target_compile_definitions(CommonQt PUBLIC FLATPAK_USED)
  endif()
endif()

target_compile_definitions(CommonQt PUBLIC JASP_USES_QT_HERE)

target_include_directories(
  CommonQt
  PUBLIC
  ${PROJECT_SOURCE_DIR}/Common
  ${PROJECT_SOURCE_DIR}/CommonData
)


target_link_libraries(
  CommonQt
  PUBLIC
  Common
  CommonData
  Qt::Core
  Qt::Qml
  Qt::Quick
  Qt::Widgets
)


list(POP_BACK CMAKE_MESSAGE_CONTEXT)
