cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Vulkan REQUIRED)

project(PoggerPark LANGUAGES C CXX)

add_subdirectory(ext)

file(GLOB_RECURSE POGGERPARK_SOURCES "src/**/*.cpp" "src/*.cpp")
file(GLOB_RECURSE POGGERPARK_HEADERS "include/**/*.h" "include/*.h")

add_executable(PoggerPark
  ${POGGERPARK_SOURCES}
  ${POGGERPARK_HEADERS}
)

# Include directories
target_include_directories(PoggerPark PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${vma_SOURCE_DIR}/include
  ${glm_SOURCE_DIR}
  ${stb_SOURCE_DIR}
  ${tinygltf_SOURCE_DIR}
  ${imgui_SOURCE_DIR}
  ${joltphysics_SOURCE_DIR}/Jolt
  ${Vulkan_INCLUDE_DIRS}
  ${vk_bootstrap_SOURCE_DIR}/src
  ${glfw_SOURCE_DIR}/include
  ${volk_SOURCE_DIR}/volk
)

# Link libraries
target_link_libraries(PoggerPark PRIVATE
  vk-bootstrap::vk-bootstrap
  fmt::fmt
  Vulkan::Vulkan
  glfw
  volk
)

# ImGui source files
target_sources(PoggerPark PRIVATE
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

# Define for stb (header-only)
target_compile_definitions(PoggerPark PRIVATE
  STB_IMAGE_IMPLEMENTATION
  VOLK_IMPLEMENTATION
)

# Warnings
if(MSVC)
  target_compile_options(PoggerPark PRIVATE /W4 /permissive-)
else()
  target_compile_options(PoggerPark PRIVATE -Wall -Wextra -pedantic -Wno-unused-parameter)
endif()
