# This will build the QMLForm library

list(APPEND CMAKE_MESSAGE_CONTEXT QMLForm)

file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_library(QMLForm ${SOURCE_FILES})

if(LINUX)

  target_link_libraries(QMLForm PUBLIC ${_LIB_RT})
  target_link_options(QMLForm PUBLIC -lrt)

endif()

target_include_directories(
  QMLForm
  PUBLIC # JASP
          ${PROJECT_SOURCE_DIR}/Common
          ${PROJECT_SOURCE_DIR}/Common/jaspColumnEncoder
         )

target_link_libraries(
  QMLForm
  PUBLIC
         Qt::Core
         Qt::Gui
         Qt::Widgets
         Qt::Qml
         Qt::Quick
         Qt::QuickLayouts
         Qt::QuickControls2
         Qt::QuickControls2Impl
         Qt::QuickWidgets
         Qt::QuickTemplates2
     )

list(POP_BACK CMAKE_MESSAGE_CONTEXT)
