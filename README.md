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

The source code is compiled using [CMake](https://cmake.org) commands.

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
    cd ..
    make -C build

The source hierarchy is simple, and it has initially 3 source files for the programmer to edit.

- **sources/description.cc** - this is where metadata is built
- **sources/effect.cc** - this is the audio effect
- **sources/ui.cc** - this is the GUI

Once compiled, you will find a lv2 directory structure inside the build directory.
Add this directory to the search path of lv2, and then you may load your plugin in your favorite host.

    export LV2_PATH="`pwd`/build/lv2"
    jalv.gtk 'urn:jpcima:lv2-example'

## Programming UI

The plugin can be associated with many kinds of graphical UIs: Gtk2, Gtk3, Qt4, Qt5, OpenGL, or none.

If you use OpenGL, make sure to also check out David Robillard's [Pugl](https://drobilla.net/software/pugl), a submodule of this framework.
You will find two UI examples with OpenGL, a basic one and an elaborate one based on [NanoVG](https://github.com/memononen/nanovg).

The CMake build environment of this project provides a set of macros to add LV2 UI targets.
These macros offer similar semantics to `add_library(ui MODULE ...)`.
They link the dependencies and rename the target according to LV2 conventions.

    add_lv2_qt5ui   # Qt 5
    add_lv2_qt4ui   # Qt 4
    add_lv2_gtk3ui  # Gtk+ 3
    add_lv2_gtk2ui  # Gtk+ 2
    add_lv2_glui    # OpenGL
    add_lv2_nvgui   # OpenGL with NanoVG

In the manifest of your UI, you want to make sure that its **uiclass** matches the UI toolkit used.
If you develop with OpenGL or native toolkits, use **LV2_UI__PlatformSpecificUI** which is defined as a synonym for the platform's native window system.

Furthermore, if you use the native window system, the UI runs in LV2's idle interface callback.
In order to get an idle callback, do these three things:
- request the **ui:idleInterface** feature;
- declare extension data for **ui:idleInterface**;
- provide the idle interface in the plugin; this is done by returning true in **UI::needs_idle_callback**.

Please note: UIs driven by idle processing have their callbacks invoked at a fixed rate; for performance consideration, it is advisable to save CPU resource by maintaining a dirty state bit in order to avoid redrawing unnecessarily.

## Limitations

There cannot be more than one effect per plugin.
