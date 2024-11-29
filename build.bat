if not exist build\NUL mkdir build\

pushd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE="C:\dev\vcpkg\scripts\buildsystems\vcpkg.cmake" -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
ninja
popd build

if %ERRORLEVEL% EQU 0 echo build success && .\build\kyoto.exe
