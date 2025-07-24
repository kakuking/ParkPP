cmake_minimum_required(VERSION 3.14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Vulkan REQUIRED)

project(PoggerPark LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(ext)

file(GLOB_RECURSE POGGERPARK_SOURCES "src/**/*.cpp" "src/*.cpp")
file(GLOB_RECURSE POGGERPARK_HEADERS "include/**/*.h" "include/*.h")

if (WIN32)
  add_executable(PoggerPark
    # WIN32 # Uncomment to detach GUI from terminal
    ${POGGERPARK_SOURCES}
    ${POGGERPARK_HEADERS}
  )
else()
  add_executable(PoggerPark
    ${POGGERPARK_SOURCES}
    ${POGGERPARK_HEADERS}
  )
endif()


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
  ${SDL3_SOURCE_DIR}/include
  ${volk_SOURCE_DIR}/volk
  ${tinyobjloader_SOURCE_DIR}
  ${pugixml_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(PoggerPark PRIVATE
  vk-bootstrap::vk-bootstrap
  fmt::fmt
  Vulkan::Vulkan
  SDL3::SDL3
  pugixml
  volk
)

if (WIN32)
  add_custom_command(TARGET PoggerPark POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:SDL3::SDL3>"
      $<TARGET_FILE_DIR:PoggerPark>
  )

  # set_target_properties(PoggerPark PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# ImGui source files
target_sources(PoggerPark PRIVATE
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

# Define for stb (header-only)
target_compile_definitions(PoggerPark PRIVATE
  TINYGLTF_IMPLEMENTATION
  VOLK_IMPLEMENTATION
)

# Warnings
if(MSVC)
  target_compile_options(PoggerPark PRIVATE /W4 /permissive-)
else()
  target_compile_options(PoggerPark PRIVATE -Wall -Wextra -pedantic -Wno-unused-parameter)
endif()

# Shader Compilation
file(GLOB SHADER_SOURCES "${CMAKE_SOURCE_DIR}/resources/shaders/*.vert" "${CMAKE_SOURCE_DIR}/resources/shaders/*.frag")

set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/$<CONFIG>/shaders")

foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(COMPILED_SHADER "${SHADER_OUTPUT_DIR}/${FILE_NAME}.spv")

    add_custom_command(
        OUTPUT ${COMPILED_SHADER}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${COMPILED_SHADER}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${FILE_NAME}"
        VERBATIM
    )

    list(APPEND COMPILED_SHADERS ${COMPILED_SHADER})
endforeach()

add_custom_target(CompileShaders ALL DEPENDS ${COMPILED_SHADERS})
add_dependencies(PoggerPark CompileShaders)

# Resource Copy
add_custom_command(
    TARGET PoggerPark PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources
            ${CMAKE_BINARY_DIR}/$<CONFIG>
    COMMENT "Copying resources to build directory"
)
