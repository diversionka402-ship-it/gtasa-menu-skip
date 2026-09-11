cmake_minimum_required(VERSION 3.20)
project(MenuSkip CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# --- Dear ImGui (у него нет своего CMakeLists.txt, берём исходники напрямую) ---
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.90.9
)
FetchContent_MakeAvailable(imgui)

# --- MinHook (для хука DirectX Present) ---
FetchContent_Declare(
  minhook
  GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(minhook)

add_library(MenuSkip SHARED
  src/dllmain.cpp
  src/hooks.cpp
  src/menu.cpp
  src/game.cpp

  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp
)

target_include_directories(MenuSkip PRIVATE
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  src
)

target_link_libraries(MenuSkip PRIVATE
  minhook
  d3d9
)

set_target_properties(MenuSkip PROPERTIES
  OUTPUT_NAME "MenuSkip"
  SUFFIX ".asi"
)
