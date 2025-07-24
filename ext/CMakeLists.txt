cmake_minimum_required(VERSION 3.14)

include(FetchContent)

# Vulkan Memory Allocator
FetchContent_Declare(
  vma
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG v3.3.0
)
FetchContent_MakeAvailable(vma)

# SDL3
FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-3.2.8
)
FetchContent_MakeAvailable(SDL3)

# GLM
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# stb_image
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG f58f558c120e9b32c217290b80bad1a0729fbb2c
)
FetchContent_MakeAvailable(stb)

# ImGui
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.9b
)
FetchContent_MakeAvailable(imgui)


# tinygltf
FetchContent_Declare(
  tinygltf
  GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
  GIT_TAG v2.9.6
)
FetchContent_MakeAvailable(tinygltf)

FetchContent_Declare(
  tinyobjloader
  GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
  GIT_TAG v2.0.0rc10
)
FetchContent_MakeAvailable(tinyobjloader)

# fmt
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 11.2.0
)
FetchContent_MakeAvailable(fmt)

# Jolt Physics
FetchContent_Declare(
  joltphysics
  GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
  GIT_TAG v5.3.0
)
FetchContent_MakeAvailable(joltphysics)

# volk (Vulkan function loader)
FetchContent_Declare(
  volk
  GIT_REPOSITORY https://github.com/zeux/volk.git
  GIT_TAG 1.4.304
)
FetchContent_MakeAvailable(volk)

# vk-bootstrap (Vulkan init helper)
FetchContent_Declare(
  vk_bootstrap
  GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap.git
  GIT_TAG v1.4.315
)
FetchContent_MakeAvailable(vk_bootstrap)

# pugixml (XML parser)
FetchContent_Declare(
  pugixml
  GIT_REPOSITORY https://github.com/zeux/pugixml.git
  GIT_TAG v1.14
)
FetchContent_MakeAvailable(pugixml)

set(imgui_SOURCE_DIR ${imgui_SOURCE_DIR} CACHE INTERNAL "imgui source dir")
set(vma_SOURCE_DIR ${vma_SOURCE_DIR} CACHE INTERNAL "vma_SOURCE_DIR")
set(glm_SOURCE_DIR ${glm_SOURCE_DIR} CACHE INTERNAL "glm_SOURCE_DIR")
set(stb_SOURCE_DIR ${stb_SOURCE_DIR} CACHE INTERNAL "stb_SOURCE_DIR")
set(tinygltf_SOURCE_DIR ${tinygltf_SOURCE_DIR} CACHE INTERNAL "tinygltf_SOURCE_DIR")
set(imgui_SOURCE_DIR ${imgui_SOURCE_DIR} CACHE INTERNAL "imgui_SOURCE_DIR")
set(joltphysics_SOURCE_DIR ${joltphysics_SOURCE_DIR} CACHE INTERNAL "joltphysics_SOURCE_DIR")
set(Vulkan_INCLUDE_DIRS ${Vulkan_INCLUDE_DIRS} CACHE INTERNAL "Vulkan_INCLUDE_DIRS")
set(vk_bootstrap_SOURCE_DIR ${vk_bootstrap_SOURCE_DIR} CACHE INTERNAL "vk_bootstrap_SOURCE_DIR")
set(volk_SOURCE_DIR ${volk_SOURCE_DIR} CACHE INTERNAL "volk_SOURCE_DIR")
set(SDL3_SOURCE_DIR ${SDL3_SOURCE_DIR} CACHE INTERNAL "SDL3_SOURCE_DIR")
set(tinyobjloader_SOURCE_DIR ${tinyobjloader_SOURCE_DIR} CACHE INTERNAL "tinyobjloader source dir")
set(pugixml_SOURCE_DIR ${pugixml_SOURCE_DIR} CACHE INTERNAL "pugixml_SOURCE_DIR")
