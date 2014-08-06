# WARNING: do not forget to configure qt creator kit (under projects tab) to build in appropiate directory
# this .pro file is tested under clang64 and mingw32 specs

QT -= core gui

NYA_ENGINE_DIR = ../..
include($${NYA_ENGINE_DIR}/build/qt5/nya_engine.pri)

TARGET = nya_engine
TEMPLATE = lib
CONFIG += staticlib
DESTDIR = $$NYA_ENGINE_LIB_DIR

INCLUDEPATH += $$NYA_ENGINE_DIR
# to compile '.o' files (like scene/shader.cpp and render/shader.cpp) in different subdirs
CONFIG += object_parallel_to_source
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder
QMAKE_OBJECTIVE_CFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder
QMAKE_OBJECTIVE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder

SOURCES += \
    $${NYA_ENGINE_DIR}/formats/dds.cpp \
    $${NYA_ENGINE_DIR}/formats/nms.cpp \
    $${NYA_ENGINE_DIR}/formats/string_convert.cpp \
    $${NYA_ENGINE_DIR}/formats/text_parser.cpp \
    $${NYA_ENGINE_DIR}/formats/tga.cpp \
    $${NYA_ENGINE_DIR}/log/log.cpp \
    $${NYA_ENGINE_DIR}/log/plain_file_log.cpp \
    $${NYA_ENGINE_DIR}/log/stdout_log.cpp \
    $${NYA_ENGINE_DIR}/log/warning.cpp \
    $${NYA_ENGINE_DIR}/math/bezier.cpp \
    $${NYA_ENGINE_DIR}/math/constants.cpp \
    $${NYA_ENGINE_DIR}/math/frustum.cpp \
    $${NYA_ENGINE_DIR}/math/matrix.cpp \
    $${NYA_ENGINE_DIR}/math/quaternion.cpp \
    $${NYA_ENGINE_DIR}/memory/memory.cpp \
    $${NYA_ENGINE_DIR}/memory/tmp_buffer.cpp \
    $${NYA_ENGINE_DIR}/render/animation.cpp \
    $${NYA_ENGINE_DIR}/render/debug_draw.cpp \
    $${NYA_ENGINE_DIR}/render/fbo.cpp \
    $${NYA_ENGINE_DIR}/render/platform_specific_gl.cpp \
    $${NYA_ENGINE_DIR}/render/render.cpp \
    $${NYA_ENGINE_DIR}/render/shader.cpp \
    $${NYA_ENGINE_DIR}/render/skeleton.cpp \
    $${NYA_ENGINE_DIR}/render/texture.cpp \
    $${NYA_ENGINE_DIR}/render/transform.cpp \
    $${NYA_ENGINE_DIR}/render/vbo.cpp \
    $${NYA_ENGINE_DIR}/resources/composite_resources_provider.cpp \
    $${NYA_ENGINE_DIR}/resources/file_resources_provider.cpp \
    $${NYA_ENGINE_DIR}/resources/resources.cpp \
    $${NYA_ENGINE_DIR}/scene/animation.cpp \
    $${NYA_ENGINE_DIR}/scene/camera.cpp \
    $${NYA_ENGINE_DIR}/scene/material.cpp \
    $${NYA_ENGINE_DIR}/scene/mesh.cpp \
    $${NYA_ENGINE_DIR}/scene/scene.cpp \
    $${NYA_ENGINE_DIR}/scene/shader.cpp \
    $${NYA_ENGINE_DIR}/scene/texture.cpp \
    $${NYA_ENGINE_DIR}/scene/transform.cpp \
    $${NYA_ENGINE_DIR}/system/shaders_cache_provider.cpp \
    $${NYA_ENGINE_DIR}/system/system.cpp \
    $${NYA_ENGINE_DIR}/ui/list.cpp \
    $${NYA_ENGINE_DIR}/ui/panel.cpp \
    $${NYA_ENGINE_DIR}/ui/slider.cpp \
    $${NYA_ENGINE_DIR}/ui/ui.cpp

macx {
    # clang gives 'file: XXX has no symbols' error for empty source files
    # remove this line if anything changes
    macx: SOURCES -= $${NYA_ENGINE_DIR}/render/platform_specific_gl.cpp
    macx: OBJECTIVE_SOURCES += $${NYA_ENGINE_DIR}/system/app.mm
} else {
    SOURCES += $${NYA_ENGINE_DIR}/system/app.cpp
}

