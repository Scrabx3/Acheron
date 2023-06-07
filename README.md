# Acheron-SKSE

SKSE64 plugin to alter and expand "death" in Skyrim by adding various alternative condition to "die" and replace "dying" in some instances with custom (external) events

For documentation on how to interact with this plugin see the [wiki](https://github.com/Scrabx3/Acheron-SKSE/wiki).


## Requirements
* [CMake](https://cmake.org/)
	* Add this to your `PATH`
* [PowerShell](https://github.com/PowerShell/PowerShell/releases/latest)
* [Vcpkg](https://github.com/microsoft/vcpkg)
	* Add the environment variable `VCPKG_ROOT` with the value as the path to the folder containing vcpkg
* [Visual Studio Community 2022](https://visualstudio.microsoft.com/)
	* Desktop development with C++
* [CommonLibSSE](https://github.com/powerof3/CommonLibSSE/tree/dev)
	* You need to build from the powerof3/dev branch

## Register Visual Studio as a Generator
* Open `x64 Native Tools Command Prompt`
* Run `cmake`
* Close the cmd window

## Building
```
git clone https://github.com/Scrabx3/Acheron-SKSE.git
cd Acheron-SKSE
# pull commonlib
git submodule init
# to update submodules to checked-out build (warning, pulling newer verisons may result in build problems)
git submodule update
```

### Skyrim SE 1.5
```
cmake --preset vs2022-windows-vcpkg-se
cmake --build build/vs2022-SE --config Release
```
### Skyrim SE 1.6
```
cmake --preset vs2022-windows-vcpkg-ae
cmake --build build/vs2022-AE --config Release
```
