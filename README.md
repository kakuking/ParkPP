<!-- markdownlint-disable-file MD026 -->

# PoggerPark!

To build in windows (with MSBuild) -

```bash
@echo off

cd build
set BUILD_TYPE=Release

cmake  -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..

msbuild PoggerPark.sln -property:Configuration=Release

.\Release\PoggerPark.exe
```
