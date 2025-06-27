<!-- markdownlint-disable-file MD026 -->

# PoggerPark!

To build in windows (with MSBuild) -
Note that you need to have [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) installed, and additionally, GLSLC as well!

```bash
@echo off

cd build
set BUILD_TYPE=Release

cmake  -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..

msbuild PoggerPark.sln -property:Configuration=Release

.\Release\PoggerPark.exe
```

## Current features

1. Basic rendering, with vertex and fragment shaders
2. Separation of Game and Engine
3. Textures 

## Things to implement

1. Depth buffering
2. Model loading (currently hardcoded)
3. Anti-aliasing
5. Lighting
6. Shadow mapping
7. Transparency
8. More complex materials
9.  Maybe moving from GLFW to SDL or SFML (for audio capabilities) 
10. Billboards and particles
11. Physics and collisions
