# skeleton.lv2
Quick-start template for LV2 plugins with GUI

## Description

This is a project template for starting the development of LV2 plugins.

Using this template, the programmer does not have to write plugin metadata (.ttl).
Instead, the metadata is defined in C++ source code, and the introspection with the
help of build system produces ttl files automatically.

The programmer can begin programming GUI using one of the examples source files
provided for each toolkit.

## How to use

This project has submodules, so make sure to check out the dependencies into the project.

The source code is compiled using [CMake](https://cmake.org) commands.

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
    make

The source hierarchy is simple, and it has initially 3 source files for the programmer to edit.

- **sources/description.cc** - this is where metadata is built
- **sources/effect.cc** - this is the audio effect
- **sources/ui.cc** - this is the GUI
