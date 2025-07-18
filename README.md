<!-- markdownlint-disable-file MD -->

# 👾scenerPark: A Vulkan-Based game engine! 📽️👾

A very simple game "engine" in C++. Only the Vulkan SDK has to be pre-installed, all other libraries are fetched using CMake Fetchfirecr

I made this since I have been very interested in Computer Graphics for a really long time, I have attempted something like this many times. This is maybe the furthest I have got into actually doing it and keeping it clean.

I was initially using GLFW but switched to SDL3 since it has more support for wider range of features including audio, gamepad and events. I also discovered that framerates were much more stable with SDL3 than compared to GLFW

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
11. ~~Maybe~~ Moving from GLFW to SDL ~~or SFML~~ (for audio capabilities and better framerates) 👨‍🔬 
12. Transparency 🔎

## Things to implement

1. Transparent Shadows 🔎⬛
2. A better abstraction for scenes and models 🏞⛲
3. Billboards and particles 🎆
4. More complex materials 🎨
5. Area Lights 🏮
6. Physics and collisions 🎯

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
I don'tscenscen