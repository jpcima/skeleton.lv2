#pragma once

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/dynmanifest/dynmanifest.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>

//==============================================================================
// Platform dependent low-level UI types

#if defined(__APPLE__)
# define LV2_UI__PlatformSpecificUI LV2_UI__CocoaUI
#elif defined(_WIN32)
# define LV2_UI__PlatformSpecificUI LV2_UI__WindowsUI
#elif defined(__unix__)
# define LV2_UI__PlatformSpecificUI LV2_UI__X11UI
#else
// unknown UI type for this platform
#endif
