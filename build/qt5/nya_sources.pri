
NYA_ENGINE_PATH = $$shell_path($$PWD/../..)
INCLUDEPATH += $$NYA_ENGINE_PATH
# to compile '.o' files (like scene/shader.cpp and render/shader.cpp) in different subdirs
CONFIG += object_parallel_to_source
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder
QMAKE_OBJECTIVE_CFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder
QMAKE_OBJECTIVE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter -Wno-reorder


SOURCES += \
    $${NYA_ENGINE_PATH}/formats/dds.cpp \
    $${NYA_ENGINE_PATH}/formats/nms.cpp \
    $${NYA_ENGINE_PATH}/formats/string_convert.cpp \
    $${NYA_ENGINE_PATH}/formats/text_parser.cpp \
    $${NYA_ENGINE_PATH}/formats/tga.cpp \
    $${NYA_ENGINE_PATH}/log/log.cpp \
    $${NYA_ENGINE_PATH}/log/plain_file_log.cpp \
    $${NYA_ENGINE_PATH}/log/stdout_log.cpp \
    $${NYA_ENGINE_PATH}/log/warning.cpp \
    $${NYA_ENGINE_PATH}/math/bezier.cpp \
    $${NYA_ENGINE_PATH}/math/constants.cpp \
    $${NYA_ENGINE_PATH}/math/frustum.cpp \
    $${NYA_ENGINE_PATH}/math/matrix.cpp \
    $${NYA_ENGINE_PATH}/math/quaternion.cpp \
    $${NYA_ENGINE_PATH}/memory/memory.cpp \
    $${NYA_ENGINE_PATH}/memory/tmp_buffer.cpp \
    $${NYA_ENGINE_PATH}/render/animation.cpp \
    $${NYA_ENGINE_PATH}/render/debug_draw.cpp \
    $${NYA_ENGINE_PATH}/render/fbo.cpp \
    $${NYA_ENGINE_PATH}/render/platform_specific_gl.cpp \
    $${NYA_ENGINE_PATH}/render/render.cpp \
    $${NYA_ENGINE_PATH}/render/shader.cpp \
    $${NYA_ENGINE_PATH}/render/skeleton.cpp \
    $${NYA_ENGINE_PATH}/render/statistics.cpp \
    $${NYA_ENGINE_PATH}/render/texture.cpp \
    $${NYA_ENGINE_PATH}/render/transform.cpp \
    $${NYA_ENGINE_PATH}/render/vbo.cpp \
    $${NYA_ENGINE_PATH}/resources/composite_resources_provider.cpp \
    $${NYA_ENGINE_PATH}/resources/file_resources_provider.cpp \
    $${NYA_ENGINE_PATH}/resources/resources.cpp \
    $${NYA_ENGINE_PATH}/scene/animation.cpp \
    $${NYA_ENGINE_PATH}/scene/camera.cpp \
    $${NYA_ENGINE_PATH}/scene/material.cpp \
    $${NYA_ENGINE_PATH}/scene/mesh.cpp \
    $${NYA_ENGINE_PATH}/scene/scene.cpp \
    $${NYA_ENGINE_PATH}/scene/shader.cpp \
    $${NYA_ENGINE_PATH}/scene/texture.cpp \
    $${NYA_ENGINE_PATH}/scene/transform.cpp \
    $${NYA_ENGINE_PATH}/system/shaders_cache_provider.cpp \
    $${NYA_ENGINE_PATH}/system/system.cpp \
    $${NYA_ENGINE_PATH}/ui/list.cpp \
    $${NYA_ENGINE_PATH}/ui/panel.cpp \
    $${NYA_ENGINE_PATH}/ui/slider.cpp \
    $${NYA_ENGINE_PATH}/ui/ui.cpp

!macx: SOURCES += $${NYA_ENGINE_PATH}/system/app.cpp
macx: OBJECTIVE_SOURCES += $${NYA_ENGINE_PATH}/system/app.mm

# clang gives 'file: XXX has no symbols' error for empty source files
# remove this line if anything changes
macx: SOURCES -= $${NYA_ENGINE_PATH}/render/platform_specific_gl.cpp