HEADERS += \
    $${NYA_ENGINE_DIR}/formats/dds.h \
    $${NYA_ENGINE_DIR}/formats/nms.h \
    $${NYA_ENGINE_DIR}/formats/string_convert.h \
    $${NYA_ENGINE_DIR}/formats/text_parser.h \
    $${NYA_ENGINE_DIR}/formats/tga.h \
    $${NYA_ENGINE_DIR}/gl/glext.h \
    $${NYA_ENGINE_DIR}/gl/wglext.h \
    $${NYA_ENGINE_DIR}/log/log.h \
    $${NYA_ENGINE_DIR}/log/output_stream.h \
    $${NYA_ENGINE_DIR}/log/plain_file_log.h \
    $${NYA_ENGINE_DIR}/log/stdout_log.h \
    $${NYA_ENGINE_DIR}/log/warning.h \
    $${NYA_ENGINE_DIR}/math/bezier.h \
    $${NYA_ENGINE_DIR}/math/constants.h \
    $${NYA_ENGINE_DIR}/math/frustum.h \
    $${NYA_ENGINE_DIR}/math/matrix.h \
    $${NYA_ENGINE_DIR}/math/quaternion.h \
    $${NYA_ENGINE_DIR}/math/vector.h \
    $${NYA_ENGINE_DIR}/memory/indexed_map.h \
    $${NYA_ENGINE_DIR}/memory/invalid_object.h \
    $${NYA_ENGINE_DIR}/memory/memory.h \
    $${NYA_ENGINE_DIR}/memory/memory_reader.h \
    $${NYA_ENGINE_DIR}/memory/memory_writer.h \
    $${NYA_ENGINE_DIR}/memory/optional.h \
    $${NYA_ENGINE_DIR}/memory/pool.h \
    $${NYA_ENGINE_DIR}/memory/shared_ptr.h \
    $${NYA_ENGINE_DIR}/memory/tmp_buffer.h \
    $${NYA_ENGINE_DIR}/render/animation.h \
    $${NYA_ENGINE_DIR}/render/debug_draw.h \
    $${NYA_ENGINE_DIR}/render/fbo.h \
    $${NYA_ENGINE_DIR}/render/platform_specific_gl.h \
    $${NYA_ENGINE_DIR}/render/render.h \
    $${NYA_ENGINE_DIR}/render/render_objects.h \
    $${NYA_ENGINE_DIR}/render/shader.h \
    $${NYA_ENGINE_DIR}/render/skeleton.h \
    $${NYA_ENGINE_DIR}/render/texture.h \
    $${NYA_ENGINE_DIR}/render/transform.h \
    $${NYA_ENGINE_DIR}/render/vbo.h \
    $${NYA_ENGINE_DIR}/resources/composite_resources_provider.h \
    $${NYA_ENGINE_DIR}/resources/file_resources_provider.h \
    $${NYA_ENGINE_DIR}/resources/resources.h \
    $${NYA_ENGINE_DIR}/resources/shared_resources.h \
    $${NYA_ENGINE_DIR}/scene/animation.h \
    $${NYA_ENGINE_DIR}/scene/camera.h \
    $${NYA_ENGINE_DIR}/scene/material.h \
    $${NYA_ENGINE_DIR}/scene/mesh.h \
    $${NYA_ENGINE_DIR}/scene/proxy.h \
    $${NYA_ENGINE_DIR}/scene/scene.h \
    $${NYA_ENGINE_DIR}/scene/shader.h \
    $${NYA_ENGINE_DIR}/scene/shared_resources.h \
    $${NYA_ENGINE_DIR}/scene/texture.h \
    $${NYA_ENGINE_DIR}/scene/transform.h \
    $${NYA_ENGINE_DIR}/system/app.h \
    $${NYA_ENGINE_DIR}/system/button_codes.h \
    $${NYA_ENGINE_DIR}/system/shaders_cache_provider.h \
    $${NYA_ENGINE_DIR}/system/system.h \
    $${NYA_ENGINE_DIR}/ui/button.h \
    $${NYA_ENGINE_DIR}/ui/label.h \
    $${NYA_ENGINE_DIR}/ui/list.h \
    $${NYA_ENGINE_DIR}/ui/panel.h \
    $${NYA_ENGINE_DIR}/ui/slider.h \
    $${NYA_ENGINE_DIR}/ui/ui.h

OTHER_FILES += \
    nya_engine.pri
