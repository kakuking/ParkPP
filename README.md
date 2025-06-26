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

## Things to implement

1. Depth buffering
2. Lighting
3. Model loading (currently hardcoded)
4. Shadow mapping
5. More complex materials
6. Textures (color and normal)
7. Maybe moving from GLFW to SDL or SFML (for audio capabilities) 
8. Transparency
9. Anti-aliasing
10. Billboards and particles
11. Physics and collisions