HEADERS += \
    $${NYA_ENGINE_PATH}/formats/dds.h \
    $${NYA_ENGINE_PATH}/formats/nms.h \
    $${NYA_ENGINE_PATH}/formats/string_convert.h \
    $${NYA_ENGINE_PATH}/formats/text_parser.h \
    $${NYA_ENGINE_PATH}/formats/tga.h \
    $${NYA_ENGINE_PATH}/gl/glext.h \
    $${NYA_ENGINE_PATH}/gl/wglext.h \
    $${NYA_ENGINE_PATH}/log/log.h \
    $${NYA_ENGINE_PATH}/log/output_stream.h \
    $${NYA_ENGINE_PATH}/log/plain_file_log.h \
    $${NYA_ENGINE_PATH}/log/stdout_log.h \
    $${NYA_ENGINE_PATH}/log/warning.h \
    $${NYA_ENGINE_PATH}/math/bezier.h \
    $${NYA_ENGINE_PATH}/math/constants.h \
    $${NYA_ENGINE_PATH}/math/frustum.h \
    $${NYA_ENGINE_PATH}/math/matrix.h \
    $${NYA_ENGINE_PATH}/math/quaternion.h \
    $${NYA_ENGINE_PATH}/math/vector.h \
    $${NYA_ENGINE_PATH}/memory/indexed_map.h \
    $${NYA_ENGINE_PATH}/memory/invalid_object.h \
    $${NYA_ENGINE_PATH}/memory/memory.h \
    $${NYA_ENGINE_PATH}/memory/memory_reader.h \
    $${NYA_ENGINE_PATH}/memory/memory_writer.h \
    $${NYA_ENGINE_PATH}/memory/optional.h \
    $${NYA_ENGINE_PATH}/memory/pool.h \
    $${NYA_ENGINE_PATH}/memory/shared_ptr.h \
    $${NYA_ENGINE_PATH}/memory/tmp_buffer.h \
    $${NYA_ENGINE_PATH}/render/animation.h \
    $${NYA_ENGINE_PATH}/render/debug_draw.h \
    $${NYA_ENGINE_PATH}/render/fbo.h \
    $${NYA_ENGINE_PATH}/render/platform_specific_gl.h \
    $${NYA_ENGINE_PATH}/render/render.h \
    $${NYA_ENGINE_PATH}/render/render_objects.h \
    $${NYA_ENGINE_PATH}/render/shader.h \
    $${NYA_ENGINE_PATH}/render/skeleton.h \    
	$${NYA_ENGINE_PATH}/render/statistics.h \
    $${NYA_ENGINE_PATH}/render/texture.h \
    $${NYA_ENGINE_PATH}/render/transform.h \
    $${NYA_ENGINE_PATH}/render/vbo.h \
    $${NYA_ENGINE_PATH}/resources/composite_resources_provider.h \
    $${NYA_ENGINE_PATH}/resources/file_resources_provider.h \
    $${NYA_ENGINE_PATH}/resources/resources.h \
    $${NYA_ENGINE_PATH}/resources/shared_resources.h \
    $${NYA_ENGINE_PATH}/scene/animation.h \
    $${NYA_ENGINE_PATH}/scene/camera.h \
    $${NYA_ENGINE_PATH}/scene/material.h \
    $${NYA_ENGINE_PATH}/scene/mesh.h \
    $${NYA_ENGINE_PATH}/scene/proxy.h \
    $${NYA_ENGINE_PATH}/scene/scene.h \
    $${NYA_ENGINE_PATH}/scene/shader.h \
    $${NYA_ENGINE_PATH}/scene/shared_resources.h \
    $${NYA_ENGINE_PATH}/scene/texture.h \
    $${NYA_ENGINE_PATH}/scene/transform.h \
    $${NYA_ENGINE_PATH}/system/app.h \
    $${NYA_ENGINE_PATH}/system/button_codes.h \
    $${NYA_ENGINE_PATH}/system/shaders_cache_provider.h \
    $${NYA_ENGINE_PATH}/system/system.h \
    $${NYA_ENGINE_PATH}/ui/button.h \
    $${NYA_ENGINE_PATH}/ui/label.h \
    $${NYA_ENGINE_PATH}/ui/list.h \
    $${NYA_ENGINE_PATH}/ui/panel.h \
    $${NYA_ENGINE_PATH}/ui/slider.h \
    $${NYA_ENGINE_PATH}/ui/ui.h

