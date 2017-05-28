
set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

if(NOT CMAKE_USE_PTHREADS_INIT)
  message(FATAL_ERROR "libpd requires pthread")
endif()

set(libpd_SOURCE_DIR
  "${PROJECT_SOURCE_DIR}/thirdparty/libpd")

set(libpd_SOURCES
  "${libpd_SOURCE_DIR}/pure-data/src/d_arithmetic.c"
  "${libpd_SOURCE_DIR}/pure-data/src/d_array.c"
  "${libpd_SOURCE_DIR}/pure-data/src/d_ctl.c"
  "${libpd_SOURCE_DIR}/pure-data/src/d_dac.c"
  "${libpd_SOURCE_DIR}/pure-data/src/d_delay.c"
  "${libpd_SOURCE_DIR}/pure-data/src/d_fft.c"
  "${libpd_SOURCE_DIR}/pure-data/src/d_fft_fftsg.c"
  "${libpd_SOURCE_DIR}/pure-data/src/d_filter.c"
  "${libpd_SOURCE_DIR}/pure-data/src/d_global.c"
  "${libpd_SOURCE_DIR}/pure-data/src/d_math.c"
  "${libpd_SOURCE_DIR}/pure-data/src/d_misc.c"
  "${libpd_SOURCE_DIR}/pure-data/src/d_osc.c"
  "${libpd_SOURCE_DIR}/pure-data/src/d_resample.c"
  "${libpd_SOURCE_DIR}/pure-data/src/d_soundfile.c"
  "${libpd_SOURCE_DIR}/pure-data/src/d_ugen.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_all_guis.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_array.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_bang.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_canvas.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_clone.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_editor.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_graph.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_guiconnect.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_hdial.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_hslider.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_io.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_mycanvas.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_numbox.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_readwrite.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_rtext.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_scalar.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_template.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_text.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_toggle.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_traversal.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_vdial.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_vslider.c"
  "${libpd_SOURCE_DIR}/pure-data/src/g_vumeter.c"
  "${libpd_SOURCE_DIR}/pure-data/src/m_atom.c"
  "${libpd_SOURCE_DIR}/pure-data/src/m_binbuf.c"
  "${libpd_SOURCE_DIR}/pure-data/src/m_class.c"
  "${libpd_SOURCE_DIR}/pure-data/src/m_conf.c"
  "${libpd_SOURCE_DIR}/pure-data/src/m_glob.c"
  "${libpd_SOURCE_DIR}/pure-data/src/m_memory.c"
  "${libpd_SOURCE_DIR}/pure-data/src/m_obj.c"
  "${libpd_SOURCE_DIR}/pure-data/src/m_pd.c"
  "${libpd_SOURCE_DIR}/pure-data/src/m_sched.c"
  "${libpd_SOURCE_DIR}/pure-data/src/s_audio.c"
  "${libpd_SOURCE_DIR}/pure-data/src/s_audio_dummy.c"
  "${libpd_SOURCE_DIR}/pure-data/src/s_file.c"
  "${libpd_SOURCE_DIR}/pure-data/src/s_inter.c"
  "${libpd_SOURCE_DIR}/pure-data/src/s_loader.c"
  "${libpd_SOURCE_DIR}/pure-data/src/s_main.c"
  "${libpd_SOURCE_DIR}/pure-data/src/s_path.c"
  "${libpd_SOURCE_DIR}/pure-data/src/s_print.c"
  "${libpd_SOURCE_DIR}/pure-data/src/s_utf8.c"
  "${libpd_SOURCE_DIR}/pure-data/src/x_acoustics.c"
  "${libpd_SOURCE_DIR}/pure-data/src/x_arithmetic.c"
  "${libpd_SOURCE_DIR}/pure-data/src/x_array.c"
  "${libpd_SOURCE_DIR}/pure-data/src/x_connective.c"
  "${libpd_SOURCE_DIR}/pure-data/src/x_gui.c"
  "${libpd_SOURCE_DIR}/pure-data/src/x_interface.c"
  "${libpd_SOURCE_DIR}/pure-data/src/x_list.c"
  "${libpd_SOURCE_DIR}/pure-data/src/x_midi.c"
  "${libpd_SOURCE_DIR}/pure-data/src/x_misc.c"
  "${libpd_SOURCE_DIR}/pure-data/src/x_net.c"
  "${libpd_SOURCE_DIR}/pure-data/src/x_scalar.c"
  "${libpd_SOURCE_DIR}/pure-data/src/x_text.c"
  "${libpd_SOURCE_DIR}/pure-data/src/x_time.c"
  "${libpd_SOURCE_DIR}/pure-data/src/x_vexp.c"
  "${libpd_SOURCE_DIR}/pure-data/src/x_vexp_if.c"
  "${libpd_SOURCE_DIR}/pure-data/src/x_vexp_fun.c"
  "${libpd_SOURCE_DIR}/libpd_wrapper/s_libpdmidi.c"
  "${libpd_SOURCE_DIR}/libpd_wrapper/x_libpdreceive.c"
  "${libpd_SOURCE_DIR}/libpd_wrapper/z_hooks.c"
  "${libpd_SOURCE_DIR}/libpd_wrapper/z_libpd.c")

# libpd extra
set(libpd_SOURCES ${libpd_SOURCES}
  "${libpd_SOURCE_DIR}/pure-data/extra/bob~/bob~.c"
  "${libpd_SOURCE_DIR}/pure-data/extra/bonk~/bonk~.c"
  "${libpd_SOURCE_DIR}/pure-data/extra/choice/choice.c"
  "${libpd_SOURCE_DIR}/pure-data/extra/fiddle~/fiddle~.c"
  "${libpd_SOURCE_DIR}/pure-data/extra/loop~/loop~.c"
  "${libpd_SOURCE_DIR}/pure-data/extra/lrshift~/lrshift~.c"
  "${libpd_SOURCE_DIR}/pure-data/extra/pique/pique.c"
  "${libpd_SOURCE_DIR}/pure-data/extra/sigmund~/sigmund~.c"
  "${libpd_SOURCE_DIR}/pure-data/extra/stdout/stdout.c")

# libpd util
set(libpd_SOURCES ${libpd_SOURCES}
  "${libpd_SOURCE_DIR}/libpd_wrapper/util/z_print_util.c"
  "${libpd_SOURCE_DIR}/libpd_wrapper/util/z_queued.c"
  "${libpd_SOURCE_DIR}/libpd_wrapper/util/ringbuffer.c")

add_library(libpd STATIC ${libpd_SOURCES})
target_compile_definitions(libpd
  PRIVATE "PD=1"
  PRIVATE "PD_INTERNAL=1"
  PRIVATE "HAVE_UNISTD_H=1"
  PRIVATE "USEAPI_DUMMY=1"
  PUBLIC "PDINSTANCE=1")
target_include_directories(libpd
  PUBLIC "${libpd_SOURCE_DIR}/pure-data/src"
  PUBLIC "${libpd_SOURCE_DIR}/libpd_wrapper"
  PUBLIC "${libpd_SOURCE_DIR}/libpd_wrapper/util")
set_target_properties(libpd PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  C_VISIBILITY_PRESET hidden)
target_link_libraries(libpd ${CMAKE_THREAD_LIBS_INIT})

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
  target_link_libraries(libpd ws2_32 kernel32)
else()
  target_compile_definitions(libpd PRIVATE "HAVE_LIBDL=1")
  target_link_libraries(libpd dl)
  if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(libpd m)
  endif()
endif()
