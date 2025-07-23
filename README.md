<!-- markdownlint-disable-file MD -->

# 👾PoggerPark: A Vulkan-Based game engine! 📽️👾

A very simple game "engine" in C++. Only the Vulkan SDK has to be pre-installed; all other libraries are fetched using CMake FetchContent.

https://github.com/user-attachments/assets/e7541e06-fd3c-400b-888e-bf18c0adb40d

I made this since I have been very interested in Computer Graphics for a really long time. I have attempted something like this many times. This is maybe the furthest I have gotten into actually doing it and keeping it clean.

I was initially using GLFW but switched to SDL3 since it has more support for a wider range of features, including audio, gamepad and events. I also discovered that framerates were much more stable with SDL3 than compared to GLFW

Most of the Vulkan Functionalities are quite basic. I picked them up from the [Vulkan-Tutorial](https://vulkan-tutorial.com/Introduction). If you are learning Vulkan I highly recommend going through the tutorial 😊

## Current features

1. Basic rendering, with vertex and fragment shaders 🎥
2. Separation of Game and Engine hand 👐
3. Textures 🖼
4. Depth buffering 🌊
5. Model loading (currently hardcoded) 🗽
6. Anti-aliasing ⬛
7. Texture Arrays 🖼🖼
8. Abstracted UBOs and render pass stuff ⚙⚙
9. Multiple textures in one model 🖼🖼🖼
10. ~~Point~~ Directional Lights ☀ & Shadow mapping 🔦
11. ~~Maybe~~ Moving from GLFW to SDL ~~or SFML~~ (for audio capabilities and better framerates) 👨‍🔬 
12. Transparency 🔎
13. A better abstraction for scenes and models 🏞⛲
14. Added support for gltf models

## Things to implement

1. Billboards 🌿 
2. Area Lights 🏮
3. More complex materials 🎨
4. Physics and collisions 🎯

## Ideas for the future
* Transparent Shadows 🔎⬛
* Particles 🎆

## To build - 
### Windows (with MSBuild) -
Note that you need to have [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) installed, and additionally, GLSLC as well!

```batch
@echo off

cd build
set BUILD_TYPE=Release

cmake  -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..

msbuild PoggerPark.sln -property:Configuration=Release

.\Release\PoggerPark.exe
```

To detach the GUI from a terminal, uncomment the appropriate line in CMakeLists.txt in the *"add_executable"* call.

### Mac & Linux - 
I don't know 😅
