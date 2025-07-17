<!-- markdownlint-disable-file MD -->

# 👾🎮 PoggerPark: A Vulkan-Based game engine! 📽️👾

A very simple game "engine" in C++. Only the Vulkan SDK has to be pre-installed, all other libraries are fetched using CMake FetchContent!

I made this since I have been very interested in Computer Graphics for a really long time, I have attempted something like this many times. This is maybe the furthest I have got into actually doing it and keeping it clean.

Most of the Vulkan Functionalities are quite basic, I picked them up from the [Vulkan-Tutorial](https://vulkan-tutorial.com/Introduction). If you are learning vulkan I highly recommend going through the tutorial 😊

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

## Things to implement

1. Transparency 🔎
2. Billboards and particles 🎆
3. More complex materials 🎨
4. Area Lights 🏮
5. Physics and collisions 🎯
6. Maybe moving from GLFW to SDL or SFML (for audio capabilities) 👨‍🔬 

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

### Mac & Linux - 
I don't know 😅